#include "common/debug.h"

#include "grammar.h"
#include "grammar.tab.h"

namespace Private {

SymbolMap settings, variables, cursors, locations, rects;
ConstantList constants;
StringQueue todefine;

void define(char *n) {
        Common::String *s = new Common::String(n);
        todefine.push(*s);
}

char *emalloc(unsigned n)	/* check return from malloc */
{
	char *p;

	p = (char*) malloc(n);
	assert(p != NULL);
	return p;
}

Symbol *lookup(Common::String s, SymbolMap symlist)	/* find s in symbol table symlist */
{
	Symbol *sp = NULL;
 
	if (symlist.tryGetVal(s,sp))
	    return sp;
	else
	    return 0;

}

void installall(char *n) {
        Common::String *s;
	assert(todefine.size() > 0);
	while (!todefine.empty()) {
	       s = new Common::String(todefine.pop());

 	       //debug("name %s", s->c_str());
	       if (strcmp(n, "settings") == 0) {
	           install(s, NAME, 0, NULL, &settings);
               }

	       else if (strcmp(n, "variables") == 0) {
	           install(s, NAME, 0, NULL, &variables);
               }

	       else if (strcmp(n, "cursors") == 0) {
	           install(s, NAME, 0, NULL, &cursors);
               }

	       else if (strcmp(n, "locations") == 0) {
	           install(s, NAME, 0, NULL, &variables);
               }

	       else if (strcmp(n, "rects") == 0) {
	           install(s, NAME, 0, NULL, &rects);
               }

	}	   

}

Symbol *addconstant(int t, int d, char *s) 
{
	Symbol *sp;
        Common::String *n = new Common::String("");

	sp = (Symbol *) emalloc(sizeof(Symbol));
	sp->name = n; 
	sp->type = t;
	if (t == NUM || t == NAME)
	   sp->u.val = d;
	else if (t == STRING)
	   sp->u.str = s;
        else
	   assert(0);

	constants.push_front(sp);
	return sp;
}


Symbol *install(Common::String *n, int t, int d, char *s, SymbolMap *symlist)  /* install s in symbol table */
{
        //Common::String *str = new Common::String(n);
 
	Symbol *sp;

	sp = (Symbol *) emalloc(sizeof(Symbol));
        sp->name = n; //str;
	sp->type = t;
	if (t == NUM || t == NAME)
	   sp->u.val = d;
	else if (t == STRING)
	   sp->u.str = s;
        else
	   assert(0);

        symlist->setVal(*n, sp);
	assert(symlist->size() > 0);
	return sp;
}

} 
