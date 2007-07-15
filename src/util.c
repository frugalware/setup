/*
 *  util.c for Frugalware setup
 * 
 *  Copyright (c) 2005-2007 by Miklos Vajna <vmiklos@frugalware.org>
 *  Copyright (c) 2006 by Alex Smith <alex@alex-smith.me.uk>
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
#include <sys/stat.h>
#include <sys/mount.h>
#include <linux/cdrom.h>
#include <dirent.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <signal.h>
#include <stdarg.h>
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
	int i;
	data_t *dp;
	
	for (i=0; i<g_list_length(*config); i++)
	{
		dp = g_list_nth_data(*config, i);
		if(!strcmp(name, dp->name))
		{
			dp->data=data;
			return;
		}
	}
	
	dp = data_new();
	dp->name = name;
	dp->data = data;
	(*config) = g_list_append((*config), dp);
}

int eject(char *dev, char *target)
{
	int fd;

	dialog_msgbox(_("Setup complete"), _("Ejecting installation media..."),
			0, 0, 0);

	umount2(target,MNT_FORCE);

	if((fd = open(dev, O_RDONLY|O_NONBLOCK))==-1)
		return(1);
	if((ioctl(fd, CDROMEJECT)) == -1)
		return(1);
	close(fd);
	return(0);
}

int copyfile(char *src, char *dest)
{
	FILE *in, *out;
	size_t len;
	char buf[4097];

	in = fopen(src, "r");
	if(in == NULL)
		return(1);

	out = fopen(dest, "w");
	if(out == NULL)
		return(1);

	while((len = fread(buf, 1, 4096, in)))
		fwrite(buf, 1, len, out);

	fclose(in);
	fclose(out);
	return(0);
}


int exit_fail(void)
{
#ifdef DIALOG
	int ret;
	ret = dialog_yesno(_("Installation error"),
		_("Errors occured while installing selected packages. It is "
			"recommended to abort the installation, however you "
			"may want to ignore this problem. Do you want to leave "
			"the installer?"), 0, 0);
	if(ret==DLG_EXIT_OK)
		return(1);
	else
		return(0);
#endif
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

	system(g_strdup_printf("/sbin/reboot >%s 2>%s", LOGDEV, LOGDEV));
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

int makepath(char *path)
{
	char *orig, *str, *ptr;
	char full[PATH_MAX] = "";
	mode_t oldmask;

	oldmask = umask(0000);

	orig = strdup(path);
	str = orig;
	while((ptr = strsep(&str, "/")))
		if(strlen(ptr))
		{
			struct stat buf;

			strcat(full, "/");
			strcat(full, ptr);
			if(stat(full, &buf))
				if(mkdir(full, 0755))
				{
					free(orig);
					umask(oldmask);
					return(1);
				}
		}
	free(orig);
	umask(oldmask);
	return(0);
}

/* does the same thing as 'rm -rf' */
int rmrf(char *path)
{
	int errflag = 0;
	struct dirent *dp;
	DIR *dirp;
	char name[PATH_MAX];

	if(!unlink(path))
		return(0);
	else
	{
		if(errno == ENOENT)
			return(0);
		else if(errno == EPERM)
		{
			/* fallthrough */
		}
		else if(errno == EISDIR)
		{
			/* fallthrough */
		}
		else if(errno == ENOTDIR)
			return(1);
		else
			/* not a directory */
			return(1);

		if((dirp = opendir(path)) == (DIR *)-1)
			return(1);
		for(dp = readdir(dirp); dp != NULL; dp = readdir(dirp))
			if(dp->d_ino)
			{
				sprintf(name, "%s/%s", path, dp->d_name);
				if(strcmp(dp->d_name, "..") && strcmp(dp->d_name, "."))
					errflag += rmrf(name);
			}
		closedir(dirp);
		if(rmdir(path))
			errflag++;
		return(errflag);
	}
	return(0);
}

