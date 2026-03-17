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

#include <functional>

#include "harvester/npc/sparky_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kDialogueC135FstPath = "GRAPHIC/FST/C135.FST";
static const char *const kDialogueC096FstPath = "GRAPHIC/FST/C096.FST";
static const char *const kDialogueC096AFstPath = "GRAPHIC/FST/C096A.FST";
static const char *const kDialogueC098FstPath = "GRAPHIC/FST/C098.FST";
static const char *const kDialogueNudetatuFstPath = "GRAPHIC/FST/NUDETATU.FST";

} // End of namespace

bool SparkyDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("SPARKY");
}

Common::Error SparkyDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	SparkyRoomDialogueState &state = _state;
	Common::String &sparkyTopicBuffer = state.currentTopicBuffer;
	int &sparkyTopicBufferLineIndex = state.currentTopicBufferLineIndex;

	auto assignSparkyTopicBuffer = [&](int responseLineIndex) {
		runtime.assignTopicBuffer(sparkyTopicBuffer, sparkyTopicBufferLineIndex,
			responseLineIndex, "Sparky topic buffer");
	};
	auto playSparkyLine = [&](int wavId, const char *speakerId = "SPARKY") -> Common::Error {
		return runtime.playDialogueLine(wavId, speakerId);
	};
	auto executeDialogueActionTag = [&](const char *tag) {
		StartupInteractionResult interaction;
		if (runtime.startupScript().executeActionTag(tag, interaction)) {
			runtime.applyImmediateDialogueInteractionEffects(interaction);
			runtime.queueDialogueInteractionIfNeeded(interaction);
		}
	};

	if (runtime.startupScript().getFlagValue("PC_TRIES_TO_TURN_ON_LIGHT")) {
		(void)runtime.startupScript().setRuntimeFlagValue("PC_TRIES_TO_TURN_ON_LIGHT", false);
		return playSparkyLine(0x3a4);
	}

	if (!usedItemName.empty())
		return playSparkyLine(0x3c7);

	if (state.introPending) {
		state.introPending = false;
		state.returnVisitPending = true;
		state.returnVisitDayIndex = runtime.startupScript().getCurrentStoryDayIndex();

		const DialogueLineEntry introLines[] = {
			{ 0x2f7, "PC", 0 },
			{ 0x2fe, "SPARKY", 0 },
			{ 0x302, "SPARKY", 0 }
		};
		Common::Error lineError = runtime.playDialogueEntrySequence(introLines, ARRAYSIZE(introLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueFst(kDialogueC135FstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		executeDialogueActionTag("SPARKY_SPOTS_MEAT");
		lineError = runtime.playDialogueFst(kDialogueC098FstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueLine(0x30d, "PC");
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x2be, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			sharedState.dialogueStateD2f04 = true;
			return playSparkyLine(0x319);
		}
		if (responseIndex == 2)
			return playSparkyLine(0x334);

		lineError = runtime.playDialogueFst(kDialogueC096FstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		return playSparkyLine(0x33f);
	}

	if (state.returnVisitPending) {
		Common::Error lineError = Common::kNoError;
		if (runtime.startupScript().getCurrentStoryDayIndex() == state.returnVisitDayIndex) {
			lineError = playSparkyLine(0x3bb);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else {
			const DialogueLineEntry returnVisitLines[] = {
				{ 0x3ac, "SPARKY", 0 },
				{ 0x3b0, "PC", 0 },
				{ 0x3b4, "SPARKY", 0 }
			};
			lineError = runtime.playDialogueEntrySequence(returnVisitLines, ARRAYSIZE(returnVisitLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			state.returnVisitDayIndex = runtime.startupScript().getCurrentStoryDayIndex();
		}
	}

	if (sparkyTopicBufferLineIndex < 0)
		assignSparkyTopicBuffer(0x2c3);

	for (;;) {
		Common::String selectedTopic;
		Common::Error menuError = runtime.runKeywordMenu(
			sparkyTopicBuffer, sparkyTopicBufferLineIndex, selectedTopic);
		if (menuError.getCode() != Common::kNoError)
			return menuError;
		if (selectedTopic.empty())
			return Common::kNoError;
		if (runtime.matchesResponseLine(selectedTopic, 0x2c3)) {
			return playSparkyLine(0x39e);
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x2c4) ||
				runtime.matchesResponseLine(selectedTopic, 0x2c5)) {
			Common::Error lineError = playSparkyLine(0x34c);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC096AFstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x2c7)) {
			Common::Error lineError = playSparkyLine(0x35f);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x2c8)) {
			Common::Error lineError = playSparkyLine(0x36c);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playSparkyLine(0x36d);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignSparkyTopicBuffer(0x2c9);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x2ca) ||
				runtime.matchesResponseLine(selectedTopic, 0x2cb)) {
			Common::Error lineError = playSparkyLine(0x37a);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x2cc, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = playSparkyLine(0x38f);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueFst(kDialogueNudetatuFstPath);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else if (responseIndex == 2) {
				lineError = playSparkyLine(0x35f);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x2ce))
			continue;

		Common::Error lineError = playSparkyLine(0x3c1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
}

} // End of namespace Harvester
