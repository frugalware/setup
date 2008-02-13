# Makefile for Frugalware Linux Setup
#
# Compiling Time: 2.45 SBU
#
# Copyright (C) 2005, 2006, 2007, 2008 Miklos Vajna <vmiklos@frugalware.org>
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

ifeq ($(STABLE),)
	STABLE = false
endif
ifeq ($(TESTING),)
	TESTING = false
endif
ifeq ($(DEBUG),)
	DEBUG = false
endif

VERSION=$(shell grep ^version configure |sed 's/.*"\(.*\)"/\1/')
GPG=$(shell [ -d ../releases ] && echo true || echo false)
QEMU_OPTS ?= -hda ~/documents/qemu/hda.img
UML_OPTS ?= ubd0=~/documents/uml/root_fs eth0=tuntap,,,192.168.0.254 mem=128MB

KERNELV = $(shell echo $(KERNELVER)|sed 's/-.*//')
KERNELREL = $(shell echo $(KERNELVER)|sed 's/.*-//')
DESTDIR = $(shell source /etc/repoman.conf; [ -e ~/.repoman.conf ] && source ~/.repoman.conf; echo $$fst_root)

CLEANUP = rm -rf $(BDIR) && mkdir $(BDIR) && rm -rf $@
UNPACK = cd $(BDIR) && tar xf $(CDIR)/$@-$($(shell echo $@|tr '[a-z]' '[A-Z]')VER)-$(CARCH).fpm
export PATH := /usr/lib/ccache/bin:$(PATH)
export CCACHE_DIR=/var/cache/ccache/setup
export CCACHE_NOLINK=1
export CCACHE_UMASK=002
DIALOG_LANGS = `find po -name *.po |sed 's/.*\/\(.*\).po/\1/' |tr '\n' ' '`
GLIBC_LANGS = en_US,ISO-8859-1 da_DK,ISO-8859-1 de_DE,ISO-8859-1 fr_FR,ISO-8859-1 hu_HU,ISO-8859-2 id_ID,ISO-8859-1 it_IT,ISO-8859-1 nl_NL,ISO-8859-1 pt_BR,ISO-8859-1 ro_RO,ISO-8859-2 sk_SK,ISO-8859-2 sv_SE,ISO-8859-1
CARCH ?= $(shell arch)
ifeq ($(CARCH),i686)
	KARCH ?= i386
endif
ifeq ($(CARCH),x86_64)
	MARCH ?= k8
	QEMU ?= qemu-x86_64
endif
MARCH ?= $(CARCH)
KARCH ?= $(CARCH)
QEMU ?= qemu
UML ?= linux
ifeq ($(DEBUG),false)
export CFLAGS = -march=$(MARCH) -O2 -pipe
else
export CFLAGS = -g
endif

CDIR = /var/cache/pacman-g2/pkg
CONFDIR = config
BDIR = build
MDIR = merge
CWD=`pwd`

fonts = lat1-16.psfu.gz lat2-16.psfu.gz lat9w-16.psfu.gz

all: initrd_gz

compile: check ccache $(packages) misc

prepare:
	rm -rf config.mak
	make -C po pos

clean:
	@if [ "`id -u`" != 0 ]; then \
	echo "error: you cannot perform this operation unless you are root."; exit 1; \
	fi
	rm -rf $(BDIR) $(MDIR) initrd-$(CARCH).img initrd-$(CARCH).img.gz
	rm -rf $(packages) vmlinuz-$(KERNELV)-fw$(KERNELREL)-$(CARCH) System.map-$(KERNELV)-fw$(KERNELREL)-$(CARCH)
	$(MAKE) -C src clean

install:
	install -d -m0755 $(DESTDIR)$(PREFIX)/share/setup
	install -m0644 vmlinuz-$(KERNELV)-fw$(KERNELREL)-$(CARCH) $(DESTDIR)$(PREFIX)/share/setup/vmlinuz-$(KERNELV)-fw$(KERNELREL)-$(CARCH)
	install -m0644 initrd-$(CARCH).img.gz $(DESTDIR)$(PREFIX)/share/setup/initrd-$(CARCH).img.gz

distclean: clean
	$(MAKE) -C po distclean
	rm -rf config.mak

