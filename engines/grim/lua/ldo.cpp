/*
** $Id$
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/


#include "engines/grim/lua/ldo.h"
#include "engines/grim/lua/lfunc.h"
#include "engines/grim/lua/lgc.h"
#include "engines/grim/lua/lmem.h"
#include "engines/grim/lua/lobject.h"
#include "engines/grim/lua/lopcodes.h"
#include "engines/grim/lua/lparser.h"
#include "engines/grim/lua/lstate.h"
#include "engines/grim/lua/ltask.h"
#include "engines/grim/lua/ltm.h"
#include "engines/grim/lua/lua.h"
#include "engines/grim/lua/luadebug.h"
#include "engines/grim/lua/lundump.h"
#include "engines/grim/lua/lvm.h"
#include "engines/grim/lua/lzio.h"

#include "common/file.h"

namespace Grim {

#ifndef STACK_LIMIT
#define STACK_LIMIT     6000
#endif

// Extra stack size to run a function: LUA_T_LINE(1), TM calls(2), ...
#define	EXTRA_STACK	5

/*
** Error messages
*/

void stderrorim() {
	fprintf(stderr, "lua error: %s\n", lua_getstring(lua_getparam(1)));
}

#define STACK_UNIT	256

// Initial size for CallInfo array
#define BASIC_CI_SIZE	8

void luaD_init() {
	ttype(&lua_state->errorim) = LUA_T_CPROTO;
	fvalue(&lua_state->errorim) = stderrorim;
}

void luaD_initthr() {
	lua_state->stack.stack = luaM_newvector(STACK_UNIT, TObject);
	lua_state->stack.top = lua_state->stack.stack;
	lua_state->stack.last = lua_state->stack.stack + (STACK_UNIT - 1);
	lua_state->base_ci = luaM_newvector(BASIC_CI_SIZE, struct CallInfo);
	memset(lua_state->base_ci, 0, sizeof(CallInfo) * BASIC_CI_SIZE);
	lua_state->base_ci_size = sizeof(CallInfo) * BASIC_CI_SIZE; 
	lua_state->ci = lua_state->base_ci;
	lua_state->ci->tf = NULL;
	lua_state->end_ci = lua_state->base_ci + BASIC_CI_SIZE;
}

void luaD_checkstack(int32 n) {
	struct Stack *S = &lua_state->stack;
	if (S->last-S->top <= n) {
		StkId top = S->top-S->stack;
		int32 stacksize = (S->last-S->stack) + 1 + STACK_UNIT + n;
		S->stack = luaM_reallocvector(S->stack, stacksize, TObject);
		S->last = S->stack + (stacksize - 1);
		S->top = S->stack + top;
		if (stacksize >= STACK_LIMIT) {  // stack overflow?
			if (lua_stackedfunction(100) == LUA_NOOBJECT)  // 100 funcs on stack?
				lua_error("Lua2C - C2Lua overflow"); // doesn't look like a rec. loop
			else
				lua_error("stack size overflow");
		}
	}
}

/*
** Adjust stack. Set top to the given value, pushing NILs if needed.
*/
void luaD_adjusttop(StkId newtop) {
	int32 diff = newtop-(lua_state->stack.top-lua_state->stack.stack);
	if (diff <= 0)
		lua_state->stack.top += diff;
	else {
		luaD_checkstack(diff);
		while (diff--)
			ttype(lua_state->stack.top++) = LUA_T_NIL;
	}
}

/*
** Open a hole below "nelems" from the lua_state->stack.top.
*/
void luaD_openstack(int32 nelems) {
	luaO_memup(lua_state->stack.top - nelems + 1, lua_state->stack.top - nelems, nelems * sizeof(TObject));
	incr_top;
}

void luaD_lineHook(int32 line) {
	struct C_Lua_Stack oldCLS = lua_state->Cstack;
	StkId old_top = lua_state->Cstack.lua2C = lua_state->Cstack.base = lua_state->stack.top-lua_state->stack.stack;
	lua_state->Cstack.num = 0;
	(*lua_linehook)(line);
	lua_state->stack.top = lua_state->stack.stack + old_top;
	lua_state->Cstack = oldCLS;
}

