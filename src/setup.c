/*
 *  setup.c for Frugalware setup
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
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <glib.h>
#include <dlfcn.h>
#ifdef DIALOG
#include <dialog.h>
#endif
#ifdef GTK
#include <gtk/gtk.h>
#endif

#include "setup.h"
#include "util.h"

GList *plugin_list;

int sort_plugins(gconstpointer a, gconstpointer b)
{
	const plugin_t *pa = a;
	const plugin_t *pb = b;
	return (memcmp(&pa->priority, &pb->priority, sizeof(int)));
}

int add_plugin(char *filename)
{
	void *handle;
	void *(*infop) (void);

	if ((handle = dlopen(filename, RTLD_NOW)) == NULL)
	{
		fprintf(stderr, "%s\n", dlerror());
		return(1);
	}
	
	if ((infop = dlsym(handle, "info")) == NULL)
	{
		fprintf(stderr, "%s\n", dlerror());
		return(1);
	}
	plugin_t *plugin = infop();
	plugin->handle = handle;
	plugin_list = g_list_append(plugin_list, plugin);

	return(0);
}

int init_plugins(char *dirname)
{
	char *filename, *ext;
	DIR *dir;
	struct dirent *ent;
	struct stat statbuf;

	dir = opendir(dirname);
	if (!dir)
	{
		perror(dirname);
		return(1);
	}
	while ((ent = readdir(dir)) != NULL)
	{
		filename = g_strdup_printf("%s/%s", dirname, ent->d_name);
		if (!stat(filename, &statbuf) && S_ISREG(statbuf.st_mode) &&
				(ext = strrchr(ent->d_name, '.')) != NULL)
			if (!strcmp(ext, SHARED_LIB_EXT))
				add_plugin(filename);
	}
	closedir(dir);
	return(0);
}

int cleanup_plugins()
{
	int i;
	plugin_t *plugin;

	for (i=0; i<g_list_length(plugin_list); i++)
	{
		plugin = g_list_nth_data(plugin_list, i);
		dlclose(plugin->handle);
	}
	return(0);
}

int main(int argc, char *argv[])
{
	int i;
	plugin_t *plugin;
	GList *config=NULL;

	init_plugins(PLUGDIR);

#ifdef DIALOG
	fw_init_dialog();
#endif
#ifdef GTK
	fw_init_gtk();
	gtk_draw_framework();
#endif

	plugin_list = g_list_sort(plugin_list, sort_plugins);
#ifdef DIALOG
	for (i=0; i<g_list_length(plugin_list); i++)
	{
		plugin = g_list_nth_data(plugin_list, i);
		plugin->run(&config);
		if(dialog_vars.input_result)
			dialog_vars.input_result[0]='\0';
	}

	fw_end_dialog();
#endif

#ifdef GTK
	plugin = g_list_nth_data(plugin_list, 0);
	plugin->run(&config);
	gtk_main();
#endif
	cleanup_plugins();
	return(0);
}
