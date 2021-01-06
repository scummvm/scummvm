#include "common/str.h"
#include "common/debug.h"

#include "grammar.h"
#include "grammar.tab.h"

namespace Private {

Setting *psetting;
SettingMap settingcode;

Datum	*stack       = NULL;	/* the stack */
Datum	*stackp      = NULL;	/* next free spot on stack */

Inst	*prog        = NULL;	/* the machine */
Inst	*progp       = NULL;	/* next free spot for code generation */
Inst	*pc          = NULL;	/* program counter during execution */

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
	        //debug("%d", arg.val);
		args.insert(args.begin(), arg) ;
        }

        execFunction(s.u.str, args);
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

int assign()	/* assign top value to next value */
{
	Datum d1, d2;
	d1 = pop();
	d2 = pop();
	/*if (d1.sym->type != VAR && d1.sym->type != UNDEF)
		execerror("assignment to non-variable",
			d1.sym->name);*/
	d1.u.sym->u.val = d2.u.val;
	d1.u.sym->type = NAME;
	push(d2);
	return 0;
}

int gt()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.u.val = (int)(d1.u.val > d2.u.val);
	push(d1);
	return 0;
}

int lt()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.u.val = (int)(d1.u.val < d2.u.val);
	push(d1);
	return 0;
}

int ge()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.u.val = (int)(d1.u.val >= d2.u.val);
	push(d1);
	return 0;
}

int le()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.u.val = (int)(d1.u.val <= d2.u.val);
	push(d1);
	return 0;
}

int eq()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.u.val = (int)(d1.u.val == d2.u.val);
	push(d1);
	return 0;
}

int ne()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.u.val = (int)(d1.u.val  !=  d2.u.val);
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
	else if (*((Inst **)(savepc+1))) /* else part? */ {
		debug("ifcode: false branch");
		execute(*((Inst **)(savepc+1)));
	}
	debug("ifcode finished");
	pc = *((Inst **)(savepc+2));	 /* next stmt */
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
