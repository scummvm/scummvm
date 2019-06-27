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

#ifndef GLK_ALAN3_PARAMETER_POSITION
#define GLK_ALAN3_PARAMETER_POSITION

/* ParameterPosition

  Represents on position in the player input holding a parameter. That
  position is filled with some words from the player, those words must
  be disambiguated to one or more instances. There are three cases:

  1) words presuming it would be a single instance (it
  might actually not be) "the chair"

  2) words indicating explicit mentioning of multiple instances, "the
  book and the chair"

  3) implicit multiple using "all" or "everything except the blue
  ball"

  For all those cases the position must be able to deliver the words,
  possible explicit or implicit multiple, and the resulting set of
  instances.
*/

#include "glk/alan3/acode.h"
#include "glk/alan3/types.h"
#include "glk/alan3/params.h"

namespace Glk {
namespace Alan3 {

/* Types: */
struct ParameterPosition {
	bool endOfList;
	bool explicitMultiple;
	bool all;
	bool them;
	bool checked;
	Aword flags;
	Parameter *parameters;
	Parameter *exceptions;
};

/* Functions: */
extern void deallocateParameterPositions(ParameterPosition *parameterPositions);
extern void uncheckAllParameterPositions(ParameterPosition parameterPositions[]);
extern void copyParameterPositions(ParameterPosition originalParameterPositions[], ParameterPosition parameterPositions[]);
extern bool equalParameterPositions(ParameterPosition parameterPositions1[], ParameterPosition parameterPositions2[]);
extern int findMultipleParameterPosition(ParameterPosition parameterPositions[]);
extern void markExplicitMultiple(ParameterPosition parameterPositions[], Parameter parameters[]);
extern void convertPositionsToParameters(ParameterPosition parameterPositions[], Parameter parameters[]);

} // End of namespace Alan3
} // End of namespace Glk

#endif
