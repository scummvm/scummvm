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


#include "harvester/npc/moynahan_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"
#include "harvester/text.h"

namespace Harvester {

namespace {

static const char *const kMoynahanNpc = "MOYNAHAN";
static const char *const kMoynahanEmbalmingPortrait = "MOYNAHAN_MBLM";
static const char *const kPcSpeaker = "PC";
static const char *const kMoynahanHotelRoom = "HTL-RT";
static const char *const kMoynahanEmbalmingRoom = "MBLM";
static const char *const kRahRoom = "RAH";
static const char *const kGlueObject = "GLUE";
static const char *const kJustGotGlueFlag = "JUST_GOT_GLUE";

static const int kMoynahanGoodCauseTopicLines[] = { 0x19a, 0x19b };
static const int kMoynahanSlashMarksTopicLines[] = { 0x19d, 0x19e, 0x19f, 0x1a0 };
static const int kMoynahanBodyTopicLines[] = { 0x1a2, 0x1a3 };
static const int kMoynahanGeneralTopicLines[] = { 0x1a9, 0x1aa, 0x1ab };
static const int kMoynahanDayFiveUnlockTopicLines[] = { 0x1b2, 0x1b3 };
static const int kMoynahanEmbalmingRoomTopicLines[] = { 0x1b4, 0x1b5 };
static const int kMoynahanLateTopicLines[] = { 0x1b8, 0x1b9, 0x1ba, 0x1bb, 0x1bc, 0x1bd, 0x1be };
static const int kMoynahanGlueTopicLines[] = { 0x1bf, 0x1c0, 0x1c1, 0x1c2 };

static const DialogueLineEntry kMoynahanGlueInterruptOpeningLines[] = {
	{ 0x3e4d, kMoynahanNpc, 2 },
	{ 0x3e52, kPcSpeaker, 0 },
	{ 0x3e56, kMoynahanNpc, 0 }
};

static const DialogueLineEntry kMoynahanGlueInterruptShortReplyLines[] = {
	{ 0x3e76, kPcSpeaker, 0 },
	{ 0x3e7a, kMoynahanNpc, 0 }
};

static const DialogueLineEntry kMoynahanGlueInterruptExtendedReplyLines[] = {
	{ 0x3e5b, kPcSpeaker, 0 },
	{ 0x3e60, kMoynahanNpc, 0 },
	{ 0x3e64, kPcSpeaker, 2 },
	{ 0x3e69, kMoynahanNpc, 0 },
	{ 0x3e6d, kPcSpeaker, 2 },
	{ 0x3e71, kMoynahanNpc, 2 }
};

static const DialogueLineEntry kMoynahanIntroLines[] = {
	{ 0x3d33, kMoynahanNpc, 0 },
	{ 0x3d37, kPcSpeaker, 0 },
	{ 0x3d3b, kMoynahanNpc, 0 },
	{ 0x3d40, kPcSpeaker, 0 },
	{ 0x3d45, kMoynahanNpc, 0 },
	{ 0x3d47, kMoynahanNpc, 0 }
};

static const DialogueLineEntry kMoynahanSlashMarksIntroLines[] = {
	{ 0x3da0, kMoynahanNpc, 3 },
	{ 0x3da1, kMoynahanNpc, 3 },
	{ 0x3da2, kMoynahanNpc, 2 }
};

static const DialogueLineEntry kMoynahanDayFiveLines[] = {
	{ 0x3fe5, kMoynahanNpc, 1 },
	{ 0x3fea, kPcSpeaker, 0 },
	{ 0x3fee, kMoynahanNpc, 2 },
	{ 0x3ff3, kPcSpeaker, 0 },
	{ 0x3ff7, kMoynahanNpc, 0 }
};

static const DialogueLineEntry kMoynahanKarinKidnapedTailLines[] = {
	{ 0x4020, kMoynahanNpc, 0 },
	{ 0x4026, kPcSpeaker, 0 },
	{ 0x402a, kMoynahanNpc, 0 },
	{ 0x4034, kPcSpeaker, 0 },
	{ 0x4038, kMoynahanNpc, 0 },
	{ 0x403d, kPcSpeaker, 0 },
	{ 0x4041, kMoynahanNpc, 1 }
};

static const DialogueLineEntry kMoynahanCasketIntroLines[] = {
	{ 0x3e84, kMoynahanNpc, 0 },
	{ 0x3e89, kPcSpeaker, 0 },
	{ 0x3e8e, kMoynahanNpc, 0 },
	{ 0x3e93, kPcSpeaker, 0 },
	{ 0x3e98, kMoynahanNpc, 2 }
};

static const DialogueLineEntry kMoynahanCasketResponseOneLines[] = {
	{ 0x3ea4, kMoynahanNpc, 0 },
	{ 0x3ea8, kPcSpeaker, 2 },
	{ 0x3ead, kMoynahanNpc, 2 }
};

static const DialogueLineEntry kMoynahanCasketFollowupLines[] = {
	{ 0x3eb7, kPcSpeaker, 2 },
	{ 0x3ebc, kMoynahanNpc, 2 }
};

static const DialogueLineEntry kMoynahanLedgerIntroLines[] = {
	{ 0x3ede, kPcSpeaker, 0 },
	{ 0x3ee3, kMoynahanNpc, 0 },
	{ 0x3ee7, kPcSpeaker, 0 },
	{ 0x3eec, kMoynahanNpc, 0 }
};

static const DialogueLineEntry kMoynahanLedgerResponseOneLines[] = {
	{ 0x3ef7, kMoynahanNpc, 0 },
	{ 0x3efb, kPcSpeaker, 0 },
	{ 0x3eff, kMoynahanNpc, 2 },
	{ 0x3f05, kPcSpeaker, 2 }
};

static const DialogueLineEntry kMoynahanLedgerResponseTwoLines[] = {
	{ 0x3f09, kMoynahanNpc, 0 },
	{ 0x3f0f, kPcSpeaker, 2 },
	{ 0x3f13, kMoynahanNpc, 0 },
	{ 0x3f18, kPcSpeaker, 2 }
};

static const DialogueLineEntry kMoynahanMatchesLines[] = {
	{ 0x4049, kPcSpeaker, 0 },
	{ 0x404d, kMoynahanNpc, 0 }
};

static const DialogueLineEntry kMoynahanGoodCauseLines[] = {
	{ 0x3d5f, kMoynahanNpc, 0 },
	{ 0x3d66, kPcSpeaker, 0 },
	{ 0x3d6a, kMoynahanNpc, 0 }
};

static const DialogueLineEntry kMoynahanSlashMarksTopicReplyLines[] = {
	{ 0x3d76, kMoynahanNpc, 0 },
	{ 0x3d7d, kPcSpeaker, 0 },
	{ 0x3d81, kMoynahanNpc, 0 },
	{ 0x3d88, kPcSpeaker, 0 },
	{ 0x3d8c, kMoynahanNpc, 0 },
	{ 0x3d92, kPcSpeaker, 0 },
	{ 0x3d96, kMoynahanNpc, 0 }
};

static const DialogueLineEntry kMoynahanGeneralTopicResponseOneLines[] = {
	{ 0x3dbe, kMoynahanNpc, 1 },
	{ 0x3dc4, kPcSpeaker, 0 },
	{ 0x3dc8, kMoynahanNpc, 2 }
};

static const DialogueLineEntry kMoynahanGeneralTopicResponseTwoLines[] = {
	{ 0x3dcf, kMoynahanNpc, 0 },
	{ 0x3dd4, kPcSpeaker, 0 },
	{ 0x3dd8, kMoynahanNpc, 1 },
	{ 0x3ddd, kPcSpeaker, 0 },
	{ 0x3de2, kMoynahanNpc, 2 }
};

static const DialogueLineEntry kMoynahanDayFiveUnlockLines[] = {
	{ 0x3df5, kMoynahanNpc, 3 },
	{ 0x3dfc, kPcSpeaker, 0 },
	{ 0x3e00, kMoynahanNpc, 0 },
	{ 0x3e05, kPcSpeaker, 0 },
	{ 0x3e0a, kMoynahanNpc, 0 }
};

static const DialogueLineEntry kMoynahanEmbalmingRoomLines[] = {
	{ 0x3e18, kMoynahanNpc, 0 },
	{ 0x3e22, kPcSpeaker, 0 },
	{ 0x3e26, kMoynahanNpc, 2 }
};

static Common::Error playDialogueSequence(DialogueRuntime &runtime,
		const DialogueLineEntry *lines, uint count) {
	return runtime.playDialogueEntrySequence(lines, count);
}

} // End of namespace

bool MoynahanDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase(kMoynahanNpc) ||
		npcName.equalsIgnoreCase(kMoynahanEmbalmingPortrait);
}

