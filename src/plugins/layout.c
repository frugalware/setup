#include <stdio.h>
#include <dialog.h>
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
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
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
	int ret;
	char **array;
	char *fn;
	FILE* fp;
	
	find("/usr/share/kbd/keymaps/i386");
	array = glist4dialog(layoutl, "");
	
	dialog_vars.backtitle=gen_backtitle(_("Configuring the keyboard"));
	dlg_put_backtitle();
	dlg_clear();
	while(1)
	{
		ret = dialog_menu(_("Keyboard map selection"),
		_("You may select one of the following keyboard maps. If you "
		"do not select a keyboard map, 'qwerty/us.map.gz' (the US "
		"keyboard map) is the default. Use the UP/DOWN arrow keys and "
		"PageUp/PageDown to scroll through the whole list of choices."),
		0, 0, 0, g_list_length(layoutl), array);
		if (ret != DLG_EXIT_CANCEL)
			break;
		if(exit_confirm())
			exit_perform();
	}

	FREE(array);
	// drop .map.gz
	dialog_vars.input_result[strlen(dialog_vars.input_result)-7]='\0';
	
	//TODO: maybe there is a proper system call for this?
	system(g_strdup_printf("loadkeys /usr/share/kbd/keymaps/i386/%s.map.gz >%s 2>%s", dialog_vars.input_result, LOGDEV, LOGDEV));
	
	fn = strdup("/tmp/setup_XXXXXX");
	mkstemp(fn);
	if ((fp = fopen(fn, "w")) == NULL)
	{
		perror(_("Could not open output file for writing"));
		return(1);
	}
	fprintf(fp, "# /etc/sysconfig/keymap\n\n"
		"# sepecify the keyboard map, maps are in "
		"/usr/share/kbd/keymaps\n\n");
	fprintf(fp, "keymap=%s\n", strstr(dialog_vars.input_result, "/")+1);
	fclose(fp);

	data_put(config, "keymap", fn);
	return(0);
}
