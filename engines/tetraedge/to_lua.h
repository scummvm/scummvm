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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef TETRAEDGE_TO_LUA_H
#define TETRAEDGE_TO_LUA_H

struct lua_State;
typedef int (*lua_CFunction)(lua_State *state);

namespace Tetraedge {

/*
 * The functions in here are basically the ones implemented by the "tolua" library.
 * They were reimplemented as a simpler subset required by this engine, but
 * referencing the tolua code.
 *
 * The original tolua copyright notice is:
 *
 * Support code for Lua bindings.
 * Written by Waldemar Celes
 * TeCGraf/PUC-Rio
 * Apr 2003
 * This code is free software; you can redistribute it and/or modify it.
 * The software provided hereunder is on an "as is" basis, and
 * the author has no obligation to provide maintenance, support, updates,
 * enhancements, or modifications.
 */
namespace ToLua {

struct tolua_Error {
	int index;
	bool array;
	const char *type;
};

void tolua_open(lua_State *L);
void tolua_module (lua_State *L, const char *name, int hasvar);
void tolua_beginmodule (lua_State *L, const char *name);
void tolua_endmodule(lua_State *L);
void tolua_function(lua_State *L, const char *name, lua_CFunction func);

int tolua_isboolean(lua_State *L, int lo, int def, tolua_Error *err);
int tolua_isnoobj(lua_State *L, int lo, tolua_Error *err);
int tolua_isnumber(lua_State *L, int lo, int def, tolua_Error *err);
int tolua_isstring(lua_State *L, int lo, int def, tolua_Error *err);

double tolua_tonumber(lua_State *L, int narg, double def);
const char* tolua_tostring(lua_State *L, int narg, const char *def);
void* tolua_tousertype(lua_State *L, int narg, void *def);
bool tolua_toboolean(lua_State *L, int narg, bool def);

void tolua_pushboolean(lua_State *L, bool val);
void tolua_pushnumber(lua_State *L, double val);
void tolua_pushstring(lua_State *L, const char *val);

} // end namespace ToLua

} // end namespace Tetraedge

#endif // TETRAEDGE_TO_LUA_H
