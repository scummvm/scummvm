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
		warning("Void");
		break;
	case INT:
		warning("%d", d.u.i);
		break;
	case FLOAT:
		warning("%f", d.u.f);
		break;
	case VAR:
		if (!d.u.sym) {
			warning("Inconsistent stack: var, val: %d", d.u.i);
		} else {
			if (d.u.sym->name)
				warning("var: %s", d.u.sym->name);
			else
				warning("Nameless var. val: %d", d.u.sym->u.val);
		}
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

void Lingo::c_fconstpush() {
	Datum d;
	inst i = (*g_lingo->_currentScript)[g_lingo->_pc];
	d.u.f = *((float *)&i);
	d.type = FLOAT;

	g_lingo->_pc += g_lingo->calcCodeAlignment(sizeof(float));

	g_lingo->push(d);
}

void Lingo::c_varpush() {
	char *name = (char *)&(*g_lingo->_currentScript)[g_lingo->_pc];
	Datum d;

	d.u.sym = g_lingo->lookupVar(name);
	if (d.u.sym->type == CASTREF) {
		d.type = INT;
		int val = d.u.sym->u.val;

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

	if (d1.u.sym->type != INT && d1.u.sym->type != VOID) {
		warning("assignment to non-variable '%s'", d1.u.sym->name);
		return;
	}

	d1.u.sym->u.val = d2.u.i;
	d1.u.sym->type  = d2.type;

	g_lingo->push(d2);
}

bool Lingo::verify(Symbol *s) {
	if (s->type != INT && s->type != VOID && s->type != FLOAT) {
		warning("attempt to evaluate non-variable '%s'", s->name);

		return false;
	}

	if (s->type == VOID)
		warning("Variable used before assigning a value '%s'", s->name);

	return true;
}

void Lingo::c_eval() {
	Datum d;
	d = g_lingo->pop();

	if (d.type != VAR) { // It could be cast ref
		g_lingo->push(d);
		return;
	}

	if (!g_lingo->verify(d.u.sym))
		return;

	d.type = d.u.sym->type;
	d.u.i  = d.u.sym->u.val;

	g_lingo->push(d);
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
	counter->u.val = d.u.i;
	counter->type = INT;

	while (true) {
		g_lingo->execute(body);	/* body */
		if (g_lingo->_returning)
			break;

		counter->u.val += inc;
		g_lingo->execute(finish);	/* condition */
		d = g_lingo->pop();
		d.toInt();

		if (counter->u.val == d.u.i + inc)
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
	g_lingo->func_gotoloop();
}

void Lingo::c_gotonext() {
	g_lingo->func_gotonext();
}

void Lingo::c_gotoprevious() {
	g_lingo->func_gotoprevious();
}

void Lingo::c_call() {
	Common::String name((char *)&(*g_lingo->_currentScript)[g_lingo->_pc]);
	g_lingo->_pc += g_lingo->calcStringAlignment(name.c_str());

	if (!g_lingo->_handlers.contains(name)) {
		error("Call to undefined handler '%s'", name.c_str());
	}

	Symbol *sym = g_lingo->_handlers[name];

	int nargs = READ_UINT32(&(*g_lingo->_currentScript)[g_lingo->_pc++]);

	for (int i = nargs; i < sym->nargs; i++) {
		Datum d;

		d.u.i = 0;
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

}
