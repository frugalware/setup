/*
 *  layout.c for Frugalware setup
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
#ifdef DIALOG
    #include <dialog.h>
#endif
#ifdef GTK
    #include <gtk/gtk.h>
#endif
#include <dirent.h>
#include <sys/stat.h>
#include <glib.h>
#include <string.h>
#include <limits.h>

#include <setup.h>
#include <util.h>
#include "common.h"

GList *layoutl;

plugin_t plugin =
{
	"layout",
	desc,
	15,
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}

char *desc()
{
	return _("Selecting the keyboard map");
}

int sort_layouts(gconstpointer a, gconstpointer b)
{
	const char *pa = a;
	const char *pb = b;
	return (strcmp(pa, pb));
}

int find(char *dirname)
{
	DIR *dir;
	struct dirent *ent;
	struct stat statbuf;
	char *ext, *fn;

	dir = opendir(dirname);
	if (!dir)
	{
		perror(dirname);
		return(1);
	}
	while ((ent = readdir(dir)) != NULL)
	{
		fn = g_strdup_printf("%s/%s", dirname, ent->d_name);
		if(!stat(fn, &statbuf) && S_ISDIR(statbuf.st_mode))
			if(strcmp(ent->d_name, ".") &&
				strcmp(ent->d_name, "..") &&
				strcmp(ent->d_name, "include"))
				find(fn);
		if(!stat(fn, &statbuf) && S_ISREG(statbuf.st_mode) &&
			(ext = strrchr(ent->d_name, '.')) != NULL)
			if (!strcmp(ext, ".gz"))
					layoutl = g_list_append(layoutl, g_strdup_printf("%s%s", strrchr(dirname, '/')+1, fn+strlen(dirname)));
	}
	closedir(dir);
	return(0);
}

int run(GList **config)
{
	char **array;
	char *fn, *ptr;
	FILE* fp;
	int ret;
	
	find("/usr/share/kbd/keymaps/i386");
	layoutl = g_list_sort(layoutl, sort_layouts);
	array = glist4dialog(layoutl, "");
	
	dialog_vars.backtitle=gen_backtitle(_("Configuring the keyboard"));
	dlg_put_backtitle();
	dlg_clear();
	dialog_vars.default_item=strdup("qwerty/us.map.gz");
	if(fw_menu(_("Keyboard map selection"),
		_("You may select one of the following keyboard maps. If you "
		"do not select a keyboard map, 'qwerty/us.map.gz' (the US "
		"keyboard map) is the default. Use the UP/DOWN arrow keys and "
		"PageUp/PageDown to scroll through the whole list of choices."),
		0, 0, 0, g_list_length(layoutl), array) == -1)
		ret = -1;
	FREE(dialog_vars.default_item);
	if(ret == -1)
		return(ret);
	ptr=strdup(dialog_vars.input_result);

	FREE(array);
	// drop .map.gz
	if(strlen(ptr) >= 7)
		ptr[strlen(ptr)-7]='\0';
	
	//TODO: maybe there is a proper system call for this?
	system(g_strdup_printf("loadkeys /usr/share/kbd/keymaps/i386/%s.map.gz >%s 2>%s", ptr, LOGDEV, LOGDEV));
	
	fn = strdup("/tmp/setup_XXXXXX");
	mkstemp(fn);
	if ((fp = fopen(fn, "w")) == NULL)
	{
		perror(_("Could not open output file for writing"));
		return(1);
	}
	fprintf(fp, "# /etc/sysconfig/keymap\n\n"
		"# specify the keyboard map, maps are in "
		"/usr/share/kbd/keymaps\n\n");
	if(strstr(ptr, "/"))
		fprintf(fp, "keymap=%s\n", strstr(ptr, "/")+1);
	FREE(ptr);
	fclose(fp);

	data_put(config, "keymap", fn);
	return(0);
}
