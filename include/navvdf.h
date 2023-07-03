/*
 * requires: stdio.h
 */

#define VDF_DEPTH 128
#define VDF_BUFSIZE 512
#define VDF_ESCSEQ     0x1 /*the file uses escape sequences*/
#define VDF_ESCSEQFILE 0x2 /*if the file starts with "//seq", it uses escape sequences*/


enum VDF_TYPE{
	VDF_DIR,
	VDF_FILE
};

enum VDF_ERR{
	VDF_CANT_ADD_CHILD_TO_PARENT = -2,
	VDF_COULDNT_CREATE_CHILD = -3,
	VDF_CD_FROM_FILE = -5, /*a file was taken for a folder. (dir/file/...)*/
	VDF_PATH_NOT_FOUND = -6,
	VDF_COULDNT_READ_FILE = -7
};

enum VDF_RES{
	VDF_NOMATCH = -2,
	VDF_ISFILE = -3,
	VDF_EOD = 3
};

typedef struct Entry{
	char *name;
	char *val;
	enum VDF_TYPE type;
	struct Entry *parent;
	struct Entry **childs;
	int childc;
	int childm;
}Entry;

typedef struct Tree{
	Entry *root;
	unsigned int options;
}Tree;

typedef struct Pos{
	Tree *tree;
	char path[VDF_BUFSIZE];
	int pathi;
	Entry *curr;
}Pos;

int vdf_treeinit(Tree *, unsigned int);
int vdf_load(Tree *, FILE *, unsigned int);
void vdf_free(Tree *);
void vdf_posinit(Pos *, Tree *);
int vdf_nav(Pos *, const char *);
int vdf_navi(Pos *, int);
int vdf_getid(Pos *, const char *);
int vdf_mkdir(Pos *, const char *);
int vdf_touch(Pos *, const char *, const char *);
void vdf_print(Tree *, FILE *);
int vdf_ispathvalid(const Pos *, const char *);
