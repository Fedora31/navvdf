/*
 * requires: stdio.h, navvdf.h
 */

void entryinit(Entry *, Entry *);
int entryaddto(Entry *, Entry *);
void entrydel(Entry *);
void entryprint(Entry *, int, FILE *, unsigned int);

