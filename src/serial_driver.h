/*
** serial_driver.h
**
** Made by xsyann
** Contact <contact@xsyann.com>
**
** Started on  Fri Apr 11 12:14:27 2014 xsyann
** Last update Tue Apr 15 16:19:10 2014 xsyann
*/

#ifndef         __SERIAL_DRIVER_H__
#define         __SERIAL_DRIVER_H__

#define SD_AUTHOR "Nicolas de Thore, Yann KOETH"
#define SD_DESC "Simple serial driver for linux kernel v3.11.0.19"
#define SD_VERSION "0.2"

#define SD_ERR_MAJOR "Unable to get a major"
#define SD_ERR_DEVADD "Unable to add device"
#define SD_ERR_DEVCREATE "Unable to create device"
#define SD_ERR_NOTFOUND "Device %d:%d not found"
#define SD_ERR_OUTOFMEM "Out of memory"
#define SD_ERR_UARTFAIL "UART configuration failed"

#define SD_INFO_LOAD "Loaded"
#define SD_INFO_UNLOAD "Unloaded"

#define PR_WARNING(ERR, ...) \
        (printk(KERN_WARNING "%s: error: " ERR "\n", KBUILD_MODNAME, ##__VA_ARGS__))

#define PR_INFO(INFO, ...) \
        (printk(KERN_WARNING "%s: " INFO "\n", KBUILD_MODNAME, ##__VA_ARGS__))

#define SD_NCOM 4

struct sd_com_info
{
        unsigned int id;
        int uart_port;
        int irq;
};

struct sd_dev
{
        struct cdev cdev;       /* Character device struct (/dev) */
        dev_t devno;            /* Device number (major & minor) */
        const char *name;       /* Device name */
        struct sd_com_info com; /* UART port & irq number */
        struct mutex read_mutex;
        struct mutex write_mutex;
        wait_queue_head_t read_wq;
        wait_queue_head_t write_wq;
        unsigned long tx;
        unsigned long rx;
};

#endif          /* __SERIAL_DRIVER_H__ */
