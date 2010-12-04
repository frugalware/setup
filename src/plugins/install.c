/*
 *  install.c for Frugalware setup
 * 
 *  Copyright (c) 2005, 2008 by Miklos Vajna <vmiklos@frugalware.org>
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
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <setup.h>
#include <util.h>
#include "common.h"

extern GList *plugin_list;

plugin_t plugin =
{
	"install",
	desc,
	50,
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}

char *desc()
{
	return _("Installing the selected packages");
}

int installpkgs_forreal(GList *pkgs, int fast)
{
	char *ptr, *cmd;

	ptr = g_list_display(pkgs, " ");
	if(ptr!=NULL && strlen(ptr))
	{
		fw_end_dialog();
		if(fast)
			cmd = g_strdup_printf("pacman-g2 -S -r ./ --noconfirm -d -f --nointegrity %s", ptr);
		else
			cmd = g_strdup_printf("pacman-g2 -S -r ./ --noconfirm %s", ptr);
		if (fw_system_interactive(cmd) != 0)
		{
			printf(_("Errors occurred while installing "
			"selected packages.\n"
			"Press ENTER to continue..."));
			fflush(stdout);
			getchar();
			fw_init_dialog();
			if(exit_fail())
				return(-1);
		}
		else
			fw_init_dialog();
	}
	FREE(ptr);
	FREE(cmd);
	return(0);
}

int ask_cdchange(void)
{
#ifdef DIALOG
	int ret;
	ret = dialog_yesno(_("Insert next disc"),
		_("Please insert the next Frugalware install disc and press "
		"ENTER to continue installing packages. If you don't "
		"have more discs, choose NO, and then you can finish  "
		"the installation. Have you inserted the next disc?"), 0, 0);
	if(ret==DLG_EXIT_OK)
		return(1);
	else
		return(0);
#endif
}

int installpkgs(GList *pkgs, GList **config)
{
	int i, first=1;

	if((char*)data_get(*config, "netinstall")==NULL)
	while(pkgs)
	{
		GList *list=NULL;
		struct stat buf;
		char *ptr;

		if(first)
			// for the first time the volume is already loaded
			first=0;
		else
		{
			plugin_t *plugin;

			eject((char*)data_get(*config, "srcdev"), SOURCEDIR);
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
		// see what packages can be useful from this volume
		for(i=0;i<g_list_length(pkgs);i++)
		{
			ptr = g_strdup_printf("%s/frugalware-%s/%s-%s.fpm", SOURCEDIR, ARCH,
				(char*)g_list_nth_data(pkgs, i), ARCH);
			if(!stat(ptr, &buf))
				list = g_list_append(list, strdup((char*)g_list_nth_data(pkgs, i)));
			FREE(ptr);
		}
		// remove them from the full list
		for(i=0;i<g_list_length(list);i++)
			pkgs=g_list_strremove(pkgs, (char*)g_list_nth_data(list, i));
		// install them
		if(installpkgs_forreal(list, 1) == -1)
			return(-1);
	}
	else
		if(installpkgs_forreal(pkgs, 0) == -1)
			return(-1);
	return(0);
}

int run(GList **config)
{
	installpkgs((GList*)data_get(*config, "packages"), config);
	// if the source media is cd, we don't need broken symlinks after
	// the installation
	if((char*)data_get(*config, "netinstall")==NULL)
		rmrf("var/cache/pacman-g2/pkg");
	return(0);
}
