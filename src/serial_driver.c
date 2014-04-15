/*
** serial_driver.c
**
** Made by xsyann
** Contact <contact@xsyann.com>
**
** Started on  Wed Apr  9 14:07:16 2014 xsyann
** Last update Tue Apr 15 20:31:03 2014 xsyann
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
#include <linux/seq_file.h>
#include <linux/proc_fs.h>

#include "serial_driver.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR(SD_AUTHOR);
MODULE_DESCRIPTION(SD_DESC);
MODULE_VERSION(SD_VERSION);

static struct class *sd_class = NULL;   /* Device class (/sys/class) */
static struct sd_dev *sd_dev = NULL;    /* Devices wrappers */
static int sd_major = 0;                /* Major number */
static int sd_ndevs = SD_NCOM;          /* Number of Devices */

static const struct sd_com_info sd_ports[SD_NCOM] = {
        {0, 0x3f8, 4}, /* COM1 */
        {1, 0x2f8, 3}, /* COM2 */
        {2, 0x3e8, 4}, /* COM3 */
        {3, 0x2e8, 3}, /* COM4 */
};

/* ************************************************************ */

static struct sd_dev *sd_get_device(int minor)
{
        int i;

        for (i = 0; i < sd_ndevs; ++i)
                if (MINOR(sd_dev[i].devno) == minor)
                        return &sd_dev[i];
        return NULL;
}

/* ************************************************************ */

static int sd_open(struct inode *inode, struct file *filp)
{
        unsigned int major = 0;
        unsigned int minor = 0;
        struct sd_dev *dev;

        major = imajor(inode);
        minor = iminor(inode);
        /* Check minor and major */
        if (major != sd_major || minor < 0 ||
            minor > sd_ndevs)
        {
                PR_WARNING(SD_ERR_NOTFOUND, major, minor);
                return -ENODEV;
        }
        PR_INFO("open()");
        dev = sd_get_device(minor);
        /* Data terminal ready | Request to send */
        outb(UART_MCR_DTR | UART_MCR_RTS | UART_MCR_OUT2,
             dev->com.uart_port + UART_MCR);
        return 0;
}

static int sd_release(struct inode *inode, struct file *filp)
{
        int minor = iminor(inode);
        struct sd_dev *dev = sd_get_device(minor);
        int port = dev->com.uart_port;
        /* Not ready */
        outb(0x00, port + UART_MCR);
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
        int line_status;
        struct sd_dev *dev;
        int port;

        /* Get device */
        dev = sd_get_device(iminor(filp->f_path.dentry->d_inode));
        port = dev->com.uart_port;
        PR_INFO("read() %d", MINOR(dev->devno));

        line_status = inb(port + UART_LSR);
        /* Data not ready */
        if ((line_status & UART_LSR_DR) == 0) {
                if (filp->f_flags & O_NONBLOCK)
                        return -EAGAIN;
                err = wait_event_interruptible(dev->read_wq,
                                               inb(port + UART_LSR) & 1);
                if (err == -ERESTARTSYS)
                        return -EINTR;
        }

        /* Lock */
        if (mutex_lock_interruptible(&dev->read_mutex))
                return -EINTR;
        /* Read */
        copied = 0;
        for (i = 0; i < count; ++i) {
                c = inb(port + UART_RX);
                if (copy_to_user(buf + i, &c, 1))
                        return -EFAULT;
                ++copied;
                if ((inb(port + UART_LSR) & UART_LSR_DR) == 0)
                        break;
        }
        dev->rx += copied;
        /* Unlock */
        mutex_unlock(&dev->read_mutex);
        return copied;
}

static ssize_t sd_write(struct file *filp, const char __user *buf,
                              size_t count, loff_t *ppos)
{
        unsigned char c;
        int err;
        unsigned int i;
        ssize_t copied;
        int modem_status;
        struct sd_dev *dev;
        int port;

        /* Get device */
        dev = sd_get_device(iminor(filp->f_path.dentry->d_inode));
        port = dev->com.uart_port;
        PR_INFO("write() %d", MINOR(dev->devno));

        modem_status = inb(port + UART_MSR);
        /* Clear To Send */
        if ((modem_status & UART_MSR_CTS) != UART_MSR_CTS)
        {
                if (filp->f_flags & O_NONBLOCK)
                        return -EAGAIN;
                err = wait_event_interruptible(
                        dev->write_wq,
                        (inb(port + UART_MSR) & UART_MSR_CTS) == UART_MSR_CTS);
                if (err == -ERESTARTSYS)
                        return -EINTR;
        }

        /* Lock */
        if (mutex_lock_interruptible(&dev->read_mutex))
                return -EINTR;
        /* Write */
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
        dev->tx += copied;
        /* Unlock */
        mutex_unlock(&dev->read_mutex);
        return copied;
}

