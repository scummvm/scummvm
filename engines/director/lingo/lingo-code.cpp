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

#include "director/cast.h"
#include "director/util.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-gr.h"

namespace Director {

static struct FuncDescr {
	const inst func;
	const char *name;
	const char *args;
} funcDescr[] = {
	{ 0,					"STOP",			"" },
	{ Lingo::c_xpop,		"c_xpop",		"" },
	{ Lingo::c_arraypush,	"c_arraypush",		"i" },
	{ Lingo::c_printtop,	"c_printtop",	"" },
	{ Lingo::c_constpush,	"c_constpush",	"i" },
	{ Lingo::c_voidpush,	"c_voidpush",	"" },
	{ Lingo::c_fconstpush,	"c_fconstpush",	"f" },
	{ Lingo::c_stringpush,	"c_stringpush",	"s" },
	{ Lingo::c_symbolpush,	"c_symbolpush",	"s" },	// D3
	{ Lingo::c_varpush,		"c_varpush",	"s" },
	{ Lingo::c_setImmediate,"c_setImmediate","i" },
	{ Lingo::c_assign,		"c_assign",		"" },
	{ Lingo::c_eval,		"c_eval",		"s" },
	{ Lingo::c_theentitypush,"c_theentitypush","ii" }, // entity, field
	{ Lingo::c_theentityassign,"c_theentityassign","ii" },
	{ Lingo::c_swap,		"c_swap",		"" },
	{ Lingo::c_add,			"c_add",		"" },
	{ Lingo::c_sub,			"c_sub",		"" },
	{ Lingo::c_mul,			"c_mul",		"" },
	{ Lingo::c_div,			"c_div",		"" },
	{ Lingo::c_mod,			"c_mod",		"" },
	{ Lingo::c_negate,		"c_negate",		"" },
	{ Lingo::c_ampersand,	"c_ampersand",	"" },
	{ Lingo::c_after,		"c_after",		"" },	// D3
	{ Lingo::c_before,		"c_before",		"" },	// D3
	{ Lingo::c_concat,		"c_concat",		"" },
	{ Lingo::c_contains,	"c_contains",	"" },
	{ Lingo::c_starts,		"c_starts",		"" },
	{ Lingo::c_intersects,	"c_intersects",	"" },
	{ Lingo::c_within,		"c_within",		"" },
	{ Lingo::c_charOf,		"c_charOf",		"" },	// D3
	{ Lingo::c_charToOf,	"c_charToOf",	"" },	// D3
	{ Lingo::c_itemOf,		"c_itemOf",		"" },	// D3
	{ Lingo::c_itemToOf,	"c_itemToOf",	"" },	// D3
	{ Lingo::c_lineOf,		"c_lineOf",		"" },	// D3
	{ Lingo::c_lineToOf,	"c_lineToOf",	"" },	// D3
	{ Lingo::c_wordOf,		"c_wordOf",		"" },	// D3
	{ Lingo::c_wordToOf,	"c_wordToOf",	"" },	// D3
	{ Lingo::c_and,			"c_and",		"" },
	{ Lingo::c_or,			"c_or",			"" },
	{ Lingo::c_not,			"c_not",		"" },
	{ Lingo::c_eq,			"c_eq",			"" },
	{ Lingo::c_neq,			"c_neq",		"" },
	{ Lingo::c_gt,			"c_gt",			"" },
	{ Lingo::c_lt,			"c_lt",			"" },
	{ Lingo::c_ge,			"c_ge",			"" },
	{ Lingo::c_le,			"c_le",			"" },
	{ Lingo::c_repeatwhilecode,"c_repeatwhilecode","oo" },
	{ Lingo::c_repeatwithcode,"c_repeatwithcode","ooooos" },
	{ Lingo::c_exitRepeat,	"c_exitRepeat",	"" },
	{ Lingo::c_ifcode,		"c_ifcode",		"oooi" },
	{ Lingo::c_tellcode,	"c_tellcode",	"o" },
	{ Lingo::c_whencode,	"c_whencode",	"os" },
	{ Lingo::c_goto,		"c_goto",		"" },
	{ Lingo::c_gotoloop,	"c_gotoloop",	"" },
	{ Lingo::c_gotonext,	"c_gotonext",	"" },
	{ Lingo::c_gotoprevious,"c_gotoprevious","" },
	{ Lingo::c_play,		"c_play",		"" },
	{ Lingo::c_playdone,	"c_playdone",	"" },
	{ Lingo::c_call,		"c_call",		"si" },
	{ Lingo::c_procret,		"c_procret",	"" },
	{ Lingo::c_global,		"c_global",		"s" },
	{ Lingo::c_property,	"c_property",	"s" },
	{ Lingo::c_instance,	"c_instance",	"s" },
	{ Lingo::c_open,		"c_open",		"" },
	{ 0, 0, 0 }
};

void Lingo::initFuncs() {
	Symbol sym;
	for (FuncDescr *fnc = funcDescr; fnc->name; fnc++) {
		sym.u.func = fnc->func;
		_functions[(void *)sym.u.s] = new FuncDesc(fnc->name, fnc->args);
	}
}

void Lingo::push(Datum d) {
	_stack.push_back(d);
}

void Lingo::pushVoid() {
	Datum d;
	d.u.s = NULL;
	d.type = VOID;
	push(d);
}

Datum Lingo::pop(void) {
	if (_stack.size() == 0)
		assert(0);

	Datum ret = _stack.back();
	_stack.pop_back();

	return ret;
}

void Lingo::c_xpop() {
	g_lingo->pop();
}

void Lingo::c_printtop(void) {
	Datum d = g_lingo->pop();

	switch (d.type) {
	case VOID:
		warning("Void, came from %s", d.u.s ? d.u.s->c_str() : "<>");
		break;
	case INT:
		warning("%d", d.u.i);
		break;
	case FLOAT:
		warning(g_lingo->_floatPrecisionFormat.c_str(), d.u.f);
		break;
	case VAR:
		if (!d.u.sym) {
			warning("Inconsistent stack: var, val: %d", d.u.i);
		} else {
			if (!d.u.sym->name.empty())
				warning("var: %s", d.u.sym->name.c_str());
			else
				warning("Nameless var. val: %d", d.u.sym->u.i);
		}
		break;
	case STRING:
		warning("%s", d.u.s->c_str());
		break;
	case POINT:
		warning("point(%d, %d)", (int)((*d.u.arr)[0]), (int)((*d.u.arr)[1]));
		break;
	case SYMBOL:
		warning("%s", d.type2str(true));
		break;
	case OBJECT:
		warning("#%s", d.u.s->c_str());
		break;
	default:
		warning("--unknown--");
	}
}

void Lingo::c_constpush() {
	Datum d;
	inst i = (*g_lingo->_currentScript)[g_lingo->_pc++];
	d.u.i = READ_UINT32(&i);
	d.type = INT;
	g_lingo->push(d);
}

void Lingo::c_voidpush() {
	Datum d;
	d.u.s = NULL;
	d.type = VOID;
	g_lingo->push(d);
}

void Lingo::c_fconstpush() {
	Datum d;
	inst i = (*g_lingo->_currentScript)[g_lingo->_pc];
	d.u.f = *(double *)(&i);
	d.type = FLOAT;

	g_lingo->_pc += g_lingo->calcCodeAlignment(sizeof(double));

	g_lingo->push(d);
}

void Lingo::c_stringpush() {
	char *s = (char *)&(*g_lingo->_currentScript)[g_lingo->_pc];
	g_lingo->_pc += g_lingo->calcStringAlignment(s);

	g_lingo->push(Datum(new Common::String(s)));
}

void Lingo::c_symbolpush() {
	char *s = (char *)&(*g_lingo->_currentScript)[g_lingo->_pc];
	g_lingo->_pc += g_lingo->calcStringAlignment(s);

	warning("STUB: c_symbolpush()");

	// TODO: FIXME: Must push symbol instead of string
	g_lingo->push(Datum(new Common::String(s)));
}

void Lingo::c_arraypush() {
	Datum d;
	inst v = (*g_lingo->_currentScript)[g_lingo->_pc++];
	int arraySize = READ_UINT32(&v);

	warning("STUB: c_arraypush()");

	for (int i = 0; i < arraySize; i++)
		g_lingo->pop();

	d.u.i = arraySize;
	d.type = INT;
	g_lingo->push(d);
}

void Lingo::c_varpush() {
	Common::String name((char *)&(*g_lingo->_currentScript)[g_lingo->_pc]);
	Datum d;

	g_lingo->_pc += g_lingo->calcStringAlignment(name.c_str());

	// In immediate mode we will push variables as strings
	// This is used for playAccel
	if (g_lingo->_immediateMode) {
		g_lingo->push(Datum(new Common::String(name)));

		return;
	}

	if (g_lingo->getHandler(name) != NULL) {
		d.type = HANDLER;
		d.u.s = new Common::String(name);
		g_lingo->push(d);
		return;
	}

	d.u.sym = g_lingo->lookupVar(name.c_str());
	if (d.u.sym->type == CASTREF) {
		d.type = INT;
		int val = d.u.sym->u.i;

		delete d.u.sym;

		d.u.i = val;
	} else {
		d.type = VAR;
	}

	g_lingo->push(d);
}

void Lingo::c_setImmediate() {
	inst i = (*g_lingo->_currentScript)[g_lingo->_pc++];

	g_lingo->_immediateMode = READ_UINT32(&i);
}

void Lingo::c_assign() {
	Datum d1, d2;
	d1 = g_lingo->pop();
	d2 = g_lingo->pop();

	if (d1.type != VAR && d1.type != REFERENCE) {
		warning("assignment to non-variable");
		return;
	}

	if (d1.type == REFERENCE) {
		if (!g_director->getCurrentScore()->_loadedText->contains(d1.u.i)) {
			if (!g_director->getCurrentScore()->_loadedText->contains(d1.u.i - 1024)) {
				warning("c_assign: Unknown REFERENCE %d", d1.u.i);
				g_lingo->pushVoid();
				return;
			} else {
				d1.u.i -= 1024;
			}
		}

		warning("STUB: c_assing REFERENCE");

		return;
	}

	if (d1.u.sym->type != INT && d1.u.sym->type != VOID &&
			d1.u.sym->type != FLOAT && d1.u.sym->type != STRING) {
		warning("assignment to non-variable '%s'", d1.u.sym->name.c_str());
		return;
	}

	if ((d1.u.sym->type == STRING || d1.u.sym->type == VOID) && d1.u.sym->u.s) // Free memory if needed
		delete d1.u.sym->u.s;

	if (d1.u.sym->type == POINT || d1.u.sym->type == RECT || d1.u.sym->type == ARRAY)
		delete d1.u.sym->u.arr;

	if (d2.type == INT) {
		d1.u.sym->u.i = d2.u.i;
	} else if (d2.type == FLOAT) {
		d1.u.sym->u.f = d2.u.f;
	} else if (d2.type == STRING) {
		d1.u.sym->u.s = new Common::String(*d2.u.s);
		delete d2.u.s;
	} else if (d2.type == POINT) {
		d1.u.sym->u.arr = new FloatArray(*d2.u.arr);
		delete d2.u.arr;
	} else if (d2.type == SYMBOL) {
		d1.u.sym->u.i = d2.u.i;
	} else if (d2.type == OBJECT) {
		d1.u.sym->u.s = d2.u.s;
	} else {
		warning("c_assign: unhandled type: %s", d2.type2str());
		d1.u.sym->u.s = d2.u.s;
	}

	d1.u.sym->type = d2.type;
}

bool Lingo::verify(Symbol *s) {
	if (s->type != INT && s->type != VOID && s->type != FLOAT && s->type != STRING && s->type != POINT && s->type != SYMBOL) {
		warning("attempt to evaluate non-variable '%s'", s->name.c_str());

		return false;
	}

	if (s->type == VOID)
		warning("Variable used before assigning a value '%s'", s->name.c_str());

	return true;
}

void Lingo::c_eval() {
	g_lingo->c_varpush();

	Datum d;
	d = g_lingo->pop();

	if (d.type == HANDLER) {
		g_lingo->call(*d.u.s, 0);
		delete d.u.s;
		return;
	}

	if (d.type != VAR) { // It could be cast ref
		g_lingo->push(d);
		return;
	}

	if (!g_lingo->verify(d.u.sym))
		return;

	d.type = d.u.sym->type;

	if (d.u.sym->type == INT)
		d.u.i = d.u.sym->u.i;
	else if (d.u.sym->type == FLOAT)
		d.u.f = d.u.sym->u.f;
	else if (d.u.sym->type == STRING)
		d.u.s = new Common::String(*d.u.sym->u.s);
	else if (d.u.sym->type == POINT)
		d.u.arr = d.u.sym->u.arr;
	else if (d.u.sym->type == SYMBOL)
		d.u.i = d.u.sym->u.i;
	else if (d.u.sym->type == VOID)
		d.u.s = new Common::String(d.u.sym->name);
	else
		warning("c_eval: unhandled type: %s", d.type2str());

	g_lingo->push(d);
}

void Lingo::c_theentitypush() {
	inst e = (*g_lingo->_currentScript)[g_lingo->_pc++];
	inst f = (*g_lingo->_currentScript)[g_lingo->_pc++];
	Datum id = g_lingo->pop();

	int entity = READ_UINT32(&e);
	int field  = READ_UINT32(&f);

	Datum d = g_lingo->getTheEntity(entity, id, field);
	g_lingo->push(d);
}

void Lingo::c_theentityassign() {
	inst e = (*g_lingo->_currentScript)[g_lingo->_pc++];
	inst f = (*g_lingo->_currentScript)[g_lingo->_pc++];
	Datum id = g_lingo->pop();

	int entity = READ_UINT32(&e);
	int field  = READ_UINT32(&f);

	Datum d = g_lingo->pop();
	g_lingo->setTheEntity(entity, id, field, d);
}

void Lingo::c_swap() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();
	g_lingo->push(d2);
	g_lingo->push(d1);
}

