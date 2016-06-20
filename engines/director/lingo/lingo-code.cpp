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
	for(_pc = pc; (*_currentScript)[_pc] != STOP;) {
		_pc++;
		(*((*_currentScript)[_pc - 1]))();
	}
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

void Lingo::func_xpop() {
	g_lingo->pop();
}

void Lingo::func_printtop(void) {
	Datum d = g_lingo->pop();

	warning("%d", d.val);
}

void Lingo::func_constpush() {
	Datum d;
	inst i = (*g_lingo->_currentScript)[g_lingo->_pc++];
	d.val = READ_LE_UINT32(&i);
	g_lingo->push(d);
}

void Lingo::func_varpush() {
	Datum d;
	Symbol *sym;
	char *name = (char *)&(*g_lingo->_currentScript)[g_lingo->_pc];

	if (!g_lingo->_vars.contains(name)) { // Create variable if it was not defined
		sym = new Symbol;
		sym->name = (char *)calloc(strlen(name) + 1, 1);
		Common::strlcpy(sym->name, name, strlen(name) + 1);
		sym->type = UNDEF;
		sym->u.val = 0;

		g_lingo->_vars[name] = sym;
	} else {
		sym = g_lingo->_vars[name];
	}

	d.sym = sym;

	g_lingo->_pc += g_lingo->calcStringAlignment(name);

	g_lingo->push(d);
}

void Lingo::func_assign() {
	Datum d1, d2;
	d1 = g_lingo->pop();
	d2 = g_lingo->pop();

	if (d1.sym->type != VAR && d1.sym->type != UNDEF) {
		warning("assignment to non-variable '%s'", d1.sym->name);
		return;
	}

	d1.sym->u.val = d2.val;
	d1.sym->type = VAR;
	g_lingo->push(d2);
}

bool Lingo::verify(Symbol *s) {
	if (s->type != VAR && s->type != UNDEF) {
		warning("attempt to evaluate non-variable '%s'", s->name);

		return false;
	}

	if (s->type == UNDEF) {
		warning("undefined variable '%s'", s->name);

		return false;
	}

	return true;
}

void Lingo::func_eval() {
	Datum d;
	d = g_lingo->pop();

	if (!g_lingo->verify(d.sym))
		return;

	d.val = d.sym->u.val;

	g_lingo->push(d);
}

void Lingo::func_add() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.val += d2.val;
	g_lingo->push(d1);
}

void Lingo::func_sub() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.val -= d2.val;
	g_lingo->push(d1);
}

void Lingo::func_mul() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.val *= d2.val;
	g_lingo->push(d1);
}

void Lingo::func_div() {
	Datum d2 = g_lingo->pop();

	if (d2.val == 0)
		error("division by zero");

	Datum d1 = g_lingo->pop();

	d1.val /= d2.val;
	g_lingo->push(d1);
}

void Lingo::func_negate() {
	Datum d = g_lingo->pop();

	d.val -= d.val;
	g_lingo->push(d);
}

void Lingo::func_eq() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.val = (d1.val == d2.val) ? 1 : 0;
	g_lingo->push(d1);
}

void Lingo::func_neq() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.val = (d1.val != d2.val) ? 1 : 0;
	g_lingo->push(d1);
}

void Lingo::func_gt() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.val = (d1.val > d2.val) ? 1 : 0;
	g_lingo->push(d1);
}

void Lingo::func_lt() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.val = (d1.val < d2.val) ? 1 : 0;
	g_lingo->push(d1);
}

void Lingo::func_ge() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.val = (d1.val >= d2.val) ? 1 : 0;
	g_lingo->push(d1);
}

void Lingo::func_le() {
	Datum d2 = g_lingo->pop();
	Datum d1 = g_lingo->pop();

	d1.val = (d1.val <= d2.val) ? 1 : 0;
	g_lingo->push(d1);
}


void Lingo::func_ifcode() {
	Datum d;
	int savepc = g_lingo->_pc;	/* then part */

	int then = READ_LE_UINT32(&(*g_lingo->_currentScript)[savepc]);
	int elsep = READ_LE_UINT32(&(*g_lingo->_currentScript)[savepc + 1]);
	int end = READ_LE_UINT32(&(*g_lingo->_currentScript)[savepc + 2]);

	warning("cond: %d end: %d  then: %d elesp: %d", savepc + 3, end, then, elsep);

	g_lingo->execute(savepc + 3);	/* condition */

	d = g_lingo->pop();
	warning("res: %d", d.val);

	if (d.val) {
		g_lingo->execute(then);
	} else if (elsep) { /* else part? */
		g_lingo->execute(elsep);
	}

	//if (!returning)
	g_lingo->_pc = end; /* next stmt */
}

//************************
// Built-in functions
//************************
void Lingo::func_mci() {
	Common::String s((char *)&(*g_lingo->_currentScript)[g_lingo->_pc]);

	g_lingo->exec_mci(s);

	g_lingo->_pc += g_lingo->calcStringAlignment(s.c_str());
}

void Lingo::func_mciwait() {
	Common::String s((char *)&(*g_lingo->_currentScript)[g_lingo->_pc]);

	g_lingo->exec_mciwait(s);

	g_lingo->_pc += g_lingo->calcStringAlignment(s.c_str());
}

void Lingo::func_goto() {
	Common::String frame((char *)&(*g_lingo->_currentScript)[g_lingo->_pc]);
	g_lingo->_pc += g_lingo->calcStringAlignment(frame.c_str());

	Common::String movie((char *)&(*g_lingo->_currentScript)[g_lingo->_pc]);
	g_lingo->_pc += g_lingo->calcStringAlignment(movie.c_str());

	g_lingo->exec_goto(frame, movie);
}

void Lingo::func_gotoloop() {
	warning("STUB: func_gotoloop()");
}

void Lingo::func_gotonext() {
	warning("STUB: func_gotonext()");
}

void Lingo::func_gotoprevious() {
	warning("STUB: func_gotoprevious()");
}

}
