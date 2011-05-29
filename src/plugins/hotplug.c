/*
 *  hotplug.c for Frugalware setup
 * 
 *  Copyright (c) 2008 by Miklos Vajna <vmiklos@frugalware.org>
 * 
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, 
 *  USA.
 */

#define _GNU_SOURCE
#include <stdio.h>
#ifdef DIALOG
    #include <dialog.h>
#endif
#ifdef GTK
    #include <gtk/gtk.h>
#endif

#include <setup.h>
#include <util.h>
#include "common.h"

plugin_t plugin =
{
	"hotplug",
	desc,
	00,
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}

char *desc()
{
	return _("Detecting hardware");
}

int run(GList **config)
{
	dialog_vars.backtitle=gen_backtitle("Detecting hardware");
	dlg_put_backtitle();
	dlg_clear();
	dialog_msgbox("Please wait", "Activating hardware detection...",
		0, 0, 0);
	// TODO: this is ugly
	fw_system("mount -t proc none /proc");
	fw_system("mount -t sysfs none /sys");
	fw_system("mount -t tmpfs none /tmp");
	fw_system("mount -t tmpfs none /run");
	fw_system("ln -sf /proc/self/mounts /etc/mtab");
	fw_system("modprobe isofs");
	fw_system("modprobe ntfs");
	fw_system("modprobe BusLogic");

	// try to load all the hub modules
	fw_system("modprobe -q ehci-hcd");
	fw_system("modprobe -q ohci-hcd");
	fw_system("modprobe -q uhci-hcd");

	// the real hw detect
	fw_system("udevd --daemon");
	fw_system("udevadm trigger --type=subsystems");
	fw_system("udevadm trigger --type=devices");
	fw_system("udevadm settle");

	fw_system("setterm -blank 0");

	// no utf8 for now
	fw_system("kbd_mode -a /dev/tty1");
	fw_system("echo -n -e '\033%@' > /dev/tty1");
	fw_system("kbd_mode -a /dev/tty2");
	fw_system("echo -n -e '\033%@' > /dev/tty2");
	fw_system("kbd_mode -a /dev/tty3");
	fw_system("echo -n -e '\033%@' > /dev/tty3");
	fw_system("kbd_mode -a /dev/tty4");
	fw_system("echo -n -e '\033%@' > /dev/tty4");

	return(0);
}
