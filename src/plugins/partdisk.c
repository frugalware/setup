#include <stdio.h>
#include <dialog.h>
#include <parted/parted.h>

#include <setup.h>
#include <util.h>
#include "common.h"

plugin_t plugin =
{
	"partdisk",
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}

GList *listparts(void)
{
	GList *devs=NULL;
	PedDevice *dev=NULL;

	ped_device_probe_all();

	if(ped_device_get_next(NULL)==NULL)
		return(NULL);

	for(dev=ped_device_get_next(NULL);dev!=NULL;dev=dev->next)
	{
		devs = g_list_append(devs, dev->path);
		devs = g_list_append(devs, g_strdup_printf("%dGB\t%s", (int)dev->length/1953125, dev->model));
	}

	return(devs);
}

char **parts2dialog(GList *list)
{
	int i;
	char **array;

	MALLOC(array, g_list_length(list)*sizeof(char*));
	for (i=0; i<g_list_length(list); i++)
	{
		array[i] = (char*)g_list_nth_data(list, i);
	}
	return(array);
}

int run(GList **config)
{
	GList *lp;
	char **array;

	if((lp = listparts())==NULL)
	{
		dialog_msgbox(_("Disk drive not found"), _("Sorry, no hard disk drives were found in this box. Press ENTER to reboot."), 0, 0, 1);
		exit_perform();
	}

	array = parts2dialog(lp);
	
	dialog_vars.backtitle=gen_backtitle(_("Creating partitions"));
	dlg_put_backtitle();
	dlg_clear();
	
	fw_menu(_("Select a hard disk to partition"),
		_("Please select a hard disk to partition. The following one "
		"are available:"), 0, 0, 0, g_list_length(lp)/2, array);

	FREE(array);
	return(0);
}
