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


#include "harvester/npc/phelps_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kGetPrnMagActionTag = "GET_PRN_MAG";
static const char *const kDeleteDirtyMagazineActionTag = "DEL_DRT_MAG";
static const char *const kUseRewardAtStoreActionTag = "USE_REWARD_AT_STORE";
static const char *const kSetGenCuStuffActionTag = "SET_GEN_CU_STUFF";

static const DialogueLineEntry kPhelpsWhaleyLines[] = {
	{ 0x1c0c, "PHELPS", 4 },
	{ 0x1c13, "PC", 0 },
	{ 0x1c17, "PHELPS", 4 }
};

static const DialogueLineEntry kPhelpsCasketLines[] = {
	{ 0x1c1e, "PHELPS", 2 },
	{ 0x1c22, "PC", 0 },
	{ 0x1c27, "PHELPS", 0 },
	{ 0x1c2c, "PC", 0 },
	{ 0x1c30, "PHELPS", 2 }
};

static const DialogueLineEntry kPhelpsQuarterLines[] = {
	{ 0x1ba4, "PHELPS", 4 },
	{ 0x1bac, "PC", 0 },
	{ 0x1bb0, "PHELPS", 1 }
};

static const DialogueLineEntry kPhelpsIntroPreludeLines[] = {
	{ 0x1b44, "PHELPS", 1 },
	{ 0x1b48, "PC", 0 },
	{ 0x1b4c, "PHELPS", 0 },
	{ 0x1b52, "PC", 0 },
	{ 0x1b56, "PHELPS", 1 }
};

static const DialogueLineEntry kPhelpsKarinFoundDeadLines[] = {
	{ 0x1d2a, "PHELPS", 3 },
	{ 0x1d33, "PHELPS", 0 },
	{ 0x1d3a, "PHELPS", 0 }
};

static const DialogueLineEntry kPhelpsKarinKidnapedLines[] = {
	{ 0x1d40, "PHELPS", 4 },
	{ 0x1d46, "PC", 0 },
	{ 0x1d4a, "PHELPS", 0 },
	{ 0x1d4f, "PC", 0 },
	{ 0x1d53, "PHELPS", 0 }
};

} // End of namespace

bool PhelpsDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("PHELPS");
}

