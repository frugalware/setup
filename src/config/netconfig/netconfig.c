/*
 *  netconfig.c for Frugalware
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
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libintl.h>

#include "netconfig.h"

int nco_dryrun  = 0;
int nco_usage   = 0;
int nco_fast   = 0;

int nc_system(const char *cmd)
{
	if(nco_dryrun)
		return(printf("%s\n", cmd));
	else
		return(system(cmd) ? 1 : 0);
}

int usage(const char *myname)
{
	printf(_("usage: %s [options] start|stop|restart|status|list\n"), myname);
	printf(_("       %s [options] [profile]\n"), myname);
	printf(_("-h | --help              This help.\n"));
	printf(_("-f | --fast              Fast mode, used by the setup.\n"));
	printf(_("     --dry-run           Do not actually perform the operation.\n"));
	return(0);
}

void i18ninit(void)
{
	char *lang=NULL;

	lang=getenv("LC_ALL");
	if(lang==NULL || lang[0]=='\0')
		lang=getenv("LC_MESSAGES");
	if (lang==NULL || lang[0]=='\0')
		lang=getenv("LANG");

	setlocale(LC_ALL, lang);
	bindtextdomain("netconfig", "/usr/share/locale");
	textdomain("netconfig");
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

int listprofiles(void)
{
	struct dirent *ent=NULL;
	DIR *dir;

	dir = opendir(NC_PATH);
	while((ent = readdir(dir)))
	{
		if(strcmp(ent->d_name, ".") && strcmp(ent->d_name, ".."))
			printf("%s\n", ent->d_name);
	}
	return(0);
}

/*
 * based on pacman's config parser, which is
 * Copyright (c) 2002-2006 by Judd Vinet <jvinet@zeroflux.org>
 */
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

	ptr = g_strdup_printf(NC_PATH "/%s", fn);
	fp = fopen(ptr, "r");
	if(fp == NULL)
	{
		printf(_("%s: No such profile!\n"), fn);
		return(NULL);
	}
	FREE(ptr);

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
				fprintf(stderr, _("profile: line %d: bad interface name\n"), n);
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
				fprintf(stderr, _("profile: line %d: syntax error\n"), n);
				return(NULL);
			}
			trim(var);
			var = strtoupper(var);
			if(!strlen(interface))
			{
				fprintf(stderr, _("profile: line %d: all directives must belong to a section\n"), n);
				return(NULL);
			}
			if(ptr != NULL)
			{
				trim(ptr);
				if (!strcmp(var, "DNS"))
					profile->dnses = g_list_append(profile->dnses, strdup(ptr));
				if (!strcmp(var, "DOMAIN") && !strlen(profile->domain))
					strncpy(profile->domain, ptr, PATH_MAX);
				if (!strcmp(var, "DESC") && !strlen(profile->desc))
					strncpy(profile->desc, ptr, PATH_MAX);
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
				if(!strcmp(var, "DHCP_OPTS") && !strlen(iface->dhcp_opts))
					strncpy(iface->dhcp_opts, ptr, PATH_MAX);
				if(!strcmp(var, "ESSID") && !strlen(iface->essid))
					strncpy(iface->essid, ptr, ESSID_MAX_SIZE);
				if(!strcmp(var, "KEY") && !strlen(iface->key))
					strncpy(iface->key, ptr, ENCODING_TOKEN_MAX);
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
		FREE(ptr);
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
				FREE(ptr);
			}
		ptr = g_strdup_printf("ifconfig %s down", iface->name);
		nc_system(ptr);
		FREE(ptr);
	}

	if(g_list_length(iface->post_downs))
		for (i=0; i<g_list_length(iface->post_downs); i++)
			nc_system((char*)g_list_nth_data(iface->post_downs, i));

	return(ret);
}

