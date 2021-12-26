/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "glk/alan2/types.h"
#include "glk/alan2/main.h"
#include "glk/alan2/stack.h"

namespace Glk {
namespace Alan2 {

/* PRIVATE DATA */

#define STACKSIZE 100

/* The AMACHINE STACK */
static Aptr stack[STACKSIZE];
static int stackp = 0;


void push(Aptr i) {
	if (stackp == STACKSIZE)
		syserr("Out of stack space.");
	stack[stackp++] = i;
}

Aptr pop() {
	if (stackp == 0)
		syserr("Stack underflow.");
	return (stack[--stackp]);
}

Aptr top() {
	return (stack[stackp - 1]);
}

} // End of namespace Alan2
} // End of namespace Glk
