/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/debug.h"
#include "private/grammar.h"
#include "private/grammar.tab.h"

namespace Private {

SymbolMap settings, variables, cursors, locations, rects;
ConstantList constants;
NameList variableList;
NameList locationList;

StringQueue stringToDefine;
RectQueue rectToDefine;

void defineSymbol(char *n, Common::Rect *r) {
    Common::String *s = new Common::String(n);
    stringToDefine.push(*s);
    rectToDefine.push(r);
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
    Symbol *r = symlist.getVal(s);
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
            locationList.push_front(*s);
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

    sp = (Symbol *)malloc(sizeof(Symbol));
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

    sp = (Symbol *)malloc(sizeof(Symbol));
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
