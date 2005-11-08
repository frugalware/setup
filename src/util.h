#define MALLOC(p, b) { if((b) > 0) \
	{ p = malloc(b); if (!(p)) \
	{ fprintf(stderr, "malloc failure: could not allocate %d bytes\n", b); \
	exit(1); }} else p = NULL; }
#define FREE(p) { if (p) { free(p); (p) = NULL; }}

char *get_version(void);
char *gen_backtitle(char *section);
data_t *data_new(void);
void *data_get(GList *config, char *title);
void data_put(GList **config, char *name, void *data);
int exit_confirm(void);
int exit_perform(void);
char **glist4dialog(GList *list, char *blank);

int fw_menu(const char *title, const char *cprompt, int height, int width,
	int menu_height, int item_no, char **items);
int fw_init_dialog(void);
int fw_end_dialog(void);
