# Makefile for Frugalware Linux Setup
#
# Compiling Time: 2.45 SBU
#
# Copyright (C) 2005, 2006, 2007, 2008, 2009, 2010 Miklos Vajna <vmiklos@frugalware.org>
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

CARCH ?= $(shell arch)
ifneq ($(CARCH),ppc)
	VMLINUZ=vmlinuz
else
	VMLINUZ=vmlinux
endif
VERSION=$(shell grep ^version configure |sed 's/.*"\(.*\)"/\1/')
GPG=$(shell [ -d ../releases ] && echo true || echo false)
QEMU_OPTS ?= -hda ~/documents/qemu/hda.img -m 1024 -enable-kvm
UML_OPTS ?= ubd0=~/documents/uml/root_fs eth0=tuntap,,,192.168.0.254 mem=128MB
TFTP_BOOTCMD = bootp
ifneq ($(TFTP_PASSWD),)
	TFTP_GRUB_PASSWD := password --md5 $(shell echo -e 'md5crypt\n$(TFTP_PASSWD)\nquit' |/sbin/grub --batch --device-map=/dev/null |grep "^Encrypted: " |sed 's/^Encrypted: //')
endif
RAMDISK_SIZE = $(shell du --block-size=1000 initrd-$(CARCH).img|sed 's/\t.*//')
CYL_COUNT = $(shell echo "$(shell du -c -B516096 $(VMLINUZ)-$(KERNELV)-fw$(KERNELREL)-$(CARCH) initrd-$(CARCH).img.gz|sed -n 's/^\(.*\)\t.*$$/\1/;$$ p')+10"|bc)
OFFSET = $(shell echo "$(shell /sbin/fdisk -u -l frugalware-$(FWVER)-$(CARCH)-usb.img | sed -ne "s|^frugalware-$(FWVER)-$(CARCH)-usb.img1[ *]*\([0-9]*\).*|\1|p")*512"|bc)

FWVER = $(shell echo $(FRUGALWAREVER)|sed 's/-.*//')
RELEASE = $(shell cat merge/etc/frugalware-release)
KERNELV = $(shell echo $(KERNELVER)|sed 's/-.*//')
KERNELREL = $(shell echo $(KERNELVER)|sed 's/.*-//')
DESTDIR = $(shell source /etc/repoman.conf; [ -e ~/.repoman.conf ] && source ~/.repoman.conf; echo $$fst_root)

CLEANUP = rm -rf $(BDIR) && mkdir $(BDIR) && rm -rf $@
UNPACK = cd $(BDIR) && bsdtar xf $(CDIR)/$@-$($(shell echo $@|tr '[a-z]' '[A-Z]')VER)-$(CARCH).fpm
DIALOG_LANGS = $(shell find po -name *.po |sed 's/.*\/\(.*\).po/\1/' |tr '\n' ' ')
GLIBC_LANGS = en_US,ISO-8859-1 da_DK,ISO-8859-1 de_DE,ISO-8859-1 fr_FR,ISO-8859-1 hu_HU,ISO-8859-2 id_ID,ISO-8859-1 it_IT,ISO-8859-1 nl_NL,ISO-8859-1 pt_BR,ISO-8859-1 ro_RO,ISO-8859-2 sk_SK,ISO-8859-2 sv_SE,ISO-8859-1 cs_CZ,ISO-8859-2 es_ES,ISO-8859-1 ru_RU,ISO-8859-5 tr_TR,ISO-8859-9 bg_BG,UTF-8
GTK_LANGS = cs da de fr hu id it nl pt_BR ro sk sv tr
ifeq ($(CARCH),x86_64)
	QEMU ?= qemu-system-x86_64
endif
ifeq ($(CARCH),ppc)
	QEMU ?= qemu-system-ppc
endif
QEMU ?= qemu
UML ?= linux
ifneq ($(DEBUG),false)
export CFLAGS = -g
endif
ifeq ($(GUI),true)
EXTRA_TARGETS += gui-iso
else
ifeq ($(USB),true)
EXTRA_TARGETS += usb_img
endif
ifeq ($(TFTP),true)
EXTRA_TARGETS += tftp_img
endif
endif

CDIR = $(PWD)/cache
CONFDIR = config
BDIR = build
MDIR = merge
CWD=`pwd`

fonts = lat1-16.psfu.gz lat2a-16.psfu.gz lat9w-16.psfu.gz

all: initrd_gz $(EXTRA_TARGETS)

compile: check $(packages) misc

prepare:
	rm -rf config.mak
	make -C po pos GLIBC_LANGS="$(GLIBC_LANGS)"

check_root:
	@if [ "$(shell id -u)" != 0 ]; then \
	echo "error: you cannot perform this operation unless you are root."; exit 1; \
	fi

