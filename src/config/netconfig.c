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
	char *profile=NULL;
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
			profile = strdup("default");
		else
			profile = argv[optind];
	}
	else
	{
		init_dialog(input, dialog_state.output);
		dialog_msgbox("title", "content", 0, 0, 0);
		end_dialog();
	}
	return(0);
}
