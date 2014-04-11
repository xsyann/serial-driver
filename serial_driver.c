/*
** serial_driver.c
**
** Made by xsyann
** Contact <contact@xsyann.com>
**
** Started on  Wed Apr  9 14:07:16 2014 xsyann
** Last update Fri Apr 11 16:31:29 2014 xsyann
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <linux/uaccess.h>

#include "serial_driver.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR(SD_AUTHOR);
MODULE_DESCRIPTION(SD_DESC);
MODULE_VERSION(SD_VERSION);

static struct class *sd_class = NULL;   /* Device class (/sys/class) */
static struct sd_dev *sd_dev = NULL;    /* Device wrapper */

/* Data buffer dynamic allocation
 * Called at first opening
 */
static int sd_alloc_data(struct sd_dev *dev)
{
        if (dev->data == NULL)
        {
                dev->data = kzalloc(SD_BUFFER_SIZE, GFP_KERNEL);
                if (sd_dev->data == NULL)
                {
                        printk(KERN_WARNING "%s: open: %s",
                               SD_DEVICE_NAME, SD_ERR_OUTOFMEM);
                        return -ENOMEM;
                }
        }
        return 0;
}

static int sd_open(struct inode *inode, struct file *filp)
{
        int err = 0;
        unsigned int major = 0;
        unsigned int minor = 0;

        major = imajor(inode);
        minor = iminor(inode);
        /* Check minor and major */
        if (MKDEV(major, minor) != sd_dev->devno ||
            inode->i_cdev != &sd_dev->cdev)
        {
                printk(KERN_WARNING "%s: " SD_ERR_NOTFOUND "\n",
                       SD_DEVICE_NAME, major, minor);
                return -ENODEV;
        }
        /* Alloc data if data is NULL */
        if (sd_dev->data == NULL)
                if ((err = sd_alloc_data(sd_dev)) < 0)
                        return err;
        /* Lock */
        if (down_interruptible(&sd_dev->sem) != 0)
                return -EINTR;
        printk(KERN_INFO "%s: open()\n", SD_DEVICE_NAME);
        return 0;
}

static int sd_release(struct inode *inode, struct file *filp)
{
        /* Unlock */
        up(&sd_dev->sem);
        printk(KERN_INFO "%s: release()\n", SD_DEVICE_NAME);
        return 0;
}

static ssize_t sd_read(struct file *filp, char __user *buf,
                              size_t count, loff_t *ppos)
{
        printk(KERN_INFO "%s: read()\n", SD_DEVICE_NAME);

        if (*ppos >= SD_BUFFER_SIZE) /* End of buffer */
                return 0;
        if (*ppos + count > SD_BUFFER_SIZE) /* Partial read to the end */
                count = SD_BUFFER_SIZE - *ppos;


        if (copy_to_user(buf, &sd_dev->data[*ppos], count) != 0)
                return -EFAULT;
        *ppos += count;
        return count;
}

static ssize_t sd_write(struct file *filp, const char __user *buf,
                              size_t count, loff_t *ppos)
{
        printk(KERN_INFO "%s: write()\n", SD_DEVICE_NAME);

        if (*ppos >= SD_BUFFER_SIZE) /* Out of buffer */
                return -EINVAL;
        if (*ppos + count > SD_BUFFER_SIZE) /* Partial write to the end */
                count = SD_BUFFER_SIZE - *ppos;

        if (copy_from_user(&sd_dev->data[*ppos], buf, count) != 0)
                return -EFAULT;
        *ppos += count;
        return count;
}

static struct file_operations sd_fops = {
        .owner = THIS_MODULE,
        .read = sd_read,
        .write = sd_write,
        .open = sd_open,
        .release = sd_release,
};

/* ************************************************************* */

static int sd_alloc_device(struct sd_dev **dev)
{
        *dev = kzalloc(sizeof(**dev), GFP_KERNEL);
        if (*dev == NULL) {
                return -ENOMEM;
        }
        return 0;
}

static void sd_free_device(struct sd_dev *dev)
{
        if (dev)
                kfree(dev);
}

/* Add char device to system
 * Call device_create() to auto create (udev daemon)
 * the node in /dev.
 */
static int sd_create_device(struct sd_dev *dev, int major, int minor,
                            struct class *class)
{
        int err = 0;
        struct device *device = NULL;

        dev->data = NULL;
        dev->devno = MKDEV(major, minor);
        sema_init(&dev->sem, 1);

        cdev_init(&dev->cdev, &sd_fops);
        err = cdev_add(&dev->cdev, dev->devno, 1);
        if (err < 0) {
                printk(KERN_WARNING "%s: error %d: %s\n",
                       SD_DEVICE_NAME, err, SD_ERR_DEVADD);
                return err;
        }

        device = device_create(class, NULL, dev->devno, NULL, SD_DEVICE_NAME);
        if (IS_ERR(device)) {
                err = PTR_ERR(device);
                printk(KERN_WARNING "%s: error %d: %s\n",
                       SD_DEVICE_NAME, err, SD_ERR_DEVCREATE);
                cdev_del(&dev->cdev);
                return err;
        }
        return 0;
}

static void sd_destroy_device(struct sd_dev *dev)
{
        if (dev != NULL)
        {
                device_destroy(sd_class, dev->devno);
                cdev_del(&dev->cdev);
                if (dev->data != NULL)
                        kfree(dev->data);
        }
}

static void sd_cleanup(void)
{
        if (sd_class != NULL)
                class_destroy(sd_class);
        unregister_chrdev_region(sd_dev->devno, 1);
}

/* Register char device (dynamic major).
 * Create device class.
 * The *major parameter is updated with major number.
 */
static int sd_setup(int *major)
{
        dev_t first;

        /* Start with minor 0, register 1 device */
        int err = alloc_chrdev_region(&first, 0, 1, SD_DEVICE_NAME);
        if (err < 0) {
                printk(KERN_WARNING "%s: %s\n", SD_DEVICE_NAME, SD_ERR_MAJOR);
                return err;
        }

        *major = MAJOR(first);

        sd_class = class_create(THIS_MODULE, SD_DEVICE_NAME);
        if (IS_ERR(sd_class)) {
                err = PTR_ERR(sd_class);
                sd_cleanup();
                return err;
        }
        return 0;
}

/* ************************************************************* */

static int __init sd_init_module(void)
{
        int err = 0;
        int major = 0;

        printk(KERN_INFO "%s: %s\n", SD_DEVICE_NAME, SD_INFO_LOAD);
        if ((err = sd_setup(&major))) {
                return err;
        }
        if ((err = sd_alloc_device(&sd_dev))) {
                sd_cleanup();
                return err;
        }
        if ((err = sd_create_device(sd_dev, major, 0, sd_class))) {
                sd_free_device(sd_dev);
                sd_cleanup();
                return err;
        }
        printk(KERN_INFO "%s: %d:%d\n",
               SD_DEVICE_NAME, MAJOR(sd_dev->devno), MINOR(sd_dev->devno));
        return 0;
}

static void __exit sd_exit_module(void)
{
        printk(KERN_INFO "%s: %s\n", SD_DEVICE_NAME, SD_INFO_UNLOAD);
        sd_destroy_device(sd_dev);
        sd_free_device(sd_dev);
        sd_cleanup();
}

module_init(sd_init_module);
module_exit(sd_exit_module);
