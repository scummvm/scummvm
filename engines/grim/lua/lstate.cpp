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
	lua_state->numCblocks = 0;
	lua_state->Cstack.base = 0;
	lua_state->Cstack.lua2C = 0;
	lua_state->Cstack.num = 0;
	lua_state->errorJmp = NULL;
	lua_state->Mbuffsize = 0;
	lua_state->Mbuffnext = 0;
	lua_state->Mbuffbase = NULL;
	lua_state->Mbuffer = NULL;
	lua_state->Tstate = RUN;
}

void lua_resetglobals() {
	globalTaskSerialId = 1;
	lua_openthr();
	lua_state->rootproto.next = NULL;
	lua_state->rootproto.marked = 0;
	lua_state->rootcl.next = NULL;
	lua_state->rootcl.marked = 0;
	lua_state->rootglobal.next = NULL;
	lua_state->rootglobal.marked = 0;
	lua_state->roottable.next = NULL;
	lua_state->roottable.marked = 0;
	lua_state->refArray = NULL;
	lua_state->refSize = 0;
	lua_state->GCthreshold = GARBAGE_BLOCK;
	lua_state->nblocks = 0;
	lua_state->root_task = luaM_new(lua_Task);
	lua_state->root_task->next = NULL;
	lua_state->last_task = lua_state->root_task;
	lua_state->curr_task = lua_state->root_task;
	luaD_init();
	luaD_initthr();
	luaS_init();
	luaX_init();
}

void callHook(lua_Function func, const char *filename, int32 line) {
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
	lua_state->GCthreshold = MAX_INT;  // to avoid GC during GC
	luaC_hashcallIM((Hash *)lua_state->roottable.next);  // GC t.methods for tables
	luaC_strcallIM(alludata);  // GC tag methods for userdata
	luaD_gcIM(&luaO_nilobject);  // GC tag method for nil (signal end of GC)
	luaH_free((Hash *)lua_state->roottable.next);
	luaF_freeproto((TProtoFunc *)lua_state->rootproto.next);
	luaF_freeclosure((Closure *)lua_state->rootcl.next);
	luaS_free(alludata);
	luaS_freeall();
	luaM_free(lua_state->stack.stack);
	luaM_free(lua_state->IMtable);
	luaM_free(lua_state->refArray);
	luaM_free(lua_state->Mbuffer);
	luaM_free(lua_state);
	lua_state = NULL;
#ifdef LUA_DEBUG
	printf("total de blocos: %ld\n", numblocks);
	printf("total de memoria: %ld\n", totalmem);
#endif
}

static void savetask(lua_Task *t) {
	t->stack = lua_state->stack;
	t->Cstack = lua_state->Cstack;
	t->errorJmp = lua_state->errorJmp;
	t->ci = lua_state->ci;
	t->base_ci = lua_state->base_ci;
	t->base_ci_size = lua_state->base_ci_size;
	t->end_ci = lua_state->end_ci;
	t->Mbuffer = lua_state->Mbuffer;
	t->Mbuffbase = lua_state->Mbuffbase;
	t->Mbuffsize = lua_state->Mbuffsize;
	t->Mbuffnext = lua_state->Mbuffnext;
	memcpy(t->Cblocks, lua_state->Cblocks, sizeof(lua_state->Cblocks));
	t->numCblocks = lua_state->numCblocks;
	t->Tstate = lua_state->Tstate;
}

static void loadtask(struct lua_Task *t) {
	lua_state->stack = t->stack;
	lua_state->Cstack = t->Cstack;
	lua_state->errorJmp = t->errorJmp;
	lua_state->ci = t->ci;
	lua_state->base_ci = t->base_ci;
	lua_state->base_ci_size = t->base_ci_size;
	lua_state->end_ci = t->end_ci;
	lua_state->Mbuffer = t->Mbuffer;
	lua_state->Mbuffbase = t->Mbuffbase;
	lua_state->Mbuffsize = t->Mbuffsize;
	lua_state->Mbuffnext = t->Mbuffnext;
	memcpy(lua_state->Cblocks, t->Cblocks, sizeof(lua_state->Cblocks));
	lua_state->numCblocks = t->numCblocks;
	lua_state->Tstate = t->Tstate;
}

void luaI_switchtask(lua_Task *t) {
	savetask(lua_state->curr_task);
	lua_state->curr_task = t;
	loadtask(t);
}

struct lua_Task *luaI_newtask() {
	struct lua_Task *result;

	savetask(lua_state->curr_task);
	result = luaM_new(lua_Task);
	lua_state->curr_task = result;
	result->next = NULL;
	lua_openthr();
	luaD_initthr();
	result->id = globalTaskSerialId++;
	return result;
}

} // end of namespace Grim