void luaD_callHook(StkId base, TProtoFunc *tf, int32 isreturn) {
	struct C_Lua_Stack oldCLS = lua_state->Cstack;
	StkId old_top = lua_state->Cstack.lua2C = lua_state->Cstack.base = lua_state->stack.top - lua_state->stack.stack;
	lua_state->Cstack.num = 0;
	if (isreturn)
		(*lua_callhook)(LUA_NOOBJECT, "(return)", 0);
	else {
		TObject *f = lua_state->stack.stack + base - 1;
		if (tf)
			(*lua_callhook)(Ref(f), tf->fileName->str, tf->lineDefined);
		else
			(*lua_callhook)(Ref(f), "(C)", -1);
	}
	lua_state->stack.top = lua_state->stack.stack + old_top;
	lua_state->Cstack = oldCLS;
}

/*
** Call a C function.
** Cstack.num is the number of arguments; Cstack.lua2C points to the
** first argument. Returns an index to the first result from C.
*/
static StkId callC(lua_CFunction f, StkId base) {
	struct C_Lua_Stack *CS = &lua_state->Cstack;
	struct C_Lua_Stack oldCLS = *CS;
	StkId firstResult;
	int32 numarg = (lua_state->stack.top - lua_state->stack.stack) - base;
	CS->num = numarg;
	CS->lua2C = base;
	CS->base = base + numarg;  // == top - stack
	if (lua_callhook) {
		TObject *f = lua_state->stack.stack + base - 1;
		(*lua_callhook)(Ref(f), "(C)", -1);
	}
	(*f)();  // do the actual call
//	if (lua_callhook)  // func may have changed lua_callhook
//		(*lua_callhook)(LUA_NOOBJECT, "(return)", 0);
	firstResult = CS->base;
	*CS = oldCLS;
	return firstResult;
}

static StkId callCclosure(struct Closure *cl, lua_CFunction f, StkId base) {
	TObject *pbase;
	int32 nup = cl->nelems;  // number of upvalues
	luaD_checkstack(nup);
	pbase = lua_state->stack.stack + base;  // care: previous call may change this
	// open space for upvalues as extra arguments
	luaO_memup(pbase+nup, pbase, (lua_state->stack.top - pbase) * sizeof(TObject));
	// copy upvalues into stack
	memcpy(pbase, cl->consts + 1, nup * sizeof(TObject));
	lua_state->stack.top += nup;
	return callC(f, base);
}

void luaD_callTM(TObject *f, int32 nParams, int32 nResults) {
	luaD_openstack(nParams);
	*(lua_state->stack.top - nParams - 1) = *f;
	luaD_call((lua_state->stack.top - lua_state->stack.stack) - nParams, nResults);
}

static void adjust_varargs(StkId first_extra_arg) {
	TObject arg;
	luaV_pack(first_extra_arg, (lua_state->stack.top - lua_state->stack.stack) - first_extra_arg, &arg);
	luaD_adjusttop(first_extra_arg);
	*lua_state->stack.top++ = arg;
}

/*
** Prepare the stack for calling a Lua function.
*/
void luaD_precall(TObject *f, StkId base, int32 nResults) {
	// Create a new CallInfo record
	if (lua_state->ci + 1 == lua_state->end_ci) {
		int32 size_ci = lua_state->end_ci - lua_state->base_ci;
		int32 index_ci = lua_state->ci - lua_state->base_ci;
		int32 new_ci_size = size_ci * 2 * sizeof(CallInfo);
		CallInfo *new_ci = (CallInfo *)luaM_malloc(new_ci_size);
		memcpy(new_ci, lua_state->base_ci, lua_state->base_ci_size);
		memset(new_ci + (lua_state->base_ci_size / sizeof(CallInfo)), 0, (new_ci_size) - lua_state->base_ci_size);
		luaM_free(lua_state->base_ci);
		lua_state->base_ci = new_ci;
		lua_state->base_ci_size = new_ci_size;
		lua_state->ci = lua_state->base_ci + index_ci;
		lua_state->end_ci = lua_state->base_ci + size_ci * 2;
	}
	lua_state->ci++;
	if (ttype(f) == LUA_T_CLOSURE) {
		lua_state->ci->c = clvalue(f);
		f = &lua_state->ci->c->consts[0];  // Get the actual function
	} else {
		lua_state->ci->c = NULL;
	}
	lua_state->ci->base = base;
	lua_state->ci->nResults = nResults;
	if (ttype(f)==LUA_T_CPROTO) {
		lua_state->ci->tf = NULL;
		lua_state->ci->pc = NULL;
	} else {
		byte *pc = tfvalue(f)->code;
		if (lua_callhook) {
			TObject *f = lua_state->stack.stack + base - 1;
			(*lua_callhook)(Ref(f), tfvalue(f)->fileName->str, tfvalue(f)->lineDefined);
		}
		luaD_checkstack((*pc++) + EXTRA_STACK);
		if (*pc < ZEROVARARG) {
			luaD_adjusttop(base + *(pc++));
		} else {  // varargs
			luaC_checkGC();
			adjust_varargs(base + (*pc++) - ZEROVARARG);
		}
		lua_state->ci->tf = tfvalue(f);
		lua_state->ci->pc = pc;
	}
}

