# Makefile for Frugalware Linux Setup
#
# Copyright (C) 2005 Miklos Vajna <mamajom@axelero.hu>
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

VERSION = 0.6.2
BASHVER = 3.0-6
BUSYVER = 1.00
DIALOGVER = 1.0_20050306-1
E2VER = 1.37-1
REISERVER = 3.6.19-2
LYNXVER = 2.8.5-2
DHCPVER = 1.3.22pl4-2
FWVER = 0.2-1
NETVER = 1.60-10
LIBCVER = 2.3.5-1
KBDVER = 1.12-9
KERNELVER = 2.6.11
KERNELREL = 6
MODULEVER = 3.1-4
NCVER = 5.4-2
PACVER = 2.9.5-8
EJECTVER = 2.0.13-2
UDEVVER = 057
UTILVER = 2.12-15
NETKITVER = 0.17-3

export PATH := /usr/lib/ccache/bin:$(PATH)
export CFLAGS = -march=i686 -O2 -pipe

CDIR = cache
CONFDIR = config
BDIR = build
MDIR = merge
CWD=`pwd`

packages = bash busybox dialog e2fsprogs reiserfsprogs lynx dhcpcd frugalware \
	   net-tools glibc kbd kernel module-init-tools ncurses pacman eject \
	   udev util-linux netkit-base
fonts = lat1-16.psfu.gz lat2-16.psfu.gz lat9w-16.psfu.gz
kpatches = linux-$(KERNELVER)-2.6.11.10.diff linux-2.6-seg-5.patch \
	   bootsplash-3.1.4-$(KERNELVER).diff
sources = $(kpatches) bash-$(BASHVER).fpm busybox-$(BUSYVER).tar.gz \
	  dhcpcd-$(DHCPVER).fpm dialog-$(DIALOGVER).fpm \
	  e2fsprogs-$(E2VER).fpm eject-$(EJECTVER).fpm frugalware-$(FWVER).fpm \
	  glibc-$(LIBCVER).fpm kbd-$(KBDVER).fpm linux-$(KERNELVER).tar.bz2 \
	  lynx-$(LYNXVER).fpm module-init-tools-$(MODULEVER).fpm \
	  ncurses-$(NCVER).fpm netkit-base-$(NETKITVER).fpm \
	  net-tools-$(NETVER).fpm pacman-$(PACVER).fpm \
	  reiserfsprogs-$(REISERVER).fpm udev-$(UDEVVER).tar.gz \
	  util-linux-$(UTILVER).fpm

compile: check $(packages) misc

clean:
	rm -rf $(BDIR) $(MDIR) $(packages)

