/*
 *  loadsource.c for Frugalware setup
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
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <ctype.h>
#ifdef DIALOG
    #include <dialog.h>
#endif
#ifdef GTK
    #include <gtk/gtk.h>
#endif
#include <sys/stat.h>

#include <setup.h>
#include <util.h>
#include "common.h"

plugin_t plugin =
{
	"loadsource",
	25,
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}

GList *extract_drives(char *line)
{
	char *p, *s;
	GList *drives=NULL;

	for (p=line+12;p!='\0';p=strstr(p+1, "\t"))
	{
		s = strdup(p+1);
		drives = g_list_append(drives, s);
		while(!isspace(*s))
			s++;
		*s='\0';
	}
	return(drives);
}

GList *grep_drives(char *file)
{
	FILE *fp;
	char line[PATH_MAX];

	if ((fp = fopen(file, "r")) == NULL)
	{
		perror(_("Could not open output file for writing"));
		return(NULL);
	}
	while(!feof(fp))
	{
		if(fgets(line, 256, fp) == NULL)
			break;
		if(line == strstr(line, "drive name:"))
		{
			return(extract_drives(line));
		}
	}
	fclose(fp);
	return(NULL);
}

int is_dvd(char *path)
{
	struct stat statbuf;
	if((!stat(g_strdup_printf("%s/frugalware-%s", path, ARCH), &statbuf)
		&& S_ISDIR(statbuf.st_mode)) &&
		(!stat(g_strdup_printf("%s/extra/frugalware-%s", path, ARCH),
		&statbuf) && S_ISDIR(statbuf.st_mode)))
		return(1);
	else
		return(0);
}

int is_netinstall(char *path)
{
	struct stat statbuf;
	if(!((!stat(g_strdup_printf("%s/frugalware-%s", path, ARCH), &statbuf)
		&& S_ISDIR(statbuf.st_mode)) ||
		(!stat(g_strdup_printf("%s/extra/frugalware-%s", path, ARCH),
		&statbuf) && S_ISDIR(statbuf.st_mode))))
		return(1);
	else
		return(0);
}

int run(GList **config)
{
	GList *drives=NULL;
	int i;

	umount_if_needed(SOURCEDIR);

	dialog_vars.backtitle=gen_backtitle(_("Selecting source media"));
	dlg_put_backtitle();
	dlg_clear();
	dialog_msgbox(_("Scanning"), _("Scanning for a CD/DVD drive containing "
		"a Frugalware install disc..."), 0, 0, 0);
	
	drives = grep_drives("/proc/sys/dev/cdrom/info");
	for (i=0; i<g_list_length(drives); i++)
	{
		if (!system(g_strdup_printf("mount -o ro -t iso9660 /dev/%s %s >%s 2>%s", (char*)g_list_nth_data(drives, i), SOURCEDIR, LOGDEV, LOGDEV)))
		{
			data_put(config, "srcdev", (char*)g_list_nth_data(drives, i));
			dlg_put_backtitle();
			dialog_msgbox(_("CD/DVD drive found"), g_strdup_printf(_("A Frugalware install disc was found in device /dev/%s."), (char*)g_list_nth_data(drives, i)), 0, 0, 0);
			if(is_dvd(SOURCEDIR))
				data_put(config, "dvd", "");
			if(is_netinstall(SOURCEDIR))
				data_put(config, "netinstall", "");
			break;
		}
	}
	// disable caching for cd/dvd
	if((char*)data_get(*config, "netinstall")==NULL)
	{
		char *pacbindir = g_strdup_printf("%s/frugalware-%s", SOURCEDIR, ARCH);
		char *pacexbindir = g_strdup_printf("%s/extra/frugalware-%s", SOURCEDIR, ARCH);
		disable_cache(pacbindir);
		disable_cache(pacexbindir);
		FREE(pacbindir);
		FREE(pacexbindir);
	}
	if(data_get(*config, "srcdev")==NULL)
	{
		dialog_msgbox(_("CD/DVD drive not found"), _("Sorry, no Frugalware install disc was found in any of your drives. Press ENTER to reboot."), 0, 0, 1);
		exit_perform();
	}
	return(0);
}
