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


#include "harvester/npc/vet_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kVetInterruptFlag = "VET_INTERRUPT";
static const int kVetInterruptResponseLine = 0x2ff;

static const DialogueLineEntry kVetInterruptIntroLines[] = {
	{ 0x6d9, "VET", 0 },
	{ 0x447, "PC", 0 },
	{ 0x6e1, "VET", 2 }
};

} // End of namespace

Common::Error VetDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &, DialogueSharedState &) {
	if (!runtime.startupScript().getFlagValue(kVetInterruptFlag))
		return Common::kNoError;

	(void)runtime.startupScript().setRuntimeFlagValue(kVetInterruptFlag, false);

	Common::Error lineError = runtime.playDialogueEntrySequence(
		kVetInterruptIntroLines, ARRAYSIZE(kVetInterruptIntroLines));
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	int responseIndex = 0;
	Common::Error responseError = runtime.runResponseMenu(kVetInterruptResponseLine, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 1) {
		lineError = runtime.playDialogueLineWithVariant(0x6ec, "VET", 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	} else if (responseIndex == 2) {
		lineError = runtime.playDialogueLineWithVariant(0x6f0, "VET", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	lineError = runtime.playDialogueLineWithVariant(0x6f4, "VET", 2);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	(void)runtime.startupScript().setRuntimeFlagValue(kVetInterruptFlag, false);
	return Common::kNoError;
}

} // End of namespace Harvester