void Lingo::c_add() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	if (g_lingo->alignTypes(d1, d2) == FLOAT) {
		d1.u.f += d2.u.f;
	} else {
		d1.u.i += d2.u.i;
	}
	g_lingo->push(d1);
}

void Lingo::c_sub() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	if (g_lingo->alignTypes(d1, d2) == FLOAT) {
		d1.u.f -= d2.u.f;
	} else {
		d1.u.i -= d2.u.i;
	}
	g_lingo->push(d1);
}

void Lingo::c_mul() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	if (g_lingo->alignTypes(d1, d2) == FLOAT) {
		d1.u.f *= d2.u.f;
	} else {
		d1.u.i *= d2.u.i;
	}
	g_lingo->push(d1);
}

void Lingo::c_div() {
	Datum d2 = g_lingo->pop();

	if ((d2.type == INT && d2.u.i == 0) ||
			(d2.type == FLOAT && d2.u.f == 0.0))
		error("division by zero");

	Datum d1 = g_lingo->pop();

	if (g_lingo->alignTypes(d1, d2) == FLOAT) {
		d1.u.f /= d2.u.f;
	} else {
		d1.u.i /= d2.u.i;
	}
	g_lingo->push(d1);
}

void Lingo::c_mod() {
	Datum d2 = g_lingo->pop();
	d2.toInt();

	if (d2.u.i == 0)
		error("division by zero");

	Datum d1 = g_lingo->pop();
	d1.toInt();

	d1.u.i %= d2.u.i;

	g_lingo->push(d1);
}

