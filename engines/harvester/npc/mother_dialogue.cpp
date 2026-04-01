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


#include "harvester/npc/mother_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kMotherSpeakerId = "MOTHER";
static const char *const kDialogueC072FstPath = "GRAPHIC/FST/C072.FST";
static const char *const kDialogueC073FstPath = "GRAPHIC/FST/C073.FST";

} // End of namespace

bool MotherDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("MOTHER");
}

Common::Error MotherDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &, DialogueSharedState &) {
	if (!_state.talkStatePending)
		return Common::kNoError;

	auto playMotherLine = [&](int wavId, int headVariant) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, kMotherSpeakerId, headVariant);
	};

	_state.talkStatePending = false;

	Common::Error lineError = playMotherLine(0x26d6, 3);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	lineError = runtime.playDialogueFst(kDialogueC072FstPath);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	lineError = playMotherLine(0x26e9, 0);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	int responseIndex = 0;
	Common::Error responseError = runtime.runResponseMenu(0x173, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 1) {
		lineError = playMotherLine(0x26f5, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		return runtime.playDialogueFst(kDialogueC073FstPath);
	}

	if (responseIndex != 2)
		return Common::kNoError;

	lineError = runtime.playDialogueFst(kDialogueC073FstPath);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	lineError = playMotherLine(0x2709, 3);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	lineError = runtime.playDialogueFst(kDialogueC072FstPath);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	lineError = playMotherLine(0x2714, 0);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	return playMotherLine(0x2719, 3);
}

} // End of namespace Harvester
