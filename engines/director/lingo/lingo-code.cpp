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

void Lingo::execute(int pc) {
	for(_pc = pc; (*_currentScript)[_pc] != STOP && !_returning;) {
		_pc++;
		(*((*_currentScript)[_pc - 1]))();
	}
}

Symbol *Lingo::lookupVar(const char *name) {
	Symbol *sym;

	if (!_vars.contains(name)) { // Create variable if it was not defined
		sym = new Symbol;
		sym->name = (char *)calloc(strlen(name) + 1, 1);
		Common::strlcpy(sym->name, name, strlen(name) + 1);
		sym->type = VOID;
		sym->u.val = 0;

		_vars[name] = sym;
	} else {
		sym = g_lingo->_vars[name];
	}

	return sym;
}

void Lingo::push(Datum d) {
	_stack.push_back(d);
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

	warning("%d", d.val);
}

void Lingo::c_constpush() {
	Datum d;
	inst i = (*g_lingo->_currentScript)[g_lingo->_pc++];
	d.val = READ_UINT32(&i);
	g_lingo->push(d);
}

void Lingo::c_varpush() {
	char *name = (char *)&(*g_lingo->_currentScript)[g_lingo->_pc];
	Datum d;

	d.sym = g_lingo->lookupVar(name);

	g_lingo->_pc += g_lingo->calcStringAlignment(name);

	g_lingo->push(d);
}

void Lingo::c_assign() {
	Datum d1, d2;
	d1 = g_lingo->pop();
	d2 = g_lingo->pop();

	if (d1.sym->type != INT && d1.sym->type != VOID) {
		warning("assignment to non-variable '%s'", d1.sym->name);
		return;
	}

	d1.sym->u.val = d2.val;
	d1.sym->type = INT;
	g_lingo->push(d2);
}

bool Lingo::verify(Symbol *s) {
	if (s->type != INT && s->type != VOID) {
		warning("attempt to evaluate non-variable '%s'", s->name);

		return false;
	}

	if (s->type == VOID) {
		warning("Variable used before assigning a value '%s'", s->name);

		return false;
	}

	return true;
}

void Lingo::c_eval() {
	Datum d;
	d = g_lingo->pop();

	if (!g_lingo->verify(d.sym))
		return;

	d.val = d.sym->u.val;

	g_lingo->push(d);
}

void Lingo::c_add() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.val += d2.val;
	g_lingo->push(d1);
}

void Lingo::c_sub() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.val -= d2.val;
	g_lingo->push(d1);
}

void Lingo::c_mul() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.val *= d2.val;
	g_lingo->push(d1);
}

void Lingo::c_div() {
	Datum d2 = g_lingo->pop();

	if (d2.val == 0)
		error("division by zero");

	Datum d1 = g_lingo->pop();

	d1.val /= d2.val;
	g_lingo->push(d1);
}

void Lingo::c_negate() {
	Datum d = g_lingo->pop();

	d.val -= d.val;
	g_lingo->push(d);
}

void Lingo::c_eq() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.val = (d1.val == d2.val) ? 1 : 0;
	g_lingo->push(d1);
}

void Lingo::c_neq() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.val = (d1.val != d2.val) ? 1 : 0;
	g_lingo->push(d1);
}

void Lingo::c_gt() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.val = (d1.val > d2.val) ? 1 : 0;
	g_lingo->push(d1);
}

void Lingo::c_lt() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.val = (d1.val < d2.val) ? 1 : 0;
	g_lingo->push(d1);
}

void Lingo::c_ge() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.val = (d1.val >= d2.val) ? 1 : 0;
	g_lingo->push(d1);
}

void Lingo::c_le() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.val = (d1.val <= d2.val) ? 1 : 0;
	g_lingo->push(d1);
}