clean: check_root
	rm -rf $(BDIR) $(MDIR) initrd-$(CARCH).img initrd-$(CARCH).img.gz
	rm -rf $(packages) $(VMLINUZ)-$(KERNELV)-fw$(KERNELREL)-$(CARCH) System.map-$(KERNELV)-fw$(KERNELREL)-$(CARCH)
	$(MAKE) -C src clean

install:
	install -d -m0755 $(DESTDIR)$(PREFIX)/share/setup
ifeq ($(GUI),true)
	install -m0644 fwife-$(FWVER)-$(CARCH).iso $(DESTDIR)$(PREFIX)/share/setup/fwife-$(FWVER)-$(CARCH).iso
else
	install -m0644 $(VMLINUZ)-$(KERNELV)-fw$(KERNELREL)-$(CARCH) $(DESTDIR)$(PREFIX)/share/setup/$(VMLINUZ)-$(KERNELV)-fw$(KERNELREL)-$(CARCH)
	install -m0644 initrd-$(CARCH).img.gz $(DESTDIR)$(PREFIX)/share/setup/initrd-$(CARCH).img.gz
ifeq ($(USB),true)
	install -m0644 frugalware-$(FWVER)-$(CARCH)-usb.img $(DESTDIR)$(PREFIX)/share/setup/frugalware-$(FWVER)-$(CARCH)-usb.img
endif
ifeq ($(TFTP),true)
	install -m0644 frugalware-$(FWVER)-$(CARCH)-tftp.img $(DESTDIR)$(PREFIX)/share/setup/frugalware-$(FWVER)-$(CARCH)-tftp.img
endif
endif

distclean: clean
	$(MAKE) -C po distclean
	rm -rf config.mak

