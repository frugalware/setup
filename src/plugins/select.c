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

GList* group2pkgs(char *group)
{
	FILE *pp;
	char line[256], *ptr, *ptr2;
	GList *list=NULL;

	if ((pp = popen(g_strdup_printf("pacman -Sg %s -r ./", group), "r"))
		== NULL)
	{
		perror("Could not open pipe for reading");
		return(NULL);
	}
	while(!feof(pp))
	{
		if(fgets(line, 255, pp) == NULL)
			break;
		// this line has data for us
		if(strstr(line, "   ")==line)
		{
			ptr = strchr(line, '\n');
			*ptr = '\0';
			ptr = line;
			// skip leading whitespace
			while(strchr(ptr, ' ')==ptr)
				ptr++;
			while(strchr(ptr, ' ')!=NULL)
			{
				ptr2 = ptr;
				ptr = strchr(ptr, ' ');
				*ptr = '\0';
				ptr++;
				list = g_list_append(list, strdup(ptr2));
				// TODO: pkgsize()
				list = g_list_append(list, pkgdesc(ptr2));
				list = g_list_append(list, strdup("On"));
			}
		}
	}
	pclose(pp);
	return(list);
}
GList *selpkg(char *category)
{
	char **arraychk;
	GList *pkglist;
	GList *ret;

	pkglist = group2pkgs(category);
	arraychk = glist2dialog(pkglist);

	dlg_put_backtitle();
	dlg_clear();
	ret = fw_checklist(_("Selecting packages"),
		g_strdup_printf(("Please select which categories to install from the %s section:"), category),
		0, 0, 0, g_list_length(pkglist)/3, arraychk,
		FLAG_CHECK);
	return(ret);
}

int selpkg_confirm(void)
{
	int ret;
	dialog_vars.defaultno=1;
	dlg_put_backtitle();
	ret = dialog_yesno(_("Use expert menus?"),
		_("If you like, you may select your packages from expert menus."
		"Where the normal menu shows a choice like 'C compiler system',"
		"the expert menus show you 'C libs', 'C compiler', 'C include"
		"files', etc - each individual package. Obviously, you should"
		"know what you're doing if you use the expert menus since it's"
		"possible to skip packages that are crucial to the functioning"
		"of a subsystem. Choose 'no' for using normal menus that select"
		"groups of packages, or choose 'yes' for using expert menus"
		"with a switch for each package."), 0, 0);
	dialog_vars.defaultno=0;
	if(ret==DLG_EXIT_OK)
		return(1);
	else
		return(0);
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
	int i, selpkgc;
	GList *list=NULL;

	dialog_vars.backtitle=gen_backtitle(_("Selecting packages"));
	chdir(TARGETDIR);
	selpkg = selpkg_confirm();
	list = selcat(0);
	fw_end_dialog(); ///
	for (i=0; i<g_list_length(list); i++)
		printf("new item: %s\n", (char*)g_list_nth_data(list, i));
	if (selpkg)
		printf("expert mode\n");
	fw_init_dialog(); ///
	return(0);
}
