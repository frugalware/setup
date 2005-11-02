#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "setup.h"
#include "util.h"

#define VERSIONFILE "/etc/frugalware-release"

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
