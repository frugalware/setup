#include <stdio.h>

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
	printf("bar\n");
	return(0);
}