dist:
	git-archive --format=tar --prefix=fwsetup-$(VERSION)/ HEAD > fwsetup-$(VERSION).tar
	mkdir -p fwsetup-$(VERSION)/po
	make -C po pos
	mv po/*.{gm,p}o fwsetup-$(VERSION)/po
	git log --no-merges |git name-rev --tags --stdin > fwsetup-$(VERSION)/Changelog
	tar rf fwsetup-$(VERSION).tar fwsetup-$(VERSION)/po/*.{gm,p}o fwsetup-$(VERSION)/Changelog
	rm -rf fwsetup-$(VERSION)
	gzip -f -9 fwsetup-$(VERSION).tar
ifeq ($(GPG),true)
	gpg --comment "See http://ftp.frugalware.org/pub/README.GPG for info" -ba -u 20F55619 fwsetup-$(VERSION).tar.gz
	mv fwsetup-$(VERSION).tar.gz.asc ../releases
	mv fwsetup-$(VERSION).tar.gz ../releases
endif

release:
	git tag -l |grep -q $(VERSION) || dg tag $(VERSION)
	$(MAKE) dist

ccache:
	install -d -m 2775 /var/cache/ccache/setup

setup:
ifeq ($(STABLE),false)
	$(MAKE) -C src current
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

install-setup: setup
	mkdir -p $(MDIR)/lib/plugins
	cp src/setup $(MDIR)/bin/
	cp src/plugins/*.so $(MDIR)/lib/plugins/

misc: merge install-setup
	cp etc/inittab $(MDIR)/etc/
ifneq ($(DEBUG),false)
	sed -i 's|/bin/setup|/usr/bin/gdb /bin/setup|' $(MDIR)/etc/inittab
endif
	cp etc/rc.S $(MDIR)/etc/rc.d/
	chmod +x $(MDIR)/etc/rc.d/rc.S
	cp bin/bootstrap $(MDIR)/bin/

devices: compile
	mknod -m 700 $(MDIR)/dev/console c 5 1
	mknod -m 600 $(MDIR)/dev/null c 1 3
	mknod -m 700 $(MDIR)/dev/tty c 5 0
	mknod -m 700 $(MDIR)/dev/tty1 c 4 1
	mknod -m 700 $(MDIR)/dev/tty2 c 4 2
	mknod -m 700 $(MDIR)/dev/tty3 c 4 3

# this target just updates the setup source itself and the initrd, suitable for qemu testing
initrd: install-setup
	dd if=/dev/zero of=initrd-$(CARCH).img bs=1k count=$$(echo "$$(`which du` -s $(MDIR)|sed 's/^\(.*\)\t.*$$/\1/')+2000"|bc)
	/sbin/mke2fs -F initrd-$(CARCH).img
	mkdir i
	grep -q loop /proc/modules || (/sbin/modprobe loop; sleep 1)
	mount -o loop -t ext2 initrd-$(CARCH).img i
	cp -a $(MDIR)/* i/
	chown -R root.root i/
	umount initrd-$(CARCH).img
	rmdir i

initrd_gz: clean config.mak devices initrd
	gzip -9 -c initrd-$(CARCH).img > initrd-$(CARCH).img.gz

update:
	git pull
	$(MAKE) -C src clean
ifeq ($(STABLE),false)
	$(MAKE) -C src current
else
	$(MAKE) -C src stable
endif
	sudo rm -rf merge initrd*
	sudo $(MAKE) initrd

check:
	pacman-g2 -Swd `grep 'VER =' config.mak |sed 's/VER =.*//' |tr '[A-Z]' '[a-z]'` --noconfirm
	@for i in $(sources); do \
		ls $(CDIR)/$$i >/dev/null || exit 1; \
	done

qemu:
	$(QEMU) -kernel vmlinuz-$(KERNELV)-fw$(KERNELREL)-$(CARCH) -initrd \
	initrd-$(CARCH).img -append "initrd=initrd-$(CARCH).img.gz \
	load_ramdisk=1 prompt_ramdisk=0 ramdisk_size=$(shell du --block-size=1000 initrd-$(CARCH).img|sed 's/\t.*//') \
	rw root=/dev/ram quiet vga=normal" $(QEMU_OPTS)

uml:
	$(UML) $(UML_OPTS) initrd=initrd-$(CARCH).img \
	load_ramdisk=1 prompt_ramdisk=0 ramdisk_size=$(shell du --block-size=1000 initrd-$(CARCH).img|sed 's/\t.*//') \
	rw root=/dev/ram

