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
#include <stdlib.h>

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

int pkgsize(char *pkg, int extra)
{
	FILE *fp;
	char line[256], *fn;
	int ret;

	if(!extra)
		fn = g_strdup_printf("%s/desc", pkgdir(pkg, PACCONF));
	else
		fn = g_strdup_printf("%s/desc", pkgdir(pkg, PACEXCONF));

	if ((fp = fopen(fn, "r"))
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

char* pkgdesc(char *pkg, int extra)
{
	FILE *fp;
	char line[256];
	char *ret=NULL, *fn, *ptr;

	if(!extra)
		fn = g_strdup_printf("%s/desc", pkgdir(pkg, PACCONF));
	else
		fn = g_strdup_printf("%s/desc", pkgdir(pkg, PACEXCONF));

	if ((fp = fopen(fn, "r"))
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

// 1: add pkgdesc and On for dialog; 0: don't add
GList* group2pkgs(char *group, int dialog)
{
	FILE *pp;
	char line[256], *lang, *ptr, *ptr2;
	GList *list=NULL;
	int extra=0;

	// get language suffix
	lang = strdup(getenv("LANG"));
	ptr = rindex(lang, '_');
	*ptr = '\0';

	if(strlen(group) >= strlen(EXGRPSUFFIX) && !strcmp(group + strlen(group) - strlen(EXGRPSUFFIX), EXGRPSUFFIX))
		extra=1;

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
				if(dialog)
				{
					// TODO: pkgsize()
					list = g_list_append(list,
						pkgdesc(ptr2, extra));
					// enable by default the packages in the
					// frugalware repo + enable the
					// language-specific parts from
					// locale-extra
					if((!strcmp(group, "locale-extra") &&
					strlen(ptr2) >= strlen(lang) &&
					!strcmp(ptr2 + strlen(ptr2) -
					strlen(lang), lang)) || !extra)
						list = g_list_append(list,
							strdup("On"));
					else
						list = g_list_append(list,
							strdup("Off"));
				}
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

	pkglist = group2pkgs(category, 1);
	arraychk = glist2dialog(pkglist);

	dlg_put_backtitle();
	dlg_clear();
	ret = fw_checklist(_("Selecting packages"),
		g_strdup_printf(("Please select packages to install from the %s section:"), category),
		0, 0, 0, g_list_length(pkglist)/3, arraychk,
		FLAG_CHECK);
	return(ret);
}

int selpkg_confirm(void)
{
	int ret;
	dialog_vars.defaultno=1;
	dlg_put_backtitle();
	dlg_clear();
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
#ifdef FINAL
				catlist = g_list_append(catlist,
					categorysize(line));
#else
				catlist = g_list_append(catlist,
					"   ");
#endif
				catlist = g_list_append(catlist, strdup("On"));
			}
		}
		else
		{
			if((index(line, '-')!=NULL) &&
				(strstr(line, "-extra")!=NULL))
			{
				catlist = g_list_append(catlist, strdup(line));
#ifdef FINAL
				catlist = g_list_append(catlist,
					categorysize(line));
#else
				catlist = g_list_append(catlist,
					"   ");
#endif
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
		repo ? _("Please select which extra categories to install:") :
		_("Please select which frugalware categories to install:"),
		0, 0, 0, g_list_length(catlist)/3, arraychk,
		FLAG_CHECK);
	return(ret);
}

int prepare_pkgdb(char *repo, GList **config)
{
	char *pacbindir, *pkgdb;
	struct stat sbuf;
	int extra=0;
#ifdef FINAL
	FILE *fp;
#endif

	if((!strcmp(repo, "extra"))||(!strcmp(repo, "extra-current")))
		extra=1;
	if(!extra)
		pacbindir = g_strdup_printf("%s/frugalware-%s",
			SOURCEDIR, ARCH);
	else
		pacbindir = g_strdup_printf("%s/extra/frugalware-%s",
			SOURCEDIR, ARCH);
	pkgdb = g_strdup_printf("%s/var/lib/pacman/%s", TARGETDIR, repo);

	// prepare pkgdb if necessary
	if(stat(pkgdb, &sbuf) || !S_ISDIR(sbuf.st_mode))
	{
		makepath(g_strdup_printf("%s/tmp", TARGETDIR));
		if((char*)data_get(*config, "netinstall")==NULL)
		{
			makepath(pkgdb);
			// TODO: use libarchive for this
			system(g_strdup_printf("tar xzf %s/%s.fdb -C %s", pacbindir, repo, pkgdb));
#ifdef FINAL
			if ((fp = fopen("/etc/pacman.conf", "w")) == NULL)
			{
				perror(_("Could not open output file for writing"));
				return(1);
			}
			if(!extra)
			{
				fprintf(fp, "[options]\n");
				fprintf(fp, "LogFile = %s/var/log/pacman.log\n", TARGETDIR);
			}
			fprintf(fp, "[%s]\n", repo);
			fprintf(fp, "Server = file://%s", pacbindir);
			fclose(fp);
#endif
		}
		else
			fw_system("pacman -Sy -r ./");
		makepath(g_strdup_printf("%s/var/cache/pacman", TARGETDIR));
		unlink("var/cache/pacman/pkg");
		if((char*)data_get(*config, "netinstall")==NULL)
			symlink(pacbindir, "var/cache/pacman/pkg");
		// pacman can't log without this
		makepath(g_strdup_printf("%s/var/log", TARGETDIR));
	}
	return(0);
}

int fw_select(char *repo, GList **config, int selpkgc)
{
	int i, extra=0;
	GList *cats=NULL;
	GList *allpkgs=NULL;

	if((!strcmp(repo, "extra"))||(!strcmp(repo, "extra-current")))
		extra=1;

	if(!extra)
	{
		prepare_pkgdb(PACCONF, config);
		prepare_pkgdb(PACEXCONF, config);
		dialog_vars.backtitle=gen_backtitle(_("Selecting frugalware "
			"packages"));
		cats = selcat(0);
	}
	else
	{
		dialog_vars.backtitle=gen_backtitle(_("Selecting extra "
			"packages"));
		cats = selcat(1);
	}
	if(!selpkgc)
	{
		dlg_put_backtitle();
		dialog_msgbox(_("Please wait"), _("Searching for packages..."),
		0, 0, 0);
	}
	for (i=0; i<g_list_length(cats); i++)
	{
		GList *pkgs=NULL;
		if(selpkgc)
			pkgs = selpkg(strdup((char*)g_list_nth_data(cats, i)));
		else
			pkgs = group2pkgs(strdup((char*)g_list_nth_data(cats, i)), 0);
		pkgs = g_list_prepend(pkgs, strdup((char*)g_list_nth_data(cats, i)));
		allpkgs = g_list_append(allpkgs, pkgs);
	}
	if(!extra)
		data_put(config, "packages", allpkgs);
	else
		data_put(config, "expackages", allpkgs);
	return(0);
}
int run(GList **config)
{
	int selpkgc;

	selpkgc = selpkg_confirm();
	chdir(TARGETDIR);
	fw_select("frugalware", config, selpkgc);
	fw_select("extra", config, selpkgc);
	return(0);
}
