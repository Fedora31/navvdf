#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "navvdf.h"
#include "str.h"
#include "field.h"
#include "entry.h"

static int getposfrompath(const Vdfpos *, const char *, Vdfpos *);
static int getabsolutepath(const Vdfpos *, const char *, char *);

/***************** H O L Y     S C R I P T U R E S *******************
 * Thou shall always assume first that entries may point to garbage. *
 * Thou shall never read entries before the path is checked.         *
 *********************************************************************/

void
vdf_posinit(Vdfpos *o, Vdftree *t)
{
	o->tree = t;
	o->curr = t->root;
	o->pathi = 0;
	o->path[0] = '\0';
}

/*
 * Modify "to" to point to the location given by path.
 * See getposfrompath().
 */
int
vdf_nav(const Vdfpos *o, const char *path, Vdfpos *to)
{
	return getposfrompath(o, path, to);
}

int
vdf_navnext(const Vdfpos *o, int *id, Vdfpos *to)
{
	Vdfpos tmpo = *o;

	if(*id < 0)
		return VDF_ID_OUT_OF_BOUNDS;

	if(!vdf_ispathvalid(&tmpo, ""))
		return VDF_PATH_NOT_FOUND;

	/*path is OK*/

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
 * Receiving a negative value means an error occured.
 * Since this function checks if the path is valid,
 * one might want to use vdfi_entrygetid() for internal
 * functions where the path has already been checked.
 */
int
vdf_getid(const Vdfpos *o, const char *name)
{
	if(!vdf_ispathvalid(o, "."))
		return VDF_PATH_NOT_FOUND;

	/*path is OK*/

	return vdfi_entrygetid(o->curr, name);
}

/*
 * Writes in the given buffer the name of the entry
 * pointed to by path. On error, the buffer is not
 * mofified.
 */
int
vdf_name(const Vdfpos *o, const char *path, char *buf)
{
	int res;
	Vdfpos tmpo;
	if((res = getposfrompath(o, path, &tmpo)) < 0)
		return res;

	/*path is OK*/

	strncpy(buf, tmpo.curr->name, VDF_BUFSIZE-1);
	buf[VDF_BUFSIZE-1] = '\0';
	return 0;
}

/*
 * Returns a pointer to the name pointed to by path.
 * /!\ Warning: this function is unsafe if you're
 * modifying and deleting stuff from the Tree!
 */
const char *
vdf_nameptr(const Vdfpos *o, const char *path)
{
	Vdfpos tmpo;
	if(getposfrompath(o, path, &tmpo) < 0)
		return NULL;
	return tmpo.curr->name;
}

/*
 * Rename the entry pointed to by path.
 */
int
vdf_rename(const Vdfpos *o, const char *path, const char *newname)
{
	int res;
	Vdfpos tmpo;
	if((res = getposfrompath(o, path, &tmpo)) < 0)
		return res;

	/*path is OK*/

	return vdfi_entrysetname(tmpo.curr, newname);
}

int
vdf_newval(const Vdfpos *o, const char *path, const char *newval)
{
	int res;
	Vdfpos tmpo;
	if((res = getposfrompath(o, path, &tmpo)) < 0)
		return res;

	/*path is OK*/

	return vdfi_filesetval(tmpo.curr, newval);
}

/*
 * Writes in the given buffer the value of the file
 * pointed to by path. On error, the buffer is not
 * modified. Wanting to get the value of a folder is
 * considered an error.
 */
int
vdf_val(const Vdfpos *o, const char *path, char *buf)
{
	int res;
	Vdfpos tmpo;
	if((res = getposfrompath(o, path, &tmpo)) < 0)
		return res;

	/*path is OK*/

	if(tmpo.curr->type == VDF_DIR)
		return VDF_ENTRY_IS_FOLDER;

	strncpy(buf, tmpo.curr->val, VDF_BUFSIZE-1);
	buf[VDF_BUFSIZE-1] = '\0';
	return 0;
}

/*
 * Returns a pointer to the value pointed to by path.
 * /!\ Warning: this function is unsafe if you're
 * modifying and deleting stuff from the Tree!
 */
const char *
vdf_valptr(const Vdfpos *o, const char *path)
{
	Vdfpos tmpo;
	if(getposfrompath(o, path, &tmpo) < 0)
		return NULL;
	return tmpo.curr->val;
}

/*
 * Checks if the given relative or absolute path exists.
 */
int
vdf_ispathvalid(const Vdfpos *o, const char *path)
{
	Vdfpos dontcare;
	if(getposfrompath(o, path, &dontcare) == 0)
		return 1;
	return 0;
}

/*
 * Create a filepath. Entries that already exist are skipped.
 * TODO: remove all the created directories on error
 */
int
vdf_mkdir(const Vdfpos *o, const char *path)
{
	Vdfpos tmpo = *o;
	char tmp[VDF_PATHSIZE];
	char *name, *ptr = tmp;
	Vdfentry *parent = tmpo.tree->root, *new_tree = NULL;
	int res, id;

	/*Theorically, it's not necessary to check if the path
	 *is valid here as we'll be working with absolute paths,
	 *but you may not want to see directories you deleted
	 *earlier suddenly reappear because you were in a part
	 *of the tree that was deleted/renamed.
	 */
	if(vdf_ispathvalid(o, path) < 0)
		return VDF_PATH_NOT_FOUND;

	if((res = getabsolutepath(&tmpo, path, tmp)) < 0)
		return res;

	while((name = bstrtok_r(&ptr, o->tree->sep)) != NULL){
		if((id = vdfi_entrygetid(parent, name)) < 0){
			if((id = vdfi_makedir(parent, name)) < 0){
				if(new_tree)
					{ /* TODO: delete the entries starting at new_tree, and remove new_tree from it's parent */ }
				return id;
			}
			if(!new_tree)
				new_tree = parent->childs[id];
		}
		parent = parent->childs[id];
	}

	return 0;
}

int
vdf_touch(const Vdfpos *o, const char *path, const char *val)
{
	Vdfpos tmpo = *o;
	char dirname[VDF_PATHSIZE];
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

	/*path is OK*/

	if(vdfi_entrygetid(tmpo.curr, basename) >= 0)
		return VDF_FILE_EXISTS;
	return vdfi_filecreate(tmpo.curr, basename, val);
}

int
vdf_rm(const Vdfpos *o, const char *path)
{
	Vdfpos tmpo = *o;
	int res;

	if((res = getposfrompath(o, path, &tmpo)) < 0)
		return res;

	/*path is OK*/

	vdfi_entrydelchild(tmpo.curr);

	return 0;
}

/*
 * Construct an absolute path from the current path in o and the
 * given path, formatting it and removing stuff like //, /./ or /../.
 * Returns the length of the new path, or a negative value on error.
 * /!\ Warning: o->path and to must not overlap!
 * /!\ Warning: this does NOT check if the path is valid, use
 *              ispathvalid() or getposfrompath() for that.
 */
static int
getabsolutepath(const Vdfpos *o, const char *relpath, char *to)
{
	char tmp[VDF_PATHSIZE];
	char *name, *ptr = tmp;
	int i = 0;

	strncpy(tmp, relpath, VDF_PATHSIZE);
	/*possible bc strncpy sets all unusued bytes to 0*/
	if(tmp[VDF_PATHSIZE-1] != '\0')
		return VDF_PATH_TOO_LONG;

	/*copy the Vdfpos path over if relpath is not absolute*/
	if(relpath[0] != o->tree->sep[0]){
		strncpy(to, o->path, o->pathi);
		i = o->pathi;
		to[i] = '\0';
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
		strncpy(&to[i], name, VDF_PATHSIZE-i-1);

		i += strlen(name);
		if(i >= VDF_PATHSIZE) /*don't think it'll ever trigger*/
			return VDF_PATH_TOO_LONG;
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
getposfrompath(const Vdfpos *o, const char *path, Vdfpos *to)
{
	char tmp[VDF_PATHSIZE];
	char *name, *ptr = tmp;
	Vdfpos tmpo = *o;
	int id;

	tmpo.curr = tmpo.tree->root;

	if((tmpo.pathi = getabsolutepath(&tmpo, path, tmp)) < 0)
		return tmpo.pathi;
	strncpy(tmpo.path, tmp, VDF_PATHSIZE);

	/*
	 * Traverse the path, checking if all names and types are correct.
	 * Going to the found position from the root folder allows to
	 * detect wrong paths, (e.g. the current entry or a parent folder
	 * was renamed/deleted). This is important because the original
	 * Vdfposition could point to freed memory. Always assume that it can
	 * be the case (see the HOLY SCRIPTURES).
	 */

	while((name = bstrtok_r(&ptr, o->tree->sep)) != NULL){
		/*we can't navigate into another entry from a file...*/
		if(tmpo.curr->type == VDF_FILE)
			return VDF_CD_FROM_FILE;

		if((id = vdfi_entrygetid(tmpo.curr, name)) < 0)
			return VDF_PATH_NOT_FOUND;
		tmpo.curr = tmpo.curr->childs[id];
	}
	*to = tmpo;
	return 0;
}
