#!/bin/sh

clear
/etc/rc.d/rc.udev
mount -t ramfs none /tmp
cat /proc/mounts >/etc/mtab
modprobe iso9660
modprobe BusLogic 2>/dev/tty2

# vim: ft=sh
