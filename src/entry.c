#include <stdio.h>
#include <stdlib.h>

#include "navvdf.h"
#include "field.h"


/*
 * Inits the Entry struct with proper values.
 */
void
entryinit(Entry *parent, Entry *e)
{
	e->parent = parent;
	e->childc = 0;
	e->childm = 0;
	e->name = NULL;
	e->val = NULL;
	e->childs = NULL;
	e->type = VDF_DIR;
}

/*
 * Makes the given entry a child of the parent entry.
 */
int
entryaddto(Entry *parent, Entry *child)
{
	if(parent->childc >= parent->childm){
		parent->childm += 1;
		if((parent->childs = realloc(parent->childs, parent->childm * sizeof(Entry**))) == NULL)
			return -1;
	}
	parent->childs[parent->childc++] = child;
	return 0;
}

/*
 * Deletes the given entry and all the other
 * possible sub-entries recursively.
 */
void
entrydel(Entry *e)
{
	int i;
	if(e->type == VDF_DIR){
		for(i = 0; i < e->childm; i++)
			entrydel(e->childs[i]);
		free(e->childs);
	}else
		free(e->val); /*only for files*/
	free(e->name);
	free(e);
}

#define PRINTLVL(x, f) {int i;\
for(i = 0; i < x; i++)\
	fputc('\t', f);}

void
entryprint(Entry *e, int level, FILE *f, unsigned int options)
{
	int i;

	PRINTLVL(level, f);
	fieldprint(e->name, f, options);

	if(e->type == VDF_DIR){
		fputs("\n", f);
		PRINTLVL(level, f);
		fputs("{\n", f);

		level++;
		for(i = 0; i < e->childm; i++)
			entryprint(e->childs[i], level, f, options);
		level--;

		PRINTLVL(level, f);
		fputs("}\n", f);
	}else{
		fputs("\t", f);
		fieldprint(e->val, f, options);
		fputs("\n", f);
	}
}
