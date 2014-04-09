/*
** serial_driver.c
**
** Made by xsyann
** Contact <contact@xsyann.com>
**
** Started on  Wed Apr  9 14:07:16 2014 xsyann
** Last update Wed Apr  9 17:37:16 2014 xsyann
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>

#define NAME "serial-driver"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas de Thore, Yann KOETH");
MODULE_DESCRIPTION("Simple serial driver for linux kernel v3.11.0.19");
MODULE_VERSION("0.1");

static int major = 0; /* Major */
static struct miscdevice md; /* Misc device handler */

/*
  module_param(major, int, 0644);
  MODULE_PARM_DESC(major, "Major number");
*/

static int serial_driver_open(struct inode *inode, struct file *file)
{
        printk(KERN_INFO "%s: open()\n", NAME);
        return 0;
}

static int serial_driver_release(struct inode *inode, struct file *file)
{
        printk(KERN_INFO "%s: release()\n", NAME);
        return 0;
}

static ssize_t serial_driver_read(struct file *file, char __user *buf,
                              size_t count, loff_t *ppos)
{
        printk(KERN_INFO "%s: read()\n", NAME);
        return count;
}

static ssize_t serial_driver_write(struct file *file, const char __user *buf,
                              size_t count, loff_t *ppos)
{
        printk(KERN_INFO "%s: write()\n", NAME);
        return count;
}

static struct file_operations serial_driver_fops = {
        .owner = THIS_MODULE,
        .read = serial_driver_read,
        .write = serial_driver_write,
        .open = serial_driver_open,
        .release = serial_driver_release,
};

/* Alloc a minor of misc and create the node in /dev.
 *
 * misc_register() calls device_create().
 * device_create() creates a device and registers it with sysfs.
 * The entry created in /sys/class/misc allow the udev deamon
 * to create the node.
 */
static int alloc_misc_minor(void)
{
        int ret;

        md.minor = MISC_DYNAMIC_MINOR;
        md.name = NAME;
        md.fops = &serial_driver_fops;
        ret = misc_register(&md);
        if (ret < 0)
                printk(KERN_WARNING "%s: Unable to get a minor.\n", NAME);
        else
                printk(KERN_INFO "%s: Misc minor allocated.\n", NAME);
        return ret;
}

/* Alloc a major. Need manual mknod. */
static int alloc_major(void)
{
        int ret;
        ret = register_chrdev(major, NAME, &serial_driver_fops);
        if (ret < 0)
                printk(KERN_WARNING "%s: Unable to get a major.\n", NAME);
        else
                printk(KERN_INFO "%s: Major %d allocated.\n", NAME, ret);
                printk(KERN_INFO "%s: Do not forget to mknod /dev/%s c %d 0.\n",
                       NAME, NAME, ret);
        return ret;
}

/* Try to allocate a misc minor (to auto manage /dev/node).
 * On failure, try to allocate a new major.
 */
static int __init init_serial_driver(void)
{
        int ret;

        printk(KERN_INFO "%s: Loaded\n", NAME);

        ret = alloc_misc_minor();
        if (ret < 0)
        {
                major = alloc_major();
                if (major < 0)
                        return major;
        }
        return 0;
}

static void __exit cleanup_serial_driver(void)
{
        printk(KERN_INFO "%s: Unloaded\n", NAME);
        if (major == 0)
        {
                if (misc_deregister(&md) < 0)
                {
                        printk(KERN_WARNING "%s: Unable to unregister.\n", NAME);
                        return;
                }
        }
        else
                unregister_chrdev(major, NAME);
}

module_init(init_serial_driver);
module_exit(cleanup_serial_driver);
