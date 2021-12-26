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

// Heavily inspired by hoc
// Copyright (C) AT&T 1995
// All Rights Reserved
//
// Permission to use, copy, modify, and distribute this software and
// its documentation for any purpose and without fee is hereby
// granted, provided that the above copyright notice appear in all
// copies and that both that the copyright notice and this
// permission notice and warranty disclaimer appear in supporting
// documentation, and that the name of AT&T or any of its entities
// not be used in advertising or publicity pertaining to
// distribution of the software without specific, written prior
// permission.
//
// AT&T DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
// INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS.
// IN NO EVENT SHALL AT&T OR ANY OF ITS ENTITIES BE LIABLE FOR ANY
// SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
// IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
// ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF
// THIS SOFTWARE.

#include "common/str.h"
#include "private/grammar.h"
#include "private/private.h"
#include "private/tokens.h"

namespace Private {

void SymbolMaps::defineSymbol(const char *n, Common::Rect *r) {
	Common::String s(n);
	stringToDefine.push(s);
	rectToDefine.push(r);
}

/*
static void showSymbol(const Symbol *s) {
	if (s->type == NUM)
		debugC(1, kPrivateDebugCode, "%s %d",s->name->c_str(), s->u.val);
	else if (s->type == STRING)
		debugC(1, kPrivateDebugCode, "%s %s", s->name->c_str(), s->u.str);
	else if (s->type == NAME)
		debugC(1, kPrivateDebugCode, "%s %d",s->name->c_str(), s->type);
	else
		debugC(1, kPrivateDebugCode, "%s %d", s->name->c_str(), s->type);
}
*/

void setSymbol(Symbol *s, int v) {
	s->u.val = v;
}

/* find s in symbol table symlist */
static Symbol *lookup(const Common::String &s, const SymbolMap &symlist) {
	Symbol *r = symlist.getVal(s);
	return r;
}

/* install some symbol s in a symbol table */
static Symbol *install(const Common::String &n, int t, int d, const char *s, Common::Rect *r, SymbolMap *symlist) {
	Symbol *sp;
	sp = (Symbol *)malloc(sizeof(Symbol));
	sp->name = new Common::String(n);
	sp->type = t;
	if (t == NUM) {
		sp->u.val = d;
		//debug("install NUM: %s %d", name->c_str(), d);
	} else if (t == NAME) {
		sp->u.val = d;
		//debug("installing NAME: %s %d", name->c_str(), d);
	} else if (t == STRING)
		sp->u.str = scumm_strdup(s); // FIXME: leaks a string here.
	else if (t == RECT)
		sp->u.rect = r;
	else
		assert(0);

	symlist->setVal(n, sp);
	assert(symlist->size() > 0);
	return sp;
}

/* lookup some name in some symbol table */
Symbol *SymbolMaps::lookupRect(Common::String *n) {
	assert(rects.contains(*n));
	return lookup(*n, rects);
}

Symbol *SymbolMaps::lookupVariable(Common::String *n) {
	assert(variables.contains(*n));
	return lookup(*n, variables);
}

Symbol *SymbolMaps::lookupLocation(Common::String *n) {
	assert(locations.contains(*n));
	return lookup(*n, locations);
}

/* lookup some name in some symbol table */
Symbol *SymbolMaps::lookupName(const char *n) {

	Symbol *s = (Symbol *)malloc(sizeof(Symbol));
	Common::String *name = new Common::String(n);
	s->name = name;
	s->type = NAME;
	s->u.val = 0;

	return s;
}

void SymbolMaps::installAll(const char *n) {
	assert(stringToDefine.size() > 0);

	while (!stringToDefine.empty()) {
		Common::String s = stringToDefine.pop();
		Common::Rect *r = rectToDefine.pop();

		//debug("name %s", s.c_str());
		if (strcmp(n, "settings") == 0) {
			//debug("new setting %s", n);
			assert(r == nullptr);
			install(s, NAME, 0, s.c_str(), r, &settings);
		} else if (strcmp(n, "variables") == 0) {
			assert(r == nullptr);
			install(s, NAME, 0, nullptr, r, &variables);
			variableList.push_front(s);
		} else if (strcmp(n, "cursors") == 0) {
			assert(r == nullptr);
			install(s, NAME, 0, nullptr, r, &cursors);
		} else if (strcmp(n, "locations") == 0) {
			assert(r == nullptr);
			install(s, NAME, 0, nullptr, r, &locations);
			locationList.push_front(s);
		} else if (strcmp(n, "rects") == 0) {
			assert(r != nullptr);
			install(s, RECT, 0, nullptr, r, &rects);
		} else
			error("invalid symbol type");
	}
}

Symbol *SymbolMaps::constant(int t, int d, const char *s) {
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

} // End of namespace Private
