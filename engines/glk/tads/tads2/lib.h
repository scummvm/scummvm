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

#ifndef GLK_TADS_TADS2_LIB
#define GLK_TADS_TADS2_LIB

#include "common/scummsys.h"
#include "common/algorithm.h"

namespace Glk {
namespace TADS {
namespace TADS2 {

/* short-hand for various types */

typedef unsigned char uchar;
typedef unsigned short ushort;
typedef unsigned long  ulong;

typedef byte  ub1;
typedef signed char    sb1;
typedef char           b1;
typedef unsigned int   ub2;
typedef signed int     sb2;
typedef int            b2;
typedef unsigned long  ub4;
typedef signed long    sb4;
typedef long           b4;
typedef int            eword;

/* maximum/minimum portable values for various types */
#define UB4MAXVAL  0xffffffffUL
#define UB2MAXVAL  0xffffU
#define UB1MAXVAL  0xffU
#define SB4MAXVAL  0x7fffffffL
#define SB2MAXVAL  0x7fff
#define SB1MAXVAL  0x7f
#define SB4MINVAL  (-(0x7fffffff)-1)
#define SB2MINVAL  (-(0x7fff)-1)
#define SB1MINVAL  (-(0x7f)-1)

/* clear a struture */
#define CLRSTRUCT(x) memset(&(x), 0, (size_t)sizeof(x))
#define CPSTRUCT(dst,src) memcpy(&(dst), &(src), (size_t)sizeof(dst))

/* TRUE and FALSE */
#define TRUE true
#define FALSE false


/* bitwise operations */
#define bit(va, bt) ((va) & (bt))
#define bis(va, bt) ((va) |= (bt))
#define bic(va, bt) ((va) &= ~(bt))

/*
 *   noreg/NOREG - was used for variables changed in error-protected code that
 *   are used in error handling code. However, since ScummVM doesn't support
 *   setjmp for portability reasons, the following can be left as blank defines
 */
# define noreg
# define NOREG(arglist)

/* 
 *   Linting directives.  You can define these before including this file
 *   if you have a fussy compiler.  
 */
#ifdef LINT
# ifndef NOTREACHED
#  define NOTREACHED return
# endif
# ifndef NOTREACHEDV
#  define NOTREACHEDV(t) return((t)0)
# endif
# ifndef VARUSED
#  define VARUSED(v) varused(v)
# endif
void varused();
#else /* LINT */
# ifndef NOTREACHED
#  define NOTREACHED
# endif
# ifndef NOTREACHEDV
#  define NOTREACHEDV(t)
# endif
# ifndef VARUSED
#  define VARUSED(v)
# endif
#endif /* LINT */

/* conditionally compile code if debugging is enabled */
#ifdef DEBUG
# define IF_DEBUG(x) x
#else /* DEBUG */
# define IF_DEBUG(x)
#endif /* DEBUG */

#ifndef offsetof
# define offsetof(s_name, m_name) (size_t)&(((s_name *)0)->m_name)
#endif /* offsetof */

/*
 *   Define our own version of isspace(), so that we don't try to interpret
 *   anything outside of the normal ASCII set as spaces. 
 */
#define t_isspace(c) \
    (((unsigned char)(c)) <= 127 && Common::isSpace((unsigned char)(c)))

} // End of namespace TADS2
} // End of namespace TADS
} // End of namespace Glk

#endif
