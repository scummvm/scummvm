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


#include "harvester/npc/curator_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const int kCuratorResponseLine = 0x4a;

static const DialogueLineEntry kCuratorTailLines[] = {
	{ 0x43a, "CURATOR", 0 },
	{ 0x43b, "CURATOR", 0 },
	{ 0x447, "PC", 0 },
	{ 0x44b, "CURATOR", 3 }
};

} // End of namespace

Common::Error CuratorDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &, DialogueSharedState &) {
	Common::Error lineError = runtime.playDialogueLineWithVariant(0x425, "CURATOR", 1);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	int responseIndex = 0;
	Common::Error responseError = runtime.runResponseMenu(kCuratorResponseLine, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 1) {
		lineError = runtime.playDialogueLineWithVariant(0x430, "CURATOR", 3);
	} else if (responseIndex == 2) {
		lineError = runtime.playDialogueLineWithVariant(0x435, "CURATOR", 0);
	} else {
		lineError = Common::kNoError;
	}
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	return runtime.playDialogueEntrySequence(kCuratorTailLines, ARRAYSIZE(kCuratorTailLines));
}

} // End of namespace Harvester
