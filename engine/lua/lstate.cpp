/*
** $Id$
** Global State
** See Copyright Notice in lua.h
*/


#include "engine/lua/lbuiltin.h"
#include "engine/lua/ldo.h"
#include "engine/lua/lauxlib.h"
#include "engine/lua/lfunc.h"
#include "engine/lua/lgc.h"
#include "engine/lua/llex.h"
#include "engine/lua/lmem.h"
#include "engine/lua/lstate.h"
#include "engine/lua/lstring.h"
#include "engine/lua/ltable.h"
#include "engine/lua/ltask.h"
#include "engine/lua/ltm.h"
#include "engine/lua/lualib.h"

LState *lua_state = NULL;

int32 globalTaskSerialId;

void stderrorim();

static luaL_reg stdErrorRimFunc[] = {
	{ "stderrorim", stderrorim }
};

static void lua_openthr() {
	L->numCblocks = 0;
	L->Cstack.base = 0;
	L->Cstack.lua2C = 0;
	L->Cstack.num = 0;
	L->errorJmp = NULL;
	L->Mbuffsize = 0;
	L->Mbuffnext = 0;
	L->Mbuffbase = NULL;
	L->Mbuffer = NULL;
	L->Tstate = RUN;
}

void lua_resetglobals() {
	globalTaskSerialId = 1;
	lua_openthr();
	L->rootproto.next = NULL;
	L->rootproto.marked = 0;
	L->rootcl.next = NULL;
	L->rootcl.marked = 0;
	L->rootglobal.next = NULL;
	L->rootglobal.marked = 0;
	L->roottable.next = NULL;
	L->roottable.marked = 0;
	L->refArray = NULL;
	L->refSize = 0;
	L->GCthreshold = GARBAGE_BLOCK;
	L->nblocks = 0;
	L->root_task = luaM_new(lua_Task);
	L->root_task->next = NULL;
	L->last_task = L->root_task;
	L->curr_task = L->root_task;
	luaD_init();
	luaD_initthr();
	luaS_init();
	luaX_init();
}

void lua_open() {
	if (lua_state)
		return;
	lua_state = luaM_new(LState);
	lua_resetglobals();
	luaT_init();
	luaB_predefine();
	luaL_addlibtolist(stdErrorRimFunc, (sizeof(stdErrorRimFunc) / sizeof(stdErrorRimFunc[0])));
}

void lua_close() {
	TaggedString *alludata = luaS_collectudata();
	L->GCthreshold = MAX_INT;  // to avoid GC during GC
	luaC_hashcallIM((Hash *)L->roottable.next);  // GC t.methods for tables
	luaC_strcallIM(alludata);  // GC tag methods for userdata
	luaD_gcIM(&luaO_nilobject);  // GC tag method for nil (signal end of GC)
	luaH_free((Hash *)L->roottable.next);
	luaF_freeproto((TProtoFunc *)L->rootproto.next);
	luaF_freeclosure((Closure *)L->rootcl.next);
	luaS_free(alludata);
	luaS_freeall();
	luaM_free(L->stack.stack);
	luaM_free(L->IMtable);
	luaM_free(L->refArray);
	luaM_free(L->Mbuffer);
	luaM_free(L);
	L = NULL;
#ifdef LUA_DEBUG
	printf("total de blocos: %ld\n", numblocks);
	printf("total de memoria: %ld\n", totalmem);
#endif
}

static void savetask(lua_Task *t) {
	t->stack = L->stack;
	t->Cstack = L->Cstack;
	t->errorJmp = L->errorJmp;
	t->ci = L->ci;
	t->base_ci = L->base_ci;
	t->base_ci_size = L->base_ci_size;
	t->end_ci = L->end_ci;
	t->Mbuffer = L->Mbuffer;
	t->Mbuffbase = L->Mbuffbase;
	t->Mbuffsize = L->Mbuffsize;
	t->Mbuffnext = L->Mbuffnext;
	memcpy(t->Cblocks, L->Cblocks, sizeof(L->Cblocks));
	t->numCblocks = L->numCblocks;
	t->Tstate = L->Tstate;
}

static void loadtask(struct lua_Task *t) {
	L->stack = t->stack;
	L->Cstack = t->Cstack;
	L->errorJmp = t->errorJmp;
	L->ci = t->ci;
	L->base_ci = t->base_ci;
	L->base_ci_size = t->base_ci_size;
	L->end_ci = t->end_ci;
	L->Mbuffer = t->Mbuffer;
	L->Mbuffbase = t->Mbuffbase;
	L->Mbuffsize = t->Mbuffsize;
	L->Mbuffnext = t->Mbuffnext;
	memcpy(L->Cblocks, t->Cblocks, sizeof(L->Cblocks));
	L->numCblocks = t->numCblocks;
	L->Tstate = t->Tstate;
}

void luaI_switchtask(lua_Task *t) {
	savetask(L->curr_task);
	L->curr_task = t;
	loadtask(t);
}

struct lua_Task *luaI_newtask() {
	struct lua_Task *result;

	savetask(L->curr_task);
	result = luaM_new(lua_Task);
	L->curr_task = result;
	result->next = NULL;
	lua_openthr();
	luaD_initthr();
	result->id = globalTaskSerialId++;
	return result;
}
