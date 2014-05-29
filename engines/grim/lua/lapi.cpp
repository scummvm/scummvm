/*
** Lua API
** See Copyright Notice in lua.h
*/

#define FORBIDDEN_SYMBOL_EXCEPTION_setjmp
#define FORBIDDEN_SYMBOL_EXCEPTION_longjmp

#include "engines/grim/lua/lapi.h"
#include "engines/grim/lua/lauxlib.h"
#include "engines/grim/lua/ldo.h"
#include "engines/grim/lua/lfunc.h"
#include "engines/grim/lua/lgc.h"
#include "engines/grim/lua/lmem.h"
#include "engines/grim/lua/lobject.h"
#include "engines/grim/lua/lstate.h"
#include "engines/grim/lua/lstring.h"
#include "engines/grim/lua/ltable.h"
#include "engines/grim/lua/ltm.h"
#include "engines/grim/lua/lua.h"
#include "engines/grim/lua/luadebug.h"
#include "engines/grim/lua/lvm.h"


namespace Grim {

char lua_ident[] = "$Lua: " LUA_VERSION " " LUA_COPYRIGHT " $\n"
"$Autores:  " LUA_AUTHORS " $";



TObject *luaA_Address(lua_Object o) {
	return Address(o);
}

static int32 normalized_type(TObject *o) {
	int32 t = ttype(o);
	switch (t) {
	case LUA_T_PMARK:
		return LUA_T_PROTO;
	case LUA_T_CMARK:
		return LUA_T_CPROTO;
	case LUA_T_CLMARK:
		return LUA_T_CLOSURE;
	default:
		return t;
	}
}

void set_normalized(TObject *d, TObject *s) {
	d->value = s->value;
	d->ttype = (lua_Type)normalized_type(s);
}

static TObject *luaA_protovalue (TObject *o) {
	return (normalized_type(o) == LUA_T_CLOSURE) ?  protovalue(o) : o;
}

void luaA_packresults() {
	luaV_pack(lua_state->Cstack.lua2C, lua_state->Cstack.num, lua_state->stack.top);
	incr_top;
}

int32 luaA_passresults() {
	luaD_checkstack(lua_state->Cstack.num);
	memcpy(lua_state->stack.top, lua_state->Cstack.lua2C + lua_state->stack.stack, lua_state->Cstack.num * sizeof(TObject));
	lua_state->stack.top += lua_state->Cstack.num;
	return lua_state->Cstack.num;
}

static void checkCparams(int32 nParams) {
	if (lua_state->stack.top - lua_state->stack.stack < lua_state->Cstack.base + nParams)
		lua_error("API error - wrong number of arguments in C2lua stack");
}

static lua_Object put_luaObject(TObject *o) {
	luaD_openstack((lua_state->stack.top - lua_state->stack.stack) - lua_state->Cstack.base);
	lua_state->stack.stack[lua_state->Cstack.base++] = *o;

	return lua_state->Cstack.base;  // this is +1 real position (see Ref)
}

static lua_Object put_luaObjectonTop() {
	luaD_openstack((lua_state->stack.top - lua_state->stack.stack) - lua_state->Cstack.base);
	lua_state->stack.stack[lua_state->Cstack.base++] = *(--lua_state->stack.top);
	return lua_state->Cstack.base;  // this is +1 real position (see Ref)
}

lua_Object lua_pop() {
	checkCparams(1);
	return put_luaObjectonTop();
}

/*
** Get a parameter, returning the object handle or LUA_NOOBJECT on error.
** 'number' must be 1 to get the first parameter.
*/
lua_Object lua_lua2C(int32 number) {
	if (number <= 0 || number > lua_state->Cstack.num)
		return LUA_NOOBJECT;
	return lua_state->Cstack.lua2C + number;
}

int32 lua_callfunction(lua_Object function) {
	if (function == LUA_NOOBJECT) {
		return 1;
	} else {
		luaD_openstack((lua_state->stack.top - lua_state->stack.stack) - lua_state->Cstack.base);
		set_normalized(lua_state->stack.stack + lua_state->Cstack.base, Address(function));
		return luaD_protectedrun(MULT_RET);
	}
}

lua_Object lua_gettagmethod(int32 tag, const char *event) {
	return put_luaObject(luaT_gettagmethod(tag, event));
}

lua_Object lua_settagmethod(int32 tag, const char *event) {
	checkCparams(1);
	luaT_settagmethod(tag, event, lua_state->stack.top - 1);
	return put_luaObjectonTop();
}

lua_Object lua_seterrormethod() {
	TObject temp = errorim;
	checkCparams(1);
	errorim = *(--lua_state->stack.top);
	return put_luaObject(&temp);
}

lua_Object lua_gettable() {
	checkCparams(2);
	luaV_gettable();
	return put_luaObjectonTop();
}

lua_Object lua_rawgettable() {
	checkCparams(2);
	if (ttype(lua_state->stack.top-2) != LUA_T_ARRAY) {
		lua_error("indexed expression not a table in rawgettable");
	} else {
		TObject *h = luaH_get(avalue(lua_state->stack.top - 2), lua_state->stack.top - 1);
		--lua_state->stack.top;
		if (h) {
			*(lua_state->stack.top-1) = *h;
		} else {
			ttype(lua_state->stack.top-1) = LUA_T_NIL;
		}
	}
	return put_luaObjectonTop();
}

void lua_settable() {
	checkCparams(3);
	luaV_settable(lua_state->stack.top-3, 1);
}

void lua_rawsettable() {
	checkCparams(3);
	luaV_settable(lua_state->stack.top-3, 0);
}

lua_Object lua_createtable() {
	TObject o;
	luaC_checkGC();
	avalue(&o) = luaH_new(0);
	ttype(&o) = LUA_T_ARRAY;
	return put_luaObject(&o);
}

lua_Object lua_getglobal(const char *name) {
	luaD_checkstack(2);  // may need that to call T.M.
	luaV_getglobal(luaS_new(name));
	return put_luaObjectonTop();
}

lua_Object lua_rawgetglobal(const char *name) {
	TaggedString *ts = luaS_new(name);
	return put_luaObject(&ts->globalval);
}

void lua_setglobal(const char *name) {
	checkCparams(1);
	luaD_checkstack(2);  // may need that to call T.M.
	luaV_setglobal(luaS_new(name));
}

void lua_rawsetglobal(const char *name) {
	TaggedString *ts = luaS_new(name);
	checkCparams(1);
	luaS_rawsetglobal(ts, --lua_state->stack.top);
}

int32 lua_isnil(lua_Object o) {
	return (o == LUA_NOOBJECT) || (ttype(Address(o)) == LUA_T_NIL);
}

int32 lua_istable(lua_Object o) {
	return (o != LUA_NOOBJECT) && (ttype(Address(o)) == LUA_T_ARRAY);
}

int32 lua_isuserdata(lua_Object o) {
	return (o != LUA_NOOBJECT) && (ttype(Address(o)) == LUA_T_USERDATA);
}

int32 lua_iscfunction(lua_Object o) {
	return (lua_tag(o) == LUA_T_CPROTO);
}

int32 lua_isnumber(lua_Object o) {
	return (o!= LUA_NOOBJECT) && (tonumber(Address(o)) == 0);
}

int32 lua_isstring(lua_Object o) {
	int32 t = lua_tag(o);
	return (t == LUA_T_STRING) || (t == LUA_T_NUMBER);
}

int32 lua_isfunction(lua_Object o) {
	int32 t = lua_tag(o);
	return (t == LUA_T_PROTO) || (t == LUA_T_CPROTO);
}

float lua_getnumber(lua_Object object) {
	if (object == LUA_NOOBJECT)
		return 0.0f;
	if (tonumber(Address(object)))
		return 0.0f;
	else
		return (nvalue(Address(object)));
}

const char *lua_getstring (lua_Object object) {
	if (object == LUA_NOOBJECT || tostring(Address(object)))
		return nullptr;
	else
		return (svalue(Address(object)));
}

int32 lua_getuserdata(lua_Object object) {
	if (object == LUA_NOOBJECT || ttype(Address(object)) != LUA_T_USERDATA)
		return 0;
	else
		return (Address(object))->value.ud.id;
}

lua_CFunction lua_getcfunction(lua_Object object) {
	if (!lua_iscfunction(object))
		return nullptr;
	else
		return fvalue(luaA_protovalue(Address(object)));
}

void lua_pushnil() {
	ttype(lua_state->stack.top) = LUA_T_NIL;
	incr_top;
}

void lua_pushnumber(float n) {
	ttype(lua_state->stack.top) = LUA_T_NUMBER;
	nvalue(lua_state->stack.top) = n;
	incr_top;
}

void lua_pushstring(const char *s) {
	if (!s)
		ttype(lua_state->stack.top) = LUA_T_NIL;
	else {
		tsvalue(lua_state->stack.top) = luaS_new(s);
		ttype(lua_state->stack.top) = LUA_T_STRING;
	}
	incr_top;
	luaC_checkGC();
}

void lua_pushCclosure(lua_CFunction fn, int32 n) {
	if (!fn)
		lua_error("API error - attempt to push a NULL Cfunction");
	checkCparams(n);
	ttype(lua_state->stack.top) = LUA_T_CPROTO;
	fvalue(lua_state->stack.top) = fn;
	incr_top;
	luaV_closure(n);
}

void lua_pushusertag(int32 u, int32 tag) {
	if (tag < 0 && tag != LUA_ANYTAG)
		luaT_realtag(tag);  // error if tag is not valid
	lua_state->stack.top->value.ud.id = u;
	lua_state->stack.top->value.ud.tag = tag;
	ttype(lua_state->stack.top) = LUA_T_USERDATA;
	incr_top;
	luaC_checkGC();
}

void luaA_pushobject(TObject *o) {
	*lua_state->stack.top = *o;
	incr_top;
}

void lua_pushobject(lua_Object o) {
	if (o == LUA_NOOBJECT)
		lua_error("API error - attempt to push a NOOBJECT");
	else {
		set_normalized(lua_state->stack.top, Address(o));
		incr_top;
	}
}


int32 lua_tag(lua_Object lo) {
	if (lo == LUA_NOOBJECT)
		return LUA_T_NIL;
	else {
		TObject *o = Address(lo);
		int32 t;
		switch (t = ttype(o)) {
		case LUA_T_USERDATA:
			return o->value.ud.tag;
		case LUA_T_ARRAY:
			return o->value.a->htag;
		case LUA_T_PMARK:
			return LUA_T_PROTO;
		case LUA_T_CMARK:
			return LUA_T_CPROTO;
		case LUA_T_CLOSURE: case LUA_T_CLMARK:
			return o->value.cl->consts[0].ttype;
#ifdef LUA_DEBUG
		case LUA_T_LINE:
			LUA_INTERNALERROR("internal error");
#endif
		default:
			return t;
		}
	}
}

void lua_settag(int32 tag) {
	checkCparams(1);
	luaT_realtag(tag);
	switch (ttype(lua_state->stack.top - 1)) {
	case LUA_T_ARRAY:
		(lua_state->stack.top - 1)->value.a->htag = tag;
		break;
	case LUA_T_USERDATA:
		(lua_state->stack.top - 1)->value.ud.tag = tag;
		break;
	default:
		luaL_verror("cannot change the tag of a %.20s", luaO_typenames[-ttype((lua_state->stack.top - 1))]);
	}
	lua_state->stack.top--;
}

/*
** =======================================================
** Debug interface
** =======================================================
*/

// Hooks
lua_CHFunction lua_callhook = nullptr;
lua_LHFunction lua_linehook = nullptr;

lua_Function lua_stackedfunction(int32 level) {
	StkId i;
	for (i = (lua_state->stack.top - 1) - lua_state->stack.stack; i >= 0; i--) {
		int32 t = lua_state->stack.stack[i].ttype;
		if (t == LUA_T_CLMARK || t == LUA_T_PMARK || t == LUA_T_CMARK)
			if (level-- == 0)
				return Ref(lua_state->stack.stack + i);
	}
	return LUA_NOOBJECT;
}

int32 lua_currentline(lua_Function func) {
	TObject *f = Address(func);
	return (f + 1 < lua_state->stack.top && (f + 1)->ttype == LUA_T_LINE) ? (f + 1)->value.i : -1;
}

lua_Object lua_getlocal(lua_Function func, int32 local_number, char **name) {
	// check whether func is a Lua function
	if (lua_tag(func) != LUA_T_PROTO)
		return LUA_NOOBJECT;
	else {
		TObject *f = Address(func);
		TProtoFunc *fp = luaA_protovalue(f)->value.tf;
		*name = luaF_getlocalname(fp, local_number, lua_currentline(func));
		if (*name) {
			// if "*name", there must be a LUA_T_LINE
			// therefore, f + 2 points to function base
			return Ref((f + 2) + (local_number - 1));
		} else
			return LUA_NOOBJECT;
	}
}

int32 lua_setlocal(lua_Function func, int32 local_number) {
	// check whether func is a Lua function
	if (lua_tag(func) != LUA_T_PROTO)
		return 0;
	else {
		TObject *f = Address(func);
		TProtoFunc *fp = luaA_protovalue(f)->value.tf;
		char *name = luaF_getlocalname(fp, local_number, lua_currentline(func));
		checkCparams(1);
		--lua_state->stack.top;
		if (name) {
			// if "name", there must be a LUA_T_LINE
			// therefore, f+2 points to function base
			*((f + 2) + (local_number - 1)) = *lua_state->stack.top;
			return 1;
		} else
			return 0;
	}
}

void lua_funcinfo(lua_Object func, const char **filename, int32 *linedefined) {
	if (!lua_isfunction(func))
		lua_error("API - `funcinfo' called with a non-function value");
    else {
		TObject *f = luaA_protovalue(Address(func));
		if (normalized_type(f) == LUA_T_PROTO) {
			*filename = tfvalue(f)->fileName->str;
			*linedefined = tfvalue(f)->lineDefined;
		} else {
			*filename = "(C)";
			*linedefined = -1;
		}
	}
}

static int32 checkfunc (TObject *o) {
	return luaO_equalObj(o, lua_state->stack.top);
}

const char *lua_getobjname (lua_Object o, const char **name) {
	// try to find a name for given function
	set_normalized(lua_state->stack.top, Address(o)); // to be accessed by "checkfunc
	*name = luaT_travtagmethods(checkfunc);
	if (*name)
		return "tag-method";
	*name = luaS_travsymbol(checkfunc);
	if (*name)
		return "global";
	else
		return "";
}

/*
** =======================================================
** BLOCK mechanism
** =======================================================
*/

void lua_beginblock() {
	if (lua_state->numCblocks >= MAX_C_BLOCKS)
		lua_error("too many nested blocks");
	lua_state->Cblocks[lua_state->numCblocks] = lua_state->Cstack;
	lua_state->numCblocks++;
}

void lua_endblock() {
	--lua_state->numCblocks;
	lua_state->Cstack = lua_state->Cblocks[lua_state->numCblocks];
	luaD_adjusttop(lua_state->Cstack.base);
}

int32 lua_ref(int32 lock) {
	int32 ref;
	checkCparams(1);
	ref = luaC_ref(lua_state->stack.top - 1, lock);
	lua_state->stack.top--;
	return ref;
}

lua_Object lua_getref(int32 r) {
	TObject *o = luaC_getref(r);
	return (o ? put_luaObject(o) : LUA_NOOBJECT);
}


#ifdef LUA_COMPAT2_5
/*
** API: set a function as a fallback
*/

static void do_unprotectedrun(lua_CFunction f, int32 nParams, int32 nResults) {
	StkId base = (lua_state->stack.top - lua_state->stack.stack) - nParams;
	luaD_openstack(nParams);
	lua_state->stack.stack[base].ttype = LUA_T_CPROTO;
	lua_state->stack.stack[base].value.f = f;
	lua_state->state_counter1++;
	luaD_call(base + 1, nResults);
	lua_state->state_counter1--;
}

lua_Object lua_setfallback(const char *name, lua_CFunction fallback) {
	lua_pushstring(name);
	lua_pushcfunction(fallback);
	do_unprotectedrun(luaT_setfallback, 2, 1);
	return put_luaObjectonTop();
}

#endif

} // end of namespace Grim

