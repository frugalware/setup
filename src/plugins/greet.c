#define _GNU_SOURCE
#include <stdio.h>
#include <dialog.h>

#include <setup.h>
#include <util.h>
#include "common.h"

plugin_t plugin =
{
	"greet",
	10,
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
	char *title=NULL;

	asprintf(&title, _("Welcome to %s"), version);

	dialog_vars.backtitle=gen_backtitle(_("Welcome"));
	dlg_put_backtitle();
	dlg_clear();
	dialog_msgbox(title, _("Welcome among the users of Frugalware!\n\n"
		"The aim of creating Frugalware was to help you make your work "
		"faster and simpler. We hope that you will like our "
		"product.\n\n"
		"The Frugalware Developer Team"), 0, 0, 1);
	FREE(version);

	return(0);
}
