/*
 *  netconfig.c for Frugalware setup
 * 
 *  Copyright (c) 2006 by Miklos Vajna <vmiklos@frugalware.org>
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
#include <getopt.h>
#include <stdlib.h>
#include <glib.h>
#include <net/if.h>

#include "netconfig.h"

int nco_dryrun  = 0;
int nco_usage   = 0;

int usage(const char *myname)
{
	printf("usage: %s [options] [profile]\n", myname);
	//printf("-v | --verbose <level>   Verbose mode.\n");
	//printf("-c | --config  <file>    Config file.\n");
	printf("-h | --help              This help.\n");
	printf("     --dry-run           Do not actually perform the operation.\n");
	exit(0);
}

char *trim(char *str)
{
	char *ptr = str;

	while(isspace(*ptr++))
		if(ptr != str)
			memmove(str, ptr, (strlen(ptr) + 1));
	ptr = (char *)(str + (strlen(str) - 1));
	while(isspace(*ptr--))
		*++ptr = '\0';
	return str;
}

char *strtoupper(char *str)
{
	char *ptr = str;

	while(*ptr)
		*ptr++ = toupper(*ptr);
	return str;
}


profile_t *parseprofile(char *fn)
{
	FILE *fp;
	char line[PATH_MAX+1];
	int i, n=0;
	char *ptr = NULL;
	char *var = NULL;
	char interface[256] = "";
	profile_t *profile;
	interface_t *iface;

	profile = (profile_t*)malloc(sizeof(profile_t));
	if(profile==NULL)
		return(NULL);
	memset(profile, 0, sizeof(profile_t));

	ptr = g_strdup_printf("/etc/sysconfig/network/%s", fn);
	fp = fopen(ptr, "r");
	free(ptr);
	if(fp == NULL)
	{
		printf("No such profile!\n");
		return(NULL);
	}

	while(fgets(line, PATH_MAX, fp))
	{
		n++;
		trim(line);
		if(strlen(line) == 0 || line[0] == '#')
			continue;
		printf("new line: %s\n", line);
		if(line[0] == '[' && line[strlen(line)-1] == ']')
		{
			// new interface
			ptr = line;
			ptr++;
			strncpy(interface, ptr, min(255, strlen(ptr)-1));
			interface[min(255, strlen(ptr)-1)] = '\0';
			if(!strlen(interface))
			{
				fprintf(stderr, "profile: line %d: bad interface name\n", n);
				return(NULL);
			}
			if(strcmp(interface, "options"))
			{
				int found = 0;
				for (i=0; !found && i<g_list_length(profile->interfaces); i++)
				{
					iface = (interface_t*)g_list_nth_data(profile->interfaces, i);
					if(!strcmp(iface->name, interface))
						found=1;
				}
				if(!found)
				{
					// start a new interface record
					iface = (interface_t*)malloc(sizeof(interface_t));
					if(iface==NULL)
						return(NULL);
					memset(iface, 0, sizeof(interface_t));
					strncpy(iface->name, interface, IF_NAMESIZE);
					profile->interfaces = g_list_append(profile->interfaces, iface);
				}
			}
		}
		else
		{
			// directive
			ptr = line;
			var = strsep(&ptr, "=");
			if(var == NULL)
			{
				fprintf(stderr, "profile: line %d: syntax error\n", n);
				return(NULL);
			}
			trim(var);
			var = strtoupper(var);
			if(!strlen(interface))
			{
				fprintf(stderr, "profile: line %d: all directives must belong to a section\n", n);
				return(NULL);
			}
			if(ptr != NULL)
			{
				trim(ptr);
				if (!strcmp(var, "DNS"))
					profile->dnses = g_list_append(profile->dnses, strdup(ptr));
			}
		}
		line[0] = '\0';
	}
	fclose(fp);
	/*printf("dnses found:\n");
	for (i=0; i<g_list_length(profile->dnses); i++)
	{
		printf("%s\n", (char*)g_list_nth_data(profile->dnses, i));
	}*/
	return(profile);
}

int main(int argc, char **argv)
{
	int opt;
	int option_index;
	static struct option opts[] =
	{
		/*{"verbose",        required_argument, 0, 'v'},
		{"config",         required_argument, 0, 'c'},*/
		{"help",           no_argument,       0, 'h'},
		{"dry-run",        no_argument,       0, 1000},
		{0, 0, 0, 0}
	};
	char *fn=NULL;
	profile_t *profile;
	// dialog
	FILE *input = stdin;
	dialog_state.output = stderr;

	while((opt = getopt_long(argc, argv, "h", opts, &option_index)))
	{
		if(opt < 0)
		{
			break;
		}
		switch(opt)
		{
			/*case 'c': strcpy(fwo_conffile, optarg); break;
			case 'v': fwo_verbose = atoi(optarg); break;*/
			case 1000: nco_dryrun = 1; break;
			case 'h':  nco_usage  = 1; break;
		}
	}
	if(nco_usage)
	{
		usage(argv[0]);
	}

	if(optind < argc)
	{
		if(!strcmp("start", argv[optind]))
			fn = strdup("default");
		else
			fn = argv[optind];
		profile = parseprofile(fn);
	}
	else
	{
		printf("no profile to load\n");
		/*init_dialog(input, dialog_state.output);
		dialog_msgbox("title", "content", 0, 0, 0);
		end_dialog();*/
	}
	return(0);
}
