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

int nc_system(const char *cmd)
{
	if(nco_dryrun)
		return(printf("%s\n", cmd));
	else
		return(system(cmd));
}

int usage(const char *myname)
{
	printf("usage: %s [options] start|stop|restart\n", myname);
	printf("       %s [options] [profile]\n", myname);
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
	interface_t *iface=NULL;

	profile = (profile_t*)malloc(sizeof(profile_t));
	if(profile==NULL)
		return(NULL);
	memset(profile, 0, sizeof(profile_t));

	ptr = g_strdup_printf("/etc/sysconfig/network/%s", fn);
	fp = fopen(ptr, "r");
	if(fp == NULL)
	{
		printf("%s: No such profile!\n", fn);
		return(NULL);
	}
	free(ptr);

	while(fgets(line, PATH_MAX, fp))
	{
		n++;
		trim(line);
		if(strlen(line) == 0 || line[0] == '#')
			continue;
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
				if (!strcmp(var, "DOMAIN") && !strlen(profile->domain))
					strncpy(profile->domain, ptr, PATH_MAX);
				if (!strcmp(var, "OPTIONS"))
					iface->options = g_list_append(iface->options, strdup(ptr));
				if (!strcmp(var, "PRE_UP"))
					iface->pre_ups = g_list_append(iface->pre_ups, strdup(ptr));
				if (!strcmp(var, "POST_UP"))
					iface->post_ups = g_list_append(iface->post_ups, strdup(ptr));
				if (!strcmp(var, "PRE_DOWN"))
					iface->pre_downs = g_list_append(iface->pre_downs, strdup(ptr));
				if (!strcmp(var, "POST_DOWN"))
					iface->post_downs = g_list_append(iface->post_downs, strdup(ptr));
				if(!strcmp(var, "MAC") && !strlen(iface->mac))
					strncpy(iface->mac, ptr, MAC_MAX_SIZE);
				if(!strcmp(var, "ESSID") && !strlen(iface->essid))
					strncpy(iface->essid, ptr, ESSID_MAX_SIZE);
				if(!strcmp(var, "GATEWAY") && !strlen(iface->gateway))
					strncpy(iface->gateway, ptr, GW_MAX_SIZE);
			}
		}
		line[0] = '\0';
	}
	fclose(fp);
	return(profile);
}

int is_dhcp(interface_t *iface)
{
	int i, dhcp=0;
	for (i=0; i<g_list_length(iface->options); i++)
		if(!strcmp((char*)g_list_nth_data(iface->options, i), "dhcp"))
			dhcp=1;
	return(dhcp);
}

int ifdown(interface_t *iface)
{
	int dhcp, ret=0, i;
	char *ptr;
	FILE *fp;

	if(g_list_length(iface->pre_downs))
		for (i=0; i<g_list_length(iface->pre_downs); i++)
			nc_system((char*)g_list_nth_data(iface->pre_downs, i));

	dhcp = is_dhcp(iface);
	if(dhcp)
	{
		char line[7];
		ptr = g_strdup_printf("/etc/dhcpc/dhcpcd-%s.pid", iface->name);
		fp = fopen(ptr, "r");
		free(ptr);
		if(fp != NULL)
		{
			fgets(line, 6, fp);
			fclose(fp);
			i = atoi(line);
			if(i>0 && !nco_dryrun)
				ret = kill(i, 15);
			else if (i>0)
				printf("kill(%d, 15);\n", i);
		}
	}
	else
	{
		if(g_list_length(iface->options)>1)
			for (i=0; i<g_list_length(iface->options); i++)
			{
				ptr = g_strdup_printf("ifconfig %s 0.0.0.0", iface->name);
				nc_system(ptr);
				free(ptr);
			}
		ptr = g_strdup_printf("ifconfig %s down", iface->name);
		nc_system(ptr);
		free(ptr);
	}

	if(g_list_length(iface->post_downs))
		for (i=0; i<g_list_length(iface->post_downs); i++)
			nc_system((char*)g_list_nth_data(iface->post_downs, i));

	return(ret);
}

