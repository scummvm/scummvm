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

#include "glk/alan3/state_stack.h"
#include "glk/alan3/syserr.h"
#include "glk/alan3/memory.h"
#include "glk/alan3/state.h"

namespace Glk {
namespace Alan3 {

/* CONSTANTS: */
#define EXTENT 10


/* PRIVATE TYPES: */
struct StateStackStructure {
	void **states;
	char **commands;
	int stackSize;
	int stackPointer;   /* Points above used stack, 0 initially */
	int elementSize;    /* Size of elements in the stack */
};


/*----------------------------------------------------------------------*/
static void *reallocate(void *from, int newSize) {
	void *newArea = realloc(from, newSize * sizeof(void *));
	if (newArea == NULL)
		syserr("Out of memory in 'reallocateStack()'");
	return newArea;
}

/*======================================================================*/
StateStackP createStateStack(int elementSize) {
	StateStackP stack = NEW(StateStackStructure);
	stack->stackSize = 0;
	stack->stackPointer = 0;
	stack->elementSize = elementSize;
	return stack;
}


/*======================================================================*/
void deleteStateStack(StateStackP stateStack) {
	if (stateStack != NULL) {
		while (stateStack->stackPointer > 0) {
			stateStack->stackPointer--;
			deallocateGameState((GameState *)stateStack->states[stateStack->stackPointer]);
			deallocate(stateStack->states[stateStack->stackPointer]);
			deallocate(stateStack->commands[stateStack->stackPointer]);
		}
		if (stateStack->stackSize > 0) {
			deallocate(stateStack->states);
			deallocate(stateStack->commands);
		}
		deallocate(stateStack);
	}
}


/*======================================================================*/
bool stateStackIsEmpty(StateStackP stateStack) {
	return stateStack->stackPointer == 0;
}


/*----------------------------------------------------------------------*/
static void ensureSpaceForGameState(StateStackP stack) {
	if (stack->stackPointer == stack->stackSize) {
		stack->states = (void **)reallocate(stack->states, stack->stackSize + EXTENT);
		stack->commands = (char **)reallocate(stack->commands, stack->stackSize + EXTENT);
		stack->stackSize += EXTENT;
	}
}


/*======================================================================*/
void pushGameState(StateStackP stateStack, void *gameState) {
	void *element = allocate(stateStack->elementSize);
	memcpy(element, gameState, stateStack->elementSize);
	ensureSpaceForGameState(stateStack);
	stateStack->commands[stateStack->stackPointer] = NULL;
	stateStack->states[stateStack->stackPointer++] = element;
}


/*======================================================================*/
void attachPlayerCommandsToLastState(StateStackP stateStack, char *playerCommands) {
	stateStack->commands[stateStack->stackPointer - 1] = scumm_strdup(playerCommands);
}


/*======================================================================*/
void popGameState(StateStackP stateStack, void *gameState, char **playerCommand) {
	if (stateStack->stackPointer == 0)
		syserr("Popping GameState from empty stack");
	else {
		stateStack->stackPointer--;
		memcpy(gameState, stateStack->states[stateStack->stackPointer], stateStack->elementSize);
		deallocate(stateStack->states[stateStack->stackPointer]);
		*playerCommand = stateStack->commands[stateStack->stackPointer];
	}
}

} // End of namespace Alan3
} // End of namespace Glk
