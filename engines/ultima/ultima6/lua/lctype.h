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
** $Id: lctype.h,v 1.12.1.1 2013/04/12 18:48:47 roberto Exp $
** 'ctype' functions for Lua
** See Copyright Notice in lua.h
*/

#ifndef ULTIMA6_LUA_lctype_h
#define ULTIMA6_LUA_lctype_h

#include "ultima/ultima6/lua/lua.h"

/*
** WARNING: the functions defined here do not necessarily correspond
** to the similar functions in the standard C ctype.h. They are
** optimized for the specific needs of Lua
*/

#if !defined(LUA_USE_CTYPE)

#if 'A' == 65 && '0' == 48
/* ASCII case: can use its own tables; faster and fixed */
#define LUA_USE_CTYPE	0
#else
/* must use standard C ctype */
#define LUA_USE_CTYPE	1
#endif

#endif


#if !LUA_USE_CTYPE	/* { */

#include <limits.h>

#include "ultima/ultima6/lua/llimits.h"

#define ALPHABIT	0
#define DIGITBIT	1
#define PRINTBIT	2
#define SPACEBIT	3
#define XDIGITBIT	4


#define MASK(B)		(1 << (B))


/*
** add 1 to char to allow index -1 (EOZ)
*/
#define testprop(c,p)	(luai_ctype_[(c)+1] & (p))

/*
** 'lalpha' (Lua alphabetic) and 'lalnum' (Lua alphanumeric) both include '_'
*/
#define lislalpha(c)	testprop(c, MASK(ALPHABIT))
#define lislalnum(c)	testprop(c, (MASK(ALPHABIT) | MASK(DIGITBIT)))
#define lisdigit(c)	testprop(c, MASK(DIGITBIT))
#define lisspace(c)	testprop(c, MASK(SPACEBIT))
#define lisprint(c)	testprop(c, MASK(PRINTBIT))
#define lisxdigit(c)	testprop(c, MASK(XDIGITBIT))

/*
** this 'ltolower' only works for alphabetic characters
*/
#define ltolower(c)	((c) | ('A' ^ 'a'))

namespace Ultima {
namespace Ultima6 {

/* two more entries for 0 and -1 (EOZ) */
LUAI_DDEC const lu_byte luai_ctype_[UCHAR_MAX + 2];

} // End of namespace Ultima6
} // End of namespace Ultima

#else			/* }{ */

/*
** use standard C ctypes
*/

#include <ctype.h>


#define lislalpha(c)	(isalpha(c) || (c) == '_')
#define lislalnum(c)	(isalnum(c) || (c) == '_')
#define lisdigit(c)	(isdigit(c))
#define lisspace(c)	(isspace(c))
#define lisprint(c)	(isprint(c))
#define lisxdigit(c)	(isxdigit(c))

#define ltolower(c)	(tolower(c))

#endif			/* } */

#endif