distclean: clean
	rm -rf $(CDIR)/* vmlinuz-$(KERNELVER)-fw$(KERNELREL)

merge: $(packages)
	rm -rf $(MDIR)
	mkdir $(MDIR)
	for i in $(packages); do \
		cp -a $$i/* $(MDIR)/; \
	done

misc: merge
	cp src/setup* src/*.conf $(MDIR)/bin/
	chmod +x $(MDIR)/bin/setup
	cp src/inittab $(MDIR)/etc/
	cp src/rc.S $(MDIR)/etc/rc.d/
	chmod +x $(MDIR)/etc/rc.d/rc.S
	cp src/rc.hotplug $(MDIR)/etc/rc.d/
	chmod +x $(MDIR)/etc/rc.d/rc.hotplug
	@echo "All done. Start 'make initrd' now."

devices: misc
	mknod -m 700 $(MDIR)/dev/console c 5 1
	mknod -m 600 $(MDIR)/dev/null c 1 3
	mknod -m 700 $(MDIR)/dev/tty c 5 0
	mknod -m 700 $(MDIR)/dev/tty1 c 4 1
	mknod -m 700 $(MDIR)/dev/tty2 c 4 2
	mknod -m 700 $(MDIR)/dev/tty3 c 4 3

initrd: devices
	dd if=/dev/zero of=initrd.img bs=1k count=$$(echo "$$(`which du` -s $(MDIR)|sed 's/^\(.*\)\t.*$$/\1/')+500"|bc)
	/sbin/mke2fs -F initrd.img
	mkdir i
	grep -q loop /proc/modules || /sbin/modprobe loop
	mount -o loop initrd.img i
	cp -a $(MDIR)/* i/
	umount initrd.img
	rmdir i
	gzip -9 initrd.img

check:
	@for i in $(sources); do \
		ls cache/$$i || exit 1; \
	done

bash:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf bash
	mkdir -p bash/{bin,etc}
	cd $(BDIR) && tar xzf ../$(CDIR)/bash-$(BASHVER).fpm
	cp -a $(BDIR)/bin/bash bash/bin/
	echo "root:x:0:0::/root:/bin/sh" >bash/etc/passwd

busybox:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf busybox
	mkdir -p busybox/mnt/{source,target}
	mkdir -p busybox/tmp
	cd $(BDIR) && tar xzf ../$(CDIR)/busybox-$(BUSYVER).tar.gz
	cp $(CONFDIR)/busybox.config $(BDIR)/busybox-$(BUSYVER)/.config
	cd $(BDIR)/busybox-$(BUSYVER); \
	sed -i "s/-march=i686/$(CFLAGS)/" .config; \
	make; \
	make PREFIX=$(CWD)/../../busybox install

dialog:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf dialog
	mkdir -p dialog/bin
	cd $(BDIR) && tar xzf ../$(CDIR)/dialog-$(DIALOGVER).fpm
	cp -a $(BDIR)/bin/dialog dialog/bin/

e2fsprogs:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf e2fsprogs
	mkdir -p e2fsprogs/{sbin,lib}
	cd $(BDIR) && tar xzf ../$(CDIR)/e2fsprogs-$(E2VER).fpm
	cp -a $(BDIR)/sbin/{mke2fs,e2fsck} e2fsprogs/sbin/
	cp -a $(BDIR)/lib/{libblkid*,libcom_err*,libe2p*,libext2fs*,libuuid*} e2fsprogs/lib/
	mkdir e2fsprogs/etc/
	touch e2fsprogs/etc/fstab

reiserfsprogs:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf reiserfsprog
	mkdir -p reiserfsprogs/sbin
	cd $(BDIR) && tar xzf ../$(CDIR)/reiserfsprogs-$(REISERVER).fpm
	cp -a $(BDIR)/sbin/{mkreiserfs,reiserfsck} reiserfsprogs/sbin/
	mkdir reiserfsprogs/etc/
	touch reiserfsprogs/etc/fstab
lynx:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf lynx
	mkdir -p lynx/usr/bin lynx/usr/lib
	cd $(BDIR) && tar xzf ../$(CDIR)/lynx-$(LYNXVER).fpm
	cp -a $(BDIR)/usr/bin/lynx lynx/usr/bin/
	cp -a $(BDIR)/usr/lib/lynx.cfg lynx/usr/lib/

dhcpcd:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf dhcpcd
	mkdir -p dhcpcd/sbin
	cd $(BDIR) && tar xzf ../$(CDIR)/dhcpcd-$(DHCPVER).fpm
	cp -a $(BDIR)/sbin/dhcpcd dhcpcd/sbin/

frugalware:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf frugalware
	mkdir -p frugalware/{var/lib/frugalware/messages/,etc}
	cd $(BDIR) && tar xzf ../$(CDIR)/frugalware-$(FWVER).fpm
	cp -a $(BDIR)/var/lib/frugalware/messages/rc.messages \
	        frugalware/var/lib/frugalware/messages/
	cp $(BDIR)/etc/frugalware-release frugalware/etc/

net-tools:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf net-tools
	mkdir -p net-tools/{etc/rc.d/,etc/sysconfig/,sbin/} \
		net-tools/var/lib/frugalware/{messages,system}
	cd $(BDIR) && tar xzf ../$(CDIR)/net-tools-$(NETVER).fpm; \
	cp -a var/lib/frugalware/messages/* \
		../net-tools/var/lib/frugalware/messages/; \
	sed -i 's/^\(gethostname .*\)/# \1\nhname=frugalware\ndname=example.net/;s/--default-item lo/--default-item dhcp/' \
		var/lib/frugalware/system/netconfig; \
	cp -a var/lib/frugalware/system/* \
		../net-tools/var/lib/frugalware/system/; \
	sed -i 's|/bin/sh|/bin/bash|' etc/rc.d/rc.interfaces; \
	sed -i 's/grep -w/grep/;s/grep -vw/grep -v/' etc/rc.d/rc.interfaces; \
	cp -a etc/rc.d/rc.interfaces ../net-tools/etc/rc.d/; \
	cp -a sbin/netconfig ../net-tools/sbin/

glibc:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf glibc
	mkdir -p glibc/lib
	cd $(BDIR) && tar xzf ../$(CDIR)/glibc-$(LIBCVER).fpm
	cp -a $(BDIR)/lib/{ld*,libc*,libm*,libdl*,libnss*,libresolv*} glibc/lib/

kbd:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf kbd
	mkdir -p kbd/usr/{bin,share/kbd/keymaps,share/kbd/consolefonts}
	cd $(BDIR) && tar xzf ../$(CDIR)/kbd-$(KBDVER).fpm
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
	mkdir -p kernel
	cd $(BDIR) && tar xjf ../$(CDIR)/linux-$(KERNELVER).tar.bz2; \
	cd linux-$(KERNELVER); \
	for i in $(kpatches); do \
		patch -p1 < ../../$(CDIR)/$$i; \
	done; \
	cp ../../$(CONFDIR)/kernel.config .config; \
	sed -i "s/EXTRAVERSION =.*/EXTRAVERSION = -fw$(KERNELREL)/" Makefile; \
	yes "" | make config >/dev/null; \
	make; \
	mkdir -p $(CWD)/../../kernel/lib; \
	make INSTALL_MOD_PATH=$(CWD)/../../kernel/ modules_install; \
	cp arch/i386/boot/bzImage \
		$(CWD)/../../vmlinuz-$(KERNELVER)-fw$(KERNELREL)
	cd kernel/ && find . -name *ko|xargs gzip

