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


#include "harvester/npc/herrill_dialogue.h"

#include "common/array.h"
#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kHerrillNpc = "HERRILL";
static const char *const kHerrillLogNpc = "HERRILL_LOG";
static const char *const kPcSpeaker = "PC";
static const char *const kGrandMuckNpc = "GRAND_MUCK";
static const int kGrandMuckDeathDamageType = 1;

static const int kHerrillGascanResponseLine = 0x280;
static const int kHerrillTopicBufferResponseLine = 0x281;

static const DialogueLineEntry kHerrillIntroLines[] = {
	{ 0x2f59, kHerrillNpc, 1 },
	{ 0x2f5e, kPcSpeaker, 0 },
	{ 0x2f62, kHerrillNpc, 1 },
	{ 0x1654, kPcSpeaker, 0 },
	{ 0x2f6e, kHerrillNpc, 0 }
};

static const DialogueLineEntry kHerrillGascanIntroLines[] = {
	{ 0x30cf, kHerrillNpc, 1 },
	{ 0x30d3, kPcSpeaker, 0 },
	{ 0x30d8, kHerrillNpc, 0 }
};

} // End of namespace

bool HerrillDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase(kHerrillNpc) ||
		npcName.equalsIgnoreCase(kHerrillLogNpc);
}

