#!/bin/bash

clear
/etc/rc.d/rc.udev
mount -t ramfs none /tmp
cat /proc/mounts >/etc/mtab
modprobe iso9660
# lang&keyb

# vim: ft=sh
