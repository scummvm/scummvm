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


#include "harvester/npc/fireman2_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

Common::Error Fireman2DialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &, DialogueSharedState &) {
	int wavId = 0x5c7;
	if (_state.talkStateBlock) {
		_state.talkStateBlock = false;
		wavId = 0x5b9;
	}

	Common::Error lineError = runtime.playDialogueLineWithVariant(wavId, "FIREMAN2", 2);
	if (lineError.getCode() != Common::kNoError)
		return lineError;
	if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") && !_state.dialogueStateD2de0) {
		_state.dialogueStateD2de0 = 1;
		lineError = runtime.playDialogueLineWithVariant(0x5c0, "FIREMAN2", 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("BOLT_OF_CLOTH_TAKEN") && !_state.dialogueStateD2de4) {
		_state.dialogueStateD2de4 = 1;
		return runtime.playDialogueLineWithVariant(0x5cd, "FIREMAN2", 3);
	}
	return Common::kNoError;
}

} // End of namespace Harvester
