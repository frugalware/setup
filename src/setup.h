#include <locale.h>
#include <libintl.h>
#include <glib.h>

#ifndef _
#define _(text) gettext(text)
#endif

#define LOGDEV "/dev/tty4"
#define SOURCEDIR "/mnt/source"
#define TARGETDIR "/mnt/target"

#ifdef FINAL
#define PLUGDIR "/lib/plugins"
#define HOTPLUGSCRIPT "/etc/rc.d/rc.hotplug"
#define NETCONFIGSCRIPT "netconfig"
#define INTERFACESSCRIPT "/etc/rc.d/rc.interfaces"
#define PACCONFPATH "/etc/pacman.d/"
#else
#define PLUGDIR "plugins"
#define HOTPLUGSCRIPT "echo /etc/rc.d/rc.hotplug"
#define NETCONFIGSCRIPT "echo netconfig >/dev/tty4"
#define INTERFACESSCRIPT "echo /etc/rc.d/rc.interfaces"
#define PACCONFPATH "plugins"
#endif

#ifndef STABLE
#define PACCONF "frugalware-current"
#define PACEXCONF "extra-current"
#else
#define PACCONF "frugalware"
#define PACEXCONF "extra"
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
