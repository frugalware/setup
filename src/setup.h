#include <locale.h>
#include <libintl.h>

#define _(text) gettext(text)

#define PLUGDIR "plugins"

#define SHARED_LIB_EXT ".so"

typedef struct
{
	char *name;
	int (*main)(void);
	void *handle;
} plugin_t;
