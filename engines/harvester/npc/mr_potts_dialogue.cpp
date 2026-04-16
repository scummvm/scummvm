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


#include "harvester/npc/mr_potts_dialogue.h"

#include "graphics/screen.h"
#include "harvester/harvester.h"
#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"
#include "harvester/text.h"

namespace Harvester {

namespace {

static const char *const kDialogueC001BFstPath = "GRAPHIC/FST/C001B.FST";
static const char *const kDialogueC016AFstPath = "GRAPHIC/FST/C016A.FST";
static const char *const kStephHallExitActionTag = "STHLEXIT2B";
static const char *const kMonsterfyPottsActionTag = "MONSTERFY_POTTS";
static const char *const kMeatObjectName = "MEAT";
static const char *const kWeddingMatchesObjectName = "WED_MATCHES";
static const char *const kMrPottsMoynahanAccusationResponseText =
	"Mr. Moynahan says you were out and/   about the night she disappeared.";
static const char *const kMrPottsWeddingMatchesResponseText =
	"I found one of our wedding match-/   books in the cemetary.";

static const DialogueLineEntry kMrPottsHarvestBladeOpeningLines[] = {
	{ 0x2f1d, "POTTS_FLESH", 2 },
	{ 0x2f22, "PC", 4 },
	{ 0x2f26, "POTTS_FLESH", 2 },
	{ 0x2f2d, "PC", 0 },
	{ 0x2f31, "POTTS_FLESH", 2 }
};

static const DialogueLineEntry kMrPottsStephanieDeadInHallLines[] = {
	{ 0x2afc, "MR_POTTS", 3 },
	{ 0x2b00, "PC", 0 },
	{ 0x2b04, "MR_POTTS", 3 },
	{ 0x2b09, "PC", 2 },
	{ 0x2b0d, "MR_POTTS", 3 }
};

static const DialogueLineEntry kMrPottsPottsdamDiggingIntroLines[] = {
	{ 0x2c63, "MR_POTTS", 2 },
	{ 0x2c69, "PC", 0 },
	{ 0x2c6d, "MR_POTTS", 4 }
};

static const DialogueLineEntry kMrPottsPottsdamDiggingKarinLines[] = {
	{ 0x2c33, "MR_POTTS", 2 },
	{ 0x2c37, "PC", 0 },
	{ 0x2c3b, "MR_POTTS", 4 },
	{ 0x2c42, "PC", 0 },
	{ 0x2c46, "MR_POTTS", 2 },
	{ 0x2c47, "MR_POTTS", 4 },
	{ 0x2c48, "MR_POTTS", 4 },
	{ 0x2c49, "MR_POTTS", 0 },
	{ 0x2c54, "PC", 0 },
	{ 0x2c58, "MR_POTTS", 2 }
};

static const DialogueLineEntry kMrPottsAuxIntroResponse1Lines[] = {
	{ 0x2994, "MR_POTTS", 1 },
	{ 0x2998, "PC", 2 },
	{ 0x299d, "MR_POTTS", 1 }
};

static const DialogueLineEntry kMrPottsAuxIntroResponse1WithMomIntroLines[] = {
	{ 0x29a3, "MR_POTTS", 1 },
	{ 0x29ab, "MRS_POTTS", 1 }
};

static const DialogueLineEntry kMrPottsAmnesiaTopicLines[] = {
	{ 0x29c5, "PC", 2 },
	{ 0x29ca, "MR_POTTS", 4 },
	{ 0x29cf, "PC", 2 },
	{ 0x29d3, "MR_POTTS", 4 }
};

static const DialogueLineEntry kMrPottsMarriageTopicLines[] = {
	{ 0x29de, "PC", 0 },
	{ 0x29e2, "MR_POTTS", 3 }
};

static const DialogueLineEntry kMrPottsCommitmentTopicLines[] = {
	{ 0x29ee, "PC", 0 },
	{ 0x29f3, "MR_POTTS", 0 },
	{ 0x29f4, "MR_POTTS", 4 },
	{ 0x29f5, "MR_POTTS", 4 },
	{ 0x29f6, "MR_POTTS", 4 }
};

static const DialogueLineEntry kMrPottsFatherTopicLines[] = {
	{ 0x2a12, "MR_POTTS", 4 },
	{ 0x2a16, "PC", 0 },
	{ 0x2a1b, "MR_POTTS", 1 },
	{ 0x2a22, "PC", 0 },
	{ 0x2a26, "MR_POTTS", 4 }
};

static const DialogueLineEntry kMrPottsSlaughterhouseTopicClosingLines[] = {
	{ 0x2a4c, "MR_POTTS", 0 },
	{ 0x2a55, "MR_POTTS", 1 }
};

static const DialogueLineEntry kMrPottsLodgeTopicLines[] = {
	{ 0x2a8b, "PC", 0 },
	{ 0x2aa2, "MR_POTTS", 1 }
};

static const DialogueLineEntry kMrPottsKarinKidnapedResponse1Lines[] = {
	{ 0x2b2f, "MR_POTTS", 3 },
	{ 0x2b34, "PC", 0 },
	{ 0x2b38, "MR_POTTS", 0 }
};

static const DialogueLineEntry kMrPottsKarinKidnapedResponse1NestedResponse2Lines[] = {
	{ 0x2b55, "MR_POTTS", 0 },
	{ 0x2b5b, "PC", 0 },
	{ 0x2b5f, "MR_POTTS", 1 }
};

static const DialogueLineEntry kMrPottsKarinFoundDeadIntroLines[] = {
	{ 0x2b8d, "MR_POTTS", 3 },
	{ 0x2b91, "PC", 3 }
};

static const DialogueLineEntry kMrPottsKarinFoundDeadWithoutStephMidgameLines[] = {
	{ 0x2b9b, "MR_POTTS", 0 },
	{ 0x2b9f, "PC", 0 },
	{ 0x2ba4, "MR_POTTS", 0 }
};

static const DialogueLineEntry kMrPottsKarinFoundDeadWithStephMidgameLines[] = {
	{ 0x2b96, "PC", 3 },
	{ 0x2b9f, "PC", 0 },
	{ 0x2ba4, "MR_POTTS", 0 }
};

static const DialogueLineEntry kMrPottsDay6KarinFoundAliveAlibiFollowupLines[] = {
	{ 0x2bf2, "MRS_POTTS", 1 },
	{ 0x2bf7, "MR_POTTS", 2 },
	{ 0x2bfe, "MR_POTTS", 4 }
};

static const DialogueLineEntry kMrPottsDay6KarinFoundAliveNoMoynahanLines[] = {
	{ 0x2c1a, "MR_POTTS", 0 },
	{ 0x2c20, "MRS_POTTS", 0 }
};

static const DialogueLineEntry kMrPottsMurderAccusationOpeningLines[] = {
	{ 0x2d58, "MR_POTTS", 2 },
	{ 0x2d60, "MRS_POTTS", 1 },
	{ 0x2d65, "MR_POTTS", 4 }
};

static const DialogueLineEntry kMrPottsWeddingMatchesAccusationLines[] = {
	{ 0x2db3, "MR_POTTS", 0 },
	{ 0x2db8, "MR_POTTS", 0 },
	{ 0x2dbd, "MR_POTTS", 4 },
	{ 0x2dc2, "MR_POTTS", 2 }
};

static const DialogueLineEntry kMrPottsPeepholeOpeningLines[] = {
	{ 0x2cff, "MR_POTTS", 0 },
	{ 0x2d03, "PC", 0 },
	{ 0x2d08, "MR_POTTS", 0 }
};

static const DialogueLineEntry kMrPottsPeepholeResponse2Lines[] = {
	{ 0x2d19, "MR_POTTS", 4 },
	{ 0x2d1d, "PC", 2 },
	{ 0x2d22, "MR_POTTS", 1 }
};

static const DialogueLineEntry kMrPottsPeepholeClosingLines[] = {
	{ 0x2d2c, "PC", 0 },
	{ 0x2d30, "MR_POTTS", 0 },
	{ 0x2d35, "PC", 0 }
};

static const DialogueLineEntry kMrPottsPeepholeFollowupLines[] = {
	{ 0x2d39, "MR_POTTS", 0 },
	{ 0x2d3e, "PC", 0 }
};

static const DialogueLineEntry kMrPottsMolestResponse1OpeningLines[] = {
	{ 0x2dfb, "MR_POTTS", 2 },
	{ 0x2e01, "MR_POTTS", 0 },
	{ 0x2e06, "PC", 2 },
	{ 0x2e0a, "MR_POTTS", 0 }
};

static const DialogueLineEntry kMrPottsMolestResponse2IntroLines[] = {
	{ 0x2e18, "MR_POTTS", 3 },
	{ 0x2e1c, "PC", 2 },
};

static const DialogueLineEntry kMrPottsMolestAlternativeOpeningLines[] = {
	{ 0x2e3a, "MR_POTTS", 1 },
	{ 0x2e3e, "PC", 0 },
	{ 0x2e42, "MR_POTTS", 0 },
	{ 0x2e46, "PC", 2 },
	{ 0x2e4a, "MR_POTTS", 1 }
};

static const DialogueLineEntry kMrPottsCasketPhotoLines[] = {
	{ 0x2c97, "MR_POTTS", 0 },
	{ 0x2c9b, "PC", 2 },
	{ 0x2ca0, "MR_POTTS", 0 }
};

static const DialogueLineEntry kMrPottsCasketPhotoFollowupLines[] = {
	{ 0x2ca4, "PC", 2 },
	{ 0x2cab, "MR_POTTS", 0 }
};

static const DialogueLineEntry kMrPottsWhaleyHerrillPhotoLines[] = {
	{ 0x2cb3, "MR_POTTS", 1 },
	{ 0x2cb8, "PC", 1 },
	{ 0x2cbd, "MR_POTTS", 0 }
};

static const DialogueLineEntry kMrPottsBlackmailEvidenceLines[] = {
	{ 0x2ce0, "MR_POTTS", 4 },
	{ 0x2ce6, "PC", 0 }
};

static const DialogueLineEntry kMrPottsTvDeedEvidenceLines[] = {
	{ 0x2ce0, "MR_POTTS", 4 },
	{ 0x2ceb, "PC", 0 },
	{ 0x2cf0, "MR_POTTS", 2 }
};

static const int kMrPottsMurderTopicResponseLines[] = { 0x20c, 0x20d, 0x20e };
static const int kMrPottsMolestTopicResponseLines[] = { 0x216, 0x217, 0x218 };
static const int kMrPottsMoynahanTopicResponseLines[] = { 0x201, 0x202 };
static const int kMrPottsPeepholeTopicResponseLines[] = {
	0x204, 0x205, 0x206, 0x207, 0x208, 0x209, 0x20a
};

} // End of namespace

bool MrPottsDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("MR_POTTS") ||
		npcName.equalsIgnoreCase("POTTS_FLESH") ||
		npcName.equalsIgnoreCase("MR_POTTS_CEM10") ||
		npcName.equalsIgnoreCase("MR_POTTS_HALL");
}