int umount_if_needed(char *sourcedir)
{
	FILE *fp;
	char line[PATH_MAX];
	char *dev=NULL;
	char *realdir;
	int i;

	realdir = g_strdup_printf("%s/%s", TARGETDIR, sourcedir);
	
	if ((fp = fopen("/proc/mounts", "r")) == NULL)
	{
		perror(_("Could not open output file for writing"));
		return(1);
	}
	while(!feof(fp))
	{
		if(fgets(line, 256, fp) == NULL)
			break;
		if(strstr(line, realdir))
		{
			for(i=0;i<strlen(line);i++)
				if(line[i]==' ')
					line[i]='\0';
			dev = strdup(line);
		}
	}
	fclose(fp);
	if(dev != NULL)
		return (umount2(dev,0));
	return(0);
}

int fw_system(char* cmd)
{
	char *ptr, line[PATH_MAX];
	FILE *pp;
	LOG("running external command: '%s'", cmd);
	ptr = g_strdup_printf("%s 2>&1", cmd);
	pp = popen(ptr, "r");
	if(!pp)
	{
		LOG("call to popen falied (%s)", strerror(errno));
		return(-1);
	}
	while(!feof(pp))
	{
		if(fgets(line, PATH_MAX, pp) == NULL)
			break;
		line[strlen(line)-1]='\0';
		LOG("> %s", line);
	}
	int ret = pclose(pp);
	FREE(ptr);
	LOG("external command returned with exit code '%d'", ret);
	return (ret);
}

char *drop_version(char *str)
{
	char *ptr;
	int i;

	for(i=0;i<2;i++)
		if((ptr = strrchr(str, '-')))
			*ptr = '\0';
	return(str);
}

char *g_list_display(GList *list, char *sep)
{
	int i, len=0;
	char *ret;

	for (i=0; i<g_list_length(list); i++)
	{
		drop_version((char*)g_list_nth_data(list, i));
		len += strlen((char*)g_list_nth_data(list, i));
		len += strlen(sep)+1;
	}
	if(len==0)
		return(NULL);
	MALLOC(ret, len);
	*ret='\0';
	for (i=0; i<g_list_length(list); i++)
	{
		strcat(ret, (char*)g_list_nth_data(list, i));
		strcat(ret, sep);
	}
	return(ret);
}

GList *g_list_strremove(GList *list, char *str)
{
	int i;

	for(i=0;i<g_list_length(list);i++)
		if(!strcmp(g_list_nth_data(list, i), str))
			return(g_list_remove(list, g_list_nth_data(list, i)));
	return(NULL);
}


int msg(char *str)
{
	printf("\e[01;36m::\e[0m \e[01m%s\e[0m\n", str);
	return(0);
}

int disable_cache(char *path)
{
	DIR *dir;
	struct dirent *ent;
	char *filename;
	char *targetname;

	dir = opendir(path);
	if (!dir)
		return(1);
	while ((ent = readdir(dir)) != NULL)
	{
		if(!strcmp(ent->d_name, ".") || !strcmp(ent->d_name, ".."))
			continue;
		filename = g_strdup_printf("%s/%s", path, ent->d_name);
		targetname = g_strdup_printf("%s/var/cache/pacman/pkg/%s", TARGETDIR, ent->d_name);
		symlink(filename, targetname);
		FREE(filename);
		FREE(targetname);
	}
	closedir(dir);
	return(0);
}

#ifdef DIALOG
int fw_menu(const char *title, const char *cprompt, int height, int width,
	int menu_height, int item_no, char **items)
{
	int ret;
	char my_buffer[MAX_LEN + 1] = "";

	dialog_vars.input_result = my_buffer;
	dlg_put_backtitle();
	dlg_clear();
	ret = dialog_menu(title, cprompt, height, width, menu_height,
		item_no, items);
	if (ret != DLG_EXIT_CANCEL)
		return(0);
	else
		return(-1);
}

