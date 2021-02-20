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

#include "common/str.h"
#include "common/debug.h"
#include "common/hash-ptr.h"

#include "private/grammar.h"
#include "private/tokens.h"
#include "private/private.h"

namespace Private {

Datum *stack  = NULL; /* the stack */
Datum *stackp = NULL; /* next free spot on stack */

Inst *prog    = NULL; /* the machine */
Inst *progp   = NULL; /* next free spot for code generation */
Inst *pc      = NULL; /* program counter during execution */

/* initialize setting for code generation */
void SettingMaps::init() {
    setting = (Setting *)malloc(sizeof(Setting));
    memset((void *)setting, 0, sizeof(Setting));

    prog = (Inst *)&setting->prog;
    stack = (Datum *)&setting->stack;

    stackp = stack;
    progp = prog;
}

void SettingMaps::save(char *name) {
    map.setVal(name, setting);
}

void SettingMaps::load(Common::String &name) {
    assert(map.contains(name));
    setting = map.getVal(name);

    debugC(1, kPrivateDebugCode, "loading setting %s", name.c_str());

    prog = (Inst *)&setting->prog;
    stack = (Datum *)&setting->stack;

    stackp = stack;
    progp = prog;
}

/* push d onto stack */
int push(Datum d) {
    assert (!(stackp >= &stack[NSTACK]));
    *stackp++ = d;
    return 0;
}

/* pop and return top elem from stack */
Datum pop() {
    assert (!(stackp <= stack));
    return *--stackp;
}

/* push constant onto stack */
int constpush() {
    Datum d;
    Symbol *s = (Symbol *)*pc++;
    d.type = NUM;
    d.u.val = s->u.val;

    debugC(1, kPrivateDebugCode, "pushing const %d with name %s", d.u.val, s->name->c_str());
    push(d);
    return 0;
}

int strpush() { /* push constant onto stack */
    Datum d;
    d.type = STRING;
    Symbol *s = (Symbol *)*pc++;
    d.u.str = s->u.str;
    debugC(1, kPrivateDebugCode, "pushing const %s with name %s", d.u.str, s->name->c_str());

    push(d);
    return 0;
}

int varpush() { /* push variable onto stack */
    Datum d;
    d.type = NAME;
    d.u.sym = (Symbol *)(*pc++);
    debugC(1, kPrivateDebugCode, "var pushing %s", d.u.sym->name->c_str());
    push(d);
    return 0;
}

int funcpush() {
    Datum s, n, arg;
    s = pop();
    n = pop();
    ArgArray args;

    debugC(1, kPrivateDebugCode, "executing %s with %d params", s.u.str, n.u.val);
    for (int i = 0; i < n.u.val; i++) {
        arg = pop();
        args.insert(args.begin(), arg) ;
    }

    call(s.u.str, args);
    return 0;
}

/* evaluate variable on stack */
int eval() {
    Datum d;
    d = pop();
    if (d.u.sym->type == NUM) {
        d.type = NUM;
        d.u.val = d.u.sym->u.val;
    } else if (d.u.sym->type == STRING) {
        d.type = STRING;
        d.u.str = d.u.sym->u.str;
        debugC(1, kPrivateDebugCode, "eval returned %s", d.u.str );
    } else if (d.u.sym->type == RECT) {
        d.type = RECT;
        d.u.rect = d.u.sym->u.rect;
    } else if (d.u.sym->type == NAME) {
        // No evaluation until is absolutely needed
    } else
        assert(0);

    push(d);
    return 0;
}

/* add top two elems on stack */
int add() {
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    if (d1.type == NAME) {
        d1.u.val = d1.u.sym->u.val;
        d1.type = NUM;
    }

    if (d2.type == NAME) {
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
    Datum d;
    d = pop();
    int v;
    if (d.type == NAME) {
        //debug("negating %s", d.u.sym->name->c_str());
        v = d.u.sym->u.val;
        d.type = NUM;
    } else if (d.type == NUM) {
        v = d.u.val;
    } else
        assert(0);

    d.u.val = !v;
    push(d);
    return 0;
}

int gt() {
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    if (d1.type == NAME) {
        //char *name =  d1.u.sym->name->c_str();
        //debug("eval %s to %d",
        d1.u.val = d1.u.sym->u.val;
        d1.type = NUM;
    }

    if (d2.type == NAME) {
        //char *name =  d1.u.sym->name->c_str();
        //debug("eval %s to %d",
        d2.u.val = d2.u.sym->u.val;
        d2.type = NUM;
    }

    d1.u.val = (int)(d1.u.val > d2.u.val);
    push(d1);
    return 0;
}

int lt() {
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    if (d1.type == NAME) {
        //char *name =  d1.u.sym->name->c_str();
        //debug("eval %s to %d",
        d1.u.val = d1.u.sym->u.val;
        d1.type = NUM;
    }

    if (d2.type == NAME) {
        //char *name =  d1.u.sym->name->c_str();
        //debug("eval %s to %d",
        d2.u.val = d2.u.sym->u.val;
        d2.type = NUM;
    }

    d1.u.val = (int)(d1.u.val < d2.u.val);
    push(d1);
    return 0;
}

int ge() {
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    if (d1.type == NAME) {
        //char *name =  d1.u.sym->name->c_str();
        //debug("eval %s to %d",
        d1.u.val = d1.u.sym->u.val;
        d1.type = NUM;
    }

    if (d2.type == NAME) {
        //char *name =  d1.u.sym->name->c_str();
        //debug("eval %s to %d",
        d2.u.val = d2.u.sym->u.val;
        d2.type = NUM;
    }

    d1.u.val = (int)(d1.u.val >= d2.u.val);
    push(d1);
    return 0;
}

int le() {
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    if (d1.type == NAME) {
        //char *name =  d1.u.sym->name->c_str();
        //debug("eval %s to %d",
        d1.u.val = d1.u.sym->u.val;
        d1.type = NUM;
    }

    if (d2.type == NAME) {
        //char *name =  d1.u.sym->name->c_str();
        //debug("eval %s to %d",
        d2.u.val = d2.u.sym->u.val;
        d2.type = NUM;
    }

    d1.u.val = (int)(d1.u.val <= d2.u.val);
    push(d1);
    return 0;
}

int eq() {
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    if (d1.type == NAME) {
        //char *name =  d1.u.sym->name->c_str();
        //debug("eval %s to %d",
        d1.u.val = d1.u.sym->u.val;
        d1.type = NUM;
    }

    if (d2.type == NAME) {
        //char *name =  d1.u.sym->name->c_str();
        //debug("eval %s to %d",
        d2.u.val = d2.u.sym->u.val;
        d2.type = NUM;
    }

    d1.u.val = (int)(d1.u.val == d2.u.val);
    push(d1);
    return 0;
}

int ne() {
    Datum d1, d2;
    d2 = pop();
    d1 = pop();
    if (d1.type == NAME) {
        d1.u.val = d1.u.sym->u.val;
        d1.type = NUM;
    }

    if (d2.type == NAME) {
        d2.u.val = d2.u.sym->u.val;
        d2.type = NUM;
    }

    d1.u.val = (int)(d1.u.val != d2.u.val);
    push(d1);
    return 0;
}

/* install one instruction or operand */
Inst *code(Inst f) {
    debugC(1, kPrivateDebugCode, "pushing code at %x", progp);
    Inst *oprogp = progp;
    assert (!(progp >= &prog[NPROG]));
    *progp++ = f;
    return oprogp;
}

int ifcode() {
    Datum d;
    Inst *savepc = pc;  /* then part */
    debugC(1, kPrivateDebugCode, "ifcode: evaluating condition");

    execute(savepc+3);  /* condition */
    d = pop();

    debugC(1, kPrivateDebugCode, "ifcode: selecting branch");

    if (d.type == NAME) {
        debugC(1, kPrivateDebugCode, "name %s", d.u.sym->name->c_str());
        d.u.val = d.u.sym->u.val;
    }

    if (d.u.val) {
        debugC(1, kPrivateDebugCode, "ifcode: true branch");
        execute(*((Inst **)(savepc)));
    } else if (*((Inst **)(savepc+1))) { /* else part? */
        debugC(1, kPrivateDebugCode, "ifcode: false branch");
        execute(*((Inst **)(savepc+1)));
    }
    debugC(1, kPrivateDebugCode, "ifcode finished");
    pc = *((Inst **)(savepc+2)); /* next stmt */
    return 0;
}

int randbool() {
    Datum d;
    d = pop();

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
    for (pc = p; *pc != STOP; ) {
        (*(*pc++))();
    }
}

} // End of namespace Private
