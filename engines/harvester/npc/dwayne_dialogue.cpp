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

static const char *const kDwayneNpc = "DWAYNE";
static const char *const kDwayneDnaLeftNpc = "DWAYNE_DNALFT";
static const char *const kDwayneStBedroomNpc = "DWAYNE_ST_BEDRM";
static const char *const kEdnaNpc = "EDNA";
static const char *const kPcSpeaker = "PC";
static const char *const kInventoryOwnerName = "INVENTORY";
static const char *const kRahRoomName = "RAH";

static const int kDwayneWhaleyTopicResponseLines[] = { 0x85, 0x86, 0x87 };
static const int kDwayneLoomisTopicResponseLines[] = { 0x8c, 0x8d };

} // End of namespace

bool DwayneDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase(kDwayneNpc) ||
		npcName.equalsIgnoreCase(kDwayneDnaLeftNpc) ||
		npcName.equalsIgnoreCase(kDwayneStBedroomNpc);
}

Common::Error DwayneDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	DwayneRoomDialogueState &state = _state;
	Script &startupScript = runtime.startupScript();
	Common::String &dwayneTopicBuffer = state.currentTopicBuffer;
	int &dwayneTopicBufferLineIndex = state.currentTopicBufferLineIndex;
	auto assignDwayneTopicBuffer = [&](int responseLineIndex) {
		runtime.assignTopicBuffer(dwayneTopicBuffer, dwayneTopicBufferLineIndex,
			responseLineIndex, "Dwayne topic buffer");
	};
	auto playSequence = [&](const DialogueLineEntry *lines, uint count) -> Common::Error {
		return runtime.playDialogueEntrySequence(lines, count);
	};
	auto playDwayneLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, kDwayneNpc, headVariant);
	};
	auto playEdnaLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, kEdnaNpc, headVariant);
	};
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
	auto executeActionTagIfSet = [&](const char *tag) {
		InteractionResult interaction;
		if (!runtime.executeActionTag(tag, interaction))
			return;

		runtime.applyImmediateDialogueInteractionEffects(interaction);
		runtime.queueDialogueInteractionIfNeeded(interaction);
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
			Common::Error lineError = playDwayneLine(0x38d6, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			(void)startupScript.setRuntimeFlagValue("PC_BUSTED_KILLED_MOYNAHAN", false);
		} else if (startupScript.getFlagValue("PC_BUSTED_KILLED_JIMMY")) {
			Common::Error lineError = playDwayneLine(0x384d, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			(void)startupScript.setRuntimeFlagValue("PC_BUSTED_KILLED_JIMMY", false);
			(void)startupScript.setRuntimeFlagValue("KILLED_JIMMY", false);
		} else if (startupScript.getFlagValue("PC_BUSTED_KILLED_JOHNSON")) {
			Common::Error lineError = playDwayneLine(0x386d, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			(void)startupScript.setRuntimeFlagValue("PC_BUSTED_KILLED_JOHNSON", false);
		} else if (startupScript.getFlagValue("PC_BUSTED_KILLED_PHELPS")) {
			Common::Error lineError = playDwayneLine(0x38bc, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			(void)startupScript.setRuntimeFlagValue("PC_BUSTED_KILLED_PHELPS", false);
		} else if (startupScript.getFlagValue("PC_BUSTED_KILLED_OREILLY")) {
			Common::Error lineError = playDwayneLine(0x38b5, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			(void)startupScript.setRuntimeFlagValue("PC_BUSTED_KILLED_OREILLY", false);
			(void)startupScript.setRuntimeFlagValue("KILLED_OREILLY", false);
		} else if (startupScript.getFlagValue("IF_KILL_POTTSDAM_AT_GRAVE")) {
			Common::Error lineError = playDwayneLine(0x37ed, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			(void)startupScript.setRuntimeFlagValue("IF_KILL_POTTSDAM_AT_GRAVE", false);
		} else if (startupScript.getFlagValue("GENERIC_BUST")) {
			Common::Error lineError = playDwayneLine(0x38fc, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			(void)startupScript.setRuntimeFlagValue("GENERIC_BUST", false);
		} else if (startupScript.getFlagValue("DNALFT_PERVERT")) {
			Common::Error lineError = playDwayneLine(0x3580, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			(void)startupScript.setRuntimeFlagValue("DNALFT_PERVERT", false);
		}

		if (startupScript.getFlagValue("PC_HAS_GOOJF_CARD") &&
				!startupScript.getFlagValue("PC_FRY_IN_CHAIR")) {
			Common::Error lineError = playDwayneLine(0x3814);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			if (startupScript.getFlagValue("PC_KILLED_ANYONE")) {
				const DialogueLineEntry goojfCardLines[] = {
					{ 0x3820, kDwayneNpc, 0 },
					{ 0x3824, kPcSpeaker, 0 },
					{ 0x3828, kDwayneNpc, 0 }
				};
				lineError = playSequence(goojfCardLines, ARRAYSIZE(goojfCardLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				int responseIndex = 0;
				Common::Error responseError = runtime.runResponseMenu(0x69, responseIndex);
				if (responseError.getCode() != Common::kNoError)
					return responseError;
				if (responseIndex == 1) {
					lineError = playDwayneLine(0x3832, 1);
				} else if (responseIndex == 2) {
					lineError = playDwayneLine(0x383b, 2);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
					lineError = runtime.playDialogueLineWithVariant(0x3840, kPcSpeaker, 0);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
					lineError = playDwayneLine(0x3844, 2);
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
			return playDwayneLine(0x3928, 2);
		if (startupScript.getFlagValue("BUSTED_TWICE"))
			return playDwayneLine(0x3921, 2);
		if (startupScript.getFlagValue("BUSTED_ONCE"))
			return playDwayneLine(0x3918);
		return Common::kNoError;
	}

	if (startupScript.getFlagValue("CHECK_EVIDENCE_DOOR")) {
		Common::Error lineError = playDwayneLine(0x3386);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		(void)startupScript.setRuntimeFlagValue("CHECK_EVIDENCE_DOOR", false);
		return Common::kNoError;
	}
	if (startupScript.getFlagValue("PC_TRIES_TO_TAKE_STEPHANIES_REMAINS"))
		return playDwayneLine(0x34e3, 2);
	if (runtime.currentRoomName().equalsIgnoreCase("ST_BEDRM")) {
		if (!startupScript.getFlagValue("SD_TALKED_OF_CARD")) {
			Common::Error lineError = playDwayneLine(0x34da);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			(void)startupScript.setRuntimeFlagValue("SD_TALKED_OF_CARD", true);
			return Common::kNoError;
		}
		return playDwayneLine(0x3395);
	}

	if (startupScript.getFlagValue("BRING_KARIN_TO_SHERIFF") &&
			!sheriffInDiner &&
			!state.bringKarinToSheriffLinePlayed) {
		setPendingKarinAliveFollowup();
		state.bringKarinToSheriffLinePlayed = true;
		const DialogueLineEntry lines[] = {
			{ 0x3750, kDwayneNpc, 1 },
			{ 0x3754, kPcSpeaker, 0 },
			{ 0x3759, kDwayneNpc, 0 },
			{ 0x375d, kDwayneNpc, 1 }
		};
		return playSequence(lines, ARRAYSIZE(lines));
	}
	if (startupScript.getFlagValue("KARIN_FOUND_ALIVE"))
		setPendingKarinAliveFollowup();

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("BOYLES_BUTTON")) {
			state.discussedBoylesButton = true;
			sharedState.dwayneDiscussedBoylesButton = true;
			const DialogueLineEntry lines[] = {
				{ 0x339b, kDwayneNpc, 0 },
				{ 0x33a0, kPcSpeaker, 0 },
				{ 0x33a5, kDwayneNpc, 0 },
				{ 0x33a9, kPcSpeaker, 0 },
				{ 0x33ad, kDwayneNpc, 2 }
			};
			return playSequence(lines, ARRAYSIZE(lines));
		}
		if (usedItemName.equalsIgnoreCase("K_PURSE")) {
			state.discussedKarinPurse = true;
			sharedState.discussedKarinPurse = 1;
			const DialogueLineEntry lines[] = {
				{ 0x37cb, kDwayneNpc, 1 },
				{ 0x37cf, kPcSpeaker, 3 },
				{ 0x37b4, kDwayneNpc, 3 },
				{ 0x37c1, kDwayneNpc, 0 },
				{ 0x37c5, kPcSpeaker, 0 }
			};
			return playSequence(lines, ARRAYSIZE(lines));
		}
		if (usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY")) {
			(void)startupScript.setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
			Common::Error lineError = playDwayneLine(0x33b9, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x6e, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1) {
				lineError = playDwayneLine(0x33c3, 2);
			} else if (responseIndex == 2) {
				lineError = playDwayneLine(0x33c7, 2);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			lineError = runtime.playDialogueLineWithVariant(0x33cb, kPcSpeaker, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playDwayneLine(0x33d2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			responseIndex = 0;
			responseError = runtime.runResponseMenu(0x6f, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1) {
				lineError = playDwayneLine(0x33e0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				state.presentedEvidenceReplyOverride = true;
			} else if (responseIndex == 2) {
				lineError = playDwayneLine(0x33e7, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				executeActionTagIfSet("PC_FRY_DADDY_1");
			}
			return Common::kNoError;
		}
		if (usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("NOTE")) {
			(void)startupScript.setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
			if (state.presentedEvidenceReplyOverride) {
				const DialogueLineEntry introLines[] = {
					{ 0x342c, kDwayneNpc, 0 },
					{ 0x3430, kPcSpeaker, 0 },
					{ 0x3435, kDwayneNpc, 2 }
				};
				Common::Error lineError = playSequence(introLines, ARRAYSIZE(introLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				int responseIndex = 0;
				Common::Error responseError = runtime.runResponseMenu(0x72, responseIndex);
				if (responseError.getCode() != Common::kNoError)
					return responseError;
				if (responseIndex == 1) {
					lineError = playDwayneLine(0x3440, 2);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
					executeActionTagIfSet("PC_FRY_DADDY_1");
					return Common::kNoError;
				}
				if (responseIndex == 2) {
					lineError = playDwayneLine(0x3448);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
					confiscateSheriffEvidence();
					awardGoojfCard();
				}
				return Common::kNoError;
			}

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

			lineError = runtime.playDialogueLineWithVariant(0x3407, kPcSpeaker, 2);
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
			if (responseIndex == 1) {
				lineError = playDwayneLine(0x341a);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				executeActionTagIfSet("PC_FRY_DADDY_1");
				return Common::kNoError;
			}
			if (responseIndex == 2) {
				lineError = playDwayneLine(0x3422, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				executeActionTagIfSet("PC_FRY_DADDY_1");
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
				{ 0x3452, kDwayneNpc, 0 },
				{ 0x3457, kPcSpeaker, 0 },
				{ 0x345c, kDwayneNpc, 0 },
				{ 0x3460, kPcSpeaker, 2 },
				{ 0x3464, kDwayneNpc, 0 },
				{ 0x346a, kPcSpeaker, 2 },
				{ 0x346e, kDwayneNpc, 0 },
				{ 0x3474, kPcSpeaker, 4 },
				{ 0x3478, kDwayneNpc, 2 }
			};
			Common::Error lineError = playSequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			(void)startupScript.resetRuntimeObjectToInitialState("LEDGER");
			(void)startupScript.resetRuntimeObjectToInitialState("LEDGER2");
			executeActionTagIfSet(sheriffInDiner ? "CALL_POLICE" : "LEDGER_BUST");
			return Common::kNoError;
		}
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			sharedState.discussedWhaleyHerrillPhoto = 1;
			(void)startupScript.setRuntimeFlagValue(DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
			const DialogueLineEntry lines[] = {
				{ 0x3975, kDwayneNpc, 3 },
				{ 0x3979, kPcSpeaker, 0 },
				{ 0x397e, kDwayneNpc, 0 }
			};
			return playSequence(lines, ARRAYSIZE(lines));
		}
		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			sharedState.discussedCasketPhotoEvidence = 1;
			(void)startupScript.setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);

			Common::Error lineError = playDwayneLine(0x3942, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			if (sharedState.dialogueStateD2ec8) {
				const DialogueLineEntry lines[] = {
					{ 0x3949, kPcSpeaker, 0 },
					{ 0x3950, kDwayneNpc, 0 },
					{ 0x3955, kPcSpeaker, 4 },
					{ 0x3959, kDwayneNpc, 1 },
					{ 0x396d, kDwayneNpc, 0 }
				};
				lineError = playSequence(lines, ARRAYSIZE(lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				hideObjectIfPresent(kInventoryOwnerName, usedItemName);
				executeActionTagIfSet("CALL_POLICE");
			} else {
				const DialogueLineEntry lines[] = {
					{ 0x3961, kPcSpeaker, 0 },
					{ 0x3967, kDwayneNpc, 0 },
					{ 0x396d, kDwayneNpc, 0 }
				};
				lineError = playSequence(lines, ARRAYSIZE(lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				hideObjectIfPresent(kInventoryOwnerName, usedItemName);
			}
			return Common::kNoError;
		}
		if (state.tvDeedReplyOverride && usedItemName.equalsIgnoreCase("TV_DEED")) {
			(void)startupScript.setRuntimeFlagValue(DialogueFlags::kShownEvidenceSheriffOwns, true);

			const DialogueLineEntry introLines[] = {
				{ 0x3686, kDwayneNpc, 0 },
				{ 0x368a, kPcSpeaker, 0 },
				{ 0x3690, kDwayneNpc, 0 }
			};
			Common::Error lineError = playSequence(introLines, ARRAYSIZE(introLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x73, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1) {
				lineError = playDwayneLine(0x369b, 2);
			} else if (responseIndex == 2) {
				const DialogueLineEntry responseLines[] = {
					{ 0x36a3, kDwayneNpc, 0 },
					{ 0x36a8, kPcSpeaker, 2 },
					{ 0x36ac, kDwayneNpc, 2 }
				};
				lineError = playSequence(responseLines, ARRAYSIZE(responseLines));
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			confiscateSheriffEvidence();
			awardGoojfCard();
			return Common::kNoError;
		}
		if (!usedItemName.equalsIgnoreCase("TV_DEED") &&
				!usedItemName.equalsIgnoreCase("TV_DEED_PHOTOCOPY")) {
			return playDwayneLine(0x3935);
		}

		(void)startupScript.setRuntimeFlagValue(DialogueFlags::kShownEvidenceSheriffOwns, true);

		Common::Error lineError = playDwayneLine(0x3608);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x74, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;
		if (responseIndex == 1) {
			lineError = playDwayneLine(0x3613);
		} else if (responseIndex == 2) {
			lineError = playDwayneLine(0x3618);
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = playDwayneLine(0x361c);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		responseIndex = 0;
		responseError = runtime.runResponseMenu(0x75, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;
		if (responseIndex == 1) {
			lineError = playDwayneLine(0x3629, 2);
		} else if (responseIndex == 2) {
			lineError = playDwayneLine(0x362d);
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		responseIndex = 0;
		responseError = runtime.runResponseMenu(0x76, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;
		if (usedItemName.equalsIgnoreCase("TV_DEED")) {
			const DialogueLineEntry deedLines[] = {
				{ 0x363b, kDwayneNpc, 0 },
				{ 0x3641, kPcSpeaker, 0 },
				{ 0x3645, kDwayneNpc, 0 }
			};
			lineError = playSequence(deedLines, ARRAYSIZE(deedLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			executeActionTagIfSet("PC_FRY_DADDY_1");
			return Common::kNoError;
		}
		if (responseIndex != 1) {
			lineError = playDwayneLine(0x367b, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			hideObjectIfPresent(kRahRoomName, "TV_DEED_PHOTOCOPY");
			return Common::kNoError;
		}

		const DialogueLineEntry copyLines[] = {
			{ 0x364e, kDwayneNpc, 0 },
			{ 0x3656, kPcSpeaker, 0 },
			{ 0x365b, kDwayneNpc, 0 }
		};
		lineError = playSequence(copyLines, ARRAYSIZE(copyLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		responseIndex = 0;
		responseError = runtime.runResponseMenu(0x77, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;
		if (responseIndex == 1) {
			lineError = playDwayneLine(0x3665);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			state.tvDeedReplyOverride = true;
			return Common::kNoError;
		}
		if (responseIndex == 2) {
			const DialogueLineEntry responseLines[] = {
				{ 0x366b, kDwayneNpc, 3 },
				{ 0x3670, kPcSpeaker, 2 },
				{ 0x3674, kDwayneNpc, 2 }
			};
			lineError = playSequence(responseLines, ARRAYSIZE(responseLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			executeActionTagIfSet("PC_FRY_DADDY_1");
		}
		return Common::kNoError;
	}

	bool handledIntroOrRevisit = false;
	if (state.sheriffInDinerIntroPending && sheriffInDiner) {
		state.sheriffInDinerIntroPending = false;
		state.sheriffInDinerIntroPlayed = true;
		setDwayneIntroduced();

		const DialogueLineEntry introLines[] = {
			{ 0x3482, kDwayneNpc, 1 },
			{ 0x3486, kPcSpeaker, 0 },
			{ 0x348b, kDwayneNpc, 1 },
			{ 0x3490, kEdnaNpc, 0 }
		};
		Common::Error lineError = playSequence(introLines, ARRAYSIZE(introLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (startupScript.getFlagValue("KARIN_KIDNAPED")) {
			sharedState.karinKidnapedDialogueState = true;
			lineError = playEdnaLine(0x3496, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		lineError = playDwayneLine(0x349b, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (state.pendingInitialConversation) {
			const DialogueLineEntry initialLines[] = {
				{ 0x34a0, kPcSpeaker, 0 },
				{ 0x34a4, kDwayneNpc, 1 }
			};
			lineError = playSequence(initialLines, ARRAYSIZE(initialLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		handledIntroOrRevisit = true;
	}
	else if (state.sheriffInDinerIntroPlayed && sheriffInDiner) {
		Common::Error lineError = playDwayneLine(0x34ac);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		handledIntroOrRevisit = true;
	} else if (state.pendingInitialConversation && !sheriffInDiner) {
		state.pendingInitialConversation = false;
		state.eventFollowupGate = true;
		setDwayneIntroduced();

		const DialogueLineEntry initialLines[] = {
			{ 0x3245, kDwayneNpc, 1 },
			{ 0x324a, "LOOMIS", 1 },
			{ 0x324e, kPcSpeaker, 0 },
			{ 0x3252, kDwayneNpc, 0 }
		};
		Common::Error lineError = playSequence(initialLines, ARRAYSIZE(initialLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		assignDwayneTopicBuffer(0x78);
		if (sharedState.boyleGascanApplicationState)
			(void)startupScript.setRuntimeFlagValue("MOVE_SHERIFF", true);
		handledIntroOrRevisit = true;
	}

	if (!handledIntroOrRevisit && state.eventFollowupGate) {
		Common::Error lineError = playDwayneLine(0x34d3, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (sharedState.dwayneWhaleyDisciplineFollowupState &&
			!state.whaleyDisciplineFollowupShown) {
		state.whaleyDisciplineFollowupShown = true;
		Common::Error lineError = playDwayneLine(0x34b3);
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

	if (sharedState.discussedNoteCheckbookEvidence != 0 &&
			!state.noteCheckbookFollowupShown) {
		state.noteCheckbookFollowupShown = true;
		Common::Error lineError = playDwayneLine(0x355f, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (sharedState.discussedTvDeedEvidence != 0) {
			const DialogueLineEntry lines[] = {
				{ 0x3565, kDwayneNpc, 2 },
				{ 0x356a, kDwayneNpc, 2 },
				{ 0x356f, kPcSpeaker, 2 },
				{ 0x3573, kDwayneNpc, 0 }
			};
			lineError = playSequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			executeActionTagIfSet("PC_FRY_DADDY_1");
			return Common::kNoError;
		}
	}

	if (!sheriffInDiner) {
		if (startupScript.getFlagValue("SCRATCHED_TUCKER") &&
				!state.scratchedTuckerShown) {
			state.scratchedTuckerShown = true;
			Common::Error lineError = playDwayneLine(0x358b, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (startupScript.getFlagValue("BOLT_OF_CLOTH_TAKEN") &&
				!state.boltOfClothTakenShown) {
			state.boltOfClothTakenShown = true;
			Common::Error lineError = playDwayneLine(0x3594, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (startupScript.getFlagValue("BARBER_POLE_STOLEN") &&
				!state.barberPoleStolenShown) {
			state.barberPoleStolenShown = true;
			const DialogueLineEntry lines[] = {
				{ 0x359d, kDwayneNpc, 0 },
				{ 0x35a4, kDwayneNpc, 0 },
				{ 0x35ab, kDwayneNpc, 0 }
			};
			Common::Error lineError = playSequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (startupScript.getFlagValue("DINER_BURNED") &&
				!state.dinerBurnedShown) {
			state.dinerBurnedShown = true;
			Common::Error lineError = playDwayneLine(0x35bc, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (startupScript.getFlagValue("KARIN_FOUND_ALIVE")) {
				const DialogueLineEntry lines[] = {
					{ 0x35c1, kDwayneNpc, 3 },
					{ 0x35c5, kDwayneNpc, 3 }
				};
				lineError = playSequence(lines, ARRAYSIZE(lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
		}
		if (startupScript.getFlagValue("PC_ESCAPED_JAIL") &&
				!state.escapedJailShown) {
			state.escapedJailShown = true;
			Common::Error lineError = playDwayneLine(0x35cd);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (startupScript.getFlagValue("GOT_REMAINS_FOR_LODGE") &&
				!state.gotRemainsForLodgeShown) {
			state.gotRemainsForLodgeShown = true;
			Common::Error lineError = playDwayneLine(0x35d7);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (startupScript.getFlagValue("BURNED_TV_STATION") &&
				!state.burnedTvStationShown) {
			state.burnedTvStationShown = true;
			const DialogueLineEntry lines[] = {
				{ 0x35f6, kDwayneNpc, 2 },
				{ 0x35fb, kDwayneNpc, 2 },
				{ 0x3601, kDwayneNpc, 2 }
			};
			Common::Error lineError = playSequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (startupScript.getFlagValue("KARIN_KIDNAPED") &&
				!state.karinKidnapedShown) {
			state.karinKidnapedShown = true;
			Common::Error lineError = playDwayneLine(0x36b2, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			sharedState.karinKidnapedDialogueState = true;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x7b, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1) {
				lineError = playDwayneLine(0x36bd);
			} else if (responseIndex == 2) {
				lineError = playDwayneLine(0x36c1);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			lineError = playDwayneLine(0x36c5);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			responseIndex = 0;
			responseError = runtime.runResponseMenu(0x7c, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1) {
				lineError = playDwayneLine(0x36d0);
			} else if (responseIndex == 2) {
				lineError = playDwayneLine(0x36d6, 1);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			lineError = playDwayneLine(0x36e7, 1);
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
			{ 0x3780, kDwayneNpc, 0 },
			{ 0x3786, kPcSpeaker, 4 },
			{ 0x378a, kDwayneNpc, 2 }
		};
		Common::Error lineError = playSequence(lines, ARRAYSIZE(lines));
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
		Common::Error lineError = playDwayneLine(0x3793);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x7f, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;
		if (responseIndex == 1) {
			return playDwayneLine(0x379d);
		} else if (responseIndex == 2) {
			lineError = playDwayneLine(0x37a4, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			responseIndex = 0;
			responseError = runtime.runResponseMenu(0x80, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1) {
				lineError = playDwayneLine(0x37ae);
			} else if (responseIndex == 2) {
				lineError = playDwayneLine(0x37b4, 3);
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			const DialogueLineEntry lines[] = {
				{ 0x37c1, kDwayneNpc, 0 },
				{ 0x37c5, kPcSpeaker, 0 }
			};
			lineError = playSequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}

	if (!startupScript.isNamedNpcDeathTypeClear("JIMMY") &&
			!sheriffInDiner &&
			!state.jimmyAbsentShown) {
		state.jimmyAbsentShown = true;
		Common::Error lineError = playDwayneLine(0x3855, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (startupScript.getFlagValue("JIMMY_ATTACKED")) {
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

	if (!startupScript.isNamedNpcDeathTypeClear("MOYNAHAN") &&
			!sheriffInDiner &&
			!state.moynahanAbsentShown) {
		state.moynahanAbsentShown = true;
		Common::Error lineError = playDwayneLine(0x38ce);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (startupScript.getFlagValue("EDNA_HUNG") &&
			!state.ednaHungShown) {
		state.ednaHungShown = true;
		Common::Error lineError = playDwayneLine(0x38f1, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (!startupScript.isNamedNpcDeathTypeClear("MCKNIGHT") &&
			!sheriffInDiner &&
			!state.mcknightAbsentShown) {
		state.mcknightAbsentShown = true;
		Common::Error lineError = startupScript.isObjectInInventory("TV_DEED")
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
					{ 0x327b, kDwayneNpc, 0 },
					{ 0x3280, kPcSpeaker, 2 },
					{ 0x3285, kDwayneNpc, 0 },
					{ 0x328b, "LOOMIS", 1 },
					{ 0x3290, kDwayneNpc, 2 },
					{ 0x3294, "LOOMIS", 3 },
					{ 0x3299, kPcSpeaker, 4 },
					{ 0x329e, kDwayneNpc, 1 }
				};
				Common::Error lineError = playSequence(lines, ARRAYSIZE(lines));
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
					{ 0x32bf, kDwayneNpc, 1 },
					{ 0x32c1, kDwayneNpc, 0 },
					{ 0x32c7, "LOOMIS", 3 },
					{ 0x32cb, kDwayneNpc, 0 },
					{ 0x32d1, "LOOMIS", 1 }
				};
				Common::Error lineError = playSequence(lines, ARRAYSIZE(lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				runtime.setActiveSpeakerPortrait(kDwayneNpc, 0);
				assignDwayneTopicBuffer(0x8e);
			}
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x8f)) {
			const DialogueLineEntry lines[] = {
				{ 0x32db, kDwayneNpc, 0 },
				{ 0x32e2, kPcSpeaker, 4 },
				{ 0x32e6, kDwayneNpc, 0 }
			};
			Common::Error lineError = playSequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignDwayneTopicBuffer(0x90);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x91)) {
			const DialogueLineEntry lines[] = {
				{ 0x32f2, kDwayneNpc, 0 },
				{ 0x32f9, kPcSpeaker, 0 },
				{ 0x32fd, kDwayneNpc, 0 },
				{ 0x32fe, kDwayneNpc, 0 },
				{ 0x32ff, kDwayneNpc, 0 }
			};
			Common::Error lineError = playSequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignDwayneTopicBuffer(0x92);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x93)) {
			Common::Error lineError = Common::kNoError;
			if (!startupScript.getFlagValue("STEPH_MIDGAME_PLAYED")) {
				const DialogueLineEntry lines[] = {
					{ 0x330d, kDwayneNpc, 1 },
					{ 0x3314, kPcSpeaker, 0 },
					{ 0x3318, kDwayneNpc, 0 },
					{ 0x332c, kDwayneNpc, 0 }
				};
				lineError = playSequence(lines, ARRAYSIZE(lines));
			} else {
				const DialogueLineEntry lines[] = {
					{ 0x331f, kDwayneNpc, 3 },
					{ 0x3320, kDwayneNpc, 0 },
					{ 0x3321, kDwayneNpc, 0 },
					{ 0x332c, kDwayneNpc, 0 }
				};
				lineError = playSequence(lines, ARRAYSIZE(lines));
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignDwayneTopicBuffer(0x94);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x95)) {
			sharedState.discussedLodgeTopic = true;
			Common::Error lineError = playDwayneLine(0x3339, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x96, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			lineError = Common::kNoError;
			if (responseIndex == 1) {
				const DialogueLineEntry responseLines[] = {
					{ 0x3345, kDwayneNpc, 2 },
					{ 0x3346, kDwayneNpc, 2 },
					{ 0x3347, kDwayneNpc, 2 }
				};
				lineError = playSequence(responseLines, ARRAYSIZE(responseLines));
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
				{ 0x335b, kDwayneNpc, 2 },
				{ 0x3360, kPcSpeaker, 2 },
				{ 0x3364, kDwayneNpc, 3 }
			};
			Common::Error lineError = playSequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = sharedState.dialogueStateD2f04
				? runtime.playDialogueLineWithVariant(0x3368, kPcSpeaker, 2)
				: runtime.playDialogueLineWithVariant(0x336c, kPcSpeaker, 2);
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
				{ 0x3386, kDwayneNpc, 0 },
				{ 0x3387, kDwayneNpc, 0 }
			};
			Common::Error lineError = playSequence(lines, ARRAYSIZE(lines));
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
