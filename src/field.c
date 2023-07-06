#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "navvdf.h"
#include "field.h"


static char escseq(char);


/*
 * Write the content of the field pointed to by p to to,
 * allocating memory. p is then moved past the found field.
 * Returns -1 if a NUL byte is found (in VDF files, this
 * should not occur).
 */
int
fieldget(char **p, char **to, unsigned int options)
{
	int i, y = 0, stop = 0;
	char c;
	int quoted = 0;
	char tmp[VDF_BUFSIZE];

	if((*p)[0] == '"'){
		quoted = 1;
		(*p)++;
	}

	for(i = 0; (c = (*p)[i]) != '\0'; i++){

		if(!quoted && (c == ' ' || c == '\t'))
			break;

		switch(c){
		case '\\':
			if(options & VDF_ESCSEQ)
				c = escseq((*p)[++i]);
			break;

		case '"':
		case '\n':
		case '\r':
			stop = 1;
			break;
		}
		if(stop)
			break;

		/*if the field is too large, it will be trunkated*/
		if(y < VDF_BUFSIZE-1)
			tmp[y++] = c;
	}
	if((*p)[i] == '\0')
		return -1;

	tmp[y] = '\0';
	*to = malloc(y+1);
	strcpy(*to, tmp);
	(*p) += i+1;

	return 0;
}

void
fieldprint(char *s, FILE *f, unsigned int options)
{
	int i, quoted = 0;

	if(strlen(s) == 0){
		fputs("\"\"", f);
		return;
	}

	for(i = 0; s[i] != '\0'; i++)
		if(s[i] == ' '  ||
		   s[i] == '\t' ||
		   s[i] == '{'  ||
		   s[i] == '}')
			quoted = 1;

	if(quoted)
		fputc('"', f);

	if(options & VDF_ESCSEQ){
		for(i = 0; s[i] != '\0'; i++){
			switch(s[i]){
			case '"':
				fputs("\\\"", f);
				break;
			case '\n':
				fputs("\\n", f);
				break;
			case '\t':
				fputs("\\t", f);
				break;
			default:
				fputc(s[i], f);
			}
		}
	}else
		fputs(s, f);


	if(quoted)
		fputc('"', f);
}

static char
escseq(char c)
{
	switch(c){
	case 'n':
		return '\n';
	case 't':
		return '\t';
	}
	return c;
}
