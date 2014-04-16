#!/usr/bin/env bash
##
## run.sh
##
## Made by xsyann
## Contact <contact@xsyann.com>
##
## Started on  Thu Apr 10 20:28:06 2014 xsyann
## Last update Wed Apr 16 11:52:14 2014 xsyann
##

module="serial_driver"

sudo dmesg -c > /dev/null

echo "   ┌───────────────────────────────────────────────────┐"
echo "-> │            $ insmod serial_driver.ko              │"
echo "   └───────────────────────────────────────────────────┘"
make load
echo "   ┌───────────────────────────────────────────────────┐"
echo "-> │      $ cat /proc/devices | grep serial_driver     │"
echo "   └───────────────────────────────────────────────────┘"
cat /proc/devices | grep ${module}
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
echo "-> │           $ cat /proc/serial_driver               │"
echo "   └───────────────────────────────────────────────────┘"
cat /proc/${module}
echo "   ┌───────────────────────────────────────────────────┐"
echo "-> │               # rmmod serial_driver               │"
echo "   └───────────────────────────────────────────────────┘"
sudo rmmod ${module}.ko
echo "   ┌───────────────────────────────────────────────────┐"
echo "-> │                      $ dmesg                      │"
echo "   └───────────────────────────────────────────────────┘"
dmesg
