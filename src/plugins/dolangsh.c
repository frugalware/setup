/*
 *  dolangsh.c for Frugalware setup
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
#include <stdlib.h>
#include <string.h>

#include <setup.h>
#include <util.h>
#include "common.h"

plugin_t plugin =
{
	"dolangsh",
	desc,
	05,
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}

char *desc()
{
	return _("Writing the language configuration file");
}

int run(GList **config)
{
	const char *lang;
	char *fn;
	FILE* fp;
	int i = 0;
	static const char *vars[] =
	{
		"LANGUAGE",
		"LC_CTYPE",
		"LC_NUMERIC",
		"LC_TIME",
		"LC_COLLATE",
		"LC_MONETARY",
		"LC_MESSAGES",
		"LC_PAPER",
		"LC_NAME",
		"LC_ADDRESS",
		"LC_TELEPHONE",
		"LC_MEASUREMENT",
		"LC_IDENTIFICATION",
		0
	};

	if((lang = getenv("LANG")) == 0)
	{
		perror(_("Could not retrieve the language settings."));
		fclose(fp);
		return(1);
	}

	fn = strdup("/tmp/setup_XXXXXX");
	mkstemp(fn);
	if ((fp = fopen(fn, "w")) == NULL)
	{
		perror(_("Could not open output file for writing"));
		return(1);
	}

	fprintf(fp,
		"# /etc/locale.conf\n"
		"# The system wide locale(s) is defined below.\n"
		"# Changes to these locale(s) require a reboot.\n"
		"# For a complete list of supported locales, run this shell command:\n"
		"# locale -a\n"
		"\n"
		"LANG=%s.utf8\n"
		"\n"
		"# The settings below should only be used for advanced configurations.\n"
		"# Please read the locale man page 7 for more information.\n"
		"\n",
		lang
	);

	for( ; vars[i] != 0 ; ++i )
		fprintf(fp,"#%s=\n",vars[i]);

	fclose(fp);

	// sample: adds a "content" string titled "stuff" to the config list
	data_put(config, "lang.sh", fn);
	return(0);
}
