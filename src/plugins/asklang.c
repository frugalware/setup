#include <stdio.h>
#include <dialog.h>

#include <setup.h>
#include <util.h>
#include "asklang.h"

#define LANGSNUM 2
char *langs[] = {
	"en_US", "English                         ",
	"hu_HU", "Hungarian                       "
};

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
	FILE *input = stdin;
	dialog_state.output = stderr;
	char my_buffer[MAX_LEN + 1] = "";

	init_dialog(input, dialog_state.output);

	dialog_vars.input_result = my_buffer;
	dialog_vars.backtitle=gen_backtitle("Selecting language");
	dlg_put_backtitle();
	dialog_menu("Please select your language",
		"Please select your language from the list. If your language"
		"is not in the list, you probably should choose English.",
		0, 0, 0, LANGSNUM, langs);

	end_dialog();

	setlocale(LC_ALL, dialog_vars.input_result);
	bindtextdomain("setup", "/usr/share/locale");
	// hardwire this for now
	bind_textdomain_codeset("setup", "ISO-8859-2");
	textdomain("setup");

	return(0);
}