dist:
	git archive --format=tar --prefix=fwsetup-$(VERSION)/ HEAD > fwsetup-$(VERSION).tar
	mkdir -p fwsetup-$(VERSION)/po
	make -C po pos
	mv po/*.po fwsetup-$(VERSION)/po
	git log --no-merges |git name-rev --tags --stdin > fwsetup-$(VERSION)/Changelog
	tar rf fwsetup-$(VERSION).tar fwsetup-$(VERSION)/po/*.po fwsetup-$(VERSION)/Changelog
	rm -rf fwsetup-$(VERSION)
	gzip -f -9 fwsetup-$(VERSION).tar
ifeq ($(GPG),true)
	gpg --comment "See http://ftp.frugalware.org/pub/README.GPG for info" -ba -u 20F55619 fwsetup-$(VERSION).tar.gz
	mv fwsetup-$(VERSION).tar.gz.asc ../releases
	mv fwsetup-$(VERSION).tar.gz ../releases
endif

release:
	git tag -l |grep -q $(VERSION) || dg tag $(VERSION)
	$(MAKE) dist GLIBC_LANGS="$(GLIBC_LANGS)"

setup:
ifeq ($(GUI),false)
ifeq ($(STABLE),false)
	$(MAKE) -C src current
else
	$(MAKE) -C src stable
endif
endif

merge: $(packages)
	rm -rf $(MDIR)
	mkdir $(MDIR)
	for i in $(packages); do \
		cp -a $$i/* $(MDIR)/; \
	done
	touch $(MDIR)/etc/ld.so.conf
	/sbin/ldconfig -r $(MDIR)
	make -C po mos

install-setup: setup
ifeq ($(GUI),false)
	mkdir -p $(MDIR)/lib/plugins
	cp src/setup $(MDIR)/bin/
	cp src/plugins/*.so $(MDIR)/lib/plugins/
endif

misc: merge install-setup
ifeq ($(GUI),true)
	cp gui/inittab $(MDIR)/etc/
	cp gui/start.sh $(MDIR)/bin/
	cp gui/startx.sh $(MDIR)/bin/
	chmod +x $(MDIR)/bin/start.sh
	chmod +x $(MDIR)/bin/startx.sh
	mkfontdir $(MDIR)/usr/share/fonts/X11/TTF
	update-mime-database $(MDIR)/usr/share/mime
	ln -s /usr/share/X11/xkb $(MDIR)/usr/lib/X11/xkb
else
	cp etc/inittab $(MDIR)/etc/
endif
ifeq ($(DEBUG),gdb)
	sed -i 's|/bin/setup|/bin/gdb-start|' $(MDIR)/etc/inittab
	cp bin/gdb-start $(MDIR)/bin/
endif
ifeq ($(DEBUG),valgrind)
	sed -i 's|/bin/setup|/bin/valgrind-start|' $(MDIR)/etc/inittab
	cp bin/valgrind-start $(MDIR)/bin/
endif
	cp bin/bootstrap $(MDIR)/bin/
	cp bin/dhcpcd $(MDIR)/sbin/
	mkdir -p $(MDIR)/usr/share/udhcpc/
	cp bin/default.script $(MDIR)/usr/share/udhcpc/

devices: compile
	mknod -m 700 $(MDIR)/dev/console c 5 1
	mknod -m 600 $(MDIR)/dev/null c 1 3
	mknod -m 700 $(MDIR)/dev/tty c 5 0
	mknod -m 700 $(MDIR)/dev/tty1 c 4 1
	mknod -m 700 $(MDIR)/dev/tty2 c 4 2
	mknod -m 700 $(MDIR)/dev/tty3 c 4 3
	mknod -m 700 $(MDIR)/dev/tty4 c 4 4
	mknod -m 700 $(MDIR)/dev/tty5 c 4 5

# this target just updates the setup source itself and the initrd, suitable for qemu testing
initrd: install-setup
	cd $(MDIR); find . |cpio -R 0:0 -H newc -o --quiet > ../initrd-$(CARCH).img

initrd_gz: clean config.mak devices initrd
	gzip -9 -c initrd-$(CARCH).img > initrd-$(CARCH).img.gz

create_usb_img:
ifeq ($(CARCH),ppc)
	dd if=/dev/zero of=frugalware-$(FWVER)-$(CARCH)-usb.img bs=516096c count=$(shell echo $(CYL_COUNT) + 2 | bc)
endif

usb_img: check_root create_usb_img
ifeq ($(CARCH),ppc)
	hformat frugalware-$(FWVER)-$(CARCH)-usb.img
	hmount frugalware-$(FWVER)-$(CARCH)-usb.img
	hcopy -r /usr/lib/yaboot/yaboot :
	hattrib -c UNIX -t tbxi :yaboot
	hattrib -b :
	humount
	mkdir i
	mount -o loop frugalware-$(FWVER)-$(CARCH)-usb.img i
	cp $(VMLINUZ)-$(KERNELV)-fw$(KERNELREL)-$(CARCH) i/$(VMLINUZ)-$(KERNELV)-fw$(KERNELREL)
	cp initrd-$(CARCH).img.gz i/
	echo -e "default=install\n\
	root=/dev/ram\n\
	message=/boot.msg\n\
	image=/$(VMLINUZ)-$(KERNELV)-fw$(KERNELREL)\n\
	label=install\n\
		initrd=/initrd-$(CARCH).img.gz\n\
		initrd-size=$(RAMDISK_SIZE)\n\
		read-write" > i/yaboot.conf
	echo -e "$(RELEASE) - $(KERNELV)-fw$(KERNELREL) \n\n\
	To boot the kernel, just hit enter, or use 'install'.\n\n\
	If the system fails to boot at all (the typical symptom is a white screen which\n\
	doesn't go away), use 'install video=ofonly'." > i/boot.msg
	umount i
	rmdir i
endif

tftp_img: check_root
	dd if=/dev/zero of=frugalware-$(FWVER)-$(CARCH)-tftp.img bs=1k count=1440
	/sbin/mke2fs -F frugalware-$(FWVER)-$(CARCH)-tftp.img
	mkdir i
	mount -o loop frugalware-$(FWVER)-$(CARCH)-tftp.img i
	mkdir -p i/boot/grub
	cp /usr/lib/grub/i386-*/stage1 i/boot/grub/
	cp /usr/lib/grub/i386-*/stage2.netboot i/boot/grub/stage2
	echo -e 'default=0 \n\
		timeout=10 \n\
		$(TFTP_GRUB_PASSWD)\n\
		title $(RELEASE) - $(KERNELV)-fw$(KERNELREL) \n\
		$(TFTP_GRUB_PASSWD)\n\
		$(TFTP_BOOTCMD)\n\
		root (nd)\n\
		kernel /$(VMLINUZ)-$(KERNELV)-fw$(KERNELREL)-$(CARCH) vga=791 \n\
		initrd /initrd-$(CARCH).img.gz \n\
		title $(RELEASE) - $(KERNELV)-fw$(KERNELREL) (nofb) \n\
		$(TFTP_GRUB_PASSWD)\n\
		$(TFTP_BOOTCMD)\n\
		root (nd)\n\
		kernel /$(VMLINUZ)-$(KERNELV)-fw$(KERNELREL)-$(CARCH) \n\
		initrd /initrd-$(CARCH).img.gz' > i/boot/grub/menu.lst
	umount i
	rmdir i
	echo -e "device (fd0) frugalware-$(FWVER)-$(CARCH)-tftp.img \n\
		root (fd0) \n\
		setup (fd0) \n\
		quit" | grub --batch --device-map=/dev/null

gui-iso:
	mkdir -p iso/boot/grub