void Lingo::c_negate() {
	Datum d = g_lingo->pop();

	if (d.type == INT)
		d.u.i = -d.u.i;
	else if (d.type == FLOAT)
		d.u.f = -d.u.f;

	g_lingo->push(d);
}

void Lingo::c_ampersand() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.toString();
	d2.toString();

	*d1.u.s += *d2.u.s;

	delete d2.u.s;

	g_lingo->push(d1);
}

void Lingo::c_after() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.toString();
	d2.toString();

	warning("STUB: c_after");

	delete d2.u.s;

	g_lingo->push(d1);
}

void Lingo::c_before() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.toString();
	d2.toString();

	warning("STUB: c_before");

	delete d2.u.s;

	g_lingo->push(d1);
}

void Lingo::c_concat() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.toString();
	d2.toString();

	*d1.u.s += " ";
	*d1.u.s += *d2.u.s;

	delete d2.u.s;

	g_lingo->push(d1);
}

void Lingo::c_contains() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.toString();
	d2.toString();

	Common::String *s1 = toLowercaseMac(d1.u.s);
	Common::String *s2 = toLowercaseMac(d2.u.s);

	int res = s1->contains(*s2) ? 1 : 0;

	delete d1.u.s;
	delete d2.u.s;
	delete s1;
	delete s2;

	d1.type = INT;
	d1.u.i = res;

	g_lingo->push(d1);
}

