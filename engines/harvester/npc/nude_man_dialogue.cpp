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


#include "harvester/npc/nude_man_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const int kNudeManInitialResponseLine = 0x25d;
static const int kNudeManFollowupResponseLine = 0x25e;

} // End of namespace

bool NudeManDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("NUDE_MAN");
}

Common::Error NudeManDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &, DialogueSharedState &sharedState) {
	auto playNudeManLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, "NUDE_MAN", headVariant);
	};

	if (runtime.startupScript().getFlagValue("DAY_FLAG"))
		return playNudeManLine(0x65c);

	if (_state.talkStatePending) {
		_state.talkStatePending = false;
		const DialogueLineEntry introLines[] = {
			{ 0x606, "PC", 4 },
			{ 0x60a, "NUDE_MAN", 0 },
			{ 0x60e, "PC", 0 },
			{ 0x612, "NUDE_MAN", 0 },
			{ 0x618, "PC", 0 },
			{ 0x61c, "NUDE_MAN", 0 }
		};
		Common::Error lineError = runtime.playDialogueEntrySequence(
			introLines, ARRAYSIZE(introLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(
			kNudeManInitialResponseLine, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playNudeManLine(0x628, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			if (sharedState.sergeantCompletedFirstTaskState == 0)
				return Common::kNoError;

			lineError = playNudeManLine(0x62c);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			responseIndex = 0;
			responseError = runtime.runResponseMenu(
				kNudeManFollowupResponseLine, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1)
				return playNudeManLine(0x636);
			if (responseIndex == 2)
				return playNudeManLine(0x63f);
			return Common::kNoError;
		}

		if (responseIndex == 2)
			return playNudeManLine(0x648, 1);
		return Common::kNoError;
	}

	const DialogueLineEntry revisitLines[] = {
		{ 0x651, "PC", 0 },
		{ 0x655, "NUDE_MAN", 0 },
		{ 0x3063, "PC", 0 },
		{ 0x628, "NUDE_MAN", 0 }
	};
	return runtime.playDialogueEntrySequence(revisitLines, ARRAYSIZE(revisitLines));
}

} // End of namespace Harvester
