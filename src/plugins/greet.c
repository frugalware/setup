#define _GNU_SOURCE
#include <stdio.h>
#include <dialog.h>

#include <setup.h>
#include <util.h>
#include "common.h"

plugin_t plugin =
{
	"greet",
	run,
	NULL // dlopen handle
};

plugin_t *info()
{
	return &plugin;
}

int run(GList **config)
{
	FILE *input = stdin;
	dialog_state.output = stderr;
	char *version = get_version();
	char *title=NULL;

	asprintf(&title, _("Welcome to %s"), version);

	init_dialog(input, dialog_state.output);

	dialog_vars.backtitle=gen_backtitle(_("Welcome"));
	dlg_put_backtitle();
	dialog_msgbox(title, _("Welcome among the users of Frugalware!\n\n"
		"The aim of creating Frugalware was to help you make your work "
		"faster and simpler. We hope that you will like our "
		"product.\n\n"
		"The Frugalware Developer Team"), 0, 0, 1);
	FREE(version);

	end_dialog();
	return(0);
}
