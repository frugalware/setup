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
#include <sys/stat.h>
#include "common.h"

static
plugin_t plugin =
{
	"checkdisk",
	desc,
	41,
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
char *get_root_device(const char *root,char *s,size_t n)
{
	FILE *file;
	regex_t re;
	char line[LINE_MAX], *dev, *dir, *p;
	regmatch_t mat;

	file = fopen("/proc/mounts","rb");

	if(!file)
		return 0;

	if(regcomp(&re,"^/dev/(sd[a-z]|hd[a-z]|vd[a-z]|md[0-9]+)",REG_EXTENDED))
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

		if(strcmp(dir,root))
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
char *get_sysfs_contents(const char *path)
{
	FILE *file;
	char line[LINE_MAX], *s;

	file = fopen(path,"rb");

	if(!file)
		return 0;

	if(!fgets(line,sizeof line,file))
	{
		fclose(file);

		return 0;
	}

	s = strchr(line,'\n');

	if(s)
		*s = 0;

	s = strdup(line);

	fclose(file);

	return s;
}

static
void free_device_list(char **devices)
{
	char **p = devices;

	while(*p)
		free(*p++);

	free(devices);
}

static
char **get_device_list(const char *root)
{
	char **devices = 0;

	if(!strncmp(root,"/dev/md",7))
	{
		int disks_count, i;
		char path[PATH_MAX], *level = 0, *disks = 0;
		struct stat st;

		snprintf(path,sizeof path,"/sys/block/%s/md/level",root+5);

		level = get_sysfs_contents(path);

		snprintf(path,sizeof path,"/sys/block/%s/md/raid_disks",root+5);

		disks = get_sysfs_contents(path);

		if(!level || !disks || strcmp(level,"raid1") || atoi(disks) < 2)
		{
			free(level);

			free(disks);

			return 0;
		}

		disks_count = atoi(disks);

		devices = malloc(sizeof(char *) * (disks_count + 1));

		for( i = 0 ; i < disks_count ; ++i )
		{
			char buf[PATH_MAX], dev[PATH_MAX];
			ssize_t n;

			snprintf(path,sizeof path,"/sys/block/%s/md/rd%d",root+5,i);

			n = readlink(path,buf,sizeof buf);

			if(n >= 0 && n < (ssize_t) sizeof buf)
				buf[n] = 0;

			if(
				n == -1                                       ||
				n == (ssize_t) sizeof buf                     ||
				strncmp(buf,"dev-",4)                         ||
				snprintf(dev,sizeof dev,"/dev/%3s",buf+4) < 0 ||
				stat(dev,&st)
			)
			{
				free(level);

				free(disks);

				devices[i] = 0;

				free_device_list(devices);

				return 0;
			}

			devices[i] = strdup(dev);
		}

		devices[i] = 0;

		free(level);

		free(disks);
	}
	else
	{
		devices = malloc(sizeof(char *) * 2);

		devices[0] = strdup(root);

		devices[1] = 0;
	}

	return devices;
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
	char dev[PATH_MAX], **devices = 0, **p;
	int pass, ignore;

	if(!get_root_device("/mnt/target",dev,sizeof dev))
		return -1;

	devices = get_device_list(dev);

	if(!devices)
		return -1;

	for( p = devices ; *p ; ++p )
	{
		pass = starts_on_sector_2048(*p);

		if(!pass)
			break;
	}

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
		{
			free_device_list(devices);
			return -1;
		}
	}

	for( p = devices ; *p ; ++p )
	{
		pass = setup_for_mbr_grub(*p);

		if(!pass)
			break;
	}

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
		{
			free_device_list(devices);
			return -1;
		}
	}

	free_device_list(devices);

	return 0;
}
