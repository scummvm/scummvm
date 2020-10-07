/*
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/

#define FORBIDDEN_SYMBOL_EXCEPTION_setjmp
#define FORBIDDEN_SYMBOL_EXCEPTION_longjmp
#define FORBIDDEN_SYMBOL_EXCEPTION_fprintf
#define FORBIDDEN_SYMBOL_EXCEPTION_stderr
#define FORBIDDEN_SYMBOL_EXCEPTION_exit

#ifdef _MSC_VER
#pragma warning(disable:4611)
#endif

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
#include "common/textconsole.h"

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
	ttype(&errorim) = LUA_T_CPROTO;
	fvalue(&errorim) = stderrorim;
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
		TObject *r = lua_state->stack.stack + base - 1;
		(*lua_callhook)(Ref(r), "(C)", -1);
	}
	lua_state->state_counter2++;
	(*f)();  // do the actual call
	lua_state->state_counter2--;
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
	lua_state->state_counter1++;
	lua_state->state_counter2++;
	luaD_call((lua_state->stack.top - lua_state->stack.stack) - nParams, nResults);
	lua_state->state_counter2--;
	lua_state->state_counter1--;
}

int32 luaD_call(StkId base, int32 nResults) {
	lua_Task *tmpTask = lua_state->task;
	if (!lua_state->task || lua_state->state_counter2) {
		lua_Task *t = luaM_new(lua_Task);
		lua_taskinit(t, lua_state->task, base, nResults);
		lua_state->task = t;
	} else {
		tmpTask = lua_state->some_task;
	}

	while (1) {
		lua_CFunction function = nullptr;
		StkId firstResult = 0;
		TObject *funcObj = lua_state->stack.stack + base - 1;
		if (ttype(funcObj) == LUA_T_CLOSURE) {
			Closure *c = clvalue(funcObj);
			TObject *proto = &(c->consts[0]);
			ttype(funcObj) = LUA_T_CLMARK;
			if (ttype(proto) == LUA_T_CPROTO) {
				function = fvalue(funcObj);
				firstResult = callCclosure(c, fvalue(proto), base);
			} else {
				lua_taskresume(lua_state->task, c, tfvalue(proto), base);
				firstResult = luaV_execute(lua_state->task);
			}
		} else if (ttype(funcObj) == LUA_T_PMARK) {
			if (!lua_state->task->some_flag) {
				TObject *im = luaT_getimbyObj(funcObj, IM_FUNCTION);
				if (ttype(im) == LUA_T_NIL)
					lua_error("call expression not a function");
				luaD_callTM(im, (lua_state->stack.top - lua_state->stack.stack) - (base - 1), nResults);
				continue;
			}
			firstResult = luaV_execute(lua_state->task);
		} else if (ttype(funcObj) == LUA_T_CMARK) {
			if (!lua_state->task->some_flag) {
				TObject *im = luaT_getimbyObj(funcObj, IM_FUNCTION);
				if (ttype(im) == LUA_T_NIL)
					lua_error("call expression not a function");
				luaD_callTM(im, (lua_state->stack.top - lua_state->stack.stack) - (base - 1), nResults);
				continue;
			}
		} else if (ttype(funcObj) == LUA_T_CLMARK) {
			Closure *c = clvalue(funcObj);
			TObject *proto = &(c->consts[0]);
			if (!lua_state->task->some_flag) {
				TObject *im = luaT_getimbyObj(funcObj, IM_FUNCTION);
				if (ttype(im) == LUA_T_NIL)
					lua_error("call expression not a function");
				luaD_callTM(im, (lua_state->stack.top - lua_state->stack.stack) - (base - 1), nResults);
				continue;
			}
			if (ttype(proto) != LUA_T_CPROTO)
				firstResult = luaV_execute(lua_state->task);
		} else if (ttype(funcObj) == LUA_T_PROTO) {
			ttype(funcObj) = LUA_T_PMARK;
			lua_taskresume(lua_state->task, nullptr, tfvalue(funcObj), base);
			firstResult = luaV_execute(lua_state->task);
		} else if (ttype(funcObj) == LUA_T_CPROTO) {
			ttype(funcObj) = LUA_T_CMARK;
			function = fvalue(funcObj);
			firstResult = callC(fvalue(funcObj), base);
		} else {
			TObject *im = luaT_getimbyObj(funcObj, IM_FUNCTION);
			if (ttype(im) == LUA_T_NIL) {
				// NOTE: Originally this throwed the lua_error. Anyway it is commented here because
				// when in year 4 bi.exit() calls bi.book.act:free(). But bi.book.act is nil,
				// hence it enters this branch and the error blocks the game.
				// Now we try instead to survive and go on with the function.
				lua_Task *t = lua_state->task;
				lua_state->task = t->next;
				lua_state->some_task = tmpTask;
				luaM_free(t);

				warning("Lua: call expression not a function");
				return 1;
// 				lua_error("call expression not a function");
			}
			luaD_callTM(im, (lua_state->stack.top - lua_state->stack.stack) - (base - 1), nResults);
			continue;
		}

		if (firstResult <= 0) {
			nResults = lua_state->task->aux;
			base = -firstResult;
			lua_Task *t = luaM_new(lua_Task);
			lua_taskinit(t, lua_state->task, base, nResults);
			lua_state->task = t;
		} else {
			nResults = lua_state->task->some_results;
			base = lua_state->task->some_base;
			if (nResults != 255)
				luaD_adjusttop(firstResult + nResults);
			base--;
			nResults = lua_state->stack.top - (lua_state->stack.stack + firstResult);
			for (int32 i = 0; i < nResults; i++)
				*(lua_state->stack.stack + base + i) = *(lua_state->stack.stack + firstResult + i);
			lua_state->stack.top -= firstResult - base;

			lua_Task *tmp = lua_state->task;
			lua_state->task = lua_state->task->next;
			luaM_free(tmp);
			if (lua_state->task) {
				nResults = lua_state->task->some_results;
				base = lua_state->task->some_base;
			}

			if (function == break_here || function == sleep_for) {
				if (!lua_state->state_counter1)  {
					lua_state->some_task = tmpTask;
					return 1;
				}
			}
		}

		if (lua_state->task == tmpTask)
			break;
	}

	return 0;
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
	LState *t;
	for (t = lua_rootState; t != nullptr; t = t->next) {
		travstack(&t->stack, fn);
	}
}

static void message(const char *s) {
	TObject im = errorim;
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
	lua_state->errorJmp = &myErrorJmp;
	lua_state->state_counter1++;
	lua_Task *tmpTask = lua_state->task;
	if (setjmp(myErrorJmp) == 0) {
		do_callinc(nResults);
		status = 0;
	} else { // an error occurred: restore lua_state->Cstack and lua_state->stack.top
		lua_state->Cstack = oldCLS;
		lua_state->stack.top = lua_state->stack.stack + lua_state->Cstack.base;
		while (tmpTask != lua_state->task) {
			lua_Task *t = lua_state->task;
			lua_state->task = lua_state->task->next;
			luaM_free(t);
		}
		status = 1;
	}
	lua_state->state_counter1--;
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
		tf = nullptr;
		status = 1;
	}
	lua_state->errorJmp = oldErr;
	if (status)
		return 1;  // error code
	if (tf == nullptr)
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
		int32 old_blocks = (luaC_checkGC(), nblocks);
		status = protectedparser(z, bin);
		if (status == 1)
			return 1;  // error
		else if (status == 2)
			return 0;  // 'natural' end
		else {
			int32 newelems2 = 2 * (nblocks - old_blocks);
			GCthreshold += newelems2;
			status = luaD_protectedrun(MULT_RET);
			GCthreshold -= newelems2;
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
	if (str == nullptr || *str == ID_CHUNK)
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
	return lua_dobuffer(str, strlen(str), nullptr);
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