int ifup(interface_t *iface)
{
	int dhcp, i;
	char *ptr;

	if(g_list_length(iface->pre_ups))
		for (i=0; i<g_list_length(iface->pre_ups); i++)
			nc_system((char*)g_list_nth_data(iface->pre_ups, i));

	dhcp = is_dhcp(iface);
	// initialize the device
	if(strlen(iface->mac))
	{
		ptr = g_strdup_printf("ifconfig %s hw ether %s", iface->name, iface->mac);
		nc_system(ptr);
		free(ptr);
	}
	if(strlen(iface->essid))
	{
		ptr = g_strdup_printf("iwconfig %s essid %s", iface->name, iface->essid);
		nc_system(ptr);
		free(ptr);
	}

	// set up the interface
	if(dhcp)
	{
		ptr = g_strdup_printf("dhcpcd -t 10 %s", iface->name);
		nc_system(ptr);
		free(ptr);
	}
	else if(g_list_length(iface->options)==1)
	{
		ptr = g_strdup_printf("ifconfig %s %s",
			iface->name, (char*)g_list_nth_data(iface->options, 0));
		nc_system(ptr);
		free(ptr);
	}
	else
	{
		ptr = g_strdup_printf("ifconfig %s 0.0.0.0", iface->name);
		nc_system(ptr);
		free(ptr);
		for (i=0; i<g_list_length(iface->options); i++)
		{
			ptr = g_strdup_printf("ifconfig %s:%d %s",
				iface->name, i+1, (char*)g_list_nth_data(iface->options, i));
			nc_system(ptr);
			free(ptr);
		}
	}

	// setup the gateway
	if(!dhcp && strlen(iface->gateway))
	{
		ptr = g_strdup_printf("route add %s", iface->gateway);
		nc_system(ptr);
		free(ptr);
	}
	if(g_list_length(iface->post_ups))
		for (i=0; i<g_list_length(iface->post_ups); i++)
			nc_system((char*)g_list_nth_data(iface->post_ups, i));

	return(0);
}

int setdns(profile_t* profile)
{
	int i;
	FILE *fp=NULL;

	if(g_list_length(profile->dnses) || strlen(profile->domain))
	{
		if(!nco_dryrun)
			fp = fopen("/etc/resolv.conf", "w");
		if(nco_dryrun || (fp != NULL))
		{
			if(strlen(profile->domain))
			{
				if(nco_dryrun)
					printf("search %s\n", profile->domain);
				else
					fprintf(fp, "search %s\n", profile->domain);
			}
			if(g_list_length(profile->dnses))
				for (i=0; i<g_list_length(profile->dnses); i++)
					if(nco_dryrun)
						printf("nameserver %s\n", (char*)g_list_nth_data(profile->dnses, i));
					else
						fprintf(fp, "nameserver %s\n", (char*)g_list_nth_data(profile->dnses, i));
			if(!nco_dryrun)
				fclose(fp);
		}
		return(1);
	}
	return(0);
}

char *lastprofile(void)
{
	FILE *fp;
	char line[PATH_MAX+1];

	fp = fopen(NC_LOCK, "r");
	if(fp==NULL)
		return(NULL);
	fgets(line, PATH_MAX, fp);
	fclose(fp);
	trim(line);
	return(strdup(line));
}

int setlastprofile(char* str)
{
	FILE *fp;

	// sanility check
	if(!str)
		return(1);

	fp = fopen(NC_LOCK, "w");
	if(fp==NULL)
		return(1);
	fprintf(fp, str);
	fclose(fp);
	return(0);
}

int loup(void)
{
	int ret=0;

	ret += nc_system("ifconfig lo 127.0.0.1");
	ret += nc_system("route add -net 127.0.0.0 netmask 255.0.0.0 lo");
	return(ret);
}

int lodown(void)
{
	return(nc_system("ifconfig lo down"));
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
	int i;
	profile_t *profile;
	// dialog
	FILE *input = stdin;
	dialog_state.output = stderr;

	while((opt = getopt_long(argc, argv, "h", opts, &option_index)))
	{
		if(opt < 0)
			break;
		switch(opt)
		{
			/*case 'c': strcpy(fwo_conffile, optarg); break;
			case 'v': fwo_verbose = atoi(optarg); break;*/
			case 1000: nco_dryrun = 1; break;
			case 'h':  nco_usage  = 1; break;
		}
	}
	if(nco_usage)
		usage(argv[0]);

	if(optind < argc)
	{
		if((fn=lastprofile()) || !strcmp("stop", argv[optind]))
		{
			if(!strcmp("stop", argv[optind]) && !fn)
				return(127);
			profile = parseprofile(fn);
			if(profile!=NULL)
				// unload the old profile
				for (i=0; i<g_list_length(profile->interfaces); i++)
					ifdown((interface_t*)g_list_nth_data(profile->interfaces, i));
			if(!strcmp("stop", argv[optind]))
			{
				lodown();
				return(0);
			}
		}
		// load the default for 'start' and for 'restart' if not yet started
		if(!strcmp("start", argv[optind]) || (!strcmp("restart", argv[optind]) && !fn))
		{
			loup();
			fn = strdup("default");
		}
		// load the target profile if != 'restart'
		else if (strcmp("restart", argv[optind]))
			fn = argv[optind];
		// load the new profile
		profile = parseprofile(fn);
		if(profile==NULL)
			return(1);
		for (i=0; i<g_list_length(profile->interfaces); i++)
			ifup((interface_t*)g_list_nth_data(profile->interfaces, i));
		setdns(profile);
		setlastprofile(fn);
		free(fn);
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