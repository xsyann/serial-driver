/*
** serial_driver.c
**
** Made by xsyann
** Contact <contact@xsyann.com>
**
** Started on  Wed Apr  9 14:07:16 2014 xsyann
** Last update Tue Apr 15 10:08:41 2014 xsyann
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/interrupt.h>
#include <linux/serial_reg.h>

#include "serial_driver.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR(SD_AUTHOR);
MODULE_DESCRIPTION(SD_DESC);
MODULE_VERSION(SD_VERSION);

static struct class *sd_class = NULL;   /* Device class (/sys/class) */
static struct sd_data *sd_data = NULL;    /* Device wrapper */

static int port = 0x2f8; /* COM2 */
static int irq = 3;

static int sd_open(struct inode *inode, struct file *filp)
{
        unsigned int major = 0;
        unsigned int minor = 0;

        major = imajor(inode);
        minor = iminor(inode);
        /* Check minor and major */
        if (MKDEV(major, minor) != sd_data->devno ||
            inode->i_cdev != &sd_data->cdev)
        {
                PR_WARNING(SD_ERR_NOTFOUND, major, minor);
                return -ENODEV;
        }
        PR_INFO("open()");
        return 0;
}

static int sd_release(struct inode *inode, struct file *filp)
{
        PR_INFO("release()");
        return 0;
}

static ssize_t sd_read(struct file *filp, char __user *buf,
                              size_t count, loff_t *ppos)
{
        unsigned char c;
        int err;
        ssize_t copied;
        unsigned int i;
        int line_status = inb(port + UART_LSR);

        PR_INFO("read()");

        /* Data not ready */
        if ((line_status & UART_LSR_DR) == 0) {
                if (filp->f_flags & O_NONBLOCK)
                        return -EAGAIN;
                err = wait_event_interruptible(sd_data->read_wq,
                                               inb(port + UART_LSR) & 1);
                if (err == -ERESTARTSYS)
                        return -EINTR;
        }

        if (mutex_lock_interruptible(&sd_data->read_mutex))
                return -EINTR;
        copied = 0;
        for (i = 0; i < count; ++i) {
                c = inb(port + UART_RX);
                if (copy_to_user(buf + i, &c, 1))
                        return -EFAULT;
                ++copied;
                if ((inb(port + UART_LSR) & UART_LSR_DR) == 0)
                        break;
        }
        mutex_unlock(&sd_data->read_mutex);
        return copied;
}

static ssize_t sd_write(struct file *filp, const char __user *buf,
                              size_t count, loff_t *ppos)
{
        unsigned char c;
        int err;
        unsigned int i;
        ssize_t copied;
        int modem_status = inb(port + UART_MSR);

        PR_INFO("write()");

        /* Clear To Send */
        if ((modem_status & UART_MSR_CTS) != UART_MSR_CTS)
        {
                if (filp->f_flags & O_NONBLOCK)
                        return -EAGAIN;
                err = wait_event_interruptible(
                        sd_data->write_wq,
                        (inb(port + UART_MSR) & UART_MSR_CTS) == UART_MSR_CTS);
                if (err == -ERESTARTSYS)
                        return -EINTR;
        }

        if (mutex_lock_interruptible(&sd_data->read_mutex))
                return -EINTR;
        copied = 0;
        for (i = 0; i < count; ++i) {
                if (copy_from_user(&c, buf + i, 1))
                        return -EFAULT;
                while ((inb(port + UART_LSR) & UART_LSR_THRE) == 0);
                outb(c, port + UART_TX);
                ++copied;
                if ((inb(port + UART_MSR) & UART_MSR_CTS) != UART_MSR_CTS)
                        break;
        }

        mutex_unlock(&sd_data->read_mutex);
        return count;
}

unsigned int sd_poll(struct file *filp, struct poll_table_struct *pt)
{
        unsigned int    mask = 0;

        poll_wait(filp, &sd_data->read_wq, pt);
        poll_wait(filp, &sd_data->write_wq, pt);

        /* Receiver data ready */
        if (inb(port + UART_LSR) & UART_LSR_DR)
                mask |= (POLLIN | POLLRDNORM);

        /* Transmit-hold-register empty */
        if (inb(port + UART_LSR) & UART_LSR_THRE)
                mask |= (POLLOUT | POLLWRNORM);

        return mask;
}

static struct file_operations sd_fops = {
        .owner = THIS_MODULE,
        .read = sd_read,
        .write = sd_write,
        .open = sd_open,
        .poll = sd_poll,
        .release = sd_release,
};

/* ************************************************************* */

static irqreturn_t sd_isr(int irq, void *dev_id)
{
        int interrupt_id = inb(port + UART_IIR) & UART_IIR_ID;
        /* No interrupts pending */
        if (interrupt_id & UART_IIR_NO_INT)
                return IRQ_NONE;

        switch (interrupt_id)
        {
        case 0x0C: /* Character timeout */
        case UART_IIR_RDI: /* Received data available */
                wake_up_interruptible(&sd_data->read_wq);
                break;
        case UART_IIR_THRI: /* Transmitter holding register empty */
                wake_up_interruptible(&sd_data->write_wq);
                break;
        case UART_IIR_RLSI: /* Receiver line status interrupt */
                inb(port + UART_LSR);
                break;
        case UART_IIR_MSI: /* Modem status interrupt */
                inb(port + UART_MSR);
                break;
 }
        return IRQ_HANDLED;
}

