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
#include <alpm.h>

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

int g_list_is_strin(char *needle, GList *haystack)
{
	int i;

	for(i=0; i<g_list_length(haystack); i++)
		if(g_list_nth_data(haystack, i) && !strcmp(g_list_nth_data(haystack, i), needle))
			return(1);
	return(0);
}

// 1: add pkgdesc and On for dialog; 0: don't add
GList* group2pkgs(GList *syncs, char *group, int dialog)
{
	PM_GRP *grp;
	PM_LIST *pmpkgs, *lp, *junk;
	GList *pkgs=NULL;
	GList *list=NULL;
	int i, extra=0, addpkg=1;
	char *ptr, *pkgname, *lang;

	// add the core group to the start of the base list
	if(!strcmp(group, "base"))
		list = group2pkgs(syncs, "core", dialog);

	// get language suffix
	lang = strdup(getenv("LANG"));
	ptr = rindex(lang, '_');
	*ptr = '\0';

	if(strlen(group) >= strlen(EXGRPSUFFIX) && !strcmp(group + strlen(group) - strlen(EXGRPSUFFIX), EXGRPSUFFIX))
		extra=1;

	for (i=0; i<g_list_length(syncs); i++)
	{
		grp = alpm_db_readgrp(g_list_nth_data(syncs, i), group);
		if(grp)
		{
			pmpkgs = alpm_grp_getinfo(grp, PM_GRP_PKGNAMES);
			for(lp = alpm_list_first(pmpkgs); lp; lp = alpm_list_next(lp))
				pkgs = g_list_append(pkgs, alpm_list_getdata(lp));
			break;
		}
	}
	if(alpm_trans_init(PM_TRANS_TYPE_SYNC, PM_TRANS_FLAG_NODEPS, NULL, NULL, NULL) == -1)
	{
		fprintf(stderr, "failed to init transaction (%s)\n",
			alpm_strerror(pm_errno));
		return(NULL);
	}
	for (i=0; i<g_list_length(pkgs); i++)
		if(alpm_trans_addtarget(g_list_nth_data(pkgs, i)))
		{
			fprintf(stderr, "failed to add target '%s' (%s)\n",
				(char*)g_list_nth_data(pkgs, i), alpm_strerror(pm_errno));
			return(NULL);
		}

	if(alpm_trans_prepare(&junk) == -1)
	{
		fprintf(stderr, "failed to prepare transaction (%s)\n",
			alpm_strerror(pm_errno));
		return(NULL);
	}
	pmpkgs = alpm_trans_getinfo(PM_TRANS_PACKAGES);
	for(lp = alpm_list_first(pmpkgs); lp; lp = alpm_list_next(lp))
	{
		PM_SYNCPKG *sync = alpm_list_getdata(lp);
		PM_PKG *pkg = alpm_sync_getinfo(sync, PM_SYNC_PKG);
		//printf("%s\n", alpm_pkg_getinfo(pkg, PM_PKG_NAME));
		pkgname = alpm_pkg_getinfo(pkg, PM_PKG_NAME);
					// enable by default the packages in the
					// frugalware repo + enable the
					// language-specific parts from
					// locale-extra
				addpkg = ((!strcmp(group, "locale-extra") &&
					strlen(pkgname) >= strlen(lang) &&
					!strcmp(pkgname + strlen(pkgname) -
					strlen(lang), lang)) || !extra);
				if(!dialog && addpkg && !g_list_is_strin(pkgname, list))
					list = g_list_append(list, strdup(pkgname));
				if(dialog)
				{
					list = g_list_append(list, strdup(pkgname));
					// TODO: PM_PKG_SIZE
					list = g_list_append(list,
						strdup(alpm_pkg_getinfo(pkg, PM_PKG_DESC)));
					if(addpkg)
						list = g_list_append(list,
							strdup("On"));
					else
						list = g_list_append(list,
							strdup("Off"));
				}
	}
	alpm_trans_release();
	return(list);
}

