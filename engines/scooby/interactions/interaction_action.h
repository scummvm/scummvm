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

#ifndef SCOOBY_INTERACTION_ACTION_H
#define SCOOBY_INTERACTION_ACTION_H

#include "common/scummsys.h"

// Preserves one registered interaction-action menu choice as a coherent value.

namespace Scooby {

struct InteractionAction {
	int32 LabelTextOffset;
	int32 ResponseDialogueOffset;
	int16 RoomObjectStateValue;
	int16 ScriptLength;
	int32 ScriptStartOffset;

	InteractionAction()
		: LabelTextOffset(0), ResponseDialogueOffset(0), RoomObjectStateValue(0), ScriptLength(0),
		  ScriptStartOffset(0) {
	}

	InteractionAction(int32 labelTextOffset, int32 responseDialogueOffset,
					  int16 roomObjectStateValue, int16 scriptLength,
					  int32 scriptStartOffset)
		: LabelTextOffset(labelTextOffset), ResponseDialogueOffset(responseDialogueOffset),
		  RoomObjectStateValue(roomObjectStateValue), ScriptLength(scriptLength),
		  ScriptStartOffset(scriptStartOffset) {
	}
};
} // namespace Scooby

#endif // SCOOBY_INTERACTION_ACTION_H
