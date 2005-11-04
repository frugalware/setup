#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <setup.h>
#include <util.h>
#include "dolangsh.h"

plugin_t plugin =
{
	"dolangsh",
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}

int run(GList **config)
{
	char *fn;
	FILE* fp;

	fn = strdup("/tmp/setup_XXXXXX");
	mkstemp(fn);
	if ((fp = fopen(fn, "w")) == NULL)
	{
		perror(_("Could not open output file for writing"));
		return(1);
	}
	fprintf(fp, "#!/bin/sh\n\n"
		"# /etc/profile.d/lang.sh\n\n"
		"# Set the system locale\n"
		"# For a list of locales which are supported by this machine, "
		"type: locale -a\n\n");
	fprintf(fp, "export LANG=%s\n", getenv("LANG"));
	fprintf(fp, "export LC_ALL=$LANG\n");
	fprintf(fp, "export CHARSET=%s\n", getenv("CHARSET"));
	fclose(fp);

	// sample: adds a "content" string titled "stuff" to the config list
	data_put(config, "lang.sh", fn);
	return(0);
}
