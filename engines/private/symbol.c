#include <stdlib.h>
#include <string.h>

#include "grammar.h"
#include "grammar.tab.h"

static Symbol *symlist = 0;  /* symbol table: linked list */

Symbol *lookup(char *s)	/* find s in symbol table */
{
	Symbol *sp;

	for (sp = symlist; sp != (Symbol *) 0; sp = sp->next)
		if (strcmp(sp->name, s) == 0)
			return sp;
	return 0;	/* 0 ==> not found */	
}

Symbol *install(char *s, int t, int d)  /* install s in symbol table */
{
	Symbol *sp;
	char *emalloc();

	sp = (Symbol *) emalloc(sizeof(Symbol));
	sp->name = emalloc(strlen(s)+1); /* +1 for '\0' */
	strcpy(sp->name, s);
	sp->type = t;
	sp->u.val = d;
	sp->next = symlist; /* put at front of list */
	symlist = sp;
	return sp;
}

char *emalloc(unsigned n)	/* check return from malloc */
{
	char *p;

	p = (char*) malloc(n);
	if (p == 0)
		abort(); //execerror("out of memory", (char *) 0);
	return p;
}
