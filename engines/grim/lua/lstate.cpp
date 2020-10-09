/*
** Global State
** See Copyright Notice in lua.h
*/

#define FORBIDDEN_SYMBOL_EXCEPTION_setjmp
#define FORBIDDEN_SYMBOL_EXCEPTION_longjmp
#define FORBIDDEN_SYMBOL_EXCEPTION_stdin
#define FORBIDDEN_SYMBOL_EXCEPTION_stdout
#define FORBIDDEN_SYMBOL_EXCEPTION_stderror
#define FORBIDDEN_SYMBOL_EXCEPTION_fprintf
#define FORBIDDEN_SYMBOL_EXCEPTION_FILE

#include "common/endian.h"

#include "engines/grim/debug.h"
#include "engines/grim/actor.h"
#include "engines/grim/color.h"

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

LState *lua_state = nullptr;
LState *lua_rootState = nullptr;

int globalTaskSerialId = 0;

void stderrorim();

static luaL_reg stdErrorRimFunc[] = {
	{ "stderrorim", stderrorim }
};

static void lua_openthr() {
	Mbuffsize = 0;
	Mbuffnext = 0;
	Mbuffbase = nullptr;
	Mbuffer = nullptr;
}

#define STACK_UNIT	256

void lua_stateinit(LState *state) {
	state->prev = nullptr;
	state->next = nullptr;
	state->all_paused = 0;
	state->paused = false;
	state->state_counter1 = 0;
	state->state_counter2 = 0;
	state->updated = false;

	state->numCblocks = 0;
	state->Cstack.base = 0;
	state->Cstack.lua2C = 0;
	state->Cstack.num = 0;
	state->errorJmp = nullptr;
	state->id = globalTaskSerialId++;
	state->task = nullptr;
	state->some_task = nullptr;
	state->taskFunc.ttype = LUA_T_NIL;
	state->sleepFor = 0;

	state->stack.stack = luaM_newvector(STACK_UNIT, TObject);
	state->stack.top = state->stack.stack;
	state->stack.last = state->stack.stack + (STACK_UNIT - 1);
}

void lua_statedeinit(LState *state) {
	if (state->prev)
		state->prev->next = state->next;
	if (state->next)
		state->next->prev = state->prev;
	state->next = nullptr;
	state->prev = nullptr;

	if (state->task) {
		lua_Task *t, *m;
		for (t = state->task; t != nullptr;) {
			m = t->next;
			luaM_free(t);
			t = m;
		}
	}

	free(state->stack.stack);
}

void lua_resetglobals() {
	lua_openthr();

	rootproto.next = nullptr;
	rootproto.marked = 0;
	rootcl.next = nullptr;
	rootcl.marked = 0;
	rootglobal.next = nullptr;
	rootglobal.marked = 0;
	roottable.next = nullptr;
	roottable.marked = 0;
	refArray = nullptr;
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
				if (lua_tag(lua_getparam(i)) == MKTAG('A','C','T','R')) {
					Actor *a = Actor::getPool().getObject(lua_getuserdata(lua_getparam(i)));
					fprintf(output, "<actor \"%s\">", a->getName().c_str());
				} else if (lua_tag(lua_getparam(i)) == MKTAG('C','O','L','R')) {
					Color c(lua_getuserdata(lua_getparam(i)));
					fprintf(output, "<color #%02x%02x%02x>", c.getRed(), c.getGreen(), c.getBlue());
				} else
					fprintf(output, "<userdata %d>", lua_getuserdata(lua_getparam(i)));
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
	if (Debug::isChannelEnabled(Debug::Lua))
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

	LState *tmpState, *state;
	for (state = lua_rootState; state != nullptr;) {
		tmpState = state->next;
		lua_statedeinit(state);
		luaM_free(state);
		state = tmpState;
	}

	Mbuffer = nullptr;
	IMtable = nullptr;
	refArray = nullptr;
	lua_rootState = lua_state = nullptr;

#ifdef LUA_DEBUG
	printf("total de blocos: %ld\n", numblocks);
	printf("total de memoria: %ld\n", totalmem);
#endif
}

bool lua_isopen() {
	return (lua_rootState);
}

} // end of namespace Grim
