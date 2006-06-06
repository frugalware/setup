# Makefile for Frugalware Linux Setup
#
# Compiling Time: 2.45 SBU
#
# Copyright (C) 2005-2006 Miklos Vajna <vmiklos@frugalware.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
#

-include config.mak

STABLE = false
KERNELV = $(shell echo $(KERNELVER)|sed 's/-.*//')
KERNELREL = $(shell echo $(KERNELVER)|sed 's/.*-//')

CLEANUP = rm -rf $(BDIR) && mkdir $(BDIR) && rm -rf $@
export PATH := /usr/lib/ccache/bin:$(PATH)
export CCACHE_DIR=/var/cache/ccache/setup
export CCACHE_NOLINK=1
export CCACHE_UMASK=002
GLIBC_LANGS = en_US es_AR de_DE fr_FR id_ID it_IT hu_HU nl_NL pl_PL pt_PT sk_SK pt_BR
CARCH ?= $(shell arch)
ifeq ($(CARCH),i686)
	KARCH ?= i386
endif
ifeq ($(CARCH),x86_64)
	MARCH ?= k8
endif
MARCH ?= $(CARCH)
KARCH ?= $(CARCH)
export CFLAGS = -march=$(MARCH) -O2 -pipe

CDIR = cache
CONFDIR = config
BDIR = build
MDIR = merge
CWD=`pwd`

packages = bash busybox dialog e2fsprogs reiserfsprogs lynx dhcpcd frugalware \
	   glibc kbd kernel module-init-tools ncurses pacman eject \
	   udev util-linux netkit-base mdadm xfsprogs ppp pppoe glib2 parted \
	   bzip2 libarchive zlib frugalwareutils wireless_tools ipw2200-firmware
fonts = lat1-16.psfu.gz lat2-16.psfu.gz lat9w-16.psfu.gz
sources = bash-$(BASHVER)-$(CARCH).fpm busybox-$(BUSYBOXVER)-$(CARCH).fpm \
	  dhcpcd-$(DHCPCDVER)-$(CARCH).fpm dialog-$(DIALOGVER)-$(CARCH).fpm \
	  e2fsprogs-$(E2FSPROGSVER)-$(CARCH).fpm eject-$(EJECTVER)-$(CARCH).fpm \
	  frugalware-$(FRUGALWAREVER)-$(CARCH).fpm \
	  glibc-$(GLIBCVER)-$(CARCH).fpm kbd-$(KBDVER)-$(CARCH).fpm \
	  kernel-$(KERNELV)-$(KERNELREL)-$(CARCH).fpm \
	  lynx-$(LYNXVER)-$(CARCH).fpm \
	  module-init-tools-$(MODULE-INIT-TOOLSVER)-$(CARCH).fpm \
	  ncurses-$(NCURSESVER)-$(CARCH).fpm netkit-base-$(NETKIT-BASEVER)-$(CARCH).fpm \
	  pacman-$(PACMANVER)-$(CARCH).fpm \
	  reiserfsprogs-$(REISERFSPROGSVER)-$(CARCH).fpm udev-$(UDEVVER)-$(CARCH).fpm \
	  util-linux-$(UTIL-LINUXVER)-$(CARCH).fpm \
	  netkit-base-$(NETKIT-BASEVER)-$(CARCH).fpm \
	  mdadm-$(MDADMVER)-$(CARCH).fpm \
	  xfsprogs-$(XFSPROGSVER)-$(CARCH).fpm \
	  ppp-$(PPPVER)-$(CARCH).fpm \
	  rp-pppoe-$(RP-PPPOEVER)-$(CARCH).fpm \
	  glib2-$(GLIB2VER)-$(CARCH).fpm \
	  parted-$(PARTEDVER)-$(CARCH).fpm \
	  bzip2-$(BZIP2VER)-$(CARCH).fpm \
	  libarchive-$(LIBARCHIVEVER)-$(CARCH).fpm \
	  zlib-$(ZLIBVER)-$(CARCH).fpm \
	  frugalwareutils-$(FRUGALWAREUTILSVER)-$(CARCH).fpm \
	  wireless_tools-$(WIRELESS_TOOLSVER)-$(CARCH).fpm \
	  ipw2200-firmware-$(IPW2200-FIRMWAREVER)-$(CARCH).fpm

