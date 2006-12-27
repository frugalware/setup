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

char *firstmirror(char *fn)
{
	FILE *fp;
	char line[PATH_MAX];
	char *ptr;
	
	if ((fp = fopen(fn, "r"))== NULL)
	{
		perror(_("Could not open output file for writing"));
		return(NULL);
	}
	while(!feof(fp))
	{
		if(fgets(line, 256, fp) == NULL)
			break;
		if(line == strstr(line, "Server = "))
		{
			fclose(fp);
			// drop /frugalware-ARCH
			ptr = strrchr(line, '/');
			*ptr = '\0';
			return(strstr(line, " = ")+3);
		}
	}
	return(NULL);
}

int updateconfig(char *fn, char *mirror)
{
	FILE *fp, *tfp;
	char line[PATH_MAX];
	char *tfn;
	int firstline=1;
	
	tfn = strdup("/tmp/setup_XXXXXX");
	mkstemp(tfn);
	
	copyfile(fn, tfn);
	if ((tfp = fopen(tfn, "r"))== NULL)
	{
		perror(_("Could not open output file for writing"));
		return(1);
	}
	if ((fp = fopen(fn, "w"))== NULL)
	{
		perror(_("Could not open output file for writing"));
		return(1);
	}
	while(!feof(tfp))
	{
		if(fgets(line, 256, tfp) == NULL)
			break;
		if((line == strstr(line, "Server = ")) && firstline)
		{
			fprintf(fp, "Server = %s/frugalware-%s\n", mirror, ARCH);
			fprintf(fp, line);
			firstline=0;
		}
		else
			fprintf(fp, line);
	}
	fclose(tfp);
	fclose(fp);
	unlink(tfn);
	return(0);
}

int mirrorconf(void)
{
	char *fn, *mirror;
	int ret;
	fn = g_strdup_printf("%s/%s", PACCONFPATH, PACCONF);
	char my_buffer[MAX_LEN + 1] = "";
	
	mirror = firstmirror(fn);
	dialog_vars.backtitle=gen_backtitle(_("Selecting a mirror"));
	dlg_put_backtitle();
	dlg_clear();
	while(1)
	{
		dialog_vars.input_result = my_buffer;
		ret = dialog_inputbox(_("Please select a mirror"), _("You may now specify the mirror closest to you in order to download the packages faster. In most cases the default value will be fine."), 0, 0, mirror, 0);
		if (ret != DLG_EXIT_CANCEL)
			break;
		if(exit_confirm())
			exit_perform();
	}

	updateconfig(fn, dialog_vars.input_result);
	
	return(0);
}

int run(GList **config)
{
	dlg_put_backtitle();
	if((char*)data_get(*config, "netinstall")!=NULL)
	{
		fw_end_dialog();
		while(1)
		{
			system(NETCONFIGSCRIPT);
			fw_init_dialog();
			dlg_put_backtitle();
			dialog_msgbox(_("Please wait"), _("Configuring the network "
				"interface..."), 0, 0, 0);
			if(!fw_system(INTERFACESSCRIPT))
				break;
		}
		mirrorconf();
	}
	return(0);
}
