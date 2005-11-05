#include <stdio.h>
#include <dialog.h>
#include <unistd.h>

#include <setup.h>
#include <util.h>
#include "common.h"

plugin_t plugin =
{
	"configsource",
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}

int run(GList **config)
{
	dlg_put_backtitle();
	if((char*)data_get(*config, "netinstall")!=NULL)
	{
		system(NETCONFIGSCRIPT);
		system(g_strdup_printf("%s >%s 2>%s", INTERFACESSCRIPT,
			LOGDEV, LOGDEV));
		// TODO: mirrorconf();
	}
	return(0);
}
