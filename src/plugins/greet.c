/*
 *  greet.c for Frugalware setup
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
	"greet",
	desc,
	10,
	run,
	NULL // dlopen handle
};

char *desc()
{
	return _("Welcome splash");
}

plugin_t *info()
{
	return &plugin;
}

int run(GList **config)
{
	char *version = get_version();
	char *title=NULL;

	asprintf(&title, _("Welcome to %s"), version);

	dialog_vars.backtitle=gen_backtitle(_("Welcome"));
	dlg_put_backtitle();
	dlg_clear();
	dialog_msgbox(title, _("Welcome among the users of Frugalware!\n\n"
		"The aim of creating Frugalware was to help you to do your work "
		"faster and simpler. We hope that you will like our "
		"product.\n\n"
		"The Frugalware Developer Team"), 0, 0, 1);
	FREE(version);

	return(0);
}
