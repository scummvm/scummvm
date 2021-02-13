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

#include "common/str.h"
#include "common/debug.h"
#include "common/hash-ptr.h"

#include "private/grammar.h"
#include "private/grammar.tab.h"
#include "private/private.h"

namespace Private {

Setting *setting;
SettingMap settingMap;

Datum	*stack       = NULL;	/* the stack */
Datum	*stackp      = NULL;	/* next free spot on stack */

Inst	*prog        = NULL;	/* the machine */
Inst	*progp       = NULL;	/* next free spot for code generation */
Inst	*pc          = NULL;	/* program counter during execution */


static struct InstDescr {
    const Inst func;
    const char *name;
} instDescr[] = {
    { 0,        "STOP",     },
    { constpush,"constpush" },
    { strpush,  "strpush",	},
    { varpush,  "varpush",	},
    { funcpush, "funcpush",	},
    { eval,	    "eval",     },
    { ifcode,   "ifcode",	},
    { add,      "add",      },
    { negate,	"negate",	},

    { 0, 0}
};

PtrToName _insts;

void initInsts() {
    for (InstDescr *fnc = instDescr; fnc->name; fnc++) {
        _insts[(void *)fnc->func] = new Common::String(fnc->name);
    }
}

/* initialize for code generation */
void initSetting() {
    setting = (Setting*)malloc(sizeof(Setting));
    memset((void *)setting, 0, sizeof(Setting));

    prog = (Inst *)&setting->prog;
    stack = (Datum *)&setting->stack;

    stackp = stack;
    progp = prog;
}

void saveSetting(char *name) {
    //Common::String s(name);
    settingMap.setVal(name, setting);
}

void loadSetting(Common::String *name) {
    assert(settingMap.contains(*name));
    setting = settingMap.getVal(*name);

    debug("loading setting %s", name->c_str());

    prog = (Inst *)&setting->prog;
    stack = (Datum *)&setting->stack;

    stackp = stack;
    progp = prog;

    /*for (Inst *pc_ = progp; pc_-progp < 100; pc_++) {
        if (_functions.contains((void *) *pc_))
            debug("%p: %s", (void*) pc_, _functions.getVal((void*) *pc_)->c_str());
        else if ( (Inst *) *pc_ >= progp && (Inst *) *pc_ <= (progp + NPROG))
            debug("%p: %p", (void*) pc_, (void*) *pc_);
        else {
            debugN("%p:", (void*) pc_);
            showSymbol((Symbol *) *pc_);
        }
    }*/
}

/* push d onto stack */
int push(Datum d) {
    assert (!(stackp >= &stack[NSTACK]));
    *stackp++ = d;
    return 0;
}

/* pop and return top elem from stack */
Datum pop()	{
    assert (!(stackp <= stack));
    return *--stackp;
}

/* push constant onto stack */
int constpush()	{
    Datum d;
    Symbol *s = (Symbol *)*pc++;
    d.type = NUM;
    d.u.val = s->u.val;

    debug("pushing const %d with name %s", d.u.val, s->name->c_str());
    push(d);
    return 0;
}

int strpush() { /* push constant onto stack */
    Datum d;
    d.type = STRING;
    Symbol *s = (Symbol *)*pc++;
    d.u.str = s->u.str;
    debug("pushing const %s with name %s", d.u.str, s->name->c_str());
    push(d);
    return 0;
}


int varpush() {	/* push variable onto stack */
    Datum d;
    d.type = NAME;
    d.u.sym = (Symbol *)(*pc++);
    debug("var pushing %s", d.u.sym->name->c_str());
    push(d);
    return 0;
}

int funcpush() {
    Datum s, n, arg;
    s = pop();
    n = pop();
    ArgArray args;

    //debug("executing %s with %d params", s.u.str, n.u.val);
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
    //debug("eval %s", d.u.sym->name->c_str());
    //if (d.sym->type == UNDEF)
    //	execerror("undefined variable", d.sym->name);
    if (d.u.sym->type == NUM) {
        d.type = NUM;
        d.u.val = d.u.sym->u.val;
    } else if (d.u.sym->type == STRING) {
        d.type = STRING;
        d.u.str = d.u.sym->u.str;
        debug("eval returned %s", d.u.str );
    } else if (d.u.sym->type == RECT) {
        d.type = RECT;
        d.u.rect = d.u.sym->u.rect;
    } else if (d.u.sym->type == NAME) {
        // No evaluation until is absolutely needed
    }
    else
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

    //printf("adding %d %d\n",d1.val, d2.val);
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
    }
    else if (d.type == NUM) {
        v = d.u.val;
    }
    else
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
    //debug("pushing code at %d", progp);
    Inst *oprogp = progp;
    assert (!(progp >= &prog[NPROG]));
    *progp++ = f;
    return oprogp;
}

int ifcode() {
    Datum d;
    Inst *savepc = pc;	/* then part */
    debug("ifcode: evaluating condition");

    execute(savepc+3);	/* condition */
    d = pop();

    debug("ifcode: selecting branch");

    if (d.type == NAME) {
        debug("name %s", d.u.sym->name->c_str()); //, d.sym->u.val);
        d.u.val = d.u.sym->u.val;
    }

    if (d.u.val) {
        debug("ifcode: true branch");
        execute(*((Inst **)(savepc)));
    }
    else if (*((Inst **)(savepc+1))) { /* else part? */
        debug("ifcode: false branch");
        execute(*((Inst **)(savepc+1)));
    }
    debug("ifcode finished");
    pc = *((Inst **)(savepc+2));	 /* next stmt */
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
