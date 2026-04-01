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


#include "harvester/npc/parsons_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kParsonsSpeakerId = "PARSONS";
static const char *const kSwellSpeakerId = "SWELL";
static const char *const kPcSpeakerId = "PC";
static const char *const kDialogueC093FstPath = "GRAPHIC/FST/C093.FST";
static const char *const kDialogueC094FstPath = "GRAPHIC/FST/C094.FST";

static const int kParsonsIntroTopicLine = 0x1c5;
static const int kParsonsExitTopicLine = 0x1c6;
static const int kParsonsPoleTopicLine = 0x1c7;
static const int kParsonsPoleFollowupTopicLine = 0x1c8;
static const int kParsonsAlarmSystemTopicLine = 0x1c9;
static const int kParsonsAlarmSystemFollowupTopicLine = 0x1ca;
static const int kParsonsAliensTopicLine = 0x1cb;
static const int kParsonsPastorelliTopicLine = 0x1cd;
static const int kParsonsBarberTopicLine = 0x1ce;
static const int kParsonsMrPastorelliTopicLine = 0x1cf;
static const int kParsonsAliensFollowupTopicLine = 0x1d0;
static const int kParsonsHiddenExitTopicLine = 0x1d1;

static const int kParsonsAliensTopicResponseLines[] = {
	kParsonsAliensTopicLine,
	kParsonsPastorelliTopicLine,
	kParsonsBarberTopicLine,
	kParsonsMrPastorelliTopicLine
};

static const DialogueLineEntry kParsonsIntroLines[] = {
	{ 0xdcc, kParsonsSpeakerId, 1 },
	{ 0xdd2, kSwellSpeakerId, 0 },
	{ 0xdd6, kParsonsSpeakerId, 0 },
	{ 0xde3, kParsonsSpeakerId, 1 }
};

static const DialogueLineEntry kParsonsStephMidgameLines[] = {
	{ 0xeab, kParsonsSpeakerId, 3 },
	{ 0xeb3, kSwellSpeakerId, 0 },
	{ 0xeb7, kParsonsSpeakerId, 1 }
};

static const DialogueLineEntry kParsonsScratchedTuckerLines[] = {
	{ 0xec2, kParsonsSpeakerId, 2 },
	{ 0xec9, kParsonsSpeakerId, 2 }
};

static const DialogueLineEntry kParsonsBurnedTvStationLines[] = {
	{ 0xee5, kParsonsSpeakerId, 0 },
	{ 0xeea, kSwellSpeakerId, 1 },
	{ 0xeef, kParsonsSpeakerId, 0 },
	{ 0xef4, kSwellSpeakerId, 3 }
};

static const DialogueLineEntry kParsonsKarinKidnapedLines[] = {
	{ 0xefa, kParsonsSpeakerId, 3 },
	{ 0xf03, kSwellSpeakerId, 3 }
};

static const DialogueLineEntry kParsonsBlackmailEvidenceLines[] = {
	{ 0xe95, kParsonsSpeakerId, 0 },
	{ 0xe99, kPcSpeakerId, 0 },
	{ 0xe9d, kParsonsSpeakerId, 1 }
};

} // End of anonymous namespace

bool ParsonsDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase(kParsonsSpeakerId);
}