Common::Error HerrillDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	HerrillRoomDialogueState &state = _state;
	Common::String &herrillTopicBuffer = state.currentTopicBuffer;
	int &herrillTopicBufferLineIndex = state.currentTopicBufferLineIndex;

	auto assignHerrillTopicBuffer = [&](int responseLineIndex) {
		runtime.assignTopicBuffer(herrillTopicBuffer, herrillTopicBufferLineIndex,
			responseLineIndex, "Herrill topic buffer");
	};
	auto playHerrillLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, kHerrillNpc, headVariant);
	};
	auto playHerrillLogLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, kHerrillLogNpc, headVariant);
	};
	auto playPcLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, kPcSpeaker, headVariant);
	};
	auto playGrandMuckLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, kGrandMuckNpc, headVariant);
	};
	auto queueLiveNpcDeathTransition = [&](const char *npcName) {
		InteractionResult interaction;
		if (runtime.startupScript().queueRuntimeNpcDeathOrMonsterfy(
				npcName, kGrandMuckDeathDamageType)) {
			interaction.mutatedRuntimeState = true;
			interaction.visualRuntimeStateChanged = true;
		}
		runtime.queueDialogueInteractionIfNeeded(interaction);
	};
	auto hasInventoryItem = [&](const char *objectName) {
		Common::Array<ObjectRecord> inventoryObjects;
		runtime.startupScript().getVisibleInventoryObjects(inventoryObjects);
		for (const ObjectRecord &inventoryObject : inventoryObjects) {
			if (inventoryObject.objectName.equalsIgnoreCase(objectName))
				return true;
		}

		return false;
	};
	auto hasRequiredEvidence = [&]() {
		return hasInventoryItem("CHECKBOOK") ||
			hasInventoryItem("CHECKBOOK_PHOTOCOPY") ||
			hasInventoryItem("NOTE") ||
			hasInventoryItem("NOTE_PHOTOCOPY") ||
			hasInventoryItem("TV_DEED") ||
			hasInventoryItem("TV_DEED_PHOTOCOPY");
	};
	auto shownWhaleyHerrillPhoto = [&]() {
		return runtime.startupScript().getFlagValue(DialogueFlags::kShownPhotoOfWhaleyHerrill);
	};

	if (runtime.startupScript().getFlagValue("HERRILL_IN_LODGE")) {
		if (!runtime.startupScript().getFlagValue("PC_TALKED_TO_HERRILL")) {
			Common::Error lineError = playHerrillLogLine(0x31ca, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playPcLine(0x31ce, 4);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playHerrillLogLine(0x31d2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playPcLine(0x31da);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playGrandMuckLine(0x31de, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (shownWhaleyHerrillPhoto()) {
				lineError = playGrandMuckLine(0x31e0, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playHerrillLogLine(0x31e2, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			queueLiveNpcDeathTransition(kGrandMuckNpc);
			(void)runtime.startupScript().setRuntimeFlagValue("PC_TALKED_TO_HERRILL", true);
		}
		return Common::kNoError;
	}

	if (state.dialogueSuppressed)
		return Common::kNoError;
	if (state.requiresEvidenceForDialogue && !hasRequiredEvidence())
		return Common::kNoError;

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			return playHerrillLine(0x309f, 2);
		}
		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
			return playHerrillLine(0x30a6);
		}
		if ((usedItemName.equalsIgnoreCase("LEDGER") ||
					usedItemName.equalsIgnoreCase("LEDGER2")) &&
				runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownLedgersToAnyone, true);
			return playHerrillLine(0x30a6);
		}
		if (usedItemName.equalsIgnoreCase("GASCAN")) {
			state.gascanPresented = true;
		} else if (usedItemName.equalsIgnoreCase("TV_DEED") ||
				usedItemName.equalsIgnoreCase("TV_DEED_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceSheriffOwns, true);
			return playHerrillLine(0x3188);
		} else if (usedItemName.equalsIgnoreCase("NOTE") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
			return playHerrillLine(0x30bf);
		} else {
			return playHerrillLine(0x3099);
		}
	}

	if (state.gascanPresented) {
		state.gascanPresented = false;

		Common::Error lineError = runtime.playDialogueEntrySequence(
			kHerrillGascanIntroLines, ARRAYSIZE(kHerrillGascanIntroLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (!sharedState.dialogueStateD2e98)
			return playPcLine(0x3123);

		state.dialogueStateD2ce4 = true;
		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(kHerrillGascanResponseLine, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playHerrillLine(0x30e7);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (sharedState.dialogueStateD2eb0) {
				lineError = playHerrillLine(0x30eb);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			if (sharedState.boyleGascanApplicationState) {
				lineError = playHerrillLine(0x30f4);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			if (runtime.startupScript().getFlagValue("HAVE_LODGE_APP")) {
				lineError = playPcLine(0x30fa);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playHerrillLine(0x30ff, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				if (sharedState.dialogueStateD2eec) {
					lineError = playHerrillLine(0x3104);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				}
				return playHerrillLine(0x3109, 2);
			}

			lineError = playHerrillLine(0x30f0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playPcLine(0x3110);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			return playHerrillLine(0x3114, 3);
		}
		if (responseIndex == 2)
			return playHerrillLine(0x311b, 1);

		return Common::kNoError;
	}

	if (state.introPending) {
		state.introPending = false;
		Common::Error lineError = runtime.playDialogueEntrySequence(
			kHerrillIntroLines, ARRAYSIZE(kHerrillIntroLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		assignHerrillTopicBuffer(kHerrillTopicBufferResponseLine);
	} else {
		Common::Error lineError = playHerrillLine(0x2ffb);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") &&
			!state.stephMidgamePlayedShown) {
		state.stephMidgamePlayedShown = true;
		Common::Error lineError = playHerrillLine(0x3004, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getCurrentStoryDayIndex() == 5 &&
			!state.dayFiveShown) {
		state.dayFiveShown = true;
		Common::Error lineError = playHerrillLine(0x300d, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("SCRATCHED_TUCKER") &&
			!state.scratchedTuckerShown) {
		state.scratchedTuckerShown = true;
		Common::Error lineError = playHerrillLine(0x3019, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playHerrillLine(0x3022, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("BARBER_POLE_STOLEN") &&
			!state.barberPoleStolenShown) {
		state.barberPoleStolenShown = true;
		Common::Error lineError = playHerrillLine(0x3029);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playHerrillLine(0x302f, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("KARIN_KIDNAPED") &&
			!state.karinKidnapedShown) {
		state.karinKidnapedShown = true;
		sharedState.karinKidnapedDialogueState = true;
		Common::Error lineError = playHerrillLine(0x3041);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("KARIN_FOUND_ALIVE") &&
			!state.karinFoundAliveShown) {
		state.karinFoundAliveShown = true;
		Common::Error lineError = playHerrillLine(0x304c, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("KARIN_FOUND_DEAD") &&
			!state.karinFoundDeadShown) {
		state.karinFoundDeadShown = true;
		Common::Error lineError = playHerrillLine(0x305e);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED")) {
			lineError = playHerrillLine(0x3067, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		lineError = shownWhaleyHerrillPhoto()
			? playHerrillLine(0x306b, 2)
			: playHerrillLine(0x3073, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("DINER_BURNED") &&
			!state.dinerBurnedShown) {
		state.dinerBurnedShown = true;
		Common::Error lineError = playHerrillLine(0x307d, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (shownWhaleyHerrillPhoto() && !state.whaleyHerrillFollowupShown) {
		state.whaleyHerrillFollowupShown = true;
		Common::Error lineError = playHerrillLine(0x31a6, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (herrillTopicBufferLineIndex < 0)
		assignHerrillTopicBuffer(kHerrillTopicBufferResponseLine);

	for (;;) {
		Common::String selectedTopic;
		Common::Error menuError = runtime.runKeywordMenu(
			herrillTopicBuffer, herrillTopicBufferLineIndex, selectedTopic);
		if (menuError.getCode() != Common::kNoError)
			return menuError;
		if (selectedTopic.empty())
			return Common::kNoError;
		if (runtime.matchesResponseLine(selectedTopic, 0x283)) {
			return playHerrillLine(0x2ff4);
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x284)) {
			Common::Error lineError = playPcLine(0x2f7b, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x286)) {
			Common::Error lineError = playHerrillLine(0x2f8f);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x288)) {
			Common::Error lineError = playHerrillLine(0x2f9e, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x28a)) {
			Common::Error lineError = playPcLine(0x2fb2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x28c)) {
			Common::Error lineError = playPcLine(0x2fcd);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x28e)) {
			sharedState.discussedLodgeTopic = true;
			Common::Error lineError = playHerrillLine(0x2fdd);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x28f)) {
			sharedState.discussedLodgeTopic = true;
			Common::Error lineError = playHerrillLine(0x2fdd);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x291)) {
			Common::Error lineError = playHerrillLine(0x2fe9, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x293))
			return playHerrillLine(0x2ff4);

		Common::Error lineError = playHerrillLine(0x3099);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		return playHerrillLine(0x2ff4);
	}
}

} // End of namespace Harvester
