#include <stdio.h>
#include <dialog.h>

#include <setup.h>
#include <util.h>
#include "common.h"

plugin_t plugin =
{
	"restart",
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}

int run(GList **config)
{
	char *version = get_version();

	system(g_strdup_printf("%s/usr/bin/eject %s >%s 2>%s", TARGETDIR,
		(char*)data_get(*config, "srcdev"), LOGDEV, LOGDEV));

	dlg_put_backtitle();
	dlg_clear();
	dialog_msgbox(_("Setup complete"), g_strdup_printf(_("System "
		"installation and configuration is successfully completed. We "
		"hope that %s will take you content. Please remove "
		"the media from your drive and press ENTER to reboot."),
		version), 0, 0, 1);
	exit_perform();
	FREE(version);
	return(0);
}
