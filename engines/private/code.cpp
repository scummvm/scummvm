#include "common/str.h"

#include "grammar.h"
#include "grammar.tab.h"

namespace Private {

Setting *psetting;
SettingMap settings;

Datum	*stack       = NULL;	/* the stack */
Datum	*stackp      = NULL;	/* next free spot on stack */

Inst	*prog        = NULL;	/* the machine */
Inst	*progp       = NULL;	/* next free spot for code generation */
Inst	*pc          = NULL;	/* program counter during execution */

void initsetting()	/* initialize for code generation */
{
	psetting = (Setting*) malloc(sizeof(Setting));
        memset((void *) psetting, 0, sizeof(Setting));

        prog = (Inst *) &psetting->prog;
        stack = (Datum *) &psetting->stack;

	stackp = stack;
	progp = prog;
}

void savesetting(char *name)
{
        //printf("saving setting %s\n", name);
	Common::String s(name);
	settings.setVal(s, psetting);
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
	d.val = ((Symbol *)*pc++)->u.val;
	//printf("pushing %d\n", d.val);
	push(d);
	return 0;
}

int strpush()	/* push constant onto stack */
{
	Datum d;
	d.str = ((Symbol *)*pc++)->u.str;
	//printf("pushing %s\n", d.str);
	push(d);
	return 0;
}


int varpush()	/* push variable onto stack */
{
	Datum d;
	d.sym = (Symbol *)(*pc++);
	//printf("var pushing %s", d.sym->name);
	push(d);
	return 0;
}


int eval()		/* evaluate variable on stack */
{
	Datum d;
	d = pop();
	//if (d.sym->type == UNDEF)
	//	execerror("undefined variable", d.sym->name);
	d.val = d.sym->u.val;
	push(d);
	return 0;
}

int add()		/* add top two elems on stack */
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	//printf("adding %d %d\n",d1.val, d2.val);
	d1.val += d2.val;
	push(d1);
	return 0;
}

int negate()
{
	Datum d;
	d = pop();
	d.val = !d.val;
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
	d1.sym->u.val = d2.val;
	d1.sym->type = NAME;
	push(d2);
	return 0;
}

int gt()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val = (int)(d1.val > d2.val);
	push(d1);
	return 0;
}

int lt()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val = (int)(d1.val < d2.val);
	push(d1);
	return 0;
}

int ge()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val = (int)(d1.val >= d2.val);
	push(d1);
	return 0;
}

int le()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val = (int)(d1.val <= d2.val);
	push(d1);
	return 0;
}

int eq()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val = (int)(d1.val == d2.val);
	push(d1);
	return 0;
}

int ne()
{
	Datum d1, d2;
	d2 = pop();
	d1 = pop();
	d1.val = (int)(d1.val  !=  d2.val);
	push(d1);
	return 0;
}

int print()		/* pop top value from stack, print it */
{
	Datum d;
	d = pop();
	//printf("\t%d\n", d.val);
	return 0;
}

Inst *code(Inst f)	/* install one instruction or operand */
{
 	Inst *oprogp = progp;
	assert (!(progp >= &prog[NPROG]));
	*progp++ = f;
	return oprogp;
}

void execute(Inst *p)	/* run the machine */
{
	for (pc = p; *pc != STOP; )
		(*(*pc++))();
}

}