unsigned int sd_poll(struct file *filp, struct poll_table_struct *pt)
{
        unsigned int mask = 0;
        struct sd_dev *dev;
        int port;

        /* Get Device */
        dev = sd_get_device(iminor(filp->f_path.dentry->d_inode));
        port = dev->com.uart_port;

        /* Enqueue current process */
        poll_wait(filp, &dev->read_wq, pt);
        poll_wait(filp, &dev->write_wq, pt);

        /* Receiver data ready */
        if (inb(port + UART_LSR) & UART_LSR_DR)
                mask |= (POLLIN | POLLRDNORM);

        /* Transmit-hold-register empty */
        if (inb(port + UART_LSR) & UART_LSR_THRE)
                mask |= (POLLOUT | POLLWRNORM);

        return mask;
}

static const struct file_operations sd_fops = {
        .owner = THIS_MODULE,
        .read = sd_read,
        .write = sd_write,
        .open = sd_open,
        .poll = sd_poll,
        .release = sd_release,
};

/* ************************************************************* */

static void sd_proc_show_status_flag(struct seq_file *sfile,
                                     int predicate, const char *name, char *sep)
{
        if (predicate)
        {
                seq_printf(sfile, "%c%s", *sep, name);
                *sep = '|';
        }
}

static void sd_proc_show_status(struct seq_file *sfile, int msr, int mcr)
{
        char sep = ' ';

        sd_proc_show_status_flag(sfile, mcr & UART_MCR_RTS, "RTS", &sep);
        sd_proc_show_status_flag(sfile, msr & UART_MSR_CTS, "CTS", &sep);
        sd_proc_show_status_flag(sfile, mcr & UART_MCR_DTR, "DTR", &sep);
        sd_proc_show_status_flag(sfile, msr & UART_MSR_DSR, "DSR", &sep);
        sd_proc_show_status_flag(sfile, msr & UART_MSR_DCD, "CD", &sep);
        sd_proc_show_status_flag(sfile, msr & UART_MSR_RI, "RI", &sep);
}

static int sd_proc_show(struct seq_file *sfile, void *v)
{
        int i;
        int mcr;
        int msr;
        struct sd_dev *dev = NULL;

        for (i = 0; i < sd_ndevs; ++i)
        {
                dev = sd_get_device(i);
                mcr = inb(dev->com.uart_port + UART_MCR);
                msr = inb(dev->com.uart_port + UART_MSR);
                seq_printf(sfile, "%d: port: %08X irq: %d tx:%ld rx:%ld",
                           i, dev->com.uart_port, dev->com.irq, dev->tx, dev->rx);
                sd_proc_show_status(sfile, msr, mcr);
                seq_printf(sfile, "\n");
        }
        return 0;
}

static int sd_proc_open(struct inode *inode, struct file *file)
{
        return single_open(file, sd_proc_show, NULL);
}

static const struct file_operations sd_proc_fops = {
        .open = sd_proc_open,
        .read = seq_read,
        .llseek = seq_lseek,
        .release = single_release,
};

