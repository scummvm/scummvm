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
** $Id: lmem.c,v 1.84.1.1 2013/04/12 18:48:47 roberto Exp $
** Interface to Memory Manager
** See Copyright Notice in lua.h
*/


#include <stddef.h>

#define lmem_c
#define LUA_CORE

#include "ultima/ultima6/lua/lua.h"

#include "ultima/ultima6/lua/ldebug.h"
#include "ultima/ultima6/lua/ldo.h"
#include "ultima/ultima6/lua/lgc.h"
#include "ultima/ultima6/lua/lmem.h"
#include "ultima/ultima6/lua/lobject.h"
#include "ultima/ultima6/lua/lstate.h"

namespace Ultima {
namespace Ultima6 {

/*
** About the realloc function:
** void * frealloc (void *ud, void *ptr, size_t osize, size_t nsize);
** (`osize' is the old size, `nsize' is the new size)
**
** * frealloc(ud, NULL, x, s) creates a new block of size `s' (no
** matter 'x').
**
** * frealloc(ud, p, x, 0) frees the block `p'
** (in this specific case, frealloc must return NULL);
** particularly, frealloc(ud, NULL, 0, 0) does nothing
** (which is equivalent to free(NULL) in ANSI C)
**
** frealloc returns NULL if it cannot create or reallocate the area
** (any reallocation to an equal or smaller size cannot fail!)
*/



#define MINSIZEARRAY	4


void *luaM_growaux_(lua_State *L, void *block, int *size, size_t size_elems,
    int limit, const char *what) {
    void *newblock;
    int newsize;
    if (*size >= limit / 2) {  /* cannot double it? */
        if (*size >= limit)  /* cannot grow even a little? */
            luaG_runerror(L, "too many %s (limit is %d)", what, limit);
        newsize = limit;  /* still have at least one free place */
    } else {
        newsize = (*size) * 2;
        if (newsize < MINSIZEARRAY)
            newsize = MINSIZEARRAY;  /* minimum size */
    }
    newblock = luaM_reallocv(L, block, *size, newsize, size_elems);
    *size = newsize;  /* update only when everything else is OK */
    return newblock;
}


l_noret luaM_toobig(lua_State *L) {
    luaG_runerror(L, "memory allocation error: block too big");
}



/*
** generic allocation routine.
*/
void *luaM_realloc_(lua_State *L, void *block, size_t osize, size_t nsize) {
    void *newblock;
    global_State *g = G(L);
    size_t realosize = (block) ? osize : 0;
    lua_assert((realosize == 0) == (block == NULL));
#if defined(HARDMEMTESTS)
    if (nsize > realosize &&g->gcrunning)
        luaC_fullgc(L, 1);  /* force a GC whenever possible */
#endif
    newblock = (*g->frealloc)(g->ud, block, osize, nsize);
    if (newblock == NULL && nsize > 0) {
        api_check(L, nsize > realosize,
            "realloc cannot fail when shrinking a block");
        if (g->gcrunning) {
            luaC_fullgc(L, 1);  /* try to free some memory... */
            newblock = (*g->frealloc)(g->ud, block, osize, nsize);  /* try again */
        }
        if (newblock == NULL)
            luaD_throw(L, LUA_ERRMEM);
    }
    lua_assert((nsize == 0) == (newblock == NULL));
    g->GCdebt = (g->GCdebt + nsize) - realosize;
    return newblock;
}

} // End of namespace Ultima6
} // End of namespace Ultima
