/*
 *  setup.h for Frugalware setup
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

#include <locale.h>
#include <libintl.h>
#include <glib.h>

#ifdef _
#undef _
#endif
#define _(text) gettext(text)

#define LOGDEV "/dev/tty4"
#define LOGFILE "/var/log/setup.log"
#define SOURCEDIR "/mnt/source"
#define TARGETDIR "/mnt/target"

#define MKSWAP "/sbin/mkswap"
#define SWAPON "/sbin/swapon"

#define PLUGDIR "/lib/plugins"
#define NETCONFIGSCRIPT "netconfig -f"
#define RAIDCONFIGSCRIPT "raidconfig"
#define INTERFACESSCRIPT "netconfig start"
#define PACCONFPATH "/etc/pacman-g2/repos/"

#ifndef STABLE
#define PACCONF "frugalware-current"
#else
#define PACCONF "frugalware"
#endif

#define EXGRPSUFFIX "-extra"

#define SHARED_LIB_EXT ".so"

typedef struct
{
	char *name;
	char* (*desc)();
	int priority;
	int (*run)(GList **config);
	void *handle;
} plugin_t;

typedef struct
{
	char *name;
	void *data;
} data_t;
