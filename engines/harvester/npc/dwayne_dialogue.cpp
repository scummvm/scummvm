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


#include "harvester/npc/dwayne_dialogue.h"

#include "common/array.h"
#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kInventoryOwnerName = "INVENTORY";
static const char *const kRahRoomName = "RAH";

static const int kDwayneWhaleyTopicResponseLines[] = { 0x85, 0x86, 0x87 };
static const int kDwayneLoomisTopicResponseLines[] = { 0x8c, 0x8d };

} // End of namespace

Common::Error DwayneDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	DwayneRoomDialogueState &state = _state;
	Script &startupScript = runtime.startupScript();
	Common::String &dwayneTopicBuffer = state.currentTopicBuffer;
	int &dwayneTopicBufferLineIndex = state.currentTopicBufferLineIndex;
	state.currentTopicBuffer.clear();
	state.currentTopicBufferLineIndex = -1;
	DialogueTopicBuffer topics(runtime, dwayneTopicBuffer, dwayneTopicBufferLineIndex, "Dwayne topic buffer");

	auto hideObjectIfPresent = [&](const char *ownerOrRoom, const Common::String &objectName) {
		if (startupScript.isObjectInInventory(objectName))
			(void)startupScript.setRuntimeObjectVisible(ownerOrRoom, objectName, false);
	};
	auto confiscateSheriffEvidence = [&]() {
		hideObjectIfPresent(kInventoryOwnerName, "CHECKBOOK");
		hideObjectIfPresent(kInventoryOwnerName, "CHECKBOOK_PHOTOCOPY");
		hideObjectIfPresent(kInventoryOwnerName, "NOTE");
		hideObjectIfPresent(kInventoryOwnerName, "NOTE_PHOTOCOPY");
		hideObjectIfPresent(kRahRoomName, "TV_DEED");
		hideObjectIfPresent(kRahRoomName, "TV_DEED_PHOTOCOPY");
	};
	auto awardGoojfCard = [&]() {
		(void)startupScript.addRuntimeObjectToInventory("GOOJF_CARD");
		(void)startupScript.setRuntimeFlagValue("PC_HAS_GOOJF_CARD", true);
	};

	auto setDwayneIntroduced = [&]() {
		(void)startupScript.setRuntimeFlagValue("DWAYNE_INTRODUCED", true);
	};
	auto setPendingKarinAliveFollowup = [&]() {
		state.pendingKarinAliveFollowup = true;
		state.pendingKarinAliveFollowupDayIndex = startupScript.getCurrentStoryDayIndex();
		sharedState.dwaynePendingKarinAliveFollowupState = 1;
	};
	auto clearGoojfCard = [&]() {
		(void)startupScript.setRuntimeFlagValue("PC_HAS_GOOJF_CARD", false);
		(void)startupScript.setRuntimeObjectVisible(kRahRoomName, "GOOJF_CARD", false);
	};
	const bool sheriffInDiner = startupScript.getFlagValue("SHERIFF_IN_DINER");
	const int currentStoryDayIndex = startupScript.getCurrentStoryDayIndex();

	if (startupScript.getFlagValue("ARREST_FLAG")) {
		(void)startupScript.setRuntimeFlagValue("ARREST_FLAG", false);

		if (startupScript.getFlagValue("PC_BUSTED_KILLED_MOYNAHAN")) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x38d6, "DWAYNE", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			(void)startupScript.setRuntimeFlagValue("PC_BUSTED_KILLED_MOYNAHAN", false);
		} else if (startupScript.getFlagValue("PC_BUSTED_KILLED_JIMMY")) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x384d, "DWAYNE", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			(void)startupScript.setRuntimeFlagValue("PC_BUSTED_KILLED_JIMMY", false);
			(void)startupScript.setRuntimeFlagValue("KILLED_JIMMY", false);
		} else if (startupScript.getFlagValue("PC_BUSTED_KILLED_JOHNSON")) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x386d, "DWAYNE", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			(void)startupScript.setRuntimeFlagValue("PC_BUSTED_KILLED_JOHNSON", false);
		} else if (startupScript.getFlagValue("PC_BUSTED_KILLED_PHELPS")) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x38bc, "DWAYNE", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			(void)startupScript.setRuntimeFlagValue("PC_BUSTED_KILLED_PHELPS", false);
		} else if (startupScript.getFlagValue("PC_BUSTED_KILLED_OREILLY")) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x38b5, "DWAYNE", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			(void)startupScript.setRuntimeFlagValue("PC_BUSTED_KILLED_OREILLY", false);
			(void)startupScript.setRuntimeFlagValue("KILLED_OREILLY", false);
		} else if (startupScript.getFlagValue("IF_KILL_POTTSDAM_AT_GRAVE")) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x37ed, "DWAYNE", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			(void)startupScript.setRuntimeFlagValue("IF_KILL_POTTSDAM_AT_GRAVE", false);
		} else if (startupScript.getFlagValue("GENERIC_BUST")) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x38fc, "DWAYNE", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			(void)startupScript.setRuntimeFlagValue("GENERIC_BUST", false);
		} else if (startupScript.getFlagValue("DNALFT_PERVERT")) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x3580, "DWAYNE", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			(void)startupScript.setRuntimeFlagValue("DNALFT_PERVERT", false);
		}

		if (startupScript.getFlagValue("PC_HAS_GOOJF_CARD") &&
				!startupScript.getFlagValue("PC_FRY_IN_CHAIR")) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x3814, "DWAYNE", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			if (startupScript.getFlagValue("PC_KILLED_ANYONE")) {
				const DialogueLineEntry goojfCardLines[] = {
					{ 0x3820, "DWAYNE", 0 },
					{ 0x3824, "PC", 0 },
					{ 0x3828, "DWAYNE", 0 }
				};
				lineError = runtime.playDialogueEntrySequence(goojfCardLines, ARRAYSIZE(goojfCardLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				int responseIndex = 0;
				Common::Error responseError = runtime.runResponseMenu(0x69, responseIndex);
				if (responseError.getCode() != Common::kNoError)
					return responseError;
				if (responseIndex == 1) {
					lineError = runtime.playDialogueLineWithVariant(0x3832, "DWAYNE", 1);
				} else if (responseIndex == 2) {
					lineError = runtime.playDialogueLineWithVariant(0x383b, "DWAYNE", 2);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
					lineError = runtime.playDialogueLineWithVariant(0x3840, "PC", 0);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
					lineError = runtime.playDialogueLineWithVariant(0x3844, "DWAYNE", 2);
				}
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			clearGoojfCard();
			return Common::kNoError;
		}

		if (startupScript.getFlagValue("PC_FRY_IN_CHAIR"))
			return Common::kNoError;
		if (startupScript.getFlagValue("BUSTED_THIRD"))
			return runtime.playDialogueLineWithVariant(0x3928, "DWAYNE", 2);
		if (startupScript.getFlagValue("BUSTED_TWICE"))
			return runtime.playDialogueLineWithVariant(0x3921, "DWAYNE", 2);
		if (startupScript.getFlagValue("BUSTED_ONCE"))
			return runtime.playDialogueLineWithVariant(0x3918, "DWAYNE", 0);
		return Common::kNoError;
	}

	if (startupScript.getFlagValue("CHECK_EVIDENCE_DOOR")) {
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x3386, "DWAYNE", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		(void)startupScript.setRuntimeFlagValue("CHECK_EVIDENCE_DOOR", false);
		return Common::kNoError;
	}
	if (startupScript.getFlagValue("PC_TRIES_TO_TAKE_STEPHANIES_REMAINS"))
		return runtime.playDialogueLineWithVariant(0x34e3, "DWAYNE", 2);
	if (runtime.currentRoomName().equalsIgnoreCase("ST_BEDRM")) {
		if (!startupScript.getFlagValue("SD_TALKED_OF_CARD")) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x34da, "DWAYNE", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			(void)startupScript.setRuntimeFlagValue("SD_TALKED_OF_CARD", true);
			return Common::kNoError;
		}
		return runtime.playDialogueLineWithVariant(0x3395, "DWAYNE", 0);
	}

	if (startupScript.getFlagValue("BRING_KARIN_TO_SHERIFF") &&
			!sheriffInDiner &&
			!state.bringKarinToSheriffLinePlayed) {
		setPendingKarinAliveFollowup();
		state.bringKarinToSheriffLinePlayed = true;
		const DialogueLineEntry lines[] = {
			{ 0x3750, "DWAYNE", 1 },
			{ 0x3754, "PC", 0 },
			{ 0x3759, "DWAYNE", 0 },
			{ 0x375d, "DWAYNE", 1 }
		};
		return runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
	}
	if (startupScript.getFlagValue("KARIN_FOUND_ALIVE"))
		setPendingKarinAliveFollowup();

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("BOYLES_BUTTON")) {
			state.discussedBoylesButton = true;
			sharedState.dwayneDiscussedBoylesButton = true;
			const DialogueLineEntry lines[] = {
				{ 0x339b, "DWAYNE", 0 },
				{ 0x33a0, "PC", 0 },
				{ 0x33a5, "DWAYNE", 0 },
				{ 0x33a9, "PC", 0 },
				{ 0x33ad, "DWAYNE", 2 }
			};
			return runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
		}
		if (usedItemName.equalsIgnoreCase("K_PURSE")) {
			state.discussedKarinPurse = true;
			sharedState.discussedKarinPurse = 1;
			const DialogueLineEntry lines[] = {
				{ 0x37cb, "DWAYNE", 1 },
				{ 0x37cf, "PC", 3 },
				{ 0x37b4, "DWAYNE", 3 },
				{ 0x37c1, "DWAYNE", 0 },
				{ 0x37c5, "PC", 0 }
			};
			return runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
		}
		if (usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY")) {
			(void)startupScript.setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x33b9, "DWAYNE", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x6e, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1) {
				lineError = runtime.playDialogueLineWithVariant(0x33c3, "DWAYNE", 2);
			} else if (responseIndex == 2) {
				lineError = runtime.playDialogueLineWithVariant(0x33c7, "DWAYNE", 2);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			lineError = runtime.playDialogueLineWithVariant(0x33cb, "PC", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x33d2, "DWAYNE", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			responseIndex = 0;
			responseError = runtime.runResponseMenu(0x6f, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1) {
				lineError = runtime.playDialogueLineWithVariant(0x33e0, "DWAYNE", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				state.presentedEvidenceReplyOverride = true;
			} else if (responseIndex == 2) {
				lineError = runtime.playDialogueLineWithVariant(0x33e7, "DWAYNE", 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				runtime.executeAndQueueAction("PC_FRY_DADDY_1");
			}
			return Common::kNoError;
		}
		if (usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("NOTE")) {
			(void)startupScript.setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
			if (state.presentedEvidenceReplyOverride) {
				const DialogueLineEntry introLines[] = {
					{ 0x342c, "DWAYNE", 0 },
					{ 0x3430, "PC", 0 },
					{ 0x3435, "DWAYNE", 2 }
				};
				Common::Error lineError = runtime.playDialogueEntrySequence(introLines, ARRAYSIZE(introLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				int responseIndex = 0;
				Common::Error responseError = runtime.runResponseMenu(0x72, responseIndex);
				if (responseError.getCode() != Common::kNoError)
					return responseError;
				if (responseIndex == 1) {
					lineError = runtime.playDialogueLineWithVariant(0x3440, "DWAYNE", 2);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
					runtime.executeAndQueueAction("PC_FRY_DADDY_1");
					return Common::kNoError;
				}
				if (responseIndex == 2) {
					lineError = runtime.playDialogueLineWithVariant(0x3448, "DWAYNE", 0);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
					confiscateSheriffEvidence();
					awardGoojfCard();
				}
				return Common::kNoError;
			}

			Common::Error lineError = runtime.playDialogueLineWithVariant(0x33f4, "DWAYNE", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x70, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			lineError = Common::kNoError;
			if (responseIndex == 1) {
				lineError = runtime.playDialogueLineWithVariant(0x33c3, "DWAYNE", 2);
			} else if (responseIndex == 2) {
				lineError = runtime.playDialogueLineWithVariant(0x3403, "DWAYNE", 2);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			lineError = runtime.playDialogueLineWithVariant(0x3407, "PC", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x340d, "DWAYNE", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			responseIndex = 0;
			responseError = runtime.runResponseMenu(0x71, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 0)
				return Common::kNoError;
			if (responseIndex == 1) {
				lineError = runtime.playDialogueLineWithVariant(0x341a, "DWAYNE", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				runtime.executeAndQueueAction("PC_FRY_DADDY_1");
				return Common::kNoError;
			}
			if (responseIndex == 2) {
				lineError = runtime.playDialogueLineWithVariant(0x3422, "DWAYNE", 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				runtime.executeAndQueueAction("PC_FRY_DADDY_1");
				return Common::kNoError;
			}
			return Common::kNoError;
		}
		if ((usedItemName.equalsIgnoreCase("LEDGER") ||
					usedItemName.equalsIgnoreCase("LEDGER2")) &&
				startupScript.getFlagValue("HAVE_BOTH_LEDGERS")) {
			sharedState.discussedLedgerEvidence = 1;
			(void)startupScript.setRuntimeFlagValue(DialogueFlags::kShownLedgersToAnyone, true);
			const DialogueLineEntry lines[] = {
				{ 0x3452, "DWAYNE", 0 },
				{ 0x3457, "PC", 0 },
				{ 0x345c, "DWAYNE", 0 },
				{ 0x3460, "PC", 2 },
				{ 0x3464, "DWAYNE", 0 },
				{ 0x346a, "PC", 2 },
				{ 0x346e, "DWAYNE", 0 },
				{ 0x3474, "PC", 4 },
				{ 0x3478, "DWAYNE", 2 }
			};
			Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			(void)startupScript.resetRuntimeObjectToInitialState("LEDGER");
			(void)startupScript.resetRuntimeObjectToInitialState("LEDGER2");
			runtime.executeAndQueueAction(sheriffInDiner ? "CALL_POLICE" : "LEDGER_BUST");
			return Common::kNoError;
		}
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			sharedState.discussedWhaleyHerrillPhoto = 1;
			(void)startupScript.setRuntimeFlagValue(DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
			const DialogueLineEntry lines[] = {
				{ 0x3975, "DWAYNE", 3 },
				{ 0x3979, "PC", 0 },
				{ 0x397e, "DWAYNE", 0 }
			};
			return runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
		}
		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			sharedState.discussedCasketPhotoEvidence = 1;
			(void)startupScript.setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);

			Common::Error lineError = runtime.playDialogueLineWithVariant(0x3942, "DWAYNE", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			if (sharedState.dialogueStateD2ec8) {
				const DialogueLineEntry lines[] = {
					{ 0x3949, "PC", 0 },
					{ 0x3950, "DWAYNE", 0 },
					{ 0x3955, "PC", 4 },
					{ 0x3959, "DWAYNE", 1 },
					{ 0x396d, "DWAYNE", 0 }
				};
				lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				hideObjectIfPresent(kInventoryOwnerName, usedItemName);
				runtime.executeAndQueueAction("CALL_POLICE");
			} else {
				const DialogueLineEntry lines[] = {
					{ 0x3961, "PC", 0 },
					{ 0x3967, "DWAYNE", 0 },
					{ 0x396d, "DWAYNE", 0 }
				};
				lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				hideObjectIfPresent(kInventoryOwnerName, usedItemName);
			}
			return Common::kNoError;
		}
		if (state.tvDeedReplyOverride && usedItemName.equalsIgnoreCase("TV_DEED")) {
			(void)startupScript.setRuntimeFlagValue(DialogueFlags::kShownEvidenceSheriffOwns, true);

			const DialogueLineEntry introLines[] = {
				{ 0x3686, "DWAYNE", 0 },
				{ 0x368a, "PC", 0 },
				{ 0x3690, "DWAYNE", 0 }
			};
			Common::Error lineError = runtime.playDialogueEntrySequence(introLines, ARRAYSIZE(introLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x73, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1) {
				lineError = runtime.playDialogueLineWithVariant(0x369b, "DWAYNE", 2);
			} else if (responseIndex == 2) {
				const DialogueLineEntry responseLines[] = {
					{ 0x36a3, "DWAYNE", 0 },
					{ 0x36a8, "PC", 2 },
					{ 0x36ac, "DWAYNE", 2 }
				};
				lineError = runtime.playDialogueEntrySequence(responseLines, ARRAYSIZE(responseLines));
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			confiscateSheriffEvidence();
			awardGoojfCard();
			return Common::kNoError;
		}
		if (!usedItemName.equalsIgnoreCase("TV_DEED") &&
				!usedItemName.equalsIgnoreCase("TV_DEED_PHOTOCOPY")) {
			return runtime.playDialogueLineWithVariant(0x3935, "DWAYNE", 0);
		}

		(void)startupScript.setRuntimeFlagValue(DialogueFlags::kShownEvidenceSheriffOwns, true);

		Common::Error lineError = runtime.playDialogueLineWithVariant(0x3608, "DWAYNE", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x74, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;
		if (responseIndex == 1) {
			lineError = runtime.playDialogueLineWithVariant(0x3613, "DWAYNE", 0);
		} else if (responseIndex == 2) {
			lineError = runtime.playDialogueLineWithVariant(0x3618, "DWAYNE", 0);
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = runtime.playDialogueLineWithVariant(0x361c, "DWAYNE", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		responseIndex = 0;
		responseError = runtime.runResponseMenu(0x75, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;
		if (responseIndex == 1) {
			lineError = runtime.playDialogueLineWithVariant(0x3629, "DWAYNE", 2);
		} else if (responseIndex == 2) {
			lineError = runtime.playDialogueLineWithVariant(0x362d, "DWAYNE", 0);
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		responseIndex = 0;
		responseError = runtime.runResponseMenu(0x76, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;
		if (usedItemName.equalsIgnoreCase("TV_DEED")) {
			const DialogueLineEntry deedLines[] = {
				{ 0x363b, "DWAYNE", 0 },
				{ 0x3641, "PC", 0 },
				{ 0x3645, "DWAYNE", 0 }
			};
			lineError = runtime.playDialogueEntrySequence(deedLines, ARRAYSIZE(deedLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			runtime.executeAndQueueAction("PC_FRY_DADDY_1");
			return Common::kNoError;
		}
		if (responseIndex != 1) {
			lineError = runtime.playDialogueLineWithVariant(0x367b, "DWAYNE", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			hideObjectIfPresent(kRahRoomName, "TV_DEED_PHOTOCOPY");
			return Common::kNoError;
		}

		const DialogueLineEntry copyLines[] = {
			{ 0x364e, "DWAYNE", 0 },
			{ 0x3656, "PC", 0 },
			{ 0x365b, "DWAYNE", 0 }
		};
		lineError = runtime.playDialogueEntrySequence(copyLines, ARRAYSIZE(copyLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		responseIndex = 0;
		responseError = runtime.runResponseMenu(0x77, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;
		if (responseIndex == 1) {
			lineError = runtime.playDialogueLineWithVariant(0x3665, "DWAYNE", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			state.tvDeedReplyOverride = true;
			return Common::kNoError;
		}
		if (responseIndex == 2) {
			const DialogueLineEntry responseLines[] = {
				{ 0x366b, "DWAYNE", 3 },
				{ 0x3670, "PC", 2 },
				{ 0x3674, "DWAYNE", 2 }
			};
			lineError = runtime.playDialogueEntrySequence(responseLines, ARRAYSIZE(responseLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			runtime.executeAndQueueAction("PC_FRY_DADDY_1");
		}
		return Common::kNoError;
	}

	bool handledIntroOrRevisit = false;
	if (state.sheriffInDinerIntroPending && sheriffInDiner) {
		state.sheriffInDinerIntroPending = false;
		state.sheriffInDinerIntroPlayed = true;
		setDwayneIntroduced();

		const DialogueLineEntry introLines[] = {
			{ 0x3482, "DWAYNE", 1 },
			{ 0x3486, "PC", 0 },
			{ 0x348b, "DWAYNE", 1 },
			{ 0x3490, "EDNA", 0 }
		};
		Common::Error lineError = runtime.playDialogueEntrySequence(introLines, ARRAYSIZE(introLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (startupScript.getFlagValue("KARIN_KIDNAPED")) {
			sharedState.karinKidnapedDialogueState = true;
			lineError = runtime.playDialogueLineWithVariant(0x3496, "EDNA", 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		lineError = runtime.playDialogueLineWithVariant(0x349b, "DWAYNE", 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (state.pendingInitialConversation) {
			const DialogueLineEntry initialLines[] = {
				{ 0x34a0, "PC", 0 },
				{ 0x34a4, "DWAYNE", 1 }
			};
			lineError = runtime.playDialogueEntrySequence(initialLines, ARRAYSIZE(initialLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		handledIntroOrRevisit = true;
	}
	else if (state.sheriffInDinerIntroPlayed && sheriffInDiner) {
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x34ac, "DWAYNE", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		handledIntroOrRevisit = true;
	} else if (state.pendingInitialConversation && !sheriffInDiner) {
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
		topics.assign(0x78);
		if (sharedState.boyleGascanApplicationState)
			(void)startupScript.setRuntimeFlagValue("MOVE_SHERIFF", true);
		handledIntroOrRevisit = true;
	}

	if (!handledIntroOrRevisit && state.eventFollowupGate) {
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x34d3, "DWAYNE", 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (sharedState.dwayneWhaleyDisciplineFollowupState &&
			!state.whaleyDisciplineFollowupShown) {
		state.whaleyDisciplineFollowupShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x34b3, "DWAYNE", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x79, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;
		lineError = Common::kNoError;
		if (responseIndex == 1) {
			lineError = runtime.playDialogueLineWithVariant(0x34bf, "DWAYNE", 2);
		} else if (responseIndex == 2) {
			lineError = runtime.playDialogueLineWithVariant(0x34c5, "DWAYNE", 1);
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueLineWithVariant(0x34cb, "DWAYNE", 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (sharedState.discussedNoteCheckbookEvidence != 0 &&
			!state.noteCheckbookFollowupShown) {
		state.noteCheckbookFollowupShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x355f, "DWAYNE", 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (sharedState.discussedTvDeedEvidence != 0) {
			const DialogueLineEntry lines[] = {
				{ 0x3565, "DWAYNE", 2 },
				{ 0x356a, "DWAYNE", 2 },
				{ 0x356f, "PC", 2 },
				{ 0x3573, "DWAYNE", 0 }
			};
			lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			runtime.executeAndQueueAction("PC_FRY_DADDY_1");
			return Common::kNoError;
		}
	}

	if (!sheriffInDiner) {
		if (startupScript.getFlagValue("SCRATCHED_TUCKER") &&
				!state.scratchedTuckerShown) {
			state.scratchedTuckerShown = true;
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x358b, "DWAYNE", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (startupScript.getFlagValue("BOLT_OF_CLOTH_TAKEN") &&
				!state.boltOfClothTakenShown) {
			state.boltOfClothTakenShown = true;
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x3594, "DWAYNE", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (startupScript.getFlagValue("BARBER_POLE_STOLEN") &&
				!state.barberPoleStolenShown) {
			state.barberPoleStolenShown = true;
			const DialogueLineEntry lines[] = {
				{ 0x359d, "DWAYNE", 0 },
				{ 0x35a4, "DWAYNE", 0 },
				{ 0x35ab, "DWAYNE", 0 }
			};
			Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (startupScript.getFlagValue("DINER_BURNED") &&
				!state.dinerBurnedShown) {
			state.dinerBurnedShown = true;
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x35bc, "DWAYNE", 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (startupScript.getFlagValue("KARIN_FOUND_ALIVE")) {
				const DialogueLineEntry lines[] = {
					{ 0x35c1, "DWAYNE", 3 },
					{ 0x35c5, "DWAYNE", 3 }
				};
				lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
		}
		if (startupScript.getFlagValue("PC_ESCAPED_JAIL") &&
				!state.escapedJailShown) {
			state.escapedJailShown = true;
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x35cd, "DWAYNE", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (startupScript.getFlagValue("GOT_REMAINS_FOR_LODGE") &&
				!state.gotRemainsForLodgeShown) {
			state.gotRemainsForLodgeShown = true;
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x35d7, "DWAYNE", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (startupScript.getFlagValue("BURNED_TV_STATION") &&
				!state.burnedTvStationShown) {
			state.burnedTvStationShown = true;
			const DialogueLineEntry lines[] = {
				{ 0x35f6, "DWAYNE", 2 },
				{ 0x35fb, "DWAYNE", 2 },
				{ 0x3601, "DWAYNE", 2 }
			};
			Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (startupScript.getFlagValue("KARIN_KIDNAPED") &&
				!state.karinKidnapedShown) {
			state.karinKidnapedShown = true;
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x36b2, "DWAYNE", 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			sharedState.karinKidnapedDialogueState = true;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x7b, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1) {
				lineError = runtime.playDialogueLineWithVariant(0x36bd, "DWAYNE", 0);
			} else if (responseIndex == 2) {
				lineError = runtime.playDialogueLineWithVariant(0x36c1, "DWAYNE", 0);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			lineError = runtime.playDialogueLineWithVariant(0x36c5, "DWAYNE", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			responseIndex = 0;
			responseError = runtime.runResponseMenu(0x7c, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1) {
				lineError = runtime.playDialogueLineWithVariant(0x36d0, "DWAYNE", 0);
			} else if (responseIndex == 2) {
				lineError = runtime.playDialogueLineWithVariant(0x36d6, "DWAYNE", 1);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			lineError = runtime.playDialogueLineWithVariant(0x36e7, "DWAYNE", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}

	if (state.pendingKarinAliveFollowup &&
			currentStoryDayIndex != state.pendingKarinAliveFollowupDayIndex &&
			!state.pendingKarinAliveFollowupLinePlayed) {
		state.pendingKarinAliveFollowupLinePlayed = true;
		state.pendingKarinAliveFollowup = false;
		sharedState.dwaynePendingKarinAliveFollowupState = 0;

		const DialogueLineEntry lines[] = {
			{ 0x3780, "DWAYNE", 0 },
			{ 0x3786, "PC", 4 },
			{ 0x378a, "DWAYNE", 2 }
		};
		Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		state.completedKarinAliveFollowup = true;
		sharedState.dwayneCompletedKarinAliveFollowup = true;
	}

	if (startupScript.getFlagValue("KARIN_FOUND_DEAD") &&
			!state.discussedKarinPurse &&
			sharedState.discussedKarinPurse == 0 &&
			!sheriffInDiner &&
			!state.karinFoundDeadWithoutPurseShown) {
		state.karinFoundDeadWithoutPurseShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x3793, "DWAYNE", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x7f, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;
		if (responseIndex == 1) {
			return runtime.playDialogueLineWithVariant(0x379d, "DWAYNE", 0);
		} else if (responseIndex == 2) {
			lineError = runtime.playDialogueLineWithVariant(0x37a4, "DWAYNE", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			responseIndex = 0;
			responseError = runtime.runResponseMenu(0x80, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1) {
				lineError = runtime.playDialogueLineWithVariant(0x37ae, "DWAYNE", 0);
			} else if (responseIndex == 2) {
				lineError = runtime.playDialogueLineWithVariant(0x37b4, "DWAYNE", 3);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			const DialogueLineEntry lines[] = {
				{ 0x37c1, "DWAYNE", 0 },
				{ 0x37c5, "PC", 0 }
			};
			lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}

	if (!startupScript.isNamedNpcDeathTypeClear("JIMMY") &&
			!sheriffInDiner &&
			!state.jimmyAbsentShown) {
		state.jimmyAbsentShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x3855, "DWAYNE", 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (startupScript.getFlagValue("JIMMY_ATTACKED")) {
			lineError = runtime.playDialogueLineWithVariant(0x37f8, "DWAYNE", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x81, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			lineError = Common::kNoError;
			if (responseIndex == 1) {
				lineError = runtime.playDialogueLineWithVariant(0x3805, "DWAYNE", 1);
			} else if (responseIndex == 2) {
				lineError = runtime.playDialogueLineWithVariant(0x380c, "DWAYNE", 0);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}

	if (!startupScript.isNamedNpcDeathTypeClear("MOYNAHAN") &&
			!sheriffInDiner &&
			!state.moynahanAbsentShown) {
		state.moynahanAbsentShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x38ce, "DWAYNE", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (startupScript.getFlagValue("EDNA_HUNG") &&
			!state.ednaHungShown) {
		state.ednaHungShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x38f1, "DWAYNE", 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (!startupScript.isNamedNpcDeathTypeClear("MCKNIGHT") &&
			!sheriffInDiner &&
			!state.mcknightAbsentShown) {
		state.mcknightAbsentShown = true;
		Common::Error lineError = startupScript.isObjectInInventory("TV_DEED")
			? runtime.playDialogueLineWithVariant(0x3904, "DWAYNE", 3)
			: runtime.playDialogueLineWithVariant(0x390f, "DWAYNE", 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	Common::String selectedTopic;
	for (;;) {
		if (runtime.matchesResponseLine(selectedTopic, 0x82))
			return runtime.playDialogueLineWithVariant(0x3395, "DWAYNE", 0);

		Common::Error menuError = runtime.runKeywordMenu(
			dwayneTopicBuffer, dwayneTopicBufferLineIndex, selectedTopic);
		if (menuError.getCode() != Common::kNoError)
			return menuError;
		if (selectedTopic.empty())
			return Common::kNoError;

		if (runtime.matchesResponseLine(selectedTopic, 0x83)) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x326e, "DWAYNE", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			topics.assign(0x84);
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
				topics.assign(0x89);
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
					{ 0x32d1, "LOOMIS", 1 }
				};
				Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				runtime.setActiveSpeakerPortrait("DWAYNE", 0);
				topics.assign(0x8e);
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
			topics.assign(0x90);
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
			topics.assign(0x92);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x93)) {
			Common::Error lineError = Common::kNoError;
			if (!startupScript.getFlagValue("STEPH_MIDGAME_PLAYED")) {
				const DialogueLineEntry lines[] = {
					{ 0x330d, "DWAYNE", 1 },
					{ 0x3314, "PC", 0 },
					{ 0x3318, "DWAYNE", 0 },
					{ 0x332c, "DWAYNE", 0 }
				};
				lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			} else {
				const DialogueLineEntry lines[] = {
					{ 0x331f, "DWAYNE", 3 },
					{ 0x3320, "DWAYNE", 0 },
					{ 0x3321, "DWAYNE", 0 },
					{ 0x332c, "DWAYNE", 0 }
				};
				lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			topics.assign(0x94);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x95)) {
			sharedState.discussedLodgeTopic = true;
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x3339, "DWAYNE", 1);
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
				lineError = runtime.playDialogueLineWithVariant(0x3350, "DWAYNE", 0);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			topics.assign(0x97);
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
				? runtime.playDialogueLineWithVariant(0x3368, "PC", 2)
				: runtime.playDialogueLineWithVariant(0x336c, "PC", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x3370, "DWAYNE", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			topics.assign(0x99);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x9a)) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x337a, "DWAYNE", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			topics.assign(0x9b);
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
			topics.assign(0x9d);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x9e))
			continue;

		Common::Error lineError = runtime.playDialogueLineWithVariant(0x393c, "DWAYNE", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
}

} // End of namespace Harvester
