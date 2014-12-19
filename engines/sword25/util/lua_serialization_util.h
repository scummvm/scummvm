/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distri8buted in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef LUA_SERIALIZATION_UTIL_H
#define LUA_SERIALIZATION_UTIL_H


struct lua_State;

#include "lua/lobject.h"

typedef TValue *StkId;

namespace Lua {

void pushObject(lua_State *luaState, TValue *obj);
void pushProto(lua_State *luaState, Proto *proto);
void pushUpValue(lua_State *luaState, UpVal *upval);
void pushString(lua_State *luaState, TString *str);
StkId getObject(lua_State *luaState, int stackpos);

} // End of namespace Lua

#endif
