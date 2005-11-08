/*
 *  asklang.c for Frugalware setup
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
#include <stdlib.h>
#include <string.h>

#include <setup.h>
#include <util.h>
#include "common.h"

#define LANGSNUM 8
char *langs[] =
{
	"en_US", "English                         ",
	"es_AR", "Spanish / Espagnole             ",
	"de_DE", "German / Deutsch                ",
	"fr_FR", "French / francais               ",
	"it_IT", "Italian / Italiano              ",
	"hu_HU", "Hungarian / Magyar              ",
	"pl_PL", "Polish / Polski                 ",
	"sk_SK", "Slovak / Slovensky              "
};

plugin_t plugin =
{
	"asklang",
	01,
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}

int setcharset(char *name)
{
	//TODO: maybe there is a proper system call for this?
	system(g_strdup_printf("setfont %s >%s 2>%s", name, LOGDEV, LOGDEV));
	bind_textdomain_codeset("setup", g_ascii_strup(name, strlen(name)-1));
	return(0);
}

int run(GList **config)
{
	dialog_vars.backtitle=gen_backtitle("Selecting language");
	dlg_put_backtitle();
	dlg_clear();
	fw_menu("Please select your language",
		"Please select your language from the list. If your language"
		"is not in the list, you probably should choose English.",
		0, 0, 0, LANGSNUM, langs);

	setenv("LC_ALL", dialog_vars.input_result, 1);
	setenv("LANG",   dialog_vars.input_result, 1);
	setlocale(LC_ALL, dialog_vars.input_result);
	bindtextdomain("setup", "/usr/share/locale");
	
	if(!strcmp("en_US", dialog_vars.input_result))
		setenv("CHARSET", "iso-8859-1", 1);
	else if(!strcmp("es_AR", dialog_vars.input_result))
	{
		setenv("CHARSET", "iso-8859-1", 1);
		setcharset("lat1-16.psfu.gz");
	}
	else if(!strcmp("de_DE", dialog_vars.input_result))
		setenv("CHARSET", "iso-8859-15", 1);
	else if(!strcmp("fr_FR", dialog_vars.input_result))
		setenv("CHARSET", "iso-8859-15", 1);
	else if(!strcmp("it_IT", dialog_vars.input_result))
	{
		setenv("CHARSET", "iso-8859-1", 1);
		setcharset("lat9w-16.psfu.gz");
	}
	else if(!strcmp("hu_HU", dialog_vars.input_result))
	{
		setenv("CHARSET", "iso-8859-2", 1);
		setcharset("lat2-16.psfu.gz");
	}
	else if(!strcmp("pl_PL", dialog_vars.input_result))
	{
		setenv("CHARSET", "iso-8859-2", 1);
		setcharset("lat2-16.psfu.gz");
	}
	else if(!strcmp("sk_SK", dialog_vars.input_result))
	{
		setenv("CHARSET", "iso-8859-2", 1);
		setcharset("lat2-16.psfu.gz");
	}
	textdomain("setup");

	return(0);
}
