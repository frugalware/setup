#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dialog.h>

#include "setup.h"
#include "util.h"

#define VERSIONFILE "/etc/frugalware-release"

char *gen_backtitle(char *section)
{
	char *backtitle;
	char *version = get_version();
	MALLOC(backtitle, 256);

	snprintf(backtitle, 255, "%s - %s %s", section, version, _("Setup"));
	FREE(version);
	return(backtitle);
}

char *get_version(void)
{
	FILE *fp;
	char *version;
	MALLOC(version, 128);

	if ((fp = fopen(VERSIONFILE, "r")) == NULL)
	{
		perror(_("Could not open file for reading"));
		return(NULL);
	}
	fgets(version, 127, fp);
	version[strlen(version)-1]='\0';
	fclose(fp);
	return(version);
}

data_t *data_new(void)
{
	data_t *data=NULL;
	
	data = (data_t*)malloc(sizeof(data_t));
	if(data==NULL)
		return(NULL);
	data->name=NULL;
	data->data=NULL;
	return(data);
}

void *data_get(GList *config, char *title)
{
	int i;
	data_t *data;
	
	for (i=0; i<g_list_length(config); i++)
	{
		data = g_list_nth_data(config, i);
		if(!strcmp(title, data->name))
			return data->data;
	}
	return(NULL);
}

void data_put(GList **config, char *name, void *data)
{
	data_t *dp = data_new();
	dp->name = name;
	dp->data = data;
	(*config) = g_list_append((*config), dp);
}

int exit_confirm(void)
{
	int ret;
	dialog_vars.defaultno=1;
	ret = dialog_yesno(_("Exit from the installer"),
		_("Are you sure you want to exit from the installer?"), 0, 0);
	dialog_vars.defaultno=0;
	if(ret==DLG_EXIT_OK)
		return(1);
	else
		return(0);
}

int exit_perform(void)
{
#ifdef DIALOG
	end_dialog();
#endif

#ifdef FINAL
	system(g_strdup_printf("/sbin/reboot >%s 2>%s", LOGDEV, LOGDEV));
#else
	printf("/sbin/reboot\n");
#endif
	exit(1);
}

char **glist4dialog(GList *list, char *blank)
{
	int i;
	char **array;

	MALLOC(array, 2*g_list_length(list)*sizeof(char*));
	
	for (i=0; i<2*g_list_length(list); i=i+2)
	{
		array[i] = (char*)g_list_nth_data(list, i/2);
		array[i+1] = blank;
	}
	return(array);
}

int fw_menu(const char *title, const char *cprompt, int height, int width,
	int menu_height, int item_no, char **items)
{
	int ret;
	char my_buffer[MAX_LEN + 1] = "";

	while(1)
	{
		dialog_vars.input_result = my_buffer;
		ret = dialog_menu(title, cprompt, height, width, menu_height,
			item_no, items);
		if (ret != DLG_EXIT_CANCEL)
			break;
		if(exit_confirm())
			exit_perform();
	}
	return(0);
}

int fw_init_dialog(void)
{
	FILE *input = stdin;
	dialog_state.output = stderr;
	//char *my_buffer;
	
	//MALLOC(my_buffer, MAX_LEN + 1);
	//*my_buffer='\0';
	init_dialog(input, dialog_state.output);
	//dialog_vars.input_result = my_buffer;
	return(0);
}

int fw_end_dialog(void)
{
	//FREE(dialog_vars.input_result);
	printf("\033[H\033[2J");
	dlg_clear();
	end_dialog();
	return(0);
}