void Lingo::c_starts() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.toString();
	d2.toString();

	Common::String *s1 = toLowercaseMac(d1.u.s);
	Common::String *s2 = toLowercaseMac(d2.u.s);

	int res = s1->hasPrefix(*s2) ? 1 : 0;

	delete d1.u.s;
	delete d2.u.s;
	delete s1;
	delete s2;

	d1.type = INT;
	d1.u.i = res;

	g_lingo->push(d1);
}

void Lingo::c_intersects() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	warning("STUB: c_intersects: %d %d", d1.u.i, d2.u.i);

	g_lingo->push(d1);
}

void Lingo::c_within() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	warning("STUB: c_within: %d %d", d1.u.i, d2.u.i);

	g_lingo->push(d1);
}

void Lingo::c_charOf() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	warning("STUB: c_charOf: %d %d", d1.u.i, d2.u.i);

	g_lingo->push(d1);
}

void Lingo::c_charToOf() {
	Datum d3 = g_lingo->pop();
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	warning("STUB: c_charToOf: %d %d %d", d1.u.i, d2.u.i, d3.u.i);

	g_lingo->push(d1);
}

void Lingo::c_itemOf() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	warning("STUB: c_itemOf: %d %d", d1.u.i, d2.u.i);

	g_lingo->push(d1);
}

