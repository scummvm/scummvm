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

#include "harvester/npc/dwayne_dialogue.h"

#include "common/array.h"
#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const int kDwayneWhaleyTopicResponseLines[] = { 0x85, 0x86, 0x87 };
static const int kDwayneLoomisTopicResponseLines[] = { 0x8c, 0x8d };

} // End of namespace

bool DwayneDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("DWAYNE");
}

Common::Error DwayneDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	DwayneRoomDialogueState &state = _state;
	Common::String &dwayneTopicBuffer = state.currentTopicBuffer;
	int &dwayneTopicBufferLineIndex = state.currentTopicBufferLineIndex;
	auto assignDwayneTopicBuffer = [&](int responseLineIndex) {
		runtime.assignTopicBuffer(dwayneTopicBuffer, dwayneTopicBufferLineIndex,
			responseLineIndex, "Dwayne topic buffer");
	};
	auto playDwayneLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, "DWAYNE", headVariant);
	};
	auto playEdnaLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, "EDNA", headVariant);
	};
	auto hasInventoryItem = [&](const char *objectName) {
		Common::Array<StartupObjectRecord> inventoryObjects;
		runtime.startupScript().getVisibleInventoryObjects(inventoryObjects);
		for (const StartupObjectRecord &inventoryObject : inventoryObjects) {
			if (inventoryObject.objectName.equalsIgnoreCase(objectName))
				return true;
		}

		return false;
	};
	auto setDwayneIntroduced = [&]() {
		(void)runtime.startupScript().setRuntimeFlagValue("DWAYNE_INTRODUCED", true);
	};
	const bool sheriffInDiner = runtime.startupScript().getFlagValue("SHERIFF_IN_DINER");

	if (runtime.startupScript().getFlagValue("ARREST_FLAG")) {
		(void)runtime.startupScript().setRuntimeFlagValue("ARREST_FLAG", false);
		if (runtime.startupScript().getFlagValue("PC_BUSTED_KILLED_MOYNAHAN"))
			return playDwayneLine(0x38d6, 2);
		if (runtime.startupScript().getFlagValue("PC_BUSTED_KILLED_JIMMY"))
			return playDwayneLine(0x384d, 2);
		if (runtime.startupScript().getFlagValue("PC_BUSTED_KILLED_JOHNSON"))
			return playDwayneLine(0x386d, 2);
		if (runtime.startupScript().getFlagValue("PC_BUSTED_KILLED_PHELPS"))
			return playDwayneLine(0x38bc, 2);
		if (runtime.startupScript().getFlagValue("PC_BUSTED_KILLED_OREILLY"))
			return playDwayneLine(0x38b5, 2);
		if (runtime.startupScript().getFlagValue("IF_KILL_POTTSDAM_AT_GRAVE"))
			return playDwayneLine(0x37ed, 2);
		if (runtime.startupScript().getFlagValue("GENERIC_BUST"))
			return playDwayneLine(0x38fc, 2);
		if (runtime.startupScript().getFlagValue("DNALFT_PERVERT"))
			return playDwayneLine(0x3580, 2);
		if (runtime.startupScript().getFlagValue("PC_HAS_GOOJF_CARD") &&
				!runtime.startupScript().getFlagValue("PC_FRY_IN_CHAIR")) {
			return playDwayneLine(0x3814);
		}
		if (!runtime.startupScript().getFlagValue("PC_FRY_IN_CHAIR")) {
			if (runtime.startupScript().getFlagValue("BUSTED_THIRD"))
				return playDwayneLine(0x3928, 2);
			if (runtime.startupScript().getFlagValue("BUSTED_TWICE"))
				return playDwayneLine(0x3921, 2);
			if (runtime.startupScript().getFlagValue("BUSTED_ONCE"))
				return playDwayneLine(0x3918);
		}
		return Common::kNoError;
	}

	if (runtime.startupScript().getFlagValue("CHECK_EVIDENCE_DOOR"))
		return playDwayneLine(0x3386);
	if (runtime.startupScript().getFlagValue("PC_TRIES_TO_TAKE_STEPHANIES_REMAINS"))
		return playDwayneLine(0x34e3, 2);
	if (runtime.currentRoomName().equalsIgnoreCase("ST_BEDRM")) {
		if (!runtime.startupScript().getFlagValue("SD_TALKED_OF_CARD")) {
			Common::Error lineError = playDwayneLine(0x34da);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		return playDwayneLine(0x3395);
	}

	if (runtime.startupScript().getFlagValue("BRING_KARIN_TO_SHERIFF") &&
			!sheriffInDiner &&
			!state.bringKarinToSheriffLinePlayed) {
		state.pendingKarinAliveFollowup = true;
		state.pendingKarinAliveFollowupDayIndex =
			runtime.startupScript().getCurrentStoryDayIndex();
		state.bringKarinToSheriffLinePlayed = true;
		return playDwayneLine(0x3750);
	}
	if (runtime.startupScript().getFlagValue("KARIN_FOUND_ALIVE")) {
		state.pendingKarinAliveFollowup = true;
		state.pendingKarinAliveFollowupDayIndex =
			runtime.startupScript().getCurrentStoryDayIndex();
	}

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("BOYLES_BUTTON")) {
			state.discussedBoylesButton = true;
			return playDwayneLine(0x339b);
		}
		if (usedItemName.equalsIgnoreCase("K_PURSE")) {
			state.discussedKarinPurse = true;
			return playDwayneLine(0x37cb);
		}
		if (usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY")) {
			return playDwayneLine(0x33b9, 2);
		}
		if (usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("NOTE")) {
			if (state.presentedEvidenceReplyOverride)
				return playDwayneLine(0x342c);

			Common::Error lineError = playDwayneLine(0x33f4, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x70, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			lineError = Common::kNoError;
			if (responseIndex == 1) {
				lineError = playDwayneLine(0x33c3, 2);
			} else if (responseIndex == 2) {
				lineError = playDwayneLine(0x3403, 2);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			lineError = playDwayneLine(0x3407, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playDwayneLine(0x340d);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			responseIndex = 0;
			responseError = runtime.runResponseMenu(0x71, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 0)
				return Common::kNoError;
			if (responseIndex == 1)
				return playDwayneLine(0x341a, 2);
			if (responseIndex == 2)
				return playDwayneLine(0x3422, 2);
			return Common::kNoError;
		}
		if ((usedItemName.equalsIgnoreCase("LEDGER") ||
					usedItemName.equalsIgnoreCase("LEDGER2")) &&
				runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownLedgersToAnyone, true);
			return playDwayneLine(0x3452);
		}
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
			return playDwayneLine(0x3975, 3);
		}
		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
			return playDwayneLine(0x3942, 2);
		}
		if (state.tvDeedReplyOverride &&
				usedItemName.equalsIgnoreCase("TV_DEED")) {
			return playDwayneLine(0x3686);
		}
		if (usedItemName.equalsIgnoreCase("TV_DEED") ||
				usedItemName.equalsIgnoreCase("TV_DEED_PHOTOCOPY")) {
			return playDwayneLine(0x3935);
		}
		return playDwayneLine(0x3608);
	}

	if (state.sheriffInDinerIntroPending && sheriffInDiner) {
		state.sheriffInDinerIntroPending = false;
		state.sheriffInDinerIntroPlayed = true;
		setDwayneIntroduced();

		const DialogueLineEntry introLines[] = {
			{ 0x3482, "DWAYNE", 0 },
			{ 0x3486, "PC", 0 },
			{ 0x348b, "DWAYNE", 1 },
			{ 0x3490, "PC", 0 }
		};
		Common::Error lineError = runtime.playDialogueEntrySequence(introLines, ARRAYSIZE(introLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (runtime.startupScript().getFlagValue("KARIN_KIDNAPED")) {
			sharedState.karinKidnapedDialogueState = true;
			lineError = playEdnaLine(0x3496, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		lineError = playDwayneLine(0x349b, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		return playDwayneLine(0x34ac);
	}

	if (state.sheriffInDinerIntroPlayed && sheriffInDiner)
		return playDwayneLine(0x34ac);

	if (state.pendingInitialConversation && !sheriffInDiner) {
		state.pendingInitialConversation = false;
		state.eventFollowupGate = true;
		setDwayneIntroduced();

		const DialogueLineEntry initialLines[] = {
			{ 0x3245, "DWAYNE", 1 },
			{ 0x324a, "LOOMIS", 1 },
			{ 0x324e, "PC", 0 },
			{ 0x3252, "DWAYNE", 0 }
		};
		Common::Error lineError = runtime.playDialogueEntrySequence(initialLines, ARRAYSIZE(initialLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		assignDwayneTopicBuffer(0x78);
		if (sharedState.boyleGascanApplicationState)
			(void)runtime.startupScript().setRuntimeFlagValue("MOVE_SHERIFF", true);
	}

	if (state.eventFollowupGate && !sharedState.boyleGascanApplicationState) {
		Common::Error lineError = playDwayneLine(0x34d3, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (sharedState.dwayneWhaleyDisciplineFollowupState &&
			!state.whaleyDisciplineFollowupShown) {
		state.whaleyDisciplineFollowupShown = true;
		Common::Error lineError = playDwayneLine(0x34b3, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x79, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;
		lineError = Common::kNoError;
		if (responseIndex == 1) {
			lineError = playDwayneLine(0x34bf, 2);
		} else if (responseIndex == 2) {
			lineError = playDwayneLine(0x34c5, 1);
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playDwayneLine(0x34cb, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue(DialogueFlags::kShownEvidenceOfBlackmail) &&
			!state.noteCheckbookFollowupShown) {
		state.noteCheckbookFollowupShown = true;
		Common::Error lineError = playDwayneLine(0x355f, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (!sheriffInDiner) {
		if (runtime.startupScript().getFlagValue("SCRATCHED_TUCKER") &&
				!state.scratchedTuckerShown) {
			state.scratchedTuckerShown = true;
			Common::Error lineError = playDwayneLine(0x358b, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("BOLT_OF_CLOTH_TAKEN") &&
				!state.boltOfClothTakenShown) {
			state.boltOfClothTakenShown = true;
			Common::Error lineError = playDwayneLine(0x3594, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("BARBER_POLE_STOLEN") &&
				!state.barberPoleStolenShown) {
			state.barberPoleStolenShown = true;
			Common::Error lineError = playDwayneLine(0x359d, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("DINER_BURNED") &&
				!state.dinerBurnedShown) {
			state.dinerBurnedShown = true;
			Common::Error lineError = playDwayneLine(0x35bc, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("PC_ESCAPED_JAIL") &&
				!state.escapedJailShown) {
			state.escapedJailShown = true;
			Common::Error lineError = playDwayneLine(0x35cd);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("GOT_REMAINS_FOR_LODGE") &&
				!state.gotRemainsForLodgeShown) {
			state.gotRemainsForLodgeShown = true;
			Common::Error lineError = playDwayneLine(0x35d7, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("BURNED_TV_STATION") &&
				!state.burnedTvStationShown) {
			state.burnedTvStationShown = true;
			Common::Error lineError = playDwayneLine(0x35f6, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("KARIN_KIDNAPED") &&
				!state.karinKidnapedShown) {
			state.karinKidnapedShown = true;
			Common::Error lineError = playDwayneLine(0x36b2, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}

	const int currentStoryDayIndex = runtime.startupScript().getCurrentStoryDayIndex();
	if (state.pendingKarinAliveFollowup &&
			currentStoryDayIndex != state.pendingKarinAliveFollowupDayIndex &&
			!state.pendingKarinAliveFollowupLinePlayed) {
		state.pendingKarinAliveFollowupLinePlayed = true;
		state.pendingKarinAliveFollowup = false;
		state.completedKarinAliveFollowup = true;

		const DialogueLineEntry lines[] = {
			{ 0x3780, "DWAYNE", 0 },
			{ 0x3786, "PC", 4 },
			{ 0x378a, "DWAYNE", 2 }
		};
		Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("KARIN_FOUND_DEAD") &&
			!state.discussedKarinPurse &&
			!sheriffInDiner &&
			!state.karinFoundDeadWithoutPurseShown) {
		state.karinFoundDeadWithoutPurseShown = true;
		Common::Error lineError = playDwayneLine(0x3793);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (!runtime.startupScript().isNamedNpcDeathTypeClear("JIMMY") &&
			!sheriffInDiner &&
			!state.jimmyAbsentShown) {
		state.jimmyAbsentShown = true;
		Common::Error lineError = playDwayneLine(0x3855, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (runtime.startupScript().getFlagValue("JIMMY_ATTACKED")) {
			lineError = playDwayneLine(0x37f8);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x81, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			lineError = Common::kNoError;
			if (responseIndex == 1) {
				lineError = playDwayneLine(0x3805, 1);
			} else if (responseIndex == 2) {
				lineError = playDwayneLine(0x380c);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}

	if (!runtime.startupScript().isNamedNpcDeathTypeClear("MOYNAHAN") &&
			!sheriffInDiner &&
			!state.moynahanAbsentShown) {
		state.moynahanAbsentShown = true;
		Common::Error lineError = playDwayneLine(0x38ce);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("EDNA_HUNG") &&
			!state.ednaHungShown) {
		state.ednaHungShown = true;
		Common::Error lineError = playDwayneLine(0x38f1, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (!runtime.startupScript().isNamedNpcDeathTypeClear("MCKNIGHT") &&
			!sheriffInDiner &&
			!state.mcknightAbsentShown) {
		state.mcknightAbsentShown = true;
		Common::Error lineError = hasInventoryItem("TV_DEED")
			? playDwayneLine(0x3904, 3)
			: playDwayneLine(0x390f, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	Common::String selectedTopic;
	for (;;) {
		if (runtime.matchesResponseLine(selectedTopic, 0x82))
			return playDwayneLine(0x3395);

		Common::Error menuError = runtime.runKeywordMenu(
			dwayneTopicBuffer, dwayneTopicBufferLineIndex, selectedTopic);
		if (menuError.getCode() != Common::kNoError)
			return menuError;
		if (selectedTopic.empty())
			return Common::kNoError;

		if (runtime.matchesResponseLine(selectedTopic, 0x83)) {
			Common::Error lineError = playDwayneLine(0x326e);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignDwayneTopicBuffer(0x84);
			continue;
		}

		if (runtime.matchesAnyResponseLine(selectedTopic, kDwayneWhaleyTopicResponseLines,
				ARRAYSIZE(kDwayneWhaleyTopicResponseLines))) {
			if (!sheriffInDiner) {
				const DialogueLineEntry lines[] = {
					{ 0x327b, "DWAYNE", 0 },
					{ 0x3280, "PC", 2 },
					{ 0x3285, "DWAYNE", 0 },
					{ 0x328b, "LOOMIS", 1 },
					{ 0x3290, "DWAYNE", 2 },
					{ 0x3294, "LOOMIS", 3 },
					{ 0x3299, "PC", 4 },
					{ 0x329e, "DWAYNE", 1 }
				};
				Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				sharedState.dialogueStateD2f04 = true;
				assignDwayneTopicBuffer(0x89);
			}
			continue;
		}

		if (runtime.matchesAnyResponseLine(selectedTopic, kDwayneLoomisTopicResponseLines,
				ARRAYSIZE(kDwayneLoomisTopicResponseLines))) {
			if (!sheriffInDiner) {
				const DialogueLineEntry lines[] = {
					{ 0x32bf, "DWAYNE", 1 },
					{ 0x32c1, "DWAYNE", 0 },
					{ 0x32c7, "LOOMIS", 3 },
					{ 0x32cb, "DWAYNE", 0 },
					{ 0x32d1, "DWAYNE", 1 }
				};
				Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignDwayneTopicBuffer(0x8e);
			}
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x8f)) {
			const DialogueLineEntry lines[] = {
				{ 0x32db, "DWAYNE", 0 },
				{ 0x32e2, "PC", 4 },
				{ 0x32e6, "DWAYNE", 0 }
			};
			Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignDwayneTopicBuffer(0x90);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x91)) {
			const DialogueLineEntry lines[] = {
				{ 0x32f2, "DWAYNE", 0 },
				{ 0x32f9, "PC", 0 },
				{ 0x32fd, "DWAYNE", 0 },
				{ 0x32fe, "DWAYNE", 0 },
				{ 0x32ff, "DWAYNE", 0 }
			};
			Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignDwayneTopicBuffer(0x92);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x93)) {
			if (!runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED")) {
				Common::Error lineError = playDwayneLine(0x330d, 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			const DialogueLineEntry lines[] = {
				{ 0x331f, "DWAYNE", 3 },
				{ 0x3320, "DWAYNE", 0 },
				{ 0x3321, "DWAYNE", 0 },
				{ 0x332c, "DWAYNE", 0 }
			};
			Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignDwayneTopicBuffer(0x94);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x95)) {
			sharedState.discussedLodgeTopic = true;
			Common::Error lineError = playDwayneLine(0x3339);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x96, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			lineError = Common::kNoError;
			if (responseIndex == 1) {
				const DialogueLineEntry responseLines[] = {
					{ 0x3345, "DWAYNE", 2 },
					{ 0x3346, "DWAYNE", 2 },
					{ 0x3347, "DWAYNE", 2 }
				};
				lineError = runtime.playDialogueEntrySequence(responseLines, ARRAYSIZE(responseLines));
			} else if (responseIndex == 2) {
				lineError = playDwayneLine(0x3350);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignDwayneTopicBuffer(0x97);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x98)) {
			const DialogueLineEntry lines[] = {
				{ 0x335b, "DWAYNE", 2 },
				{ 0x3360, "PC", 2 },
				{ 0x3364, "DWAYNE", 3 }
			};
			Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = sharedState.dialogueStateD2f04
				? playDwayneLine(0x3368, 2)
				: playDwayneLine(0x336c, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playDwayneLine(0x3370, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignDwayneTopicBuffer(0x99);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x9a)) {
			Common::Error lineError = playDwayneLine(0x337a, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignDwayneTopicBuffer(0x9b);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x9c)) {
			const DialogueLineEntry lines[] = {
				{ 0x3386, "DWAYNE", 0 },
				{ 0x3387, "DWAYNE", 0 }
			};
			Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignDwayneTopicBuffer(0x9d);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x9e))
			continue;

		Common::Error lineError = playDwayneLine(0x393c);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
}

} // End of namespace Harvester
