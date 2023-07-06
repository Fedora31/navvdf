/*
 * requires: stdio.h
 */

/*careful with these, as the PATHSIZE can increase really fast*/
#define VDF_DEPTH 20
#define VDF_BUFSIZE 256
#define VDF_PATHSIZE VDF_BUFSIZE * VDF_DEPTH + 1
#define VDF_ESCSEQ     0x1 /*the file uses escape sequences*/
#define VDF_ESCSEQFILE 0x2 /*if the file starts with "//seq", it uses escape sequences*/


enum VDF_TYPE{
	VDF_DIR,
	VDF_FILE
};

enum VDF_ERR{
	VDF_CANT_ADD_CHILD_TO_PARENT = -2,
	VDF_COULDNT_CREATE_CHILD = -3,
	VDF_STRING_TOO_LONG = -4,
	VDF_CD_FROM_FILE = -5, /*a file was taken for a folder. (dir/file/...)*/
	VDF_PATH_NOT_FOUND = -6,
	VDF_COULDNT_READ_FILE = -7,
	VDF_COULDNT_MALLOC = -8,
	VDF_ID_OUT_OF_BOUNDS = -9,
	VDF_ENTRY_IS_FOLDER = -10,
	VDF_FILE_EXISTS = -11,
	VDF_VAL_TOO_LONG = -12,
	VDF_NAME_TOO_LONG = -13,
	VDF_PATH_TOO_LONG = -14
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
	int childi;
	int childm;
}Entry;

typedef struct Tree{
	Entry *root;
	char sep[2]; /*for the separator and the null byte*/
	unsigned int options;
}Tree;

typedef struct Pos{
	Tree *tree;
	char path[VDF_PATHSIZE];
	int pathi;
	Entry *curr; /*Thou shall not read this struct directly*/
}Pos;

int vdf_treeinit(Tree *, char, unsigned int);
int vdf_load(Tree *, FILE *, char, unsigned int);
int vdf_clean(Tree *);
void vdf_free(Tree *);
void vdf_posinit(Pos *, Tree *);
int vdf_nav(const Pos *, const char *, Pos *);
int vdf_navnext(const Pos *, int *, Pos *);
int vdf_getid(const Pos *, const char *);
int vdf_name(const Pos *, const char *, char *);
int vdf_val(const Pos *, const char *, char *);
int vdf_rename(const Pos *, const char *, const char *);
int vdf_mkdir(const Pos *, const char *);
int vdf_touch(const Pos *, const char *, const char *);
int vdf_rm(const Pos *, const char *);
void vdf_print(Tree *, FILE *);
int vdf_ispathvalid(const Pos *, const char *);
