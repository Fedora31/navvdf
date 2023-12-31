#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "navvdf.h"
#include "entry.h"
#include "field.h"

static char *defval = "";
static char *rootname = "root";
static char *esccomment = "//esc";

static int fnextentry(const char **, Vdfentry *, unsigned int);


/*
 * Create a bare tree.
 */
int
vdf_treeinit(Vdftree *t, char sep, unsigned int options)
{
	t->options = options;
	t->root = malloc(sizeof(Vdfentry));
	entryinit(NULL, t->root);
	t->root->name = rootname;
	t->root->val = defval;
	t->sep[0] = sep;
	t->sep[1] = '\0';
	return 0;
}

int
vdf_load(Vdftree *t, const char *buf, char sep, unsigned int options)
{
	int res;
	Vdfentry *parent, *child;
	const char *p = buf;


	/*init the tree, the first parent and child*/

	vdf_treeinit(t, sep, options);
	parent = t->root;

	child = malloc(sizeof(Vdfentry));
	entryinit(parent, child);


	/*create all the entries*/

	if(t->options & VDF_ESCSEQFILE)
		if(strncmp(buf, esccomment, strlen(esccomment)) == 0)
			t->options |= VDF_ESCSEQ;

	while((res = fnextentry(&p, child, t->options)) >= 0){
		if(res == VDF_EOD){
			parent = parent->parent;
			child->parent = parent;
			continue;
		}

		if(entryaddto(parent, child) < 0)
			return VDF_CANT_ADD_CHILD_TO_PARENT;

		if(res == VDF_DIR)
			parent = child;

		if((child = malloc(sizeof(Vdfentry))) == NULL)
			return VDF_COULDNT_CREATE_CHILD;
		entryinit(parent, child);
	}

	free(child);
	return 0;
}

/*
 * Load a VDF tree from a file and create
 * a Vdftree from it.
 */
int
vdf_loadf(Vdftree *t, FILE *f, char sep, unsigned int options)
{
	unsigned int size;
	int res;
	char *buf;

	/*load the content of the file*/

	fseek(f, 0, SEEK_END);
	size = ftell(f);
	rewind(f);

	buf = malloc(size+1);
	if(fread(buf, 1, size, f) < size)
		return VDF_COULDNT_READ_FILE;

	buf[size] = '\0';

	res = vdf_load(t, buf, sep, options);

	free(buf);
	return res;
}

int
vdf_clean(Vdftree *t)
{
	return vdfi_entryclean(t->root);
}

/*
 * Free all resources used by the given tree.
 */
void
vdf_free(Vdftree *t)
{
	int i;
	for(i = 0; i < t->root->childm; i++)
		entrydel(t->root->childs[i]);
	free(t->root->childs);
	free(t->root);
}

void
vdf_print(Vdftree *t, FILE *f)
{
	int i;

	/*if the file said to use escape sequences, print the comment*/
	if(t->options & VDF_ESCSEQFILE && t->options & VDF_ESCSEQ)
		fprintf(f, "%s\n", esccomment);

	for(i = 0; i < t->root->childm; i++){
		entryprint(t->root->childs[i], 0, f, t->options);
	}
}

/*
 * Find and get the values from the next
 * Vdfentry in the buffer pointed to by p, which is
 * modified to point after the found entry.
 * Returns VDF_EOD if it arrived at the end of a
 * directory. In that case, the given Vdfentry struct
 * isn't modified and p is modified to point after
 * the directory (in the parent dir).
 */
static int
fnextentry(const char **p, Vdfentry *e, unsigned int options)
{
	/*This code doesn't check for comments between names and values.*/

	int i = 0;

	/*get to the next entry, going over any comments*/
	while(1){
		for(;
		(*p)[i] == '\n' ||
		(*p)[i] == '\r' ||
		(*p)[i] == '\t' ||
		(*p)[i] == ' ';
		i++);

		if(
		(*p)[i]   == '/' &&
		(*p)[i+1] == '/'){

			for(;
			(*p)[i] != '\n' &&
			(*p)[i] != '\0';
			i++);

			if((*p)[i] == '\0'){
				/*printf("null outside\n");*/
				return -1;
			}

			continue;
		}
		break;
	}
	(*p)+=i;

	if((*p)[0] == '}'){
		(*p)++; /*jump over*/
		return VDF_EOD;
	}

	if((i = fieldget(p, &e->name, options)) == -1){
		/*printf("null 1\n");*/
		return -1;
	}

	/*printf("(%d) name: %s\n", i, e->name);*/

	for(i = 0;
	(*p)[i] == '\n' ||
	(*p)[i] == '\r' ||
	(*p)[i] == '\t' ||
	(*p)[i] == ' ';
	i++);

	/*printf("2 p[%d]=%c\n", i, (*p)[i]);*/

	if((*p)[i] == '{'){
		(*p)+=i+1;
		e->type = VDF_DIR;
		e->val = defval;
		return VDF_DIR;
	}
	(*p)+=i;

	if((i = fieldget(p, &e->val, options)) == -1){
		/*printf("null 2\n");*/
		return -1;
	}

	e->type = VDF_FILE;

	/*printf("(%d) val: %s\n", i, e->val);*/


	return VDF_FILE;
}
