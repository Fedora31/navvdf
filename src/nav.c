#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "navvdf.h"
#include "str.h"
#include "field.h"
#include "entry.h"

static int getposfrompath(const Pos *, const char *, Pos *);
static int getabsolutepath(const Pos *, const char *, char *);

/*********************************************************************
 * Thou shall always assume first that entries may point to garbage. *
 *********************************************************************/

void
vdf_posinit(Pos *o, Tree *t)
{
	o->tree = t;
	o->curr = t->root;
	o->pathi = 0;
	o->path[0] = '\0';
}

/*
 * Modify "to" to point to the location given by path.
 * If an error occured, to isn't modified (see getposfrompath()).
 */
int
vdf_nav(const Pos *o, const char *path, Pos *to)
{
	return getposfrompath(o, path, to);
}

int
vdf_navnext(const Pos *o, int *id, Pos *to)
{
	Pos tmpo = *o;

	if(*id < 0)
		return VDF_ID_OUT_OF_BOUNDS;

	if(!vdf_ispathvalid(&tmpo, ""))
		return VDF_PATH_NOT_FOUND;

	/*path ok, can read curr*/

	/*get the next entry that is valid*/
	for(; *id < tmpo.curr->childm; (*id)++){
		if(tmpo.curr->childs[*id]){
			tmpo.curr = tmpo.curr->childs[*id];
			*to = tmpo;
			return 0;
		}
	}
	return VDF_ID_OUT_OF_BOUNDS;
}

/*
 * Returns the ID of the entry with the given name.
 * Returns -1 if the entry wasn't found.
 */
int
vdf_getid(const Pos *o, const char *name)
{
	return vdfi_entrygetid(o->curr, name);
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

int
vdf_mkdir(const Pos *o, const char *path)
{
	Pos tmpo = *o;
	char tmp[VDF_BUFSIZE];
	char *name, *ptr = tmp;
	Entry *parent = tmpo.tree->root, *new_tree = NULL;
	int res, id;

	if((res = getabsolutepath(&tmpo, path, tmp)) < 0)
		return res;

	while((name = bstrtok_r(&ptr, o->tree->sep)) != NULL){
		if((id = vdf_getid(&tmpo, name)) < 0){
			if((id = vdfi_makedir(parent, name)) < 0){
				if(new_tree)
					{ /* delete the entries starting at new_tree, and remove new_tree from it's parent */ }
				return id;
			}
			if(!new_tree)
				new_tree = parent->childs[id];
		}
		parent = parent->childs[id];
	}

	return 0;

	/*this sends the absolute path!*/
	/*return vdfi_pathcreate(tmpo.tree->root, tmp);*/
}

int
vdf_touch(const Pos *o, const char *path, const char *val)
{
	Pos tmpo = *o;
	char dirname[VDF_BUFSIZE];
	const char *basename;
	char *sep = strrchr(path, o->tree->sep[0]);
	int res;

	if(!sep){
		basename = path;
		dirname[0] = '\0';
	}else{
		basename = ++sep;
		strncpy(dirname, path, sep-path);
		dirname[sep-path-1] = '\0';
	}

	if((res = getposfrompath(&tmpo, dirname, &tmpo)) < 0)
		return res;

	/*Successfully verified that the path is valid!*/

	return vdfi_filecreate(tmpo.curr, basename, val);
}

int
vdf_rm(const Pos *o, const char *path)
{
	Pos tmpo = *o;
	int res;

	if((res = getposfrompath(o, path, &tmpo)) < 0)
		return res;

	vdfi_entrydelchild(tmpo.curr);

	return 0;
}

/*
 * Construct an absolute path from the current path in o and the
 * given path.
 * Returns the length of the new path, or a negative value on error.
 * /!\ Warning: o->path and to must not overlap! /!\
 */
static int
getabsolutepath(const Pos *o, const char *relpath, char *to)
{
	char tmp[VDF_BUFSIZE];
	char *name, *ptr = tmp;
	int i = 0;

	strncpy(tmp, relpath, VDF_BUFSIZE);

	/*copy the Pos path over if relpath is not absolute*/
	if(relpath[0] != o->tree->sep[0]){
		strncpy(to, o->path, o->pathi);
		i = o->pathi;
	}

	while((name = bstrtok_r(&ptr, o->tree->sep)) != NULL){

		if(strcmp(".", name) == 0)
			continue;

		/*go back one dir*/
		if(strcmp("..", name) == 0){
			for(; i > 0 && to[i] != o->tree->sep[0]; i--);
			to[i] = '\0';
			continue;
		}

		to[i++] = o->tree->sep[0];
		strncpy(&to[i], name, VDF_BUFSIZE-i-1);

		i += strlen(name);
		if(i >= VDF_BUFSIZE)
			return VDF_STRING_TOO_LONG;
	}
	to[i] = '\0';
	return strlen(to);
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

	if((tmpo.pathi = getabsolutepath(&tmpo, path, tmp)) < 0)
		return tmpo.pathi;
	strncpy(tmpo.path, tmp, VDF_BUFSIZE);

	/*
	 * Traverse the path, checking if all names and types are correct.
	 * Going to the found position from the root folder allows to
	 * detect wrong paths, (e.g. the current entry or a parent folder
	 * was renamed/deleted). This is important because the original
	 * Position could point to freed memory. Always assume that it can
	 * be the case.
	 */

	while((name = bstrtok_r(&ptr, o->tree->sep)) != NULL){
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
