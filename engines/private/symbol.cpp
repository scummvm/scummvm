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
        //debug("looking up %s", s.c_str());	
	return symlist.getVal(s);
}

Symbol *lookupName(char *n)  /* install s in some symbol table */
{
	//debug("looking up %s", n);
        Common::String *s = new Common::String(n);
 
        if (settings.contains(*s))
	    return lookup(*s, settings);

	else if (variables.contains(*s))
	    return lookup(*s, variables);

	else if (cursors.contains(*s))
	    return lookup(*s, cursors);

	else if (locations.contains(*s))
	    return lookup(*s, locations);

	else if (rects.contains(*s))
	    return lookup(*s, rects);

	else {
	    debug("WARNING: %s not defined", n);
	    return addconstant(NAME, 0, n);
	}

}




void installall(char *n) {
        Common::String *s;
	assert(todefine.size() > 0);
	while (!todefine.empty()) {
	       s = new Common::String(todefine.pop());

 	       //debug("name %s", s->c_str());
	       if (strcmp(n, "settings") == 0) {
	           install(s, STRING, 0, (char*) s->c_str(), &settings);
               }

	       else if (strcmp(n, "variables") == 0) {
	           install(s, NAME, 0, NULL, &variables);
               }

	       else if (strcmp(n, "cursors") == 0) {
	           install(s, NAME, 0, NULL, &cursors);
               }

	       else if (strcmp(n, "locations") == 0) {
	           install(s, NAME, 0, NULL, &locations);
               }

	       else if (strcmp(n, "rects") == 0) {
	           install(s, NAME, 0, NULL, &rects);
               }
	       else
		   assert(0);

	}	   

}

Symbol *addconstant(int t, int d, char *s) 
{
	Symbol *sp;
        Common::String *n = new Common::String("<constant>");

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
        Common::String *name = new Common::String(*n);
 
	Symbol *sp;

	sp = (Symbol *) emalloc(sizeof(Symbol));
        sp->name = name;
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
