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


#include "harvester/npc/whaley_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kDialogueC052FstPath = "GRAPHIC/FST/C052.FST";
static const char *const kDialogueC053FstPath = "GRAPHIC/FST/C053.FST";
static const char *const kDialogueC055FstPath = "GRAPHIC/FST/C055.FST";
static const char *const kDialogueC057FstPath = "GRAPHIC/FST/C057.FST";
static const char *const kBaseballBatObjectName = "BAT";
static const char *const kInventoryOwnerName = "INVENTORY";
static const char *const kReverseBroomActionTag = "REVERSE_BROOM";
static const char *const kSoundAirRaidActionTag = "SOUND_AIR_RAID";
static const char *const kWhaleyPhotoObjectName = "PHOTO_OF_WHALEY_HERRILL";

static const DialogueLineEntry kWhaleyBustedScrewingResponse2WithoutPhotoLines[] = {
	{ 0x1456, "HERRILL", 2 },
	{ 0x145a, "HERRILL", 2 },
	{ 0x145f, "WHALEY", 2 }
};

static const DialogueLineEntry kWhaleyBustedScrewingResponse2Menu2Lines[] = {
	{ 0x1491, "HERRILL", 2 },
	{ 0x1495, "WHALEY", 3 },
	{ 0x1499, "HERRILL", 0 },
	{ 0x149e, "WHALEY", 1 }
};

static const DialogueLineEntry kWhaleyCasketPhotoLines[] = {
	{ 0x1417, "WHALEY", 0 },
	{ 0x141b, "PC", 0 },
	{ 0x141f, "WHALEY", 2 },
	{ 0x1424, "PC", 0 }
};

static const DialogueLineEntry kWhaleyLedgerLines[] = {
	{ 0x14a4, "WHALEY", 2 },
	{ 0x14a9, "PC", 0 },
	{ 0x14ae, "WHALEY", 1 },
	{ 0x14b7, "WHALEY", 2 },
	{ 0x14bc, "PC", 2 },
	{ 0x14c0, "WHALEY", 0 }
};

static const DialogueLineEntry kWhaleyBlackmailLines[] = {
	{ 0x14c7, "WHALEY", 0 },
	{ 0x14cb, "PC", 0 },
	{ 0x14cf, "WHALEY", 0 },
	{ 0x14d7, "WHALEY", 0 }
};

static const DialogueLineEntry kWhaleyIntroLines[] = {
	{ 0x12b6, "WHALEY", 1 },
	{ 0x12c0, "WHALEY", 3 },
	{ 0x12c5, "PC", 4 },
	{ 0x12c9, "WHALEY", 0 },
	{ 0x12cf, "PC", 0 },
	{ 0x12d3, "WHALEY", 0 },
	{ 0x12de, "WHALEY", 1 },
	{ 0x12e3, "PC", 0 },
	{ 0x12e7, "WHALEY", 0 },
	{ 0x130b, "WHALEY", 1 }
};

static const DialogueLineEntry kWhaleyRevisitLines[] = {
	{ 0x1385, "WHALEY", 0 },
	{ 0x138e, "WHALEY", 0 }
};

static const DialogueLineEntry kWhaleyStephMidgameOpeningLines[] = {
	{ 0x1394, "WHALEY", 1 },
	{ 0x139d, "WHALEY", 1 }
};

static const DialogueLineEntry kWhaleyStephMidgameResponse1Lines[] = {
	{ 0x13a8, "WHALEY", 0 },
	{ 0x13ac, "PC", 0 },
	{ 0x13b0, "WHALEY", 2 }
};

static const DialogueLineEntry kWhaleyDay5Response2Lines[] = {
	{ 0x13d7, "WHALEY", 2 },
	{ 0x13df, "WHALEY", 2 }
};

static const DialogueLineEntry kWhaleyBurnedTvStationLines[] = {
	{ 0x14f7, "WHALEY", 1 },
	{ 0x14fe, "WHALEY", 1 },
	{ 0x1506, "WHALEY", 2 },
	{ 0x150b, "WHALEY", 2 }
};

static const DialogueLineEntry kWhaleyTopic255Lines[] = {
	{ 0x132a, "PC", 0 },
	{ 0x132e, "WHALEY", 0 },
	{ 0x1333, "PC", 0 },
	{ 0x1337, "WHALEY", 0 }
};

static const DialogueLineEntry kWhaleyLodgeTopicLines[] = {
	{ 0x1340, "PC", 0 },
	{ 0x1344, "WHALEY", 0 }
};

} // End of namespace

