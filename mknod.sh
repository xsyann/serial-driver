#!/usr/bin/env bash
#
# mknod.sh
#
# Made by xsyann
# Contact <contact@xsyann.com>
#
# Started on  Wed Apr  9 16:46:21 2014 xsyann
# Last update Fri Apr 11 12:43:20 2014 xsyann
#

if [ $# -eq 0 ]; then
    module_name="serial-driver"
else
    module_name=$1
fi
sudo insmod ${module_name}.ko
major=`grep $module_name /proc/devices`
if [ "$major" != "" ]; then
    set $major
    sudo rm -i /dev/$module_name
    sudo mknod /dev/$2 c $1 0
    sudo rmmod ${module_name}.ko
else
    echo "Module $module_name not loaded"
    exit 1
fi
