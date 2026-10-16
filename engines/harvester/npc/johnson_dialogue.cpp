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


#include "harvester/npc/johnson_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {


static const int kJohnsonInitialTopicBufferLine = 0x176;
static const int kJohnsonResponseTopicBufferLine = 0x17b;
static const int kJohnsonStephTopicBufferLine = 0x17d;
static const int kJohnsonKarinAliveTopicBufferLine = 0x17f;
static const int kJohnsonResponseMenuLine = 0x17a;
static const int kJohnsonDirectExitTopicLines[] = { 0x177 };
static const int kJohnsonIntroductionTopicLines[] = { 0x178, 0x179 };
static const int kJohnsonGenericReplyTopicLines[] = { 0x180, 0x181, 0x182 };
static const int kJohnsonSecondGenericReplyTopicLines[] = { 0x184, 0x185 };

static const DialogueLineEntry kJohnsonIntroLines[] = {
	{ 0xa86, "JOHNSON", 1 },
	{ 0xa8a, "PC", 0 },
	{ 0xa8e, "JOHNSON", 1 },
	{ 0xa97, "PC", 0 },
	{ 0xa9b, "JOHNSON", 0 }
};

static const DialogueLineEntry kJohnsonWhaleyPhotoLines[] = {
	{ 0xb75, "JOHNSON", 1 },
	{ 0xb7b, "PC", 1 },
	{ 0xb80, "JOHNSON", 1 }
};

static const DialogueLineEntry kJohnsonLedgerLines[] = {
	{ 0xba0, "JOHNSON", 1 },
	{ 0xba4, "PC", 0 },
	{ 0xba8, "JOHNSON", 1 }
};

static const DialogueLineEntry kJohnsonScratchedTuckerLines[] = {
	{ 0xc25, "JOHNSON", 2 },
	{ 0xc2b, "JOHNSON", 3 },
	{ 0xc30, "JOHNSON", 2 }
};

static const DialogueLineEntry kJohnsonBoltOfClothLines[] = {
	{ 0xc5f, "JOHNSON", 3 },
	{ 0xc69, "JOHNSON", 3 }
};

static const DialogueLineEntry kJohnsonStephMidgameLines[] = {
	{ 0xbf0, "JOHNSON", 3 },
	{ 0xbf1, "JOHNSON", 3 },
	{ 0xbf2, "JOHNSON", 3 }
};

static const DialogueLineEntry kJohnsonKarinKidnapedLines[] = {
	{ 0xbff, "JOHNSON", 0 },
	{ 0xc06, "JOHNSON", 0 }
};

static const DialogueLineEntry kJohnsonBarberPoleStolenLines[] = {
	{ 0xc46, "JOHNSON", 3 },
	{ 0xc4d, "JOHNSON", 3 },
	{ 0xc59, "JOHNSON", 3 }
};

} // End of namespace

