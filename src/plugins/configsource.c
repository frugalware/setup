/*
 *  configsource.c for Frugalware setup
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
#ifdef DIALOG
    #include <dialog.h>
#endif
#ifdef GTK
    #include <gtk/gtk.h>
#endif
#include <unistd.h>
#include <stdlib.h>

#include <setup.h>
#include <util.h>
#include "common.h"

plugin_t plugin =
{
	"configsource",
	desc,
	30,
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}

char *desc()
{
	return _("Configuring the source of the installation");
}

GList *getmirrors(char *fn)
{
	FILE *fp;
	char line[PATH_MAX], *ptr, *country, *preferred;
	GList *mirrors=NULL;

	if ((fp = fopen(fn, "r"))== NULL) { //fopen error
		printf("Could not open output file for reading");
		return(NULL);
	}

	/* this string should be the best mirror for the given language from
	 * /etc/pacman-g2/repos */
	preferred = strdup(_("ftp://ftp5."));
	while(!feof(fp)) {
		if(fgets(line, PATH_MAX, fp) == NULL)
			break;
		if(line == strstr(line, "# - ")) { // country
			ptr = strrchr(line, ' ');
			*ptr = '\0';
			ptr = strrchr(line, ' ')+1;
			country = strdup(ptr);
		}
		if(line == strstr(line, "Server = ")) { // server
			ptr = strrchr(line, '/'); // drops frugalware-ARCH
			*ptr = '\0';
			ptr = strstr(line, "Server = ")+9; //drops 'Server = ' part
			mirrors = g_list_append(mirrors, strdup(ptr));
			mirrors = g_list_append(mirrors, strdup(country));
			if(!strncmp(ptr, preferred, strlen(preferred)))
				mirrors = g_list_append(mirrors, strdup("On"));
			else
				mirrors = g_list_append(mirrors, strdup("Off")); //unchecked by default in checkbox
		}
	}
	free(preferred);
	fclose(fp);
	return (mirrors);
}

int updateconfig(char *fn, GList *mirrors)
{
	FILE *fp;
	short i;

	if ((fp = fopen(fn, "w"))== NULL)
	{
		perror(_("Could not open output file for writing"));
		return(1);
	}
	fprintf(fp, "#\n# %s repository\n#\n\n[%s]\n\n", PACCONF, PACCONF);
	for (i=0; i<g_list_length(mirrors); i+=2) {
		// do not change the country style as it will cause getmirrors() misbehaviour
		fprintf(fp, "# - %s -\n", (char *)g_list_nth_data(mirrors, i+1));
		fprintf(fp, "Server = %s/frugalware-%s\n", (char *)g_list_nth_data(mirrors, i), ARCH);
	}
	fclose(fp);
	g_list_free(mirrors);
	return(0);
}

GList *mirrorconf(void)
{
	short i,j;
	GList *mirrorlist=NULL, *newmirrorlist=NULL;
	char *fn;
	char **arraychk;

	fn = g_strdup_printf("%s/%s", PACCONFPATH, PACCONF);

	dialog_vars.backtitle=gen_backtitle(_("Selecting a mirror"));
	dlg_put_backtitle();
	dlg_clear();

	mirrorlist = getmirrors(fn);
	arraychk = glist2dialog(mirrorlist);

	newmirrorlist = fw_checklist(_("Please select mirrors"),
					_("Here you can chose one or more "
					"nearby mirror to speed up "
					"package dowloading."),
					0, 0, 0,
					g_list_length(mirrorlist)/3,
					arraychk,
					FLAG_CHECK);
	for (i=0; i<g_list_length(newmirrorlist); i++)
		LOG("selected preferred mirror '%s'", (char*)g_list_nth_data(newmirrorlist, i));

	// removes the checkbox related part (Off state)
	for (i=0; i<g_list_length(mirrorlist); i++) {
		if (!strcmp(g_list_nth_data(mirrorlist, i), "Off") ||
				!strcmp(g_list_nth_data(mirrorlist, i), "On"))
		{
			mirrorlist = g_list_remove(mirrorlist, g_list_nth_data(mirrorlist, i));
		}
	}
	// adds country info to the selected mirrors
	// also removes the duplicate mirrors
	for (i=0; i<g_list_length(mirrorlist); i+=2) {
		for (j=0; j<g_list_length(newmirrorlist); j++) {
			if (!strcmp((char*)g_list_nth_data(mirrorlist, i), (char*)g_list_nth_data(newmirrorlist, j))) {
				newmirrorlist = g_list_insert(newmirrorlist, g_list_nth_data(mirrorlist, i+1), j+1);
				mirrorlist = g_list_remove(mirrorlist, g_list_nth_data(mirrorlist, i));
				mirrorlist = g_list_remove(mirrorlist, g_list_nth_data(mirrorlist, i));
			}
		}
	}
	// merges the selected and remain mirrors
	newmirrorlist = g_list_concat(newmirrorlist, mirrorlist);
	dialog_vars.defaultno=1;
	dialog_vars.cancel_label = strdup(_("Continue"));
	if(fw_inputbox(_("Custom mirror"), _("You may now specify "
					"a custom mirror (eg. LAN) "
					"so you can download packages "
					"faster. In most cases a "
					"Cancel enough here."), 0, 0,
				(char*)g_list_nth_data(newmirrorlist, 0), 0) != -1) { //not cancel
		if (strcmp(dialog_vars.input_result, "\0")) { //not empty
				LOG("added custom mirror '%s'", dialog_vars.input_result);
				newmirrorlist = g_list_insert(newmirrorlist, strdup(dialog_vars.input_result), 0);
				newmirrorlist = g_list_insert(newmirrorlist, strdup("CUSTOM"), 1);
		}
	}
	dialog_vars.defaultno=0;
	FREE(dialog_vars.cancel_label);
	updateconfig(fn, newmirrorlist);
	return(newmirrorlist);
}

int run(GList **config)
{
	dlg_put_backtitle();
	if((char*)data_get(*config, "netinstall")!=NULL)
	{
		fw_end_dialog();
		while(1)
		{
			fw_system_interactive(NETCONFIGSCRIPT);
			fw_init_dialog();
			dlg_put_backtitle();
			dialog_msgbox(_("Please wait"), _("Configuring the network "
				"interface..."), 0, 0, 0);
			if(!fw_system(INTERFACESSCRIPT))
				break;
		}
		if(mirrorconf() == NULL)
			return(-1);
	}
	return(0);
}
