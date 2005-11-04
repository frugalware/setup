#include <locale.h>
#include <libintl.h>
#include <glib.h>

#ifndef _
#define _(text) gettext(text)
#endif

#define LOGDEV "/dev/tty4"

#define PLUGDIR "plugins"

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
