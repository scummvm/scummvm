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


#include "harvester/npc/dad_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kDadNpc = "DAD";
static const char *const kPcSpeaker = "PC";
static const char *const kInventoryOwnerName = "INVENTORY";
static const char *const kDialogueC011FstPath = "GRAPHIC/FST/C011.FST";
static const char *const kDialogueC112S4FstPath = "GRAPHIC/FST/C112S4.FST";

static const DialogueLineEntry kDadIntroLines[] = {
	{ 0x39a5, kDadNpc, 0 },
	{ 0x39a9, kPcSpeaker, 0 },
	{ 0x39ad, kDadNpc, 0 },
	{ 0x39b1, kPcSpeaker, 2 }
};

static const DialogueLineEntry kDadWhaleyPhotoLines[] = {
	{ 0x3a41, kDadNpc, 0 },
	{ 0x3a46, kPcSpeaker, 0 },
	{ 0x3a4a, kDadNpc, 0 }
};

static const DialogueLineEntry kDadBlackmailEvidenceLines[] = {
	{ 0x3a51, kDadNpc, 0 },
	{ 0x3a55, kPcSpeaker, 0 },
	{ 0x3a59, kDadNpc, 0 }
};

static const DialogueLineEntry kDadMeatPermissionConversionLines[] = {
	{ 0x39f2, kPcSpeaker, 0 },
	{ 0x39f7, kDadNpc, 0 }
};

static const DialogueLineEntry kDadLodgeTopicLines[] = {
	{ 0x3a7d, kDadNpc, 0 },
	{ 0x3a83, kPcSpeaker, 0 },
	{ 0x3a88, kDadNpc, 0 }
};

} // End of namespace

bool DadDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase(kDadNpc);
}

Common::Error DadDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	DadRoomDialogueState &state = _state;
	auto playDadLine = [&](int wavId, const char *speakerId = kDadNpc,
			int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, speakerId, headVariant);
	};
	auto playSequence = [&](const DialogueLineEntry *lines, uint count) -> Common::Error {
		return runtime.playDialogueEntrySequence(lines, count);
	};
	auto playDadMeatPermissionConversion = [&]() -> Common::Error {
		Common::Error lineError = playSequence(kDadMeatPermissionConversionLines,
			ARRAYSIZE(kDadMeatPermissionConversionLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = runtime.playDialogueFst(kDialogueC112S4FstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		sharedState.dialogueStateD2eb4 = 1;
		(void)runtime.startupScript().addRuntimeObjectToInventory("MEAT_PERMISSION");
		(void)runtime.startupScript().setRuntimeObjectVisible(
			kInventoryOwnerName, "MEAT_PERMISSION0", false);
		return Common::kNoError;
	};
	auto isBlackmailEvidence = [&](const Common::String &itemName) {
		return itemName.equalsIgnoreCase("NOTE") ||
			itemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
			itemName.equalsIgnoreCase("CHECKBOOK") ||
			itemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY");
	};
	auto isLodgeTopic = [&](const Common::String &topic) {
		return topic.equalsIgnoreCase("Lodge") || topic.equalsIgnoreCase("The Lodge");
	};

	if (runtime.startupScript().getFlagValue("TAKING_BONDAGE")) {
		Common::Error lineError = playDadLine(0x3a61);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = runtime.playDialogueFst(kDialogueC011FstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		return playDadLine(0x3a6b);
	}

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			return playDadLine(0x3a39);
		}

		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			sharedState.discussedWhaleyHerrillPhoto = 1;
			return playSequence(kDadWhaleyPhotoLines, ARRAYSIZE(kDadWhaleyPhotoLines));
		}

		if (isBlackmailEvidence(usedItemName)) {
			sharedState.discussedNoteCheckbookEvidence = 1;
			return playSequence(kDadBlackmailEvidenceLines, ARRAYSIZE(kDadBlackmailEvidenceLines));
		}

		if (usedItemName.equalsIgnoreCase("MEAT_PERMISSION0")) {
			if (sharedState.dadMeatPermissionState != 0)
				return playDadMeatPermissionConversion();

			return playDadLine(0x3a32);
		}

		return playDadLine(0x3a32);
	}

	if (state.introPending) {
		sharedState.dialogueStateD2ed0 = true;
		state.introPending = false;

		Common::Error lineError = playSequence(kDadIntroLines, ARRAYSIZE(kDadIntroLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		sharedState.dialogueStateD2f04 = true;
		return playDadLine(0x39b5);
	}

	Common::Error lineError = playDadLine(0x3a17);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	if (sharedState.dadMeatPermissionState != 0) {
		lineError = playDadMeatPermissionConversion();
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	const Common::String emptyKeywordList;
	for (;;) {
		Common::String selectedTopic;
		Common::Error menuError = runtime.runKeywordMenu(emptyKeywordList, -1, selectedTopic);
		if (menuError.getCode() != Common::kNoError)
			return menuError;
		if (selectedTopic.empty() || selectedTopic.equalsIgnoreCase(runtime.genericByeTopic()))
			return Common::kNoError;

		if (isLodgeTopic(selectedTopic)) {
			sharedState.discussedLodgeTopic = true;
			lineError = playSequence(kDadLodgeTopicLines, ARRAYSIZE(kDadLodgeTopicLines));
		} else {
			lineError = playDadLine(0x3a2c);
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
}

} // End of namespace Harvester
