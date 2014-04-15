#!/usr/bin/env bash
##
## run.sh
##
## Made by xsyann
## Contact <contact@xsyann.com>
##
## Started on  Thu Apr 10 20:28:06 2014 xsyann
## Last update Tue Apr 15 20:40:24 2014 xsyann
##

module="serial_driver"

echo "   ┌───────────────────────────────────────────────────┐"
echo "-> │            $ insmod serial_driver.ko              │"
echo "   └───────────────────────────────────────────────────┘"
sudo insmod ${module}.ko
echo "   ┌───────────────────────────────────────────────────┐"
echo "-> │      $ cat /proc/devices | grep serial_driver     │"
echo "   └───────────────────────────────────────────────────┘"
cat /proc/devices | grep "serial"
echo "   ┌───────────────────────────────────────────────────┐"
echo "-> │            $ ll /sys/class/serial_driver          │"
echo "   └───────────────────────────────────────────────────┘"
ls -l /sys/class/${module}/
echo "   ┌───────────────────────────────────────────────────┐"
echo "-> │ $ cat /sys/class/serial_driver/serial_driver*/dev │"
echo "   └───────────────────────────────────────────────────┘"
cat /sys/class/${module}/${module}*/dev
echo "   ┌───────────────────────────────────────────────────┐"
echo "-> │           $ ls /dev | grep serial_driver          │"
echo "   └───────────────────────────────────────────────────┘"
ls -l /dev | grep ${module}
echo "   ┌───────────────────────────────────────────────────┐"
echo "-> │               # rmmod serial_driver               │"
echo "   └───────────────────────────────────────────────────┘"
sudo rmmod ${module}.ko
echo "   ┌───────────────────────────────────────────────────┐"
echo "-> │                      $ dmesg                      │"
echo "   └───────────────────────────────────────────────────┘"
dmesg | tail -n 6