void Lingo::c_itemToOf() {
	Datum d3 = g_lingo->pop();
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	warning("STUB: c_itemToOf: %d %d %d", d1.u.i, d2.u.i, d3.u.i);

	g_lingo->push(d1);
}

void Lingo::c_lineOf() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	warning("STUB: c_lineOf: %d %d", d1.u.i, d2.u.i);

	g_lingo->push(d1);
}

void Lingo::c_lineToOf() {
	Datum d3 = g_lingo->pop();
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	warning("STUB: c_lineToOf: %d %d %d", d1.u.i, d2.u.i, d3.u.i);

	g_lingo->push(d1);
}

void Lingo::c_wordOf() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	warning("STUB: c_wordOf: %d %d", d1.u.i, d2.u.i);

	g_lingo->push(d1);
}

void Lingo::c_wordToOf() {
	Datum d3 = g_lingo->pop();
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	warning("STUB: c_wordToOf: %d %d %d", d1.u.i, d2.u.i, d3.u.i);

	g_lingo->push(d1);
}

void Lingo::c_and() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.toInt();
	d2.toInt();

	d1.u.i = (d1.u.i && d2.u.i) ? 1 : 0;

	g_lingo->push(d1);
}

void Lingo::c_or() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.toInt();
	d2.toInt();

	d1.u.i = (d1.u.i || d2.u.i) ? 1 : 0;

	g_lingo->push(d1);
}

void Lingo::c_not() {
	Datum d = g_lingo->pop();

	d.toInt();

	d.u.i = ~d.u.i ? 1 : 0;

	g_lingo->push(d);
}

void Lingo::c_eq() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	if (g_lingo->alignTypes(d1, d2) == FLOAT) {
		d1.u.i = (d1.u.f == d2.u.f) ? 1 : 0;
		d1.type = INT;
	} else {
		d1.u.i = (d1.u.i == d2.u.i) ? 1 : 0;
	}
	g_lingo->push(d1);
}

void Lingo::c_neq() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	if (g_lingo->alignTypes(d1, d2) == FLOAT) {
		d1.u.i = (d1.u.f != d2.u.f) ? 1 : 0;
		d1.type = INT;
	} else {
		d1.u.i = (d1.u.i != d2.u.i) ? 1 : 0;
	}
	g_lingo->push(d1);
}

void Lingo::c_gt() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	if (g_lingo->alignTypes(d1, d2) == FLOAT) {
		d1.u.i = (d1.u.f > d2.u.f) ? 1 : 0;
		d1.type = INT;
	} else {
		d1.u.i = (d1.u.i > d2.u.i) ? 1 : 0;
	}
	g_lingo->push(d1);
}

