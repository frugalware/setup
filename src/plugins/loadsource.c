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
#include <libvolume_id.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>

#include <setup.h>
#include <util.h>
#include "common.h"

#define BLKGETSIZE64 _IOR(0x12,114,size_t)

plugin_t plugin =
{
	"loadsource",
	desc,
	25,
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}

char *desc()
{
	return _("Loading the installation source");
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

int is_netinstall(char *path)
{
	struct stat statbuf;
	char *ptr;
	int ret;

	ptr = g_strdup_printf("%s/frugalware-%s", path, ARCH);
	if(!stat(ptr, &statbuf)
		&& S_ISDIR(statbuf.st_mode))
		ret = 0;
	else
		ret = 1;
	FREE(ptr);
	return(ret);
}

static char* get_volume_id(char *device)
{
	int fd;
	struct volume_id *vid = NULL;
	uint64_t size;
	const char *label;
	char *ret;
	char path[PATH_MAX];

	snprintf(path, PATH_MAX, "/dev/%s", device);

	fd = open(path, O_RDONLY);
	if(fd<0)
		return NULL;
	vid = volume_id_open_fd(fd);
	ioctl(fd, BLKGETSIZE64, &size);
	volume_id_probe_all(vid, 0, size);
	volume_id_get_label(vid, &label);
	ret = strdup(label);
	volume_id_close(vid);
	return ret;
}

int run(GList **config)
{
	GList *drives=NULL;
	int i;
	int found = 0;
	char *ptr;

	umount_if_needed(SOURCEDIR);

	dialog_vars.backtitle=gen_backtitle(_("Selecting source media"));
	dlg_put_backtitle();
	dlg_clear();
	dialog_msgbox(_("Scanning"), _("Scanning for a CD/DVD drive containing "
		"a Frugalware install disc..."), 0, 0, 0);
	
	drives = grep_drives("/proc/sys/dev/cdrom/info");
	for (i=0; i<g_list_length(drives); i++)
	{
		ptr = get_volume_id((char*)g_list_nth_data(drives, i));
		if(ptr && !strcmp(ptr, "Frugalware Install"))
		{
			LOG("install medium found in %s", (char*)g_list_nth_data(drives, i));
			FREE(ptr);
			ptr = g_strdup_printf("mount -o ro -t iso9660 /dev/%s %s", (char*)g_list_nth_data(drives, i),
					SOURCEDIR);
			fw_system(ptr);
			data_put(config, "srcdev", (char*)g_list_nth_data(drives, i));
			dlg_put_backtitle();
			dialog_msgbox(_("CD/DVD drive found"), g_strdup_printf(_("A Frugalware install disc was found in device /dev/%s."), (char*)g_list_nth_data(drives, i)), 0, 0, 0);
			if(is_netinstall(SOURCEDIR))
			{
				data_put(config, "netinstall", "");
				LOG("install medium contains no packages, performing a network installation");
			}
			else
				LOG("install medium contains packages, performing an offline installation");
			found = 1;
			break;
		}
		else
			LOG("skipping non-install medium in %s", (char*)g_list_nth_data(drives, i));
		FREE(ptr);
	}
	if(!found)
	{
		LOG("no package database found, performing a network installation");
		data_put(config, "netinstall", "");
	}
	// disable caching for cds
	if((char*)data_get(*config, "netinstall")==NULL)
	{
		char *pacbindir = g_strdup_printf("%s/frugalware-%s", SOURCEDIR, ARCH);
		disable_cache(pacbindir);
		FREE(pacbindir);
	}
	if(data_get(*config, "srcdev")==NULL)
	{
		LOG("no cd/dvd drive found, this is normal if you are running setup from a pendrive or in an emulator");
	}
	return(0);
}
