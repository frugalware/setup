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
	"skel",
	50,
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}

int installpkgs_forreal(GList *pkgs)
{
	char *ptr;

	ptr = g_list_display(pkgs, " ");
	if(ptr!=NULL && strlen(ptr))
	{
		fw_end_dialog();
		if (system(g_strdup_printf("pacman -S -r ./ --noconfirm %s", ptr))
			!= 0)
		{
			printf(_("Errors occured while installing "
			"selected packages.\n"
			"Press ENTER to continue..."));
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
	return(0);
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
		// see what packages can be usefull from this volume
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
		installpkgs_forreal(list);
	}
	else
		installpkgs_forreal(pkgs);
	return(0);
}

int run(GList **config)
{
	/* update /etc/mtab so that statvfs() will detect the free space of the
	 * just mounted partitions */
	copyfile("/proc/mounts", "/etc/mtab");
	installpkgs((GList*)data_get(*config, "packages"), config);
	// if the source media is cd, we don't need broken symlinks after
	// the installtion
	if((char*)data_get(*config, "netinstall")==NULL)
		rmrf("var/cache/pacman/pkg");
	return(0);
}