void Lingo::c_lt() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	if (g_lingo->alignTypes(d1, d2) == FLOAT) {
		d1.u.i = (d1.u.f < d2.u.f) ? 1 : 0;
		d1.type = INT;
	} else {
		d1.u.i = (d1.u.i < d2.u.i) ? 1 : 0;
	}
	g_lingo->push(d1);
}

void Lingo::c_ge() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	if (g_lingo->alignTypes(d1, d2) == FLOAT) {
		d1.u.i = (d1.u.f >= d2.u.f) ? 1 : 0;
		d1.type = INT;
	} else {
		d1.u.i = (d1.u.i >= d2.u.i) ? 1 : 0;
	}
	g_lingo->push(d1);
}

void Lingo::c_le() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	if (g_lingo->alignTypes(d1, d2) == FLOAT) {
		d1.u.i = (d1.u.f <= d2.u.f) ? 1 : 0;
		d1.type = INT;
	} else {
		d1.u.i = (d1.u.i <= d2.u.i) ? 1 : 0;
	}
	g_lingo->push(d1);
}

void Lingo::c_repeatwhilecode(void) {
	Datum d;
	int savepc = g_lingo->_pc;

	uint body = READ_UINT32(&(*g_lingo->_currentScript)[savepc]);
	uint end =  READ_UINT32(&(*g_lingo->_currentScript)[savepc + 1]);

	g_lingo->execute(savepc + 2);	/* condition */
	d = g_lingo->pop();
	d.toInt();

	while (d.u.i) {
		g_lingo->execute(body + savepc - 1);	/* body */
		if (g_lingo->_returning)
			break;

		if (g_lingo->_exitRepeat) {
			g_lingo->_exitRepeat = false;
			break;
		}

		g_lingo->execute(savepc + 2);	/* condition */
		d = g_lingo->pop();
		d.toInt();
	}

	if (!g_lingo->_returning)
		g_lingo->_pc = end + savepc - 1; /* next stmt */
}

void Lingo::c_repeatwithcode(void) {
	Datum d;
	int savepc = g_lingo->_pc;

	uint init = READ_UINT32(&(*g_lingo->_currentScript)[savepc]);
	uint finish =  READ_UINT32(&(*g_lingo->_currentScript)[savepc + 1]);
	uint body = READ_UINT32(&(*g_lingo->_currentScript)[savepc + 2]);
	int inc = (int32)READ_UINT32(&(*g_lingo->_currentScript)[savepc + 3]);
	uint end =  READ_UINT32(&(*g_lingo->_currentScript)[savepc + 4]);
	Common::String countername((char *)&(*g_lingo->_currentScript)[savepc + 5]);
	Symbol *counter = g_lingo->lookupVar(countername.c_str());

	if (counter->type == CASTREF) {
		error("Cast ref used as index: %s", countername.c_str());
	}

	g_lingo->execute(init + savepc - 1);	/* condition */
	d = g_lingo->pop();
	d.toInt();
	counter->u.i = d.u.i;
	counter->type = INT;

	while (true) {
		g_lingo->execute(body + savepc - 1);	/* body */
		if (g_lingo->_returning)
			break;

		if (g_lingo->_exitRepeat) {
			g_lingo->_exitRepeat = false;
			break;
		}

		counter->u.i += inc;
		g_lingo->execute(finish + savepc - 1);	/* condition */
		d = g_lingo->pop();
		d.toInt();

		if (counter->u.i == d.u.i + inc)
			break;
	}

	if (!g_lingo->_returning)
		g_lingo->_pc = end + savepc - 1; /* next stmt */
}

void Lingo::c_exitRepeat(void) {
	g_lingo->_exitRepeat = true;
}

void Lingo::c_ifcode() {
	Datum d;
	int savepc = g_lingo->_pc;	/* then part */

	uint then =    READ_UINT32(&(*g_lingo->_currentScript)[savepc]);
	uint elsep =   READ_UINT32(&(*g_lingo->_currentScript)[savepc + 1]);
	uint end =     READ_UINT32(&(*g_lingo->_currentScript)[savepc + 2]);
	uint skipEnd = READ_UINT32(&(*g_lingo->_currentScript)[savepc + 3]);

	debugC(8, kDebugLingoExec, "executing cond (have to %s end)", skipEnd ? "skip" : "execute");
	g_lingo->execute(savepc + 4);	/* condition */

	d = g_lingo->pop();

	if (d.toInt()) {
		debugC(8, kDebugLingoExec, "executing then");
		g_lingo->execute(then + savepc - 1);
	} else if (elsep) { /* else part? */
		debugC(8, kDebugLingoExec, "executing else");
		g_lingo->execute(elsep + savepc - 1);
	}

	if (!g_lingo->_returning && !skipEnd) {
		g_lingo->_pc = end + savepc - 1; /* next stmt */
		debugC(8, kDebugLingoExec, "executing end");
	} else {
		debugC(8, kDebugLingoExec, "Skipped end");
	}
}

