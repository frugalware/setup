/*
 *  postconf.c for Frugalware setup
 * 
 *  Copyright (c) 2005, 2006, 2007, 2008 by Miklos Vajna <vmiklos@frugalware.org>
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
	desc,
	55,
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}

char *desc()
{
	return _("Configuring the installed system");
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

int change_pw(char *user, char **passbuf)
{
	char *pass=NULL, *ptr;
	int ret;

	while(1)
	{
		ptr = g_strdup_printf(_("Please enter a password for %s"), user);
		ret = fw_inputbox(_("Password"), ptr, 0, 0, "", 1);
		FREE(ptr);
		if(ret != -1)
		{
			pass = strdup(dialog_vars.input_result);
			ptr = g_strdup_printf(_("Please re-enter your password for %s"), user);
			ret = fw_inputbox(_("Password"), ptr, 0, 0, "", 1);
			FREE(ptr);
			if(ret != -1)
			{
				if(strcmp(pass, dialog_vars.input_result))
				{
					FREE(pass);
					if(dialog_yesno(_("Passwords don't match"),
							_("Sorry, the passwords do not match. Try again?"), 0, 0) != DLG_EXIT_OK)
						return(-1);
					else
						continue;
				}
				else
				{
					if(!passbuf)
					{
						ptr = g_strdup_printf("echo '%s:%s' |chroot ./ /usr/sbin/chpasswd", user, pass);
						ret = fw_system(ptr);
						FREE(ptr);
						FREE(pass);
						return(ret);
					}
					else
					{
						*passbuf = pass;
						return(0);
					}
				}
			}
		}
 
		FREE(pass);
		ptr = g_strdup_printf(_("Are you sure you want to skip setting the password for %s?"), user);
		ret = dialog_yesno(_("Ignore setting password"), ptr, 0, 0);
		FREE(ptr);
		if(ret != DLG_EXIT_OK)
			return(-1);
	}
}

int add_user()
{
	char *login, *fn, *pass, *ptr;
	int ret;
	struct stat buf;
	char home[PATH_MAX];

	if(fw_inputbox(_("Enter login"), _("Please enter the login name you should like to create:"), 0, 0, "", 0) == -1)
		return(-1);
	login = strdup(dialog_vars.input_result);
	if(fw_inputbox(_("Enter full name"), _("Please enter the full name of the user you would like to create:"), 0, 0, "", 0) == -1)
	{
		FREE(login);
		return(-1);
	}
	fn = strdup(dialog_vars.input_result);
	if(change_pw(login, &pass) == -1)
	{
		FREE(login);
		FREE(fn);
		return(-1);
	}
	snprintf(home, PATH_MAX, TARGETDIR "/home/%s", login);
	if(!stat(home, &buf))
	{
		ptr = g_strdup_printf(_("'/home/%s' already exists! "
				"Please change the home directory path to a non-existing directory:"), login);
		if(fw_inputbox(_("Enter home directory"), ptr, 0, 0, "", 0) == -1)
		{
			FREE(ptr);
			return(-1);
		}
		strncpy(home, dialog_vars.input_result, PATH_MAX);
	} else {
		snprintf(home, PATH_MAX, "/home/%s", login);
	}
	ptr = g_strdup_printf("yes ''|chroot ./ /usr/sbin/adduser '%s' \"%s\" '%s' '%s'", login, fn, pass, home);
	ret = fw_system(ptr);
	FREE(ptr);
	FREE(login);
	FREE(fn);
	FREE(pass);
	return(ret);
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
	fprintf(fp, "# specify the console font\n");
	fprintf(fp, "FONT=%s\n", font);
	fclose(fp);
	return(0);
}

void write_locale(GList **config)
{
	char *op, *np;

	op = (char*)data_get(*config, "lang.sh");
	np = g_strdup_printf("%s/%s", TARGETDIR, "/etc/locale.conf");
	copyfile(op, np);
	unlink(op);
	chmod(np, S_IRUSR|S_IWUSR|S_IXUSR|S_IRGRP|S_IXGRP|S_IROTH|S_IXOTH);
	FREE(np);
}

int run(GList **config)
{
	char *ptr;
	//struct stat buf;

	write_locale(config);

	dialog_vars.backtitle=gen_backtitle(_("Post-install configuration"));

	// this prevents mounting and umounting dev/proc/sys for each config
	// tool
	ptr = g_strdup_printf("mount none -t devtmpfs %s/dev", TARGETDIR);
	fw_system(ptr);
	free(ptr);
	ptr = g_strdup_printf("mount /proc -o bind %s/proc", TARGETDIR);
	fw_system(ptr);
	free(ptr);
	ptr = g_strdup_printf("mount /sys -o bind %s/sys", TARGETDIR);
	fw_system(ptr);
	free(ptr);

	fw_end_dialog();
#ifndef ARCH_PPC
	fw_system_interactive("chroot ./ /sbin/grubconfig");
#else
	fw_system_interactive("chroot ./ /sbin/yabootcfg");
#endif
	fw_init_dialog();

	dlg_put_backtitle();
	dlg_clear();
//	dialog_msgbox(_("Configuring kernel modules"),
//		_("Updating module dependencies..."), 0, 0, 0);
//	fw_system("chroot ./ /sbin/depmod -a");

	while(!has_rootpw("etc/shadow") && confirm_rootpw())
	{
		change_pw("root", NULL);
	}

	while(!has_user("etc/passwd") && confirm_user())
	{
		add_user();
	}

	char* op = (char*)data_get(*config, "vconsole.conf");
	char* np = g_strdup_printf("%s/%s", TARGETDIR, "/etc/vconsole.conf");
	copyfile(op, np);
	unlink(op);
	chmod (np, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	FREE(np);
	if((ptr = (char*)data_get(*config, "font")))
		append_font("etc/vconsole.conf", ptr);

	fw_end_dialog();
	fw_system_interactive("chroot ./ /sbin/netconfig");
	fw_system_interactive("chroot ./ /sbin/timeconfig");
//	fw_system_interactive("chroot ./ /sbin/mouseconfig");

//	if(!stat("usr/bin/X", &buf))
//	{
//		fw_system_interactive("chroot ./ /sbin/xwmconfig --silent");
//	}
	ptr = g_strdup_printf("umount %s/sys", TARGETDIR);
	fw_system(ptr);
	free(ptr);
	ptr = g_strdup_printf("umount %s/proc", TARGETDIR);
	fw_system(ptr);
	free(ptr);
	ptr = g_strdup_printf("umount %s/dev", TARGETDIR);
	fw_system(ptr);
	free(ptr);

	fw_init_dialog();

	return(0);
}