static int uart_config(void)
{
        int err = 0;

        outb(0, port + UART_IER);
        outb(UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RCVR |
             UART_FCR_CLEAR_XMIT | UART_FCR6_R_TRIGGER_28, port + UART_FCR);

        outb(UART_LCR_WLEN8 | UART_LCR_DLAB, port + UART_LCR);
        outb(0x01, port + UART_DLL);
        outb(0x00, port + UART_DLM);
        outb(UART_LCR_WLEN8, port + UART_LCR);

        outb(UART_MCR_DTR | UART_MCR_RTS, port + UART_MCR);

        /* RO registers */
        inb(port + UART_MSR);
        inb(port + UART_LSR);
        inb(port + UART_RX);
        inb(port + UART_IIR);

        err = request_irq(irq, sd_isr, IRQF_SHARED, KBUILD_MODNAME, THIS_MODULE);
        if (err < 0) {
                PR_WARNING(SD_ERR_UARTFAIL);
                return err;
        }

        outb(UART_MCR_DTR | UART_MCR_RTS | UART_MCR_OUT2, port + UART_MCR);
        outb(UART_IER_RDI | UART_IER_RLSI | UART_IER_THRI, port + UART_IER);

        return err;
}

static void uart_clean(void)
{
        outb(0, port + UART_FCR);
        outb(0, port + UART_MCR);
        outb(0, port + UART_IER);
        free_irq(irq, THIS_MODULE);
}

/* ************************************************************* */

static int sd_alloc_device(struct sd_data **dev)
{
        *dev = kzalloc(sizeof(**dev), GFP_KERNEL);
        if (*dev == NULL) {
                return -ENOMEM;
        }
        return 0;
}

static void sd_free_device(struct sd_data *dev)
{
        if (dev)
                kfree(dev);
}

/* Add char device to system
 * Call device_create() to auto create (udev daemon)
 * the node in /dev.
 */
static int sd_create_device(struct sd_data *dev, int major, int minor,
                            struct class *class)
{
        int err = 0;
        struct device *device = NULL;

        dev->devno = MKDEV(major, minor);
        mutex_init(&sd_data->read_mutex);
        mutex_init(&sd_data->write_mutex);
        init_waitqueue_head(&dev->read_wq);
        init_waitqueue_head(&dev->write_wq);

        cdev_init(&dev->cdev, &sd_fops);
        err = cdev_add(&dev->cdev, dev->devno, 1);
        if (err < 0) {
                PR_WARNING(SD_ERR_DEVADD);
                return err;
        }

        device = device_create(class, NULL, dev->devno, NULL, KBUILD_MODNAME);
        if (IS_ERR(device)) {
                err = PTR_ERR(device);
                PR_WARNING(SD_ERR_DEVCREATE);
                cdev_del(&dev->cdev);
                return err;
        }
        return 0;
}

static void sd_destroy_device(struct sd_data *dev)
{
        if (dev != NULL)
        {
                device_destroy(sd_class, dev->devno);
                cdev_del(&dev->cdev);
        }
}

static void sd_cleanup_device(void)
{
        if (sd_class != NULL)
                class_destroy(sd_class);
        unregister_chrdev_region(sd_data->devno, 1);
}

/* Register char device (dynamic major).
 * Create device class.
 * The *major parameter is updated with major number.
 */
static int sd_setup_device(int *major)
{
        int err = 0;
        dev_t first;

        /* Start with minor 0, register 1 device */
        err = alloc_chrdev_region(&first, 0, 1, KBUILD_MODNAME);
        if (err < 0) {
                PR_WARNING(SD_ERR_MAJOR);
                return err;
        }

        *major = MAJOR(first);

        sd_class = class_create(THIS_MODULE, KBUILD_MODNAME);
        if (IS_ERR(sd_class)) {
                err = PTR_ERR(sd_class);
                sd_cleanup_device();
                return err;
        }
        return 0;
}

static int sd_register_device(void)
{
        int err = 0;
        int major = 0;

        if ((err = sd_setup_device(&major))) {
                return err;
        }
        if ((err = sd_alloc_device(&sd_data))) {
                sd_cleanup_device();
                return err;
        }
        if ((err = sd_create_device(sd_data, major, 0, sd_class))) {
                sd_free_device(sd_data);
                sd_cleanup_device();
                return err;
        }
        PR_INFO("%d:%d", MAJOR(sd_data->devno), MINOR(sd_data->devno));
        return err;
}

static void sd_unregister_device(void)
{
        sd_destroy_device(sd_data);
        sd_free_device(sd_data);
        sd_cleanup_device();
}

/* ************************************************************* */

static int __init sd_init_module(void)
{
        int err = 0;

        PR_INFO(SD_INFO_LOAD);
        err = sd_register_device();
        if (err < 0) {
                return err;
        }
        err = uart_config();
        if (err < 0) {
                PR_WARNING(SD_ERR_UARTFAIL);
                return err;
        }

        return err;
}

static void __exit sd_exit_module(void)
{
        PR_INFO(SD_INFO_UNLOAD);
        uart_clean();
        sd_unregister_device();
}

module_init(sd_init_module);
module_exit(sd_exit_module);