void Lingo::c_whencode() {
	Datum d;
	uint start = g_lingo->_pc;
	uint end = READ_UINT32(&(*g_lingo->_currentScript)[start]) + start - 1;
	Common::String eventname((char *)&(*g_lingo->_currentScript)[start + 1]);

	start += g_lingo->calcStringAlignment(eventname.c_str()) + 1;

	debugC(1, kDebugLingoExec, "c_whencode([%5d][%5d], %s)", start, end, eventname.c_str());

	int entity = g_lingo->_currentEntityId;
	g_lingo->_currentEntityId = 0;

	g_lingo->define(eventname, start, 0, NULL, end);

	g_lingo->_currentEntityId = entity;

	if (debugChannelSet(1, kDebugLingoExec)) {
		uint pc = start;
		while (pc <= end) {
			Common::String instr = g_lingo->decodeInstruction(pc, &pc);
			debugC(1, kDebugLingoExec, "[%5d] %s", pc, instr.c_str());
		}
	}

	g_lingo->_pc = end;
}

void Lingo::c_tellcode() {
	warning("STUB: c_tellcode");
}


//************************
// Built-in functions
//************************
void Lingo::c_goto() {
	Datum mode = g_lingo->pop();
	Datum frame, movie;

	if (mode.u.i == 2 || mode.u.i == 3)
		movie = g_lingo->pop();

	if (mode.u.i == 1 || mode.u.i == 3)
		frame = g_lingo->pop();

	g_lingo->func_goto(frame, movie);
}

void Lingo::c_gotoloop() {
	g_lingo->func_gotoloop();
}

void Lingo::c_gotonext() {
	g_lingo->func_gotonext();
}

void Lingo::c_gotoprevious() {
	g_lingo->func_gotoprevious();
}

void Lingo::c_play() {
	Datum mode = g_lingo->pop();
	Datum frame, movie;

	if (mode.u.i == 2 || mode.u.i == 3)
		movie = g_lingo->pop();

	if (mode.u.i == 1 || mode.u.i == 3)
		frame = g_lingo->pop();

	g_lingo->func_play(frame, movie);
}

void Lingo::c_playdone() {
	g_lingo->func_playdone();
}

void Lingo::c_call() {
	Common::String name((char *)&(*g_lingo->_currentScript)[g_lingo->_pc]);
	g_lingo->_pc += g_lingo->calcStringAlignment(name.c_str());

	int nargs = READ_UINT32(&(*g_lingo->_currentScript)[g_lingo->_pc++]);

	g_lingo->call(name, nargs);
}

