/*
 * requires: stdio.h, navvdf.h
 */

void entryinit(Vdfentry *, Vdfentry *);
int vdfi_filecreate(Vdfentry *, const char *, const char *);
int vdfi_makedir(Vdfentry *, const char *);
int entryaddto(Vdfentry *, Vdfentry *);
int vdfi_entrygetid(Vdfentry *, const char *);
int vdfi_entryclean(Vdfentry *);
void entrydel(Vdfentry *);
void vdfi_entrydelchild(Vdfentry *);
void entryprint(Vdfentry *, int, FILE *, unsigned int);
int vdfi_entrysetname(Vdfentry *, const char *);
int vdfi_filesetval(Vdfentry *, const char *);

