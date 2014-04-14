#!/usr/bin/env bash
##
## run.sh
##
## Made by xsyann
## Contact <contact@xsyann.com>
##
## Started on  Thu Apr 10 20:28:06 2014 xsyann
## Last update Mon Apr 14 20:45:04 2014 xsyann
##

module="serial_driver"

echo "   ┌───────────────────────────────────────────────────┐"
echo "-> │            $ insmod serial-driver.ko              │"
echo "   └───────────────────────────────────────────────────┘"
sudo insmod ${module}.ko
echo "   ┌───────────────────────────────────────────────────┐"
echo "-> │      $ cat /proc/devices | grep serial-driver     │"
echo "   └───────────────────────────────────────────────────┘"
cat /proc/devices | grep "serial"
echo "   ┌───────────────────────────────────────────────────┐"
echo "-> │            $ ll /sys/class/serial-driver          │"
echo "   └───────────────────────────────────────────────────┘"
ls -l /sys/class/${module}/
echo "   ┌───────────────────────────────────────────────────┐"
echo "-> │ $ cat /sys/class/serial-driver/serial-driver/dev  │"
echo "   └───────────────────────────────────────────────────┘"
cat /sys/class/${module}/${module}/dev
echo "   ┌───────────────────────────────────────────────────┐"
echo "-> │           $ ls /dev | grep serial-driver          │"
echo "   └───────────────────────────────────────────────────┘"
ls -l /dev | grep ${module}
echo "   ┌───────────────────────────────────────────────────┐"
echo "-> │               # rmmod serial-driver               │"
echo "   └───────────────────────────────────────────────────┘"
sudo rmmod ${module}.ko
echo "   ┌───────────────────────────────────────────────────┐"
echo "-> │                      $ dmesg                      │"
echo "   └───────────────────────────────────────────────────┘"
dmesg | tail -n 6
