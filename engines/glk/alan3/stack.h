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

#ifndef GLK_ALAN3_STACK
#define GLK_ALAN3_STACK

/* Header file for stack handler in Alan interpreter */

#include "glk/alan3/acode.h"

namespace Glk {
namespace Alan3 {

struct StackStructure {
	Aword *stack;           // Array that can take Awords
	int stackSize;
	int stackp;
	int framePointer;
};

typedef StackStructure *Stack;

/* FUNCTIONS: */

/* NB: The stack uses Aptr size elements since we need to be able to store pointers to allocated memory */

extern Stack createStack(int size);
extern void deleteStack(Stack stack);
extern void dumpStack(Stack theStack);
extern Aptr pop(Stack stack);
extern void push(Stack stack, Aptr item);
extern Aptr top(Stack theStack);
extern int stackDepth(Stack theStack);

extern void newFrame(Stack theStack, Aint noOfLocals);
extern void setLocal(Stack theStack, Aint blocksBelow, Aint variableNumber, Aptr value);
extern Aptr getLocal(Stack theStack, Aint level, Aint variable);
extern void endFrame(Stack theStack);

} // End of namespace Alan3
} // End of namespace Glk

#endif
