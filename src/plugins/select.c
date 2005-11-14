/*
 *  select.c for Frugalware setup
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
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <setup.h>
#include <util.h>
#include "common.h"

plugin_t plugin =
{
	"select",
	45,
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}

char* categorysize(char *category)
{
	FILE *pp;
	char *line, *ptr;

	if ((pp = popen(g_strdup_printf("echo -e 'y\nn'|pacman -Syd %s -r ./", category), "r"))== NULL)
	{
		perror("Could not open pipe for reading");
		return(NULL);
	}
	MALLOC(line, 255);
	while(!feof(pp))
	{
		if(fgets(line, 255, pp) == NULL)
			break;
		if(strstr(line, "Total")==line)
		{
			line = strstr(line, ":");
			line++;
			ptr = strstr(line, "MB");
			ptr+=2;
			*ptr = '\0';
			pclose(pp);
			if(strlen(line)==9)
				return(g_strdup_printf("  %s", line));
			else if(strlen(line)==10)
				return(g_strdup_printf(" %s", line));
			else
				return(line);
		}
	}
	FREE(line);
	pclose(pp);
	return(NULL);
}

char* pkgdir(char *pkg, char *repo)
{
	DIR *dir;
	struct dirent *ent;
	char *targetdir, *dirname=NULL, *name, *ptr;
	int gotit=0;

	targetdir = g_strdup_printf("var/lib/pacman/%s", repo);
	
	dir = opendir(targetdir);
	if (!dir)
	{
		perror(targetdir);
		return(NULL);
	}

	while(!gotit && ((ent = readdir(dir)) != NULL))
	{
		if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
			continue;
		name = strdup(ent->d_name);
		dirname = strdup(ent->d_name);
		if((ptr = rindex(name, '-')))
			*ptr = '\0';
		if((ptr = rindex(name, '-')))
			*ptr = '\0';
		if(!strcmp(name, pkg))
			gotit=1;
		FREE(name);
	}
	closedir(dir);
	FREE(targetdir);
	if(gotit)
	{
		ptr = g_strdup_printf("var/lib/pacman/%s/%s", repo, dirname);
		FREE(dirname);
		return(ptr);
	}
	else
		return(NULL);
}

int pkgsize(char *pkg)
{
	FILE *fp;
	char line[256];
	int ret;

	if ((fp = fopen(g_strdup_printf("%s/desc", pkgdir(pkg, PACCONF)), "r"))
		== NULL)
	{
		perror(_("Could not open output file for writing"));
		return(0);
	}
	while(!feof(fp))
	{
		if(fgets(line, 256, fp) == NULL)
			break;
		if(!strcmp(line, "%CSIZE%\n"))
			fscanf(fp, "%d", &ret);
	}
	fclose(fp);
	return(ret);
}

char* pkgdesc(char *pkg)
{
	FILE *fp;
	char line[256];
	char *ret=NULL, *ptr;

	if ((fp = fopen(g_strdup_printf("%s/desc", pkgdir(pkg, PACCONF)), "r"))
		== NULL)
	{
		perror(_("Could not open output file for writing"));
		return(0);
	}
	while(!feof(fp))
	{
		if(fgets(line, 256, fp) == NULL)
			break;
		if(!strcmp(line, "%DESC%\n"))
		{
			fgets(line, 256, fp);
			ptr = strchr(line, '\n');
			*ptr = '\0';
			ret = strdup(line);
		}
	}
	fclose(fp);
	return(ret);
}

// 0: frugalware; 1: extra
GList *selcat(int repo)
{
	FILE *pp;
	char *line, *ptr;
	GList *catlist=NULL;
	char **arraychk;
	GList *ret;

	// query the list
	dlg_put_backtitle();
	dlg_clear();
	dialog_msgbox(_("Please wait"), _("Searching for categories..."),
		0, 0, 0);
	if ((pp = popen("pacman -Sg -r ./", "r"))== NULL)
	{
		perror("Could not open pipe for reading");
		return(NULL);
	}
	MALLOC(line, 255);
	while(!feof(pp))
	{
		if(fgets(line, 255, pp) == NULL)
			break;
		ptr = strchr(line, '\n');
		*ptr = '\0';
		if(!repo)
		{
			if((index(line, '-')==NULL) && strcmp(line, "core"))
			{
				catlist = g_list_append(catlist, strdup(line));
				catlist = g_list_append(catlist,
					categorysize(line));
				catlist = g_list_append(catlist, strdup("On"));
			}
		}
		else
		{
			if((index(line, '-')!=NULL) &&
				(strstr(line, "-extra")!=NULL))
			{
				catlist = g_list_append(catlist, strdup(line));
				catlist = g_list_append(catlist,
					categorysize(line));
				if(strcmp(line, "locale-extra"))
					catlist = g_list_append(catlist,
						strdup("Off"));
				else
					catlist = g_list_append(catlist,
						strdup("On"));
			}
		}
	}
	FREE(line);
	pclose(pp);

	// now display the list
	arraychk = glist2dialog(catlist);

	dlg_put_backtitle();
	dlg_clear();
	ret = fw_checklist(_("Selecting categories"),
		_("Please select which categories to install:"),
		0, 0, 0, g_list_length(catlist)/3, arraychk,
		FLAG_CHECK);
	return(ret);
}

int run(GList **config)
{
	int i;
	GList *list;

	dialog_vars.backtitle=gen_backtitle(_("Selecting packages"));
	chdir(TARGETDIR);
	//dialog_msgbox("bash", pkgdesc("bash"), 0, 0, 1);
	list = selcat(1);
	fw_end_dialog(); ///
	for (i=0; i<g_list_length(list); i++)
		printf("new item: %s\n", (char*)g_list_nth_data(list, i));
	fw_init_dialog(); ///
	return(0);
}
