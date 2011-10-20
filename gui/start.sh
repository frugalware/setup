#!/bin/sh

clear

echo
echo
echo -e "\033[1;7;34;47mWelcome to Fwife Installer.\033[0m"
echo
echo

echo " * Setting umask.."
umask 022
echo " * Mounting /proc.."
mount -t proc   none   /proc
echo " * Mounting metafilesystems.."
mount -t sysfs  none  /sys
mount -t tmpfs none /tmp
echo " * Initialising mtab.."
ln -sf /proc/self/mounts /etc/mtab
echo " * Load kernel modules.."
modprobe isofs
modprobe ext2
modprobe ext3
modprobe ext4
modprobe reiserfs
modprobe ntfs
modprobe BusLogic
modprobe -q ehci-hcd
modprobe -q ohci-hcd
modprobe -q uhci-hcd
 echo -e "\033[1;6;34m* Detecting hardware - This can take some time..\033[0m"
udevd --daemon
udevadm trigger --type=subsystems
udevadm trigger --type=devices
udevadm settle

## Ugly hack for gparted witch use hal when a new partition is created but we don't have hal here :).
## Creating all possible nodes do the trick.
if [ -e "/dev/sda" ]; then
	i=1
	for part in `seq 1 15`;
	do
		mknod -m 660 /dev/sda$i b 8 $part
		i=$(expr $i + 1)
	done
fi

if [ -e "/dev/sdb" ]; then
	i=1
	for part in `seq 17 31`;
	do
		mknod -m 660 /dev/sdb$i b 8 $part
		i=$(expr $i + 1)
	done
fi

if [ -e "/dev/sdc" ]; then
	i=1
	for part in `seq 33 47`;
	do
		mknod -m 660 /dev/sdc$i b 8 $part
		i=$(expr $i + 1)
	done
fi

if [ -e "/dev/sdd" ]; then
	i=1
	for part in `seq 49 63`;
	do
		mknod -m 660 /dev/sdd$i b 8 $part
		i=$(expr $i + 1)
	done
fi

if [ -e "/dev/sde" ]; then
	i=1
	for part in `seq 65 79`;
	do
		mknod -m 660 /dev/sde$i b 8 $part
		i=$(expr $i + 1)
	done
fi

if [ -e "/dev/sdf" ]; then
	i=1
	for part in `seq 81 95`;
	do
		mknod -m 660 /dev/sdf$i b 8 $part
		i=$(expr $i + 1)
	done
fi

if [ -e "/dev/sdg" ]; then
	i=1
	for part in `seq 97 111`;
	do
		mknod -m 660 /dev/sdg$i b 8 $part
		i=$(expr $i + 1)
	done
fi

if [ -e "/dev/sdh" ]; then
	i=1
	for part in `seq 113 127`;
	do
		mknod -m 660 /dev/sdh$i b 8 $part
		i=$(expr $i + 1)
	done
fi

if [ -e "/dev/hda" ]; then
	i=1
	for part in `seq 1 63`;
	do
		mknod -m 660 /dev/hda$i b 3 $part
		i=$(expr $i + 1)
	done
fi

if [ -e "/dev/hdb" ]; then
	i=1
	for part in `seq 65 127`;
	do
		mknod -m 660 /dev/hdb$i b 3 $part
		i=$(expr $i + 1)
	done
fi

if [ -e "/dev/hdc" ]; then
	i=1
	for part in `seq 1 63`;
	do
		mknod -m 660 /dev/hdc$i b 22 $part
		i=$(expr $i + 1)
	done
fi

if [ -e "/dev/hdd" ]; then
	i=1
	for part in `seq 65 127`;
	do
		mknod -m 660 /dev/hdd$i b 22 $part
		i=$(expr $i + 1)
	done
fi

if [ -e "/dev/hde" ]; then
	i=1
	for part in `seq 1 63`;
	do
		mknod -m 660 /dev/hde$i b 33 $part
		i=$(expr $i + 1)
	done
fi

if [ -e "/dev/hdf" ]; then
	i=1
	for part in `seq 65 127`;
	do
		mknod -m 660 /dev/hdf$i b 33 $part
		i=$(expr $i + 1)
	done
fi

if [ -e "/dev/hdg" ]; then
	i=1
	for part in `seq 1 63`;
	do
		mknod -m 660 /dev/hdg$i b 34 $part
		i=$(expr $i + 1)
	done
fi

if [ -e "/dev/hdh" ]; then
	i=1
	for part in `seq 65 127`;
	do
		mknod  -m 660 /dev/hdh$i b 34 $part
		i=$(expr $i + 1)
	done
fi

export DISPLAY=:0
MOUSE=/dev/input/mice

## launch dhcpcd, if an interface is finded it'll be detected by fwife, so the user don't need to configure it
echo " * Starting dhcp.."
mkdir -p /var/run/dhcpcd/resolv.conf/
listiface=`ls /sys/class/net`
for i in $listiface; do
	ifconfig $i up
	udhcpc -n -t 2 -i $i
	if [[ "$?" == "0" ]]; then
		# used by fwife to find working dhcp connection
		touch /var/run/dhcpcd/resolv.conf/$i
		break
	fi
done

echo " * Parsing kernel cmdline for Fwife options.."

for opt in `cat /proc/cmdline`
do
	case $opt in
		kmap=*)
			# Check for a specified keymap (kmap=*).
			echo " * Setting system keymap to: $KEYMAP.."
			loadkeys ${opt#kmap=}
			;;
		text)
			SCREEN="text" ;;
		mouse=*)
			MOUSE=${opt#mouse=} ;;
		*)
			continue ;;
	esac
done


#echo " * Create link for the mouse.."
#ln -fs $MOUSE /dev/mouse

if [[ "$SCREEN" == "text" ]]; then
	echo " * Trying to start a shell.."
	exec /bin/sh
else
	echo " * Trying to start X server.."
	startx.sh || exec /bin/sh
fi
