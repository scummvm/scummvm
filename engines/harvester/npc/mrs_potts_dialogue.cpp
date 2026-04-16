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


#include "harvester/npc/mrs_potts_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"
#include "harvester/harvester.h"

namespace Harvester {

namespace {

static const char *const kDialogueC040FstPath = "GRAPHIC/FST/C040.FST";
static const char *const kDieInChairActionTag = "DIE_IN_CHAIR";
static const char *const kStephHallExitActionTag = "STEF_HALL_EXIT";
static const char *const kPermissionToSeeStephanieFlag = "PERMISSION_TO_SEE_STEPHANIE";

static const DialogueLineEntry kMrsPottsWhaleyLines[] = {
	{ 0x293c, "MRS_POTTS", 1 },
	{ 0x2940, "PC", 1 },
	{ 0x2945, "MRS_POTTS", 1 }
};

static const DialogueLineEntry kMrsPottsBlackmailLines[] = {
	{ 0x2954, "MRS_POTTS", 0 },
	{ 0x2959, "PC", 0 },
	{ 0x295e, "MRS_POTTS", 0 },
	{ 0x2962, "PC", 0 },
	{ 0x2966, "MRS_POTTS", 1 }
};

static const DialogueLineEntry kMrsPottsTvDeedLines[] = {
	{ 0x296e, "MRS_POTTS", 1 },
	{ 0x2973, "PC", 0 },
	{ 0x2977, "MRS_POTTS", 0 }
};

static const DialogueLineEntry kMrsPottsIntroResponse1Lines[] = {
	{ 0x2730, "MRS_POTTS", 2 },
	{ 0x2735, "PC", 0 },
	{ 0x273a, "MRS_POTTS", 0 }
};

static const DialogueLineEntry kMrsPottsIntroResponse2Lines[] = {
	{ 0x2744, "MRS_POTTS", 0 },
	{ 0x2748, "PC", 0 },
	{ 0x274c, "MRS_POTTS", 0 }
};

static const DialogueLineEntry kMrsPottsPearlsLines[] = {
	{ 0x2756, "MRS_POTTS", 0 },
	{ 0x275d, "PC", 0 },
	{ 0x2762, "MRS_POTTS", 0 },
	{ 0x2766, "PC", 0 },
	{ 0x276b, "MRS_POTTS", 0 }
};

static const DialogueLineEntry kMrsPottsBakeSaleLines[] = {
	{ 0x2775, "PC", 0 },
	{ 0x2779, "MRS_POTTS", 3 },
	{ 0x277f, "PC", 0 },
	{ 0x2783, "MRS_POTTS", 0 }
};

static const DialogueLineEntry kMrsPottsWeddingLines[] = {
	{ 0x278c, "PC", 0 },
	{ 0x2790, "MRS_POTTS", 0 },
	{ 0x2795, "MRS_POTTS", 0 },
	{ 0x2799, "PC", 0 }
};

static const DialogueLineEntry kMrsPottsGroundedIntroLines[] = {
	{ 0x27a3, "MRS_POTTS", 3 },
	{ 0x27a8, "PC", 0 },
	{ 0x27ac, "MRS_POTTS", 2 },
	{ 0x27b0, "PC", 2 },
	{ 0x27b5, "MRS_POTTS", 2 }
};

static const DialogueLineEntry kMrsPottsGotRemainsIntroLines[] = {
	{ 0x289e, "MRS_POTTS", 2 },
	{ 0x28a8, "MRS_POTTS", 2 },
	{ 0x28ad, "MRS_POTTS", 2 }
};

static const DialogueLineEntry kMrsPottsRemainsDeathLines[] = {
	{ 0x28b7, "MRS_POTTS", 1 },
	{ 0x28bd, "PC", 0 },
	{ 0x28c1, "MRS_POTTS", 1 }
};

static const DialogueLineEntry kMrsPottsKarinKidnapedResponse1Lines[] = {
	{ 0x28e8, "MRS_POTTS", 0 },
	{ 0x28ee, "PC", 0 },
	{ 0x28f2, "MRS_POTTS", 2 }
};

static const DialogueLineEntry kMrsPottsKarinKidnapedResponse2Lines[] = {
	{ 0x28fb, "MRS_POTTS", 3 },
	{ 0x2903, "PC", 0 },
	{ 0x2907, "MRS_POTTS", 2 }
};

static const DialogueLineEntry kMrsPottsLodgeLines[] = {
	{ 0x27ef, "PC", 0 },
	{ 0x27f3, "MRS_POTTS", 1 }
};

static const int kMrsPottsTopic27d6ResponseLines[] = { 0x241, 0x242 };
static const int kMrsPottsTopic27e3ResponseLines[] = { 0x244, 0x245 };
static const int kMrsPottsLodgeTopicResponseLines[] = { 0x247, 0x248 };

} // End of namespace

bool MrsPottsDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("MRS_POTTS") ||
		npcName.equalsIgnoreCase("MRS_POTTS_ST_BEDRM");
}

