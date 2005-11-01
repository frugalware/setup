#include <stdio.h>

#include <setup.h>
#include "foo.h"

plugin_t plugin =
{
	"foo",
	main,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}

int main(void)
{
	printf("foo\n");
	return(0);
}
