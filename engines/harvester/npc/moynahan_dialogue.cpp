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

#include "harvester/npc/moynahan_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"
#include "harvester/text.h"

namespace Harvester {

namespace {

static const int kMoynahanGoodCauseTopicLines[] = { 0x19a, 0x19b };
static const int kMoynahanSlashMarksTopicLines[] = { 0x19d, 0x19e, 0x19f, 0x1a0 };
static const int kMoynahanBodyTopicLines[] = { 0x1a2, 0x1a3 };
static const int kMoynahanGeneralTopicLines[] = { 0x1a9, 0x1aa, 0x1ab };
static const int kMoynahanDayFiveUnlockTopicLines[] = { 0x1b2, 0x1b3 };
static const int kMoynahanEmbalmingRoomTopicLines[] = { 0x1b4, 0x1b5 };
static const int kMoynahanLateTopicLines[] = { 0x1b8, 0x1b9, 0x1ba, 0x1bb, 0x1bc, 0x1bd, 0x1be };
static const int kMoynahanGlueTopicLines[] = { 0x1bf, 0x1c0, 0x1c1, 0x1c2 };

static const char *const kMoynahanNpc = "MOYNAHAN";
static const char *const kMoynahanEmbalmingPortrait = "MOYNAHAN_MBLM";
static const char *const kMoynahanHotelRoom = "HTL-RT";
static const char *const kMoynahanEmbalmingRoom = "MBLM";
static const char *const kRahRoom = "RAH";
static const char *const kGlueObject = "GLUE";
static const char *const kJustGotGlueFlag = "JUST_GOT_GLUE";

static Common::Error playDialogueSequence(DialogueRuntime &runtime,
		const DialogueLineEntry *lines, uint count) {
	return runtime.playDialogueEntrySequence(lines, count);
}

} // End of namespace

bool MoynahanDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("MOYNAHAN");
}

