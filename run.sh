#!/usr/bin/env bash
## run.sh for serial-driver
##
## Made by xsyann
## Contact <contact@xsyann.com>
##
## Started on  Sat Apr  5 16:59:46 2014 xsyann
## Last update Sat Apr  5 17:00:00 2014 xsyann
##

sudo insmod serial-driver.ko
sudo rmmod serial-driver
dmesg | tail -n 10
