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

#include "common/debug.h"
#include "common/hash-ptr.h"
#include "common/str.h"

#include "private/grammar.h"
#include "private/private.h"
#include "private/tokens.h"

namespace Private {

Gen::VM *Gen::g_vm;

void Gen::VM::run() {
	Gen::execute(Gen::g_vm->_prog);
}

namespace Settings {

using namespace Gen;

SettingMaps *g_setts;

/* initialize setting for code generation */
void SettingMaps::init() {
	_setting = (Setting *)malloc(sizeof(Setting));
	memset((void *)_setting, 0, sizeof(Setting));

	g_vm->_prog = (Inst *)&_setting->prog;
	g_vm->_stack = (Datum *)&_setting->stack;

	g_vm->_progp = Gen::g_vm->_prog;
	g_vm->_stackp = Gen::g_vm->_stack;
}

void SettingMaps::save(const char *name) {
	_map.setVal(name, _setting);
}

void SettingMaps::load(const Common::String &name) {
	assert(_map.contains(name));
	_setting = _map.getVal(name);

	debugC(1, kPrivateDebugCode, "loading setting %s", name.c_str());

	g_vm->_prog = (Inst *)&_setting->prog;
	g_vm->_stack = (Datum *)&_setting->stack;

	g_vm->_stackp = g_vm->_stack;
	g_vm->_progp = g_vm->_prog;
}

} // end of namespace Settings

namespace Gen {

/* pop and return top elem from stack */
Datum pop() {
	assert(!(g_vm->_stackp <= g_vm->_stack));
	return *--g_vm->_stackp;
}

/* push d onto stack */
int push(const Datum &d) {
	assert(!(g_vm->_stackp >= &g_vm->_stack[NSTACK]));
	*g_vm->_stackp++ = d;
	return 0;
}

/* push constant onto stack */
int constpush() {
	Datum d;
	Symbol *s = (Symbol *)*g_vm->_pc++;
	d.type = NUM;
	d.u.val = s->u.val;

	debugC(1, kPrivateDebugCode, "pushing const %d with name %s", d.u.val, s->name->c_str());
	push(d);
	return 0;
}

int strpush() { /* push constant onto stack */
	Datum d;
	d.type = STRING;
	Symbol *s = (Symbol *)*g_vm->_pc++;
	d.u.str = s->u.str;
	debugC(1, kPrivateDebugCode, "pushing const %s with name %s", d.u.str, s->name->c_str());

	push(d);
	return 0;
}

int varpush() { /* push variable onto stack */
	Datum d;
	d.type = NAME;
	d.u.sym = (Symbol *)(*g_vm->_pc++);
	debugC(1, kPrivateDebugCode, "var pushing %s", d.u.sym->name->c_str());
	push(d);
	return 0;
}

int funcpush() {
	Datum s = pop();
	Datum n = pop();
	ArgArray args;

	debugC(1, kPrivateDebugCode, "executing %s with %d params", s.u.str, n.u.val);
	for (int i = 0; i < n.u.val; i++) {
		Datum arg = pop();
		args.insert(args.begin(), arg);
	}

	call(s.u.str, args);
	return 0;
}

/* evaluate variable on stack */
int eval() {
	Datum d = pop();
	if (d.u.sym->type == NUM) {
		d.type = NUM;
		d.u.val = d.u.sym->u.val;
		debugC(1, kPrivateDebugCode, "eval NUM returned %d", d.u.val);
	} else if (d.u.sym->type == STRING) {
		d.type = STRING;
		d.u.str = d.u.sym->u.str;
		debugC(1, kPrivateDebugCode, "eval STR returned %s", d.u.str);
	} else if (d.u.sym->type == RECT) {
		d.type = RECT;
		d.u.rect = d.u.sym->u.rect;
		debugC(1, kPrivateDebugCode, "eval RECT");
	} else if (d.u.sym->type == NAME) {
		debugC(1, kPrivateDebugCode, "eval NAME is noop");
		// No evaluation until is absolutely needed
	} else
		assert(0);

	push(d);
	return 0;
}

/* add top two elems on stack */
int add() {
	Datum d2 = pop();
	Datum d1 = pop();
	if (d1.type == NAME) {
		d1.u.sym = g_private->maps.lookupVariable(d1.u.sym->name);
		d1.u.val = d1.u.sym->u.val;
		d1.type = NUM;
	}

	if (d2.type == NAME) {
		d2.u.sym = g_private->maps.lookupVariable(d2.u.sym->name);
		d2.u.val = d2.u.sym->u.val;
		d2.type = NUM;
	}

	assert(d1.type == NUM);
	assert(d2.type == NUM);

	debugC(1, kPrivateDebugCode, "adding %d %d\n", d1.u.val, d2.u.val);
	d1.u.val += d2.u.val;
	push(d1);
	return 0;
}

int negate() {
	Datum d = pop();
	int v = 0;
	if (d.type == NAME) {
		d.u.sym = g_private->maps.lookupVariable(d.u.sym->name);
		v = d.u.sym->u.val;
		d.type = NUM;
	} else if (d.type == NUM) {
		v = d.u.val;
	} else
		assert(0);

	debugC(1, kPrivateDebugCode, "negating %d\n", d.u.val);
	d.u.val = !v;
	push(d);
	return 0;
}

int gt() {
	Datum d2 = pop();
	Datum d1 = pop();
	if (d1.type == NAME) {
		//char *name =  d1.u.sym->name->c_str();
		//debug("eval %s to %d",
		d1.u.sym = g_private->maps.lookupVariable(d1.u.sym->name);
		d1.u.val = d1.u.sym->u.val;
		d1.type = NUM;
	}

	if (d2.type == NAME) {
		//char *name =  d1.u.sym->name->c_str();
		//debug("eval %s to %d",
		d2.u.sym = g_private->maps.lookupVariable(d2.u.sym->name);
		d2.u.val = d2.u.sym->u.val;
		d2.type = NUM;
	}

	d1.u.val = (int)(d1.u.val > d2.u.val);
	push(d1);
	return 0;
}

int lt() {
	Datum d2 = pop();
	Datum d1 = pop();
	if (d1.type == NAME) {
		//char *name =  d1.u.sym->name->c_str();
		//debug("eval %s to %d",
		d1.u.sym = g_private->maps.lookupVariable(d1.u.sym->name);
		d1.u.val = d1.u.sym->u.val;
		d1.type = NUM;
	}

	if (d2.type == NAME) {
		//char *name =  d1.u.sym->name->c_str();
		//debug("eval %s to %d",
		d2.u.sym = g_private->maps.lookupVariable(d2.u.sym->name);
		d2.u.val = d2.u.sym->u.val;
		d2.type = NUM;
	}

	d1.u.val = (int)(d1.u.val < d2.u.val);
	push(d1);
	return 0;
}

int ge() {
	Datum d2 = pop();
	Datum d1 = pop();
	if (d1.type == NAME) {
		//char *name =  d1.u.sym->name->c_str();
		//debug("eval %s to %d",
		d1.u.sym = g_private->maps.lookupVariable(d1.u.sym->name);
		d1.u.val = d1.u.sym->u.val;
		d1.type = NUM;
	}

	if (d2.type == NAME) {
		//char *name =  d1.u.sym->name->c_str();
		//debug("eval %s to %d",
		d2.u.sym = g_private->maps.lookupVariable(d2.u.sym->name);
		d2.u.val = d2.u.sym->u.val;
		d2.type = NUM;
	}

	d1.u.val = (int)(d1.u.val >= d2.u.val);
	push(d1);
	return 0;
}

int le() {
	Datum d2 = pop();
	Datum d1 = pop();
	if (d1.type == NAME) {
		//char *name =  d1.u.sym->name->c_str();
		//debug("eval %s to %d",
		d1.u.sym = g_private->maps.lookupVariable(d1.u.sym->name);
		d1.u.val = d1.u.sym->u.val;
		d1.type = NUM;
	}

	if (d2.type == NAME) {
		//char *name =  d1.u.sym->name->c_str();
		//debug("eval %s to %d",
		d2.u.sym = g_private->maps.lookupVariable(d2.u.sym->name);
		d2.u.val = d2.u.sym->u.val;
		d2.type = NUM;
	}

	d1.u.val = (int)(d1.u.val <= d2.u.val);
	push(d1);
	return 0;
}

int eq() {
	Datum d2 = pop();
	Datum d1 = pop();
	if (d1.type == NAME) {
		//char *name =  d1.u.sym->name->c_str();
		//debug("eval %s to %d",
		d1.u.sym = g_private->maps.lookupVariable(d1.u.sym->name);
		d1.u.val = d1.u.sym->u.val;
		d1.type = NUM;
	}

	if (d2.type == NAME) {
		//char *name =  d1.u.sym->name->c_str();
		//debug("eval %s to %d",
		d2.u.sym = g_private->maps.lookupVariable(d2.u.sym->name);
		d2.u.val = d2.u.sym->u.val;
		d2.type = NUM;
	}

	d1.u.val = (int)(d1.u.val == d2.u.val);
	push(d1);
	return 0;
}

int ne() {
	Datum d2 = pop();
	Datum d1 = pop();
	if (d1.type == NAME) {
		d1.u.sym = g_private->maps.lookupVariable(d1.u.sym->name);
		d1.u.val = d1.u.sym->u.val;
		d1.type = NUM;
	}

	if (d2.type == NAME) {
		d2.u.sym = g_private->maps.lookupVariable(d2.u.sym->name);
		d2.u.val = d2.u.sym->u.val;
		d2.type = NUM;
	}

	d1.u.val = (int)(d1.u.val != d2.u.val);
	push(d1);
	return 0;
}

/* install one instruction or operand */
Inst *code(const Inst &f) {
	//debugC(1, kPrivateDebugCode, "pushing code at %x", progp);
	Inst *oprogp = g_vm->_progp;
	assert(!(g_vm->_progp >= &g_vm->_prog[NPROG]));
	*g_vm->_progp++ = f;
	return oprogp;
}

int ifcode() {
	Inst *savepc = g_vm->_pc; /* then part */
	debugC(1, kPrivateDebugCode, "ifcode: evaluating condition");

	execute(savepc + 3); /* condition */
	Datum d = pop();

	debugC(1, kPrivateDebugCode, "ifcode: selecting branch");

	if (d.type == NAME) {
		debugC(1, kPrivateDebugCode, "name %s", d.u.sym->name->c_str());
		d.u.sym = g_private->maps.lookupVariable(d.u.sym->name);
		d.u.val = d.u.sym->u.val;
	}

	if (d.u.val) {
		debugC(1, kPrivateDebugCode, "ifcode: true branch");
		execute(*((Inst **)(savepc)));
	} else if (*((Inst **)(savepc + 1))) { /* else part? */
		debugC(1, kPrivateDebugCode, "ifcode: false branch");
		execute(*((Inst **)(savepc + 1)));
	}
	debugC(1, kPrivateDebugCode, "ifcode finished");
	g_vm->_pc = *((Inst **)(savepc + 2)); /* next stmt */
	return 0;
}

int randbool() {
	Datum d = pop();

	int v = g_private->getRandomBool(d.u.val);

	d.u.val = v;
	push(d);
	return 0;
}

int fail() {
	assert(0);
	return 0;
}

/* run the machine */
void execute(Inst *p) {
	for (g_vm->_pc = p; *(g_vm->_pc) != STOP;) {
		(*(*(g_vm->_pc++)))();
	}
}

} // End of namespace Gen

} // End of namespace Private
