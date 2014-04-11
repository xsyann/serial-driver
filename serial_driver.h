/*
** serial_driver.h
**
** Made by xsyann
** Contact <contact@xsyann.com>
**
** Started on  Fri Apr 11 12:14:27 2014 xsyann
** Last update Fri Apr 11 15:24:26 2014 xsyann
*/

#ifndef         __SERIAL_DRIVER_H__
#define         __SERIAL_DRIVER_H__

#define SD_AUTHOR "Nicolas de Thore, Yann KOETH"
#define SD_DESC "Simple serial driver for linux kernel v3.11.0.19"
#define SD_VERSION "0.1"

#define SD_DEVICE_NAME "serial-driver"

#define SD_ERR_MAJOR "Unable to get a major"
#define SD_ERR_DEVADD "Unable to add device"
#define SD_ERR_DEVCREATE "Unable to create device"
#define SD_ERR_NOTFOUND "Device %d:%d not found"
#define SD_ERR_OUTOFMEM "Out of memory"

#define SD_INFO_LOAD "Loaded"
#define SD_INFO_UNLOAD "Unloaded"

#define SD_BUFFER_SIZE 512

struct sd_dev
{
        struct cdev cdev;       /* Character device struct (/dev) */
        dev_t devno;            /* Device number (major & minor) */
        unsigned char *data;    /* Buffer */
        struct semaphore sem;   /* Semaphore */
};

#endif          /* __SERIAL_DRIVER_H__ */
