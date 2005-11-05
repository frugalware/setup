#include <stdio.h>
#include <dialog.h>
#include <unistd.h>
#include <stdlib.h>

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

int copyfile(char *src, char *dest)
{
	FILE *in, *out;
	size_t len;
	char buf[4097];

	in = fopen(src, "r");
	if(in == NULL)
		return(1);

	out = fopen(dest, "w");
	if(out == NULL)
		return(1);

	while((len = fread(buf, 1, 4096, in)))
		fwrite(buf, 1, len, out);

	fclose(in);
	fclose(out);
	return(0);
}

char *firstmirror(char *fn)
{
	FILE *fp;
	char line[PATH_MAX];
	char *ptr;
	
	if ((fp = fopen(fn, "r"))== NULL)
	{
		perror(_("Could not open output file for writing"));
		return(NULL);
	}
	while(!feof(fp))
	{
		if(fgets(line, 256, fp) == NULL)
			break;
		if(line == strstr(line, "Server = "))
		{
			fclose(fp);
			// drop [/extra]/frugalware-ARCH
			ptr = strrchr(line, '/');
			*ptr = '\0';
			return(strstr(line, " = ")+3);
		}
	}
	return(NULL);
}

int updateconfig(char *fn, char *mirror, int extra)
{
	FILE *fp, *tfp;
	char line[PATH_MAX];
	char *tfn;
	int firstline=1;
	
	tfn = strdup("/tmp/setup_XXXXXX");
	mkstemp(tfn);
	
	copyfile(fn, tfn);
	if ((tfp = fopen(tfn, "r"))== NULL)
	{
		perror(_("Could not open output file for writing"));
		return(1);
	}
	if ((fp = fopen(fn, "w"))== NULL)
	{
		perror(_("Could not open output file for writing"));
		return(1);
	}
	while(!feof(tfp))
	{
		if(fgets(line, 256, tfp) == NULL)
			break;
		if((line == strstr(line, "Server = ")) && firstline)
		{
			if(!extra)
				fprintf(fp, "Server = %s/frugalware-%s\n", mirror, ARCH);
			else
				fprintf(fp, "Server = %s/extra/frugalware-%s\n", mirror, ARCH);
			fprintf(fp, line);
			firstline=0;
		}
		else
			fprintf(fp, line);
	}
	fclose(tfp);
	fclose(fp);
	unlink(tfn);
	return(0);
}

int mirrorconf(void)
{
	char *fn, *mirror;
	int ret;
	fn = g_strdup_printf("%s/%s", PACCONFPATH, PACCONF);
	
	mirror = firstmirror(fn);
	dialog_vars.backtitle=gen_backtitle(_("Selecting a mirror"));
	dlg_put_backtitle();
	while(1)
	{
		ret = dialog_inputbox(_("Please select a mirror"), _("You may now specify the mirror closest to you in order to download the packages faster. In most cases the default value will be fine."), 0, 0, mirror, 0);
		if (ret != DLG_EXIT_CANCEL)
			break;
		if(exit_confirm())
			exit_perform();
	}

	updateconfig(fn, dialog_vars.input_result, 0);
	fn = g_strdup_printf("%s/%s", PACCONFPATH, PACEXCONF);
	updateconfig(fn, dialog_vars.input_result, 1);
	
	return(0);
}

int run(GList **config)
{
	dlg_put_backtitle();
	if((char*)data_get(*config, "netinstall")!=NULL)
	{
		system(NETCONFIGSCRIPT);
		system(g_strdup_printf("%s >%s 2>%s", INTERFACESSCRIPT,
			LOGDEV, LOGDEV));
		mirrorconf();
	}
	return(0);
}
