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

#ifndef GLK_ALAN3_STATE_STACK
#define GLK_ALAN3_STATE_STACK

#include "glk/alan3/types.h"

namespace Glk {
namespace Alan3 {

/* TYPES */
typedef struct StateStackStructure *StateStackP;

/* FUNCTIONS */
extern StateStackP createStateStack(int elementSize);
extern bool stateStackIsEmpty(StateStackP stateStack);
extern void pushGameState(StateStackP stateStack, void *state);
extern void popGameState(StateStackP stateStack, void *state, char **playerCommandPointer);
extern void attachPlayerCommandsToLastState(StateStackP stateStack, char *playerCommand);
extern void deleteStateStack(StateStackP stateStack);

} // End of namespace Alan3
} // End of namespace Glk

#endif
