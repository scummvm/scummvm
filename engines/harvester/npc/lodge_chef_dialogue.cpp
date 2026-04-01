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


#include "harvester/npc/lodge_chef_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kLodgeChefNpc = "LODGE_CHEF";
static const char *const kPcSpeaker = "PC";
static const int kLodgeChefResponseLine = 0x21;

static const DialogueLineEntry kLodgeChefIntroLines[] = {
	{ 0x1b10, kLodgeChefNpc, 2 },
	{ 0x1b19, kLodgeChefNpc, 0 },
	{ 0x1b1a, kLodgeChefNpc, 3 },
	{ 0x1b1b, kLodgeChefNpc, 0 }
};

static const DialogueLineEntry kLodgeChefTailLines[] = {
	{ 0x1b36, kPcSpeaker, 0 },
	{ 0x1b3a, kLodgeChefNpc, 3 }
};

} // End of namespace

bool LodgeChefDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase(kLodgeChefNpc);
}

Common::Error LodgeChefDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &, DialogueSharedState &) {
	Common::Error lineError = runtime.playDialogueEntrySequence(
		kLodgeChefIntroLines, ARRAYSIZE(kLodgeChefIntroLines));
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	int responseIndex = 0;
	Common::Error responseError = runtime.runResponseMenu(
		kLodgeChefResponseLine, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 1) {
		lineError = runtime.playDialogueLineWithVariant(0x1b2c, kLodgeChefNpc, 3);
	} else if (responseIndex == 2) {
		lineError = runtime.playDialogueLineWithVariant(0x1b31, kLodgeChefNpc, 0);
	} else {
		lineError = Common::kNoError;
	}
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	return runtime.playDialogueEntrySequence(kLodgeChefTailLines, ARRAYSIZE(kLodgeChefTailLines));
}

} // End of namespace Harvester
