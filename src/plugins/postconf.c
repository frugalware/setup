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
#include <sys/stat.h>

#include <setup.h>
#include <util.h>
#include "common.h"

plugin_t plugin =
{
	"postconf",
	"Configuring the installed system",
	55,
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}

int has_rootpw(char *fn)
{
	FILE *fp;
	char line[256], *ptr;

	if((fp=fopen(fn, "r"))==NULL)
	{
		perror("Could not open output file for reading");
		return(1);
	}
	while(!feof(fp))
	{
		if(fgets(line, 255, fp) == NULL)
			break;
		if(strstr(line, "root")==line)
		{
			ptr = strchr(line, ':');
			fclose(fp);
			if(*++ptr==':')
				return(0);
			else
				return(1);
		}
	}
	return(0);
}

int confirm_rootpw()
{
	int ret;

	ret = dialog_yesno(_("No root password detected"),
		_("There is currently no password set on the system "
		"administrator account (root). It is recommended that you set "
		"one now so that it is active the first time the machine is "
		"rebooted. This is especially important if your machine is on "
		"an internet connected LAN. Would you like to set a root "
		"password?"), 0, 0);
	if(ret==DLG_EXIT_OK)
		return(1);
	else
		return(0);
}

int has_user(char *fn)
{
	FILE *fp;
	char line[256];

	if((fp=fopen(fn, "r"))==NULL)
	{
		perror("Could not open output file for reading");
		return(1);
	}
	while(!feof(fp))
	{
		if(fgets(line, 255, fp) == NULL)
			break;
		if(strstr(line, ":100:")!=NULL)
				return(1);
	}
	return(0);
}

int confirm_user()
{
	int ret;

	ret = dialog_yesno(_("No normal user account detected"),
		_("There is currently no non-root user account configured. "
		"It is strongly recommended to create one. Would you like to "
		"create a normal user account now?"), 0, 0);
	if(ret==DLG_EXIT_OK)
		return(1);
	else
		return(0);
}

int append_font(char *fn, char *font)
{
	FILE* fp;

	if((fp=fopen(fn, "a"))==NULL)
	{
		perror("Could not open output file for appending");
		return(1);
	}
	fprintf(fp, "font=%s\n", font);
	fclose(fp);
	return(0);
}

int run(GList **config)
{
	char *ptr;
	struct stat buf;

	dialog_vars.backtitle=gen_backtitle(_("Post-install configuration"));

	// TODO: somehow /proc gets mounted sometimes
	// this is just a workaround, we should find and fix the affected pkgs
	fw_system(g_strdup_printf("umount %s/proc", TARGETDIR));

	fw_end_dialog();
	system("chroot ./ /sbin/grubconfig");
	fw_init_dialog();

	dlg_put_backtitle();
	dlg_clear();
	dialog_msgbox(_("Configuring kernel modules"),
		_("Updating module dependencies..."), 0, 0, 0);
	fw_system("chroot ./ /sbin/depmod -a");

	// newer shadow requires /dev/stdin :/
	ptr = g_strdup_printf("mount /dev -o bind %s/dev", TARGETDIR);
	system(ptr);
	free(ptr);
	while(!has_rootpw("etc/shadow") && confirm_rootpw())
	{
		fw_end_dialog();
		system("chroot ./ /usr/bin/passwd root");
		fw_init_dialog();
	}

	while(!has_user("etc/passwd") && confirm_user())
	{
		fw_end_dialog();
		system("chroot ./ /usr/sbin/adduser");
		fw_init_dialog();
	}
	ptr = g_strdup_printf("umount %s/dev", TARGETDIR);
	system(ptr);
	free(ptr);

	if((ptr = (char*)data_get(*config, "font")))
		append_font("etc/sysconfig/font", ptr);

	fw_end_dialog();
	system("chroot ./ /sbin/netconfig");
	system("chroot ./ /sbin/timeconfig");
	system("chroot ./ /sbin/mouseconfig");

	if(!stat("usr/bin/X", &buf))
	{
		system("chroot ./ su -c /sbin/xconfig");
		system("chroot ./ /sbin/xwmconfig --silent");
	}
	fw_init_dialog();

	return(0);
}
