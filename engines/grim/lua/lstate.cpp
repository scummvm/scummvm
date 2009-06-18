/*
** $Id$
** Global State
** See Copyright Notice in lua.h
*/

#include "common/endian.h"

#include "engines/grim/actor.h"
#include "engines/grim/grim.h"

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

GCnode rootproto;
GCnode rootcl;
GCnode rootglobal;
GCnode roottable;
struct ref *refArray;
int32 refSize;
int32 GCthreshold;
int32 nblocks;
int32 Mbuffsize;
int32 Mbuffnext;
char *Mbuffbase;
char *Mbuffer;
TObject errorim;
stringtable *string_root;
int32 last_tag;
struct IM *IMtable;
int32 IMtable_size;

LState *lua_state = NULL;
LState *lua_rootState = NULL;

int globalTaskSerialId = 0;

void stderrorim();

static luaL_reg stdErrorRimFunc[] = {
	{ "stderrorim", stderrorim }
};

static void lua_openthr() {
	Mbuffsize = 0;
	Mbuffnext = 0;
	Mbuffbase = NULL;
	Mbuffer = NULL;
}

#define STACK_UNIT	256

void lua_stateinit(LState *state) {
	state->prev = NULL;
	state->next = NULL;
	state->paused = 0;
	state->state_counter1 = 0;
	state->state_counter2 = 0;
	state->flag2 = 0;

	state->numCblocks = 0;
	state->Cstack.base = 0;
	state->Cstack.lua2C = 0;
	state->Cstack.num = 0;
	state->errorJmp = NULL;
	state->id = globalTaskSerialId++;
	state->task = NULL;
	state->some_task = NULL;

	state->stack.stack = luaM_newvector(STACK_UNIT, TObject);
	state->stack.top = state->stack.stack;
	state->stack.last = state->stack.stack + (STACK_UNIT - 1);
}

void lua_statedeinit(LState *state) {
	if (state->prev)
		state->prev->next = state->next;
	if (state->next)
		state->next->prev = state->prev;
	state->next = NULL;
	state->prev = NULL;

	if (state->task) {
		lua_Task *t, *m;
		for (t = state->task; t != NULL;) {
			m = t->next;
			luaM_free(t);
			t = m;
		}
	}

	free(state->stack.stack);
}

void lua_resetglobals() {
	lua_openthr();

	rootproto.next = NULL;
	rootproto.marked = 0;
	rootcl.next = NULL;
	rootcl.marked = 0;
	rootglobal.next = NULL;
	rootglobal.marked = 0;
	roottable.next = NULL;
	roottable.marked = 0;
	refArray = NULL;
	refSize = 0;
	GCthreshold = GARBAGE_BLOCK;
	nblocks = 0;

	luaD_init();
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
	lua_rootState = lua_state = luaM_new(LState);
	lua_stateinit(lua_state);
	lua_resetglobals();
	luaT_init();
	luaB_predefine();
	luaL_addlibtolist(stdErrorRimFunc, (sizeof(stdErrorRimFunc) / sizeof(stdErrorRimFunc[0])));
	if (gDebugLevel == DEBUG_LUA || gDebugLevel == DEBUG_ALL)
		lua_callhook = callHook;
}

void lua_close() {
	TaggedString *alludata = luaS_collectudata();
	GCthreshold = MAX_INT;  // to avoid GC during GC
	luaC_hashcallIM((Hash *)roottable.next);  // GC t.methods for tables
	luaC_strcallIM(alludata);  // GC tag methods for userdata
	luaD_gcIM(&luaO_nilobject);  // GC tag method for nil (signal end of GC)
	luaH_free((Hash *)roottable.next);
	luaF_freeproto((TProtoFunc *)rootproto.next);
	luaF_freeclosure((Closure *)rootcl.next);
	luaS_free(alludata);
	luaS_freeall();
	luaM_free(IMtable);
	luaM_free(refArray);
	luaM_free(Mbuffer);

	LState *tmpState, *state = lua_rootState;
	for (state = lua_rootState; state != NULL;) {
		tmpState = state->next;
		lua_statedeinit(state);
		luaM_free(state);
		state = tmpState;
	}

	Mbuffer = NULL;
	IMtable = NULL;
	refArray = NULL;
	lua_rootState = lua_state = NULL;

#ifdef DEBUG
	printf("total de blocos: %ld\n", numblocks);
	printf("total de memoria: %ld\n", totalmem);
#endif
}

} // end of namespace Grim