Common::Error WhaleyDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	WhaleyRoomDialogueState &state = _state;
	Common::String whaleyTopicBuffer;
	int whaleyTopicBufferLineIndex = -1;

	DialogueTopicBuffer topics(runtime, whaleyTopicBuffer, whaleyTopicBufferLineIndex, "Whaley topic buffer");

	auto hasInventoryItem = [&](const char *objectName) {
		Common::Array<ObjectRecord> inventoryObjects;
		runtime.startupScript().getVisibleInventoryObjects(inventoryObjects);
		for (const ObjectRecord &inventoryObject : inventoryObjects) {
			if (inventoryObject.objectName.equalsIgnoreCase(objectName))
				return true;
		}
		return false;
	};
	auto tradePhotoForBat = [&]() {
		(void)runtime.startupScript().addRuntimeObjectToInventory(kBaseballBatObjectName);
		(void)runtime.startupScript().setRuntimeObjectVisible(
			kInventoryOwnerName, kWhaleyPhotoObjectName, false);
	};

	if (runtime.startupScript().getFlagValue("BUSTED_SCREWING_MIDGAME")) {
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x143b, "WHALEY", 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x24b, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = runtime.playDialogueLineWithVariant(0x1445, "WHALEY", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			lineError = runtime.playDialogueLineWithVariant(0x1449, "HERRILL", 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x144d, "PC", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			if (!hasInventoryItem(kWhaleyPhotoObjectName)) {
				lineError = runtime.playDialogueEntrySequence(kWhaleyBustedScrewingResponse2WithoutPhotoLines,
					ARRAYSIZE(kWhaleyBustedScrewingResponse2WithoutPhotoLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else {
				lineError = runtime.playDialogueLineWithVariant(0x1465, "HERRILL", 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueLineWithVariant(0x1469, "WHALEY", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				responseIndex = 0;
				responseError = runtime.runResponseMenu(0x24c, responseIndex);
				if (responseError.getCode() != Common::kNoError)
					return responseError;

				if (responseIndex == 1) {
					lineError = runtime.playDialogueLineWithVariant(0x1476, "WHALEY", 2);
					if (lineError.getCode() != Common::kNoError)
						return lineError;

					int nestedResponseIndex = 0;
					responseError = runtime.runResponseMenu(0x24d, nestedResponseIndex);
					if (responseError.getCode() != Common::kNoError)
						return responseError;

					if (nestedResponseIndex == 1) {
						lineError = runtime.playDialogueLineWithVariant(0x1482, "WHALEY", 0);
						if (lineError.getCode() != Common::kNoError)
							return lineError;
						tradePhotoForBat();
					} else if (nestedResponseIndex == 2) {
						lineError = runtime.playDialogueLineWithVariant(0x1487, "WHALEY", 2);
						if (lineError.getCode() != Common::kNoError)
							return lineError;
						lineError = runtime.playDialogueLineWithVariant(0x148c, "HERRILL", 2);
						if (lineError.getCode() != Common::kNoError)
							return lineError;
					}
				} else if (responseIndex == 2) {
					lineError = runtime.playDialogueEntrySequence(kWhaleyBustedScrewingResponse2Menu2Lines,
						ARRAYSIZE(kWhaleyBustedScrewingResponse2Menu2Lines));
					if (lineError.getCode() != Common::kNoError)
						return lineError;
					tradePhotoForBat();
				}
			}
		}

		(void)runtime.startupScript().setRuntimeFlagValue("BUSTED_SCREWING_MIDGAME", false);
		if (runtime.startupScript().getFlagValue("TALKING_BROOM_WHALEY")) {
			(void)runtime.startupScript().setRuntimeFlagValue("TALKING_BROOM_WHALEY", false);
			runtime.executeAndQueueAction(kReverseBroomActionTag);
		}
		return Common::kNoError;
	}

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase(kWhaleyPhotoObjectName))
			return runtime.playDialogueLineWithVariant(0x1411, "WHALEY", 2);

		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			sharedState.discussedCasketPhotoEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
			Common::Error lineError = runtime.playDialogueEntrySequence(kWhaleyCasketPhotoLines,
				ARRAYSIZE(kWhaleyCasketPhotoLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (runtime.startupScript().getFlagValue("IF_SAW_THE_SLASHES_ON_THE_BODY")) {
				lineError = runtime.playDialogueLineWithVariant(0x1428, "WHALEY", 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueFst(kDialogueC055FstPath);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueLineWithVariant(0x1430, "WHALEY", 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			return Common::kNoError;
		}

		if ((usedItemName.equalsIgnoreCase("LEDGER") ||
				usedItemName.equalsIgnoreCase("LEDGER2")) &&
				runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
			sharedState.discussedLedgerEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownLedgersToAnyone, true);
			return runtime.playDialogueEntrySequence(kWhaleyLedgerLines, ARRAYSIZE(kWhaleyLedgerLines));
		}

		if (usedItemName.equalsIgnoreCase("NOTE") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
			sharedState.discussedNoteCheckbookEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
			return runtime.playDialogueEntrySequence(kWhaleyBlackmailLines, ARRAYSIZE(kWhaleyBlackmailLines));
		}

		return runtime.playDialogueLineWithVariant(0x140b, "WHALEY", 2);
	}

	if (state.introPending) {
		state.introPending = false;
		Common::Error lineError = runtime.playDialogueEntrySequence(kWhaleyIntroLines, ARRAYSIZE(kWhaleyIntroLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x24e, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			topics.assign(0x24f);
			lineError = runtime.playDialogueLineWithVariant(0x1316, "WHALEY", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			topics.clear();
			lineError = runtime.playDialogueLineWithVariant(0x131a, "WHALEY", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			return Common::kNoError;
		}
	} else {
		Common::Error lineError = runtime.playDialogueEntrySequence(kWhaleyRevisitLines, ARRAYSIZE(kWhaleyRevisitLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		topics.assign(0x250);
	}

	if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") &&
			!state.stephMidgameShown) {
		state.stephMidgameShown = true;

		Common::Error lineError = runtime.playDialogueEntrySequence(kWhaleyStephMidgameOpeningLines,
			ARRAYSIZE(kWhaleyStephMidgameOpeningLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x251, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = runtime.playDialogueEntrySequence(kWhaleyStephMidgameResponse1Lines,
				ARRAYSIZE(kWhaleyStephMidgameResponse1Lines));
		} else if (responseIndex == 2) {
			lineError = runtime.playDialogueLineWithVariant(0x13b5, "WHALEY", 1);
		} else {
			lineError = Common::kNoError;
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getCurrentStoryDayIndex() == 5 && !state.day5Shown) {
		state.day5Shown = true;

		Common::Error lineError = runtime.playDialogueLineWithVariant(0x13bf, "WHALEY", 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x252, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = runtime.playDialogueLineWithVariant(0x13ca, "WHALEY", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC057FstPath);
		} else if (responseIndex == 2) {
			lineError = runtime.playDialogueEntrySequence(kWhaleyDay5Response2Lines, ARRAYSIZE(kWhaleyDay5Response2Lines));
		} else {
			lineError = Common::kNoError;
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("KARIN_KIDNAPED") &&
			!state.karinKidnapedShown) {
		state.karinKidnapedShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x13e5, "WHALEY", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		sharedState.karinKidnapedDialogueState = true;
		lineError = runtime.playDialogueLineWithVariant(0x13ee, "WHALEY", 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueLineWithVariant(0x13f3, "PC", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueLineWithVariant(0x13f8, "WHALEY", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueLineWithVariant(0x13ff, "WHALEY", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (sharedState.discussedWhaleyHerrillPhoto != 0 &&
			!state.whaleyHerrillPhotoFollowupShown) {
		state.whaleyHerrillPhotoFollowupShown = true;
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x14dd, "WHALEY", 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("BURNED_TV_STATION") &&
			!state.burnedTvStationShown) {
		state.burnedTvStationShown = true;

		Common::Error lineError = runtime.playDialogueEntrySequence(kWhaleyBurnedTvStationLines,
			ARRAYSIZE(kWhaleyBurnedTvStationLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueFst(kDialogueC055FstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueLineWithVariant(0x1512, "WHALEY", 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	for (;;) {
		Common::String selectedTopic;
		Common::Error menuError = runtime.runKeywordMenu(
			whaleyTopicBuffer, whaleyTopicBufferLineIndex, selectedTopic);
		if (menuError.getCode() != Common::kNoError)
			return menuError;
		if (selectedTopic.empty())
			return Common::kNoError;

		if (runtime.matchesResponseLine(selectedTopic, 0x254))
			return runtime.playDialogueLineWithVariant(0x1373, "WHALEY", 0);

		if (runtime.matchesResponseLine(selectedTopic, 0x255)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(kWhaleyTopic255Lines, ARRAYSIZE(kWhaleyTopic255Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			topics.assign(0x256);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x257)) {
			sharedState.discussedLodgeTopic = true;
			Common::Error lineError = runtime.playDialogueEntrySequence(kWhaleyLodgeTopicLines,
				ARRAYSIZE(kWhaleyLodgeTopicLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			topics.assign(0x258);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x259)) {
			Common::Error lineError = runtime.playDialogueLineWithVariant(0x134e, "WHALEY", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC053FstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			runtime.executeAndQueueAction(kSoundAirRaidActionTag);
			lineError = runtime.playDialogueLineWithVariant(0x135f, "WHALEY", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC052FstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			sharedState.dwayneWhaleyDisciplineFollowupState = true;
			lineError = runtime.playDialogueLineWithVariant(0x136c, "WHALEY", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			topics.assign(0x25b);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x25c))
			continue;

		Common::Error lineError = runtime.playDialogueLineWithVariant(0x1405, "WHALEY", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
}

} // End of namespace Harvester