ifneq ($(CARCH),ppc)
	cp /usr/lib/grub/i386-frugalware/stage2_eltorito iso/boot/grub
	cp /boot/grub/message-frugalware iso/boot/grub/message
	cp initrd-$(CARCH).img.gz iso/boot/
	cp $(VMLINUZ)-$(KERNELV)-fw$(KERNELREL)-$(CARCH) iso/boot/$(VMLINUZ)-$(KERNELV)-fw$(KERNELREL)
	echo -e "default=0 \n\
		timeout=10 \n\
		gfxmenu /boot/grub/message \n\
		title Fwife $(FWVER) - $(KERNELV)-fw$(KERNELREL)-$(CARCH) \n\
		kernel /boot/$(VMLINUZ)-$(KERNELV)-fw$(KERNELREL) \n\
		initrd /boot/initrd-$(CARCH).img.gz" > iso/boot/grub/menu.lst
	mkisofs -R -b boot/grub/stage2_eltorito -no-emul-boot \
         -boot-load-size 4 -boot-info-table -o fwife-$(FWVER)-$(CARCH).iso iso
endif
	rm -rf iso

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

pacman-g2.conf:
	sed 's|/var/log/pacman-g2.log|$(PWD)/pacman-g2.log|' /etc/pacman-g2.conf > pacman-g2.conf
	sed -i 's|LogFile|CacheDir = $(CDIR)\nLogFile|' pacman-g2.conf

check: pacman-g2.conf
	pacman-g2 --config pacman-g2.conf -Swd $(shell grep 'VER =' config.mak |sed 's/VER =.*//' |tr '[A-Z]' '[a-z]') --noconfirm
	@for i in $(sources); do \
		ls $(CDIR)/$$i >/dev/null || exit 1; \
	done

qemu:
	$(QEMU) -kernel $(VMLINUZ)-$(KERNELV)-fw$(KERNELREL)-$(CARCH) -initrd initrd-$(CARCH).img $(QEMU_OPTS)

uml:
	$(UML) $(UML_OPTS)

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
	ln -s linuxrc busybox/init

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
# mkfs.ext{2,3} is used by gparted
ifeq ($(GUI),false)
	cp -a $(BDIR)/sbin/{mke2fs,e2fsck,badblocks,resize2fs,mkfs.ext4} e2fsprogs/sbin/
else
	cp -a $(BDIR)/sbin/{mke2fs,e2fsck,badblocks,resize2fs,mkfs.ext4,mkfs.ext2,mkfs.ext3} e2fsprogs/sbin/
endif
	cp -a $(BDIR)/lib/{libcom_err*,libe2p*,libext2fs*} e2fsprogs/lib/
	mkdir e2fsprogs/etc/
	touch e2fsprogs/etc/fstab

reiserfsprogs:
	$(CLEANUP)
	mkdir -p reiserfsprogs/sbin
	$(UNPACK)
	cp -a $(BDIR)/sbin/{mkreiserfs,reiserfsck,resize_reiserfs} reiserfsprogs/sbin/
	mkdir reiserfsprogs/etc/
	touch reiserfsprogs/etc/fstab

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
	mkdir -p kbd/usr/{bin,share/keymaps,share/consolefonts}
	$(UNPACK)
	cp -a $(BDIR)/usr/bin/{loadkeys,setfont,kbd_mode} kbd/usr/bin/
	cp -a $(BDIR)/usr/share/keymaps/{i386,include} \
		kbd/usr/share/keymaps/
	for i in $(fonts); do \
		cp -a $(BDIR)/usr/share/consolefonts/$$i \
			kbd/usr/share/consolefonts/; \
	done

kernel:
	$(CLEANUP)
	mkdir -p kernel/lib
	$(UNPACK)
	cp -a $(BDIR)/lib/modules kernel/lib/
	cp $(BDIR)/boot/$(VMLINUZ)-$(KERNELV)-fw$(KERNELREL) \
		$(CWD)/$(VMLINUZ)-$(KERNELV)-fw$(KERNELREL)-$(CARCH)
	cd kernel/ && find . -name *ko|xargs gzip
	for i in drivers/{cpufreq,telephony,hwmon,media/{dvb,radio,video}} security sound; do rm -rfv kernel/lib/modules/$(KERNELV)-fw$(KERNELREL)/kernel/$$i; done
	cp $(BDIR)/boot/System.map-$(KERNELV)-fw$(KERNELREL) \
		$(CWD)/System.map-$(KERNELV)-fw$(KERNELREL)-$(CARCH)
	/sbin/depmod -b kernel/ -a -e -F System.map-$(KERNELV)-fw$(KERNELREL)-$(CARCH) -r $(KERNELV)-fw$(KERNELREL)

