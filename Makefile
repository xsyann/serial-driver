###
## Makefile
##
## Made by xsyann
## Contact <contact@xsyann.com>
##
## Started on  Wed Apr  9 14:05:43 2014 xsyann
## Last update Wed Apr  9 14:05:47 2014 xsyann
##

TARGET	= serial-driver

obj-m	+= $(TARGET).o

$(TARGET)-objs := serial_driver.o

KDIR	= /lib/modules/$(shell uname -r)/build

all	:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean	:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