/*
** Adjust the stack to the desired number of results
*/
void luaD_postret(StkId firstResult) {
	int32 i;
	StkId base = lua_state->ci->base;
	int32 nResults = lua_state->ci->nResults;
	if (lua_state->ci == lua_state->base_ci)
		lua_error("call stack underflow");
	// adjust the number of results
	if (nResults != MULT_RET)
		luaD_adjusttop(firstResult + nResults);
	// move results to base - 1 (to erase parameters and function)
	base--;
	nResults = lua_state->stack.top - (lua_state->stack.stack + firstResult);  // actual number of results
	for (i = 0; i < nResults; i++)
		*(lua_state->stack.stack + base + i) = *(lua_state->stack.stack + firstResult + i);
	lua_state->stack.top -= firstResult - base;
	// pop off the current CallInfo
	lua_state->ci--;
}

/*
** Call a function (C or Lua). The parameters must be on the lua_state->stack.stack,
** between [lua_state->stack.stack+base,lua_state->stack.top). The function to be called is at lua_state->stack.stack+base-1.
** When returns, the results are on the lua_state->stack.stack, between [lua_state->stack.stack+base-1,lua_state->stack.top).
** The number of results is nResults, unless nResults=MULT_RET.
*/
void luaD_call(StkId base, int32 nResults) {
	StkId firstResult;
	TObject *func = lua_state->stack.stack + base - 1;
	switch (ttype(func)) {
	case LUA_T_CPROTO:
		luaD_precall(func, base, nResults);
		ttype(func) = LUA_T_CMARK;
		firstResult = callC(fvalue(func), base);
		break;
	case LUA_T_PROTO:
		luaD_precall(func, base, nResults);
		ttype(func) = LUA_T_PMARK;
		firstResult = luaV_execute(lua_state->ci);
		break;
	case LUA_T_CLOSURE:
		{
			Closure *c = clvalue(func);
			TObject *proto = &(c->consts[0]);
			luaD_precall(func, base, nResults);
			ttype(func) = LUA_T_CLMARK;
			firstResult = (ttype(proto) == LUA_T_CPROTO) ? callCclosure(c, fvalue(proto), base) : luaV_execute(lua_state->ci);
			break;
		}
	default:
		{ // func is not a function
			// Check the tag method for invalid functions
			TObject *im = luaT_getimbyObj(func, IM_FUNCTION);
			if (ttype(im) == LUA_T_NIL)
				lua_error("call expression not a function");
			luaD_callTM(im, (lua_state->stack.top - lua_state->stack.stack) - (base - 1), nResults);
			return;
		}
	}
	luaD_postret(firstResult);
}

static void travstack(struct Stack *S, int32 (*fn)(TObject *)) {
	StkId i;
	for (i = (S->top - 1) - S->stack; i >= 0; i--)
		fn(S->stack + i);
}

/*
** Traverse all objects on lua_state->stack.stack, and all other active stacks
*/
void luaD_travstack(int32(*fn)(TObject *)) {
	struct lua_Task *t;
	travstack(&lua_state->stack, fn);
	for (t = lua_state->root_task; t != NULL; t = t->next)
		if (t != lua_state->curr_task && t->Tstate != DONE)
			travstack(&t->stack, fn);
}

static void message(const char *s) {
	TObject im = lua_state->errorim;
	if (ttype(&im) != LUA_T_NIL) {
		lua_pushstring(s);
		luaD_callTM(&im, 1, 0);
	}
}

/*
** Reports an error, and jumps up to the available recover label
*/
void lua_error(const char *s) {
	if (s)
		message(s);
	if (lua_state->errorJmp) {
		longjmp(*((jmp_buf *)lua_state->errorJmp), 1);
	} else {
		fprintf(stderr, "lua: exit(1). Unable to recover\n");
		exit(1);
	}
}

