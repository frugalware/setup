/*
 *  skel.c for Frugalware setup
 * 
 *  Copyright (c) 2005 by Miklos Vajna <vmiklos@frugalware.org>
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

#include <stdio.h>
#include <dialog.h>

#include <setup.h>
#include <util.h>
#include "common.h"

plugin_t plugin =
{
	"postconf",
	55,
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}

int run(GList **config)
{
	dialog_vars.backtitle=gen_backtitle(_("Post-install configuration"));
	fw_end_dialog();
	system("chroot ./ /sbin/grubconfig");
	fw_init_dialog();

	dlg_put_backtitle();
	dlg_clear();
	dialog_msgbox(_("Configuring kernel modules"),
		_("Updating module dependencies..."), 0, 0, 0);
	fw_system("chroot ./ /sbin/depmod -a");
	return(0);
}