Common::Error MrsPottsDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	MrsPottsRoomDialogueState &state = _state;
	Common::String mrsPottsTopicBuffer;
	int mrsPottsTopicBufferLineIndex = -1;

	auto assignMrsPottsTopicBuffer = [&](int responseLineIndex) {
		runtime.assignTopicBuffer(mrsPottsTopicBuffer, mrsPottsTopicBufferLineIndex,
			responseLineIndex, "Mrs Potts topic buffer");
	};
	auto playMrsPottsLine = [&](int wavId, int headVariant = 0,
			const char *speakerId = "MRS_POTTS") -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, speakerId, headVariant);
	};
	auto playSequence = [&](const DialogueLineEntry *lines, uint count) -> Common::Error {
		return runtime.playDialogueEntrySequence(lines, count);
	};
	auto executeDialogueActionTag = [&](const char *tag) {
		InteractionResult interaction;
		if (runtime.executeActionTag(tag, interaction)) {
			runtime.applyImmediateDialogueInteractionEffects(interaction);
			runtime.queueDialogueInteractionIfNeeded(interaction);
		}
	};
	auto clearScreenToBlack = [&]() {
		Graphics::Screen *screen = runtime.engine().getScreen();
		if (!screen)
			return;

		screen->fillRect(screen->getBounds(), 0);
		screen->update();
	};
	auto playRemainsDeathSequence = [&]() -> Common::Error {
		Common::Error lineError = playSequence(
			kMrsPottsRemainsDeathLines, ARRAYSIZE(kMrsPottsRemainsDeathLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		clearScreenToBlack();
		lineError = runtime.playDialogueFst(kDialogueC040FstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		executeDialogueActionTag(kDieInChairActionTag);
		return Common::kNoError;
	};

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			sharedState.discussedWhaleyHerrillPhoto = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
			return playSequence(kMrsPottsWhaleyLines, ARRAYSIZE(kMrsPottsWhaleyLines));
		}
		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			sharedState.discussedCasketPhotoEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
			return playMrsPottsLine(0x294d, 3);
		}
		if ((usedItemName.equalsIgnoreCase("LEDGER") ||
				usedItemName.equalsIgnoreCase("LEDGER2")) &&
				runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
			sharedState.discussedLedgerEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownLedgersToAnyone, true);
			return playMrsPottsLine(0x294d, 3);
		}
		if (usedItemName.equalsIgnoreCase("NOTE") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
			sharedState.discussedNoteCheckbookEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownEvidenceOfBlackmail, true);
			return playSequence(kMrsPottsBlackmailLines, ARRAYSIZE(kMrsPottsBlackmailLines));
		}
		if (usedItemName.equalsIgnoreCase("TV_DEED") ||
				usedItemName.equalsIgnoreCase("TV_DEED_PHOTOCOPY")) {
			sharedState.discussedTvDeedEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(
				DialogueFlags::kShownEvidenceSheriffOwns, true);
			return playSequence(kMrsPottsTvDeedLines, ARRAYSIZE(kMrsPottsTvDeedLines));
		}
		if (usedItemName.equalsIgnoreCase("REMAINS"))
			return playRemainsDeathSequence();

		return playMrsPottsLine(0x2936);
	}

	if (runtime.startupScript().getFlagValue("STEPHANIE_IS_DEAD")) {
		Common::Error lineError = Common::kNoError;
		if (!sharedState.dialogueStateD2eb8) {
			lineError = playMrsPottsLine(0x283b, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x230, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1)
				return playMrsPottsLine(0x2846);
			if (responseIndex == 2)
				return playMrsPottsLine(0x284b, 3);
			return Common::kNoError;
		}

		if (!runtime.currentRoomName().equalsIgnoreCase("STKITCHN")) {
			lineError = playMrsPottsLine(0x2854, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x22f, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1)
				return playMrsPottsLine(0x285e, 3);
			if (responseIndex == 2)
				return playMrsPottsLine(0x2864);
			return Common::kNoError;
		}

		if (runtime.startupScript().getFlagValue("GOT_REMAINS_FOR_LODGE") &&
				!state.gotRemainsForLodgeLinePlayed) {
			state.gotRemainsForLodgeLinePlayed = true;
			lineError = playSequence(
				kMrsPottsGotRemainsIntroLines, ARRAYSIZE(kMrsPottsGotRemainsIntroLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x22d, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1)
				return playRemainsDeathSequence();
			if (responseIndex == 2)
				return playMrsPottsLine(0x28d6, 1);
			return Common::kNoError;
		}

		if (runtime.startupScript().getFlagValue("TAKEN_INVITE_TO_SERGEANT") &&
				!state.takenInviteToSergeantLinePlayed) {
			state.takenInviteToSergeantLinePlayed = true;
			lineError = playMrsPottsLine(0x286b, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x22e, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1) {
				lineError = playMrsPottsLine(0x2876, 3);
			} else if (responseIndex == 2) {
				lineError = playMrsPottsLine(0x287a, 3);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			return playMrsPottsLine(0x2881, 3);
		}

		return playMrsPottsLine(0x2835, 1);
	}

	if (state.auxIntroPending) {
		state.auxIntroPending = false;
		state.returnVisitFollowupPending = true;
		Common::Error lineError = playMrsPottsLine(0x2726, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x231, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;
		if (responseIndex == 1) {
			lineError = playSequence(
				kMrsPottsIntroResponse1Lines, ARRAYSIZE(kMrsPottsIntroResponse1Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMrsPottsTopicBuffer(0x232);
		} else if (responseIndex == 2) {
			lineError = playSequence(
				kMrsPottsIntroResponse2Lines, ARRAYSIZE(kMrsPottsIntroResponse2Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMrsPottsTopicBuffer(0x233);
		}
	} else if (!state.returnVisitFollowupPending) {
		const int storyDayIndex = runtime.startupScript().getCurrentStoryDayIndex();
		if (storyDayIndex > 1 && storyDayIndex < 6) {
			state.returnVisitFollowupPending = true;
			Common::Error lineError = playMrsPottsLine(0x281b, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playMrsPottsLine(
				runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") ? 0x2825 : 0x2820,
				runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") ? 0 : 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else {
			state.returnVisitFollowupPending = true;
			Common::Error lineError = playMrsPottsLine(0x282d, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			(void)runtime.startupScript().setRuntimeFlagValue(kPermissionToSeeStephanieFlag, true);
			executeDialogueActionTag(kStephHallExitActionTag);
		}
	} else {
		state.returnVisitFollowupPending = false;
		Common::Error lineError = playMrsPottsLine(0x2800, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x234, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;
		if (responseIndex == 1) {
			lineError = playMrsPottsLine(0x280a, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			(void)runtime.startupScript().setRuntimeFlagValue(kPermissionToSeeStephanieFlag, true);
			executeDialogueActionTag(kStephHallExitActionTag);
		} else if (responseIndex == 2) {
			lineError = playMrsPottsLine(0x2811);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}

	if (runtime.startupScript().getCurrentStoryDayIndex() == 5 && !state.day5LinePlayed) {
		state.day5LinePlayed = true;
		Common::Error lineError = playMrsPottsLine(0x2889, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("BURNED_TV_STATION") &&
			!state.burnedTvStationLinePlayed) {
		state.burnedTvStationLinePlayed = true;
		Common::Error lineError = playMrsPottsLine(0x2891, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("KARIN_KIDNAPED") &&
			!state.karinKidnappedLinePlayed) {
		state.karinKidnappedLinePlayed = true;
		sharedState.karinKidnapedDialogueState = true;

		Common::Error lineError = playMrsPottsLine(0x28dc);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x235, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;
		if (responseIndex == 1) {
			lineError = playSequence(kMrsPottsKarinKidnapedResponse1Lines,
				ARRAYSIZE(kMrsPottsKarinKidnapedResponse1Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			sharedState.discussedMrsPottsTuesdayNightAlibi = true;
		} else if (responseIndex == 2) {
			lineError = playSequence(kMrsPottsKarinKidnapedResponse2Lines,
				ARRAYSIZE(kMrsPottsKarinKidnapedResponse2Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}
	if (runtime.startupScript().getFlagValue("DINER_BURNED") &&
			!state.dinerBurnedLinePlayed) {
		state.dinerBurnedLinePlayed = true;
		Common::Error lineError = playMrsPottsLine(0x290e, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		if (runtime.startupScript().getFlagValue("KARIN_HUNG")) {
			lineError = playMrsPottsLine(0x2914, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		lineError = playMrsPottsLine(0x2918);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x236, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;
		if (responseIndex == 1) {
			lineError = playMrsPottsLine(0x2926);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			return playRemainsDeathSequence();
		}
	}

	Common::String selectedTopic;
	for (;;) {
		Common::Error menuError = runtime.runKeywordMenu(
			mrsPottsTopicBuffer, mrsPottsTopicBufferLineIndex, selectedTopic);
		if (menuError.getCode() != Common::kNoError)
			return menuError;
		if (runtime.matchesResponseLine(selectedTopic, 0x237)) {
			return playMrsPottsLine(0x297e);
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x238)) {
			Common::Error lineError = playSequence(
				kMrsPottsPearlsLines, ARRAYSIZE(kMrsPottsPearlsLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMrsPottsTopicBuffer(0x239);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x23a)) {
			Common::Error lineError = playSequence(
				kMrsPottsBakeSaleLines, ARRAYSIZE(kMrsPottsBakeSaleLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMrsPottsTopicBuffer(0x23b);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x23c)) {
			Common::Error lineError = playSequence(
				kMrsPottsWeddingLines, ARRAYSIZE(kMrsPottsWeddingLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMrsPottsTopicBuffer(0x23d);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x23e)) {
			Common::Error lineError = playSequence(
				kMrsPottsGroundedIntroLines, ARRAYSIZE(kMrsPottsGroundedIntroLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x23f, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1) {
				lineError = playMrsPottsLine(0x27c3, 1);
			} else if (responseIndex == 2) {
				lineError = playMrsPottsLine(0x27c7, 2, "PC");
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playMrsPottsLine(0x27cc, 1);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			(void)runtime.startupScript().setRuntimeFlagValue(kPermissionToSeeStephanieFlag, true);
			executeDialogueActionTag(kStephHallExitActionTag);
			assignMrsPottsTopicBuffer(0x240);
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMrsPottsTopic27d6ResponseLines,
				ARRAYSIZE(kMrsPottsTopic27d6ResponseLines))) {
			Common::Error lineError = playMrsPottsLine(0x27d6, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMrsPottsTopicBuffer(0x243);
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMrsPottsTopic27e3ResponseLines,
				ARRAYSIZE(kMrsPottsTopic27e3ResponseLines))) {
			Common::Error lineError = playMrsPottsLine(0x27e3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMrsPottsTopicBuffer(0x246);
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMrsPottsLodgeTopicResponseLines,
				ARRAYSIZE(kMrsPottsLodgeTopicResponseLines))) {
			sharedState.discussedLodgeTopic = true;
			Common::Error lineError = playSequence(
				kMrsPottsLodgeLines, ARRAYSIZE(kMrsPottsLodgeLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMrsPottsTopicBuffer(0x249);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x24a))
			continue;
		Common::Error lineError = playMrsPottsLine(0x2930);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
}

} // End of namespace Harvester