Common::Error PhelpsDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	PhelpsRoomDialogueState &state = _state;

	auto executeDialogueActionTag = [&](const char *tag) {
		InteractionResult interaction;
		if (runtime.executeActionTag(tag, interaction)) {
			runtime.applyImmediateDialogueInteractionEffects(interaction);
			runtime.queueDialogueInteractionIfNeeded(interaction);
		}
	};
	auto playPhelpsLine = [&](int wavId, int headVariant = 0,
			const char *speakerId = "PHELPS") -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, speakerId, headVariant);
	};
	auto playSequence = [&](const DialogueLineEntry *lines, uint count) -> Common::Error {
		return runtime.playDialogueEntrySequence(lines, count);
	};

	if (usedItemName.empty()) {
		if (state.talkStatePending) {
			state.talkStatePending = false;
			Common::Error lineError = playSequence(kPhelpsIntroPreludeLines,
				ARRAYSIZE(kPhelpsIntroPreludeLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			sharedState.dialogueStateD2f04 = true;
			return playPhelpsLine(0x1b5a, 0, "PC");
		}

		Common::Error lineError = playPhelpsLine(0x1bf4, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		if (runtime.startupScript().getCurrentStoryDayIndex() < 5) {
			lineError = playPhelpsLine(0x1b7b, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x22a, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1) {
				lineError = playPhelpsLine(0x1b85);
			} else {
				lineError = playPhelpsLine(0x1bbc, 1);
				if (lineError.getCode() == Common::kNoError)
					lineError = playPhelpsLine(0x1c44, 1);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") && !state.stephMidgameShown) {
			state.stephMidgameShown = true;
			lineError = playPhelpsLine(0x1c4a, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getCurrentStoryDayIndex() == 5 && !state.dayFiveShown) {
			state.dayFiveShown = true;
			lineError = playPhelpsLine(0x1c56, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("SCRATCHED_TUCKER") && !state.scratchedTuckerShown) {
			state.scratchedTuckerShown = true;
			lineError = playPhelpsLine(0x1c65, 4);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("BARBER_POLE_STOLEN") &&
				!state.barberPoleStolenShown) {
			state.barberPoleStolenShown = true;
			lineError = playPhelpsLine(0x1c6e, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playPhelpsLine(0x1c73, 4);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("BOLT_OF_CLOTH_TAKEN") &&
				!state.boltOfClothTakenShown) {
			state.boltOfClothTakenShown = true;
			lineError = playPhelpsLine(0x1c80);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (runtime.startupScript().isNamedNpcDeathTypeClear("FIREMAN2") &&
					runtime.startupScript().isNamedNpcDeathTypeClear("SPARKY")) {
				lineError = playPhelpsLine(0x1c87);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
		}
		if (runtime.startupScript().getFlagValue("DINER_BURNED") && !state.dinerBurnedShown) {
			state.dinerBurnedShown = true;
			lineError = playPhelpsLine(0x1c8f);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("PC_ESCAPED_JAIL") && !state.escapedJailShown) {
			state.escapedJailShown = true;
			lineError = playPhelpsLine(0x1c98, 4);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("GOT_REMAINS_FOR_LODGE") &&
				runtime.startupScript().isNamedNpcDeathTypeClear("MOYNAHAN") &&
				!state.gotRemainsForLodgeShown) {
			state.gotRemainsForLodgeShown = true;
			lineError = playPhelpsLine(0x1ca0, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("BURNED_TV_STATION") &&
				!state.burnedTvStationShown) {
			state.burnedTvStationShown = true;
			lineError = playPhelpsLine(0x1cb4);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (!runtime.startupScript().isNamedNpcDeathTypeClear("BUTCHER") &&
				!state.butcherDeadShown) {
			state.butcherDeadShown = true;
			lineError = playPhelpsLine(0x1cfe, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (!runtime.startupScript().isNamedNpcDeathTypeClear("MOYNAHAN") &&
				!state.moynahanDeadShown) {
			state.moynahanDeadShown = true;
			lineError = playPhelpsLine(0x1d06, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (!runtime.startupScript().isNamedNpcDeathTypeClear("JIMMY") &&
				!state.jimmyDeadShown) {
			state.jimmyDeadShown = true;
			lineError = playPhelpsLine(0x1d0f);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x22c, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1) {
				lineError = playPhelpsLine(0x1d1c, 1);
			} else if (responseIndex == 2) {
				lineError = playPhelpsLine(0x1d21);
			} else {
				lineError = Common::kNoError;
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("KARIN_FOUND_DEAD") &&
				!state.karinFoundDeadShown) {
			state.karinFoundDeadShown = true;
			return playSequence(kPhelpsKarinFoundDeadLines, ARRAYSIZE(kPhelpsKarinFoundDeadLines));
		}
		if (runtime.startupScript().getFlagValue("KARIN_KIDNAPED") &&
				!state.karinKidnapedShown) {
			state.karinKidnapedShown = true;
			sharedState.karinKidnapedDialogueState = true;
			return playSequence(kPhelpsKarinKidnapedLines, ARRAYSIZE(kPhelpsKarinKidnapedLines));
		}

		return Common::kNoError;
	}

	if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
		return playSequence(kPhelpsWhaleyLines, ARRAYSIZE(kPhelpsWhaleyLines));
	}
	if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
			usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
		return playSequence(kPhelpsCasketLines, ARRAYSIZE(kPhelpsCasketLines));
	}
	if ((usedItemName.equalsIgnoreCase("LEDGER") ||
				usedItemName.equalsIgnoreCase("LEDGER2")) &&
			runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownLedgersToAnyone, true);
		return playPhelpsLine(0x1c36);
	}
	if (usedItemName.equalsIgnoreCase("NOTE") ||
			usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK") ||
			usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
		(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
		return playPhelpsLine(0x1c3d, 1);
	}
	if (usedItemName.equalsIgnoreCase("QUARTER")) {
		executeDialogueActionTag(kGetPrnMagActionTag);
		Common::Error lineError = playSequence(kPhelpsQuarterLines, ARRAYSIZE(kPhelpsQuarterLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		executeDialogueActionTag(kDeleteDirtyMagazineActionTag);
		return Common::kNoError;
	}
	if (usedItemName.equalsIgnoreCase("REWARD_MONEY")) {
		executeDialogueActionTag(kUseRewardAtStoreActionTag);
		executeDialogueActionTag(kSetGenCuStuffActionTag);
		return Common::kNoError;
	}

	return playPhelpsLine(0x1c06, 1);
}

} // End of namespace Harvester
