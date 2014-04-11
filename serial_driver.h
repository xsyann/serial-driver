/*
** serial_driver.h
**
** Made by xsyann
** Contact <contact@xsyann.com>
**
** Started on  Fri Apr 11 12:14:27 2014 xsyann
** Last update Fri Apr 11 12:20:05 2014 xsyann
*/

#ifndef         __SERIAL_DRIVER_H__
#define         __SERIAL_DRIVER_H__

struct sd_dev
{
        struct cdev cdev;       /* Character device struct (/dev) */
        dev_t devno;            /* Device number (major & minor) */
        unsigned char *data;    /* Buffer */
        struct semaphore sem;   /* Semaphore */
};

#endif          /* __SERIAL_DRIVER_H__ */