module-init-tools:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf module-init-tools
	mkdir -p module-init-tools/{bin,sbin}
	cd $(BDIR) && tar xzf ../$(CDIR)/module-init-tools-$(MODULEVER).fpm
	cp -a $(BDIR)/bin/* module-init-tools/bin/
	cp -a $(BDIR)/sbin/* module-init-tools/sbin/

ncurses:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf ncurses
	mkdir -p ncurses/{lib,usr/share/terminfo/l}
	cd $(BDIR) && tar xzf ../$(CDIR)/ncurses-$(NCVER).fpm
	cp -a $(BDIR)/lib/libncurses* ncurses/lib/
	cp -a $(BDIR)/usr/share/terminfo/l/linux ncurses/usr/share/terminfo/l/

pacman:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf pacman
	mkdir -p pacman/bin pacman/etc/pacman.d/
	cd $(BDIR) && tar xzf ../$(CDIR)/pacman-$(PACVER).fpm
	cp -a $(BDIR)/usr/bin/pacman.static pacman/bin/pacman
	cp -a $(BDIR)/usr/bin/vercmp pacman/bin/
	cp -a $(BDIR)/etc/pacman.d/* pacman/etc/pacman.d/
	echo "Include = /etc/pacman.d/frugalware-current" >> pacman/etc/pacman.conf
	echo "Include = /etc/pacman.d/extra-current" >>pacman/etc/pacman.conf
	echo "# Include = /etc/pacman.d/frugalware" >>pacman/etc/pacman.conf
	echo "# Include = /etc/pacman.d/extra" >>pacman/etc/pacman.conf

udev:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf udev
	mkdir -p udev
	cd $(BDIR) && tar xzf ../$(CDIR)/udev-$(UDEVVER).tar.gz; \
	cd udev-$(UDEVVER); \
	sed -i 's|udevdir =\t$${prefix}/udev|udevdir =\t$${prefix}/dev|' Makefile; \
	make; \
	make DESTDIR=$(CWD)/../../udev install
	mkdir udev/etc/rc.d/
	cat $(BDIR)/udev-$(UDEVVER)/extras/start_udev |sed 's/echo "/#echo "/' \
		>udev/etc/rc.d/rc.udev
	chmod 755 udev/etc/rc.d/rc.udev
	mkdir udev/{proc,sys}
	ln -s udevsend udev/sbin/hotplug

eject:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf eject
	mkdir -p eject/bin
	cd $(BDIR) && tar xzf ../$(CDIR)/eject-$(EJECTVER).fpm
	cp -a $(BDIR)/usr/bin/eject eject/bin/

util-linux:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf util-linux
	mkdir -p util-linux/{sbin,usr/bin}
	cd $(BDIR) && tar xzf ../$(CDIR)/util-linux-$(UTILVER).fpm
	cp -a $(BDIR)/sbin/{cfdisk,fdisk} util-linux/sbin/
	cp -a $(BDIR)/usr/bin/setterm util-linux/usr/bin/

netkit-base:
	rm -rf $(BDIR)
	mkdir $(BDIR)
	rm -rf netkit-base
	mkdir -p netkit-base/etc
	cd $(BDIR) && tar xzf ../$(CDIR)/netkit-base-$(NETKITVER).fpm
	cp -a $(BDIR)/etc/services netkit-base/etc/
