#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas de Thore");
MODULE_DESCRIPTION("Simple serial driver for linux kernel v3.11.0.19");

void		cleanup_serial_driver(void)
{
  printk(KERN_INFO "Serial driver cleaned up.");
}

void		init_serial_driver(void)
{
  printk(KERN_INFO "Serial driver loaded.");
}

module_init(init_serial_driver);
module_exit(cleanup_serial_driver);
