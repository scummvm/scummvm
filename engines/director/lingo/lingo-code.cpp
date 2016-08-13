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

#include "engines/director/lingo/lingo.h"
#include "common/file.h"
#include "audio/decoders/wave.h"

#include "director/lingo/lingo-gr.h"

namespace Director {

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
		error("stack underflow");

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
			if (d.u.sym->name)
				warning("var: %s", d.u.sym->name);
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
	d.u.f = *((double *)&i);
	d.type = FLOAT;

	g_lingo->_pc += g_lingo->calcCodeAlignment(sizeof(double));

	g_lingo->push(d);
}

void Lingo::c_stringpush() {
	char *s = (char *)&(*g_lingo->_currentScript)[g_lingo->_pc];
	g_lingo->_pc += g_lingo->calcStringAlignment(s);

	g_lingo->push(Datum(new Common::String(s)));
}

void Lingo::c_varpush() {
	char *name = (char *)&(*g_lingo->_currentScript)[g_lingo->_pc];
	Datum d;

	d.u.sym = g_lingo->lookupVar(name);
	if (d.u.sym->type == CASTREF) {
		d.type = INT;
		int val = d.u.sym->u.i;

		delete d.u.sym;

		d.u.i = val;
	} else {
		d.type = VAR;
	}

	g_lingo->_pc += g_lingo->calcStringAlignment(name);

	g_lingo->push(d);
}

void Lingo::c_assign() {
	Datum d1, d2;
	d1 = g_lingo->pop();
	d2 = g_lingo->pop();

	if (d1.type != VAR) {
		warning("assignment to non-variable");
		return;
	}

	if (d1.u.sym->type != INT && d1.u.sym->type != VOID &&
			d1.u.sym->type != FLOAT && d1.u.sym->type != STRING) {
		warning("assignment to non-variable '%s'", d1.u.sym->name);
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
	} else {
		warning("c_assign: unhandled type: %s", d2.type2str());
	}

	d1.u.sym->type = d2.type;

	g_lingo->push(d1);
}

bool Lingo::verify(Symbol *s) {
	if (s->type != INT && s->type != VOID && s->type != FLOAT && s->type != STRING && s->type != POINT) {
		warning("attempt to evaluate non-variable '%s'", s->name);

		return false;
	}

	if (s->type == VOID)
		warning("Variable used before assigning a value '%s'", s->name);

	return true;
}

void Lingo::c_eval() {
	g_lingo->c_varpush();

	Datum d;
	d = g_lingo->pop();

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

	g_lingo->push(d); // Dummy value
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

	Common::String *s1 = g_lingo->toLowercaseMac(d1.u.s);
	Common::String *s2 = g_lingo->toLowercaseMac(d2.u.s);

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

	Common::String *s1 = g_lingo->toLowercaseMac(d1.u.s);
	Common::String *s2 = g_lingo->toLowercaseMac(d2.u.s);

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

	warning("STUB: c_intersects: %d", d2.u.i);

	g_lingo->push(d1);
}

void Lingo::c_within() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	warning("STUB: c_within: %d", d2.u.i);

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

	int body = READ_UINT32(&(*g_lingo->_currentScript)[savepc]);
	int end =  READ_UINT32(&(*g_lingo->_currentScript)[savepc + 1]);

	g_lingo->execute(savepc + 2);	/* condition */
	d = g_lingo->pop();
	d.toInt();

	while (d.u.i) {
		g_lingo->execute(body);	/* body */
		if (g_lingo->_returning)
			break;

		g_lingo->execute(savepc + 2);	/* condition */
		d = g_lingo->pop();
		d.toInt();
	}

	if (!g_lingo->_returning)
		g_lingo->_pc = end; /* next stmt */
}

void Lingo::c_repeatwithcode(void) {
	Datum d;
	int savepc = g_lingo->_pc;

	int init = READ_UINT32(&(*g_lingo->_currentScript)[savepc]);
	int finish =  READ_UINT32(&(*g_lingo->_currentScript)[savepc + 1]);
	int body = READ_UINT32(&(*g_lingo->_currentScript)[savepc + 2]);
	int inc = (int32)READ_UINT32(&(*g_lingo->_currentScript)[savepc + 3]);
	int end =  READ_UINT32(&(*g_lingo->_currentScript)[savepc + 4]);
	Common::String countername((char *)&(*g_lingo->_currentScript)[savepc + 5]);
	Symbol *counter = g_lingo->lookupVar(countername.c_str());

	if (counter->type == CASTREF) {
		error("Cast ref used as index: %s", countername.c_str());
	}

	g_lingo->execute(init);	/* condition */
	d = g_lingo->pop();
	d.toInt();
	counter->u.i = d.u.i;
	counter->type = INT;

	while (true) {
		g_lingo->execute(body);	/* body */
		if (g_lingo->_returning)
			break;

		counter->u.i += inc;
		g_lingo->execute(finish);	/* condition */
		d = g_lingo->pop();
		d.toInt();

		if (counter->u.i == d.u.i + inc)
			break;
	}

	if (!g_lingo->_returning)
		g_lingo->_pc = end; /* next stmt */
}

