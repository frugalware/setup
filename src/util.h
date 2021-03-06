/*
 *  util.h for Frugalware setup
 * 
 *  Copyright (c) 2005-2007 by Miklos Vajna <vmiklos@frugalware.org>
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

#define MALLOC(p, b) { if((b) > 0) \
	{ p = malloc(b); if (!(p)) \
	{ fprintf(stderr, "malloc failure: could not allocate %d bytes\n", (int)(b)); \
	exit(1); }} else p = NULL; }
#define FREE(p) { if (p) { free(p); (p) = NULL; }}

#ifdef GTK
    #include <gtk/gtk.h>

GtkWidget *frame;
#endif

char *get_version(void);
char *gen_backtitle(char *section);
data_t *data_new(void);
void *data_get(GList *config, char *title);
void data_put(GList **config, char *name, void *data);
int eject(char *dev, char *target);
int copyfile(char *src, char *dest);
int exit_fail(void);
int exit_confirm(void);
int exit_perform(void);
char **glist4dialog(GList *list, char *blank);
int fw_system(char* cmd);
int fw_system_interactive(char* cmd);
int makepath(char *path);
int rmrf(char *path);
int umount_if_needed(char *sourcedir);
char *drop_version(char *str);
char *g_list_display(GList *list, char *sep);
GList *g_list_strremove(GList *list, char *str);
int msg(char *str);
int disable_cache(char *path);

#ifdef DIALOG
int fw_menu(const char *title, const char *cprompt, int height, int width,
	int menu_height, int item_no, char **items);
int fw_inputbox(const char *title, const char *cprompt, int height, int width,
	const char *init, const int password);
GList* fw_checklist(const char *title, const char *cprompt, int height,
	int width, int menu_height, int item_no, char **items, int flag);
int fw_info(char *title, char *msg);
int fw_init_dialog(void);
int fw_end_dialog(void);
char **glist2dialog(GList *list);
#endif
#ifdef GTK
void fw_init_gtk(void);
void fw_end_gtk(GtkWidget *win);
void gtk_draw_framework();
int plugin_next();
#endif

void signal_handler(int signum);
void show_menu(GList *plugin_list, int *state);
int setup_log(char *file, int line, char *fmt, ...);
#define LOG(fmt, args...) setup_log(__FILE__, __LINE__, fmt, ##args)
void cb_log(unsigned short level, char *msg);
char *fsize(int length);
