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

#include "glk/alan3/parameter_position.h"
#include "glk/alan3/memory.h"
#include "glk/alan3/lists.h"

namespace Glk {
namespace Alan3 {

/*======================================================================*/
void deallocateParameterPositions(ParameterPosition *parameterPositions) {
	int i;
	for (i = 0; !parameterPositions[i].endOfList; i++) {
		ParameterPosition *position = &parameterPositions[i];
		freeParameterArray(position->parameters);
		if (position->exceptions)
			freeParameterArray(position->exceptions);
	}
	deallocate(parameterPositions);
}

/*======================================================================*/
void uncheckAllParameterPositions(ParameterPosition parameterPositions[]) {
	int position;
	for (position = 0; position < MAXPARAMS; position++) {
		parameterPositions[position].checked = FALSE;
	}
}


/*======================================================================*/
void copyParameterPositions(ParameterPosition originalParameterPositions[], ParameterPosition parameterPositions[]) {
	int i;
	for (i = 0; !originalParameterPositions[i].endOfList; i++)
		parameterPositions[i] = originalParameterPositions[i];
	parameterPositions[i].endOfList = TRUE;
}


/*======================================================================*/
bool equalParameterPositions(ParameterPosition parameterPositions1[], ParameterPosition parameterPositions2[]) {
	int i;
	for (i = 0; !parameterPositions1[i].endOfList; i++) {
		if (parameterPositions2[i].endOfList)
			return FALSE;
		if (!equalParameterArrays(parameterPositions1[i].parameters, parameterPositions2[i].parameters))
			return FALSE;
	}
	return parameterPositions2[i].endOfList;
}


/*======================================================================*/
int findMultipleParameterPosition(ParameterPosition parameterPositions[]) {
	Aint parameterNumber;
	for (parameterNumber = 0; !parameterPositions[parameterNumber].endOfList; parameterNumber++)
		if (parameterPositions[parameterNumber].explicitMultiple)
			return parameterNumber;
	return -1;
}


/*======================================================================*/
void markExplicitMultiple(ParameterPosition parameterPositions[], Parameter parameters[]) {
	int parameterCount;
	for (parameterCount = 0; !parameterPositions[parameterCount].endOfList; parameterCount++)
		if (parameterPositions[parameterCount].explicitMultiple)
			parameters[parameterCount].instance = 0;
}

/*======================================================================*/
void convertPositionsToParameters(ParameterPosition parameterPositions[], Parameter parameters[]) {
	ParameterPosition *position = parameterPositions;

	clearParameterArray(parameters);
	while (!position->endOfList) {
		addParameterToParameterArray(parameters, &position->parameters[0]);
		position++;
	}
}

} // End of namespace Alan3
} // End of namespace Glk
