#!/usr/bin/env bash
##
## run.sh
##
## Made by xsyann
## Contact <contact@xsyann.com>
##
## Started on  Thu Apr 10 20:28:06 2014 xsyann
## Last update Thu Apr 10 20:28:16 2014 xsyann
##

echo "   ┌───────────────────────────────────────────────────┐"
echo "-> │            $ insmod serial-driver.ko              │"
echo "   └───────────────────────────────────────────────────┘"
sudo insmod serial-driver.ko
echo "   ┌───────────────────────────────────────────────────┐"
echo "-> │      $ cat /proc/devices | grep serial-driver     │"
echo "   └───────────────────────────────────────────────────┘"
cat /proc/devices | grep "serial"
echo "   ┌───────────────────────────────────────────────────┐"
echo "-> │            $ ll /sys/class/serial-driver          │"
echo "   └───────────────────────────────────────────────────┘"
ls -l /sys/class/serial-driver/
echo "   ┌───────────────────────────────────────────────────┐"
echo "-> │ $ cat /sys/class/serial-driver/serial-driver/dev  │"
echo "   └───────────────────────────────────────────────────┘"
cat /sys/class/serial-driver/serial-driver/dev
echo "   ┌───────────────────────────────────────────────────┐"
echo "-> │           $ ls /dev | grep serial-driver          │"
echo "   └───────────────────────────────────────────────────┘"
ls -l /dev | grep "serial"
echo "   ┌───────────────────────────────────────────────────┐"
echo "-> │               # rmmod serial-driver               │"
echo "   └───────────────────────────────────────────────────┘"
sudo rmmod serial-driver
echo "   ┌───────────────────────────────────────────────────┐"
echo "-> │                      $ dmesg                      │"
echo "   └───────────────────────────────────────────────────┘"
dmesg | tail -n 3
