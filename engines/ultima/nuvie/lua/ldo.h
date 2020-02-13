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
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL

/*
** $Id: ldo.h,v 2.20.1.1 2013/04/12 18:48:47 roberto Exp $
** Stack and Call structure of Lua
** See Copyright Notice in lua.h
*/

#ifndef NUVIE_LUA_ldo_h
#define NUVIE_LUA_ldo_h


#include "ultima/nuvie/lua/lobject.h"
#include "ultima/nuvie/lua/lstate.h"
#include "ultima/nuvie/lua/lzio.h"

namespace Ultima {
namespace Nuvie {

#define luaD_checkstack(L,n)	if (L->stack_last - L->top <= (n)) \
				    luaD_growstack(L, n); else condmovestack(L);


#define incr_top(L) {L->top++; luaD_checkstack(L,0);}

#define savestack(L,p)		((char *)(p) - (char *)L->stack)
#define restorestack(L,n)	((TValue *)((char *)L->stack + (n)))


/* type of protected functions, to be ran by `runprotected' */
typedef void (*Pfunc) (lua_State *L, void *ud);

LUAI_FUNC int luaD_protectedparser(lua_State *L, ZIO *z, const char *name,
    const char *mode);
LUAI_FUNC void luaD_hook(lua_State *L, int event, int line);
LUAI_FUNC int luaD_precall(lua_State *L, StkId func, int nresults);
LUAI_FUNC void luaD_call(lua_State *L, StkId func, int nResults,
    int allowyield);
LUAI_FUNC int luaD_pcall(lua_State *L, Pfunc func, void *u,
    ptrdiff_t oldtop, ptrdiff_t ef);
LUAI_FUNC int luaD_poscall(lua_State *L, StkId firstResult);
LUAI_FUNC void luaD_reallocstack(lua_State *L, int newsize);
LUAI_FUNC void luaD_growstack(lua_State *L, int n);
LUAI_FUNC void luaD_shrinkstack(lua_State *L);

LUAI_FUNC l_noret luaD_throw(lua_State *L, int errcode);
LUAI_FUNC int luaD_rawrunprotected(lua_State *L, Pfunc f, void *ud);

} // End of namespace Nuvie
} // End of namespace Ultima

#endif