Common::Error ParsonsDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	auto playParsonsLine = [&](int wavId, int headVariant) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, kParsonsSpeakerId, headVariant);
	};
	auto playSwellLine = [&](int wavId, int headVariant) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, kSwellSpeakerId, headVariant);
	};
	auto playPcLine = [&](int wavId, int headVariant) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, kPcSpeakerId, headVariant);
	};
	auto assignParsonsTopicBuffer = [&](Common::String &topicBuffer, int &topicBufferLineIndex,
			int responseLineIndex) {
		runtime.assignTopicBuffer(topicBuffer, topicBufferLineIndex,
			responseLineIndex, "Parsons topic buffer");
	};

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
			return playParsonsLine(0xe77, 1);
		}

		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownPhotoOfCorpse, true);

			Common::Error lineError = playParsonsLine(0xe7e, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (!sharedState.dialogueStateD2ebc)
				return Common::kNoError;

			lineError = playPcLine(0xe84, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			return playParsonsLine(0xe88, 0);
		}

		if ((usedItemName.equalsIgnoreCase("LEDGER") ||
					usedItemName.equalsIgnoreCase("LEDGER2")) &&
				runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownLedgersToAnyone, true);
			return playParsonsLine(0xe8e, 0);
		}

		if (usedItemName.equalsIgnoreCase("NOTE") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownEvidenceOfBlackmail, true);
			return runtime.playDialogueEntrySequence(
				kParsonsBlackmailEvidenceLines, ARRAYSIZE(kParsonsBlackmailEvidenceLines));
		}

		if (usedItemName.equalsIgnoreCase("TV_DEED") ||
				usedItemName.equalsIgnoreCase("TV_DEED_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownEvidenceSheriffOwns, true);
			return playParsonsLine(0xea4, 1);
		}

		return playParsonsLine(0xe71, 2);
	}

	Common::Error lineError = Common::kNoError;
	Common::String topicBuffer;
	int topicBufferLineIndex = -1;
	assignParsonsTopicBuffer(topicBuffer, topicBufferLineIndex, kParsonsIntroTopicLine);

	if (_state.introPending) {
		_state.introPending = false;
		lineError = runtime.playDialogueEntrySequence(kParsonsIntroLines, ARRAYSIZE(kParsonsIntroLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	} else {
		lineError = playParsonsLine(0xe65, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") &&
			!_state.stephMidgameShown) {
		_state.stephMidgameShown = true;
		lineError = runtime.playDialogueEntrySequence(
			kParsonsStephMidgameLines, ARRAYSIZE(kParsonsStephMidgameLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("SCRATCHED_TUCKER") &&
			!_state.scratchedTuckerShown) {
		_state.scratchedTuckerShown = true;
		lineError = runtime.playDialogueEntrySequence(
			kParsonsScratchedTuckerLines, ARRAYSIZE(kParsonsScratchedTuckerLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("BARBER_POLE_STOLEN") &&
			!_state.barberPoleStolenShown) {
		_state.barberPoleStolenShown = true;
		lineError = playParsonsLine(0xed2, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("DINER_BURNED") &&
			!_state.dinerBurnedShown) {
		_state.dinerBurnedShown = true;
		lineError = playParsonsLine(0xedd, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("BURNED_TV_STATION") &&
			!_state.burnedTvStationShown) {
		_state.burnedTvStationShown = true;
		lineError = runtime.playDialogueEntrySequence(
			kParsonsBurnedTvStationLines, ARRAYSIZE(kParsonsBurnedTvStationLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		runtime.setActiveSpeakerPortrait(kParsonsSpeakerId, 0);
	}

	if (runtime.startupScript().getFlagValue("KARIN_KIDNAPED") &&
			!_state.karinKidnapedShown) {
		_state.karinKidnapedShown = true;
		lineError = runtime.playDialogueEntrySequence(
			kParsonsKarinKidnapedLines, ARRAYSIZE(kParsonsKarinKidnapedLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		runtime.setActiveSpeakerPortrait(kParsonsSpeakerId, 0);
	}

	for (;;) {
		Common::String selectedTopic;
		Common::Error menuError = runtime.runKeywordMenu(topicBuffer, topicBufferLineIndex, selectedTopic);
		if (menuError.getCode() != Common::kNoError)
			return menuError;
		if (selectedTopic.empty())
			return Common::kNoError;

		if (selectedTopic.equalsIgnoreCase(runtime.genericByeTopic()) ||
				runtime.matchesResponseLine(selectedTopic, kParsonsExitTopicLine) ||
				runtime.matchesResponseLine(selectedTopic, kParsonsHiddenExitTopicLine)) {
			return playParsonsLine(0xe5f, 0);
		}

		if (runtime.matchesResponseLine(selectedTopic, kParsonsPoleTopicLine)) {
			lineError = playParsonsLine(0xdee, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC093FstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playParsonsLine(0xdf9, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playSwellLine(0xe00, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playParsonsLine(0xe04, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignParsonsTopicBuffer(topicBuffer, topicBufferLineIndex, kParsonsPoleFollowupTopicLine);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, kParsonsAlarmSystemTopicLine)) {
			lineError = playParsonsLine(0xe0f, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignParsonsTopicBuffer(topicBuffer, topicBufferLineIndex,
				kParsonsAlarmSystemFollowupTopicLine);
			continue;
		}

		if (runtime.matchesAnyResponseLine(selectedTopic, kParsonsAliensTopicResponseLines,
					ARRAYSIZE(kParsonsAliensTopicResponseLines))) {
			lineError = playParsonsLine(0xe1c, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playPcLine(0xe24, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playParsonsLine(0xe28, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC094FstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playParsonsLine(0xe33, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playSwellLine(0xe39, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playParsonsLine(0xe3e, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playParsonsLine(0xe46, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playParsonsLine(0xe4b, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignParsonsTopicBuffer(topicBuffer, topicBufferLineIndex, kParsonsAliensFollowupTopicLine);
			continue;
		}

		lineError = playParsonsLine(0xe6b, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
}

} // End of namespace Harvester
