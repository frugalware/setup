#define _GNU_SOURCE
#include <stdio.h>
#include <dialog.h>

#include <setup.h>
#include <util.h>
#include "common.h"

plugin_t plugin =
{
	"hotplug",
	20,
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}

int run(GList **config)
{
	dialog_vars.backtitle=gen_backtitle(_("Detecting hardware"));
	dlg_put_backtitle();
	dlg_clear();
	dialog_msgbox(_("Please wait"), _("Scanning for SCSI and PCI cards"),
		0, 0, 0);
	system(g_strdup_printf("%s >%s 2>%s", HOTPLUGSCRIPT,
		LOGDEV, LOGDEV));

	return(0);
}
