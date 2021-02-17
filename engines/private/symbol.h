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

#ifndef PRIVATE_SYMBOL_H
#define PRIVATE_SYMBOL_H

#include "common/str.h"
#include "common/hash-str.h"
#include "common/hash-ptr.h"
#include "common/queue.h"
#include "common/list.h"
#include "common/array.h"
#include "common/rect.h"

namespace Private {

typedef struct Symbol {     /* symbol table entry */
    Common::String *name;
    short  type;            /* NAME, NUM, STRING or RECT  */
    union {
        int val;            /* NAME or NUM */
        char *str;          /* STRING */
        Common::Rect *rect; /* RECT */
    } u;
} Symbol;

// Symbols

//extern void showSymbol(Symbol *);
extern void setSymbol(Symbol *, int);

typedef Common::HashMap<Common::String, Symbol *> SymbolMap;
typedef Common::List<Common::String> NameList;
typedef Common::List<Symbol *> ConstantList;

class SymbolMaps {
    public:
    SymbolMap settings; 
    SymbolMap variables;
    SymbolMap cursors;
    SymbolMap locations;
    SymbolMap rects;

    Symbol *constant(int t, int d, char *s);
    Symbol *lookupName(char *n);
    void installAll(char *n);
};

//extern SymbolMap settings, variables, cursors, locations, rects;
extern ConstantList constants;
extern NameList variableList;
extern NameList locationList;

extern void     defineSymbol(char *, Common::Rect *);
extern Symbol  *install(Common::String *, int, int, char *, Common::Rect *, SymbolMap *);

} // End of namespace Private

#endif
