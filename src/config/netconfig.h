/*
 *  netconfig.h for Frugalware setup
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

#define MAC_MAX_SIZE 17
#define ESSID_MAX_SIZE 32
#define ENCODING_TOKEN_MAX   32
#define GW_MAX_SIZE 26

#define NC_LOCK "/var/run/netconfig"

#define min(p, q)  ((p) < (q) ? (p) : (q))

typedef struct __interface_t {
	char name[IF_NAMESIZE+1];
	GList *options;
	GList *pre_ups;
	GList *post_ups;
	GList *pre_downs;
	GList *post_downs;
	char mac[MAC_MAX_SIZE+1];
	char dhcp_opts[PATH_MAX+1];
	char essid[ESSID_MAX_SIZE+1];
	char key[ENCODING_TOKEN_MAX+1];
	char gateway[GW_MAX_SIZE+1];
} interface_t;

typedef struct __profile_t {
	GList *dnses;
	char desc[PATH_MAX+1];
	char domain[PATH_MAX+1];
	GList *interfaces; // GList of interface_t*
} profile_t;

