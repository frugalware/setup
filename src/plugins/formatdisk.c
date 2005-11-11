/*
 *  formatdisk.c for Frugalware setup
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
#include <parted/parted.h>
#include <string.h>
#include <limits.h>

#include <setup.h>
#include <util.h>
#include "common.h"

plugin_t plugin =
{
	"formatswap",
	40,
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}

GList *parts=NULL;
GList *partschk=NULL;

int partdetails(PedPartition *part)
{
	char *pname, *ptype, *ptr;
	PedFileSystemType *type;

	if ((type = ped_file_system_probe (&part->geom)) == NULL)
		// prevent a nice segfault
		ptype = strdup("unknown");
	else
		ptype = (char*)type->name;

	pname = ped_partition_get_path(part);

	// remove the unnecessary "p-1" suffix from sw raid device names
	if((ptr = strstr(pname, "p-1"))!=NULL)
		*ptr='\0';

	// for dialog menus
	parts = g_list_append(parts, pname);
	parts = g_list_append(parts, g_strdup_printf("%dGB\t%s", (int)part->geom.length/1953125, ptype));
	// for dialog checklists
	partschk = g_list_append(partschk, pname);
	partschk = g_list_append(partschk, g_strdup_printf("%dGB %s", (int)part->geom.length/1953125, ptype));
	partschk = g_list_append(partschk, strdup("Off"));

	return(0);
}

int listparts(PedDisk *disk)
{
	PedPartition *part = NULL;
	PedPartition *extpart = NULL;

	if(ped_disk_next_partition(disk, NULL)==NULL)
		// no partition detected
		return(1);
	for(part=ped_disk_next_partition(disk, NULL);
		part!=NULL;part=part->next)
	{
		if((part->num>0) && (part->type != PED_PARTITION_EXTENDED) && !ped_partition_get_flag(part, PED_PARTITION_RAID))
			partdetails(part);
		if(part->type == PED_PARTITION_EXTENDED)
			for(extpart=part->part_list;
				extpart!=NULL;extpart=extpart->next)
				if(extpart->num>0 && !ped_partition_get_flag(extpart, PED_PARTITION_RAID))
					partdetails(extpart);
	}
	return(0);
}

int detect_raids()
{
	FILE *fp;
	char *line, *ptr;
	PedDevice *dev = NULL;
	PedDisk *disk = NULL;
	PedPartition *part = NULL;

	if ((fp = fopen("/proc/mdstat", "r"))== NULL)
	{
		perror("Could not open output file for writing");
		return(1);
	}
	MALLOC(line, PATH_MAX);
	while(!feof(fp))
	{
		if(fgets(line, PATH_MAX, fp) == NULL)
			break;
		if(strstr(line, "md")==line)
		{
			ptr = line;
			while(*ptr!=' ')
				ptr++;
			*ptr='\0';
			dev = ped_device_get(g_strdup_printf("/dev/%s", line));
			disk = ped_disk_new_fresh(dev, ped_disk_type_get ("loop"));
			part=ped_disk_next_partition(disk, NULL);
			partdetails(part);
		}
	}
	FREE(line);
	fclose(fp);
	return(0);
}

int run(GList **config)
{
	PedDevice *dev = NULL;
	PedDisk *disk = NULL;
	char **array;
	char **arraychk;
	GList *partlist;
	int i;

	ped_device_probe_all();

	if(ped_device_get_next(NULL)==NULL)
		// no disk detected already handled before, no need to inform
		// the user about this
		return(1);

	for(dev=ped_device_get_next(NULL);dev!=NULL;dev=dev->next)
	{
		if(dev->read_only)
			// we don't want to partition cds ;-)
			continue;
		disk = ped_disk_new(dev);
		listparts(disk);
	}

	// software raids
	detect_raids();

	// select swap partitions to use
	array = parts2dialog(parts);
	arraychk = parts2dialog(partschk);

	dialog_vars.backtitle=gen_backtitle(_("Setting up swap space"));
	dlg_put_backtitle();
	partlist = fw_checklist(_("Setting up swap partitions"),
		_("Please select which swap partitions do you want Frugalware "
		"to use:"), 0, 0, 0, g_list_length(partschk)/3, arraychk,
		FLAG_CHECK);
	return(0);
	//never reached, TODO: remove this block
	for (i=0; i<g_list_length(partlist); i++)
		dialog_msgbox("title", g_strdup_printf("new item: %s\n", (char*)g_list_nth_data(partlist, i)), 0, 0, 1);
	/*dialog_vars.backtitle=gen_backtitle(_("Setting up root the partition"));
	dlg_put_backtitle();
	fw_menu(_("Select the Linux installation partition"),
		_("Please select a partition from the following list to use "
		"for your root (/) partition. The following ones "
		"are available:"), 0, 0, 0, g_list_length(parts)/2, array);*/

	// sample: simple msgbox
	//dialog_msgbox("title", "content", 0, 0, 1);
	
	// sample: gets the string titled "stuff" from the config list
	//printf("%s\n", (char*)data_get(*config, "stuff"));
	
	// sample: adds a "content" string titled "stuff" to the config list
	//data_put(config, "stuff", "content");
}
