#include <stdio.h>
#include <stdlib.h>

#include <setup.h>
#include "bar.h"

plugin_t plugin =
{
	"bar",
	main,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}

int main(void)
{
	printf(_("bar\n"));
	printf("%s\n", getenv("LC_ALL"));
	printf("%s\n", getenv("CHARSET"));
	return(0);
}
