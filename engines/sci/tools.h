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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef SCI_TOOLS_H
#define SCI_TOOLS_H

#include "common/scummsys.h"
#include "common/endian.h"

namespace Sci {


struct GTimeVal {
	long tv_sec;
	long tv_usec;
};



/**** FUNCTION DECLARATIONS ****/

#define getInt16	(int16)READ_LE_UINT16
#define getUInt16	READ_LE_UINT16
#define putInt16	WRITE_LE_UINT16


/* --- */

void sci_gettime(long *seconds, long *useconds);
/* Calculates the current time in seconds and microseconds
** Parameters: (long *) seconds: Pointer to the variable the seconds part of the
**                               current time will be stored in
**             (long *) useconds: Pointer to the variable the microseconds part
**                                of the current time will be stored in
** Returns   : (void)
** The resulting values must be relative to an arbitrary fixed point in time
** (typically 01/01/1970 on *NIX systems).
*/

void sci_get_current_time(GTimeVal *val);
/* GTimeVal version of sci_gettime()
** Parameters: (GTimeVal *) val: Pointer to the structure the values will be stored in
** Returns   : (void)
*/

int sciprintf(const char *fmt, ...) GCC_PRINTF(1, 2);
#define gfxprintf sciprintf
/* Prints a string to the console stack
** Parameters: fmt: a printf-style format string
**             ...: Additional parameters as defined in fmt
** Returns   : (int) 1
** Implementation is in src/scicore/console.c
*/

/** Find first set bit in bits and return its index. Returns 0 if bits is 0. */
int sci_ffs(int bits);


/* The following was originally based on glib.h code, which was
 * Copyright (C) 1995-1997  Peter Mattis, Spencer Kimball and Josh MacDonald
 */
#if defined (__GNUC__) && __GNUC__ >= 2
#  if defined (__i386__)
#    define BREAKPOINT()          {__asm__ __volatile__ ("int $03"); }
#  elif defined(__alpha__)
#    define BREAKPOINT()          {__asm__ __volatile__ ("call_pal 0x80"); }
#  endif /* !__i386__ && !__alpha__ */
#elif defined (_MSC_VER)
#  if defined (_M_IX86)
#    define BREAKPOINT()          { __asm { int 03 } }
#  elif defined(_M_ALPHA)
#    define BREAKPOINT()          { __asm { bpt } }
#  endif /* !_M_IX86 && !_M_ALPHA */
#elif defined (__DECC)
#  if defined(__alpha__)
#    define BREAKPOINT()          {asm ("call_pal 0x80"); }
#  endif /* !__i386__ && !__alpha__ */
#endif
#ifndef BREAKPOINT
#  define BREAKPOINT() { fprintf(stderr, "Missed breakpoint in %s, line %d\n", __FILE__, __LINE__); *((int *) NULL) = 42; }
#endif  /* !BREAKPOINT() */

} // End of namespace Sci

#endif // SCI_TOOLS_H
