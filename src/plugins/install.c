/*
 *  install.c for Frugalware setup
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
#include <unistd.h>

#include <setup.h>
#include <util.h>
#include "common.h"

plugin_t plugin =
{
	"skel",
	50,
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}

int run(GList **config)
{
	GList *cats;
	int i;
	char *section;

	// TODO: handle cd changing
	cats = (GList*)data_get(*config, "packages");
	for (i=0; i<g_list_length(cats); i++)
	{
		section = (char*)g_list_nth_data((GList*)g_list_nth_data(cats, i), 0);
		fw_end_dialog();
		system(g_strdup_printf("echo pacman -S %s", g_list_display((GList*)g_list_nth_data(cats, i), " ")));
		fw_init_dialog();
		sleep(3);
	}
	return(0);
}