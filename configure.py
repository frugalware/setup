#!/usr/bin/env python

import alpm, os, sys

# since we're not in chroot, we need up to date libs. but don't be too
# agressive, we add libs to here just in case there was a bugreport about it
# parted: #807
libs = ['parted', 'pacman']
packages = ['bash', 'kernel', 'busybox', 'dhcpcd', 'dialog', 'e2fsprogs',
		'eject', 'frugalware', 'glibc', 'kbd', 'module-init-tools',
		'ncurses', 'netkit-base', 'reiserfsprogs', 'udev',
		'util-linux', 'mdadm', 'xfsprogs', 'ppp', 'rp-pppoe', 'glib2',
		'bzip2', 'libarchive', 'zlib', 'frugalwareutils',
		'wireless_tools', 'ipw2200-firmware', 'dropbear', 'bastet',
		'readline', 'acx100', 'shadow', 'madwifi-ng']

def pkgGetVers(db, names, ret={}):
	lp = alpm.db_getpkgcache(db)
	while lp:
		pkg = alpm.void_to_PM_PKG(alpm.list_getdata(lp))
		pkgname = alpm.void_to_char(alpm.pkg_getinfo(pkg, alpm.PKG_NAME))
		if pkgname in names and pkgname not in ret:
			ret[pkgname] = pkgname = alpm.void_to_char(alpm.pkg_getinfo(pkg, alpm.PKG_VERSION))
		lp = alpm.list_next(lp)
	return ret

try:
	os.unlink('config.mak')
except OSError:
	pass

if alpm.initialize("/") == -1:
	raise "failed to init the alpm lib"

remote = alpm.db_register("frugalware-current")
local = alpm.db_register("local")
sys.stdout.write("reading the remote database... ")
sys.stdout.flush()
remotevers = pkgGetVers(remote, libs)
sys.stdout.write("done.\nreading the local database... ")
sys.stdout.flush()
localvers = pkgGetVers(local, libs)
sys.stdout.write("done.\n")
sys.stdout.flush()

for k, v in remotevers.items():
	sys.stdout.write("checking for host %s... " % k)
	if remotevers[k] != localvers[k]:
		sys.stdout.write("failed, please do a 'pacman -S %s'\n" % k)
		sys.exit(1)
	else:
		sys.stdout.write("done.\n")

remotevers = pkgGetVers(remote, packages, remotevers)
socket = open("config.mak", "w")
for k, v in localvers.items():
	print "checking for %s... %s" % (k, v)
	socket.write("%sVER = %s\n" % (k.upper(), v))
socket.write("\n")

socket.write("packages = \\\n")
for k, v in localvers.items():
	socket.write("\t   %s \\\n" % k)
socket.write("\t   \n")

socket.write("sources = \\\n")
for k, v in localvers.items():
	socket.write("\t  %s-$(%sVER)-$(CARCH).fpm \\\n" % (k, k.upper()))
socket.write("\t  \n")

socket.close()
alpm.release()
