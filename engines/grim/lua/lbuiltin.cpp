/*
** Built-in functions
** See Copyright Notice in lua.h
*/

#define FORBIDDEN_SYMBOL_EXCEPTION_setjmp
#define FORBIDDEN_SYMBOL_EXCEPTION_longjmp
#define FORBIDDEN_SYMBOL_EXCEPTION_printf

#include "common/util.h"

#include "engines/grim/lua/lapi.h"
#include "engines/grim/lua/lauxlib.h"
#include "engines/grim/lua/lbuiltin.h"
#include "engines/grim/lua/ldo.h"
#include "engines/grim/lua/lfunc.h"
#include "engines/grim/lua/lmem.h"
#include "engines/grim/lua/lobject.h"
#include "engines/grim/lua/lstate.h"
#include "engines/grim/lua/lstring.h"
#include "engines/grim/lua/ltable.h"
#include "engines/grim/lua/ltask.h"
#include "engines/grim/lua/ltm.h"
#include "engines/grim/lua/lua.h"

namespace Grim {

static void pushstring(TaggedString *s) {
	TObject o;
	o.ttype = LUA_T_STRING;
	o.value.ts = s;
	luaA_pushobject(&o);
}

static void nextvar() {
	TObject *o = luaA_Address(luaL_nonnullarg(1));
	TaggedString *g;
	if (ttype(o) == LUA_T_NIL)
		g = (TaggedString *)rootglobal.next;
	else {
		luaL_arg_check(ttype(o) == LUA_T_STRING, 1, "variable name expected");
		g = tsvalue(o);
		// check whether name is in global var list
		luaL_arg_check((GCnode *)g != g->head.next, 1, "variable name expected");
		g = (TaggedString *)g->head.next;
	}
	while (g && g->globalval.ttype == LUA_T_NIL) {
		// skip globals with nil
		g = (TaggedString *)g->head.next;
	}
	if (g) {
		pushstring(g);
		luaA_pushobject(&g->globalval);
	}
}

static void foreachvar() {
	TObject f = *luaA_Address(luaL_functionarg(1));
	GCnode *g;
	StkId name = lua_state->Cstack.base++;  // place to keep var name (to avoid GC)
	ttype(lua_state->stack.stack + name) = LUA_T_NIL;
	lua_state->stack.top++;
	for (g = rootglobal.next; g; g = g->next) {
		TaggedString *s = (TaggedString *)g;
		if (s->globalval.ttype != LUA_T_NIL) {
			ttype(lua_state->stack.stack + name) = LUA_T_STRING;
			tsvalue(lua_state->stack.stack + name) = s;  // keep s on stack to avoid GC
			luaA_pushobject(&f);
			pushstring(s);
			luaA_pushobject(&s->globalval);
			lua_state->state_counter1++;
			luaD_call((lua_state->stack.top - lua_state->stack.stack) - 2, 1);
			lua_state->state_counter1--;
			if (ttype(lua_state->stack.top - 1) != LUA_T_NIL)
				return;
			lua_state->stack.top--;
		}
	}
}

static void next() {
	lua_Object o = luaL_tablearg(1);
	lua_Object r = luaL_nonnullarg(2);
	Node *n = luaH_next(luaA_Address(o), luaA_Address(r));
	if (n) {
		luaA_pushobject(&n->ref);
		luaA_pushobject(&n->val);
	}
}

static void foreach() {
	TObject t = *luaA_Address(luaL_tablearg(1));
	TObject f = *luaA_Address(luaL_functionarg(2));
	int32 i;
	for (i = 0; i < avalue(&t)->nhash; i++) {
		Node *nd = &(avalue(&t)->node[i]);
		if (ttype(ref(nd)) != LUA_T_NIL && ttype(val(nd)) != LUA_T_NIL) {
			luaA_pushobject(&f);
			luaA_pushobject(ref(nd));
			luaA_pushobject(val(nd));
			lua_state->state_counter1++;
			luaD_call((lua_state->stack.top - lua_state->stack.stack) - 2, 1);
			lua_state->state_counter1--;
			if (ttype(lua_state->stack.top - 1) != LUA_T_NIL)
				return;
			lua_state->stack.top--;
		}
	}
}

static void internaldostring() {
	if (lua_getparam(2) != LUA_NOOBJECT)
		lua_error("invalid 2nd argument (probably obsolete code)");
	if (lua_dostring(luaL_check_string(1)) == 0)
		if (luaA_passresults() == 0)
			lua_pushuserdata(0);  // at least one result to signal no errors
}

static const char *to_string(lua_Object obj) {
	char *buff = luaL_openspace(30);
	TObject *o = luaA_Address(obj);
	switch (ttype(o)) {
	case LUA_T_NUMBER:
	case LUA_T_STRING:
		return lua_getstring(obj);
	case LUA_T_ARRAY:
		{
			sprintf(buff, "table: %p", (void *)o->value.a);
			return buff;
		}
	case LUA_T_CLOSURE:
		{
			sprintf(buff, "function: %p", (void *)o->value.cl);
			return buff;
		}
	case LUA_T_PROTO:
		{
			sprintf(buff, "function: %p", (void *)o->value.tf);
			return buff;
		}
	case LUA_T_CPROTO:
		{
			// WORKAROUND: C++ forbids casting from a pointer-to-function to a
			// pointer-to-object. We use a union to work around that.
			union {
				void *objPtr;
				lua_CFunction funcPtr;
			} ptrUnion;

			ptrUnion.funcPtr = o->value.f;

			sprintf(buff, "function: %p", ptrUnion.objPtr);
			return buff;
		}
	case LUA_T_USERDATA:
		{
			sprintf(buff, "userdata: %08X", o->value.ud.id);
			return buff;
		}
	case LUA_T_TASK:
		{
			sprintf(buff, "task: %d", (int)o->value.n);
			return buff;
		}
	case LUA_T_NIL:
		return "nil";
	default:
#ifdef LUA_DEBUG
		LUA_INTERNALERROR("internal error");
#endif
		return nullptr;
	}
}

static void bi_tostring() {
	lua_pushstring(to_string(lua_getparam(1)));
}

static void luaI_print() {
	int32 i = 1;
	lua_Object obj;
	while ((obj = lua_getparam(i++)) != LUA_NOOBJECT)
		printf("%s\t", to_string(obj));
	printf("\n");
}

static void luaI_type() {
	lua_Object o = luaL_nonnullarg(1);
	lua_pushstring(luaO_typenames[-ttype(luaA_Address(o))]);
	lua_pushnumber(lua_tag(o));
}

static void tonumber() {
	int32 base = (int32)luaL_opt_number(2, 10);
	if (base == 10) {  // standard conversion
		lua_Object o = lua_getparam(1);
		if (lua_isnumber(o))
			lua_pushnumber(lua_getnumber(o));
	} else {
		const char *s = luaL_check_string(1);
		char *e;
		int32 n;
		luaL_arg_check(0 <= base && base <= 36, 2, "base out of range");
		n = (int32)strtol(s, &e, base);
		while (Common::isSpace(*e))
			e++; // skip trailing spaces
		if (*e)
			return; // invalid format: return nil
		lua_pushnumber(n);
	}
}

static void luaI_error() {
	lua_error(lua_getstring(lua_getparam(1)));
}

static void luaI_assert() {
	lua_Object p = lua_getparam(1);
	if (p == LUA_NOOBJECT || lua_isnil(p))
		luaL_verror("assertion failed!  %.100s", luaL_opt_string(2, ""));
}

static void setglobal() {
	const char *n = luaL_check_string(1);
	lua_Object value = luaL_nonnullarg(2);
	lua_pushobject(value);
	lua_setglobal(n);
	lua_pushobject(value);  // return given value
}

static void rawsetglobal() {
	const char *n = luaL_check_string(1);
	lua_Object value = luaL_nonnullarg(2);
	lua_pushobject(value);
	lua_rawsetglobal(n);
	lua_pushobject(value);  // return given value
}

static void getglobal() {
	lua_pushobject(lua_getglobal(luaL_check_string(1)));
}

static void rawgetglobal() {
	lua_pushobject(lua_rawgetglobal(luaL_check_string(1)));
}

static void luatag() {
	lua_pushnumber(lua_tag(lua_getparam(1)));
}

static int32 getnarg(lua_Object table) {
	lua_Object temp;
	// temp = table.n
	lua_pushobject(table);
	lua_pushstring("n");
	temp = lua_rawgettable();
	return (lua_isnumber(temp) ? (int32)lua_getnumber(temp) : MAX_WORD);
}

static void luaI_call() {
	lua_Object f = luaL_nonnullarg(1);
	lua_Object arg = luaL_tablearg(2);
	const char *options = luaL_opt_string(3, "");
	lua_Object err = lua_getparam(4);
	int32 narg = getnarg(arg);
	int32 i, status;
	if (err != LUA_NOOBJECT) {  // set new error method
		lua_pushobject(err);
		err = lua_seterrormethod();
	}
	// push arg[1...n]
	for (i = 0; i < narg; i++) {
		lua_Object temp;
		// temp = arg[i + 1]
		lua_pushobject(arg);
		lua_pushnumber(i + 1);
		temp = lua_rawgettable();
		if (narg == MAX_WORD && lua_isnil(temp))
			break;
		lua_pushobject(temp);
	}
	status = lua_callfunction(f);
	if (err != LUA_NOOBJECT) {  // restore old error method
		lua_pushobject(err);
		lua_seterrormethod();
	}
	if (status) {  // error in call?
		if (strchr(options, 'x'))
			return;  // return nil to signal the error
		else
			lua_error(nullptr);
	} else { // no errors
		if (strchr(options, 'p'))
			luaA_packresults();
		else
			luaA_passresults();
	}
}

static void settag() {
	lua_Object o = luaL_tablearg(1);
	lua_pushobject(o);
	lua_settag((int32)luaL_check_number(2));
}

static void newtag() {
	lua_pushnumber(lua_newtag());
}

static void copytagmethods() {
	lua_pushnumber(lua_copytagmethods((int32)luaL_check_number(1), (int32)luaL_check_number(2)));
}

static void rawgettable() {
	lua_Object t = luaL_nonnullarg(1);
	lua_Object i = luaL_nonnullarg(2);
	lua_pushobject(t);
	lua_pushobject(i);
	lua_pushobject(lua_rawgettable());
}

static void rawsettable() {
	lua_Object t = luaL_nonnullarg(1);
	lua_Object i = luaL_nonnullarg(2);
	lua_Object v = luaL_nonnullarg(3);
	lua_pushobject(t);
	lua_pushobject(i);
	lua_pushobject(v);
	lua_rawsettable();
}

static void settagmethod() {
	lua_Object nf = luaL_nonnullarg(3);
	lua_pushobject(nf);
	lua_pushobject(lua_settagmethod((int32)luaL_check_number(1), luaL_check_string(2)));
}

static void gettagmethod() {
	lua_pushobject(lua_gettagmethod((int32)luaL_check_number(1), luaL_check_string(2)));
}

static void seterrormethod() {
	lua_Object nf = luaL_functionarg(1);
	lua_pushobject(nf);
	lua_pushobject(lua_seterrormethod());
}

static void luaI_collectgarbage() {
	lua_pushnumber(lua_collectgarbage((int32)luaL_opt_number(1, 0)));
}

/*
** =======================================================
** some DEBUG functions
** =======================================================
*/
#ifdef LUA_DEBUG

static void mem_query() {
	lua_pushnumber(totalmem);
	lua_pushnumber(numblocks);
}

static void countlist() {
	char *s = luaL_check_string(1);
	GCnode *l = (s[0] == 't') ? lua_state->roottable.next : (s[0] == 'c') ? lua_state->rootcl.next :
			(s[0] == 'p') ? lua_state->rootproto.next : lua_state->rootglobal.next;
	int32 i = 0;
	while (l) {
		i++;
		l = l->next;
	}
	lua_pushnumber(i);
}

static void testC() {
#define getnum(s)	((*s++) - '0')
#define getname(s)	(nome[0] = *s++, nome)

	static int32 locks[10];
	lua_Object reg[10];
	char nome[2];
	char *s = luaL_check_string(1);
	nome[1] = 0;
	while (1) {
		switch (*s++) {
			case '0': case '1': case '2': case '3': case '4':
			case '5': case '6': case '7': case '8': case '9':
				lua_pushnumber(*(s - 1) - '0');
				break;
			case 'c':
				reg[getnum(s)] = lua_createtable();
				break;
			case 'C':
				{
					lua_CFunction f = lua_getcfunction(lua_getglobal(getname(s)));
					lua_pushCclosure(f, getnum(s));
					break;
				}
			case 'P':
				reg[getnum(s)] = lua_pop();
				break;
			case 'g':
				{
					int32 n=getnum(s);
					reg[n] = lua_getglobal(getname(s));
					break;
				}
			case 'G':
				{
					int32 n = getnum(s);
					reg[n] = lua_rawgetglobal(getname(s));
					break;
				}
			case 'l':
				locks[getnum(s)] = lua_ref(1);
				break;
			case 'L':
				locks[getnum(s)] = lua_ref(0);
				break;
			case 'r':
				{
					int32 n = getnum(s);
					reg[n] = lua_getref(locks[getnum(s)]);
					break;
				}
			case 'u':
				lua_unref(locks[getnum(s)]);
				break;
			case 'p':
				{
					int32 n = getnum(s);
					reg[n] = lua_getparam(getnum(s));
					break;
				}
			case '=':
				lua_setglobal(getname(s));
				break;
			case 's':
				lua_pushstring(getname(s));
				break;
			case 'o':
				lua_pushobject(reg[getnum(s)]);
				break;
			case 'f':
				lua_call(getname(s));
				break;
			case 'i':
				reg[getnum(s)] = lua_gettable();
				break;
			case 'I':
				reg[getnum(s)] = lua_rawgettable();
				break;
			case 't':
				lua_settable();
				break;
			case 'T':
				lua_rawsettable();
				break;
			default:
				luaL_verror("unknown command in `testC': %c", *(s - 1));
		}
		if (*s == 0)
			return;
		if (*s++ != ' ')
			lua_error("missing ` ' between commands in `testC'");
	}
}

#endif

/*
** Internal functions
*/
static struct luaL_reg int_funcs[] = {
#ifdef LUA_COMPAT2_5
	{ "setfallback", luaT_setfallback },
#endif
#ifdef LUA_DEBUG
	{ "testC", testC },
	{ "totalmem", mem_query },
	{ "count", countlist },
#endif
	{ "assert", luaI_assert },
	{ "call", luaI_call },
	{ "collectgarbage", luaI_collectgarbage },
	{ "copytagmethods", copytagmethods },
	{ "dostring", internaldostring },
	{ "error", luaI_error },
	{ "foreach", foreach },
	{ "foreachvar", foreachvar },
	{ "getglobal", getglobal },
	{ "newtag", newtag },
	{ "next", next },
	{ "nextvar", nextvar },
	{ "print", luaI_print },
	{ "rawgetglobal", rawgetglobal },
	{ "rawgettable", rawgettable },
	{ "rawsetglobal", rawsetglobal },
	{ "rawsettable", rawsettable },
	{ "seterrormethod", seterrormethod },
	{ "setglobal", setglobal },
	{ "settagmethod", settagmethod },
	{ "gettagmethod", gettagmethod },
	{ "settag", settag },
	{ "tonumber", tonumber },
	{ "tostring", bi_tostring },
	{ "tag", luatag },
	{ "type", luaI_type },
	{ "start_script", start_script },
	{ "stop_script", stop_script },
	{ "next_script", next_script },
	{ "identify_script", identify_script },
	{ "pause_scripts", pause_scripts },
	{ "unpause_scripts", unpause_scripts },
	{ "find_script", find_script },
	{ "sleep_for", sleep_for },
	{ "break_here", break_here },
	{ "pause_script", pause_script },
	{ "unpause_script", unpause_script }
};

void luaB_predefine() {
	// pre-register mem error messages, to avoid loop when error arises
	luaS_newfixedstring(tableEM);
	luaS_newfixedstring(memEM);
	luaL_openlib(int_funcs, sizeof(int_funcs) / sizeof(int_funcs[0]));
	lua_pushstring(LUA_VERSION);
	lua_setglobal("_VERSION");
}

} // end of namespace Grim
