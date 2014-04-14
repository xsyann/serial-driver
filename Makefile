##
## Makefile
##
## Made by xsyann
## Contact <contact@xsyann.com>
##
## Started on  Wed Apr  9 14:05:43 2014 xsyann
## Last update Mon Apr 14 20:45:54 2014 xsyann
##

TARGET	= serial_driver

obj-m	+= $(TARGET).o

$(TARGET)-objs := src/serial_driver.o

KDIR	= /lib/modules/$(shell uname -r)/build

all	:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean	:
	$(MAKE) -C $(KDIR) M=$(PWD) clean

load	:
	sudo insmod ./$(TARGET).ko

unload	:
	sudo rmmod ./$(TARGET).ko
