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
	char *fn;
	FILE* fp;

	fn = strdup("/tmp/setup_XXXXXX");
	mkstemp(fn);
	if ((fp = fopen(fn, "w")) == NULL)
	{
		perror(_("Could not open output file for writing"));
		return(1);
	}
	fprintf(fp, "#!/bin/sh\n\n"
		"# /etc/profile.d/lang.sh\n\n"
		"# Set the system locale\n"
		"# For a list of locales which are supported by this machine, "
		"type: locale -a\n\n");
	if(!getenv("LANG") || strcmp(getenv("LANG"), "zh_CN"))
		fprintf(fp, "export LANG=%s\n", getenv("LANG"));
	else
	{
		fprintf(fp, "if [ \"$TERM\" = \"linux\" ]; then\n\techo -ne \"\\e%%G\"\nfi\n");
		fprintf(fp, "export LANG=zh_CN.utf8\n");
	}
	fprintf(fp, "export LC_ALL=$LANG\n");
	if(getenv("CHARSET"))
		fprintf(fp, "export CHARSET=%s\n", getenv("CHARSET"));
	fclose(fp);

	// sample: adds a "content" string titled "stuff" to the config list
	data_put(config, "lang.sh", fn);
	return(0);
}
