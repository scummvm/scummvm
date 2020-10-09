/*
** Tag methods
** See Copyright Notice in lua.h
*/

#define FORBIDDEN_SYMBOL_EXCEPTION_setjmp
#define FORBIDDEN_SYMBOL_EXCEPTION_longjmp
#define FORBIDDEN_SYMBOL_EXCEPTION_fprintf
#define FORBIDDEN_SYMBOL_EXCEPTION_stderr

#include "engines/grim/lua/lauxlib.h"
#include "engines/grim/lua/lmem.h"
#include "engines/grim/lua/lobject.h"
#include "engines/grim/lua/lstate.h"
#include "engines/grim/lua/ltm.h"
#include "engines/grim/lua/lapi.h"

namespace Grim {

const char *luaT_eventname[] = {  // ORDER IM
	"gettable", "settable", "index", "getglobal", "setglobal", "add",
	"sub", "mul", "div", "pow", "unm", "lt", "le", "gt", "ge",
	"concat", "gc", "function", nullptr
};

static int32 luaI_checkevent(const char *name, const char *list[]) {
	int32 e = luaO_findstring(name, list);
	if (e < 0)
		luaL_verror("`%.50s' is not a valid event name", name);
	return e;
}

/* events in LUA_T_NIL are all allowed, since this is used as a
 *  'placeholder' for "default" fallbacks
 */
static char validevents[NUM_TAGS][IM_N] = { // ORDER LUA_T, ORDER IM
	{1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1},  // LUA_T_USERDATA
	{1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1},  // LUA_T_NUMBER
	{1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},  // LUA_T_STRING
	{0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},  // LUA_T_ARRAY
	{1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},  // LUA_T_PROTO
	{1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},  // LUA_T_CPROTO
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},  // LUA_T_TASK
	{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1}   // LUA_T_NIL
};

static int32 validevent(int32 t, int32 e) { // ORDER LUA_T
	return (t < LUA_T_NIL) ? 1 : validevents[-t][e];
}

static void init_entry(int32 tag) {
	int32 i;
	for (i = 0; i < IM_N; i++)
		ttype(luaT_getim(tag, i)) = LUA_T_NIL;
}

static bool tmFBAdded = false;

void luaT_init() {
	int32 t;
	IMtable_size = NUM_TAGS * 2;
	last_tag = -(NUM_TAGS - 1);
	IMtable = luaM_newvector(IMtable_size, struct IM);
	for (t = -(IMtable_size - 1); t <= 0; t++)
		init_entry(t);

	tmFBAdded = false;
}

int32 lua_newtag() {
	--last_tag;
	if (-last_tag >= IMtable_size)
		IMtable_size = luaM_growvector(&IMtable, IMtable_size, struct IM, memEM, MAX_INT);
	init_entry(last_tag);
	return last_tag;
}

static void checktag(int32 tag) {
	if (!(last_tag <= tag && tag <= 0))
		luaL_verror("%d is not a valid tag", tag);
}

void luaT_realtag(int32 tag) {
	if (!(last_tag <= tag && tag < LUA_T_NIL))
		luaL_verror("tag %d is not result of `newtag'", tag);
}

int32 lua_copytagmethods(int32 tagto, int32 tagfrom) {
	int32 e;
	checktag(tagto);
	checktag(tagfrom);
	for (e = 0; e < IM_N; e++) {
		if (validevent(tagto, e))
			*luaT_getim(tagto, e) = *luaT_getim(tagfrom, e);
	}
	return tagto;
}

int32 luaT_efectivetag(TObject *o) {
	int32 t;
	switch (t = ttype(o)) {
	case LUA_T_ARRAY:
		return o->value.a->htag;
	case LUA_T_USERDATA:
		{
			int32 tag = o->value.ud.tag;
			return (tag >= 0) ? LUA_T_USERDATA : tag;
		}
	case LUA_T_CLOSURE:
		return o->value.cl->consts[0].ttype;
#ifdef LUA_DEBUG
	case LUA_T_PMARK:
	case LUA_T_CMARK:
	case LUA_T_CLMARK:
	case LUA_T_LINE:
		lua_error("internal error");
#endif
	default:
		return t;
	}
}

