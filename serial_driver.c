/*
** serial_driver.c
**
** Made by xsyann
** Contact <contact@xsyann.com>
**
** Started on  Wed Apr  9 14:07:16 2014 xsyann
** Last update Thu Apr 10 18:45:22 2014 xsyann
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas de Thore, Yann KOETH");
MODULE_DESCRIPTION("Simple serial driver for linux kernel v3.11.0.19");
MODULE_VERSION("0.1");

#define SD_DEVICE_NAME "serial-driver"

#define SD_ERR_MAJOR "unable to get a major"
#define SD_ERR_DEVADD "unable to add device"
#define SD_ERR_DEVCREATE "unable to create device"

#define SD_INFO_LOAD "loaded"
#define SD_INFO_UNLOAD "unloaded"


static struct class *sd_class;  /* Device class (/sys/class) */
static struct cdev sd_cdev;     /* Character device struct (/dev) */
static dev_t sd_dev;            /* Device number (major & minor) */


static int sd_open(struct inode *inode, struct file *filp)
{
        printk(KERN_INFO "%s: open()\n", SD_DEVICE_NAME);
        return 0;
}

static int sd_release(struct inode *inode, struct file *filp)
{
        printk(KERN_INFO "%s: release()\n", SD_DEVICE_NAME);
        return 0;
}

static ssize_t sd_read(struct file *filp, char __user *buf,
                              size_t count, loff_t *ppos)
{
        printk(KERN_INFO "%s: read()\n", SD_DEVICE_NAME);
        return count;
}

static ssize_t sd_write(struct file *filp, const char __user *buf,
                              size_t count, loff_t *ppos)
{
        printk(KERN_INFO "%s: write()\n", SD_DEVICE_NAME);
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

static int sd_create_device(void)
{
        int err;
        struct device *dev = NULL;

        cdev_init(&sd_cdev, &sd_fops);
        err = cdev_add(&sd_cdev, sd_dev, 1);
        if (err) {
                printk(KERN_WARNING "%s: error %d: %s\n",
                       SD_DEVICE_NAME, err, SD_ERR_DEVADD);
                return err;
        }
        dev = device_create(sd_class, NULL, sd_dev, NULL, SD_DEVICE_NAME);
        if (IS_ERR(dev)) {
                err = PTR_ERR(dev);
                printk(KERN_WARNING "%s: error %d: %s\n",
                       SD_DEVICE_NAME, err, SD_ERR_DEVCREATE);
                cdev_del(&sd_cdev);
                return err;
        }
        return 0;
}

static void sd_destroy_device(void)
{
        device_destroy(sd_class, sd_dev);
        cdev_del(&sd_cdev);
}

static void sd_cleanup(void)
{
        if (sd_class)
                class_destroy(sd_class);
        unregister_chrdev_region(sd_dev, 1);
}

static int sd_setup(void)
{
        int err = alloc_chrdev_region(&sd_dev, 0, 1, SD_DEVICE_NAME);
        if (err < 0) {
                printk(KERN_WARNING "%s: %s\n", SD_DEVICE_NAME, SD_ERR_MAJOR);
                return err;
        }
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
        int err;

        printk(KERN_INFO "%s: %s\n", SD_DEVICE_NAME, SD_INFO_LOAD);
        if ((err = sd_setup())) {
                sd_cleanup();
                return err;
        }
        if ((err = sd_create_device())) {
                sd_cleanup();
                return err;
        }
        printk(KERN_INFO "%s: %d:%d\n",
               SD_DEVICE_NAME, MAJOR(sd_dev), MINOR(sd_dev));
        return 0;
}

static void __exit sd_exit_module(void)
{
        printk(KERN_INFO "%s: %s\n", SD_DEVICE_NAME, SD_INFO_UNLOAD);
        sd_destroy_device();
        sd_cleanup();
}

module_init(sd_init_module);
module_exit(sd_exit_module);
