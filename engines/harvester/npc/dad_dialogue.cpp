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

#include "harvester/harvester.h"
#include "harvester/npc/dialogue_runtime.h"
#include "harvester/text.h"

namespace Harvester {

namespace {

static const char *const kInventoryOwnerName = "INVENTORY";
static const char *const kEnglishDadIntroResponseMenu = "1. I'd like that./2. I'd rather not.";
static const int kFrenchDadIntroResponseMenuLineIndex = 0x328;
static const char *const kDialogueC011FstPath = "GRAPHIC/FST/C011.FST";
static const char *const kDialogueC112S4FstPath = "GRAPHIC/FST/C112S4.FST";
static const char *const kDialogueC112S7FstPath = "GRAPHIC/FST/C112S7.FST";

static const DialogueLineEntry kDadIntroLines[] = {
	{ 0x39a5, "DAD", 0 },
	{ 0x39a9, "PC", 0 },
	{ 0x39ad, "DAD", 0 },
	{ 0x39b1, "PC", 2 }
};

static const DialogueLineEntry kDadIntroContinuationLines[] = {
	{ 0x39b5, "DAD", 0 },
	{ 0x39ba, "PC", 3 }
};

static const DialogueLineEntry kDadOfferPromptLines[] = {
	{ 0x39c2, "PC", 2 },
	{ 0x39c6, "DAD", 0 }
};

static const DialogueLineEntry kDadOfferAcceptedLines[] = {
	{ 0x39d3, "DAD", 0 },
	{ 0x39d4, "DAD", 0 },
	{ 0x39d5, "DAD", 0 },
	{ 0x39d6, "DAD", 0 },
	{ 0x39d7, "DAD", 0 }
};

static const DialogueLineEntry kDadOfferClosingLines[] = {
	{ 0x39e6, "PC", 0 },
	{ 0x39eb, "DAD", 0 }
};

static const DialogueLineEntry kDadMeatPermissionUnavailableLines[] = {
	{ 0x39fe, "PC", 0 },
	{ 0x3a02, "DAD", 0 }
};

static const DialogueLineEntry kDadWhaleyPhotoLines[] = {
	{ 0x3a41, "DAD", 0 },
	{ 0x3a46, "PC", 0 },
	{ 0x3a4a, "DAD", 0 }
};

static const DialogueLineEntry kDadBlackmailEvidenceLines[] = {
	{ 0x3a51, "DAD", 0 },
	{ 0x3a55, "PC", 0 },
	{ 0x3a59, "DAD", 0 }
};

static const DialogueLineEntry kDadMeatPermissionConversionLines[] = {
	{ 0x39f2, "PC", 0 },
	{ 0x39f7, "DAD", 0 }
};

static const DialogueLineEntry kDadLodgeTopicLines[] = {
	{ 0x3a7d, "DAD", 0 },
	{ 0x3a83, "PC", 0 },
	{ 0x3a88, "DAD", 0 }
};

static Common::String getDadIntroResponseMenu(DialogueRuntime &runtime) {
	if (runtime.engine().getLanguage() == Common::FR_FRA) {
		const Common::Array<Common::String> &responseLines =
			runtime.startupText().getDialogueResponseLines();
		if (responseLines.empty())
			return kEnglishDadIntroResponseMenu;

		if (responseLines.size() > (uint)kFrenchDadIntroResponseMenuLineIndex)
			return responseLines[kFrenchDadIntroResponseMenuLineIndex];

		// The French executable requests DIALOG.RSP[0x328], but failed reads past
		// its 807-line file leave the final line in the native reader's buffer.
		return responseLines[responseLines.size() - 1];
	}

	return kEnglishDadIntroResponseMenu;
}

} // End of namespace

Common::Error DadDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	DadRoomDialogueState &state = _state;

	auto playDadMeatPermissionConversion = [&]() -> Common::Error {
		Common::Error lineError = runtime.playDialogueEntrySequence(kDadMeatPermissionConversionLines,
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
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x3a61, "DAD", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = runtime.playDialogueFst(kDialogueC011FstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		return runtime.playDialogueLineWithVariant(0x3a6b, "DAD", 0);
	}

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			return runtime.playDialogueLineWithVariant(0x3a39, "DAD", 0);
		}

		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			sharedState.discussedWhaleyHerrillPhoto = 1;
			return runtime.playDialogueEntrySequence(kDadWhaleyPhotoLines, ARRAYSIZE(kDadWhaleyPhotoLines));
		}

		if (isBlackmailEvidence(usedItemName)) {
			sharedState.discussedNoteCheckbookEvidence = 1;
			return runtime.playDialogueEntrySequence(kDadBlackmailEvidenceLines, ARRAYSIZE(kDadBlackmailEvidenceLines));
		}

		if (usedItemName.equalsIgnoreCase("MEAT_PERMISSION0")) {
			if (sharedState.dadMeatPermissionState != 0)
				return playDadMeatPermissionConversion();

			return runtime.playDialogueLineWithVariant(0x3a32, "DAD", 0);
		}

		return runtime.playDialogueLineWithVariant(0x3a32, "DAD", 0);
	}

	Common::Error lineError = Common::kNoError;
	if (state.introPending) {
		sharedState.dialogueStateD2ed0 = true;
		state.introPending = false;

		lineError = runtime.playDialogueEntrySequence(kDadIntroLines, ARRAYSIZE(kDadIntroLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		sharedState.dialogueStateD2f04 = true;
		lineError = runtime.playDialogueEntrySequence(kDadIntroContinuationLines, ARRAYSIZE(kDadIntroContinuationLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = runtime.playDialogueFst(kDialogueC112S7FstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = runtime.playDialogueEntrySequence(kDadOfferPromptLines, ARRAYSIZE(kDadOfferPromptLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		const Common::String responseMenu = getDadIntroResponseMenu(runtime);
		Common::Error responseError = runtime.runResponseMenuText(
			responseMenu, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = runtime.playDialogueEntrySequence(kDadOfferAcceptedLines, ARRAYSIZE(kDadOfferAcceptedLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			lineError = runtime.playDialogueLineWithVariant(0x39e0, "DAD", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		lineError = runtime.playDialogueEntrySequence(kDadOfferClosingLines, ARRAYSIZE(kDadOfferClosingLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		if (sharedState.dadMeatPermissionState != 0) {
			lineError = playDadMeatPermissionConversion();
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else {
			lineError = runtime.playDialogueEntrySequence(kDadMeatPermissionUnavailableLines,
				ARRAYSIZE(kDadMeatPermissionUnavailableLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			lineError = runtime.playDialogueFst(kDialogueC011FstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			lineError = runtime.playDialogueLineWithVariant(0x3a0f, "DAD", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	} else {
		lineError = runtime.playDialogueLineWithVariant(0x3a17, "DAD", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		if (sharedState.dadMeatPermissionState != 0) {
			lineError = playDadMeatPermissionConversion();
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
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
			lineError = runtime.playDialogueEntrySequence(kDadLodgeTopicLines, ARRAYSIZE(kDadLodgeTopicLines));
		} else {
			lineError = runtime.playDialogueLineWithVariant(0x3a2c, "DAD", 0);
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
}

} // End of namespace Harvester