Common::Error MoynahanDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	MoynahanRoomDialogueState &state = _state;
	Common::String topicBuffer;
	int topicBufferLineIndex = -1;

	auto assignTopicBuffer = [&](int responseLineIndex) {
		runtime.assignTopicBuffer(topicBuffer, topicBufferLineIndex,
			responseLineIndex, "Moynahan topic buffer");
	};
	auto appendTopicBuffer = [&](int responseLineIndex) {
		if (topicBuffer.empty()) {
			assignTopicBuffer(responseLineIndex);
			return;
		}

		topicBuffer += runtime.startupText().getDialogueResponseLine(responseLineIndex);
		topicBufferLineIndex = -1;
	};
	auto clearTopicBuffer = [&]() {
		topicBuffer.clear();
		topicBufferLineIndex = -1;
	};
	auto playMoynahanLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, kMoynahanNpc, headVariant);
	};
	auto queueRoomVisualMutation = [&](bool changed) {
		if (!changed)
			return;

		InteractionResult interaction;
		interaction.mutatedRuntimeState = true;
		interaction.visualRuntimeStateChanged = true;
		runtime.queueDialogueInteractionIfNeeded(interaction);
	};
	auto giveGlue = [&]() {
		bool changed = false;
		changed |= runtime.startupScript().setRuntimeObjectVisible(kRahRoom, kGlueObject, false);
		changed |= runtime.startupScript().addRuntimeObjectToInventory(kGlueObject);
		changed |= runtime.startupScript().setRuntimeFlagValue(kJustGotGlueFlag, true);
		state.gotGlue = true;
		return changed;
	};
	auto isCasketPhoto = [&](const Common::String &itemName) {
		return itemName.equalsIgnoreCase("CASKET_PHOTO") ||
			itemName.equalsIgnoreCase("CASKET_PHOTOCOPY");
	};
	auto isLedger = [&](const Common::String &itemName) {
		return itemName.equalsIgnoreCase("LEDGER") ||
			itemName.equalsIgnoreCase("LEDGER2");
	};
	auto isBlackmailEvidence = [&](const Common::String &itemName) {
		return itemName.equalsIgnoreCase("NOTE") ||
			itemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
			itemName.equalsIgnoreCase("CHECKBOOK") ||
			itemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY");
	};

	if (runtime.startupScript().getFlagValue("IF_TRY_TO_TAKE_THE_GLUE")) {
		Common::Error lineError = playDialogueSequence(runtime,
			kMoynahanGlueInterruptOpeningLines, ARRAYSIZE(kMoynahanGlueInterruptOpeningLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		if (!runtime.startupScript().getFlagValue("IF_SAW_THE_SLASHES_ON_THE_BODY") ||
				!state.slashMarksTopicUnlocked) {
			return playDialogueSequence(runtime,
				kMoynahanGlueInterruptShortReplyLines,
				ARRAYSIZE(kMoynahanGlueInterruptShortReplyLines));
		}

		return playDialogueSequence(runtime,
			kMoynahanGlueInterruptExtendedReplyLines,
			ARRAYSIZE(kMoynahanGlueInterruptExtendedReplyLines));
	}

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			sharedState.discussedWhaleyHerrillPhoto = 1;
			return playMoynahanLine(0x3e45);
		}

		if (isCasketPhoto(usedItemName)) {
			Common::Error lineError = playDialogueSequence(runtime,
				kMoynahanCasketIntroLines, ARRAYSIZE(kMoynahanCasketIntroLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x18c, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = playDialogueSequence(runtime, kMoynahanCasketResponseOneLines,
					ARRAYSIZE(kMoynahanCasketResponseOneLines));
			} else if (responseIndex == 2) {
				lineError = playMoynahanLine(0x3eb1, 2);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			lineError = playDialogueSequence(runtime, kMoynahanCasketFollowupLines,
				ARRAYSIZE(kMoynahanCasketFollowupLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			if (state.gotGlue)
				return playMoynahanLine(0x3ec3, 2);

			lineError = playMoynahanLine(0x3ec8, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			responseIndex = 0;
			responseError = runtime.runResponseMenu(0x18d, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = playMoynahanLine(0x3ed2, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				bool changed = false;
				changed |= runtime.startupScript().setRuntimeObjectVisible(kRahRoom, usedItemName, false);
				changed |= giveGlue();
				queueRoomVisualMutation(changed);
				return Common::kNoError;
			}

			if (responseIndex == 2)
				return playMoynahanLine(0x3ed7, 1);

			return Common::kNoError;
		}

		if (isLedger(usedItemName) &&
				runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
			if (sharedState.discussedLedgerEvidence != 0 && !state.ledgerEvidenceLinePlayed) {
				state.ledgerEvidenceLinePlayed = true;
				Common::Error lineError = playMoynahanLine(0x3f87, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				runtime.setActiveSpeakerPortrait(
					runtime.currentRoomName().equalsIgnoreCase(kMoynahanHotelRoom)
						? kMoynahanNpc
						: kMoynahanEmbalmingPortrait,
					0);
				return Common::kNoError;
			}

			Common::Error lineError = playDialogueSequence(runtime,
				kMoynahanLedgerIntroLines, ARRAYSIZE(kMoynahanLedgerIntroLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x18e, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = playDialogueSequence(runtime,
					kMoynahanLedgerResponseOneLines,
					ARRAYSIZE(kMoynahanLedgerResponseOneLines));
			} else if (responseIndex == 2) {
				lineError = playDialogueSequence(runtime,
					kMoynahanLedgerResponseTwoLines,
					ARRAYSIZE(kMoynahanLedgerResponseTwoLines));
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			return playMoynahanLine(0x3ed7, 1);
		}

		if (isBlackmailEvidence(usedItemName)) {
			sharedState.discussedNoteCheckbookEvidence = 1;

			Common::Error lineError = playMoynahanLine(0x3f3e);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x190, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = playMoynahanLine(0x3f4a);
			} else if (responseIndex == 2) {
				lineError = playMoynahanLine(0x3f4f, 2);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			return playMoynahanLine(0x3f54, 2);
		}

		if (usedItemName.equalsIgnoreCase("WED_MATCHES")) {
			Common::Error lineError = playDialogueSequence(runtime,
				kMoynahanMatchesLines, ARRAYSIZE(kMoynahanMatchesLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED"))
				return playMoynahanLine(0x4054, 3);
			return Common::kNoError;
		}

		return playMoynahanLine(0x3e3f, 2);
	}

	if (state.introPending) {
		state.introPending = false;
		return playDialogueSequence(runtime, kMoynahanIntroLines, ARRAYSIZE(kMoynahanIntroLines));
	}

	Common::Error lineError = playMoynahanLine(0x3fd1);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	if (runtime.startupScript().getFlagValue("IF_SAW_THE_SLASHES_ON_THE_BODY") &&
			!state.slashMarksIntroShown) {
		lineError = playDialogueSequence(runtime, kMoynahanSlashMarksIntroLines,
			ARRAYSIZE(kMoynahanSlashMarksIntroLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		sharedState.dialogueStateD2ec8 = true;
		state.slashMarksTopicUnlocked = true;
		state.slashMarksIntroShown = true;
		if (topicBuffer.empty())
			assignTopicBuffer(0x194);
		else
			appendTopicBuffer(0x195);
	}

	if (sharedState.discussedCasketPhotoEvidence != 0 && !state.introPending) {
		state.introPending = true;
		lineError = playMoynahanLine(0x3f5b, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS") && !state.ledgersIntroduced) {
		state.ledgersIntroduced = true;
		lineError = playMoynahanLine(0x3f65);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x196, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playMoynahanLine(0x3f70);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			(void)runtime.startupScript().resetRuntimeObjectToInitialState("LEDGER");
			(void)runtime.startupScript().resetRuntimeObjectToInitialState("LEDGER2");
		} else if (responseIndex == 2) {
			lineError = playMoynahanLine(0x3f76);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			if (runtime.startupScript().getCurrentStoryDayIndex() > 2) {
				sharedState.moynahanKarinKidnapedDiscussionState = true;
				lineError = playMoynahanLine(0x3f7a);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			lineError = playMoynahanLine(0x3f7e, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}

	if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") &&
			!state.stephMidgameShown) {
		state.stephMidgameShown = true;
		lineError = playMoynahanLine(0x3fd9);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getCurrentStoryDayIndex() == 5 && !state.dayFiveShown) {
		state.dayFiveShown = true;
		lineError = playDialogueSequence(runtime, kMoynahanDayFiveLines,
			ARRAYSIZE(kMoynahanDayFiveLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("BURNED_TV_STATION") &&
			!state.burnedTvStationShown) {
		state.burnedTvStationShown = true;
		lineError = playMoynahanLine(0x3ffe);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("KARIN_KIDNAPED") &&
			!state.karinKidnapedShown) {
		state.karinKidnapedShown = true;
		lineError = playMoynahanLine(0x4007);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x197, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playMoynahanLine(0x4012);
		} else if (responseIndex == 2) {
			const DialogueLineEntry responseTwoLines[] = {
				{ 0x4017, kMoynahanNpc, 0 },
				{ 0x401c, kPcSpeaker, 0 }
			};
			lineError = playDialogueSequence(runtime, responseTwoLines, ARRAYSIZE(responseTwoLines));
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = playDialogueSequence(runtime, kMoynahanKarinKidnapedTailLines,
			ARRAYSIZE(kMoynahanKarinKidnapedTailLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		sharedState.karinKidnapedDialogueState = true;
		sharedState.moynahanKarinKidnapedDiscussionState = true;
	}

	if (!runtime.startupScript().isNamedNpcDeathTypeClear("BUTCHER") &&
			!state.butcherDeadShown) {
		state.butcherDeadShown = true;
		lineError = playMoynahanLine(0x40bb, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (!runtime.startupScript().isNamedNpcDeathTypeClear("JIMMY") &&
			!state.jimmyDeadShown) {
		state.jimmyDeadShown = true;
		lineError = playMoynahanLine(0x40cd, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("KARIN_FOUND_DEAD") &&
			!state.karinFoundDeadShown) {
		state.karinFoundDeadShown = true;
		lineError = playMoynahanLine(0x40d4, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	for (;;) {
		Common::String selectedTopic;
		Common::Error menuError = runtime.runKeywordMenu(
			topicBuffer, topicBufferLineIndex, selectedTopic);
		if (menuError.getCode() != Common::kNoError)
			return menuError;
		if (selectedTopic.empty())
			return Common::kNoError;

		if (selectedTopic.equalsIgnoreCase(runtime.genericByeTopic()) ||
				runtime.matchesResponseLine(selectedTopic, 0x199))
			return playMoynahanLine(0x3e32, 3);

		if (runtime.matchesAnyResponseLine(selectedTopic, kMoynahanGoodCauseTopicLines,
				ARRAYSIZE(kMoynahanGoodCauseTopicLines))) {
			sharedState.momGoodCauseDay5State = true;
			lineError = playDialogueSequence(runtime, kMoynahanGoodCauseLines,
				ARRAYSIZE(kMoynahanGoodCauseLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignTopicBuffer(0x19c);
			continue;
		}

		if (runtime.matchesAnyResponseLine(selectedTopic, kMoynahanSlashMarksTopicLines,
				ARRAYSIZE(kMoynahanSlashMarksTopicLines))) {
			lineError = playDialogueSequence(runtime, kMoynahanSlashMarksTopicReplyLines,
				ARRAYSIZE(kMoynahanSlashMarksTopicReplyLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignTopicBuffer(0x1a1);
			continue;
		}

		if (runtime.matchesAnyResponseLine(selectedTopic, kMoynahanBodyTopicLines,
				ARRAYSIZE(kMoynahanBodyTopicLines))) {
			lineError = playMoynahanLine(0x3da0, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			if (runtime.startupScript().getCurrentStoryDayIndex() > 4) {
				lineError = playMoynahanLine(0x3da1, 3);
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				sharedState.dialogueStateD2ec8 = true;
				state.slashMarksTopicUnlocked = true;
			}

			lineError = playMoynahanLine(0x3da2, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			assignTopicBuffer(0x1a6);
			if (runtime.startupScript().getCurrentStoryDayIndex() > 4)
				appendTopicBuffer(0x1a7);
			if (runtime.currentRoomName().equalsIgnoreCase(kMoynahanEmbalmingRoom))
				appendTopicBuffer(0x1a8);
			continue;
		}

		if (runtime.matchesAnyResponseLine(selectedTopic, kMoynahanGeneralTopicLines,
				ARRAYSIZE(kMoynahanGeneralTopicLines))) {
			lineError = playMoynahanLine(0x3db1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x1ac, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = playDialogueSequence(runtime,
					kMoynahanGeneralTopicResponseOneLines,
					ARRAYSIZE(kMoynahanGeneralTopicResponseOneLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignTopicBuffer(0x1ad);
			} else if (responseIndex == 2) {
				lineError = playDialogueSequence(runtime,
					kMoynahanGeneralTopicResponseTwoLines,
					ARRAYSIZE(kMoynahanGeneralTopicResponseTwoLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				if (runtime.startupScript().getCurrentStoryDayIndex() > 4) {
					lineError = playMoynahanLine(0x3deb);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
					sharedState.dialogueStateD2ec8 = true;
				}

				assignTopicBuffer(0x1b0);
				if (runtime.startupScript().getCurrentStoryDayIndex() > 4)
					appendTopicBuffer(0x1b1);
			}
			continue;
		}

		if (runtime.matchesAnyResponseLine(selectedTopic, kMoynahanDayFiveUnlockTopicLines,
				ARRAYSIZE(kMoynahanDayFiveUnlockTopicLines))) {
			if (runtime.startupScript().getCurrentStoryDayIndex() >= 5) {
				sharedState.dialogueStateD2ec8 = true;
				sharedState.dialogueStateD2ebc = true;
				sharedState.dialogueStateD2ec0 = true;
				lineError = playDialogueSequence(runtime, kMoynahanDayFiveUnlockLines,
					ARRAYSIZE(kMoynahanDayFiveUnlockLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				state.slashMarksTopicUnlocked = true;
				clearTopicBuffer();
			}
			continue;
		}

		if (runtime.matchesAnyResponseLine(selectedTopic, kMoynahanEmbalmingRoomTopicLines,
				ARRAYSIZE(kMoynahanEmbalmingRoomTopicLines))) {
			if (runtime.currentRoomName().equalsIgnoreCase(kMoynahanEmbalmingRoom)) {
				lineError = playDialogueSequence(runtime, kMoynahanEmbalmingRoomLines,
					ARRAYSIZE(kMoynahanEmbalmingRoomLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignTopicBuffer(0x1b6);
			}
			continue;
		}

		if (runtime.matchesAnyResponseLine(selectedTopic, kMoynahanLateTopicLines,
				ARRAYSIZE(kMoynahanLateTopicLines))) {
			if (runtime.startupScript().getCurrentStoryDayIndex() > 4) {
				lineError = playMoynahanLine(0x40f5);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			continue;
		}

		if (runtime.matchesAnyResponseLine(selectedTopic, kMoynahanGlueTopicLines,
				ARRAYSIZE(kMoynahanGlueTopicLines))) {
			if (runtime.startupScript().getCurrentStoryDayIndex() > 4) {
				if (!state.gotGlue) {
					lineError = playMoynahanLine(0x40fb);
					if (lineError.getCode() != Common::kNoError)
						return lineError;

					int responseIndex = 0;
					Common::Error responseError = runtime.runResponseMenu(0x1c3, responseIndex);
					if (responseError.getCode() != Common::kNoError)
						return responseError;

					if (responseIndex == 1) {
						lineError = playMoynahanLine(0x4108, 2);
						if (lineError.getCode() != Common::kNoError)
							return lineError;
						queueRoomVisualMutation(giveGlue());
					} else if (responseIndex == 2) {
						lineError = playMoynahanLine(0x410d, 2);
						if (lineError.getCode() != Common::kNoError)
							return lineError;
					}
				} else {
					lineError = playMoynahanLine(0x410d, 2);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				}
			}
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x1c4))
			continue;

		lineError = playMoynahanLine(0x3e39);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
}

} // End of namespace Harvester