TObject *luaT_gettagmethod(int32 t, const char *event) {
	int32 e = luaI_checkevent(event, luaT_eventname);
	checktag(t);
	if (validevent(t, e))
		return luaT_getim(t,e);
	else
		return &luaO_nilobject;
}

void luaT_settagmethod(int32 t, const char *event, TObject *func) {
	TObject temp = *func;
	int32 e = luaI_checkevent(event, luaT_eventname);
	checktag(t);
	if (!validevent(t, e))
		luaL_verror("settagmethod: cannot change tag method `%.20s' for tag %d",
		luaT_eventname[e], t);
	*func = *luaT_getim(t,e);
	*luaT_getim(t, e) = temp;
}

const char *luaT_travtagmethods(int32 (*fn)(TObject *)) {
	int32 e;
	if (fn(&errorim))
		return "error";
	for (e = IM_GETTABLE; e <= IM_FUNCTION; e++) {  // ORDER IM
		int32 t;
		for (t = 0; t >= last_tag; t--)
			if (fn(luaT_getim(t, e)))
				return luaT_eventname[e];
	}
	return nullptr;
}

/*
* compatibility with old fallback system
*/
#ifdef LUA_COMPAT2_5

void errorFB() {
	lua_Object o = lua_getparam(1);
	if (lua_isstring(o))
		fprintf(stderr, "lua: %s\n", lua_getstring(o));
	else
		fprintf(stderr, "lua: unknown error\n");
}

void nilFB() { }

void typeFB () {
	lua_error("unexpected type");
}

static void fillvalids(IMS e, TObject *func) {
	int32 t;
	for (t = LUA_T_NIL; t <= LUA_T_USERDATA; t++)
		if (validevent(t, e))
			*luaT_getim(t, e) = *func;
}

static luaL_reg tmFB[] = {
	{"  typeFB", typeFB},
	{"  errorFB", errorFB},
	{"  nilFB", nilFB}
};

void luaT_setfallback() {
	static const char *oldnames [] = { "error", "getglobal", "arith", "order", nullptr };
	TObject oldfunc;
	lua_CFunction replace;
	if (!tmFBAdded) {
		luaL_addlibtolist(tmFB, (sizeof(tmFB) / sizeof(tmFB[0])));
		tmFBAdded = true;
	}
	const char *name = luaL_check_string(1);
	lua_Object func = lua_getparam(2);
	luaL_arg_check(lua_isfunction(func), 2, "function expected");
	switch (luaO_findstring(name, oldnames)) {
	case 0:  // old error fallback
		oldfunc = errorim;
		errorim = *luaA_Address(func);
		replace = errorFB;
		break;
	case 1:  // old getglobal fallback
		oldfunc = *luaT_getim(LUA_T_NIL, IM_GETGLOBAL);
		*luaT_getim(LUA_T_NIL, IM_GETGLOBAL) = *luaA_Address(func);
		replace = nilFB;
		break;
	case 2:
		{  // old arith fallback
			int32 i;
			oldfunc = *luaT_getim(LUA_T_NUMBER, IM_POW);
			for (i = IM_ADD; i <= IM_UNM; i++)  // ORDER IM
				fillvalids(i, luaA_Address(func));
			replace = typeFB;
			break;
		}
	case 3:
		{  // old order fallback
			int32 i;
			oldfunc = *luaT_getim(LUA_T_NIL, IM_LT);
			for (i = IM_LT; i <= IM_GE; i++)  // ORDER IM
				fillvalids(i, luaA_Address(func));
			replace = typeFB;
			break;
		}
	default:
		{
			int32 e;
			if ((e = luaO_findstring(name, luaT_eventname)) >= 0) {
				oldfunc = *luaT_getim(LUA_T_NIL, e);
				fillvalids(e, luaA_Address(func));
				replace = (e == IM_GC || e == IM_INDEX) ? nilFB : typeFB;
			} else {
				luaL_verror("`%.50s' is not a valid fallback name", name);
				replace = nullptr;  // to avoid warnings
				oldfunc.ttype = LUA_T_NIL; // to avoid warnings
			}
		}
	}
	if (oldfunc.ttype != LUA_T_NIL)
		luaA_pushobject(&oldfunc);
	else
		lua_pushcfunction(replace);
}

#endif

} // end of namespace Grim
