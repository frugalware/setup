#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <glib.h>
#include <dlfcn.h>
#ifdef DIALOG
#include <dialog.h>
#endif

#include "setup.h"

GList *plugin_list;

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

int main()
{
	int i;
	plugin_t *plugin;
	GList *config=NULL;
#ifdef DIALOG
	FILE *input = stdin;
	dialog_state.output = stderr;
	char my_buffer[MAX_LEN + 1] = "";
#endif

	init_plugins(PLUGDIR);

#ifdef DIALOG
	init_dialog(input, dialog_state.output);
	dialog_vars.input_result = my_buffer;
#endif

	for (i=0; i<g_list_length(plugin_list); i++)
	{
		plugin = g_list_nth_data(plugin_list, i);
		plugin->run(&config);
#ifdef DIALOG
		dialog_vars.input_result[0]='\0';
#endif
	}

#ifdef DIALOG
	dlg_clear();
	end_dialog();
#endif

	cleanup_plugins();
	return(0);
}
