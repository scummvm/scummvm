/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

typedef struct Symbol {	 /* symbol table entry */
	Common::String *name;
	short  type;			/* NAME, NUM, STRING or RECT  */
	union {
		int val;			/* NAME or NUM */
		const char *str;	/* STRING */
		Common::Rect *rect; /* RECT */
	} u;
} Symbol;

// Symbols

void setSymbol(Symbol *, int);

typedef Common::HashMap<Common::String, Symbol *> SymbolMap;
typedef Common::List<Common::String> NameList;
typedef Common::List<Symbol *> ConstantList;

typedef Common::Queue<Common::String> StringQueue;
typedef Common::Queue<Common::Rect *> RectQueue;

class SymbolMaps {
private:
	StringQueue stringToDefine;
	RectQueue rectToDefine;

public:
	SymbolMap settings;
	SymbolMap variables;
	SymbolMap cursors;
	SymbolMap locations;
	SymbolMap rects;
	ConstantList constants;

	NameList variableList;
	NameList locationList;

	Symbol *constant(int t, int d, const char *s);
	Symbol *lookupVariable(Common::String *n);
	Symbol *lookupLocation(Common::String *n);
	Symbol *lookupRect(Common::String *n);
	Symbol *lookupName(const char *n);
	void installAll(const char *n);
	void defineSymbol(const char *, Common::Rect *);
};

} // End of namespace Private

#endif
