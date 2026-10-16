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


#include "harvester/npc/sparky_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kDialogueC135FstPath = "GRAPHIC/FST/C135.FST";
static const char *const kDialogueC096FstPath = "GRAPHIC/FST/C096.FST";
static const char *const kDialogueC096AFstPath = "GRAPHIC/FST/C096A.FST";
static const char *const kDialogueC097FstPath = "GRAPHIC/FST/C097.FST";
static const char *const kDialogueC098FstPath = "GRAPHIC/FST/C098.FST";
static const char *const kDialogueNudetatuFstPath = "GRAPHIC/FST/NUDETATU.FST";

} // End of namespace

Common::Error SparkyDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	SparkyRoomDialogueState &state = _state;
	Common::String sparkyTopicBuffer;
	int sparkyTopicBufferLineIndex = -1;

	DialogueTopicBuffer topics(runtime, sparkyTopicBuffer, sparkyTopicBufferLineIndex, "Sparky topic buffer");

	if (runtime.startupScript().getFlagValue("PC_TRIES_TO_TURN_ON_LIGHT")) {
		(void)runtime.startupScript().setRuntimeFlagValue("PC_TRIES_TO_TURN_ON_LIGHT", false);
		return runtime.playDialogueLineWithVariant(0x3a4, "SPARKY", 0);
	}

	if (!usedItemName.empty())
		return runtime.playDialogueLineWithVariant(0x3c7, "SPARKY", 2);

	if (state.introPending) {
		state.introPending = false;
		state.returnVisitPending = true;
		state.returnVisitDayIndex = runtime.startupScript().getCurrentStoryDayIndex();

		const DialogueLineEntry introLines[] = {
			{ 0x2f7, "PC", 1 },
			{ 0x2fe, "SPARKY", 2 },
			{ 0x302, "SPARKY", 0 }
		};
		Common::Error lineError = runtime.playDialogueEntrySequence(introLines, ARRAYSIZE(introLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueFst(kDialogueC135FstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		runtime.executeAndQueueAction("SPARKY_SPOTS_MEAT");
		lineError = runtime.playDialogueFst(kDialogueC098FstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueLineWithVariant(0x30d, "PC", 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x2be, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			sharedState.dialogueStateD2f04 = true;
			const DialogueLineEntry responseLines[] = {
				{ 0x319, "SPARKY", 1 },
				{ 0x322, "PC", 0 },
				{ 0x326, "SPARKY", 4 },
				{ 0x330, "PC", 0 }
			};
			lineError = runtime.playDialogueEntrySequence(responseLines, ARRAYSIZE(responseLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			lineError = runtime.playDialogueLineWithVariant(0x334, "SPARKY", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		lineError = runtime.playDialogueFst(kDialogueC096FstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueLineWithVariant(0x33f, "SPARKY", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueFst(kDialogueC097FstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		topics.assign(0x2c1);
	} else if (state.returnVisitPending) {
		Common::Error lineError = Common::kNoError;
		if (runtime.startupScript().getCurrentStoryDayIndex() == state.returnVisitDayIndex) {
			lineError = runtime.playDialogueLineWithVariant(0x3bb, "SPARKY", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else {
			const DialogueLineEntry returnVisitLines[] = {
				{ 0x3ac, "SPARKY", 1 },
				{ 0x3b0, "PC", 0 },
				{ 0x3b4, "SPARKY", 0 }
			};
			lineError = runtime.playDialogueEntrySequence(returnVisitLines, ARRAYSIZE(returnVisitLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			state.returnVisitDayIndex = runtime.startupScript().getCurrentStoryDayIndex();
		}
	}

	for (;;) {
		Common::String selectedTopic;
		Common::Error menuError = runtime.runKeywordMenu(
			sparkyTopicBuffer, sparkyTopicBufferLineIndex, selectedTopic);
		if (menuError.getCode() != Common::kNoError)
			return menuError;
		if (selectedTopic.empty())
			return Common::kNoError;
		if (runtime.matchesResponseLine(selectedTopic, 0x2c3)) {
			return runtime.playDialogueLineWithVariant(0x39e, "SPARKY", 1);
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x2c4) ||
				runtime.matchesResponseLine(selectedTopic, 0x2c5)) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x34c, "SPARKY", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC096AFstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x2c7)) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x35f, "SPARKY", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x2c8)) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x36c, "SPARKY", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x36d, "SPARKY", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			topics.assign(0x2c9);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x2ca) ||
				runtime.matchesResponseLine(selectedTopic, 0x2cb)) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x37a, "SPARKY", 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x2cc, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = runtime.playDialogueLineWithVariant(0x38f, "SPARKY", 4);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueFst(kDialogueNudetatuFstPath);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else if (responseIndex == 2) {
				lineError = runtime.playDialogueLineWithVariant(0x35f, "SPARKY", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x2ce))
			continue;

		Common::Error lineError = runtime.playDialogueLineWithVariant(0x3c1, "SPARKY", 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
}

} // End of namespace Harvester
