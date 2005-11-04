#include <locale.h>
#include <libintl.h>
#include <glib.h>

#ifndef _
#define _(text) gettext(text)
#endif

#define LOGDEV "/dev/tty4"
#define SOURCEDIR "/mnt/source"

#ifdef FINAL
#define PLUGDIR "/lib/plugins"
#define HOTPLUGSCRIPT "/etc/rc.d/rc.hotplug"
#else
#define PLUGDIR "plugins"
#define HOTPLUGSCRIPT "echo /etc/rc.d/rc.hotplug"
#endif

#define SHARED_LIB_EXT ".so"

typedef struct
{
	char *name;
	int (*run)(GList **config);
	void *handle;
} plugin_t;

typedef struct
{
	char *name;
	void *data;
} data_t;
