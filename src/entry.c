#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "navvdf.h"
#include "entry.h"
#include "field.h"
#include "str.h"

/*
 * Inits the Entry struct with proper values.
 */
void
entryinit(Entry *parent, Entry *e)
{
	e->parent = parent;
	e->childi = 0;
	e->childm = 0;
	e->name = NULL;
	e->val = NULL; /*do not free if its a dir*/
	e->childs = NULL;
	e->type = VDF_DIR;
}

int
vdfi_entrygetid(Entry *e, const char *name)
{
	int i;
	for(i = 0; i < e->childm; i++){
		if(!e->childs[i])
			continue;
		if(strcmp(e->childs[i]->name, name) == 0)
			return i;
	}
	return -1;
}

int
vdfi_makedir(Entry *parent, const char *name)
{
	Entry *e;
	int id, res, size = strlen(name)+1;

	if(size > VDF_BUFSIZE) {size = VDF_BUFSIZE;}

	if((e = malloc(sizeof(Entry))) == NULL)
		return VDF_COULDNT_MALLOC;

	entryinit(parent, e);
	if((res = vdfi_entrysetname(e, name)) < 0){
		entrydel(e);
		return res;
	}
	strncpy(e->name, name, size);

	if((id = entryaddto(parent, e)) < 0){
		entrydel(e);
		return VDF_CANT_ADD_CHILD_TO_PARENT;
	}

	return id;
}

int
vdfi_entrysetname(Entry *e, const char *name)
{
	void *mem;
	int size = strlen(name)+1;

	if(size > VDF_BUFSIZE)
		return VDF_NAME_TOO_LONG;
	if((mem = realloc(e->name, size)) == NULL)
		return VDF_COULDNT_MALLOC;
	e->name = mem;
	strncpy(e->name, name, size);

	return 0;
}

int
vdfi_filesetval(Entry *e, const char *val)
{
	void *mem;
	int size = strlen(val)+1;

	if(e->type != VDF_FILE)
		return VDF_ENTRY_IS_FOLDER;
	if(size > VDF_BUFSIZE)
		return VDF_VAL_TOO_LONG;
	if((mem = realloc(e->val, size)) == NULL)
		return VDF_COULDNT_MALLOC;
	e->val = mem;
	strncpy(e->val, val, size);

	return 0;
}

/*
 * TODO: check for duplicates
 */
int
vdfi_filecreate(Entry *parent, const char *name, const char *val)
{
	int res;
	Entry *e;

	if(parent->type == VDF_FILE)
		return VDF_CD_FROM_FILE;

	if((e = malloc(sizeof(Entry))) == NULL)
		return VDF_COULDNT_MALLOC;

	entryinit(parent, e);
	e->type = VDF_FILE;

	if((res = vdfi_entrysetname(e, name)) < 0){
		entrydel(e);
		return res;
	}

	if((res = vdfi_filesetval(e, val)) < 0){
		entrydel(e);
		return res;
	}

	if((res = entryaddto(parent, e)) < 0){
		entrydel(e);
		return res;
	}
	return 0;
}

/*
 * Makes the given entry a child of the parent entry.
 * Returns the index of the directory on success.
 */
int
entryaddto(Entry *parent, Entry *child)
{
	/*
	 * This function doesn't reuse freed slots in the parent, this
	 * makes it so that entries are always added at the end. Like this,
	 * entries added later don't randomly get added in the middle of
	 * folders, and it's possible to add entries in the midst of a
	 * folder parse with vdf_navnext(). The downside is that the slot
	 * can only grow larger, so it may be good to call vdf_clean()
	 * after a lot of deletions. (Also, shifting the slots by one after
	 * a deletion would make it possible to miss entries with
	 * vdf_navnext().)
	 */

	void *mem;

	if(parent->type == VDF_FILE)
		return VDF_CD_FROM_FILE;

	if(parent->childi >= parent->childm){
		parent->childm += 1;
		if((mem = realloc(parent->childs, parent->childm * sizeof(Entry**))) == NULL)
			return VDF_COULDNT_MALLOC;
		parent->childs = mem;
	}
	parent->childs[parent->childi++] = child;
	child->parent = parent;

	return parent->childi-1;
}

/*
 * Regroup the child pointers and resize the slot array.
 * e.g.:  xx.x..xxx.x -> xxxxxxx.... -> xxxxxxx
 */
int
vdfi_entryclean(Entry *e)
{
	int i, a;
	void *mem;

	if(e->type == VDF_FILE)
		return 0;


	for(i = 0, a = 0; a < e->childm; i++, a++){

		if(!e->childs[i]){
			for(; !e->childs[a] && a < e->childm; a++);
			if(a == e->childm)
				break;
		}

		if(i != a){
			e->childs[i] = e->childs[a];
			e->childs[a] = NULL;
		}

		if(vdfi_entryclean(e->childs[i]) < 0)
			return VDF_COULDNT_MALLOC;
	}

	for(i = 0; i < e->childm; i++)
		if(!e->childs[i])
			break;

	if((mem = realloc(e->childs, i * sizeof(Entry**))) == NULL)
		return VDF_COULDNT_MALLOC;

	e->childs = mem;
	e->childi = i;
	e->childm = i;

	return 0;
}

/*
 * Calls entrydel() and also removes the child's handle in the parent.
 */
void
vdfi_entrydelchild(Entry *e)
{
	int i;
	Entry *parent = e->parent;

	if(parent == NULL) /*if you tried to rm the root folder...*/
		return;

	for(i = 0; i < parent->childm; i++)
		if(parent->childs[i] == e){
			parent->childs[i] = NULL;
			entrydel(e);
			break;
		}
}

/*
 * Deletes the given entry and all the other
 * possible sub-entries recursively.
 */
void
entrydel(Entry *e)
{
	int i;
	if(!e)
		return;
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

	if(!e)
		return;

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
