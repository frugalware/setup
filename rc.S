#!/bin/sh

clear
mount -t proc none /proc
mount -t sysfs none /sys
mount -t ramfs none /tmp
cat /proc/mounts >/etc/mtab
modprobe iso9660
modprobe BusLogic 2>/dev/null
/etc/rc.d/rc.udev

# vim: ft=sh
