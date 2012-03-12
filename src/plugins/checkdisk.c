/*
 *  skel.c for Frugalware setup
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
#include <regex.h>
#include <setup.h>
#include <util.h>
#include <string.h>
#include <limits.h>
#include "common.h"

static
plugin_t plugin =
{
	"checkdisk",
	desc,
	39,
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}


char *desc()
{
	return _("Basic sanity checks on the hard drive the root partition is on.");
}

static
char *get_root_device(char *s,size_t n)
{
	FILE *file;
	regex_t re;
	char line[LINE_MAX], *dev, *dir, *p;
	regmatch_t mat;

	file = fopen("/proc/mounts","rb");

	if(!file)
		return 0;

	if(regcomp(&re,"^/dev/[hsv]d[a-z]",REG_EXTENDED))
	{
		fclose(file);

		return 0;
	}

	*s = 0;

	while(fgets(line,sizeof line,file))
	{
		dev = strtok_r(line," ",&p);

		if(!dev)
	        continue;

		dir = strtok_r(0," ",&p);

		if(!dir)
			continue;

		if(strcmp(dir,"/mnt/target"))
			continue;

		if(regexec(&re,dev,1,&mat,0))
			continue;

		snprintf(s,n,"%.*s",mat.rm_eo - mat.rm_so,dev);

		break;
	}

	fclose(file);

	regfree(&re);

	return *s ? s : 0;
}

static
char *get_first_device(char *s,size_t n)
{
	FILE *file;
	regex_t re;
	int i = 1;
	char line[LINE_MAX], *p, *dev;

	file = fopen("/proc/partitions","rb");

	if(!file)
		return 0;

	if(regcomp(&re,"^[hsv]d[a-z]$",REG_EXTENDED | REG_NOSUB))
	{
		fclose(file);

		return 0;
	}

	*s = 0;

	while(fgets(line,sizeof line,file))
	{
		if(i < 3)
		{
			++i;

			continue;
		}

		if(!strtok_r(line," \n",&p))
			continue;

		if(!strtok_r(0," \n",&p))
			continue;

		if(!strtok_r(0," \n",&p))
			continue;

		dev = strtok_r(0," \n",&p);

		if(!dev)
			continue;

		if(regexec(&re,dev,0,0,0))
			continue;

		snprintf(s,n,"/dev/%s",dev);

		break;
	}

	fclose(file);

	regfree(&re);

	return *s ? s : 0;
}

static
int user_ignore_warning(const char *prompt,const char *body)
{
	int rv;

	dialog_vars.defaultno = 1;

	dlg_put_backtitle();

	dlg_clear();

	rv = dialog_yesno(prompt,body,0,0);

	dialog_vars.defaultno = 0;

	return rv == DLG_EXIT_OK;
}

static
int starts_on_sector_2048(const char *path)
{
	PedDevice *device = NULL;
	PedDisk *disk = NULL;
	PedPartition *partition;
	int rv = 0;

	device = ped_device_get(path);

	if(!device)
		goto bail;

	disk = ped_disk_new(device);

	if(!disk)
		goto bail;

	if(strcmp(disk->type->name,"msdos") && strcmp(disk->type->name,"gpt"))
	{
		rv = 1;
		goto bail;
	}

	for( partition = ped_disk_next_partition(disk,NULL) ; partition && partition->num == -1 ; partition = ped_disk_next_partition(disk,partition) )
		;

	if(!partition || partition->geom.start != 2048)
		goto bail;

	rv = 1;

	bail:

	if(disk)
		ped_disk_destroy(disk);

	if(device)
		ped_device_destroy(device);

	return rv;
}

static
int setup_for_mbr_grub(const char *path)
{
	PedDevice *device = NULL;
	PedDisk *disk = NULL;
	PedPartition *partition;
	int rv = 0;

	device = ped_device_get(path);

	if(!device)
		goto bail;

	disk = ped_disk_new(device);

	if(!disk)
		goto bail;

	if(strcmp(disk->type->name,"gpt"))
	{
		rv = 1;
		goto bail;
	}

	for( partition = ped_disk_next_partition(disk,NULL) ; partition && partition->num == -1 ; partition = ped_disk_next_partition(disk,partition) )
		;

	if(!partition)
		goto bail;

	for( ; partition && partition->num != -1 && !ped_partition_get_flag(partition,PED_PARTITION_BIOS_GRUB) ; partition = ped_disk_next_partition(disk,partition) )
		;

	if(!partition || partition->num == -1 || ped_unit_get_size(device,PED_UNIT_SECTOR) * partition->geom.length < PED_MEGABYTE_SIZE)
		goto bail;

	rv = 1;

	bail:

	if(disk)
		ped_disk_destroy(disk);

	if(device)
		ped_device_destroy(device);

	return rv;
}

int run(GList **config)
{
	char device[PATH_MAX];
	int pass, ignore;

	if(!get_root_device(device,sizeof device) && !get_first_device(device,sizeof device))
		return -1;

	pass = starts_on_sector_2048(device);

	if(!pass)
	{
		ignore = user_ignore_warning(_("Drive Alignment"),
			_(
			"The first partition of the hard drive your root partition is located on"
			" does not start at sector 2048. This is now required for two reasons. First,"
			" if you are using a new hard drive with 4096 byte sectors, the old alignment"
			" at sector 63 will cause serious performance degradation. Second, GRUB2 requires"
			" this extra space to be able to successfully install itself, if you are using a"
			" MSDOS disk label. GPT disk labels use a different installation method, but the"
			" first reason still applies in those cases. It is advised that you repartition"
			" your disk to meet this requirement. If you choose to proceed without doing so,"
			" you do so at your own peril. Proceed anyway?"
			)
			);
		if(!ignore)
			return -1;
	}

	pass = setup_for_mbr_grub(device);

	if(!pass)
	{
		ignore = user_ignore_warning(_("GPT BIOS Setup"),
			_(
			"You must have a BIOS boot partition on the hard drive your root partition is located on"
			" and it must be greater than or equal to a megabyte in size. Since you have chosen a GPT"
			" disk label for this drive, it is mandatory that you meet these two conditions for GRUB2"
			" to be able to install itself to this drive. It is advised that you repartition your disk"
			" to meet these requirements. If you choose to proceed without doing so, you do so at your"
			" own peril. Proceed anyway?"
			)
			);
		if(!ignore)
			return -1;
	}

	return 0;
}