/* ************************************************************* */

 irqreturn_t sd_isr(int irq, void *dev_id)
{
        int port;
        int interrupt_id;
        struct sd_dev *dev = (struct sd_dev *)dev_id;

        if (dev == NULL)
                return IRQ_NONE;
        port = dev->com.uart_port;

        interrupt_id = inb(port + UART_IIR) & UART_IIR_ID;
        /* No interrupts pending */
        if (interrupt_id & UART_IIR_NO_INT)
                return IRQ_NONE;

        switch (interrupt_id)
        {
        case 0x0C: /* Character timeout */
        case UART_IIR_RDI: /* Received data available */
                wake_up_interruptible(&dev->read_wq);
                break;
        case UART_IIR_THRI: /* Transmitter holding register empty */
                wake_up_interruptible(&dev->write_wq);
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

static int uart_config(struct sd_dev *dev)
{
        int err = 0;
        int port = dev->com.uart_port;

        outb(0, port + UART_IER);
        outb(UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RCVR |
             UART_FCR_CLEAR_XMIT | UART_FCR6_R_TRIGGER_28, port + UART_FCR);

        outb(UART_LCR_WLEN8 | UART_LCR_DLAB, port + UART_LCR);
        outb(0x01, port + UART_DLL);
        outb(0x00, port + UART_DLM);
        outb(UART_LCR_WLEN8, port + UART_LCR);

        /* RO registers */
        inb(port + UART_MSR);
        inb(port + UART_LSR);
        inb(port + UART_RX);
        inb(port + UART_IIR);

        err = request_irq(dev->com.irq, sd_isr, IRQF_SHARED, dev->name, dev);
        if (err < 0) {
                PR_WARNING(SD_ERR_UARTFAIL);
                return err;
        }

        outb(UART_IER_RDI | UART_IER_RLSI | UART_IER_THRI, port + UART_IER);

        return err;
}

static void uart_clean(struct sd_dev *dev)
{
        int port = dev->com.uart_port;

        outb(0, port + UART_FCR);
        outb(0, port + UART_MCR);
        outb(0, port + UART_IER);
        free_irq(dev->com.irq, dev);
}

/* ************************************************************* */

/* Alloc a device wrapper array */
static int sd_alloc_devices(struct sd_dev **dev)
{
        *dev = kzalloc(sd_ndevs * sizeof(**dev), GFP_KERNEL);
        if (*dev == NULL) {
                return -ENOMEM;
        }
        return 0;
}

/* Initialize a device wrapper */
static void sd_dev_init(struct sd_dev *dev, int minor)
{
        int i;

        dev->tx = 0;
        dev->rx = 0;
        dev->devno = MKDEV(sd_major, minor);
        mutex_init(&dev->read_mutex);
        mutex_init(&dev->write_mutex);
        init_waitqueue_head(&dev->read_wq);
        init_waitqueue_head(&dev->write_wq);

        for (i = 0; i < sd_ndevs; ++i)
                if (sd_ports[i].id == minor)
                        dev->com = sd_ports[i];
}

/* Add char device to system
 * Call device_create() to auto create (udev daemon)
 * the node in /dev.
 */
static int sd_create_device(struct sd_dev *dev, int minor)
{
        int err = 0;
        struct device *device = NULL;

        sd_dev_init(dev, minor);

        cdev_init(&dev->cdev, &sd_fops);
        err = cdev_add(&dev->cdev, dev->devno, 1);
        if (err < 0) {
                PR_WARNING(SD_ERR_DEVADD);
                return err;
        }

        device = device_create(sd_class, NULL, dev->devno, NULL, KBUILD_MODNAME "%d", minor);
        if (IS_ERR(device)) {
                err = PTR_ERR(device);
                PR_WARNING(SD_ERR_DEVCREATE);
                cdev_del(&dev->cdev);
                return err;
        }

        dev->name = dev_name(device);
        uart_config(dev);

        return 0;
}

static void sd_destroy_device(struct sd_dev *dev)
{
        if (dev != NULL) {
                device_destroy(sd_class, dev->devno);
                cdev_del(&dev->cdev);
        }
}

/* Destroy all */
static void sd_cleanup_driver(int ndev)
{
        unsigned int i;

        if (sd_dev) {
                for (i = 0; i < ndev; ++i) {
                        uart_clean(&sd_dev[i]);
                        sd_destroy_device(&sd_dev[i]);
                }
                kfree(sd_dev);
        }
        if (sd_class != NULL)
                class_destroy(sd_class);
        unregister_chrdev_region(MKDEV(sd_major, 0), sd_ndevs);
        remove_proc_entry(KBUILD_MODNAME, NULL);
}

/*
 * Create proc entry
 * Register char device (dynamic major).
 * Create device class.
 */
static int sd_create_driver(void)
{
        int err = 0;
        dev_t first;

       if (proc_create(KBUILD_MODNAME, 0, NULL, &sd_proc_fops) == NULL)
                return -ENOMEM;

        /* Start with minor 0, register n devices */
        err = alloc_chrdev_region(&first, 0, sd_ndevs, KBUILD_MODNAME);
        if (err < 0) {
                PR_WARNING(SD_ERR_MAJOR);
                return err;
        }

        sd_major = MAJOR(first);

        sd_class = class_create(THIS_MODULE, KBUILD_MODNAME);
        if (IS_ERR(sd_class)) {
                err = PTR_ERR(sd_class);
                return err;
        }
        return 0;
}

static int sd_register_devices(void)
{
        int err = 0;
        int i;

        if ((err = sd_create_driver())) {
                if (sd_major)
                        sd_cleanup_driver(0);
                return err;
        }
        if ((err = sd_alloc_devices(&sd_dev))) {
                sd_cleanup_driver(0);
                return err;
        }
        for (i = 0; i < sd_ndevs; ++i) {
                if ((err = sd_create_device(&sd_dev[i], i))) {
                        sd_cleanup_driver(i);
                        return err;
                }
                PR_INFO("%d:%d", MAJOR(sd_dev[i].devno), MINOR(sd_dev[i].devno));
        }
        return err;
}

static void sd_unregister_devices(void)
{
        sd_cleanup_driver(sd_ndevs);
}

/* ************************************************************* */

static int __init sd_init_module(void)
{
        PR_INFO(SD_INFO_LOAD);
        return sd_register_devices();
}

static void __exit sd_exit_module(void)
{
        PR_INFO(SD_INFO_UNLOAD);
        sd_unregister_devices();
}

module_init(sd_init_module);
module_exit(sd_exit_module);
