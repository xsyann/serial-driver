/*
** serial_driver.c for serial-driver
**
** Made by xsyann
** Contact <contact@xsyann.com>
**
** Started on  Sat Apr  5 16:50:32 2014 xsyann
** Last update Sat Apr  5 16:57:34 2014 xsyann
*/

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nicolas de Thore");
MODULE_DESCRIPTION("Simple serial driver for linux kernel v3.11.0.19");
MODULE_VERSION("0.1");

static int	init_serial_driver(void)
{
	printk(KERN_INFO "Serial driver loaded.\n");
	return 0;
}

static void	cleanup_serial_driver(void)
{
	printk(KERN_INFO "Serial driver cleaned up.\n");
}

module_init(init_serial_driver);
module_exit(cleanup_serial_driver);
