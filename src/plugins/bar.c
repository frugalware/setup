#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <setup.h>
#include "bar.h"

plugin_t plugin =
{
	"bar",
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}

int run(GList **config)
{
	int i;
	printf(_("bar\n"));
	printf("%s\n", getenv("LC_ALL"));
	printf("%s\n", getenv("CHARSET"));
	// for debugging purposes
	fflush(stdout);
	sleep(300);
	return(0);
}