void Lingo::c_repeatwhilecode(void) {
	Datum d;
	int savepc = g_lingo->_pc;

	int body = READ_UINT32(&(*g_lingo->_currentScript)[savepc]);
	int end =  READ_UINT32(&(*g_lingo->_currentScript)[savepc + 1]);

	g_lingo->execute(savepc + 2);	/* condition */
	d = g_lingo->pop();

	while (d.val) {
		g_lingo->execute(body);	/* body */
		if (g_lingo->_returning)
			break;

		g_lingo->execute(savepc + 2);	/* condition */
		d = g_lingo->pop();
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

	g_lingo->execute(init);	/* condition */
	d = g_lingo->pop();
	counter->u.val = d.val;
	counter->type = INT;

	while (true) {
		g_lingo->execute(body);	/* body */
		if (g_lingo->_returning)
			break;

		counter->u.val += inc;
		g_lingo->execute(finish);	/* condition */
		d = g_lingo->pop();

		if (counter->u.val == d.val + inc)
			break;
	}

	if (!g_lingo->_returning)
		g_lingo->_pc = end; /* next stmt */
}

void Lingo::c_ifcode() {
	Datum d;
	int savepc = g_lingo->_pc;	/* then part */

	int then =  READ_UINT32(&(*g_lingo->_currentScript)[savepc]);
	int elsep = READ_UINT32(&(*g_lingo->_currentScript)[savepc + 1]);
	int end =   READ_UINT32(&(*g_lingo->_currentScript)[savepc + 2]);

	g_lingo->execute(savepc + 3);	/* condition */

	d = g_lingo->pop();

	if (d.val) {
		g_lingo->execute(then);
	} else if (elsep) { /* else part? */
		g_lingo->execute(elsep);
	}

	if (!g_lingo->_returning)
		g_lingo->_pc = end; /* next stmt */
}

//************************
// Built-in functions
//************************
void Lingo::c_exit() {
	warning("STUB: c_exit()");
}

void Lingo::c_mci() {
	Common::String s((char *)&(*g_lingo->_currentScript)[g_lingo->_pc]);

	g_lingo->func_mci(s);

	g_lingo->_pc += g_lingo->calcStringAlignment(s.c_str());
}

void Lingo::c_mciwait() {
	Common::String s((char *)&(*g_lingo->_currentScript)[g_lingo->_pc]);

	g_lingo->func_mciwait(s);

	g_lingo->_pc += g_lingo->calcStringAlignment(s.c_str());
}

void Lingo::c_goto() {
	Common::String frame((char *)&(*g_lingo->_currentScript)[g_lingo->_pc]);
	g_lingo->_pc += g_lingo->calcStringAlignment(frame.c_str());

	Common::String movie((char *)&(*g_lingo->_currentScript)[g_lingo->_pc]);
	g_lingo->_pc += g_lingo->calcStringAlignment(movie.c_str());

	g_lingo->func_goto(frame, movie);
}

void Lingo::c_gotoloop() {
	warning("STUB: c_gotoloop()");
}

void Lingo::c_gotonext() {
	warning("STUB: c_gotonext()");
}

void Lingo::c_gotoprevious() {
	warning("STUB: c_gotoprevious()");
}

void Lingo::define(Common::String &name, int start, int end, int nargs) {
	warning("STUB: define(\"%s\", %d, %d, %d)", name.c_str(), start, end, nargs);

	Symbol *sym;

	if (!_handlers.contains(name)) { // Create variable if it was not defined
		sym = new Symbol;

		delete sym->u.defn;

		_handlers[name] = sym;
	} else {
		sym = g_lingo->_handlers[name];

		sym->name = (char *)calloc(name.size() + 1, 1);
		Common::strlcpy(sym->name, name.c_str(), name.size() + 1);
		sym->type = HANDLER;
	}

	sym->u.defn = new ScriptData(&(*_currentScript)[start], end - start + 1);
}

void Lingo::codeArg(Common::String &s) {
	g_lingo->code1(g_lingo->c_varpush);
	g_lingo->codeString(s.c_str());
	g_lingo->code1(g_lingo->c_assign);
}

void Lingo::c_procret() {
	warning("STUB: c_procret()");
}

}
