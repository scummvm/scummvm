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
	kParsonsPastorelliTopicLine,
	kParsonsBarberTopicLine,
	kParsonsMrPastorelliTopicLine
};

static const DialogueLineEntry kParsonsIntroLines[] = {
	{ 0xdcc, "PARSONS", 1 },
	{ 0xdd2, "SWELL", 0 },
	{ 0xdd6, "PARSONS", 0 },
	{ 0xde3, "PARSONS", 1 }
};

static const DialogueLineEntry kParsonsStephMidgameLines[] = {
	{ 0xeab, "PARSONS", 3 },
	{ 0xeb3, "SWELL", 0 },
	{ 0xeb7, "PARSONS", 1 }
};

static const DialogueLineEntry kParsonsScratchedTuckerLines[] = {
	{ 0xec2, "PARSONS", 2 },
	{ 0xec9, "PARSONS", 2 }
};

static const DialogueLineEntry kParsonsBurnedTvStationLines[] = {
	{ 0xee5, "PARSONS", 0 },
	{ 0xeea, "SWELL", 1 },
	{ 0xeef, "PARSONS", 0 },
	{ 0xef4, "SWELL", 3 }
};

static const DialogueLineEntry kParsonsKarinKidnapedLines[] = {
	{ 0xefa, "PARSONS", 3 },
	{ 0xf03, "SWELL", 3 }
};

static const DialogueLineEntry kParsonsBlackmailEvidenceLines[] = {
	{ 0xe95, "PARSONS", 0 },
	{ 0xe99, "PC", 0 },
	{ 0xe9d, "PARSONS", 1 }
};

} // End of anonymous namespace

Common::Error ParsonsDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			sharedState.discussedWhaleyHerrillPhoto = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
			return runtime.playDialogueLineWithVariant(0xe77, "PARSONS", 1);
		}

		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			sharedState.discussedCasketPhotoEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownPhotoOfCorpse, true);

			Common::Error lineError = runtime.playDialogueLineWithVariant(0xe7e, "PARSONS", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (!sharedState.dialogueStateD2ebc)
				return Common::kNoError;

			lineError = runtime.playDialogueLineWithVariant(0xe84, "PC", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			return runtime.playDialogueLineWithVariant(0xe88, "PARSONS", 0);
		}

		if ((usedItemName.equalsIgnoreCase("LEDGER") ||
					usedItemName.equalsIgnoreCase("LEDGER2")) &&
				runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
			sharedState.discussedLedgerEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownLedgersToAnyone, true);
			return runtime.playDialogueLineWithVariant(0xe8e, "PARSONS", 0);
		}

		if (usedItemName.equalsIgnoreCase("NOTE") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
			sharedState.discussedNoteCheckbookEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownEvidenceOfBlackmail, true);
			return runtime.playDialogueEntrySequence(
				kParsonsBlackmailEvidenceLines, ARRAYSIZE(kParsonsBlackmailEvidenceLines));
		}

		if (usedItemName.equalsIgnoreCase("TV_DEED") ||
				usedItemName.equalsIgnoreCase("TV_DEED_PHOTOCOPY")) {
			sharedState.discussedTvDeedEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownEvidenceSheriffOwns, true);
			return runtime.playDialogueLineWithVariant(0xea4, "PARSONS", 1);
		}

		return runtime.playDialogueLineWithVariant(0xe71, "PARSONS", 2);
	}

	Common::Error lineError = Common::kNoError;
	Common::String topicBuffer;
	int topicBufferLineIndex = -1;
	runtime.assignTopicBuffer(topicBuffer, topicBufferLineIndex, kParsonsIntroTopicLine, "Parsons topic buffer");

	if (_state.introPending) {
		_state.introPending = false;
		lineError = runtime.playDialogueEntrySequence(kParsonsIntroLines, ARRAYSIZE(kParsonsIntroLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	} else {
		lineError = runtime.playDialogueLineWithVariant(0xe65, "PARSONS", 2);
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
		lineError = runtime.playDialogueLineWithVariant(0xed2, "PARSONS", 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("DINER_BURNED") &&
			!_state.dinerBurnedShown) {
		_state.dinerBurnedShown = true;
		lineError = runtime.playDialogueLineWithVariant(0xedd, "PARSONS", 2);
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
		runtime.setActiveSpeakerPortrait("PARSONS", 0);
	}

	if (runtime.startupScript().getFlagValue("KARIN_KIDNAPED") &&
			!_state.karinKidnapedShown) {
		_state.karinKidnapedShown = true;
		lineError = runtime.playDialogueEntrySequence(
			kParsonsKarinKidnapedLines, ARRAYSIZE(kParsonsKarinKidnapedLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		runtime.setActiveSpeakerPortrait("PARSONS", 0);
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
			return runtime.playDialogueLineWithVariant(0xe5f, "PARSONS", 0);
		}

		if (runtime.matchesResponseLine(selectedTopic, kParsonsPoleTopicLine)) {
			lineError = runtime.playDialogueLineWithVariant(0xdee, "PARSONS", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC093FstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0xdf9, "PARSONS", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0xe00, "SWELL", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0xe04, "PARSONS", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			runtime.assignTopicBuffer(topicBuffer, topicBufferLineIndex, kParsonsPoleFollowupTopicLine, "Parsons topic buffer");
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, kParsonsAlarmSystemTopicLine)) {
			lineError = runtime.playDialogueLineWithVariant(0xe0f, "PARSONS", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			runtime.assignTopicBuffer(topicBuffer, topicBufferLineIndex, kParsonsAlarmSystemFollowupTopicLine, "Parsons topic buffer");
			continue;
		}

		if (runtime.matchesAnyResponseLine(selectedTopic, kParsonsAliensTopicResponseLines,
				ARRAYSIZE(kParsonsAliensTopicResponseLines))) {
			lineError = runtime.playDialogueLineWithVariant(0xe54, "PARSONS", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			runtime.assignTopicBuffer(topicBuffer, topicBufferLineIndex, kParsonsAliensFollowupTopicLine, "Parsons topic buffer");
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, kParsonsAliensTopicLine)) {
			lineError = runtime.playDialogueLineWithVariant(0xe1c, "PARSONS", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0xe24, "PC", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0xe28, "PARSONS", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC094FstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0xe33, "PARSONS", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0xe39, "SWELL", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0xe3e, "PARSONS", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0xe46, "PARSONS", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0xe4b, "PARSONS", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}

		lineError = runtime.playDialogueLineWithVariant(0xe6b, "PARSONS", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
}

} // End of namespace Harvester