void Lingo::c_ifcode() {
	Datum d;
	int savepc = g_lingo->_pc;	/* then part */

	int then =    READ_UINT32(&(*g_lingo->_currentScript)[savepc]);
	int elsep =   READ_UINT32(&(*g_lingo->_currentScript)[savepc + 1]);
	int end =     READ_UINT32(&(*g_lingo->_currentScript)[savepc + 2]);
	int skipEnd = READ_UINT32(&(*g_lingo->_currentScript)[savepc + 3]);

	debug(8, "executing cond (have to %s end)", skipEnd ? "skip" : "execute");
	g_lingo->execute(savepc + 4);	/* condition */

	d = g_lingo->pop();

	if (d.toInt()) {
		debug(8, "executing then");
		g_lingo->execute(then);
	} else if (elsep) { /* else part? */
		debug(8, "executing else");
		g_lingo->execute(elsep);
	}

	if (!g_lingo->_returning && !skipEnd) {
		g_lingo->_pc = end; /* next stmt */
		debug(8, "executing end");
	} else
		debug(8, "Skipped end");
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

	if (frame.type == VOID) {
		frame.u.s = new Common::String("<void>");
		frame.type = STRING;
	}
	frame.toString();

	if (movie.type == VOID) {
		movie.u.s = new Common::String("<void>");
		movie.type = STRING;
	}
	movie.toString();

	warning("STUB: c_play(%s, %s)", frame.u.s->c_str(), movie.u.s->c_str());
}

void Lingo::c_playdone() {
	warning("STUB: c_playdone()");
}

void Lingo::c_call() {
	Common::String name((char *)&(*g_lingo->_currentScript)[g_lingo->_pc]);
	g_lingo->_pc += g_lingo->calcStringAlignment(name.c_str());

	int nargs = READ_UINT32(&(*g_lingo->_currentScript)[g_lingo->_pc++]);

	g_lingo->call(name, nargs);
}

void Lingo::call(Common::String &name, int nargs) {
	bool drop = false;

	Symbol *sym;

	if (!g_lingo->_handlers.contains(name)) {
		Symbol *s = g_lingo->lookupVar(name.c_str(), false);
		if (s && s->type == OBJECT) {
			debug(3, "Dereferencing object reference: %s to %s", name.c_str(), s->u.s->c_str());
			name = *s->u.s;
		}
	}

	if (!g_lingo->_handlers.contains(name)) {
		warning("Call to undefined handler '%s'. Dropping %d stack items", name.c_str(), nargs);
		drop = true;
	} else {
		sym = g_lingo->_handlers[name];

		if (sym->type == BLTIN && sym->nargs != -1 && sym->nargs != nargs && sym->maxArgs != nargs) {
			if (sym->nargs == sym->maxArgs)
				warning("Incorrect number of arguments to handler '%s', expecting %d. Dropping %d stack items", name.c_str(), sym->nargs, nargs);
			else
				warning("Incorrect number of arguments to handler '%s', expecting %d or %d. Dropping %d stack items", name.c_str(), sym->nargs, sym->maxArgs, nargs);

			drop = true;
		}
	}

	if (drop) {
		for (int i = 0; i < nargs; i++)
			g_lingo->pop();

		// Push dummy value
		g_lingo->pushVoid();

		return;
	}

	if (sym->nargs != -1 && sym->nargs < nargs) {
		warning("Incorrect number of arguments for function %s. Dropping extra %d", name.c_str(), nargs - sym->nargs);
		for (int i = 0; i < nargs - sym->nargs; i++)
			g_lingo->pop();
	}

	if (sym->type == BLTIN) {
		if (sym->u.bltin == b_factory)
			g_lingo->factoryCall(name, nargs);
		else
			(*sym->u.bltin)(nargs);

		return;
	}

	for (int i = nargs; i < sym->nargs; i++) {
		Datum d;

		d.u.s = NULL;
		d.type = VOID;
		g_lingo->push(d);
	}

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
		warning("Call stack underflow");
		g_lingo->_returning = true;
		return;
	}

	CFrame *fp = g_lingo->_callstack.back();

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

void Lingo::c_open() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.toString();
	d2.toString();

	warning("STUB: c_open(%s, %s)", d1.u.s->c_str(), d2.u.s->c_str());
}

}