bash:
	$(CLEANUP)
	mkdir -p bash/{bin,etc}
	$(UNPACK)
	cp -a $(BDIR)/bin/bash bash/bin/
	echo "root:x:0:0::/root:/bin/sh" >bash/etc/passwd

busybox:
	$(CLEANUP)
	$(UNPACK)
	cp -a $(BDIR)/usr/share/busybox busybox
	mkdir -p busybox/mnt/{source,target}
	mkdir -p busybox/tmp
	rm -f busybox/sbin/mkswap

dialog:
	$(CLEANUP)
	mkdir -p dialog/{bin,usr/share/locale}
	$(UNPACK)
	cp -a $(BDIR)/bin/dialog dialog/bin/
	for i in $(DIALOG_LANGS); do \
		if [ -d $(BDIR)/usr/share/locale/$$i ]; then cp -a $(BDIR)/usr/share/locale/$$i dialog/usr/share/locale; fi; \
	done

e2fsprogs:
	$(CLEANUP)
	mkdir -p e2fsprogs/{sbin,lib}
	$(UNPACK)
	cp -a $(BDIR)/sbin/{mke2fs,e2fsck,badblocks,resize2fs} e2fsprogs/sbin/
	cp -a $(BDIR)/lib/{libblkid*,libcom_err*,libe2p*,libext2fs*,libuuid*} e2fsprogs/lib/
	mkdir e2fsprogs/etc/
	touch e2fsprogs/etc/fstab

reiserfsprogs:
	$(CLEANUP)
	mkdir -p reiserfsprogs/sbin
	$(UNPACK)
	cp -a $(BDIR)/sbin/{mkreiserfs,reiserfsck,resize_reiserfs} reiserfsprogs/sbin/
	mkdir reiserfsprogs/etc/
	touch reiserfsprogs/etc/fstab

dhcpcd:
	$(CLEANUP)
	mkdir -p dhcpcd/sbin
	$(UNPACK)
	cp -a $(BDIR)/sbin/dhcpcd dhcpcd/sbin/

frugalware:
	$(CLEANUP)
	mkdir -p frugalware/{var/lib/frugalware/messages/,var/log,var/run,etc}
	$(UNPACK)
	cp -a $(BDIR)/var/lib/frugalware/messages/rc.messages \
	        frugalware/var/lib/frugalware/messages/
	cp $(BDIR)/etc/frugalware-release frugalware/etc/
ifeq ($(CARCH),x86_64)
	cp -a $(BDIR)/lib64 frugalware/lib64
endif

