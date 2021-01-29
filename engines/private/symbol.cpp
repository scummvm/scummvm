#include "common/debug.h"
#include "grammar.h"
#include "grammar.tab.h"

namespace Private {

SymbolMap settings, variables, cursors, locations, rects;
ConstantList constants;
VariableList variableList;

StringQueue stringToDefine;
RectQueue rectToDefine;

void defineSymbol(char *n, Common::Rect *r) {
    Common::String *s = new Common::String(n);
    stringToDefine.push(*s);
    rectToDefine.push(r);
}

char *emalloc(unsigned n) {
    char *p;

    p = (char*) malloc(n);
    assert(p != NULL);
    return p;
}

void showSymbol(Symbol *s) {
    if (s->type == NUM)
        debug("%s %d",s->name->c_str(), s->u.val);
    else if (s->type == STRING)
        debug("%s %s", s->name->c_str(), s->u.str);
    else if (s->type == NAME)
        debug("%s %d",s->name->c_str(), s->type);
    else
        debug("%s %d", s->name->c_str(), s->type);
}

void setSymbol(Symbol *s, int v) {
    s->u.val = v;
}

/* find s in symbol table symlist */
Symbol *lookup(Common::String s, SymbolMap symlist)	 {
    //debug("looking up %s", s.c_str());
    Symbol *r = symlist.getVal(s);
    /*if (strcmp(s.c_str(), "m_640x480") == 0) {
        showSymbol(r);
    assert(0);
    }*/

    return r;
}

/* lookup some name in some symbol table */
Symbol *lookupName(char *n) {
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
        debug("WARNING: %s not defined", s->c_str());
        return constant(STRING, 0, (char*) s->c_str());
    }

}


void installAll(char *n) {
    Common::String *s;
    Common::Rect *r;

    assert(stringToDefine.size() > 0);

    while (!stringToDefine.empty()) {
        s = new Common::String(stringToDefine.pop());
        r = rectToDefine.pop();

        //debug("name %s", s->c_str());
        if (strcmp(n, "settings") == 0) {
            assert(r == NULL);
            install(s, STRING, 0, (char*) s->c_str(), r, &settings);
        }

        else if (strcmp(n, "variables") == 0) {
            assert(r == NULL);
            install(s, NAME, 0, NULL, r, &variables);
            variableList.push_front(*s);
        }

        else if (strcmp(n, "cursors") == 0) {
            assert(r == NULL);
            install(s, NAME, 0, NULL, r, &cursors);
        }

        else if (strcmp(n, "locations") == 0) {
            assert(r == NULL);
            install(s, NAME, 0, NULL, r, &locations);
        }

        else if (strcmp(n, "rects") == 0) {
            assert(r != NULL);
            install(s, RECT, 0, NULL, r, &rects);
        }
        else
            assert(0);

    }

}

Symbol *constant(int t, int d, char *s) {
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

/* install some symbol s in a symbol table */
Symbol *install(Common::String *n, int t, int d, char *s, Common::Rect *r, SymbolMap *symlist) {
    Common::String *name = new Common::String(*n);

    Symbol *sp;

    sp = (Symbol *) emalloc(sizeof(Symbol));
    sp->name = name;
    sp->type = t;
    if (t == NUM || t == NAME)
        sp->u.val = d;
    else if (t == STRING)
        sp->u.str = s;
    else if (t == RECT)
        sp->u.rect = r;
    else
        assert(0);

    symlist->setVal(*n, sp);
    assert(symlist->size() > 0);
    return sp;
}

}
