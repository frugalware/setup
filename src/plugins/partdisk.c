/*
 *  partdisk.c for Frugalware setup
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
#ifdef DIALOG
    #include <dialog.h>
#endif
#ifdef GTK
    #include <gtk/gtk.h>
#endif
#include <parted/parted.h>

#include <setup.h>
#include <util.h>
#include "common.h"

plugin_t plugin =
{
	"partdisk",
	35,
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
		if(dev->read_only)
			// we don't want to partition cds ;-)
			continue;
		devs = g_list_append(devs, dev->path);
		devs = g_list_append(devs, g_strdup_printf("%dGB\t%s", (int)dev->length/1953125, dev->model));
	}

	return(devs);
}

char *selpartsw()
{
	int swnum=2;
	char *sws[] =
	{
		"cfdisk", _("User friendly (curses based) version of fdisk"),
		"fdisk", _("The traditional partitioning program for Linux")
	};
	
	dialog_vars.backtitle=gen_backtitle(_("Creating partitions"));
	dlg_put_backtitle();
	dlg_clear();
	fw_menu(_("Select partitioning program"),
		_("Select the program you want to use for partitioning:"),
		0, 0, 0, swnum, sws);

	return(dialog_vars.input_result);
}

int raid_confirm(void)
{
#ifdef DIALOG
	int ret;
	dialog_vars.defaultno=1;
	ret = dialog_yesno(_("Want to create RAID?"),
		_("Do you want to create software raid partitions during "
		"partitioning?"), 0, 0);
	dialog_vars.defaultno=0;
	if(ret==DLG_EXIT_OK)
		return(1);
	else
		return(0);
#endif
}

int run(GList **config)
{
	GList *lp;
	char **array;
	char path[PATH_MAX];
	char partsw[PATH_MAX];
	int ret;
	char my_buffer[MAX_LEN + 1] = "";
	int wantraid;

	if((lp = listparts())==NULL)
	{
		dialog_msgbox(_("Disk drive not found"), _("Sorry, no hard disk drives were found in this computer. Press ENTER to reboot."), 0, 0, 1);
		exit_perform();
	}

	array = glist2dialog(lp);

	dialog_vars.backtitle=gen_backtitle(_("Creating partitions"));
	dlg_put_backtitle();
	wantraid = raid_confirm();

	while(1)
	{
		if(wantraid)
		{
			dialog_vars.extra_button=1;
			dialog_vars.extra_label=strdup(_("Create RAID"));
		}

		dlg_put_backtitle();
		dlg_clear();
		dialog_vars.cancel_label = strdup(_("Continue"));
		dialog_vars.input_result = my_buffer;
		dialog_vars.input_result[0]='\0';
		ret = dialog_menu(_("Select a hard disk to partition"),
		_("Please select a hard disk to partition. The following ones "
		"are available:"), 0, 0, 0, g_list_length(lp)/2, array);
		dialog_vars.cancel_label = '\0';
		if(wantraid)
		{
			dialog_vars.extra_button=0;
			dialog_vars.extra_label[0]='\0';
		}
		if (ret == DLG_EXIT_OK)
		{
			strcpy(path, dialog_vars.input_result);
			dialog_vars.input_result[0]='\0';
			strcpy(partsw, selpartsw());
			fw_end_dialog();
			system(g_strdup_printf("%s %s", partsw, path));
			fw_init_dialog();
		}
		else if (ret == DLG_EXIT_EXTRA)
		{
			system(RAIDCONFIGSCRIPT);
		}
		else
			break;
	}

	FREE(array);
	return(0);
}
