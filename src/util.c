/*
 *  util.c for Frugalware setup
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
#ifdef DIALOG
#include <dialog.h>
#endif
#ifdef GTK
#include <gtk/gtk.h>
#endif

#include "setup.h"
#include "util.h"

#define VERSIONFILE "/etc/frugalware-release"

char *gen_backtitle(char *section)
{
	char *backtitle;
	char *version = get_version();
	MALLOC(backtitle, 256);

	snprintf(backtitle, 255, "%s - %s %s", section, version, _("Setup"));
	FREE(version);
	return(backtitle);
}

char *get_version(void)
{
	FILE *fp;
	char *version;
	MALLOC(version, 128);

	if ((fp = fopen(VERSIONFILE, "r")) == NULL)
	{
		perror(_("Could not open file for reading"));
		return(NULL);
	}
	fgets(version, 127, fp);
	version[strlen(version)-1]='\0';
	fclose(fp);
	return(version);
}

data_t *data_new(void)
{
	data_t *data=NULL;
	
	data = (data_t*)malloc(sizeof(data_t));
	if(data==NULL)
		return(NULL);
	data->name=NULL;
	data->data=NULL;
	return(data);
}

void *data_get(GList *config, char *title)
{
	int i;
	data_t *data;
	
	for (i=0; i<g_list_length(config); i++)
	{
		data = g_list_nth_data(config, i);
		if(!strcmp(title, data->name))
			return data->data;
	}
	return(NULL);
}

void data_put(GList **config, char *name, void *data)
{
	data_t *dp = data_new();
	dp->name = name;
	dp->data = data;
	(*config) = g_list_append((*config), dp);
}

int exit_confirm(void)
{
#ifdef DIALOG
	int ret;
	dialog_vars.defaultno=1;
	ret = dialog_yesno(_("Exit from the installer"),
		_("Are you sure you want to exit from the installer?"), 0, 0);
	dialog_vars.defaultno=0;
	if(ret==DLG_EXIT_OK)
		return(1);
	else
		return(0);
#endif
}

int exit_perform(void)
{
#ifdef DIALOG
	end_dialog();
#endif

#ifdef FINAL
	system(g_strdup_printf("/sbin/reboot >%s 2>%s", LOGDEV, LOGDEV));
#else
	printf("/sbin/reboot\n");
#endif
	exit(1);
}

char **glist4dialog(GList *list, char *blank)
{
	int i;
	char **array;

	MALLOC(array, 2*g_list_length(list)*sizeof(char*));
	
	for (i=0; i<2*g_list_length(list); i=i+2)
	{
		array[i] = (char*)g_list_nth_data(list, i/2);
		array[i+1] = blank;
	}
	return(array);
}

int fw_system(char* cmd)
{
#ifdef FINAL
	return(system(g_strdup_printf("%s >%s", cmd, LOGDEV)));
#else
	return(system(g_strdup_printf("echo %s >%s", cmd, LOGDEV)));
#endif
}

#ifdef DIALOG
int fw_menu(const char *title, const char *cprompt, int height, int width,
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
		if(exit_confirm())
			exit_perform();
	}
	return(0);
}

int fw_init_dialog(void)
{
	FILE *input = stdin;
	dialog_state.output = stderr;
	
	init_dialog(input, dialog_state.output);
	return(0);
}

int fw_end_dialog(void)
{
	printf("\033[H\033[2J");
	dlg_clear();
	end_dialog();
	return(0);
}

char **parts2dialog(GList *list)
{
	int i;
	char **array;

	MALLOC(array, g_list_length(list)*sizeof(char*));
	for (i=0; i<g_list_length(list); i++)
	{
		array[i] = (char*)g_list_nth_data(list, i);
	}
	return(array);
}

GList* fw_checklist(const char *title, const char *cprompt, int height,
	int width, int menu_height, int item_no, char **items, int flag)
{
	int ret;
	char my_buffer[MAX_LEN + 1] = "";
	char *ptr, *ptrn;
	GList *list=NULL;

	while(1)
	{
		dialog_vars.input_result = my_buffer;
		ret = dialog_checklist(title, cprompt, height, width,
			menu_height, item_no, items, flag);
		if (ret != DLG_EXIT_CANCEL)
			break;
		if(exit_confirm())
			exit_perform();
	}

	if(strlen(dialog_vars.input_result)==0)
		// no item selected
		return(list);

	ptr=strstr(dialog_vars.input_result, "\"")+1;
	while(strstr(ptr, "\" \""))
	{
		ptrn=strstr(ptr, "\" \"");
		if(ptrn)
		{
			*ptrn='\0';
			ptrn += 3;
		}
		list = g_list_append(list, ptr);
		ptr=ptrn;
	}
	ptrn=ptr+strlen(ptr)-1;
	*ptrn='\0';
	list = g_list_append(list, strdup(ptr));
	return(list);
}

int fw_info(char *title, char *msg)
{
	dlg_put_backtitle();
	dlg_clear();
	return(dialog_msgbox(title, msg, 0, 0, 0));
}
#endif

#ifdef GTK
void fw_init_gtk(void)
{
	gtk_init(NULL, NULL);
	gtk_set_locale();
}

void fw_end_gtk(GtkWidget *win)
{
	gtk_widget_destroy(win);
	gtk_main_quit();
}

void gtk_draw_framework()
{
	GtkWidget *mainwindow;
	GtkWidget *hbox, *vbox, *label, *button;
	
	mainwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(mainwindow), "destroy", G_CALLBACK(fw_end_gtk), NULL);
	gtk_widget_show(mainwindow);

	vbox = gtk_vbox_new(FALSE, 0);
	gtk_container_add(mainwindow, vbox);
	gtk_widget_show(vbox);

	hbox = gtk_hbox_new(FALSE, 0);

	label = gtk_label_new("Valamiiiii");
	gtk_box_pack_start(GTK_BOX(vbox), label, 1,1,1);
	gtk_widget_show(label);

	gtk_box_pack_start(GTK_BOX(vbox), hbox, 1,1,1);
	gtk_widget_show(hbox);

	label = gtk_label_new("Valamiiiii2/a");
	gtk_box_pack_start(GTK_BOX(hbox), label, 1,1,1);
	gtk_widget_show(label);

	frame = gtk_frame_new("Valami");
	gtk_widget_show_all(frame);
	gtk_box_pack_end(GTK_BOX(hbox), frame, 1,1,1);
	label = gtk_label_new("Valamiiiii2/b");

	hbox = gtk_hbox_new(FALSE, 0);

	button = gtk_button_new_with_label("Back");
	gtk_box_pack_start(GTK_BOX(hbox), button, 1,1,1);
	gtk_widget_show(hbox);
	gtk_widget_show(button);
	button = gtk_button_new_with_label("Next");
	gtk_box_pack_start(GTK_BOX(hbox), button, 1,1,1);
	g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(plugin_next), NULL);
	gtk_widget_show(hbox);
	gtk_widget_show(button);

	gtk_box_pack_start(GTK_BOX(vbox), hbox, 0,1,0);

}

int plugin_next(GtkWidget *w, gpointer user_data)
{
	printf("Plugin name\n");
}

#endif
