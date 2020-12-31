#include <stdlib.h>
#include <string.h>

#include "grammar.h"
#include "grammar.tab.h"

namespace Private {

Symbol *symlist;  /* symbol table: linked list */

char *emalloc(unsigned n)	/* check return from malloc */
{
	char *p;

	p = (char*) malloc(n);
	assert(p != NULL);
	//	abort(); //execerror("out of memory", (char *) 0);
	return p;
}

Symbol *lookup(char *s)	/* find s in symbol table */
{
	Symbol *sp;

	for (sp = symlist; sp != (Symbol *) 0; sp = sp->next)
		if (strcmp(sp->name, s) == 0)
			return sp;
	return 0;	/* 0 ==> not found */	
}

Symbol *install(char *n, int t, int d, char *s)  /* install s in symbol table */
{
	Symbol *sp;

	sp = (Symbol *) emalloc(sizeof(Symbol));
	sp->name = emalloc(strlen(n)+1); /* +1 for '\0' */
	strcpy(sp->name, n);
	sp->type = t;
	if (t == NUM || t == NAME)
	   sp->u.val = d;
	else if (t == STRING)
	   sp->u.str = s;
        else
	   assert(0); //abort();

	sp->next = symlist; /* put at front of list */
	symlist = sp;
	return sp;
}

} 