Common::Error MrPottsDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	MrPottsRoomDialogueState &state = _state;
	Common::String mrPottsTopicBuffer;
	int mrPottsTopicBufferLineIndex = state.topicBufferLineIndex;

	auto assignMrPottsTopicBuffer = [&](int responseLineIndex) {
		runtime.assignTopicBuffer(mrPottsTopicBuffer, mrPottsTopicBufferLineIndex,
			responseLineIndex, "Mr Potts topic buffer");
		state.topicBufferLineIndex = mrPottsTopicBufferLineIndex;
	};
	auto restoreMrPottsTopicBuffer = [&]() {
		if (state.topicBufferLineIndex < 0)
			return;

		runtime.assignTopicBuffer(mrPottsTopicBuffer, mrPottsTopicBufferLineIndex,
			state.topicBufferLineIndex, "Mr Potts topic buffer");
	};
	auto playMrPottsLine = [&](int wavId, int headVariant) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, "MR_POTTS", headVariant);
	};
	auto playMrsPottsLine = [&](int wavId, int headVariant) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, "MRS_POTTS", headVariant);
	};
	auto playPcLine = [&](int wavId, int headVariant) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, "PC", headVariant);
	};
	auto playPottsFleshLine = [&](int wavId, int headVariant) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, "POTTS_FLESH", headVariant);
	};
	auto playSequence = [&](const DialogueLineEntry *lines, uint count) -> Common::Error {
		return runtime.playDialogueEntrySequence(lines, count);
	};
	auto hasInventoryItem = [&](const char *objectName) {
		return runtime.startupScript().isObjectInInventory(objectName);
	};
	auto executeActionTagIfSet = [&](const char *tag) {
		InteractionResult interaction;
		if (!runtime.executeActionTag(tag, interaction))
			return;

		runtime.applyImmediateDialogueInteractionEffects(interaction);
		runtime.queueDialogueInteractionIfNeeded(interaction);
	};
	auto queueNpcDeathOrMonsterfyTransition = [&](const char *npcName) {
		InteractionResult interaction;
		if (runtime.startupScript().finalizeRuntimeNpcDeathOrMonsterfy(npcName))
			interaction.mutatedRuntimeState = true;
		runtime.queueDialogueInteractionIfNeeded(interaction);
	};
	auto clearScreenToBlack = [&]() {
		Graphics::Screen *screen = runtime.engine().getScreen();
		if (!screen)
			return;

		screen->fillRect(screen->getBounds(), 0);
		screen->update();
	};

	Common::Error lineError = Common::kNoError;
	const int currentStoryDayIndex = runtime.startupScript().getCurrentStoryDayIndex();
	const bool stephanieIsDead = runtime.startupScript().getFlagValue("STEPHANIE_IS_DEAD");

	if (hasInventoryItem("HARVEST_BLADE")) {
		(void)runtime.startupScript().setRuntimeFlagValue("PC_TALKED_TO_POTTS", true);
		lineError = playSequence(kMrPottsHarvestBladeOpeningLines,
			ARRAYSIZE(kMrPottsHarvestBladeOpeningLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x1d2, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playPottsFleshLine(0x2f3d, 4);
		} else if (responseIndex == 2) {
			lineError = playPottsFleshLine(0x2f42, 0);
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = playPottsFleshLine(0x2f46, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playPottsFleshLine(0x2f4d, 4);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		executeActionTagIfSet(kMonsterfyPottsActionTag);
		return Common::kNoError;
	}

	if (runtime.startupScript().getFlagValue("STEPHANIE_DEAD_IN_HALL_JUST_OUTSIDE")) {
		(void)runtime.startupScript().setRuntimeFlagValue(
			"STEPHANIE_DEAD_IN_HALL_JUST_OUTSIDE", false);
		lineError = playSequence(kMrPottsStephanieDeadInHallLines,
			ARRAYSIZE(kMrPottsStephanieDeadInHallLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		sharedState.dialogueStateD2eb8 = true;
		(void)runtime.startupScript().setRuntimeFlagValue("STEPH_MIDGAME_PLAYED", true);
		clearScreenToBlack();
		lineError = runtime.playDialogueFst(kDialogueC001BFstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		executeActionTagIfSet(kStephHallExitActionTag);
		return Common::kNoError;
	}

	if (runtime.startupScript().getFlagValue("POTTSDAM_DIGGING")) {
		if (!state.pottsdamDiggingLinePlayed) {
			state.pottsdamDiggingLinePlayed = true;
			lineError = runtime.playDialogueFst(kDialogueC016AFstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playSequence(kMrPottsPottsdamDiggingIntroLines,
				ARRAYSIZE(kMrPottsPottsdamDiggingIntroLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			return Common::kNoError;
		}

		lineError = playMrPottsLine(0x2cd9, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		return Common::kNoError;
	}

	if (runtime.startupScript().getFlagValue("POTTSDAM_DIGGING_KARIN")) {
		lineError = playSequence(kMrPottsPottsdamDiggingKarinLines,
			ARRAYSIZE(kMrPottsPottsdamDiggingKarinLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		queueNpcDeathOrMonsterfyTransition("MR_POTTS_CEM10");
		return Common::kNoError;
	}

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("MEAT")) {
			lineError = playMrPottsLine(0x2c7f, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x1d4, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = playMrPottsLine(0x2c8a, 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				(void)runtime.startupScript().resetRuntimeObjectToInitialState(kMeatObjectName);
			} else if (responseIndex == 2) {
				lineError = playMrPottsLine(0x2c8f, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			return Common::kNoError;
		} else if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			sharedState.discussedCasketPhotoEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
			lineError = playSequence(kMrPottsCasketPhotoLines, ARRAYSIZE(kMrPottsCasketPhotoLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (sharedState.dialogueStateD2ebc) {
				lineError = playSequence(
					kMrPottsCasketPhotoFollowupLines, ARRAYSIZE(kMrPottsCasketPhotoFollowupLines));
			}
		} else if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			sharedState.discussedWhaleyHerrillPhoto = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
			lineError = playSequence(
				kMrPottsWhaleyHerrillPhotoLines, ARRAYSIZE(kMrPottsWhaleyHerrillPhotoLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x1d5, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = playMrPottsLine(0x2cca, 0);
			} else if (responseIndex == 2) {
				lineError = playMrPottsLine(0x2ccf, 0);
			} else if (responseIndex == 3) {
				lineError = playMrPottsLine(0x2cd4, 0);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playMrPottsLine(0x2cd9, 0);
		} else if (usedItemName.equalsIgnoreCase("NOTE") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
			sharedState.discussedNoteCheckbookEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownEvidenceOfBlackmail, true);
			lineError = playSequence(
				kMrPottsBlackmailEvidenceLines, ARRAYSIZE(kMrPottsBlackmailEvidenceLines));
		} else if (usedItemName.equalsIgnoreCase("TV_DEED") ||
				usedItemName.equalsIgnoreCase("TV_DEED_PHOTOCOPY")) {
			sharedState.discussedTvDeedEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownEvidenceSheriffOwns, true);
			lineError = playSequence(
				kMrPottsTvDeedEvidenceLines, ARRAYSIZE(kMrPottsTvDeedEvidenceLines));
		} else {
			lineError = playMrPottsLine(0x2c79, 0);
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		return Common::kNoError;
	}

	if (runtime.startupScript().getFlagValue("IF_TRY_TO_SEE_STEPHANIE")) {
		lineError = playMrPottsLine(0x2b18, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		return Common::kNoError;
	}

	if (state.auxIntroPending && !stephanieIsDead) {
		state.auxIntroPending = false;
		state.followupDayIndex = currentStoryDayIndex;

		lineError = playMrPottsLine(0x2988, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x1d7, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playSequence(kMrPottsAuxIntroResponse1Lines,
				ARRAYSIZE(kMrPottsAuxIntroResponse1Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			sharedState.dialogueStateD2f04 = true;
			if (sharedState.dialogueStateD2ef4) {
				lineError = playSequence(kMrPottsAuxIntroResponse1WithMomIntroLines,
					ARRAYSIZE(kMrPottsAuxIntroResponse1WithMomIntroLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			lineError = playMrPottsLine(0x29b0, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			assignMrPottsTopicBuffer(
				runtime.startupScript().getFlagValue("STEPHANIE_IS_DEAD") ? 0x1d9 : 0x1d8);
		} else if (responseIndex == 2) {
			lineError = playMrPottsLine(0x29ba, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMrPottsTopicBuffer(
				runtime.startupScript().getFlagValue("STEPHANIE_IS_DEAD") ? 0x1db : 0x1da);
		}
	} else if (currentStoryDayIndex == state.followupDayIndex) {
		lineError = stephanieIsDead
			? playMrPottsLine(0x2aed, 3)
			: playMrPottsLine(0x2ac7, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	} else if (!stephanieIsDead) {
		lineError = playMrPottsLine(0x2ace, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x1dc, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playMrPottsLine(0x2ad8, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			lineError = playMrPottsLine(0x2adc, 4);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		if ((runtime.startupScript().getFlagValue("DAY_2") ||
				runtime.startupScript().getFlagValue("DAY_3") ||
				runtime.startupScript().getFlagValue("DAY_4") ||
				runtime.startupScript().getFlagValue("DAY_5")) &&
				!state.stephanieAliveLaterDayShown) {
			state.stephanieAliveLaterDayShown = true;
			lineError = playMrPottsLine(0x2ae5, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		state.followupDayIndex = runtime.startupScript().getCurrentStoryDayIndex();
	} else {
		lineError = playMrPottsLine(0x2bdf, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("KARIN_KIDNAPED") && !state.karinKidnapedShown) {
		state.karinKidnapedShown = true;

		lineError = playMrPottsLine(0x2b1f, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(stephanieIsDead ? 0x1df : 0x1de, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playSequence(kMrPottsKarinKidnapedResponse1Lines,
				ARRAYSIZE(kMrPottsKarinKidnapedResponse1Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int nestedResponseIndex = 0;
			responseError = runtime.runResponseMenu(0x1e0, nestedResponseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (nestedResponseIndex == 1) {
				lineError = playMrPottsLine(0x2b45, 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else if (nestedResponseIndex == 2) {
				lineError = playMrPottsLine(0x2b4b, 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				int ignoredResponseIndex = 0;
				responseError = runtime.runResponseMenu(0x1e1, ignoredResponseIndex);
				if (responseError.getCode() != Common::kNoError)
					return responseError;

				lineError = playSequence(kMrPottsKarinKidnapedResponse1NestedResponse2Lines,
					ARRAYSIZE(kMrPottsKarinKidnapedResponse1NestedResponse2Lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
		} else if (responseIndex == 2) {
			lineError = runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED")
				? playMrPottsLine(0x2b79, 3)
				: playMrPottsLine(0x2b65, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}

	if (runtime.startupScript().getFlagValue("DAY_6") &&
			runtime.startupScript().getFlagValue("KARIN_FOUND_DEAD") &&
			!state.karinFoundDeadShown) {
		state.karinFoundDeadShown = true;
		lineError = playMrPottsLine(0x2b82, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("KARIN_FOUND_DEAD") && !state.karinFoundAliveShown) {
		state.karinFoundAliveShown = true;
		lineError = playSequence(kMrPottsKarinFoundDeadIntroLines,
			ARRAYSIZE(kMrPottsKarinFoundDeadIntroLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED")
			? playSequence(kMrPottsKarinFoundDeadWithStephMidgameLines,
				ARRAYSIZE(kMrPottsKarinFoundDeadWithStephMidgameLines))
			: playSequence(kMrPottsKarinFoundDeadWithoutStephMidgameLines,
				ARRAYSIZE(kMrPottsKarinFoundDeadWithoutStephMidgameLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x1e2, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playMrPottsLine(0x2bb2, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			lineError = playMrPottsLine(0x2bb6, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}

	if (runtime.startupScript().getFlagValue("DAY_6") &&
			(runtime.startupScript().getFlagValue("KARIN") ||
				runtime.startupScript().getFlagValue("KARIN_FOUND_DEAD")) &&
			!state.day6KarinShown) {
		state.day6KarinShown = true;

		lineError = playMrPottsLine(0x2bbd, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x1e3, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playMrPottsLine(0x2bc8, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			lineError = playMrPottsLine(0x2bd1, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			if (runtime.startupScript().getFlagValue("KARIN_FOUND_ALIVE") &&
					!state.day6KarinFoundAliveFollowupShown) {
				state.day6KarinFoundAliveFollowupShown = true;

				int followupResponseIndex = 0;
				responseError = runtime.runResponseMenu(0x1e4, followupResponseIndex);
				if (responseError.getCode() != Common::kNoError)
					return responseError;

				if (followupResponseIndex == 1) {
					lineError = playMrPottsLine(0x2be9, 2);
					if (lineError.getCode() != Common::kNoError)
						return lineError;

					sharedState.discussedMrPottsTuesdayNightAlibi = true;
					lineError = playSequence(kMrPottsDay6KarinFoundAliveAlibiFollowupLines,
						ARRAYSIZE(kMrPottsDay6KarinFoundAliveAlibiFollowupLines));
					if (lineError.getCode() != Common::kNoError)
						return lineError;

					const bool discussedMoynahanKarinKidnaped =
						sharedState.moynahanKarinKidnapedDiscussionState;
					int nestedResponseIndex = 0;
					responseError = runtime.runResponseMenu(
						discussedMoynahanKarinKidnaped ? 0x1e5 : 0x1e6,
						nestedResponseIndex);
					if (responseError.getCode() != Common::kNoError)
						return responseError;

					if (nestedResponseIndex == 1) {
						lineError = discussedMoynahanKarinKidnaped
							? playMrPottsLine(0x2c15, 2)
							: playSequence(kMrPottsDay6KarinFoundAliveNoMoynahanLines,
								ARRAYSIZE(kMrPottsDay6KarinFoundAliveNoMoynahanLines));
					} else if (nestedResponseIndex == 2) {
						lineError = playMrsPottsLine(0x2c24, 2);
					}
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				} else if (followupResponseIndex == 2) {
					lineError = playMrPottsLine(0x2c2c, 0);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				}
			}
		}
	}

	if (runtime.startupScript().getFlagValue("SCRATCHED_TUCKER") && !state.scratchedTuckerShown) {
		state.scratchedTuckerShown = true;
		lineError = playMrPottsLine(0x2e54, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("BARBER_POLE_STOLEN") && !state.barberPoleStolenShown) {
		state.barberPoleStolenShown = true;
		lineError = playMrPottsLine(0x2e61, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("BOLT_OF_CLOTH_TAKEN") && !state.boltOfClothTakenShown) {
		state.boltOfClothTakenShown = true;
		lineError = playMrPottsLine(0x2e69, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("DINER_BURNED") && !state.dinerBurnedShown) {
		state.dinerBurnedShown = true;
		lineError = playMrPottsLine(0x2e72, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = runtime.startupScript().getFlagValue("KARIN_KIDNAPED")
			? playMrPottsLine(0x2e77, 0)
			: playMrPottsLine(0x2e7c, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("GOT_REMAINS_FOR_LODGE") &&
			!state.gotRemainsForLodgeShown) {
		state.gotRemainsForLodgeShown = true;

		lineError = playMrPottsLine(0x2e82, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x1e7, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playMrPottsLine(0x2e8d, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			lineError = playMrPottsLine(0x2e91, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		lineError = playMrPottsLine(0x2e95, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("PC_ESCAPED_JAIL") && !state.escapedJailShown) {
		state.escapedJailShown = true;
		lineError = playMrPottsLine(0x2e9c, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (!runtime.startupScript().isNamedNpcDeathTypeClear("BUTCHER") && !state.butcherDeathShown) {
		state.butcherDeathShown = true;
		lineError = playMrPottsLine(0x2ef2, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (!runtime.startupScript().isNamedNpcDeathTypeClear("MOYNAHAN") && !state.moynahanDeathShown) {
		state.moynahanDeathShown = true;
		lineError = playMrPottsLine(0x2f08, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("EDNA_HUNG") && !state.ednaHungShown) {
		state.ednaHungShown = true;
		lineError = playMrPottsLine(0x2f0f, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		if (runtime.startupScript().getFlagValue("KARIN_HUNG")) {
			lineError = playMrPottsLine(0x2f14, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}

	restoreMrPottsTopicBuffer();
	if (mrPottsTopicBufferLineIndex < 0)
		return Common::kNoError;

	Common::String selectedTopic;
	for (;;) {
		if (runtime.matchesResponseLine(selectedTopic, 0x1e9))
			return Common::kNoError;

		Common::Error keywordError = runtime.runKeywordMenu(
			mrPottsTopicBuffer, mrPottsTopicBufferLineIndex, selectedTopic);
		if (keywordError.getCode() != Common::kNoError)
			return keywordError;
		if (runtime.matchesResponseLine(selectedTopic, 0x1e9))
			return Common::kNoError;

		if (runtime.matchesResponseLine(selectedTopic, 0x1ea)) {
			if (!runtime.startupScript().getFlagValue("STEPHANIE_IS_DEAD")) {
				lineError = playSequence(kMrPottsAmnesiaTopicLines, ARRAYSIZE(kMrPottsAmnesiaTopicLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignMrPottsTopicBuffer(0x1eb);
			}
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x1ec)) {
			if (!runtime.startupScript().getFlagValue("STEPHANIE_IS_DEAD")) {
				lineError = playSequence(kMrPottsMarriageTopicLines, ARRAYSIZE(kMrPottsMarriageTopicLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignMrPottsTopicBuffer(0x1ed);
			}
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x1ee)) {
			if (!runtime.startupScript().getFlagValue("STEPHANIE_IS_DEAD")) {
				lineError = playSequence(
					kMrPottsCommitmentTopicLines, ARRAYSIZE(kMrPottsCommitmentTopicLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignMrPottsTopicBuffer(0x1ef);
			}
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x1f0)) {
			if (sharedState.dialogueStateD2ed0) {
				lineError = playMrPottsLine(0x2a0d, 0);
			} else if (runtime.startupScript().getFlagValue("HEARD_DAD_MOAN")) {
				lineError = playMrPottsLine(0x2a09, 0);
			} else {
				lineError = playMrPottsLine(0x2a08, 0);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playSequence(kMrPottsFatherTopicLines, ARRAYSIZE(kMrPottsFatherTopicLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMrPottsTopicBuffer(
				runtime.startupScript().getFlagValue("STEPHANIE_IS_DEAD") ? 0x1f2 : 0x1f1);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x1f3)) {
			lineError = playMrPottsLine(0x2a31, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x1f4, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = playMrPottsLine(0x2a3c, 2);
			} else if (responseIndex == 2) {
				lineError = playMrPottsLine(0x2a42, 1);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playSequence(kMrPottsSlaughterhouseTopicClosingLines,
				ARRAYSIZE(kMrPottsSlaughterhouseTopicClosingLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMrPottsTopicBuffer(
				runtime.startupScript().getFlagValue("STEPHANIE_IS_DEAD") ? 0x1f6 : 0x1f5);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x1f7)) {
			if (!runtime.startupScript().getFlagValue("STEPHANIE_IS_DEAD")) {
				lineError = playMrPottsLine(0x2a5e, 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignMrPottsTopicBuffer(0x1f8);
			}
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x1f9)) {
			if (!runtime.startupScript().getFlagValue("STEPHANIE_IS_DEAD")) {
				int responseIndex = 0;
				Common::Error responseError = runtime.runResponseMenu(0x1fa, responseIndex);
				if (responseError.getCode() != Common::kNoError)
					return responseError;

				if (responseIndex == 1) {
					lineError = playMrPottsLine(0x2a73, 0);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				} else if (responseIndex == 2) {
					lineError = playMrPottsLine(0x2a7c, 0);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				}

				lineError = playMrPottsLine(0x2a81, 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignMrPottsTopicBuffer(0x1fb);
			}
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x1fd)) {
			if (!runtime.startupScript().getFlagValue("STEPHANIE_IS_DEAD")) {
				sharedState.discussedLodgeTopic = true;
				lineError = playSequence(kMrPottsLodgeTopicLines, ARRAYSIZE(kMrPottsLodgeTopicLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignMrPottsTopicBuffer(0x1fe);
			}
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x1ff)) {
			lineError = playMrPottsLine(0x2aaf, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMrPottsTopicBuffer(0x200);
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMrPottsMoynahanTopicResponseLines,
				ARRAYSIZE(kMrPottsMoynahanTopicResponseLines))) {
			if (!runtime.startupScript().getFlagValue("STEPHANIE_IS_DEAD")) {
				lineError = playMrPottsLine(0x2abb, 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignMrPottsTopicBuffer(0x203);
			}
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMrPottsMurderTopicResponseLines,
				ARRAYSIZE(kMrPottsMurderTopicResponseLines))) {
			if (!runtime.startupScript().getFlagValue("KARIN_KIDNAPED") ||
					runtime.startupScript().getFlagValue("KARIN_FOUND_ALIVE") ||
					runtime.startupScript().getFlagValue("KARIN_FOUND_DEAD")) {
				lineError = playMrPottsLine(0x2bd9, 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				continue;
			}

			lineError = playSequence(kMrPottsMurderAccusationOpeningLines,
				ARRAYSIZE(kMrPottsMurderAccusationOpeningLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			const bool hasMoynahanAccusation = sharedState.moynahanKarinKidnapedDiscussionState;
			const bool hasWeddingMatchesAccusation = hasInventoryItem(kWeddingMatchesObjectName);
			if (!hasMoynahanAccusation && !hasWeddingMatchesAccusation) {
				lineError = playPcLine(0x2e46, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playMrPottsLine(0x2e4a, 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				continue;
			}

			Common::String accusationMenuLine = runtime.startupText().getDialogueResponseLine(0x20f);
			int nextResponseIndex = 2;
			int moynahanResponseIndex = 0;
			int weddingMatchesResponseIndex = 0;
			if (hasMoynahanAccusation) {
				moynahanResponseIndex = nextResponseIndex++;
				accusationMenuLine = Common::String::format(
					runtime.startupText().getDialogueResponseLine(0x210).c_str(),
					accusationMenuLine.c_str(), kMrPottsMoynahanAccusationResponseText);
			}
			if (hasWeddingMatchesAccusation) {
				weddingMatchesResponseIndex = nextResponseIndex++;
				accusationMenuLine = Common::String::format(
					runtime.startupText().getDialogueResponseLine(0x212).c_str(),
					accusationMenuLine.c_str(), weddingMatchesResponseIndex,
					kMrPottsWeddingMatchesResponseText);
			}

			int accusationResponseIndex = 0;
			Common::Error responseError = runtime.runResponseMenuText(
				accusationMenuLine, accusationResponseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (accusationResponseIndex == 1) {
				lineError = playMrPottsLine(0x2d7a, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else if (accusationResponseIndex == moynahanResponseIndex) {
				lineError = playMrPottsLine(0x2d80, 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				int responseIndex = 1;
				if (sharedState.discussedMrPottsTuesdayNightAlibi) {
					responseError = runtime.runResponseMenu(0x213, responseIndex);
					if (responseError.getCode() != Common::kNoError)
						return responseError;
				}

				if (responseIndex == 1) {
					lineError = playMrPottsLine(0x2d8e, 2);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				} else if (responseIndex == 2) {
					lineError = playMrPottsLine(0x2d93, 2);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				}
			} else if (accusationResponseIndex == weddingMatchesResponseIndex) {
				lineError = playSequence(kMrPottsWeddingMatchesAccusationLines,
					ARRAYSIZE(kMrPottsWeddingMatchesAccusationLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x214)) {
			lineError = playMrPottsLine(0x2dd3, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x215, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = playMrPottsLine(0x2ddf, 1);
			} else if (responseIndex == 2) {
				lineError = playMrPottsLine(0x2de4, 0);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMrPottsMolestTopicResponseLines,
				ARRAYSIZE(kMrPottsMolestTopicResponseLines))) {
			lineError = playMrPottsLine(0x2def, 4);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x219, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = playSequence(kMrPottsMolestResponse1OpeningLines,
					ARRAYSIZE(kMrPottsMolestResponse1OpeningLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				responseIndex = 0;
				responseError = runtime.runResponseMenu(0x21a, responseIndex);
				if (responseError.getCode() != Common::kNoError)
					return responseError;

				if (responseIndex == 1) {
					lineError = playMrPottsLine(0x2e14, 3);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				} else if (responseIndex == 2) {
					lineError = playSequence(kMrPottsMolestResponse2IntroLines,
						ARRAYSIZE(kMrPottsMolestResponse2IntroLines));
					if (lineError.getCode() != Common::kNoError)
						return lineError;

					for (;;) {
						lineError = playMrPottsLine(0x2e2b, 3);
						if (lineError.getCode() != Common::kNoError)
							return lineError;

						int followupResponseIndex = 0;
						responseError = runtime.runResponseMenu(0x21b, followupResponseIndex);
						if (responseError.getCode() != Common::kNoError)
							return responseError;

						if (followupResponseIndex == 1)
							continue;
						if (followupResponseIndex == 2) {
							lineError = playMrPottsLine(0x2e2f, 3);
							if (lineError.getCode() != Common::kNoError)
								return lineError;
							lineError = playPcLine(0x2e34, 2);
							if (lineError.getCode() != Common::kNoError)
								return lineError;
						}
						break;
					}
				}
			} else if (responseIndex == 2) {
				lineError = playSequence(kMrPottsMolestAlternativeOpeningLines,
					ARRAYSIZE(kMrPottsMolestAlternativeOpeningLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x21c)) {
			selectedTopic.clear();
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMrPottsPeepholeTopicResponseLines,
				ARRAYSIZE(kMrPottsPeepholeTopicResponseLines))) {
			if (runtime.startupScript().getFlagValue("STEPHANIE_IS_DEAD")) {
				lineError = playMrPottsLine(0x2bd9, 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				continue;
			}

			lineError = playSequence(kMrPottsPeepholeOpeningLines,
				ARRAYSIZE(kMrPottsPeepholeOpeningLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x20b, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = playMrPottsLine(0x2d14, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else if (responseIndex == 2) {
				lineError = playSequence(kMrPottsPeepholeResponse2Lines,
					ARRAYSIZE(kMrPottsPeepholeResponse2Lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else if (responseIndex == 3) {
				lineError = playMrPottsLine(0x2d28, 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			lineError = playSequence(kMrPottsPeepholeClosingLines,
				ARRAYSIZE(kMrPottsPeepholeClosingLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			sharedState.confrontedMrPottsAboutSpyhole = true;
			lineError = playSequence(kMrPottsPeepholeFollowupLines,
				ARRAYSIZE(kMrPottsPeepholeFollowupLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			selectedTopic.clear();
			continue;
		}

		lineError = playMrPottsLine(0x2c73, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
}

} // End of namespace Harvester
