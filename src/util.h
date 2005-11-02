#define MALLOC(p, b) { if((b) > 0) \
	{ p = malloc(b); if (!(p)) \
	{ fprintf(stderr, "malloc failure: could not allocate %d bytes\n", b); \
	exit(1); }} else p = NULL; }
#define FREE(p) { if (p) { free(p); (p) = NULL; }}

char *get_version(void);
