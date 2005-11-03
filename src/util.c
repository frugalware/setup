#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "setup.h"
#include "util.h"

#define VERSIONFILE "/etc/frugalware-release"

char *gen_backtitle(char *section)
{
	char *backtitle;
	char *version = get_version();
	MALLOC(backtitle, 256);

	snprintf(backtitle, 255, "%s - %s %s", section, version, _("Setup"));
	FREE(version);
	return(backtitle);
}

char *get_version(void)
{
	FILE *fp;
	char *version;
	MALLOC(version, 128);

	if ((fp = fopen(VERSIONFILE, "r")) == NULL)
	{
		perror(_("Could not open file for reading"));
		return(NULL);
	}
	fgets(version, 127, fp);
	version[strlen(version)-1]='\0';
	fclose(fp);
	return(version);
}

data_t *data_new(void)
{
	data_t *data=NULL;
	
	data = (data_t*)malloc(sizeof(data_t));
	if(data==NULL)
		return(NULL);
	data->name=NULL;
	data->data=NULL;
	return(data);
}
