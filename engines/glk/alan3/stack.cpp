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

#include "glk/alan3/stack.h"
#include "glk/alan3/glkio.h"
#include "glk/alan3/types.h"
#include "glk/alan3/memory.h"
#include "glk/alan3/options.h"
#include "glk/alan3/syserr.h"

namespace Glk {
namespace Alan3 {

/*======================================================================*/
Stack createStack(int size) {
	StackStructure *theStack = NEW(StackStructure);

	theStack->stack = (Aword *)allocate(size * sizeof(Aptr));
	theStack->stackSize = size;
	theStack->framePointer = -1;

	return theStack;
}


/*======================================================================*/
void deleteStack(Stack theStack) {
	if (theStack == NULL)
		syserr("deleting a NULL stack");

	deallocate(theStack->stack);
	deallocate(theStack);
}


/*======================================================================*/
int stackDepth(Stack theStack) {
	return theStack->stackp;
}


/*======================================================================*/
void dumpStack(Stack theStack) {
	int i;

	if (theStack == NULL)
		syserr("NULL stack not supported anymore");

	printf("[");
	for (i = 0; i < theStack->stackp; i++)
		printf("%ld ", (unsigned long) theStack->stack[i]);
	printf("]");
	if (!traceInstructionOption && !tracePushOption)
		printf("\n");
}


/*======================================================================*/
void push(Stack theStack, Aptr i) {
	if (theStack == NULL)
		syserr("NULL stack not supported anymore");

	if (theStack->stackp == theStack->stackSize)
		syserr("Out of stack space.");
	theStack->stack[(theStack->stackp)++] = i;
}


/*======================================================================*/
Aptr pop(Stack theStack) {
	if (theStack == NULL)
		syserr("NULL stack not supported anymore");

	if (theStack->stackp == 0)
		syserr("Stack underflow.");
	return theStack->stack[--(theStack->stackp)];
}


/*======================================================================*/
Aptr top(Stack theStack) {
	if (theStack == NULL)
		syserr("NULL stack not supported anymore");

	return theStack->stack[theStack->stackp - 1];
}


/* The AMACHINE Block Frames */

/*======================================================================*/
void newFrame(Stack theStack, Aint noOfLocals) {
	int n;

	if (theStack == NULL)
		syserr("NULL stack not supported anymore");

	push(theStack, theStack->framePointer);
	theStack->framePointer = theStack->stackp;
	for (n = 0; n < noOfLocals; n++)
		push(theStack, 0);
}


/*======================================================================*/
/* Local variables are numbered 1 and up and stored on their index-1 */
Aptr getLocal(Stack theStack, Aint framesBelow, Aint variableNumber) {
	int frame;
	int frameCount;

	if (variableNumber < 1)
		syserr("Reading a non-existing block-local variable.");

	if (theStack == NULL)
		syserr("NULL stack not supported anymore");

	frame = theStack->framePointer;

	if (framesBelow != 0)
		for (frameCount = framesBelow; frameCount != 0; frameCount--)
			frame = theStack->stack[frame - 1];

	return theStack->stack[frame + variableNumber - 1];
}


/*======================================================================*/
void setLocal(Stack theStack, Aint framesBelow, Aint variableNumber, Aptr value) {
	int frame;
	int frameCount;

	if (variableNumber < 1)
		syserr("Writing a non-existing block-local variable.");

	if (theStack == NULL)
		syserr("NULL stack not supported anymore");

	frame = theStack->framePointer;
	if (framesBelow != 0)
		for (frameCount = framesBelow; frameCount != 0; frameCount--)
			frame = theStack->stack[frame - 1];

	theStack->stack[frame + variableNumber - 1] = value;
}

/*======================================================================*/
void endFrame(Stack theStack) {
	if (theStack == NULL)
		syserr("NULL stack not supported anymore");

	theStack->stackp = theStack->framePointer;
	theStack->framePointer = pop(theStack);
}

} // End of namespace Alan3
} // End of namespace Glk
