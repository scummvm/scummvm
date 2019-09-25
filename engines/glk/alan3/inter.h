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

#ifndef GLK_ALAN3_INTER
#define GLK_ALAN3_INTER

/* The interpreter of Acode */

#include "glk/alan3/types.h"
#include "glk/alan3/stack.h"
#include "glk/jumps.h"

namespace Glk {
namespace Alan3 {

/* DATA: */

extern bool stopAtNextLine;
extern int currentLine;
extern int recursionDepth;

/* Global failure flag */
extern bool fail;


/* FUNCTIONS: */

extern void setInterpreterMock(void (*mock)(Aaddr adr));
extern void setInterpreterStack(Stack stack);
extern void interpret(CONTEXT, Aaddr adr);
extern Aword evaluate(CONTEXT, Aaddr adr);

} // End of namespace Alan3
} // End of namespace Glk

#endif