int ifup(interface_t *iface)
{
	int dhcp, ret=0, i;
	char *ptr;

	if(g_list_length(iface->pre_ups))
		for (i=0; i<g_list_length(iface->pre_ups); i++)
			ret += nc_system((char*)g_list_nth_data(iface->pre_ups, i));

	dhcp = is_dhcp(iface);
	// initialize the device
	if(strlen(iface->mac))
	{
		ptr = g_strdup_printf("ifconfig %s hw ether %s", iface->name, iface->mac);
		ret += nc_system(ptr);
		FREE(ptr);
	}
	if(strlen(iface->essid))
	{
		ptr = g_strdup_printf("iwconfig %s essid %s", iface->name, iface->essid);
		ret += nc_system(ptr);
		FREE(ptr);
	}

	if(strlen(iface->key))
	{
		ptr = g_strdup_printf("iwconfig %s key %s", iface->name, iface->key);
		ret += nc_system(ptr);
		FREE(ptr);
	}

	// set up the interface
	if(dhcp)
	{
		if(strlen(iface->dhcp_opts))
			ptr = g_strdup_printf("dhcpcd %s %s", iface->dhcp_opts, iface->name);
		else
			ptr = g_strdup_printf("dhcpcd -t 10 %s", iface->name);
		ret += nc_system(ptr);
		FREE(ptr);
	}
	else if(g_list_length(iface->options)==1)
	{
		ptr = g_strdup_printf("ifconfig %s %s",
			iface->name, (char*)g_list_nth_data(iface->options, 0));
		ret += nc_system(ptr);
		FREE(ptr);
	}
	else
	{
		ptr = g_strdup_printf("ifconfig %s 0.0.0.0", iface->name);
		ret += nc_system(ptr);
		FREE(ptr);
		for (i=0; i<g_list_length(iface->options); i++)
		{
			ptr = g_strdup_printf("ifconfig %s:%d %s",
				iface->name, i+1, (char*)g_list_nth_data(iface->options, i));
			ret += nc_system(ptr);
			FREE(ptr);
		}
	}

	// setup the gateway
	if(!dhcp && strlen(iface->gateway))
	{
		ptr = g_strdup_printf("route add %s", iface->gateway);
		ret += nc_system(ptr);
		FREE(ptr);
	}
	if(g_list_length(iface->post_ups))
		for (i=0; i<g_list_length(iface->post_ups); i++)
			ret += nc_system((char*)g_list_nth_data(iface->post_ups, i));

	return(ret);
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

int is_wireless_device(char *dev)
{
	FILE *pp;
	char *ptr;
	char line[256];
	struct stat buf;

	if(stat("/usr/sbin/iwconfig", &buf))
	{
		/* no iwconfig found */
		return(0);
	}

	ptr = g_strdup_printf("iwconfig %s 2>&1", dev);
	pp = popen(ptr, "r");
	FREE(ptr);
	if(pp==NULL)
		return(0);
	while(fgets(line, 255, pp))
		if(strstr(line, "no wireless extensions"))
		{
			pclose(pp);
			return(0);
		}
	pclose(pp);
	return(1);
}

void dialog_backtitle(char *title)
{
	FILE *fp;
	char line[128];

	if ((fp = fopen(VERSIONFILE, "r")) == NULL)
		return;
	fgets(line, 127, fp);
	line[strlen(line)-1]='\0';
	fclose(fp);
	if(dialog_vars.backtitle)
		FREE(dialog_vars.backtitle);
	dialog_vars.backtitle=g_strdup_printf("%s - %s %s", title, line, _("Setup"));
	dlg_put_backtitle();
	dlg_clear();
}

int dialog_confirm(void)
{
	int ret;
	dialog_vars.defaultno=1;
	ret = dialog_yesno(_("Exit"), _("Are you sure you want to exit?"), 0, 0);
	dialog_vars.defaultno=0;
	if(ret==DLG_EXIT_OK)
		return(1);
	else
		return(0);
}

void dialog_exit(void)
{
	end_dialog();
	exit(0);
}

char *dialog_ask(char *title, char *desc, char *init)
{
	char my_buffer[MAX_LEN + 1] = "";
	int ret;

	dlg_put_backtitle();
	dlg_clear();
	while(1)
	{
		dialog_vars.input_result = my_buffer;
		ret = dialog_inputbox(title, desc, 0, 0, init, 0);
		if (ret != DLG_EXIT_CANCEL)
			break;
		if(dialog_confirm())
			dialog_exit();
	}
	return(strdup(my_buffer));
}

char *dialog_mymenu(const char *title, const char *cprompt, int height, int width,
	int menu_height, int item_no, char **items)
{
	int ret;
	char my_buffer[MAX_LEN + 1] = "";

	while(1)
	{
		dialog_vars.input_result = my_buffer;
		dlg_put_backtitle();
		dlg_clear();
		ret = dialog_menu(title, cprompt, height, width, menu_height,
			item_no, items);
		if (ret != DLG_EXIT_CANCEL)
			break;
		if(dialog_confirm())
			dialog_exit();
	}
	return(strdup(dialog_vars.input_result));
}

int dialog_myyesno(char *title, char *desc)
{
	int ret;

	dlg_put_backtitle();
	dlg_clear();
	ret = dialog_yesno(title, desc, 0, 0);
	if(ret==DLG_EXIT_OK)
		return(1);
	else
		return(0);
}

char *selnettype()
{
	int typenum=4;
	char *types[] =
	{
		"dhcp", _("Use a DHCP server"),
		"static", _("Use a static IP address"),
		"dsl", _("DSL connection"),
		"lo", _("No network - loopback connection only")
	};
	return(dialog_mymenu(_("Select network connection type"),
		_("Now we need to know how your machine connects to the network.\n"
		"If you have an internal network card and an assigned IP address, gateway, and DNS, use 'static' "
		"to enter these values. Also may use this if you have a DSL connection.\n"
		"If your IP address is assigned by a DHCP server (commonly used by cable modem services), select 'dhcp'. \n"
		"If you just have a network card to connect directly to a DSL modem, then select 'dsl'. \n"
		"Finally, if you do not have a network card, select the 'lo' choice. \n"),
		0, 0, 0, typenum, types));
}

int dsl_hook(void)
{
	struct stat buf;
	int ret;

	// do we have adslconfig?
	if(stat("/usr/sbin/adslconfig", &buf))
		return(0);
	if(dialog_myyesno(_("DSL configuration"), _("Do you want to configure a DSL connetion now?")))
	{
		if(!nco_fast)
			return(nc_system("adslconfig"));
		else
		{
			ret = nc_system("adslconfig --fast");
			nc_system("mkdir /var/run");
			nc_system("mount -t devpts none /dev/pts");
			nc_system("pppoe-connect >/dev/tty4 2>/dev/tty4 &");
			return(ret);
		}
	}
	return(0);
}

char *hostname(char *ptr)
{
	char *str=ptr;

	while(*ptr++!='.');
	*--ptr='\0';
	return(str);
}

/* Copyright 1994 by David Niemi.  Written in about 30 minutes on 13 Aug.
 * The author places no restrictions on the use of this program, provided
 * that this copyright is preserved in any derived source code.
 */
char *netaddr(char *ip, char *nm)
{
	unsigned long netmask, ipaddr, netaddr;
	int in[4], i;
	unsigned char na[4];

	// sanility checks for netmask
	if (4 != sscanf(ip,"%d.%d.%d.%d", &in[0],&in[1],&in[2],&in[3]))
		// invalid netmask
		return(NULL);
	for (i=0; i<4; ++i)
		if (in[i]<0 || in[i]>255)
			// invalid octet in netmask
			return(NULL);
	netmask = in[3] + 256 * (in[2] + 256 * (in[1] + 256 * in[0]));

	// sanility check for ip
	if (4 != sscanf(nm,"%d.%d.%d.%d", &in[0],&in[1],&in[2],&in[3]))
		// invalid ip
		return(NULL);
	for (i=0; i<4; ++i)
		if (in[i]<0 || in[i]>255)
			// invalied octet in ip
			return(NULL);

	ipaddr = in[3] + 256 * (in[2] + 256 * (in[1] + 256 * in[0]));

	netaddr = ipaddr & netmask;
	na[0] = netaddr / 256 / 256 / 256;
	na[1] = (netaddr / 256 / 256) % 256;
	na[2] = (netaddr / 256) % 256;
	na[3] = netaddr % 256;

	return(g_strdup_printf("%d.%d.%d.%d", na[0], na[1], na[2], na[3]));
}

int writeconfig(char *host, char *nettype, char *dhcphost, char *ipaddr, char *netmask, char *gateway, char *dns,
	char *essid, char *key)
{
	// TODO: here the profile name ('default') and eth0 is hardwired
	FILE *fp;
	char *network=NULL;
	int fakeip=0;

	fp = fopen(NC_PATH "/default", "w");
	if(fp==NULL)
		return(1);
	if(dns != NULL && strlen(dns))
	{
		fprintf(fp, "[options]\n");
		fprintf(fp, "dns = %s\n", dns);
	}
	if(strcmp(nettype, "lo"))
		fprintf(fp, "[eth0]\n");
	if(essid != NULL && strlen(essid))
		fprintf(fp, "essid = %s\n", essid);
	if(key != NULL && strlen(key))
		fprintf(fp, "key = %s\n", key);
	if(!strcmp(nettype, "dhcp"))
	{
		fprintf(fp, "options = dhcp\n");
		if(strlen(dhcphost))
			fprintf(fp, "dhcp_opts = -t 10 -h %s\n", dhcphost);
	}
	else if (!strcmp(nettype, "static"))
	{
		if(strlen(ipaddr) && strlen(netmask))
			fprintf(fp, "options = %s netmask %s\n", ipaddr, netmask);
		if(strlen(ipaddr) && !strlen(netmask))
			fprintf(fp, "options = %s\n", ipaddr);
		if(strlen(gateway))
			fprintf(fp, "gateway = default gw %s\n", gateway);
	}
	fclose(fp);

	fp = fopen("/etc/HOSTNAME", "w");
	if(fp==NULL)
		return(1);
	fprintf(fp, "%s\n", host);
	fclose(fp);

	if(strcmp(nettype, "static"))
	{
		fakeip=1;
		ipaddr=strdup("127.0.0.1");
		network=strdup("127.0.0.0");
	}
	else
		network=netaddr(ipaddr, netmask);

	fp = fopen("/etc/hosts", "w");
	if(fp==NULL)
		return(1);
	fprintf(fp, "#\n"
		"# hosts         This file describes a number of hostname-to-address\n"
		"#               mappings for the TCP/IP subsystem.  It is mostly\n"
		"#               used at boot time, when no name servers are running.\n"
		"#               On small systems, this file can be used instead of a\n"
		"#               'named' name server.  Just add the names, addresses\n"
		"#               and any aliases to this file...\n"
		"#\n\n"
		"# For loopbacking.\n"
		"127.0.0.1               localhost\n");
	fprintf(fp, "%s\t%s %s\n", ipaddr, host, hostname(host));
	fprintf(fp, "\n# End of hosts.\n");
	fclose(fp);

	fp = fopen("/etc/networks", "w");
	if(fp==NULL)
		return(1);
	fprintf(fp, "#\n"
		"# networks      This file describes a number of netname-to-address\n"
		"#               mappings for the TCP/IP subsystem.  It is mostly\n"
		"#               used at boot time, when no name servers are running.\n"
		"#\n\n"
		"loopback        127.0.0.0\n");
	fprintf(fp, "localnet        %s\n", network);
	fprintf(fp, "\n# End of networks.\n");
	fclose(fp);

	if(fakeip)
	{
		FREE(ipaddr);
		ipaddr=NULL;
	}
	FREE(network);
	return(0);
}

int dialog_config()
{
	FILE *input = stdin;
	char *host, *nettype;
	char *dhcphost=NULL;
	char *ipaddr=NULL, *netmask=NULL, *gateway=NULL, *dns=NULL, *essid=NULL, *key=NULL;

	dialog_state.output = stderr;
	init_dialog(input, dialog_state.output);
	dialog_backtitle(_("Network configuration"));

	if(!nco_fast)
		host = dialog_ask(_("Enter hostname"), _("We'll need the name you'd like to give your host.\n"
		"The full hostname is needed, such as:\n\n"
		"frugalware.example.net\n\n"
		"Enter full hostname:"), "frugalware.example.net");
	else
		host = strdup("frugalware.example.net");
	nettype = selnettype();
	if(strcmp(nettype, "lo") && is_wireless_device("eth0"))
	{
		essid = dialog_ask(_("Extended network name"), _("It seems that this network card has a wireless "
			"extension. In order to use it, you must set your extended netwok name (ESSID). Enter your ESSID:"),
			NULL);
		key = dialog_ask(_("Encryption key"), _("If you have an encryption key, then please enter it below.\n"
			"Examples: 4567-89AB-CD or  s:password"), NULL);
	}
	if(!strcmp(nettype, "dhcp"))
		dhcphost = dialog_ask(_("Set DHCP hostname"), _("Some network providers require that the DHCP hostname be"
			"set in order to connect. If so, they'll have assigned a hostname to your machine. If you were"
			"assigned a DHCP hostname, please enter it below. If you do not have a DHCP hostname, just"
			"hit enter."), NULL);
	else if(!strcmp(nettype, "static"))
	{
		ipaddr = dialog_ask(_("Enter ip address"), _("Enter your IP address for the local machine."), NULL);
		netmask = dialog_ask(_("Enter netmask for local network"),
			_("Enter your netmask. This will generally look something like this: 255.255.255.0\n"
			"If unsure, just hit enter."), "255.255.255.0");
		gateway = dialog_ask(_("Enter gateway address"), _("Enter the address for the gateway on your network. "
			"If you don't have a gateway on your network just hit enter, without entering any ip address."),
			NULL);
		dns = dialog_ask(_("Select nameserver"), _("Please give the IP address of the name server to use. You can"
			"add more Domain Name Servers later by editing /etc/sysconfig/network/default.\n"
			"If you don't have a name server on your network just hit enter, without entering any ip address."),
			NULL);
	}
	if(!strcmp(nettype, "static") || !strcmp(nettype, "dsl"))
		dsl_hook();

	if(dialog_myyesno(_("Adjust configuration files"), _("Accept these settings and adjust configuration files?"))
		&& !nco_dryrun)
		writeconfig(host, nettype, dhcphost, ipaddr, netmask, gateway, dns, essid, key);

	FREE(host);
	FREE(nettype);
	FREE(dhcphost);
	FREE(ipaddr);
	FREE(netmask);
	FREE(gateway);
	FREE(dns);
	FREE(essid);
	FREE(key);
	end_dialog();
	return(0);
}

int main(int argc, char **argv)
{
	int opt;
	int option_index;
	static struct option opts[] =
	{
		{"help",           no_argument,       0, 'h'},
		{"fast",           no_argument,       0, 'f'},
		{"dry-run",        no_argument,       0, 1000},
		{0, 0, 0, 0}
	};
	char *fn=NULL;
	int ret=0, i;
	profile_t *profile;

	while((opt = getopt_long(argc, argv, "hf", opts, &option_index)))
	{
		if(opt < 0)
			break;
		switch(opt)
		{
			case 1000: nco_dryrun = 1; break;
			case 'h':  nco_usage  = 1; break;
			case 'f':  nco_fast   = 1; break;
		}
	}
	i18ninit();
	if(nco_usage)
	{
		usage(argv[0]);
		return(0);
	}

	if(optind < argc)
	{
		if(!strcmp("list", argv[optind]))
		{
			listprofiles();
			return(0);
		}
		if((fn=lastprofile()) || !strcmp("stop", argv[optind]) || !strcmp("status", argv[optind]))
		{
			if((!strcmp("stop", argv[optind])) && !fn)
				return(127);
			if((!strcmp("status", argv[optind])) && !fn)
			{
				printf(_("No profile loaded.\n"));
				return(1);
			}
			else if ((!strcmp("status", argv[optind])) && fn)
			{
				printf(_("Current profile: %s\n"), fn);
				return(0);
			}
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
			ret += ifup((interface_t*)g_list_nth_data(profile->interfaces, i));
		setdns(profile);
		setlastprofile(fn);
		FREE(fn);
	}
	else
		dialog_config();
	return(ret);
}
