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

plugin_t plugin =
{
	"checkdisk",
	desc,
	45,
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
        char line[LINE_MAX], *dev, *dir, *p;
        regex_t re;
        regmatch_t mat;

        file = fopen("/proc/mounts","rb");

        if(!file)
                return 0;

        if(regcomp(&re,"^/dev/[hsv]d[a-z]",REG_EXTENDED))
        {
                fclose(file);

                return 0;
        }

        for( *s = 0 ; fgets(line,sizeof line,file) ; )
        {
                dev = strtok_r(line," ",&p);

                if(!dev)
                        continue;

                dir = strtok_r(0," ",&p);

                if(!dir)
                        continue;

                if(strcmp(dir,"/"))
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



	// sample: gets the string titled "stuff" from the config list
	//printf("%s\n", (char*)data_get(*config, "stuff"));

	// sample: adds a "content" string titled "stuff" to the config list
	//data_put(config, "stuff", "content");



	return(0);
}
