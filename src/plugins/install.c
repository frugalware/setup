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

extern GList *plugin_list;

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

GList *genfwcats(int cdnum)
{
	GList *list=NULL;

	if(cdnum==1)
	{
		list = g_list_append(list, "base");
		list = g_list_append(list, "apps");
		list = g_list_append(list, "lib");
		list = g_list_append(list, "multimedia");
		list = g_list_append(list, "network");
		list = g_list_append(list, "devel");
	}
	else if(cdnum==2)
	{
		list = g_list_append(list, "x11");
		list = g_list_append(list, "xlib");
		list = g_list_append(list, "xapps");
		list = g_list_append(list, "xfce4");
		list = g_list_append(list, "gnome");
		list = g_list_append(list, "kde");
	}
	return(list);
}

int installpkgs_forreal(GList *cats)
{
	int i;
	char *section, *ptr;

	for (i=0; i<g_list_length(cats); i++)
	{
		section = (char*)g_list_nth_data((GList*)g_list_nth_data(cats, i), 0);
		ptr = g_list_display((GList*)g_list_nth_data(cats, i), " ");
		if(ptr!=NULL)
		{
			fw_end_dialog();
#ifdef FINAL
			if (system(g_strdup_printf("pacman -S -r ./ --noconfirm %s", ptr))
#else
			if (system(g_strdup_printf("echo %s: pacman -S -r ./ --noconfirm %s && sleep 3", section, ptr))
#endif
				!= 0)
			{
				printf(_("Errors occured while installing "
				"selected packages from the %s section.\n"
				"Press ENTER to continue..."), section);
				fflush(stdout);
				getchar();
				fw_init_dialog();
				if(exit_fail())
					exit_perform();
			}
			else
				fw_init_dialog();
		}
		FREE(ptr);
	}
	return(0);
}

int cat_isin(GList *list, char *cat)
{
	int i;
	for (i=0; i<g_list_length(list); i++)
	{
		if(!strcmp((char*)g_list_nth_data((GList*)g_list_nth_data(list, i), 0), cat))
			return(i);
	}
	return(-1);
}

GList *mergecats(GList *allowed, GList *all)
{
	GList *final = NULL;
	char *section;
	int i, pos;

	for (i=0; i<g_list_length(allowed); i++)
	{
		section = g_list_nth_data(allowed, i);
		pos = cat_isin(all, section);
		final = g_list_append(final, (GList*)g_list_nth_data(all, pos));
	}
	return(final);
}

int ask_cdchange(void)
{
#ifdef DIALOG
	int ret;
	ret = dialog_yesno(_("Insert next disc"),
		_("Please insert the next Frugalware install disc and press "
		"ENTER to continue installing packages. If you don't "
		"have more disk, choose NO, and then you can finish up "
		"the installation. Have you inserted the next disk?"), 0, 0);
	if(ret==DLG_EXIT_OK)
		return(1);
	else
		return(0);
#endif
}

int installpkgs(GList *cats, int extra, GList **config)
{
	int i;
	plugin_t *plugin;

	if(!extra)
	{
		installpkgs_forreal(mergecats(genfwcats(1), cats));
		// do we need to change cds?
		if(((char*)data_get(*config, "netinstall")==NULL) &&
			((char*)data_get(*config, "dvd")==NULL))
		{
			eject((char*)data_get(*config, "srcdev"));
			if(ask_cdchange())
			{
				for (i=0; i<g_list_length(plugin_list); i++)
				{
					plugin = g_list_nth_data(plugin_list, i);
					if(!strcmp(plugin->name, "loadsource"))
						plugin->run(config);
				}
			}
			else
				return(0);
		}
		installpkgs_forreal(mergecats(genfwcats(2), cats));
	}
	else
	{
		if((char*)data_get(*config, "netinstall")==NULL)
			symlink(g_strdup_printf("%s/extra/frugalware-%s",
				SOURCEDIR, ARCH), "var/cache/pacman/pkg");
		installpkgs_forreal(cats);
	}
	return(0);
}

int run(GList **config)
{
	installpkgs((GList*)data_get(*config, "packages"), 0, config);
	if(((char*)data_get(*config, "netinstall")!=NULL) ||
			((char*)data_get(*config, "dvd")!=NULL))
		installpkgs((GList*)data_get(*config, "expackages"), 1, config);
	return(0);
}
