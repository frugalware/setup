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
	// hardwire this for now
	setlocale(LC_ALL, "en_US");
	bindtextdomain("setup", "/usr/share/locale");
	bind_textdomain_codeset("setup", "ISO-8859-2");
	textdomain("setup");

	return(0);
}