Common::Error JohnsonDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	JohnsonRoomDialogueState &state = _state;
	Common::String &johnsonTopicBuffer = state.currentTopicBuffer;
	int &johnsonTopicBufferLineIndex = state.currentTopicBufferLineIndex;
	state.currentTopicBuffer.clear();
	state.currentTopicBufferLineIndex = -1;

	DialogueTopicBuffer topics(runtime, johnsonTopicBuffer, johnsonTopicBufferLineIndex, "Johnson topic buffer");

	if (runtime.startupScript().getFlagValue("2ND_SCRATCH_TUCKER"))
		return runtime.playDialogueLineWithVariant(0xbe6, "JOHNSON", 2);
	if (runtime.startupScript().getFlagValue("TRY_TO_SCRATCH_TUCKER") &&
			!state.tryToScratchTuckerShown) {
		state.tryToScratchTuckerShown = true;
		return runtime.playDialogueLineWithVariant(0xbe0, "JOHNSON", 2);
	}
	if (runtime.startupScript().getFlagValue("OPENING_MANHOLE_DAYTIME"))
	{
		Common::Error lineError = runtime.playDialogueLineWithVariant(0xbe0, "JOHNSON", 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		(void)runtime.startupScript().setRuntimeFlagValue("OPENING_MANHOLE_DAYTIME", false);
		return Common::kNoError;
	}

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
			sharedState.discussedWhaleyHerrillPhoto = 1;
			return runtime.playDialogueEntrySequence(
				kJohnsonWhaleyPhotoLines, ARRAYSIZE(kJohnsonWhaleyPhotoLines));
		}
		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
			sharedState.discussedCasketPhotoEvidence = 1;
			Common::Error lineError = runtime.playDialogueLineWithVariant(0xb89, "JOHNSON", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (!sharedState.dialogueStateD2ebc)
				return Common::kNoError;

			const DialogueLineEntry followupLines[] = {
				{ 0xb90, "PC", 0 },
				{ 0xb94, "JOHNSON", 0 },
				{ 0xb99, "JOHNSON", 0 }
			};
			return runtime.playDialogueEntrySequence(followupLines, ARRAYSIZE(followupLines));
		}
		if ((usedItemName.equalsIgnoreCase("LEDGER") ||
					usedItemName.equalsIgnoreCase("LEDGER2")) &&
				runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownLedgersToAnyone, true);
			sharedState.discussedLedgerEvidence = 1;
			return runtime.playDialogueEntrySequence(
				kJohnsonLedgerLines, ARRAYSIZE(kJohnsonLedgerLines));
		}
		if (usedItemName.equalsIgnoreCase("NOTE") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
			sharedState.discussedNoteCheckbookEvidence = 1;
			return runtime.playDialogueLineWithVariant(0xbb1, "JOHNSON", 1);
		}
		if (usedItemName.equalsIgnoreCase("TV_DEED") ||
				usedItemName.equalsIgnoreCase("TV_DEED_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceSheriffOwns, true);
			sharedState.discussedTvDeedEvidence = 1;
			return runtime.playDialogueLineWithVariant(0xbce, "JOHNSON", 1);
		}

		return runtime.playDialogueLineWithVariant(0xb6f, "JOHNSON", 0);
	}

	if (runtime.startupScript().getFlagValue("SCRATCHED_TUCKER") &&
			!state.scratchedTuckerReintroShown) {
		state.scratchedTuckerReintroShown = true;
		state.introPending = true;
		Common::Error lineError = runtime.playDialogueEntrySequence(
			kJohnsonScratchedTuckerLines, ARRAYSIZE(kJohnsonScratchedTuckerLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		if (runtime.startupScript().getFlagValue("BOLT_OF_CLOTH_TAKEN") &&
				!state.boltOfClothTakenShown) {
			state.boltOfClothTakenShown = true;
			lineError = runtime.playDialogueEntrySequence(
				kJohnsonBoltOfClothLines, ARRAYSIZE(kJohnsonBoltOfClothLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	} else {
		if (state.introPending) {
			state.introPending = false;
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kJohnsonIntroLines, ARRAYSIZE(kJohnsonIntroLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			sharedState.dialogueStateD2f04 = true;
			topics.assign(kJohnsonInitialTopicBufferLine);
		} else {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0xb62, "JOHNSON", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}

	if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") &&
			!state.stephMidgamePlayedShown) {
		state.stephMidgamePlayedShown = true;
		Common::Error lineError = runtime.playDialogueEntrySequence(
			kJohnsonStephMidgameLines, ARRAYSIZE(kJohnsonStephMidgameLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("KARIN_KIDNAPED") &&
			!state.karinKidnapedShown) {
		state.karinKidnapedShown = true;
		sharedState.karinKidnapedDialogueState = true;
		Common::Error lineError = runtime.playDialogueEntrySequence(
			kJohnsonKarinKidnapedLines, ARRAYSIZE(kJohnsonKarinKidnapedLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("KARIN_FOUND_ALIVE") &&
			!state.karinFoundAliveShown) {
		state.karinFoundAliveShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0xc13, "JOHNSON", 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueLineWithVariant(0xc17, "JOHNSON", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("BARBER_POLE_STOLEN") &&
			!state.barberPoleStolenShown) {
		state.barberPoleStolenShown = true;
		Common::Error lineError = runtime.playDialogueEntrySequence(
			kJohnsonBarberPoleStolenLines, ARRAYSIZE(kJohnsonBarberPoleStolenLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("DINER_BURNED") &&
			!state.dinerBurnedShown) {
		state.dinerBurnedShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0xc6f, "JOHNSON", 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("PC_ESCAPED_JAIL") &&
			!state.escapedJailShown) {
		state.escapedJailShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0xc77, "JOHNSON", 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("GOT_REMAINS_FOR_LODGE") &&
			!state.gotRemainsForLodgeShown) {
		state.gotRemainsForLodgeShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0xc7f, "JOHNSON", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("BURNED_TV_STATION") &&
			!state.burnedTvStationShown) {
		state.burnedTvStationShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0xc8f, "JOHNSON", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	for (;;) {
		Common::String selectedTopic;
		Common::Error menuError = runtime.runKeywordMenu(
			johnsonTopicBuffer, johnsonTopicBufferLineIndex, selectedTopic);
		if (menuError.getCode() != Common::kNoError)
			return menuError;
		if (selectedTopic.empty())
			return Common::kNoError;
		if (runtime.matchesAnyResponseLine(selectedTopic, kJohnsonDirectExitTopicLines,
					ARRAYSIZE(kJohnsonDirectExitTopicLines))) {
			return runtime.playDialogueLineWithVariant(0xbda, "JOHNSON", 1);
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kJohnsonIntroductionTopicLines,
				ARRAYSIZE(kJohnsonIntroductionTopicLines))) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0xaa6, "JOHNSON", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(
				kJohnsonResponseMenuLine, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = runtime.playDialogueLineWithVariant(0xab4, "JOHNSON", 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else if (responseIndex == 2) {
				lineError = runtime.playDialogueLineWithVariant(0xab9, "JOHNSON", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			const DialogueLineEntry followupLines[] = {
				{ 0xabd, "JOHNSON", 0 },
				{ 0xac3, "PC", 0 },
				{ 0xac7, "JOHNSON", 2 }
			};
			lineError = runtime.playDialogueEntrySequence(followupLines, ARRAYSIZE(followupLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			topics.assign(kJohnsonResponseTopicBufferLine);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x17c)) {
			const DialogueLineEntry lines[] = {
				{ 0xb17, "JOHNSON", 0 },
				{ 0xb1c, "PC", 0 },
				{ 0xb20, "JOHNSON", 1 }
			};
			Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			topics.assign(kJohnsonStephTopicBufferLine);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x17e)) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0xb2d, "JOHNSON", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			topics.assign(kJohnsonKarinAliveTopicBufferLine);
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kJohnsonGenericReplyTopicLines,
				ARRAYSIZE(kJohnsonGenericReplyTopicLines))) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0xb3a, "JOHNSON", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			topics.assign(0x183);
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kJohnsonSecondGenericReplyTopicLines,
				ARRAYSIZE(kJohnsonSecondGenericReplyTopicLines))) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0xb46, "JOHNSON", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			topics.assign(0x186);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x187)) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0xb52, "PC", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0xb56, "JOHNSON", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			topics.assign(0x188);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x189))
			continue;

		Common::Error lineError = runtime.playDialogueLineWithVariant(0xb69, "JOHNSON", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		return runtime.playDialogueLineWithVariant(0xbda, "JOHNSON", 1);
	}
}

} // End of namespace Harvester
