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

static const char *const kGrandMuckNpc = "GRAND_MUCK";
static const int kGrandMuckDeathDamageType = 1;

static const int kHerrillGascanResponseLine = 0x280;
static const int kHerrillTopicBufferResponseLine = 0x281;

static const DialogueLineEntry kHerrillIntroLines[] = {
	{ 0x2f59, "HERRILL", 1 },
	{ 0x2f5e, "PC", 0 },
	{ 0x2f62, "HERRILL", 1 },
	{ 0x1654, "PC", 0 },
	{ 0x2f6e, "HERRILL", 0 }
};

static const DialogueLineEntry kHerrillGascanIntroLines[] = {
	{ 0x30cf, "HERRILL", 1 },
	{ 0x30d3, "PC", 0 },
	{ 0x30d8, "HERRILL", 0 }
};

} // End of namespace

Common::Error HerrillDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	HerrillRoomDialogueState &state = _state;
	Common::String &herrillTopicBuffer = state.currentTopicBuffer;
	int &herrillTopicBufferLineIndex = state.currentTopicBufferLineIndex;
	state.currentTopicBuffer.clear();
	state.currentTopicBufferLineIndex = -1;

	DialogueTopicBuffer topics(runtime, herrillTopicBuffer, herrillTopicBufferLineIndex, "Herrill topic buffer");

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
		return sharedState.discussedWhaleyHerrillPhoto != 0;
	};

	if (runtime.startupScript().getFlagValue("HERRILL_IN_LODGE")) {
		if (!runtime.startupScript().getFlagValue("PC_TALKED_TO_HERRILL")) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x31ca, "HERRILL_LOG", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x31ce, "PC", 4);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x31d2, "HERRILL_LOG", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x31da, "PC", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x31de, kGrandMuckNpc, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (shownWhaleyHerrillPhoto()) {
				lineError = runtime.playDialogueLineWithVariant(0x31e0, kGrandMuckNpc, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueLineWithVariant(0x31e2, "HERRILL_LOG", 2);
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

	bool showGascanResponse = false;
	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			return runtime.playDialogueLineWithVariant(0x309f, "HERRILL", 2);
		}
		const bool isCasketPhoto = usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
			usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY");
		const bool isLedger = (usedItemName.equalsIgnoreCase("LEDGER") ||
			usedItemName.equalsIgnoreCase("LEDGER2")) &&
			runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS");
		if (isCasketPhoto || isLedger) {
			if (isCasketPhoto) {
				sharedState.discussedCasketPhotoEvidence = 1;
				(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
			} else {
				sharedState.discussedLedgerEvidence = 1;
				(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownLedgersToAnyone, true);
			}
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x30a6, "HERRILL", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x27f, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;
			if (responseIndex == 1)
				return runtime.playDialogueLineWithVariant(0x30b1, "HERRILL", 2);
			if (responseIndex == 2)
				return runtime.playDialogueLineWithVariant(0x30b6, "HERRILL", 2);
			return Common::kNoError;
		}
		if (usedItemName.equalsIgnoreCase("GASCAN")) {
			state.gascanPresented = true;
		} else if (usedItemName.equalsIgnoreCase("TV_DEED") ||
				usedItemName.equalsIgnoreCase("TV_DEED_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceSheriffOwns, true);
			sharedState.discussedTvDeedEvidence = 1;
			const DialogueLineEntry lines[] = {
				{ 0x3188, "HERRILL", 0 },
				{ 0x318c, "PC", 0 },
				{ 0x3191, "HERRILL", 2 }
			};
			Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			sharedState.dialogueStateD2eec = true;
			if (!state.dialogueStateD2ce4)
				return Common::kNoError;
			lineError = runtime.playDialogueLineWithVariant(0x3198, "HERRILL", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			return runtime.playDialogueLineWithVariant(0x319d, "HERRILL", 2);
		} else if (usedItemName.equalsIgnoreCase("NOTE") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
			sharedState.discussedNoteCheckbookEvidence = 1;
			const DialogueLineEntry lines[] = {
				{ 0x30bf, "HERRILL", 0 },
				{ 0x30c4, "PC", 0 },
				{ 0x30c8, "HERRILL", 0 }
			};
			Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			showGascanResponse = true;
		} else {
			return runtime.playDialogueLineWithVariant(0x3099, "HERRILL", 0);
		}
	}

	if (state.gascanPresented || showGascanResponse) {
		Common::Error lineError;
		if (state.gascanPresented) {
			lineError = runtime.playDialogueEntrySequence(
				kHerrillGascanIntroLines, ARRAYSIZE(kHerrillGascanIntroLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			state.gascanPresented = false;
		}
		if (!sharedState.dialogueStateD2e98)
			return runtime.playDialogueLineWithVariant(0x3123, "PC", 0);

		state.dialogueStateD2ce4 = true;
		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(kHerrillGascanResponseLine, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = runtime.playDialogueLineWithVariant(0x30e7, "HERRILL", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (sharedState.dialogueStateD2eb0) {
				lineError = runtime.playDialogueLineWithVariant(0x30eb, "PC", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			if (sharedState.boyleGascanApplicationState) {
				lineError = runtime.playDialogueLineWithVariant(0x30f4, "HERRILL", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			if (runtime.startupScript().getFlagValue("HAVE_LODGE_APP")) {
				lineError = runtime.playDialogueLineWithVariant(0x30fa, "PC", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueLineWithVariant(0x30ff, "HERRILL", 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				if (sharedState.dialogueStateD2eec) {
					lineError = runtime.playDialogueLineWithVariant(0x3104, "HERRILL", 0);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				}
				return runtime.playDialogueLineWithVariant(0x3109, "HERRILL", 2);
			}

			lineError = runtime.playDialogueLineWithVariant(0x30f0, "HERRILL", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x3110, "PC", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			return runtime.playDialogueLineWithVariant(0x3114, "HERRILL", 3);
		}
		if (responseIndex == 2)
			return runtime.playDialogueLineWithVariant(0x311b, "HERRILL", 1);

		return Common::kNoError;
	}

	if (state.introPending) {
		state.introPending = false;
		Common::Error lineError = runtime.playDialogueEntrySequence(
			kHerrillIntroLines, ARRAYSIZE(kHerrillIntroLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		topics.assign(kHerrillTopicBufferResponseLine);
	} else {
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x2ffb, "HERRILL", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") &&
			!state.stephMidgamePlayedShown) {
		state.stephMidgamePlayedShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x3004, "HERRILL", 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getCurrentStoryDayIndex() == 5 &&
			!state.dayFiveShown) {
		state.dayFiveShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x300d, "HERRILL", 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("SCRATCHED_TUCKER") &&
			!state.scratchedTuckerShown) {
		state.scratchedTuckerShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x3019, "HERRILL", 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueLineWithVariant(0x3022, "HERRILL", 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("BARBER_POLE_STOLEN") &&
			!state.barberPoleStolenShown) {
		state.barberPoleStolenShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x3029, "HERRILL", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueLineWithVariant(0x302f, "HERRILL", 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("KARIN_KIDNAPED") &&
			!state.karinKidnapedShown) {
		state.karinKidnapedShown = true;
		sharedState.karinKidnapedDialogueState = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x3041, "HERRILL", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("KARIN_FOUND_ALIVE") &&
			!state.karinFoundAliveShown) {
		state.karinFoundAliveShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x304c, "HERRILL", 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = shownWhaleyHerrillPhoto()
			? runtime.playDialogueLineWithVariant(0x3050, "HERRILL", 2)
			: runtime.playDialogueLineWithVariant(0x3057, "HERRILL", 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("KARIN_FOUND_DEAD") &&
			!state.karinFoundDeadShown) {
		state.karinFoundDeadShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x305e, "HERRILL", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueLineWithVariant(0x461b, "PC", 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED")) {
			lineError = runtime.playDialogueLineWithVariant(0x3067, "HERRILL", 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		lineError = shownWhaleyHerrillPhoto()
			? runtime.playDialogueLineWithVariant(0x306b, "HERRILL", 2)
			: runtime.playDialogueLineWithVariant(0x3073, "HERRILL", 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("DINER_BURNED") &&
			!state.dinerBurnedShown) {
		state.dinerBurnedShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x307d, "HERRILL", 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (shownWhaleyHerrillPhoto() && !state.whaleyHerrillFollowupShown) {
		state.whaleyHerrillFollowupShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x31a6, "HERRILL", 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x282, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;
		if (responseIndex == 1) {
			lineError = runtime.playDialogueLineWithVariant(0x31b1, "HERRILL", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			state.dialogueSuppressed = true;
			return Common::kNoError;
		}
		if (responseIndex == 2) {
			lineError = runtime.playDialogueLineWithVariant(0x31b9, "HERRILL", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			state.requiresEvidenceForDialogue = true;
			return Common::kNoError;
		}
	}

	for (;;) {
		Common::String selectedTopic;
		Common::Error menuError = runtime.runKeywordMenu(
			herrillTopicBuffer, herrillTopicBufferLineIndex, selectedTopic);
		if (menuError.getCode() != Common::kNoError)
			return menuError;
		if (selectedTopic.empty())
			return Common::kNoError;
		if (runtime.matchesResponseLine(selectedTopic, 0x283)) {
			return runtime.playDialogueLineWithVariant(0x2ff4, "HERRILL", 0);
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x284)) {
			const DialogueLineEntry lines[] = {
				{ 0x2f7b, "PC", 2 },
				{ 0x2f7f, "HERRILL", 0 },
				{ 0x2f80, "HERRILL", 0 },
				{ 0x2f81, "HERRILL", 0 }
			};
			Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			topics.assign(0x285);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x286)) {
			const DialogueLineEntry lines[] = {
				{ 0x2f8f, "HERRILL", 0 },
				{ 0x2f90, "HERRILL", 0 },
				{ 0x2f91, "HERRILL", 1 }
			};
			Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			topics.assign(0x287);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x288)) {
			const DialogueLineEntry lines[] = {
				{ 0x2f9e, "HERRILL", 1 },
				{ 0x2fa3, "PC", 2 },
				{ 0x2fa7, "HERRILL", 1 }
			};
			Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			topics.assign(0x289);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x28a)) {
			const DialogueLineEntry lines[] = {
				{ 0x2fb2, "PC", 0 },
				{ 0x2fb6, "HERRILL", 0 },
				{ 0x2fbe, "HERRILL", 1 },
				{ 0x2fc2, "HERRILL", 1 }
			};
			Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			topics.assign(0x28b);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x28c)) {
			const DialogueLineEntry lines[] = {
				{ 0x2fcd, "PC", 0 },
				{ 0x2fd2, "HERRILL", 0 }
			};
			Common::Error lineError = runtime.playDialogueEntrySequence(lines, ARRAYSIZE(lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			topics.assign(0x28d);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x28e) ||
				runtime.matchesResponseLine(selectedTopic, 0x28f)) {
			sharedState.discussedLodgeTopic = true;
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x2fdd, "HERRILL", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			topics.assign(0x290);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x291)) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x2fe9, "HERRILL", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			topics.assign(0x292);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x293))
			return runtime.playDialogueLineWithVariant(0x2ff4, "HERRILL", 0);

		Common::Error lineError = runtime.playDialogueLineWithVariant(0x3099, "HERRILL", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
}

} // End of namespace Harvester
