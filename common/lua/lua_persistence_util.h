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

/**
 * This code is heavily based on the Pluto code base. Copyright below
 */

/* Tamed Pluto - Heavy-duty persistence for Lua
 * Copyright (C) 2004 by Ben Sunshine-Hill, and released into the public
 * domain. People making use of this software as part of an application
 * are politely requested to email the author at sneftel@gmail.com
 * with a brief description of the application, primarily to satisfy his
 * curiosity.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Instrumented by Stefan Reich (info@luaos.net)
 * for Mobile Lua (http://luaos.net/pages/mobile-lua.php)
 */


#ifndef LUA_PERISTENCE_UTIL_H
#define LUA_PERISTENCE_UTIL_H


struct lua_State;

#include "lobject.h"

typedef TValue *StkId;

namespace Lua {

#define lua_malloc(luaState, nsize) lua_realloc(luaState, nullptr, 0, nsize)
#define lua_reallocv(luaState, block, on, n, e) lua_realloc(luaState, block, (on) * (e), (n) * (e))
#define lua_reallocvector(luaState, vec, oldn, n, T) ((vec) = (T *)(lua_reallocv(luaState, vec, oldn, n, sizeof(T))))
#define lua_newVector(luaState, num, T) ((T *)lua_reallocv(luaState, nullptr, 0, num, sizeof(T)))
#define lua_new(luaState,T) (T *)lua_malloc(luaState, sizeof(T))

void *lua_realloc(lua_State *luaState, void *block, size_t osize, size_t nsize);

void pushObject(lua_State *luaState, TValue *obj);
void pushProto(lua_State *luaState, Proto *proto);
void pushUpValue(lua_State *luaState, UpVal *upval);
void pushString(lua_State *luaState, TString *str);

StkId getObject(lua_State *luaState, int stackpos);

void lua_linkObjToGC(lua_State *luaState, GCObject *obj, lu_byte type);

#define sizeLclosure(n) ((sizeof(LClosure)) + sizeof(TValue *) * ((n) - 1))

Closure *lua_newLclosure(lua_State *luaState, int numElements, Table *elementTable);
void pushClosure(lua_State *luaState, Closure *closure);

Proto *createProto(lua_State *luaState);
Proto *makeFakeProto(lua_State *L, lu_byte nups);

TString *createString(lua_State *luaState, const char *str, size_t len);

UpVal *createUpValue(lua_State *luaState, int stackpos);
void unboxUpValue(lua_State *luaState);

/* Appends one stack to another stack, but the stack is reversed in the process */
size_t appendStackToStack_reverse(lua_State *from, lua_State *to);
void correctStack(lua_State *L, TValue *oldstack);
void lua_reallocstack(lua_State *L, int newsize);

void lua_reallocCallInfo(lua_State *lauState, int newsize);

/* Does basically the opposite of luaC_link().
 * Right now this function is rather inefficient; it requires traversing the
 * entire root GC set in order to find one object. If the GC list were doubly
 * linked this would be much easier, but there's no reason for Lua to have
 * that. */
void GCUnlink(lua_State *luaState, GCObject *gco);

TString *lua_newlstr(lua_State *luaState, const char *str, size_t len);
void lua_link(lua_State *luaState, GCObject *o, lu_byte tt);
Proto *lua_newproto(lua_State *luaState) ;

UpVal *makeUpValue(lua_State *luaState, int stackPos);
/**
 * The GC is not fond of finding upvalues in tables. We get around this
 * during persistence using a weakly keyed table, so that the GC doesn't
 * bother to mark them. This won't work in unpersisting, however, since
 * if we make the values weak they'll be collected (since nothing else
 * references them). Our solution, during unpersisting, is to represent
 * upvalues as dummy functions, each with one upvalue.
 */
void boxUpValue_start(lua_State *luaState);
void boxUpValue_finish(lua_State *luaState);

} // End of namespace Lua

#endif