char* categorysize(GList *syncs, char *category)
{
	int i;
	double size=0;
	PM_GRP *grp;
	PM_LIST *pmpkgs, *lp, *junk;
	GList *pkgs=NULL;

	for (i=0; i<g_list_length(syncs); i++)
	{
		grp = alpm_db_readgrp(g_list_nth_data(syncs, i), category);
		if(grp)
		{
			pmpkgs = alpm_grp_getinfo(grp, PM_GRP_PKGNAMES);
			for(lp = alpm_list_first(pmpkgs); lp; lp = alpm_list_next(lp))
				pkgs = g_list_append(pkgs, alpm_list_getdata(lp));
			break;
		}
	}
	if(alpm_trans_init(PM_TRANS_TYPE_SYNC, PM_TRANS_FLAG_NODEPS, NULL, NULL, NULL) == -1)
	{
		fprintf(stderr, "failed to init transaction (%s)\n",
			alpm_strerror(pm_errno));
		return(NULL);
	}
	for (i=0; i<g_list_length(pkgs); i++)
		if(alpm_trans_addtarget(g_list_nth_data(pkgs, i)))
		{
			fprintf(stderr, "failed to add target '%s' (%s)\n",
				(char*)g_list_nth_data(pkgs, i), alpm_strerror(pm_errno));
			return(NULL);
		}

	if(alpm_trans_prepare(&junk) == -1)
	{
		fprintf(stderr, "failed to prepare transaction (%s)\n",
			alpm_strerror(pm_errno));
		return(NULL);
	}
	pmpkgs = alpm_trans_getinfo(PM_TRANS_PACKAGES);
	for(lp = alpm_list_first(pmpkgs); lp; lp = alpm_list_next(lp))
	{
		PM_SYNCPKG *sync = alpm_list_getdata(lp);
		PM_PKG *pkg = alpm_sync_getinfo(sync, PM_SYNC_PKG);
		size += (int)alpm_pkg_getinfo(pkg, PM_PKG_SIZE);
	}
	alpm_trans_release();

	size = (double)(size/1048576.0);
	if(size < 0.1)
		size=0.1;
	return(g_strdup_printf("    %.1f MB", size));
}

GList *selcat(PM_DB *db, GList *syncs)
{
	char *name, *ptr;
	GList *catlist=NULL;
	char **arraychk;
	GList *ret;
	PM_LIST *lp;

	name = alpm_db_getinfo(db, PM_DB_TREENAME);

	for(lp = alpm_db_getgrpcache(db); lp; lp = alpm_list_next(lp))
	{
		PM_GRP *grp = alpm_list_getdata(lp);

		ptr = (char *)alpm_grp_getinfo(grp, PM_GRP_NAME);

		if(!strcmp(name, "frugalware-current") || !strcmp(name, "frugalware"))
		{
			if((index(ptr, '-')==NULL) && strcmp(ptr, "core"))
			{
				catlist = g_list_append(catlist, strdup(ptr));
#ifdef FINAL
				catlist = g_list_append(catlist,
					categorysize(syncs, ptr));
#else
				catlist = g_list_append(catlist,
					"   ");
#endif
				catlist = g_list_append(catlist, strdup("On"));
			}
		}
		else
		{
			if((index(ptr, '-')!=NULL) &&
				(strstr(ptr, "-extra")!=NULL))
			{
				catlist = g_list_append(catlist, strdup(ptr));
#ifdef FINAL
				catlist = g_list_append(catlist,
					categorysize(syncs, ptr));
#else
				catlist = g_list_append(catlist,
					"   ");
#endif
				if(strcmp(ptr, "locale-extra"))
					catlist = g_list_append(catlist,
						strdup("Off"));
				else
					catlist = g_list_append(catlist,
						strdup("On"));
			}
		}
	}

	// now display the list
	arraychk = glist2dialog(catlist);

	dlg_put_backtitle();
	dlg_clear();
	ret = fw_checklist(_("Selecting categories"),
		(!strcmp(name, "frugalware-current") || !strcmp(name, "frugalware")) ?
		_("Please select which extra categories to install:") :
		_("Please select which frugalware categories to install:"),
		0, 0, 0, g_list_length(catlist)/3, arraychk,
		FLAG_CHECK);
	return(ret);
}

GList *selpkg(char *category, GList *syncs)
{
	char **arraychk;
	GList *pkglist;
	GList *ret;

	pkglist = group2pkgs(syncs, category, 1);
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
		_("If you like, you may select your packages from expert "
		"menus. Where the normal menu shows a choice like 'C compiler "
		"system', the expert menus show you 'C libs', 'C compiler', "
		"'C include files', etc - each individual package. Obviously, "
		"you should know what you're doing if you use the expert menus "
		"since it's possible to skip packages that are crucial to the "
		"functioning of a subsystem. Choose 'no' for using normal "
		"menus that select groups of packages, or choose 'yes' for "
		"using expert menus with a switch for each package."), 0, 0);
	dialog_vars.defaultno=0;
	if(ret==DLG_EXIT_OK)
		return(1);
	else
		return(0);
}

