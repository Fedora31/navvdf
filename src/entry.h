/*
 * requires: stdio.h, navvdf.h
 */

void entryinit(Entry *, Entry *);
int vdfi_filecreate(Entry *, const char *, const char *);
int vdfi_makedir(Entry *, const char *);
int entryaddto(Entry *, Entry *);
int vdfi_entrygetid(Entry *, const char *);
int vdfi_entryclean(Entry *);
void entrydel(Entry *);
void vdfi_entrydelchild(Entry *);
void entryprint(Entry *, int, FILE *, unsigned int);