module-init-tools:
	$(CLEANUP)
	mkdir -p module-init-tools/{bin,sbin}
	$(UNPACK)
	cp -a $(BDIR)/sbin/* module-init-tools/sbin/

ncurses:
	$(CLEANUP)
	mkdir -p ncurses/{lib,usr/share/terminfo/l}
	$(UNPACK)
	cp -a $(BDIR)/lib/libncurses*.so* ncurses/lib/
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
	echo "LogFile     = /var/log/pacman-g2.log" >>pacman-g2/etc/pacman-g2.conf
	echo "MaxTries = 5" >>pacman-g2/etc/pacman-g2.conf
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
	mkdir -p udev/{proc,sys,dev,run}
	$(UNPACK)
	cp -a $(BDIR)/{etc,lib,sbin} udev/

eject:
	$(CLEANUP)
	mkdir -p eject/bin
	$(UNPACK)
	cp -a $(BDIR)/usr/bin/eject eject/bin/

util-linux:
	$(CLEANUP)
	mkdir -p util-linux/{sbin,usr/bin,lib}
	$(UNPACK)
	cp -a $(BDIR)/sbin/{cfdisk,fdisk,mkswap,blkid} util-linux/sbin/
	cp -a $(BDIR)/usr/bin/setterm util-linux/usr/bin/

libblkid:
	$(CLEANUP)
	mkdir -p libblkid/lib
	$(UNPACK)
	cp -a $(BDIR)/lib/libblkid.so* libblkid/lib/

libuuid:
	$(CLEANUP)
	mkdir -p libuuid/lib
	$(UNPACK)
	cp -a $(BDIR)/lib/libuuid.so* libuuid/lib/

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
	$(UNPACK)
ifeq ($(GUI),false)
	cp -a $(BDIR)/usr/lib/libglib-2.0.so* glib2/usr/lib/
else
	cp -a $(BDIR)/usr/lib/*.so* glib2/usr/lib/
endif

parted:
	$(CLEANUP)
	mkdir -p parted/usr/{lib,sbin,share/locale}
	$(UNPACK); \
	cp -a usr/lib/{libparted.so*,libparted-*} ../parted/usr/lib/; \
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

# A minimal libpython, just to make gdb happy
python:
	$(CLEANUP)
	mkdir -p python/usr/lib/python2.7
	$(UNPACK); \
	cp -a usr/lib/libpython* ../python/usr/lib/; \
	touch ../python/usr/lib/python2.7/site.py

# Deals with only a minimal subset of valgrind to keep the initrd small
valgrind:
	$(CLEANUP)
	mkdir -p valgrind/usr/bin/
	mkdir -p valgrind/usr/lib/valgrind/x86-linux/
	$(UNPACK); \
	cp -a usr/bin/valgrind* ../valgrind/usr/bin/; \
	cp -a usr/lib/valgrind/*.supp ../valgrind/usr/lib/valgrind/; \
	cp -a usr/lib/valgrind/x86-linux/memcheck ../valgrind/usr/lib/valgrind/x86-linux/

expat:
	$(CLEANUP)
	mkdir -p expat/usr/lib
	$(UNPACK); \
	cp -a usr/lib/*.so* ../expat/usr/lib/

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

libnl:
	$(CLEANUP)
	mkdir -p libnl/usr/lib
	$(UNPACK); \
	cp -a usr/lib/*.so.* ../libnl/usr/lib/

openssl:
	$(CLEANUP)
	mkdir -p openssl/usr/lib
	$(UNPACK); \
	cp -a usr/lib/*.so.* ../openssl/usr/lib/

pciutils:
	$(CLEANUP)
	mkdir -p pciutils/usr/{lib,share/misc}
	$(UNPACK); \
	cp -a usr/share/misc/* ../pciutils/usr/share/misc/; \
	cp -a usr/lib/*.so.* ../pciutils/usr/lib/

b43-fwcutter:
	$(CLEANUP)
	mkdir b43-fwcutter
	$(UNPACK); \
	cp -a lib ../b43-fwcutter/

mac-fdisk:
	$(CLEANUP)
	mkdir mac-fdisk
	$(UNPACK); \
	cp -a sbin ../mac-fdisk/

lzma:
	$(CLEANUP)
	mkdir -p lzma/usr/lib
	$(UNPACK); \
	cp -a usr/lib/liblzmadec.so* ../lzma/usr/lib/

linux-firmware:
	$(CLEANUP)
	mkdir linux-firmware
	$(UNPACK); \
	cp -a lib ../linux-firmware/

coreutils:
	$(CLEANUP)
	mkdir -p coreutils/bin
	$(UNPACK)
	cp -a $(BDIR)/bin/nice coreutils/bin/

tzdata:
	$(CLEANUP)
	mkdir -p tzdata/usr/share/zoneinfo
	$(UNPACK); \
	cp -a usr/share/zoneinfo/zone.tab ../tzdata/usr/share/zoneinfo/

libxau:
	$(CLEANUP)
	mkdir -p libxau/usr/lib
	$(UNPACK); \
	cp -a usr/lib/libXau.so* ../libxau/usr/lib

libxcb:
	$(CLEANUP)
	mkdir -p libxcb/usr/lib
	$(UNPACK); \
	cp -a usr/lib/libxcb.so* ../libxcb/usr/lib

libxdmcp:
	$(CLEANUP)
	mkdir -p libxdmcp/usr/lib
	$(UNPACK); \
	cp -a usr/lib/libXdmcp.so* ../libxdmcp/usr/lib

libx11:
	$(CLEANUP)
	mkdir -p libx11/usr/{lib,share}
	$(UNPACK); \
	cp -a usr/lib/libX11.so* ../libx11/usr/lib/; \
	cp -a usr/lib/X11 ../libx11/usr/lib/; \
	cp -a usr/share/X11 ../libx11/usr/share/; \
	rm -rf ../libx11/usr/share/X11/locales

libfontenc:
	$(CLEANUP)
	mkdir -p libfontenc/usr/lib
	$(UNPACK); \
	cp -a usr/lib/libfontenc.so* ../libfontenc/usr/lib

libxfont:
	$(CLEANUP)
	mkdir -p libxfont/usr/lib
	$(UNPACK); \
	cp -a usr/lib/libXfont.so* ../libxfont/usr/lib

libxext:
	$(CLEANUP)
	mkdir -p libxext/usr/lib
	$(UNPACK); \
	cp -a usr/lib/libXext.so* ../libxext/usr/lib

libxrender:
	$(CLEANUP)
	mkdir -p libxrender/usr/lib
	$(UNPACK); \
	cp -a usr/lib/libXrender.so* ../libxrender/usr/lib

libxrandr:
	$(CLEANUP)
	mkdir -p libxrandr/usr/lib
	$(UNPACK); \
	cp -a usr/lib/libXrandr.so* ../libxrandr/usr/lib

libxft:
	$(CLEANUP)
	mkdir -p libxft/usr/lib
	$(UNPACK); \
	cp -a usr/lib/libXft.so* ../libxft/usr/lib

libxpm:
	$(CLEANUP)
	mkdir -p libxpm/usr/lib
	$(UNPACK); \
	cp -a usr/lib/libXpm.so* ../libxpm/usr/lib

libxkbfile:
	$(CLEANUP)
	mkdir -p libxkbfile/usr/lib
	$(UNPACK); \
	cp -a usr/lib/*.so* ../libxkbfile/usr/lib

libice:
	$(CLEANUP)
	mkdir -p libice/usr/lib
	$(UNPACK); \
	cp -a usr/lib/libICE.so* ../libice/usr/lib

libsm:
	$(CLEANUP)
	mkdir -p libsm/usr/lib
	$(UNPACK); \
	cp -a usr/lib/libSM.so* ../libsm/usr/lib

pixman:
	$(CLEANUP)
	mkdir -p pixman/usr/lib
	$(UNPACK); \
	cp -a usr/lib/libpixman-1.so* ../pixman/usr/lib

libpciaccess:
	$(CLEANUP)
	mkdir -p libpciaccess/usr/lib
	$(UNPACK); \
	cp -a usr/lib/libpciaccess.so* ../libpciaccess/usr/lib

setxkbmap:
	$(CLEANUP)
	mkdir -p setxkbmap/usr/bin
	$(UNPACK); \
	cp -a usr/bin/setxkbmap ../setxkbmap/usr/bin

xkbcomp:
	$(CLEANUP)
	mkdir -p xkbcomp/usr/bin
	$(UNPACK); \
	cp -a usr/bin/xkbcomp ../xkbcomp/usr/bin

xkeyboard-config:
	$(CLEANUP)
	mkdir -p xkeyboard-config/usr/share
	$(UNPACK); \
	cp -a usr/share/X11 ../xkeyboard-config/usr/share/

xorg-server-minimal:
	$(CLEANUP)
	mkdir -p xorg-server-minimal/{var/{log,lib/xkb},usr/{bin,lib/{X11,xorg/modules/extensions}}}
	$(UNPACK); \
	cp -a usr/bin/Xorg-fwife ../xorg-server-minimal/usr/bin/Xorg; \
	cp -a etc ../xorg-server-minimal/; \
	cp -a usr/lib/xorg-fwife/modules/*.so ../xorg-server-minimal/usr/lib/xorg/modules/; \
	cp -a usr/lib/xorg-fwife/modules/extensions/*.so ../xorg-server-minimal/usr/lib/xorg/modules/extensions/

xf86-input-evdev:
	$(CLEANUP)
	mkdir -p xf86-input-evdev/usr/lib/xorg/modules/input
	$(UNPACK); \
	cp -a usr/lib/xorg/modules/input/*.so ../xf86-input-evdev/usr/lib/xorg/modules/input/

xf86-video-vesa:
	$(CLEANUP)
	mkdir -p xf86-video-vesa/usr/lib/xorg/modules/drivers/
	$(UNPACK); \
	cp -a usr/lib/xorg/modules/drivers/*.so ../xf86-video-vesa/usr/lib/xorg/modules/drivers/

freetype2:
	$(CLEANUP)
	mkdir -p freetype2/usr/lib
	$(UNPACK); \
	cp -a usr/lib/*.so* ../freetype2/usr/lib

fontconfig:
	$(CLEANUP)
	mkdir -p fontconfig/usr/lib
	$(UNPACK); \
	cp -a usr/lib/*.so* ../fontconfig/usr/lib; \
	cp -a etc ../fontconfig/; \
	cp -a usr/bin ../fontconfig/

shared-mime-info:
	$(CLEANUP)
	mkdir -p shared-mime-info/usr/share
	$(UNPACK); \
	cp -a usr/share/mime ../shared-mime-info/usr/share

xloadimage:
	$(CLEANUP)
	mkdir -p xloadimage/usr/bin
	$(UNPACK); \
	cp -a usr/bin/xloadimage ../xloadimage/usr/bin/

libxml2:
	$(CLEANUP)
	mkdir -p libxml2/usr/lib
	$(UNPACK); \
	cp -a usr/lib/*.so* ../libxml2/usr/lib

atk:
	$(CLEANUP)
	mkdir -p atk/usr/{lib,share}
	$(UNPACK); \
	cp -a usr/lib/*.so* ../atk/usr/lib

libpng:
	$(CLEANUP)
	mkdir -p libpng/usr/lib
	$(UNPACK); \
	cp -a usr/lib/*.so* ../libpng/usr/lib

libjpeg:
	$(CLEANUP)
	mkdir -p libjpeg/usr/lib
	$(UNPACK); \
	cp -a usr/lib/*.so* ../libjpeg/usr/lib

libstdc++:
	$(CLEANUP)
	mkdir -p libstdc++/usr/lib
	$(UNPACK); \
	cp -a usr/lib/*.so* ../libstdc++/usr/lib

libgcc:
	$(CLEANUP)
	mkdir -p libgcc/usr/lib
	$(UNPACK); \
	cp -a usr/lib/*.so* ../libgcc/usr/lib

libsigc++2:
	$(CLEANUP)
	mkdir -p libsigc++2/usr/lib
	$(UNPACK); \
	cp -a usr/lib/*.so* ../libsigc++2/usr/lib

openbox-minimal:
	$(CLEANUP)
	mkdir -p openbox-minimal/usr/{bin,lib}
	$(UNPACK); \
	cp -a usr/bin/openbox-fwife ../openbox-minimal/usr/bin/openbox; \
	cp -a usr/lib/openbox-fwife/*.so* ../openbox-minimal/usr/lib

cairo-minimal:
	$(CLEANUP)
	mkdir -p cairo-minimal/usr/lib/
	$(UNPACK); \
	cp -a usr/lib/cairo-fwife/libcairo.so* ../cairo-minimal/usr/lib

pango-minimal:
	$(CLEANUP)
	mkdir -p pango-minimal/{etc/pango,usr/{bin,lib}}
	$(UNPACK); \
	cp -a usr/lib/pango-fwife/libpango*.so* ../pango-minimal/usr/lib; \
	cp -a etc/pango-fwife/* ../pango-minimal/etc/pango/

gdk-pixbuf2-minimal:
	$(CLEANUP)
	mkdir -p gdk-pixbuf2-minimal/usr/lib
	$(UNPACK); \
	cp -a usr/lib/gdk-pixbuf2-fwife/libgdk_pixbuf-2.0.so* ../gdk-pixbuf2-minimal/usr/lib

gtk+2-minimal:
	$(CLEANUP)
	mkdir -p gtk+2-minimal/usr/{share/locale,lib}
	$(UNPACK); \
	cp -a usr/lib/gtk-fwife/lib{gdk,gtk}*.so* ../gtk+2-minimal/usr/lib; \
	cp -a usr/lib/gtk-fwife/gtk-2.0 ../gtk+2-minimal/usr/lib; \
	rm -rf ../gtk+2-minimal/usr/lib/gtk-2.0/{include,*/*/*.la,modules/*.la}
	for i in $(GTK_LANGS); do \
		if [ -d $(BDIR)/usr/share/locale-fwife/$$i ]; then cp -a $(BDIR)/usr/share/locale-fwife/$$i gtk+2-minimal/usr/share/locale; fi; \
	done

fwife-minimal:
	$(CLEANUP)
	mkdir -p fwife-minimal/usr/lib/fwife/plugins/
	$(UNPACK); \
	cp -a usr/share/fwife-minimal/usr/{share,bin} ../fwife-minimal/usr/; \
	cp -a usr/share/fwife-minimal/usr/lib/fwife/plugins/*.so ../fwife-minimal/usr/lib/fwife/plugins/

glibmm:
	$(CLEANUP)
	mkdir -p glibmm/usr/lib
	$(UNPACK); \
	cp -a usr/lib/*.so* ../glibmm/usr/lib

cairomm-minimal:
	$(CLEANUP)
	mkdir -p cairomm-minimal/usr/lib
	$(UNPACK); \
	cp -a usr/lib/cairomm-fwife/*.so* ../cairomm-minimal/usr/lib

pangomm-minimal:
	$(CLEANUP)
	mkdir -p pangomm-minimal/usr/lib
	$(UNPACK); \
	cp -a usr/lib/pangomm-fwife/*.so* ../pangomm-minimal/usr/lib

gtkmm-minimal:
	$(CLEANUP)
	mkdir -p gtkmm-minimal/usr/lib
	$(UNPACK); \
	cp -a usr/lib/gtkmm-fwife/*.so* ../gtkmm-minimal/usr/lib

gparted-minimal:
	$(CLEANUP)
	mkdir -p gparted-minimal/usr/{bin,share/locale}
	$(UNPACK); \
	cp -a usr/sbin/gpartedbin-fwife ../gparted-minimal/usr/bin/gparted
	for i in $(GTK_LANGS); do \
		if [ -d $(BDIR)/usr/share/locale-fwife/$$i ]; then cp -a $(BDIR)/usr/share/locale-fwife/$$i gparted-minimal/usr/share/locale; fi; \
	done

dejavu-ttf:
	$(CLEANUP)
	mkdir -p dejavu-ttf/usr/share/fonts/X11/TTF
	$(UNPACK); \
	cp -a usr/share/fonts/X11/TTF/{DejaVuSans-Bold.ttf,DejaVuSans.ttf,DejaVuSans-Oblique.ttf} ../dejavu-ttf/usr/share/fonts/X11/TTF

fwifecd-config:
	$(CLEANUP)
	mkdir -p fwifecd-config/{etc,usr/share}
	$(UNPACK); \
	cp -a usr/share/fwifecd-config/openbox/* ../fwifecd-config/usr/share/; \
	cp -a usr/share/fwifecd-config/gtk/lib ../fwifecd-config/usr/; \
	cp -a usr/share/fwifecd-config/gtk/gtk-2.0 ../fwifecd-config/etc/

nouveau-firmware:
	$(CLEANUP)
	mkdir -p nouveau-firmware/lib/firmware/nouveau
	$(UNPACK); \
	cp -a lib/firmware/nouveau/* ../nouveau-firmware/lib/firmware/nouveau/

xf86-video-ati:
	$(CLEANUP)
	mkdir -p xf86-video-ati/usr/lib/xorg/modules/drivers/
	$(UNPACK); \
	cp -a usr/lib/xorg/modules/drivers/*.so ../xf86-video-ati/usr/lib/xorg/modules/drivers/

xf86-video-geode:
	$(CLEANUP)
	mkdir -p xf86-video-geode/usr/lib/xorg/modules/drivers/
	$(UNPACK); \
	cp -a usr/lib/xorg/modules/drivers/*.so ../xf86-video-geode/usr/lib/xorg/modules/drivers/

xf86-video-intel:
	$(CLEANUP)
	mkdir -p xf86-video-intel/usr/lib/xorg/modules/drivers/
	$(UNPACK); \
	cp -a usr/lib/xorg/modules/drivers/*.so ../xf86-video-intel/usr/lib/xorg/modules/drivers/

xf86-video-nouveau:
	$(CLEANUP)
	mkdir -p xf86-video-nouveau/usr/lib/xorg/modules/drivers/
	$(UNPACK); \
	cp -a usr/lib/xorg/modules/drivers/*.so ../xf86-video-nouveau/usr/lib/xorg/modules/drivers/

xf86-video-openchrome:
	$(CLEANUP)
	mkdir -p xf86-video-openchrome/usr/lib/xorg/modules/drivers/
	$(UNPACK); \
	cp -a usr/lib/xorg/modules/drivers/*.so ../xf86-video-openchrome/usr/lib/xorg/modules/drivers/

libdrm:
	$(CLEANUP)
	mkdir -p libdrm/usr/lib
	$(UNPACK); \
	cp -a usr/lib/*.so* ../libdrm/usr/lib

icu4c:
	$(CLEANUP)
	mkdir -p icu4c/usr/lib
	$(UNPACK); \
	cp -a usr/lib/*.so* ../icu4c/usr/lib

gptfdisk:
	$(CLEANUP)
	mkdir -p gptfdisk/sbin
	$(UNPACK); \
	cp -a sbin/{gdisk,fixparts} ../gptfdisk/sbin

xz:
	$(CLEANUP)
	mkdir -p xz/usr/lib
	$(UNPACK); \
	cp -a usr/lib/*.so* ../xz/usr/lib/

dbus:
	$(CLEANUP)
	mkdir -p dbus/usr/lib
	$(UNPACK); \
	cp -a usr/lib/*.so* ../dbus/usr/lib/

libffi:
	$(CLEANUP)
	mkdir -p libffi/usr/lib
	$(UNPACK); \
	cp -a usr/lib/libffi.so* ../libffi/usr/lib

.NOTPARALLEL:
