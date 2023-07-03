#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "navvdf.h"
#include "str.h"
#include "field.h"

static int getposfrompath(const Pos *, const char *, Pos *);

/***************************************************************
 * Thou shall always assume that entries may point to garbage. *
 ***************************************************************/

void
vdf_posinit(Pos *o, Tree *t)
{
	o->tree = t;
	o->curr = t->root;
	o->pathi = 0;
	o->path[0] = '\0';
}

/*
 * Modify the given Pos struct to point to the location given by path.
 * If an error occured, the Pos struct isn't modified (see getposfrompath()).
 */
int
vdf_nav(Pos *o, const char *path)
{
	return getposfrompath(o, path, o);
}

/*
 * Returns the ID of the entry with the given name.
 * Returns -1 if the entry wasn't found.
 */
int
vdf_getid(Pos *o, const char *name)
{
	int i;
	for(i = 0; i < o->curr->childc; i++){
		if(strcmp(o->curr->childs[i]->name, name) == 0)
			return i;
	}
	return -1;
}

/*
 * Checks if the given relative or absolute path exists.
 */
int
vdf_ispathvalid(const Pos *o, const char *path)
{
	Pos dontcare;
	if(getposfrompath(o, path, &dontcare) == 0)
		return 1;
	return 0;
}

/*
 * Tries to naviguate to the given relative or absolute path,
 * then modifies "to" to the found position. If an error occured,
 * to isn't modified.
 */
static int
getposfrompath(const Pos *o, const char *path, Pos *to)
{
	char tmp[VDF_BUFSIZE];
	char *name, *ptr = tmp;
	Pos tmpo = *o;
	int id;

	tmpo.curr = tmpo.tree->root;
	strncpy(tmp, path, VDF_BUFSIZE);

	/*if absolute path*/
	if(tmp[0] == '/'){
		tmpo.pathi = 0;
		tmpo.path[0] = '\0';
	}


	/*construct an absolute path from the path of tmpo*/

	while((name = bstrtok_r(&ptr, "/")) != NULL){

		if(strcmp(".", name) == 0) {continue;}

		/*go back one dir*/
		if(strcmp("..", name) == 0){
			for(; tmpo.pathi > 0 && tmpo.path[tmpo.pathi] != '/'; tmpo.pathi--);
			tmpo.path[tmpo.pathi] = '\0';
			continue;
		}

		tmpo.path[tmpo.pathi++] = '/';
		strncpy(&tmpo.path[tmpo.pathi], name, VDF_BUFSIZE-tmpo.pathi-1);

		tmpo.pathi += strlen(name);
		if(tmpo.pathi >= VDF_BUFSIZE) {tmpo.pathi = VDF_BUFSIZE-1;}
	}

	strncpy(tmp, tmpo.path, VDF_BUFSIZE);
	ptr = tmp;

	/*
	 * Traverse the path, checking if all names and types are correct.
	 * Going to the found position from the root folder allows to
	 * detect wrong paths, (e.g. the current entry or a parent folder
	 * was renamed/deleted). This is important because the original
	 * Position could point to freed memory. Always assume that it can
	 * be the case.
	 */

	while((name = bstrtok_r(&ptr, "/")) != NULL){
		/*we can't navigate into another entry from a file...*/
		if(tmpo.curr->type == VDF_FILE)
			return VDF_CD_FROM_FILE;

		if((id = vdf_getid(&tmpo, name)) < 0)
			return VDF_PATH_NOT_FOUND;
		tmpo.curr = tmpo.curr->childs[id];
	}
	*to = tmpo;
	return 0;
}
