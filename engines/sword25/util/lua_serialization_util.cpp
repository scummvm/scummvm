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

#include "sword25/util/lua_serialization_util.h"

#include "common/scummsys.h"

#include "lua/lobject.h"
#include "lua/lstate.h"
#include "lua/lgc.h"


namespace Lua {

void pushObject(lua_State *luaState, TValue *obj) {
	setobj2s(luaState, luaState->top, obj);

	api_check(luaState, luaState->top < luaState->ci->top);
	luaState->top++;
}

void pushProto(lua_State *luaState, Proto *proto) {
	TValue obj;
	setptvalue(luaState, &obj, proto);

	pushObject(luaState, &obj);
}

void pushUpValue(lua_State *luaState, UpVal *upval) {
	TValue obj;

	obj.value.gc = cast(GCObject *, upval);
	obj.tt = LUA_TUPVAL;
	checkliveness(G(L), obj);

	pushObject(luaState, &obj);
}

void pushString(lua_State *luaState, TString *str) {
	TValue o;
	setsvalue(luaState, &o, str);

	pushObject(luaState, &o);
}

/* A simple reimplementation of the unfortunately static function luaA_index.
 * Does not support the global table, registry, or upvalues. */
StkId getObject(lua_State *luaState, int stackpos) {
	if (stackpos > 0) {
		lua_assert(luaState->base + stackpos - 1 < luaState->top);
		return luaState->base + stackpos - 1;
	} else {
		lua_assert(L->top - stackpos >= L->base);
		return luaState->top + stackpos;
	}
}

} // End of namespace Lua
