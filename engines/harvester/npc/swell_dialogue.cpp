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


#include "harvester/npc/swell_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kSwellNpc = "SWELL";
static const char *const kPcSpeaker = "PC";
static const int kSwellIntroTopicBufferLine = 0x21d;
static const int kSwellExitTopicLine = 0x21e;
static const int kSwellTopicBranchAResponseLines[] = { 0x21f, 0x220 };
static const int kSwellTopicBranchATopicBufferLine = 0x221;
static const int kSwellTopicBranchBResponseLine = 0x222;
static const int kSwellTopicBranchBTopicBufferLine = 0x223;
static const int kSwellTopicBranchCResponseLines[] = { 0x224, 0x225, 0x226 };
static const int kSwellTopicBranchCTopicBufferLine = 0x227;
static const int kSwellTopicNoReplyResponseLine = 0x229;

static const DialogueLineEntry kSwellIntroLines[] = {
	{ 0xf61, kSwellNpc, 1 },
	{ 0xf65, kPcSpeaker, 0 },
	{ 0xf69, kSwellNpc, 1 },
	{ 0x3d50, kPcSpeaker, 0 },
	{ 0xf72, kSwellNpc, 1 },
	{ 0xf7f, kSwellNpc, 0 }
};

static const DialogueLineEntry kSwellLedgerLines[] = {
	{ 0xfd4, kSwellNpc, 0 },
	{ 0xfd9, kPcSpeaker, 0 },
	{ 0xfde, kSwellNpc, 0 }
};

static const DialogueLineEntry kSwellStephMidgameLines[] = {
	{ 0xff7, kSwellNpc, 3 },
	{ 0xffc, kPcSpeaker, 0 },
	{ 0x1001, kSwellNpc, 3 }
};

static const DialogueLineEntry kSwellBarberPoleStolenLines[] = {
	{ 0x100b, kSwellNpc, 3 },
	{ 0x100c, kSwellNpc, 0 }
};

} // End of anonymous namespace

bool SwellDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase(kSwellNpc);
}

Common::Error SwellDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	auto playSwellLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, kSwellNpc, headVariant);
	};
	auto playSequence = [&](const DialogueLineEntry *lines, uint count) -> Common::Error {
		return runtime.playDialogueEntrySequence(lines, count);
	};
	auto assignSwellTopicBuffer = [&](Common::String &topicBuffer, int &topicBufferLineIndex,
			int responseLineIndex) {
		runtime.assignTopicBuffer(topicBuffer, topicBufferLineIndex, responseLineIndex,
			"Swell topic buffer");
	};

	if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
		sharedState.discussedWhaleyHerrillPhoto = 1;
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
		return playSwellLine(0xfc6, 1);
	}
	if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
			usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
		sharedState.discussedCasketPhotoEvidence = 1;
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
		return playSwellLine(0xfcd);
	}
	if ((usedItemName.equalsIgnoreCase("LEDGER") ||
				usedItemName.equalsIgnoreCase("LEDGER2")) &&
			runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
		sharedState.discussedLedgerEvidence = 1;
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownLedgersToAnyone, true);
		return playSequence(kSwellLedgerLines, ARRAYSIZE(kSwellLedgerLines));
	}
	if (usedItemName.equalsIgnoreCase("NOTE") ||
			usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
		sharedState.discussedNoteCheckbookEvidence = 1;
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
		return playSwellLine(0xfe6, 1);
	}
	if (!usedItemName.empty()) {
		return playSwellLine(0xfc0);
	}

	Common::Error lineError = Common::kNoError;
	Common::String topicBuffer;
	int topicBufferLineIndex = -1;

	if (_state.talkStatePending) {
		_state.talkStatePending = false;
		lineError = playSequence(kSwellIntroLines, ARRAYSIZE(kSwellIntroLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		assignSwellTopicBuffer(topicBuffer, topicBufferLineIndex, kSwellIntroTopicBufferLine);
	} else {
		lineError = playSwellLine(0xfb3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") && !_state.stephMidgameShown) {
		_state.stephMidgameShown = true;
		lineError = playSequence(kSwellStephMidgameLines, ARRAYSIZE(kSwellStephMidgameLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("BARBER_POLE_STOLEN") &&
			!_state.barberPoleStolenShown) {
		_state.barberPoleStolenShown = true;
		lineError = playSequence(kSwellBarberPoleStolenLines, ARRAYSIZE(kSwellBarberPoleStolenLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("BOLT_OF_CLOTH_TAKEN") &&
			!_state.boltOfClothTakenShown) {
		_state.boltOfClothTakenShown = true;
		lineError = playSwellLine(0x1017, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("BURNED_TV_STATION") &&
			!_state.burnedTvStationShown) {
		_state.burnedTvStationShown = true;
		lineError = playSwellLine(0x1021, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("KARIN_KIDNAPED") && !_state.karinKidnapedShown) {
		_state.karinKidnapedShown = true;
		lineError = playSwellLine(0x1029, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	for (;;) {
		Common::String selectedTopic;
		Common::Error menuError = runtime.runKeywordMenu(topicBuffer, topicBufferLineIndex, selectedTopic);
		if (menuError.getCode() != Common::kNoError)
			return menuError;
		if (selectedTopic.empty())
			return Common::kNoError;

		if (runtime.matchesResponseLine(selectedTopic, kSwellExitTopicLine))
			return playSwellLine(0xff1, 1);

		if (runtime.matchesAnyResponseLine(selectedTopic, kSwellTopicBranchAResponseLines,
					ARRAYSIZE(kSwellTopicBranchAResponseLines))) {
			lineError = playSwellLine(0xf88, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignSwellTopicBuffer(topicBuffer, topicBufferLineIndex, kSwellTopicBranchATopicBufferLine);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, kSwellTopicBranchBResponseLine)) {
			lineError = playSwellLine(0xf95);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignSwellTopicBuffer(topicBuffer, topicBufferLineIndex, kSwellTopicBranchBTopicBufferLine);
			continue;
		}

		if (runtime.matchesAnyResponseLine(selectedTopic, kSwellTopicBranchCResponseLines,
					ARRAYSIZE(kSwellTopicBranchCResponseLines))) {
			lineError = playSwellLine(0xfa0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignSwellTopicBuffer(topicBuffer, topicBufferLineIndex, kSwellTopicBranchCTopicBufferLine);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x228)) {
			lineError = playSwellLine(0xfaa, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, kSwellTopicNoReplyResponseLine))
			continue;

		lineError = playSwellLine(0xfba, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
}

} // End of namespace Harvester