void Lingo::call(Common::String name, int nargs) {
	bool dropArgs = false;

	if (debugChannelSet(3, kDebugLingoExec))
		printSTUBWithArglist(name.c_str(), nargs, "call:");

	Symbol *sym = g_lingo->getHandler(name);

	if (!g_lingo->_eventHandlerTypeIds.contains(name)) {
		Symbol *s = g_lingo->lookupVar(name.c_str(), false);
		if (s && s->type == OBJECT) {
			debugC(3, kDebugLingoExec,  "Dereferencing object reference: %s to %s", name.c_str(), s->u.s->c_str());
			name = *s->u.s;
			sym = g_lingo->getHandler(name);
		}
	}

	if (sym == NULL) {
		warning("Call to undefined handler '%s'. Dropping %d stack items", name.c_str(), nargs);
		dropArgs = true;
	} else {
		if ((sym->type == BLTIN || sym->type == FBLTIN || sym->type == RBLTIN)
				&& sym->nargs != -1 && sym->nargs != nargs && sym->maxArgs != nargs) {
			if (sym->nargs == sym->maxArgs)
				warning("Incorrect number of arguments to handler '%s', expecting %d. Dropping %d stack items", name.c_str(), sym->nargs, nargs);
			else
				warning("Incorrect number of arguments to handler '%s', expecting %d or %d. Dropping %d stack items", name.c_str(), sym->nargs, sym->maxArgs, nargs);

			dropArgs = true;
		}
	}

	if (dropArgs) {
		for (int i = 0; i < nargs; i++)
			g_lingo->pop();

		// Push dummy value
		g_lingo->pushVoid();

		return;
	}

	if (sym->nargs != -1 && sym->maxArgs < nargs) {
		warning("Incorrect number of arguments for function %s (%d, expected %d to %d). Dropping extra %d",
					name.c_str(), nargs, sym->nargs, sym->maxArgs, nargs - sym->nargs);
		for (int i = 0; i < nargs - sym->maxArgs; i++)
			g_lingo->pop();
	}

	if (sym->type == BLTIN || sym->type == FBLTIN || sym->type == RBLTIN) {
		if (sym->u.bltin == b_factory) {
			g_lingo->factoryCall(name, nargs);
		} else {
			int stackSize = _stack.size() - nargs;

			(*sym->u.bltin)(nargs);

			int stackNewSize = _stack.size();

			if (sym->type == FBLTIN || sym->type == RBLTIN) {
				if (stackNewSize - stackSize != 1)
					warning("built-in function %s did not return value", name.c_str());
			} else {
				if (stackNewSize - stackSize != 0)
					warning("built-in procedure %s returned extra %d values", name.c_str(), stackNewSize - stackSize);
			}
		}

		return;
	}

	for (int i = nargs; i < sym->nargs; i++) {
		Datum d;

		d.u.s = NULL;
		d.type = VOID;
		g_lingo->push(d);
	}

	debugC(5, kDebugLingoExec, "Pushing frame %d", g_lingo->_callstack.size() + 1);
	CFrame *fp = new CFrame;

	fp->sp = sym;
	fp->retpc = g_lingo->_pc;
	fp->retscript = g_lingo->_currentScript;
	fp->localvars = g_lingo->_localvars;

	// Create new set of local variables
	g_lingo->_localvars = new SymbolHash;

	g_lingo->_callstack.push_back(fp);

	g_lingo->_currentScript = sym->u.defn;
	g_lingo->execute(0);

	g_lingo->_returning = false;
}

void Lingo::c_procret() {
	if (!g_lingo->_callstack.size()) {
		warning("c_procret: Call stack underflow");
		g_lingo->_returning = true;
		return;
	}

	debugC(5, kDebugLingoExec, "Popping frame %d", g_lingo->_callstack.size() + 1);

	CFrame *fp = g_lingo->_callstack.back();
	g_lingo->_callstack.pop_back();

	g_lingo->_currentScript = fp->retscript;
	g_lingo->_pc = fp->retpc;

	g_lingo->cleanLocalVars();

	// Restore local variables
	g_lingo->_localvars = fp->localvars;

	delete fp;

	g_lingo->_returning = true;
}

void Lingo::c_global() {
	Common::String name((char *)&(*g_lingo->_currentScript)[g_lingo->_pc]);

	Symbol *s = g_lingo->lookupVar(name.c_str(), false);
	if (s && !s->global) {
		warning("Local variable %s declared as global", name.c_str());
	}

	s = g_lingo->lookupVar(name.c_str(), true, true);
	s->global = true;

	g_lingo->_pc += g_lingo->calcStringAlignment(name.c_str());
}

void Lingo::c_property() {
	Common::String name((char *)&(*g_lingo->_currentScript)[g_lingo->_pc]);

	g_lingo->_pc += g_lingo->calcStringAlignment(name.c_str());

	warning("STUB: c_property()");
}

void Lingo::c_instance() {
	Common::String name((char *)&(*g_lingo->_currentScript)[g_lingo->_pc]);

	warning("STUB: c_instance(%s)", name.c_str());

	g_lingo->_pc += g_lingo->calcStringAlignment(name.c_str());
}

void Lingo::c_open() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.toString();
	d2.toString();

	warning("STUB: c_open(%s, %s)", d1.u.s->c_str(), d2.u.s->c_str());
}

}