int prepare_pkgdb(char *repo, GList **config, GList **syncs)
{
	char *pacbindir, *pkgdb;
	struct stat sbuf;
	int extra=0;
	PM_DB *i;
#ifdef FINAL
	char *mode;
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
			if(!extra)
				mode = strdup("w");
			else
				mode = strdup("a");
			if ((fp = fopen("/etc/pacman.conf", mode)) == NULL)
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
			fprintf(fp, "Server = file://%s\n\n", pacbindir);
			fclose(fp);
			FREE(mode);
#endif
		}
		else
			// TODO: handle if pacman returns an error, probably the
			// network has not been configured properly
			fw_system("pacman -Sy -r ./");
		if(((char*)data_get(*config, "netinstall")==NULL) && !extra)
		{
			makepath(g_strdup_printf("%s/var/cache/pacman",
				TARGETDIR));
			unlink("var/cache/pacman/pkg");
			symlink(pacbindir, "var/cache/pacman/pkg");
		}
		// pacman can't log without this
		makepath(g_strdup_printf("%s/var/log", TARGETDIR));
	}

	// register the database
	if(!extra)
	{
		i = alpm_db_register(PACCONF);
		if(i==NULL)
		{
			fprintf(stderr, "could not register '%s' database (%s)\n",
				PACCONF, alpm_strerror(pm_errno));
			return(1);
		}
		else
			*syncs = g_list_append(*syncs, i);
	}
	else
	{
		i = alpm_db_register(PACEXCONF);
		if(i==NULL)
		{
			fprintf(stderr, "could not register '%s' database (%s)\n",
				PACEXCONF, alpm_strerror(pm_errno));
			return(1);
		}
		else
			*syncs = g_list_append(*syncs, i);
	}
	return(0);
}

int fw_select(char *repo, GList **config, int selpkgc, GList *syncs)
{
	int i, extra=0;
	GList *cats=NULL;
	GList *allpkgs=NULL;

	if((!strcmp(repo, "extra"))||(!strcmp(repo, "extra-current")))
		extra=1;

	if(!extra)
		dialog_vars.backtitle=gen_backtitle(_("Selecting frugalware "
			"packages"));
	else
		dialog_vars.backtitle=gen_backtitle(_("Selecting extra "
			"packages"));
	dlg_put_backtitle();
	dlg_clear();
	dialog_msgbox(_("Please wait"), _("Searching for categories..."),
		0, 0, 0);
	if(!extra)
	{
		prepare_pkgdb(PACCONF, config, &syncs);
		if(((char*)data_get(*config, "netinstall")!=NULL) ||
			((char*)data_get(*config, "dvd")!=NULL))
			prepare_pkgdb(PACEXCONF, config, &syncs);
		cats = selcat(g_list_nth_data(syncs, 1), syncs);
	}
	else
		cats = selcat(g_list_nth_data(syncs, 2), syncs);
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
			pkgs = selpkg(strdup((char*)g_list_nth_data(cats, i)), syncs);
		else
			pkgs = group2pkgs(syncs, strdup((char*)g_list_nth_data(cats, i)), 0);
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
	PM_DB *i;
	GList *syncs=NULL;

	if(alpm_initialize("/mnt/target") == -1)
	{
		fprintf(stderr, "failed to initilize alpm library (%s)\n",
			alpm_strerror(pm_errno));
		return(1);
	}
	if(alpm_set_option(PM_OPT_DBPATH, (long)PM_DBPATH) == -1)
	{
		fprintf(stderr, "failed to set option DBPATH (%s)\n",
				alpm_strerror(pm_errno));
		return(1);
	}
	i = alpm_db_register("local");
	if(i==NULL)
	{
		fprintf(stderr, "could not register 'local' database (%s)\n",
			alpm_strerror(pm_errno));
		return(1);
	}
	else
		syncs = g_list_append(syncs, i);

	selpkgc = selpkg_confirm();
	chdir(TARGETDIR);
	fw_select("frugalware", config, selpkgc, syncs);
	if(((char*)data_get(*config, "netinstall")!=NULL) ||
		((char*)data_get(*config, "dvd")!=NULL))
		fw_select("extra", config, selpkgc, syncs);
	alpm_release();
	return(0);
}