Common::Error MoynahanDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	MoynahanRoomDialogueState &state = _state;
	Common::String topicBuffer;
	int topicBufferLineIndex = -1;

	auto appendSlashMarksTopic = [&]() {
		if (topicBuffer.empty()) {
			runtime.assignTopicBuffer(topicBuffer, topicBufferLineIndex,
				0x194, "Moynahan keyword topics");
			return;
		}

		topicBuffer += runtime.startupText().getDialogueResponseLine(0x195);
		topicBufferLineIndex = -1;
	};

	if (runtime.startupScript().getFlagValue("IF_TRY_TO_TAKE_THE_GLUE")) {
		const DialogueLineEntry initialLines[] = {
			{ 0x3e4d, kMoynahanNpc, 0 },
			{ 0x3e52, "PC", 0 },
			{ 0x3e56, kMoynahanNpc, 0 }
		};
		Common::Error lineError = playDialogueSequence(runtime, initialLines, ARRAYSIZE(initialLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		if (!runtime.startupScript().getFlagValue("IF_SAW_THE_SLASHES_ON_THE_BODY") ||
				!state.slashMarksTopicUnlocked) {
			const DialogueLineEntry shortReplyLines[] = {
				{ 0x3e76, "PC", 0 },
				{ 0x3e7a, kMoynahanNpc, 0 }
			};
			return playDialogueSequence(runtime, shortReplyLines, ARRAYSIZE(shortReplyLines));
		}

		const DialogueLineEntry extendedReplyLines[] = {
			{ 0x3e5b, "PC", 0 },
			{ 0x3e60, kMoynahanNpc, 0 },
			{ 0x3e64, "PC", 0 },
			{ 0x3e69, kMoynahanNpc, 0 },
			{ 0x3e6d, "PC", 0 },
			{ 0x3e71, kMoynahanNpc, 0 }
		};
		return playDialogueSequence(runtime, extendedReplyLines, ARRAYSIZE(extendedReplyLines));
	}

	if (usedItemName.empty()) {
		if (state.introPending) {
			state.introPending = false;
			const DialogueLineEntry introLines[] = {
				{ 0x3d33, kMoynahanNpc, 0 },
				{ 0x3d37, "PC", 0 },
				{ 0x3d3b, kMoynahanNpc, 0 },
				{ 0x3d40, "PC", 0 },
				{ 0x3d45, kMoynahanNpc, 0 },
				{ 0x3d47, kMoynahanNpc, 0 }
			};
			return playDialogueSequence(runtime, introLines, ARRAYSIZE(introLines));
		}

		Common::Error lineError = runtime.playDialogueLine(0x3fd1, kMoynahanNpc);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		if (runtime.startupScript().getFlagValue("IF_SAW_THE_SLASHES_ON_THE_BODY") &&
				!state.slashMarksIntroShown) {
			const DialogueLineEntry slashMarksLines[] = {
				{ 0x3da0, kMoynahanNpc, 0 },
				{ 0x3da1, kMoynahanNpc, 0 },
				{ 0x3da2, kMoynahanNpc, 0 }
			};
			lineError = playDialogueSequence(runtime, slashMarksLines, ARRAYSIZE(slashMarksLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			sharedState.dialogueStateD2ec8 = true;
			state.slashMarksTopicUnlocked = true;
			state.slashMarksIntroShown = true;
			appendSlashMarksTopic();
		}

		if (runtime.startupScript().getFlagValue(DialogueFlags::kShownPhotoOfCorpse) &&
				!state.introPending) {
			state.introPending = true;
			lineError = runtime.playDialogueLine(0x3f5b, kMoynahanNpc);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		if (runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS") && !state.ledgersIntroduced) {
			state.ledgersIntroduced = true;
			lineError = runtime.playDialogueLine(0x3f65, kMoynahanNpc);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x196, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = runtime.playDialogueLine(0x3f70, kMoynahanNpc);
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				(void)runtime.startupScript().setRuntimeObjectVisible("INVENTORY", "LEDGER", false);
				(void)runtime.startupScript().setRuntimeObjectVisible("INVENTORY", "LEDGER2", false);
			} else if (responseIndex == 2) {
				lineError = runtime.playDialogueLine(0x3f76, kMoynahanNpc);
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				if (runtime.startupScript().getCurrentStoryDayIndex() > 2) {
					sharedState.moynahanKarinKidnapedDiscussionState = true;
					lineError = runtime.playDialogueLine(0x3f7a, kMoynahanNpc);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				}

				lineError = runtime.playDialogueLine(0x3f7e, kMoynahanNpc);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
		}

		if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") &&
				!state.stephMidgameShown) {
			state.stephMidgameShown = true;
			lineError = runtime.playDialogueLine(0x3fd9, kMoynahanNpc);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		if (runtime.startupScript().getCurrentStoryDayIndex() == 5 && !state.dayFiveShown) {
			state.dayFiveShown = true;
			const DialogueLineEntry dayFiveLines[] = {
				{ 0x3fe5, kMoynahanNpc, 0 },
				{ 0x3fea, "PC", 0 },
				{ 0x3fee, kMoynahanNpc, 0 },
				{ 0x3ff3, "PC", 0 },
				{ 0x3ff7, kMoynahanNpc, 0 }
			};
			lineError = playDialogueSequence(runtime, dayFiveLines, ARRAYSIZE(dayFiveLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		if (runtime.startupScript().getFlagValue("BURNED_TV_STATION") &&
				!state.burnedTvStationShown) {
			state.burnedTvStationShown = true;
			lineError = runtime.playDialogueLine(0x3ffe, kMoynahanNpc);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		if (runtime.startupScript().getFlagValue("KARIN_KIDNAPED") &&
				!state.karinKidnapedShown) {
			state.karinKidnapedShown = true;
			lineError = runtime.playDialogueLineWithVariant(0x4007, kMoynahanNpc, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x197, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = runtime.playDialogueLine(0x4012, kMoynahanNpc);
			} else if (responseIndex == 2) {
				const DialogueLineEntry responseTwoLines[] = {
					{ 0x4017, kMoynahanNpc, 0 },
					{ 0x401c, "PC", 0 }
				};
				lineError = playDialogueSequence(runtime, responseTwoLines, ARRAYSIZE(responseTwoLines));
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			const DialogueLineEntry kidnapedLines[] = {
				{ 0x4020, kMoynahanNpc, 0 },
				{ 0x4026, "PC", 0 },
				{ 0x402a, kMoynahanNpc, 0 },
				{ 0x4034, "PC", 0 },
				{ 0x4038, kMoynahanNpc, 0 },
				{ 0x403d, "PC", 0 }
			};
			return playDialogueSequence(runtime, kidnapedLines, ARRAYSIZE(kidnapedLines));
		}

		if (!runtime.startupScript().isNamedNpcDeathTypeClear("BUTCHER") &&
				!state.butcherDeadShown) {
			state.butcherDeadShown = true;
			return runtime.playDialogueLine(0x40bb, kMoynahanNpc);
		}

		if (!runtime.startupScript().isNamedNpcDeathTypeClear("JIMMY") &&
				!state.jimmyDeadShown) {
			state.jimmyDeadShown = true;
			return runtime.playDialogueLine(0x40cd, kMoynahanNpc);
		}

		if (runtime.startupScript().getFlagValue("KARIN_FOUND_DEAD") &&
				!state.karinFoundDeadShown) {
			state.karinFoundDeadShown = true;
			return runtime.playDialogueLine(0x40d4, kMoynahanNpc);
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
				return runtime.playDialogueLine(0x3e32, kMoynahanNpc);

			if (runtime.matchesAnyResponseLine(selectedTopic, kMoynahanGoodCauseTopicLines,
					ARRAYSIZE(kMoynahanGoodCauseTopicLines))) {
				sharedState.momGoodCauseDay5State = true;
				return runtime.playDialogueLine(0x3d5f, kMoynahanNpc);
			}

			if (runtime.matchesAnyResponseLine(selectedTopic, kMoynahanSlashMarksTopicLines,
					ARRAYSIZE(kMoynahanSlashMarksTopicLines)))
				return runtime.playDialogueLine(0x3d76, kMoynahanNpc);

			if (runtime.matchesAnyResponseLine(selectedTopic, kMoynahanBodyTopicLines,
					ARRAYSIZE(kMoynahanBodyTopicLines)))
				return runtime.playDialogueLine(0x3da0, kMoynahanNpc);

			if (runtime.matchesAnyResponseLine(selectedTopic, kMoynahanGeneralTopicLines,
					ARRAYSIZE(kMoynahanGeneralTopicLines)))
				return runtime.playDialogueLine(0x3db1, kMoynahanNpc);

			if (runtime.matchesAnyResponseLine(selectedTopic, kMoynahanDayFiveUnlockTopicLines,
					ARRAYSIZE(kMoynahanDayFiveUnlockTopicLines))) {
				if (runtime.startupScript().getCurrentStoryDayIndex() > 4) {
					sharedState.dialogueStateD2ec8 = true;
					sharedState.dialogueStateD2ebc = true;
					sharedState.dialogueStateD2ec0 = true;
					return runtime.playDialogueLine(0x3df5, kMoynahanNpc);
				}
			}

			if (runtime.matchesAnyResponseLine(selectedTopic, kMoynahanEmbalmingRoomTopicLines,
					ARRAYSIZE(kMoynahanEmbalmingRoomTopicLines))) {
				if (runtime.currentRoomName().equalsIgnoreCase(kMoynahanEmbalmingRoom))
					return runtime.playDialogueLine(0x3e18, kMoynahanNpc);
			}

			if (runtime.matchesAnyResponseLine(selectedTopic, kMoynahanLateTopicLines,
					ARRAYSIZE(kMoynahanLateTopicLines))) {
				if (runtime.startupScript().getCurrentStoryDayIndex() > 4)
					return runtime.playDialogueLine(0x40f5, kMoynahanNpc);
			}

			if (runtime.matchesAnyResponseLine(selectedTopic, kMoynahanGlueTopicLines,
					ARRAYSIZE(kMoynahanGlueTopicLines))) {
				if (runtime.startupScript().getCurrentStoryDayIndex() > 4) {
					if (!state.gotGlue)
						return runtime.playDialogueLine(0x40fb, kMoynahanNpc);
					return runtime.playDialogueLine(0x410d, kMoynahanNpc);
				}
			}

			if (runtime.matchesResponseLine(selectedTopic, 0x1c4))
				continue;

			return runtime.playDialogueLine(0x3e39, kMoynahanNpc);
		}
	}

	if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
		return runtime.playDialogueLine(0x3e45, kMoynahanNpc);
	}

	if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
			usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
		const DialogueLineEntry casketIntroLines[] = {
			{ 0x3e84, kMoynahanNpc, 0 },
			{ 0x3e89, "PC", 0 },
			{ 0x3e8e, kMoynahanNpc, 0 },
			{ 0x3e93, "PC", 0 },
			{ 0x3e98, kMoynahanNpc, 0 }
		};
		Common::Error lineError = playDialogueSequence(runtime, casketIntroLines, ARRAYSIZE(casketIntroLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x18c, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			const DialogueLineEntry responseOneLines[] = {
				{ 0x3ea4, kMoynahanNpc, 0 },
				{ 0x3ea8, "PC", 0 },
				{ 0x3ead, kMoynahanNpc, 0 }
			};
			lineError = playDialogueSequence(runtime, responseOneLines, ARRAYSIZE(responseOneLines));
		} else if (responseIndex == 2) {
			lineError = runtime.playDialogueLine(0x3eb1, kMoynahanNpc);
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		const DialogueLineEntry casketFollowupLines[] = {
			{ 0x3eb7, "PC", 0 },
			{ 0x3ebc, kMoynahanNpc, 0 }
		};
		lineError = playDialogueSequence(runtime, casketFollowupLines, ARRAYSIZE(casketFollowupLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		if (state.gotGlue)
			return runtime.playDialogueLine(0x3ec3, kMoynahanNpc);

		lineError = runtime.playDialogueLine(0x3ec8, kMoynahanNpc);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		responseIndex = 0;
		responseError = runtime.runResponseMenu(0x18d, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = runtime.playDialogueLine(0x3ed2, kMoynahanNpc);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			(void)runtime.startupScript().resetRuntimeObjectToInitialState(usedItemName);
			(void)runtime.startupScript().setRuntimeObjectVisible(kRahRoom, usedItemName, true);
			(void)runtime.startupScript().setRuntimeObjectVisible(kRahRoom, kGlueObject, true);
			(void)runtime.startupScript().addRuntimeObjectToInventory(kGlueObject);
			(void)runtime.startupScript().setRuntimeFlagValue(kJustGotGlueFlag, true);
			state.gotGlue = true;
			return Common::kNoError;
		}

		if (responseIndex == 2)
			return runtime.playDialogueLine(0x3ed7, kMoynahanNpc);

		return Common::kNoError;
	}

	if ((usedItemName.equalsIgnoreCase("LEDGER") ||
				usedItemName.equalsIgnoreCase("LEDGER2")) &&
			runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
		if (runtime.startupScript().getFlagValue(DialogueFlags::kShownLedgersToAnyone) &&
				!state.ledgerEvidenceLinePlayed) {
			state.ledgerEvidenceLinePlayed = true;
			Common::Error lineError = runtime.playDialogueLine(0x3f87, kMoynahanNpc);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			runtime.setActiveSpeakerPortrait(
				runtime.currentRoomName().equalsIgnoreCase(kMoynahanHotelRoom)
					? kMoynahanNpc
					: kMoynahanEmbalmingPortrait,
				0);
			return Common::kNoError;
		}

		const DialogueLineEntry ledgerIntroLines[] = {
			{ 0x3ede, "PC", 0 },
			{ 0x3ee3, kMoynahanNpc, 0 },
			{ 0x3ee7, "PC", 0 },
			{ 0x3eec, kMoynahanNpc, 0 }
		};
		Common::Error lineError = playDialogueSequence(runtime, ledgerIntroLines, ARRAYSIZE(ledgerIntroLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x18e, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			const DialogueLineEntry responseOneLines[] = {
				{ 0x3ef7, kMoynahanNpc, 0 },
				{ 0x3efb, "PC", 0 },
				{ 0x3eff, kMoynahanNpc, 0 },
				{ 0x3f05, "PC", 0 }
			};
			lineError = playDialogueSequence(runtime, responseOneLines, ARRAYSIZE(responseOneLines));
		} else if (responseIndex == 2) {
			const DialogueLineEntry responseTwoLines[] = {
				{ 0x3f09, kMoynahanNpc, 0 },
				{ 0x3f0f, "PC", 0 },
				{ 0x3f13, kMoynahanNpc, 0 },
				{ 0x3f18, "PC", 0 }
			};
			lineError = playDialogueSequence(runtime, responseTwoLines, ARRAYSIZE(responseTwoLines));
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		return runtime.playDialogueLine(0x3ed7, kMoynahanNpc);
	}
	if (usedItemName.equalsIgnoreCase("NOTE") ||
			usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
		Common::Error lineError = runtime.playDialogueLine(0x3f3e, kMoynahanNpc);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x190, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = runtime.playDialogueLine(0x3f4a, kMoynahanNpc);
		} else if (responseIndex == 2) {
			lineError = runtime.playDialogueLine(0x3f4f, kMoynahanNpc);
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		return runtime.playDialogueLine(0x3f54, kMoynahanNpc);
	}

	if (usedItemName.equalsIgnoreCase("WED_MATCHES")) {
		const DialogueLineEntry matchesLines[] = {
			{ 0x4049, "PC", 0 },
			{ 0x404d, kMoynahanNpc, 0 }
		};
		Common::Error lineError = playDialogueSequence(runtime, matchesLines, ARRAYSIZE(matchesLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED"))
			return runtime.playDialogueLine(0x4054, kMoynahanNpc);
		return Common::kNoError;
	}

	return runtime.playDialogueLine(0x3e3f, kMoynahanNpc);
}

} // End of namespace Harvester
