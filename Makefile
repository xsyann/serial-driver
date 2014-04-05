##
## Makefile for serial-driver
##
## Made by xsyann
## Contact <contact@xsyann.com>
##
## Started on  Sat Apr  5 16:29:28 2014 xsyann
## Last update Sat Apr  5 16:36:18 2014 xsyann
##

TARGET	= serial-driver

obj-m	+= $(TARGET).o

$(TARGET)-objs := serial_driver.o

KDIR	= /lib/modules/$(shell uname -r)/build

all	:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean	:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
