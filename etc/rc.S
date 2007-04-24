#!/bin/sh

clear
mount -t proc none /proc
mount -t sysfs none /sys
mount -t tmpfs none /tmp
cat /proc/mounts >/etc/mtab
modprobe iso9660
modprobe ntfs
modprobe BusLogic 2>/dev/null

# try to load all the hub modules:
modprobe -q ehci-hcd >/dev/null 2>&1
modprobe -q ohci-hcd >/dev/null 2>&1
modprobe -q uhci-hcd >/dev/null 2>&1
				
/etc/rc.d/rc.udev
setterm -blank 0

# vim: ft=sh
