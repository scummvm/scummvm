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

#include "private/grammar.h"
#include "private/private.h"
#include "private/tokens.h"
#include "common/str.h"

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
	Common::String *name = new Common::String(n);

	Symbol *sp;

	sp = (Symbol *)malloc(sizeof(Symbol));
	sp->name = name;
	sp->type = t;
	if (t == NUM || t == NAME)
		sp->u.val = d;
	else if (t == STRING)
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
Symbol *SymbolMaps::lookupName(const char *n) {
	//debug("looking up %s", n);
	Common::String s(n);

	if (settings.contains(s))
		return lookup(s, settings);

	else if (variables.contains(s))
		return lookup(s, variables);

	else if (cursors.contains(s))
		return lookup(s, cursors);

	else if (locations.contains(s))
		return lookup(s, locations);

	else if (rects.contains(s))
		return lookup(s, rects);

	else {
		debugC(1, kPrivateDebugCode, "WARNING: %s not defined", n);
		return constant(STRING, 0, n);
	}
}

void SymbolMaps::installAll(const char *n) {
	assert(stringToDefine.size() > 0);

	while (!stringToDefine.empty()) {
		Common::String s = stringToDefine.pop();
		Common::Rect *r = rectToDefine.pop();

		//debug("name %s", s.c_str());
		if (strcmp(n, "settings") == 0) {
			assert(r == NULL);
			install(s, STRING, 0, s.c_str(), r, &settings);
		} else if (strcmp(n, "variables") == 0) {
			assert(r == NULL);
			install(s, NAME, 0, NULL, r, &variables);
			variableList.push_front(s);
		} else if (strcmp(n, "cursors") == 0) {
			assert(r == NULL);
			install(s, NAME, 0, NULL, r, &cursors);
		} else if (strcmp(n, "locations") == 0) {
			assert(r == NULL);
			install(s, NAME, 0, NULL, r, &locations);
			locationList.push_front(s);
		} else if (strcmp(n, "rects") == 0) {
			assert(r != NULL);
			install(s, RECT, 0, NULL, r, &rects);
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
