##
## Makefile
##
## Made by xsyann
## Contact <contact@xsyann.com>
##
## Started on  Wed Apr  9 14:05:43 2014 xsyann
## Last update Wed Apr 16 11:37:10 2014 xsyann
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
	@sudo rmmod ./$(TARGET).ko 2> /dev/null; sudo insmod ./$(TARGET).ko
	@echo "Load $(TARGET).ko"

unload	:
	@sudo rmmod ./$(TARGET).ko
	@echo "Unload $(TARGET).ko"
