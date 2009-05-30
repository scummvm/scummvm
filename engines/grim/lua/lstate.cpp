/*
** $Id$
** Global State
** See Copyright Notice in lua.h
*/

#include "common/endian.h"

#include "engines/grim/actor.h"

#include "engines/grim/lua/lbuiltin.h"
#include "engines/grim/lua/ldo.h"
#include "engines/grim/lua/lauxlib.h"
#include "engines/grim/lua/lfunc.h"
#include "engines/grim/lua/lgc.h"
#include "engines/grim/lua/llex.h"
#include "engines/grim/lua/lmem.h"
#include "engines/grim/lua/lstate.h"
#include "engines/grim/lua/lstring.h"
#include "engines/grim/lua/ltable.h"
#include "engines/grim/lua/ltask.h"
#include "engines/grim/lua/ltm.h"
#include "engines/grim/lua/lualib.h"
#include "engines/grim/lua/luadebug.h"

namespace Grim {

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

void callHook(lua_Function func, const char *filename, int line) {
	const char *name, *type;
	FILE *output = stdout;
	int i;

	type = lua_getobjname(func, &name);
	if (func == LUA_NOOBJECT) {
		fprintf(output, "%s\n", filename);
		return;
	}

	switch (*type) {
	case 'g':
		fprintf(output, "function: %s(", name);
		for (i = 1; ; i++) {
			if (lua_getparam(i) == LUA_NOOBJECT)
				break;
			if (lua_isnil(lua_getparam(i)))
				fprintf(output, "nil");
			else if (lua_istable(lua_getparam(i)))
				fprintf(output, "{...}");
			else if (lua_isuserdata(lua_getparam(i))) {
				if (lua_tag(lua_getparam(i)) == MKID_BE('ACTR')) {
					Actor *a = static_cast<Actor *>(lua_getuserdata(lua_getparam(i)));
					fprintf(output, "<actor \"%s\">", a->name());
				} else if (lua_tag(lua_getparam(i)) == MKID_BE('COLR')) {
					Color *c = static_cast<Color *>(lua_getuserdata(lua_getparam(i)));
					fprintf(output, "<color #%02x%02x%02x>", c->red(), c->green(), c->blue());
				} else
					fprintf(output, "<userdata %p>", lua_getuserdata(lua_getparam(i)));
			} else if (lua_isfunction(lua_getparam(i))) {
				fprintf(output, "<function>");
			} else if (lua_isnumber(lua_getparam(i)))
				fprintf(output, "%g", lua_getnumber(lua_getparam(i)));
			else if (lua_isstring(lua_getparam(i)))
				fprintf(output, "\"%s\"", lua_getstring(lua_getparam(i)));
			else
				fprintf(output, "<unknown>");
			if (lua_getparam(i + 1) != LUA_NOOBJECT)
				fprintf(output, ", ");
		}
		fprintf(output, ")");
			break;
	case 't':
		fprintf(output, "`%s' tag method", name);
		break;
	default:
		{
			if (line == 0)
				fprintf(output, "{START SCRIPT: %s}", filename);
			else if (line < 0) {
				fprintf(output, "%s", filename);
			} else
				fprintf(output, "function (%s:%d)", filename, line);
		}
	}
	fprintf(output, "\n");
}

void lua_open() {
	if (lua_state)
		return;
	lua_state = luaM_new(LState);
	lua_resetglobals();
	luaT_init();
	luaB_predefine();
	luaL_addlibtolist(stdErrorRimFunc, (sizeof(stdErrorRimFunc) / sizeof(stdErrorRimFunc[0])));
	if (gDebugLevel == DEBUG_LUA || gDebugLevel == DEBUG_ALL)
		lua_callhook = callHook;
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

} // end of namespace Grim
