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

#ifndef GLK_ALAN2_PARSE
#define GLK_ALAN2_PARSE

/* Parse data for ALAN interpreter module. */

#include "engines/glk/jumps.h"

namespace Glk {
namespace Alan2 {

extern Aword wrds[];    // List of Parsed Word
extern int wrdidx;      // and an index into it

extern ParamElem *params;   // List of parameters
extern Boolean plural;

extern LitElem litValues[];
extern int litCount;

extern int vrbwrd;

// Parse a new player command
extern void parse(CONTEXT);

} // End of namespace Alan2
} // End of namespace Glk

#endif
