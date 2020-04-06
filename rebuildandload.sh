#!/bin/bash
make clean
make
rmmod chardev
rm /dev/UNGS
insmod chardev.ko
mknod /dev/UNGS c 244 0
chmod 666 /dev/UNGS
