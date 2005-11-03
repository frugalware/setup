#include <stdio.h>

#include <setup.h>
#include "skel.h"

plugin_t plugin =
{
	"skel",
	main,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}

int run(GList **config)
{
	data_t *data;

	// sample: gets the string titled "stuff" from the config list
	printf("%s\n", (char*)data_get(*config, "stuff"));
	
	// sample: adds a "content" string titled "stuff" to the config list
	data_put(config, "stuff", "content");
	return(0);
}
