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

	// sample: dump the config list
	for (i=0; i<g_list_length(*config); i++)
	{
		data = g_list_nth_data((*config), i);
		printf("detected conf data: %s (%s)\n", data->name, (char*)data->data);
	}
	
	// sample: adds a "content" string titled "stuff" to the config list
	data_t *data = data_new();
	data->name = strdup("stuff");
	data->data = strdup("content");
	*config = g_list_append(*config, data);
	return(0);
}
