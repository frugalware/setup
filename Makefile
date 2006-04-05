# Makefile for Frugalware Linux Setup
#
# Compiling Time: 2.45 SBU
#
# Copyright (C) 2005 Miklos Vajna <vmiklos@frugalware.org>
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

VERSION = 0.6.4
STABLE = false
BASHVER = 3.1-3
BUSYVER = 1.1.1-1
DIALOGVER = 1.0_20051030-1
E2VER = 1.38-2
REISERVER = 3.6.19-2
LYNXVER = 2.8.5-3
DHCPVER = 1.3.22pl4-3
FWVER = 0.4-1
NETVER = 1.60-17
LIBCVER = 2.3.6-1
KBDVER = 1.12-17
KERNELVER = 2.6.16
KERNELREL = 2
MODULEVER = 3.2.2-3
NCVER = 5.5-1
PACVER = 2.9.99.24-1
EJECTVER = 2.1.0-1
UDEVVER = 088-2
UTILVER = 2.12-22
NETKITVER = 0.17-3
MDVER = 2.3.1-1
XFSVER = 2.7.11-1
PPPVER = 2.4.3-5
PPPOEVER = 3.7-2
GLIBVER = 2.10.1-1
PEDVER = 1.6.25.1-1
BZVER = 1.0.3-3
ARCHIVEVER = 1.2.51-1
ZVER = 1.2.3-1

export PATH := /usr/lib/ccache/bin:$(PATH)
export CCACHE_DIR=/var/cache/ccache/setup
export CCACHE_NOLINK=1
export CCACHE_UMASK=002
GLIBC_LANGS = en_US es_AR de_DE fr_FR it_IT hu_HU nl_NL pl_PL pt_PT sk_SK pt_BR
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
	   net-tools glibc kbd kernel module-init-tools ncurses pacman eject \
	   udev util-linux netkit-base mdadm xfsprogs ppp pppoe glib2 parted \
	   bzip2 libarchive zlib
fonts = lat1-16.psfu.gz lat2-16.psfu.gz lat9w-16.psfu.gz
sources = bash-$(BASHVER)-$(CARCH).fpm busybox-$(BUSYVER)-$(CARCH).fpm \
	  dhcpcd-$(DHCPVER)-$(CARCH).fpm dialog-$(DIALOGVER)-$(CARCH).fpm \
	  e2fsprogs-$(E2VER)-$(CARCH).fpm eject-$(EJECTVER)-$(CARCH).fpm \
	  frugalware-$(FWVER)-$(CARCH).fpm \
	  glibc-$(LIBCVER)-$(CARCH).fpm kbd-$(KBDVER)-$(CARCH).fpm \
	  kernel-$(KERNELVER)-$(KERNELREL)-$(CARCH).fpm \
	  lynx-$(LYNXVER)-$(CARCH).fpm \
	  module-init-tools-$(MODULEVER)-$(CARCH).fpm \
	  ncurses-$(NCVER)-$(CARCH).fpm netkit-base-$(NETKITVER)-$(CARCH).fpm \
	  net-tools-$(NETVER)-$(CARCH).fpm pacman-$(PACVER)-$(CARCH).fpm \
	  reiserfsprogs-$(REISERVER)-$(CARCH).fpm udev-$(UDEVVER)-$(CARCH).fpm \
	  util-linux-$(UTILVER)-$(CARCH).fpm \
	  netkit-base-$(NETKITVER)-$(CARCH).fpm \
	  mdadm-$(MDVER)-$(CARCH).fpm \
	  xfsprogs-$(XFSVER)-$(CARCH).fpm \
	  ppp-$(PPPVER)-$(CARCH).fpm \
	  rp-pppoe-$(PPPOEVER)-$(CARCH).fpm \
	  glib2-$(GLIBVER)-$(CARCH).fpm \
	  parted-$(PEDVER)-$(CARCH).fpm \
	  bzip2-$(BZVER)-$(CARCH).fpm \
	  libarchive-$(ARCHIVEVER)-$(CARCH).fpm \
	  zlib-$(ZVER)-$(CARCH).fpm

compile: check ccache setup $(packages) misc