all: initrd

compile: check ccache setup $(packages) misc

clean:
	rm -rf $(BDIR) $(MDIR) initrd-$(CARCH).img.gz dl.lst config.mak
	rm -rf $(packages) vmlinuz-$(KERNELV)-fw$(KERNELREL)-$(CARCH) $(CDIR)/*
	$(MAKE) -C src clean

ccache:
	install -d -m 2775 /var/cache/ccache/setup

setup:
ifeq ($(STABLE),false)
	$(MAKE) -C src final
else
	$(MAKE) -C src stable
endif

merge: $(packages)
	rm -rf $(MDIR)
	mkdir $(MDIR)
	for i in $(packages); do \
		cp -a $$i/* $(MDIR)/; \
	done
	make -C po mos

misc: merge
	mkdir $(MDIR)/lib/plugins
	cp src/setup $(MDIR)/bin/
	cp src/plugins/*.so $(MDIR)/lib/plugins/
	cp etc/inittab $(MDIR)/etc/
	cp etc/rc.S $(MDIR)/etc/rc.d/
	chmod +x $(MDIR)/etc/rc.d/rc.S

devices: compile
	mknod -m 700 $(MDIR)/dev/console c 5 1
	mknod -m 600 $(MDIR)/dev/null c 1 3
	mknod -m 700 $(MDIR)/dev/tty c 5 0
	mknod -m 700 $(MDIR)/dev/tty1 c 4 1
	mknod -m 700 $(MDIR)/dev/tty2 c 4 2
	mknod -m 700 $(MDIR)/dev/tty3 c 4 3

initrd: config.mak devices
	dd if=/dev/zero of=initrd-$(CARCH).img bs=1k count=$$(echo "$$(`which du` -s $(MDIR)|sed 's/^\(.*\)\t.*$$/\1/')+500"|bc)
	/sbin/mke2fs -F initrd-$(CARCH).img
	mkdir i
	grep -q loop /proc/modules || /sbin/modprobe loop
	mount -o loop -t ext2 initrd-$(CARCH).img i
	cp -a $(MDIR)/* i/
	chown -R root.root i/
	umount initrd-$(CARCH).img
	rmdir i
	gzip -9 initrd-$(CARCH).img

update:
	darcs pull -a -v
	$(MAKE) -C src clean
	$(MAKE) -C src final
	sudo rm -rf merge initrd*
	sudo $(MAKE) initrd

upload:
	scp initrd-$(CARCH).img.gz frugalware.org:/home/ftp/pub/frugalware/frugalware-current/boot/

config.mak:
	sh configure

check: dl.lst
	cd $(CDIR) && sh ../bin/download ../dl.lst
	@for i in $(sources); do \
		ls $(CDIR)/$$i >/dev/null || exit 1; \
	done

bash:
	$(CLEANUP)
	mkdir -p bash/{bin,etc}
	cd $(BDIR) && tar xjf ../$(CDIR)/bash-$(BASHVER)-$(CARCH).fpm
	cp -a $(BDIR)/bin/bash bash/bin/
	echo "root:x:0:0::/root:/bin/sh" >bash/etc/passwd

busybox:
	$(CLEANUP)
	cd $(BDIR) && tar xjf ../$(CDIR)/busybox-$(BUSYBOXVER)-$(CARCH).fpm
	cp -a $(BDIR)/usr/share/busybox busybox
	mkdir -p busybox/mnt/{source,target}
	mkdir -p busybox/tmp

dialog:
	$(CLEANUP)
	mkdir -p dialog/bin
	cd $(BDIR) && tar xjf ../$(CDIR)/dialog-$(DIALOGVER)-$(CARCH).fpm
	cp -a $(BDIR)/bin/dialog dialog/bin/

e2fsprogs:
	$(CLEANUP)
	mkdir -p e2fsprogs/{sbin,lib}
	cd $(BDIR) && tar xjf ../$(CDIR)/e2fsprogs-$(E2FSPROGSVER)-$(CARCH).fpm
	cp -a $(BDIR)/sbin/{mke2fs,e2fsck} e2fsprogs/sbin/
	cp -a $(BDIR)/lib/{libblkid*,libcom_err*,libe2p*,libext2fs*,libuuid*} e2fsprogs/lib/
	mkdir e2fsprogs/etc/
	touch e2fsprogs/etc/fstab

reiserfsprogs:
	$(CLEANUP)
	mkdir -p reiserfsprogs/sbin
	cd $(BDIR) && tar xzf ../$(CDIR)/reiserfsprogs-$(REISERFSPROGSVER)-$(CARCH).fpm
	cp -a $(BDIR)/sbin/{mkreiserfs,reiserfsck} reiserfsprogs/sbin/
	mkdir reiserfsprogs/etc/
	touch reiserfsprogs/etc/fstab
lynx:
	$(CLEANUP)
	mkdir -p lynx/usr/bin lynx/etc/lynx
	cd $(BDIR) && tar xjf ../$(CDIR)/lynx-$(LYNXVER)-$(CARCH).fpm
	cp -a $(BDIR)/usr/bin/lynx lynx/usr/bin/
	cp -a $(BDIR)/etc/lynx/lynx.cfg lynx/etc/lynx/

dhcpcd:
	$(CLEANUP)
	mkdir -p dhcpcd/sbin
	cd $(BDIR) && tar xjf ../$(CDIR)/dhcpcd-$(DHCPCDVER)-$(CARCH).fpm
	cp -a $(BDIR)/sbin/dhcpcd dhcpcd/sbin/

frugalware:
	$(CLEANUP)
	mkdir -p frugalware/{var/lib/frugalware/messages/,etc}
	cd $(BDIR) && tar xjf ../$(CDIR)/frugalware-$(FRUGALWAREVER)-$(CARCH).fpm
	cp -a $(BDIR)/var/lib/frugalware/messages/rc.messages \
	        frugalware/var/lib/frugalware/messages/
	cp $(BDIR)/etc/frugalware-release frugalware/etc/
ifeq ($(CARCH),x86_64)
	cp -a $(BDIR)/lib64 frugalware/lib64
endif

glibc:
	$(CLEANUP)
	mkdir -p glibc/{lib,usr/lib/locale}
	cd $(BDIR) && tar xjf ../$(CDIR)/glibc-$(GLIBCVER)-$(CARCH).fpm
	cp -a $(BDIR)/lib/{ld*,libc*,libm*,libdl*,libnss*,libresolv*} glibc/lib/
	
	# generate the necessary locales
	cd $(BDIR) && rm -rf usr/ && mkdir -p usr/lib/locale/
	cd $(BDIR); \
	for i in $(GLIBC_LANGS); do \
		localedef --prefix=./ -c -i $$i $$i; true; \
	done
	cp -a $(BDIR)/usr/lib/locale/locale-archive glibc/usr/lib/locale/

kbd:
	$(CLEANUP)
	mkdir -p kbd/usr/{bin,share/kbd/keymaps,share/kbd/consolefonts}
	cd $(BDIR) && tar xjf ../$(CDIR)/kbd-$(KBDVER)-$(CARCH).fpm
	cp -a $(BDIR)/{bin/loadkeys,usr/bin/setfont} kbd/usr/bin/
	cp -a $(BDIR)/usr/share/kbd/keymaps/{i386,include} \
		kbd/usr/share/kbd/keymaps/
	for i in $(fonts); do \
		cp -a $(BDIR)/usr/share/kbd/consolefonts/$$i \
			kbd/usr/share/kbd/consolefonts/; \
	done

kernel:
	$(CLEANUP)
	mkdir -p kernel/lib
	cd $(BDIR) && tar xjf ../$(CDIR)/kernel-$(KERNELV)-$(KERNELREL)-$(CARCH).fpm
	cp -a $(BDIR)/lib/modules kernel/lib/
	cp $(BDIR)/boot/vmlinuz-$(KERNELV)-fw$(KERNELREL) \
		$(CWD)/vmlinuz-$(KERNELV)-fw$(KERNELREL)-$(CARCH)
	cd kernel/ && find . -name *ko|xargs gzip
	for i in drivers/{cpufreq,telephony,hwmon,media/{dvb,radio,video}} security; do rm -rfv kernel/lib/modules/$(KERNELV)-fw$(KERNELREL)/kernel/$$i; done

module-init-tools:
	$(CLEANUP)
	mkdir -p module-init-tools/{bin,sbin}
	cd $(BDIR) && tar xjf ../$(CDIR)/module-init-tools-$(MODULE-INIT-TOOLSVER)-$(CARCH).fpm
	cp -a $(BDIR)/sbin/* module-init-tools/sbin/

ncurses:
	$(CLEANUP)
	mkdir -p ncurses/{lib,usr/share/terminfo/l}
	cd $(BDIR) && tar xjf ../$(CDIR)/ncurses-$(NCURSESVER)-$(CARCH).fpm
	cp -a $(BDIR)/lib/libncurses* ncurses/lib/
	cp -a $(BDIR)/usr/share/terminfo/l/linux ncurses/usr/share/terminfo/l/

pacman:
	$(CLEANUP)
	mkdir -p pacman/bin pacman/etc/pacman.d/ pacman/usr/{lib,share}
	cd $(BDIR) && tar xjf ../$(CDIR)/pacman-$(PACMANVER)-$(CARCH).fpm
	cp -a $(BDIR)/usr/bin/pacman pacman/bin/pacman
	cp -a $(BDIR)/usr/lib/libalpm.so* pacman/usr/lib/
	cp -a $(BDIR)/usr/bin/vercmp pacman/bin/
	cp -a $(BDIR)/usr/share/locale pacman/usr/share/
	cp -a $(BDIR)/etc/pacman.d/* pacman/etc/pacman.d/
	echo "[options]" >>pacman/etc/pacman.conf
	echo "LogFile     = /mnt/target/var/log/pacman.log" >> pacman/etc/pacman.conf
ifeq ($(STABLE),false)
	echo "Include = /etc/pacman.d/frugalware-current" >> pacman/etc/pacman.conf
	echo "Include = /etc/pacman.d/extra-current" >>pacman/etc/pacman.conf
else
	echo "Include = /etc/pacman.d/frugalware" >> pacman/etc/pacman.conf
	echo "Include = /etc/pacman.d/extra" >>pacman/etc/pacman.conf
endif


udev:
	$(CLEANUP)
	mkdir -p udev/{proc,sys,dev}
	cd $(BDIR) && tar xjf ../$(CDIR)/udev-$(UDEVVER)-$(CARCH).fpm
	cp -a $(BDIR)/{etc,lib,sbin} udev/
	sed -i 's/^source/#source/;s/msg /#msg /;s/ok /#ok /;s/rc_exec/rc_start/;s/ko/ko.gz/' udev/etc/rc.d/rc.hotplug
	sed -i 's|^mount /|#mount /|;s/mount none/#mount none/;s|! \[ `pidof -o .*` \]|true|' udev/etc/rc.d/rc.udev

eject:
	$(CLEANUP)
	mkdir -p eject/bin
	cd $(BDIR) && tar xzf ../$(CDIR)/eject-$(EJECTVER)-$(CARCH).fpm
	cp -a $(BDIR)/usr/bin/eject eject/bin/

util-linux:
	$(CLEANUP)
	mkdir -p util-linux/{sbin,usr/bin}
	cd $(BDIR) && tar xjf ../$(CDIR)/util-linux-$(UTIL-LINUXVER)-$(CARCH).fpm
	cp -a $(BDIR)/sbin/{cfdisk,fdisk} util-linux/sbin/
	cp -a $(BDIR)/usr/bin/setterm util-linux/usr/bin/

netkit-base:
	$(CLEANUP)
	mkdir -p netkit-base/etc
	cd $(BDIR) && tar xjf ../$(CDIR)/netkit-base-$(NETKIT-BASEVER)-$(CARCH).fpm
	cp -a $(BDIR)/etc/services netkit-base/etc/

mdadm:
	$(CLEANUP)
	mkdir -p mdadm/sbin
	cd $(BDIR) && tar xjf ../$(CDIR)/mdadm-$(MDADMVER)-$(CARCH).fpm; \
	cp -a sbin/mdadm ../mdadm/sbin/

xfsprogs:
	$(CLEANUP)
	mkdir -p xfsprogs/sbin
	cd $(BDIR) && tar xjf ../$(CDIR)/xfsprogs-$(XFSPROGSVER)-$(CARCH).fpm; \
	cp -a sbin/mkfs.xfs ../xfsprogs/sbin/

ppp:
	$(CLEANUP)
	mkdir -p ppp/usr
	cd $(BDIR) && tar xzf ../$(CDIR)/ppp-$(PPPVER)-$(CARCH).fpm; \
	cp -a etc ../ppp/; \
	cp -a usr/{lib,sbin} ../ppp/usr/

pppoe:
	$(CLEANUP)
	mkdir -p pppoe/usr/share
	cd $(BDIR) && tar xjf ../$(CDIR)/rp-pppoe-$(RP-PPPOEVER)-$(CARCH).fpm; \
	cp -a etc ../pppoe/; \
	cp -a usr/sbin ../pppoe/usr/; \
	cp -a usr/share/locale ../pppoe/usr/share/
	sed -i 's|/bin/sh|/bin/bash|' pppoe/usr/sbin/adslconfig

glib2:
	$(CLEANUP)
	mkdir -p glib2/usr/lib
	cd $(BDIR) && tar xjf ../$(CDIR)/glib2-$(GLIB2VER)-$(CARCH).fpm; \
	cp -a usr/lib/libglib-2.0.so* ../glib2/usr/lib/

parted:
	$(CLEANUP)
	mkdir -p parted/usr/{lib,sbin,share/locale}
	cd $(BDIR) && tar xjf ../$(CDIR)/parted-$(PARTEDVER)-$(CARCH).fpm; \
	cp -a usr/lib/{libparted.so,libparted-*} ../parted/usr/lib/; \
	cp -a usr/sbin/* ../parted/usr/sbin/; \
	cp -a usr/share/locale/* ../parted/usr/share/locale/

bzip2:
	$(CLEANUP)
	mkdir -p bzip2/lib
	cd $(BDIR) && tar xjf ../$(CDIR)/bzip2-$(BZIP2VER)-$(CARCH).fpm; \
	cp -a lib/libbz2.so* ../bzip2/lib/

libarchive:
	$(CLEANUP)
	mkdir -p libarchive/usr/lib
	cd $(BDIR) && tar xjf ../$(CDIR)/libarchive-$(LIBARCHIVEVER)-$(CARCH).fpm; \
	cp -a usr/lib/libarchive.so* ../libarchive/usr/lib/

zlib:
	$(CLEANUP)
	mkdir -p zlib/usr/lib
	cd $(BDIR) && tar xzf ../$(CDIR)/zlib-$(ZLIBVER)-$(CARCH).fpm; \
	cp -a usr/lib/libz.so* ../zlib/usr/lib/

frugalwareutils:
	$(CLEANUP)
	mkdir -p frugalwareutils/{etc/sysconfig/network,usr/share}
	cd $(BDIR) && tar xjf ../$(CDIR)/frugalwareutils-$(FRUGALWAREUTILSVER)-$(CARCH).fpm; \
	cp -a lib sbin ../frugalwareutils/; \
	cp -a usr/share/locale ../frugalwareutils/usr/share/

wireless_tools:
	$(CLEANUP)
	mkdir -p wireless_tools/usr/
	cd $(BDIR) && tar xjf ../$(CDIR)/wireless_tools-$(WIRELESS_TOOLSVER)-$(CARCH).fpm; \
	cp -a usr/{lib,sbin} ../wireless_tools/usr/

ipw2200-firmware:
	$(CLEANUP)
	mkdir ipw2200-firmware
	cd $(BDIR) && tar xjf ../$(CDIR)/ipw2200-firmware-$(IPW2200-FIRMWAREVER)-$(CARCH).fpm; \
	cp -a lib ../ipw2200-firmware/
