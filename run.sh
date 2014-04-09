#!/usr/bin/env bash
#
# run.sh
#
# Made by xsyann
# Contact <contact@xsyann.com>
#
# Started on  Wed Apr  9 14:06:41 2014 xsyann
# Last update Wed Apr  9 17:33:02 2014 xsyann
#

echo "   ┌──────────────────────────────────────────────┐"
echo "-> │         $ insmod serial-driver.ko            │"
echo "   └──────────────────────────────────────────────┘"
sudo insmod serial-driver.ko
echo "   ┌──────────────────────────────────────────────┐"
echo "-> │         $ ls /dev | grep serial-driver       │"
echo "   └──────────────────────────────────────────────┘"
ls -l /dev | grep "serial"
echo ""
echo "   ┌──────────────────────────────────────────────┐"
echo "-> │   $ cat /proc/devices | grep serial-driver   │"
echo "   └──────────────────────────────────────────────┘"
cat /proc/devices | grep "serial"
echo "   ┌──────────────────────────────────────────────┐"
echo "-> │     $ cat /proc/misc | grep serial-driver    │"
echo "   └──────────────────────────────────────────────┘"
cat /proc/misc | grep "serial"
echo "   ┌──────────────────────────────────────────────┐"
echo "-> │   $ ll /sys/class/misc | grep serial-driver  │"
echo "   └──────────────────────────────────────────────┘"
ls -l /sys/class/misc | grep "serial"
echo "   ┌──────────────────────────────────────────────┐"
echo "-> │           # rmmod serial-driver              │"
echo "   └──────────────────────────────────────────────┘"
sudo rmmod serial-driver
echo "   ┌──────────────────────────────────────────────┐"
echo "-> │                    $ dmesg                   │"
echo "   └──────────────────────────────────────────────┘"
dmesg | tail -n 5