clean:
	rm -rf $(BDIR) $(MDIR) initrd-$(CARCH).img.gz dl.lst
	$(MAKE) -C src clean

distclean: clean
	rm -rf $(packages) vmlinuz-$(KERNELVER)-fw$(KERNELREL)-$(CARCH)

cleancache: distclean
	rm -rf $(CDIR)/*
	darcs rev -a $(CDIR)

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
	@echo "All done. Start 'make initrd' now."

devices: compile
	mknod -m 700 $(MDIR)/dev/console c 5 1
	mknod -m 600 $(MDIR)/dev/null c 1 3
	mknod -m 700 $(MDIR)/dev/tty c 5 0
	mknod -m 700 $(MDIR)/dev/tty1 c 4 1
	mknod -m 700 $(MDIR)/dev/tty2 c 4 2
	mknod -m 700 $(MDIR)/dev/tty3 c 4 3

initrd: devices
	dd if=/dev/zero of=initrd-$(CARCH).img bs=1k count=$$(echo "$$(`which du` -s $(MDIR)|sed 's/^\(.*\)\t.*$$/\1/')+500"|bc)
	/sbin/mke2fs -F initrd-$(CARCH).img
	mkdir i
	grep -q loop /proc/modules || /sbin/modprobe loop
	mount -o loop initrd-$(CARCH).img i
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

%.lst: %.lst.in
	sed "s/@CARCH@/$(CARCH)/g" $< > $@

check: dl.lst
	cd $(CDIR) && wget -c -i ../dl.lst
	@for i in $(sources); do \
		ls $(CDIR)/$$i >/dev/null || exit 1; \
	done

bash:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf bash
	mkdir -p bash/{bin,etc}
	cd $(BDIR) && tar xjf ../$(CDIR)/bash-$(BASHVER)-$(CARCH).fpm
	cp -a $(BDIR)/bin/bash bash/bin/
	echo "root:x:0:0::/root:/bin/sh" >bash/etc/passwd

busybox:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf busybox
	cd $(BDIR) && tar xjf ../$(CDIR)/busybox-$(BUSYVER)-$(CARCH).fpm
	cp -a $(BDIR)/usr/share/busybox busybox
	mkdir -p busybox/mnt/{source,target}
	mkdir -p busybox/tmp

dialog:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf dialog
	mkdir -p dialog/bin
	cd $(BDIR) && tar xjf ../$(CDIR)/dialog-$(DIALOGVER)-$(CARCH).fpm
	cp -a $(BDIR)/bin/dialog dialog/bin/

e2fsprogs:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf e2fsprogs
	mkdir -p e2fsprogs/{sbin,lib}
	cd $(BDIR) && tar xjf ../$(CDIR)/e2fsprogs-$(E2VER)-$(CARCH).fpm
	cp -a $(BDIR)/sbin/{mke2fs,e2fsck} e2fsprogs/sbin/
	cp -a $(BDIR)/lib/{libblkid*,libcom_err*,libe2p*,libext2fs*,libuuid*} e2fsprogs/lib/
	mkdir e2fsprogs/etc/
	touch e2fsprogs/etc/fstab

reiserfsprogs:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf reiserfsprog
	mkdir -p reiserfsprogs/sbin
	cd $(BDIR) && tar xzf ../$(CDIR)/reiserfsprogs-$(REISERVER)-$(CARCH).fpm
	cp -a $(BDIR)/sbin/{mkreiserfs,reiserfsck} reiserfsprogs/sbin/
	mkdir reiserfsprogs/etc/
	touch reiserfsprogs/etc/fstab
lynx:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf lynx
	mkdir -p lynx/usr/bin lynx/etc/lynx
	cd $(BDIR) && tar xjf ../$(CDIR)/lynx-$(LYNXVER)-$(CARCH).fpm
	cp -a $(BDIR)/usr/bin/lynx lynx/usr/bin/
	cp -a $(BDIR)/etc/lynx/lynx.cfg lynx/etc/lynx/

dhcpcd:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf dhcpcd
	mkdir -p dhcpcd/sbin
	cd $(BDIR) && tar xjf ../$(CDIR)/dhcpcd-$(DHCPVER)-$(CARCH).fpm
	cp -a $(BDIR)/sbin/dhcpcd dhcpcd/sbin/

frugalware:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf frugalware
	mkdir -p frugalware/{var/lib/frugalware/messages/,etc}
	cd $(BDIR) && tar xjf ../$(CDIR)/frugalware-$(FWVER)-$(CARCH).fpm
	cp -a $(BDIR)/var/lib/frugalware/messages/rc.messages \
	        frugalware/var/lib/frugalware/messages/
	cp $(BDIR)/etc/frugalware-release frugalware/etc/
ifeq ($(CARCH),x86_64)
	cp -a $(BDIR)/lib64 frugalware/lib64
endif

net-tools:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf net-tools
	mkdir -p net-tools/{etc/sysconfig/network,usr/share,sbin}
	cd $(BDIR) && tar xjf ../$(CDIR)/net-tools-$(NETVER)-$(CARCH).fpm; \
	cp -a sbin/netconfig ../net-tools/sbin/; \
	cp -a usr/share/locale ../net-tools/usr/share/

glibc:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf glibc
	mkdir -p glibc/{lib,usr/lib/locale}
	cd $(BDIR) && tar xjf ../$(CDIR)/glibc-$(LIBCVER)-$(CARCH).fpm
	cp -a $(BDIR)/lib/{ld*,libc*,libm*,libdl*,libnss*,libresolv*} glibc/lib/
	
	# generate the necessary locales
	cd $(BDIR) && rm -rf usr/ && mkdir -p usr/lib/locale/
	cd $(BDIR); \
	for i in $(GLIBC_LANGS); do \
		localedef --prefix=./ -c -i $$i $$i; true; \
	done
	cp -a $(BDIR)/usr/lib/locale/locale-archive glibc/usr/lib/locale/

kbd:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf kbd
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
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf kernel
	mkdir -p kernel/lib
	cd $(BDIR) && tar xjf ../$(CDIR)/kernel-$(KERNELVER)-$(KERNELREL)-$(CARCH).fpm
	cp -a $(BDIR)/lib/modules kernel/lib/
	cp $(BDIR)/boot/boot/vmlinuz-$(KERNELVER)-fw$(KERNELREL) \
		$(CWD)/../../vmlinuz-$(KERNELVER)-fw$(KERNELREL)-$(CARCH)
	cd kernel/ && find . -name *ko|xargs gzip

module-init-tools:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf module-init-tools
	mkdir -p module-init-tools/{bin,sbin}
	cd $(BDIR) && tar xjf ../$(CDIR)/module-init-tools-$(MODULEVER)-$(CARCH).fpm
	cp -a $(BDIR)/sbin/* module-init-tools/sbin/

ncurses:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf ncurses
	mkdir -p ncurses/{lib,usr/share/terminfo/l}
	cd $(BDIR) && tar xjf ../$(CDIR)/ncurses-$(NCVER)-$(CARCH).fpm
	cp -a $(BDIR)/lib/libncurses* ncurses/lib/
	cp -a $(BDIR)/usr/share/terminfo/l/linux ncurses/usr/share/terminfo/l/

pacman:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf pacman
	mkdir -p pacman/bin pacman/etc/pacman.d/ pacman/usr/lib
	cd $(BDIR) && tar xjf ../$(CDIR)/pacman-$(PACVER)-$(CARCH).fpm
	cp -a $(BDIR)/usr/bin/pacman pacman/bin/pacman
	cp -a $(BDIR)/usr/lib/libalpm.so* pacman/usr/lib/
	cp -a $(BDIR)/usr/bin/vercmp pacman/bin/
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
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf udev
	mkdir -p udev/{proc,sys,dev}
	cd $(BDIR) && tar xjf ../$(CDIR)/udev-$(UDEVVER)-$(CARCH).fpm
	cp -a $(BDIR)/{etc,lib,sbin} udev/
	sed -i 's/^source/#source/;s/msg /#msg /;s/ok /#ok /;s/rc_exec/rc_start/;s/ko/ko.gz/' udev/etc/rc.d/rc.hotplug
	sed -i 's|^mount /|#mount /|;s/mount none/#mount none/;s|! \[ `pidof -o .*` \]|true|' udev/etc/rc.d/rc.udev

eject:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf eject
	mkdir -p eject/bin
	cd $(BDIR) && tar xzf ../$(CDIR)/eject-$(EJECTVER)-$(CARCH).fpm
	cp -a $(BDIR)/usr/bin/eject eject/bin/

util-linux:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf util-linux
	mkdir -p util-linux/{sbin,usr/bin}
	cd $(BDIR) && tar xjf ../$(CDIR)/util-linux-$(UTILVER)-$(CARCH).fpm
	cp -a $(BDIR)/sbin/{cfdisk,fdisk} util-linux/sbin/
	cp -a $(BDIR)/usr/bin/setterm util-linux/usr/bin/

netkit-base:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf netkit-base
	mkdir -p netkit-base/etc
	cd $(BDIR) && tar xzf ../$(CDIR)/netkit-base-$(NETKITVER)-$(CARCH).fpm
	cp -a $(BDIR)/etc/services netkit-base/etc/

mdadm:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf mdadm
	mkdir -p mdadm/sbin mdadm/var/lib/frugalware/{messages,system}
	cd $(BDIR) && tar xjf ../$(CDIR)/mdadm-$(MDVER)-$(CARCH).fpm; \
	cp -a sbin/mdadm ../mdadm/sbin/; \
	cp -a var/lib/frugalware/messages/* \
		../mdadm/var/lib/frugalware/messages/; \
	cp -a var/lib/frugalware/system/* \
		../mdadm/var/lib/frugalware/system/; \
	cp -a sbin/raidconfig ../mdadm/sbin/

xfsprogs:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf xfsprogs
	mkdir -p xfsprogs/sbin
	cd $(BDIR) && tar xjf ../$(CDIR)/xfsprogs-$(XFSVER)-$(CARCH).fpm; \
	cp -a sbin/mkfs.xfs ../xfsprogs/sbin/

ppp:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf ppp
	mkdir -p ppp/usr
	cd $(BDIR) && tar xzf ../$(CDIR)/ppp-$(PPPVER)-$(CARCH).fpm; \
	cp -a etc ../ppp/; \
	cp -a usr/{lib,sbin} ../ppp/usr/

pppoe:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf pppoe
	mkdir -p pppoe/usr/share
	cd $(BDIR) && tar xjf ../$(CDIR)/rp-pppoe-$(PPPOEVER)-$(CARCH).fpm; \
	cp -a etc ../pppoe/; \
	cp -a usr/sbin ../pppoe/usr/; \
	cp -a usr/share/locale ../pppoe/usr/share/
	sed -i 's|/bin/sh|/bin/bash|' pppoe/usr/sbin/adslconfig

glib2:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf glib2
	mkdir -p glib2/usr/lib
	cd $(BDIR) && tar xjf ../$(CDIR)/glib2-$(GLIBVER)-$(CARCH).fpm; \
	cp -a usr/lib/libglib-2.0.so* ../glib2/usr/lib/

parted:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf parted
	mkdir -p parted/usr/{lib,sbin,share/locale}
	cd $(BDIR) && tar xjf ../$(CDIR)/parted-$(PEDVER)-$(CARCH).fpm; \
	cp -a usr/lib/{libparted.so,libparted-*} ../parted/usr/lib/; \
	cp -a usr/sbin/* ../parted/usr/sbin/; \
	cp -a usr/share/locale/* ../parted/usr/share/locale/

bzip2:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf bzip2
	mkdir -p bzip2/lib
	cd $(BDIR) && tar xjf ../$(CDIR)/bzip2-$(BZVER)-$(CARCH).fpm; \
	cp -a lib/libbz2.so* ../bzip2/lib/

libarchive:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf libarchive
	mkdir -p libarchive/usr/lib
	cd $(BDIR) && tar xjf ../$(CDIR)/libarchive-$(ARCHIVEVER)-$(CARCH).fpm; \
	cp -a usr/lib/libarchive.so* ../libarchive/usr/lib/

zlib:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf zlib
	mkdir -p zlib/usr/lib
	cd $(BDIR) && tar xzf ../$(CDIR)/zlib-$(ZVER)-$(CARCH).fpm; \
	cp -a usr/lib/libz.so* ../zlib/usr/lib/
