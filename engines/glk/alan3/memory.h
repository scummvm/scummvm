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

#ifndef GLK_ALAN3_MEMORY
#define GLK_ALAN3_MEMORY

#include "glk/alan3/sysdep.h"
#include "glk/alan3/acode.h"

namespace Glk {
namespace Alan3 {

/* DATA */
extern Aword *memory;
extern ACodeHeader *header;
extern int memTop;


/* FUNCTIONS */
extern void *allocate(unsigned long lengthInBytes);
extern void *duplicate(void *original, unsigned long len);
extern void deallocate(void *ptr);

extern void resetPointerMap();
extern void *fromAptr(Aptr aptr);
extern Aptr toAptr(void *ptr);

} // End of namespace Alan3
} // End of namespace Glk

#endif
