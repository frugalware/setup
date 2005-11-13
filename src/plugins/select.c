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

	chdir(TARGETDIR);
	if ((pp = popen(g_strdup_printf("echo -e 'y\nn'|pacman -Syd %s -r ./", category), "r"))== NULL)
	{
		perror("Could not open pipe for reading");
		return(1);
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
			return(line);
		}
	}
	FREE(line);
	pclose(pp);
	return(NULL);
}

int run(GList **config)
{
	dialog_msgbox("base", categorysize("base"), 0, 0, 1);
	
	return(0);
}