glibc:
	$(CLEANUP)
	mkdir -p glibc/{lib,usr/lib/locale}
	$(UNPACK)
	cp -a $(BDIR)/lib/{ld*,libc*,libm*,libdl*,libnss*,libresolv*,libutil*,libnsl*,librt*,libpthread*,libthread*} glibc/lib/
	
	# generate the necessary locales
	cd $(BDIR) && rm -rf usr/ && mkdir -p usr/lib/locale/
	cd $(BDIR); \
	for i in $(GLIBC_LANGS); do \
		localedef --prefix=./ -c -f $${i##*,} -i $${i%%,*} $${i%%,*}; true; \
	done
	cp -a $(BDIR)/usr/lib/locale/locale-archive glibc/usr/lib/locale/

kbd:
	$(CLEANUP)
	mkdir -p kbd/usr/{bin,share/kbd/keymaps,share/kbd/consolefonts}
	$(UNPACK)
	cp -a $(BDIR)/{bin/loadkeys,usr/bin/setfont,usr/bin/kbd_mode} kbd/usr/bin/
	cp -a $(BDIR)/usr/share/kbd/keymaps/{i386,include} \
		kbd/usr/share/kbd/keymaps/
	for i in $(fonts); do \
		cp -a $(BDIR)/usr/share/kbd/consolefonts/$$i \
			kbd/usr/share/kbd/consolefonts/; \
	done

kernel:
	$(CLEANUP)
	mkdir -p kernel/lib
	$(UNPACK)
	cp -a $(BDIR)/lib/modules kernel/lib/
	cp $(BDIR)/boot/vmlinuz-$(KERNELV)-fw$(KERNELREL) \
		$(CWD)/vmlinuz-$(KERNELV)-fw$(KERNELREL)-$(CARCH)
	cd kernel/ && find . -name *ko|xargs gzip
	for i in drivers/{cpufreq,telephony,hwmon,media/{dvb,radio,video}} security; do rm -rfv kernel/lib/modules/$(KERNELV)-fw$(KERNELREL)/kernel/$$i; done
	cp $(BDIR)/boot/System.map-$(KERNELV)-fw$(KERNELREL) \
		$(CWD)/System.map-$(KERNELV)-fw$(KERNELREL)-$(CARCH)
	depmod -b kernel/ -a -e -F System.map-$(KERNELV)-fw$(KERNELREL)-$(CARCH) -r $(KERNELV)-fw$(KERNELREL)

module-init-tools:
	$(CLEANUP)
	mkdir -p module-init-tools/{bin,sbin}
	$(UNPACK)
	cp -a $(BDIR)/sbin/* module-init-tools/sbin/

ncurses:
	$(CLEANUP)
	mkdir -p ncurses/{lib,usr/share/terminfo/l}
	$(UNPACK)
	cp -a $(BDIR)/lib/libncurses* ncurses/lib/
	cp -a $(BDIR)/usr/share/terminfo/l/linux ncurses/usr/share/terminfo/l/

pacman-g2:
	$(CLEANUP)
	mkdir -p pacman-g2/bin pacman-g2/etc/pacman-g2/repos/ pacman-g2/usr/{lib,share}
	$(UNPACK)
	cp -a $(BDIR)/usr/bin/pacman{,-g2} pacman-g2/bin/
	cp -a $(BDIR)/usr/lib/libpacman.so* pacman-g2/usr/lib/
	cp -a $(BDIR)/usr/bin/vercmp pacman-g2/bin/
	cp -a $(BDIR)/usr/share/locale pacman-g2/usr/share/
	cp -a $(BDIR)/etc/pacman-g2/repos/* pacman-g2/etc/pacman-g2/repos/
	echo "[options]" >>pacman-g2/etc/pacman-g2.conf
ifeq ($(STABLE),false)
	echo "Include = /etc/pacman-g2/repos/frugalware-current" >> pacman-g2/etc/pacman-g2.conf
else
	echo "Include = /etc/pacman-g2/repos/frugalware" >> pacman-g2/etc/pacman-g2.conf
endif
ifneq ($(TESTING),false)
	sed -i 's|current/|testing/|' pacman-g2/etc/pacman-g2/repos/frugalware-current
endif


udev:
	$(CLEANUP)
	mkdir -p udev/{proc,sys,dev}
	$(UNPACK)
	cp -a $(BDIR)/{etc,lib,sbin} udev/
	sed -i 's|^mount /|#mount /|;s/mount none/#mount none/;s|! \[ `pidof -o .*` \]|true|' udev/etc/rc.d/rc.udev

eject:
	$(CLEANUP)
	mkdir -p eject/bin
	$(UNPACK)
	cp -a $(BDIR)/usr/bin/eject eject/bin/

util-linux-ng:
	$(CLEANUP)
	mkdir -p util-linux-ng/{sbin,usr/bin}
	$(UNPACK)
	cp -a $(BDIR)/sbin/{cfdisk,fdisk,mkswap} util-linux-ng/sbin/
	cp -a $(BDIR)/usr/bin/setterm util-linux-ng/usr/bin/

netkit-base:
	$(CLEANUP)
	mkdir -p netkit-base/etc
	$(UNPACK)
	cp -a $(BDIR)/etc/services netkit-base/etc/

mdadm:
	$(CLEANUP)
	mkdir -p mdadm/sbin
	$(UNPACK); \
	cp -a sbin/mdadm ../mdadm/sbin/

xfsprogs:
	$(CLEANUP)
	mkdir -p xfsprogs/sbin
	$(UNPACK); \
	cp -a sbin/mkfs.xfs ../xfsprogs/sbin/

ppp:
	$(CLEANUP)
	mkdir -p ppp/usr
	$(UNPACK); \
	cp -a etc ../ppp/; \
	cp -a usr/{lib,sbin} ../ppp/usr/

rp-pppoe:
	$(CLEANUP)
	mkdir -p rp-pppoe/usr/share
	$(UNPACK); \
	cp -a etc ../rp-pppoe/; \
	cp -a usr/sbin ../rp-pppoe/usr/; \
	cp ../bin/pppoe-start ../rp-pppoe/usr/sbin/

glib2:
	$(CLEANUP)
	mkdir -p glib2/usr/lib
	$(UNPACK); \
	cp -a usr/lib/libglib-2.0.so* ../glib2/usr/lib/

parted:
	$(CLEANUP)
	mkdir -p parted/usr/{lib,sbin,share/locale}
	$(UNPACK); \
	cp -a usr/lib/{libparted.so,libparted-*} ../parted/usr/lib/; \
	cp -a usr/sbin/* ../parted/usr/sbin/; \
	cp -a usr/share/locale/* ../parted/usr/share/locale/

bzip2:
	$(CLEANUP)
	mkdir -p bzip2/lib
	$(UNPACK); \
	cp -a lib/libbz2.so* ../bzip2/lib/

libarchive:
	$(CLEANUP)
	mkdir -p libarchive/usr/lib
	$(UNPACK); \
	cp -a usr/lib/libarchive.so* ../libarchive/usr/lib/

zlib:
	$(CLEANUP)
	mkdir -p zlib/usr/lib
	$(UNPACK); \
	cp -a usr/lib/libz.so* ../zlib/usr/lib/

frugalwareutils:
	$(CLEANUP)
	mkdir -p frugalwareutils/{etc/sysconfig/network,usr/share}
	$(UNPACK); \
	cp -a lib sbin ../frugalwareutils/; \
	cp -a usr/share/locale ../frugalwareutils/usr/share/

wireless_tools:
	$(CLEANUP)
	mkdir -p wireless_tools/usr/
	$(UNPACK); \
	cp -a usr/{lib,sbin} ../wireless_tools/usr/

dropbear:
	$(CLEANUP)
	mkdir -p dropbear/usr/bin
	$(UNPACK); \
	cp -a usr/bin/dbclient ../dropbear/usr/bin/ssh; \
	cp -a usr/bin/dbscp ../dropbear/usr/bin/scp; \
	ln -s ssh ../dropbear/usr/bin/dbclient

bastet:
	$(CLEANUP)
	mkdir -p bastet/usr/bin
	$(UNPACK); \
	cp -a usr/bin/bastet ../bastet/usr/bin/


ipw2200-firmware:
	$(CLEANUP)
	mkdir ipw2200-firmware
	$(UNPACK); \
	cp -a lib ../ipw2200-firmware/

acx100:
	$(CLEANUP)
	mkdir -p acx100
	$(UNPACK); \
	cp -a lib ../acx100/

readline:
	$(CLEANUP)
	mkdir -p readline/usr/lib/
	$(UNPACK); \
	cp -a usr/lib/libreadline.so* ../readline/usr/lib/

shadow:
	$(CLEANUP)
	mkdir -p shadow/etc
	$(UNPACK); \
	cp -a etc/{passwd,group} ../shadow/etc

madwifi:
	$(CLEANUP)
	mkdir -p madwifi/usr/bin
	$(UNPACK); \
	cp -a lib ../madwifi/; \
	cp -a usr/bin/wlanconfig ../madwifi/usr/bin/

rt2500:
	$(CLEANUP)
	mkdir -p rt2500
	$(UNPACK); \
	cp -a lib ../rt2500

gdb:
	$(CLEANUP)
	mkdir -p gdb/usr/bin/
	$(UNPACK); \
	cp -a usr/bin/gdb ../gdb/usr/bin/

expat:
	$(CLEANUP)
	mkdir -p expat/usr
	$(UNPACK); \
	cp -a usr/lib ../expat/usr/

device-mapper:
	$(CLEANUP)
	mkdir -p device-mapper
	$(UNPACK); \
	cp -a lib ../device-mapper/

lvm2:
	$(CLEANUP)
	mkdir -p lvm2
	$(UNPACK); \
	cp -a {etc,sbin} ../lvm2/

wpa_supplicant:
	$(CLEANUP)
	mkdir -p wpa_supplicant/usr
	$(UNPACK); \
	cp -a usr/sbin/ ../wpa_supplicant/usr/

openssl:
	$(CLEANUP)
	mkdir -p openssl/usr/lib
	$(UNPACK); \
	cp -a usr/lib/*.so.* ../openssl/usr/lib/
