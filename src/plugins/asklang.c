#include <stdio.h>

#include <setup.h>
#include "asklang.h"

plugin_t plugin =
{
	"asklang",
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
