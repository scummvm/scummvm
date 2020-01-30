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

/*
** $Id: ltm.c,v 2.14.1.1 2013/04/12 18:48:47 roberto Exp $
** Tag methods
** See Copyright Notice in lua.h
*/


#include <string.h>

#define ltm_c
#define LUA_CORE

#include "ultima/nuvie/lua/lua.h"

#include "ultima/nuvie/lua/lobject.h"
#include "ultima/nuvie/lua/lstate.h"
#include "ultima/nuvie/lua/lstring.h"
#include "ultima/nuvie/lua/ltable.h"
#include "ultima/nuvie/lua/ltm.h"

namespace Ultima {
namespace Nuvie {

static const char udatatypename[] = "userdata";

LUAI_DDEF const char *const luaT_typenames_[LUA_TOTALTAGS] = {
  "no value",
  "nil", "boolean", udatatypename, "number",
  "string", "table", "function", udatatypename, "thread",
  "proto", "upval"  /* these last two cases are used for tests only */
};


void luaT_init(lua_State *L) {
    static const char *const luaT_eventname[] = {  /* ORDER TM */
      "__index", "__newindex",
      "__gc", "__mode", "__len", "__eq",
      "__add", "__sub", "__mul", "__div", "__mod",
      "__pow", "__unm", "__lt", "__le",
      "__concat", "__call"
    };
    int i;
    for (i = 0; i < TM_N; i++) {
        G(L)->tmname[i] = luaS_new(L, luaT_eventname[i]);
        luaS_fix(G(L)->tmname[i]);  /* never collect these names */
    }
}


/*
** function to be used with macro "fasttm": optimized for absence of
** tag methods
*/
const TValue *luaT_gettm(Table *events, TMS event, TString *ename) {
    const TValue *tm = luaH_getstr(events, ename);
    lua_assert(event <= TM_EQ);
    if (ttisnil(tm)) {  /* no tag method? */
        events->flags |= cast_byte(1u << event);  /* cache this fact */
        return NULL;
    } else return tm;
}


const TValue *luaT_gettmbyobj(lua_State *L, const TValue *o, TMS event) {
    Table *mt;
    switch (ttypenv(o)) {
    case LUA_TTABLE:
        mt = hvalue(o)->metatable;
        break;
    case LUA_TUSERDATA:
        mt = uvalue(o)->metatable;
        break;
    default:
        mt = G(L)->mt[ttypenv(o)];
    }
    return (mt ? luaH_getstr(mt, G(L)->tmname[event]) : luaO_nilobject);
}

} // End of namespace Nuvie
} // End of namespace Ultima