int fw_inputbox(const char *title, const char *cprompt, int height, int width,
	const char *init, const int password)
{
	int ret;
	char my_buffer[MAX_LEN + 1] = "";

	dialog_vars.input_result = my_buffer;
	dlg_put_backtitle();
	dlg_clear();
	ret = dialog_inputbox(title, cprompt, height, width, init,
		password);
	if (ret != DLG_EXIT_CANCEL)
		return(0);
	else
		return(-1);
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
	// TODO: sure, there must be a system call for this
	system("clear");
	return(0);
}

char **glist2dialog(GList *list)
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

	dialog_vars.input_result = my_buffer;
	ret = dialog_checklist(title, cprompt, height, width,
		menu_height, item_no, items, flag);
	if (ret == DLG_EXIT_CANCEL)
		return(NULL);

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
		list = g_list_append(list, strdup(ptr));
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

void signal_handler(int signum)
{
	if (signum == SIGSEGV)
	{
		fw_init_dialog();
		dialog_vars.backtitle = gen_backtitle(_("Oops"));
		dlg_put_backtitle();
		dialog_msgbox(_("A problem has occurred"), _("Internal error: Segmentation fault. Please report this issue to http://bugs.frugalware.org"), 7, 40, 1);
		exit_perform();
	} else if (signum == SIGINT) {
		signal(SIGINT, signal_handler);
	} else {
		exit_perform();
	}
}

void show_menu(GList *plugin_list, int *state)
{
	GList *menu = NULL;
	char **items;
	int i;
	plugin_t *plugin;
	int ret;
	char my_buffer[MAX_LEN + 1] = "";

	for(i=0;i<g_list_length(plugin_list);i++)
	{
		plugin = g_list_nth_data(plugin_list, i);
		menu = g_list_append(menu, plugin->name);
		menu = g_list_append(menu, plugin->desc());
	}
	items = glist2dialog(menu);

	dialog_vars.backtitle=gen_backtitle(_("Selecting a task"));
	dlg_put_backtitle();
	dlg_clear();
	while(1)
	{
		dialog_vars.input_result = my_buffer;
		dlg_put_backtitle();
		dlg_clear();
		ret = dialog_menu(_("Select task to continue with"),
		_("You selected to change the way normally Frugalware is "
			"installed. Here are the list of tasks you can continue "
			"with:"),
		0, 0, 0, g_list_length(menu)/2, items);
		if (ret != DLG_EXIT_CANCEL)
			break;
		if(exit_confirm())
			exit_perform();
	}
	for(i=0;i<g_list_length(plugin_list);i++)
	{
		plugin = g_list_nth_data(plugin_list, i);
		if(!strcmp(plugin->name, dialog_vars.input_result))
			*state = i;
	}
	free(items);
}

int setup_log(char *file, int line, char *fmt, ...)
{
	static FILE *ldp = NULL, *lfp = NULL;
	va_list args;
	char str[PATH_MAX], *ptr;
	struct stat buf;

	va_start(args, fmt);
	vsnprintf(str, PATH_MAX, fmt, args);
	va_end(args);

	if(!ldp)
	{
		ldp = fopen(LOGDEV, "w");
		if(!ldp)
			return(-1);
	}
	ptr = g_strdup_printf("%s/%s", TARGETDIR, LOGFILE);
	if(stat(LOGFILE, &buf) && !stat(ptr, &buf))
	{
		fclose(lfp);
		lfp = NULL;
	}
	else
	{
		FREE(ptr);
		ptr = strdup(LOGFILE);
	}
	if(!lfp)
	{
		lfp = fopen(ptr, "a");
		if(!lfp)
			return(-1);
	}
	fprintf(ldp, "%s:%d: %s\n", file, line, str);
	fflush(ldp);
	fprintf(lfp, "%s:%d: %s\n", file, line, str);
	fflush(lfp);
	return(0);
}

void cb_log(unsigned short level, char *msg)
{
	LOG("[libpacman, level %d] %s", level, msg);
}
