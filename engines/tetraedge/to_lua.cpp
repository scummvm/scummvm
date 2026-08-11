/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "tetraedge/to_lua.h"
#include "common/lua/lua.h"
#include "common/lua/lauxlib.h"
#include "common/textconsole.h"

namespace Tetraedge {

namespace ToLua {

// Also see the tolua copyright notice in to_lua.h.

static char toluaname[128] = "tolua.";

static void tolua_push_globals_table(lua_State *L) {
	/*
	lua_pushvalue(L, LUA_GLOBALSINDEX);
	lua_pushnumber(L, 2.0);
	lua_rawget(L, -2);
	lua_remove(L, -2);
	*/
	lua_pushvalue(L, LUA_GLOBALSINDEX);
}

static int class_index_event(lua_State *L) {
	error("TODO: Implement ToLua::class_index_event");
}

static int class_newindex_event(lua_State *L) {
	error("TODO: Implement ToLua::class_newindex_event");
}

static int class_add_event(lua_State *L) {
	error("TODO: Implement ToLua::class_add_event");
}

static int class_sub_event(lua_State *L) {
	error("TODO: Implement ToLua::class_sub_event");
}

static int class_mul_event(lua_State *L) {
	error("TODO: Implement ToLua::class_mul_event");
}

static int class_div_event(lua_State *L) {
	error("TODO: Implement ToLua::class_div_event");
}

static int class_lt_event(lua_State *L) {
	error("TODO: Implement ToLua::class_lt_event");
}

static int class_le_event(lua_State *L) {
	error("TODO: Implement ToLua::class_le_event");
}

static int class_eq_event(lua_State *L) {
	error("TODO: Implement ToLua::class_eq_event");
}

static int class_gc_event(lua_State *L) {
	error("TODO: Implement ToLua::class_gc_event");
}

static void tolua_classevents(lua_State *L) {
	lua_pushstring(L, "__index");
	lua_pushcclosure(L, class_index_event, 0);
	lua_rawset(L, -3);
	lua_pushstring(L, "__newindex");
	lua_pushcclosure(L, class_newindex_event, 0);
	lua_rawset(L, -3);
	lua_pushstring(L, "__add");
	lua_pushcclosure(L, class_add_event, 0);
	lua_rawset(L, -3);
	lua_pushstring(L, "__sub");
	lua_pushcclosure(L, class_sub_event, 0);
	lua_rawset(L, -3);
	lua_pushstring(L, "__mul");
	lua_pushcclosure(L, class_mul_event, 0);
	lua_rawset(L, -3);
	lua_pushstring(L, "__div");
	lua_pushcclosure(L, class_div_event, 0);
	lua_rawset(L, -3);
	lua_pushstring(L, "__lt");
	lua_pushcclosure(L, class_lt_event, 0);
	lua_rawset(L, -3);
	lua_pushstring(L, "__le");
	lua_pushcclosure(L, class_le_event, 0);
	lua_rawset(L, -3);
	lua_pushstring(L, "__eq");
	lua_pushcclosure(L, class_eq_event, 0);
	lua_rawset(L, -3);
	lua_pushstring(L, "__gc");
	lua_pushcclosure(L, class_gc_event, 0);
	lua_rawset(L, -3);
}

static void tolua_newmetatable(lua_State *L, const char *name) {
	snprintf(toluaname + 6, 122, "%.120s", name);
	int num = luaL_newmetatable(L, toluaname);
	if (num) {
		lua_pushvalue(L, -1);
		lua_pushstring(L, name);
		lua_rawset(L, LUA_REGISTRYINDEX);
	}
	tolua_classevents(L); // skip these?
	lua_settop(L, -2);
}


static const char* tolua_typename(lua_State *L, int lo) {
	int tag = lua_type(L, lo);
	if (tag == LUA_TNONE)
		lua_pushstring(L, "[no object]");
	else if (tag != LUA_TUSERDATA && tag != LUA_TTABLE)
		lua_pushstring(L, lua_typename(L, tag));
	else if (tag == LUA_TUSERDATA) {
		if (!lua_getmetatable(L, lo)) {
			lua_pushstring(L, lua_typename(L, tag));
		} else {
			lua_rawget(L, LUA_REGISTRYINDEX);
			if (!lua_isstring(L, -1)) {
				lua_pop(L, 1);
				lua_pushstring(L, "[undefined]");
			}
		}
	} else {
		lua_pushvalue(L, lo);
		lua_rawget(L, LUA_REGISTRYINDEX);
		if (!lua_isstring(L, -1)) {
			lua_pop(L, 1);
			lua_pushstring(L, "table");
		} else {
			lua_pushstring(L, "class ");
			lua_insert(L, -2);
			lua_concat(L, 2);
		}
	}
	return lua_tostring(L, -1);
}

static int tolua_bnd_type(lua_State *L) {
	tolua_typename(L, lua_gettop(L));
	return 1;
}

static void *tolua_clone(lua_State *L, void *dest, lua_CFunction fn) {
	lua_pushstring(L, "tolua_gc");
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_pushlightuserdata(L, dest);
	lua_pushcclosure(L, fn, 0);
	lua_rawset(L, -3);
	lua_settop(L, -2);
	return dest;
}


static int tolua_bnd_takeownership(lua_State *L) {
	lua_CFunction fn = nullptr;
	if (lua_isuserdata(L, 1)) {
		if (lua_getmetatable(L, 1)) {
			lua_pushstring(L, ".collector");
			lua_rawget(L, -2);
			if (lua_iscfunction(L, -1)) {
				fn = lua_tocfunction(L, -1);
			}
			lua_settop(L, -3);
			void *data = lua_touserdata(L, 1);
			tolua_clone(L, data, fn);
		}
	}
	lua_pushboolean(L, (uint)(fn != nullptr));
	return 1;
}

static int tolua_bnd_releaseownership(lua_State *L) {
	int result = lua_isuserdata(L, 1);
	if (result) {
		void *p = *(void **)lua_touserdata(L, 1);
		lua_pushstring(L, "tolua_gc");
		lua_rawget(L, LUA_REGISTRYINDEX);
		lua_pushlightuserdata(L, p);
		lua_rawget(L, -2);
		lua_pushlightuserdata(L, p);
		lua_pushnil(L);
		lua_rawset(L, -4);
	}
	lua_pushboolean(L, result != 0);
	return 1;
}

static void tolua_pushusertype(lua_State *L, void *val, const char *name) {
	error("TODO: Implement tolua_pushusertype");
}

static int tolua_bnd_cast(lua_State *L) {
	void *p = tolua_tousertype(L, 1, 0);
	const char *str = tolua_tostring(L, 2, nullptr);
	if (!p) {
		lua_pushnil(L);
	} else if (!str) {
		error("Invalid arguments for 'tolua.cast' function");
	} else {
		snprintf(toluaname + 6, 122, "%.120s", str);
		lua_getfield(L, LUA_REGISTRYINDEX, toluaname);
		if (lua_type(L, -1) == LUA_TNIL) {
			error("Unknown 'type' for 'tolua.cast' function");
		}
		tolua_pushusertype(L, p, str);
	}
	return 1;
}

static void tolua_release(lua_State *L, void *p) {
	lua_pushstring(L, "tolua_ubox");
	lua_rawget(L, LUA_REGISTRYINDEX);
	lua_pushlightuserdata(L, p);
	lua_rawget(L, -2);
	long *pu = static_cast<long *>(lua_touserdata(L, -1));
	if (pu) {
		*pu = 0;
	}
	lua_settop(L, -2);
	lua_pushlightuserdata(L, p);
	lua_pushnil(L);
	lua_rawset(L, -3);
	lua_settop(L, -2);
	return;
}

static int tolua_bnd_release(lua_State *L) {
	void *p = tolua_tousertype(L, 1, 0);
	if (p) {
		tolua_release(L, p);
	}
	return 1;
}

void tolua_open(lua_State *L) {
	int luatop = lua_gettop(L);
	lua_pushstring(L, "tolua_opened");
	lua_rawget(L, LUA_REGISTRYINDEX);
	if (lua_type(L, -1) != LUA_TBOOLEAN) {
		lua_pushstring(L, "tolua_opened");
		lua_pushboolean(L, 1);
		lua_rawset(L, LUA_REGISTRYINDEX);
		lua_pushstring(L, "tolua_ubox");
		lua_createtable(L, 0, 0);
		lua_pushvalue(L, -1);
		lua_pushstring(L, "__mode");
		lua_pushstring(L, "v");
		lua_rawset(L, -3);
		lua_setmetatable(L, -2);
		lua_rawset(L, LUA_REGISTRYINDEX);
		lua_pushstring(L, "tolua_peer");
		lua_createtable(L, 0, 0);
		lua_pushvalue(L, -1);
		lua_pushstring(L, "__mode");
		lua_pushstring(L, "k");
		lua_rawset(L, -3);
		lua_setmetatable(L, -2);
		lua_rawset(L, LUA_REGISTRYINDEX);
		lua_pushstring(L, "tolua_super");
		lua_createtable(L, 0, 0);
		lua_rawset(L, LUA_REGISTRYINDEX);
		lua_pushstring(L, "tolua_gc");
		lua_createtable(L, 0, 0);
		lua_rawset(L, LUA_REGISTRYINDEX);
		tolua_newmetatable(L, "tolua_commonclass");
		tolua_module(L, NULL, 0);
		tolua_beginmodule(L, NULL);
		tolua_module(L, "tolua", 0);
		tolua_beginmodule(L, "tolua");
		tolua_function(L, "type", tolua_bnd_type);
		tolua_function(L, "takeownership", tolua_bnd_takeownership);
		tolua_function(L, "releaseownership", tolua_bnd_releaseownership);
		tolua_function(L, "cast", tolua_bnd_cast);
		tolua_function(L, "release", tolua_bnd_release);
		tolua_endmodule(L);
		tolua_endmodule(L);
	}
	lua_settop(L, luatop);
}

static int module_index_event(lua_State *L) {
	error ("TODO: Implement ToLua::module_index_event");
}

static int module_newindex_event(lua_State *L) {
	error ("TODO: Implement ToLua::module_index_event");
}

static void tolua_moduleevents(lua_State *L) {
	lua_pushstring(L, "__index");
	lua_pushcclosure(L, module_index_event, 0);
	lua_rawset(L, -3);
	lua_pushstring(L, "__newindex");
	lua_pushcclosure(L, module_newindex_event, 0);
	lua_rawset(L, -3);
}

static bool tolua_ismodulemetatable(lua_State *L) {
	if (lua_getmetatable(L, -1)) {
		lua_pushstring(L, "__index");
		lua_rawget(L, -2);
		lua_CFunction fn = lua_tocfunction(L, -1);
		lua_settop(L, -3);
		return fn == module_index_event;
	}
	return false;
}

void tolua_module(lua_State *L, const char *name, int hasvar) {
	if (!name) {
		tolua_push_globals_table(L);
	} else {
		lua_pushstring(L, name);
		lua_rawget(L, -2);
		if (lua_type(L, -1) != LUA_TTABLE) {
			lua_settop(L, -2);
			lua_createtable(L, 0, 0);
			lua_pushstring(L, name);
			lua_pushvalue(L, -2);
			lua_rawset(L, -4);
		}
	}
	if (hasvar) {
		if (!tolua_ismodulemetatable(L)) {
			lua_createtable(L, 0, 0);
			tolua_moduleevents(L);
			if (lua_getmetatable(L, -2)) {
				lua_setmetatable(L, -2);
			}
			lua_setmetatable(L, -2);
		}
	}
	lua_settop(L, -2);
}

void tolua_beginmodule(lua_State *L, const char *name) {
	if (name) {
		lua_pushstring(L, name);
		lua_rawget(L, -2);
	} else {
		tolua_push_globals_table(L);
	}
}

void tolua_function(lua_State *L, const char *name, lua_CFunction func) {
	lua_pushstring(L, name);
	lua_pushcclosure(L, func, 0);
	lua_rawset(L, -3);
}

void tolua_endmodule(lua_State *L) {
	lua_settop(L, -2);
}


int tolua_isboolean(lua_State *L, int lo, int def, tolua_Error *err) {
	if (def && lua_gettop(L) < abs(lo))
		return 1;
	if (lua_isnil(L, lo) || lua_isboolean(L, lo))
		return 1;
	err->index = lo;
	err->array = false;
	err->type = "boolean";
	return 0;
}

int tolua_isnoobj(lua_State *L, int lo, tolua_Error *err) {
	if (lua_gettop(L) < abs(lo))
		return 1;
	err->index = lo;
	err->array = false;
	err->type = "[no object]";
	return 0;
}

int tolua_isnumber(lua_State *L, int lo, int def, tolua_Error *err) {
	if (def && lua_gettop(L) < abs(lo))
		return 1;
	if (lua_isnumber(L, lo))
		return 1;
	err->index = lo;
	err->array = false;
	err->type = "number";
	return 0;
}

int tolua_isstring(lua_State *L, int lo, int def, tolua_Error *err) {
	if (def && lua_gettop(L) < abs(lo))
		return 1;
	if (lua_isnil(L, lo) || lua_isstring(L, lo))
		return 1;
	err->index = lo;
	err->array = false;
	err->type = "string";
	return 0;
}


double tolua_tonumber(lua_State *L, int narg, double def) {
	return lua_gettop(L) < abs(narg) ? def : lua_tonumber(L, narg);
}

const char *tolua_tostring(lua_State *L, int narg, const char *def) {
	return lua_gettop(L) < abs(narg) ? def : lua_tostring(L, narg);
}

void *tolua_tousertype(lua_State *L, int narg, void* def) {
	return lua_gettop(L) < abs(narg) ?  def : lua_touserdata(L, narg);
}

bool tolua_toboolean(lua_State *L, int narg, bool def) {
	return lua_gettop(L) < abs(narg) ?  def : (lua_toboolean(L, narg) != 0);
}

void tolua_pushboolean(lua_State *L, bool val) {
	lua_pushboolean(L, val);
}

void tolua_pushnumber(lua_State *L, double val) {
	lua_pushnumber(L, val);
}

void tolua_pushstring(lua_State *L, const char *val) {
	lua_pushstring(L, val);
}

} // end namespace ToLua

} // end namespace Tetraedge
