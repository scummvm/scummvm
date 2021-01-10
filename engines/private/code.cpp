#include "common/str.h"
#include "common/debug.h"
#include "common/hash-ptr.h"

#include "grammar.h"
#include "grammar.tab.h"
#include "private.h"

namespace Private {

Setting *psetting;
SettingMap settingcode;

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


void initSetting()	/* initialize for code generation */
{
    psetting = (Setting*) malloc(sizeof(Setting));
    memset((void *) psetting, 0, sizeof(Setting));

    prog = (Inst *) &psetting->prog;
    stack = (Datum *) &psetting->stack;

    stackp = stack;
    progp = prog;
}

void saveSetting(char *name)
{
    Common::String s(name);
    settingcode.setVal(s, psetting);
    //debug("setting %s", name);
}

void loadSetting(Common::String *name)
{
    assert(settingcode.contains(*name));
    psetting = settingcode.getVal(*name);

    debug("loading setting %s", name->c_str());

    prog = (Inst *) &psetting->prog;
    stack = (Datum *) &psetting->stack;

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


int push(Datum d)		/* push d onto stack */
{
    assert (!(stackp >= &stack[NSTACK]));
    *stackp++ = d;
    return 0;
}

Datum pop()	/* pop and return top elem from stack */
{
    assert (!(stackp <= stack));
    return *--stackp;
}

int constpush()	/* push constant onto stack */
{
    Datum d;
    Symbol *s = (Symbol *)*pc++;
    d.type = NUM;
    d.u.val = s->u.val;

    debug("pushing const %d with name %s", d.u.val, s->name->c_str());
    push(d);
    return 0;
}

int strpush()	/* push constant onto stack */
{
    Datum d;
    d.type = STRING;
    Symbol *s = (Symbol *)*pc++;
    d.u.str = s->u.str;
    debug("pushing const %s with name %s", d.u.str, s->name->c_str());
    push(d);
    return 0;
}


int varpush()	/* push variable onto stack */
{
    Datum d;
    d.type = NAME;
    d.u.sym = (Symbol *)(*pc++);
    debug("var pushing %s", d.u.sym->name->c_str());
    push(d);
    return 0;
}

int funcpush() //(char *name, int nargs)
{
    Datum s, n, arg;
    s = pop();
    n = pop();
    ArgArray args;

    debug("executing %s with %d params", s.u.str, n.u.val);
    for (int i = 0; i < n.u.val; i++) {
        arg = pop();
        args.insert(args.begin(), arg) ;
    }

    call(s.u.str, args);
    return 0;
}



int eval()		/* evaluate variable on stack */
{
    Datum d;
    d = pop();
    debug("eval %s", d.u.sym->name->c_str());
    //if (d.sym->type == UNDEF)
    //	execerror("undefined variable", d.sym->name);
    if (d.u.sym->type == NUM) {
        d.type = NUM;
        d.u.val = d.u.sym->u.val;
    } else if (d.u.sym->type == STRING) {
        d.type = STRING;
        d.u.str = d.u.sym->u.str;
    } else if (d.u.sym->type == RECT) {
        d.type = RECT;
        d.u.rect = d.u.sym->u.rect;
    } else if (d.u.sym->type == NAME) {
        //debug("NAME %s", d.sym->name->c_str());
        //d.sym = d.sym;
    }
    else
        assert(0);


    push(d);
    return 0;
}

int add()		/* add top two elems on stack */
{
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

    assert(d1.type == NUM);
    assert(d2.type == NUM);

    //printf("adding %d %d\n",d1.val, d2.val);
    d1.u.val += d2.u.val;
    push(d1);
    return 0;
}

int negate()
{
    Datum d;
    d = pop();
    int v;
    if (d.type == NAME) {
        debug("negating %s", d.u.sym->name->c_str());
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

int gt()
{
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

int lt()
{
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

int ge()
{
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

int le()
{
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

int eq()
{
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

int ne()
{
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


    d1.u.val = (int)(d1.u.val != d2.u.val);
    push(d1);
    return 0;
}

Inst *code(Inst f)	/* install one instruction or operand */
{
    //debug("pushing code at %d", progp);
    Inst *oprogp = progp;
    assert (!(progp >= &prog[NPROG]));
    *progp++ = f;
    return oprogp;
}

int ifcode()
{
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
    //debug("ptr: %x", *((Inst **)(savepc+1)));
    //debug("ptr: %x", *((Inst **)(savepc+2)));
    //debug("ptr: %x", *((Inst **)(savepc+3)));

    //assert(0);
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

int randbool()
{
    Datum d;
    d = pop();
    
    int v = g_private->getRandomBool(d.u.val);
     
    d.u.val = v;
    push(d);
    return 0;
}

int fail()
{
    assert(0);
    return 0;
}

void execute(Inst *p)	/* run the machine */
{
    for (pc = p; *pc != STOP; ) {
        (*(*pc++))();
    }
}

}