/*
** Call the function at lua_state->Cstack.base, and incorporate results on
** the Lua2C structure.
*/
static void do_callinc(int32 nResults) {
	StkId base = lua_state->Cstack.base;
	luaD_call(base + 1, nResults);
	lua_state->Cstack.lua2C = base;  // position of the luaM_new results 
	lua_state->Cstack.num = (lua_state->stack.top - lua_state->stack.stack) - base;  // number of results
	lua_state->Cstack.base = base + lua_state->Cstack.num;  // incorporate results on lua_state->stack.stack/
}

/*
** Execute a protected call. Assumes that function is at lua_state->Cstack.base and
** parameters are on top of it. Leave nResults on the stack.
*/
int32 luaD_protectedrun(int32 nResults) {
	jmp_buf myErrorJmp;
	int32 status;
	struct C_Lua_Stack oldCLS = lua_state->Cstack;
	jmp_buf *oldErr = lua_state->errorJmp;
	int32 ci_len = lua_state->ci - lua_state->base_ci;
	lua_state->errorJmp = &myErrorJmp;
	if (setjmp(myErrorJmp) == 0) {
		do_callinc(nResults);
		status = 0;
	} else { // an error occurred: restore lua_state->Cstack and lua_state->stack.top
		lua_state->Cstack = oldCLS;
		lua_state->stack.top = lua_state->stack.stack + lua_state->Cstack.base;
		lua_state->ci = lua_state->base_ci + ci_len;
		status = 1;
	}
	lua_state->errorJmp = oldErr;
	return status;
}

/*
** returns 0 = chunk loaded; 1 = error; 2 = no more chunks to load
*/
static int32 protectedparser(ZIO *z, int32 bin) {
	int32 status;
	TProtoFunc *tf;
	jmp_buf myErrorJmp;
	jmp_buf *oldErr = lua_state->errorJmp;
	lua_state->errorJmp = &myErrorJmp;
	if (setjmp(myErrorJmp) == 0) {
		tf = bin ? luaU_undump1(z) : luaY_parser(z);
		status = 0;
	} else {
		tf = NULL;
		status = 1;
	}
	lua_state->errorJmp = oldErr;
	if (status)
		return 1;  // error code 
	if (tf == NULL)
		return 2;  // 'natural' end
	luaD_adjusttop(lua_state->Cstack.base + 1);  // one slot for the pseudo-function 
	lua_state->stack.stack[lua_state->Cstack.base].ttype = LUA_T_PROTO;
	lua_state->stack.stack[lua_state->Cstack.base].value.tf = tf;
	luaV_closure(0);
	return 0;
}

static int32 do_main(ZIO *z, int32 bin) {
	int32 status;
	do {
		int32 old_blocks = (luaC_checkGC(), lua_state->nblocks);
		status = protectedparser(z, bin);
		if (status == 1)
			return 1;  // error
		else if (status == 2)
			return 0;  // 'natural' end 
		else {
			int32 newelems2 = 2 * (lua_state->nblocks - old_blocks);
			lua_state->GCthreshold += newelems2;
			status = luaD_protectedrun(MULT_RET);
			lua_state->GCthreshold -= newelems2;
		}
	} while (bin && status == 0);
	return status;
}

void luaD_gcIM(TObject *o) {
	TObject *im = luaT_getimbyObj(o, IM_GC);
	if (ttype(im) != LUA_T_NIL) {
		*lua_state->stack.top = *o;
		incr_top;
		luaD_callTM(im, 1, 0);
	}
}

#define SIZE_PREF 20  // size of string prefix to appear in error messages
#define SSIZE_PREF "20"

static void build_name (const char *str, char *name) {
	if (str == NULL || *str == ID_CHUNK)
		strcpy(name, "(buffer)");
	else {
		char *temp;
		sprintf(name, "(dostring) >> \"%." SSIZE_PREF "s\"", str);
		temp = strchr(name, '\n');
		if (temp) {  // end string after first line
			*temp = '"';
			*(temp + 1) = 0;
		}
	}
}

int32 lua_dostring(const char *str) {
	return lua_dobuffer(str, strlen(str), NULL);
}

int32 lua_dobuffer(const char *buff, int32 size, const char *name) {
	char newname[SIZE_PREF + 25];
	ZIO z;
	int32 status;

	if (!name) {
		build_name(buff, newname);
		name = newname;
	}
	luaZ_mopen(&z, buff, size, name);
	status = do_main(&z, buff[0] == ID_CHUNK);
	return status;
}

} // end of namespace Grim
