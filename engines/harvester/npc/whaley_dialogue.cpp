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

bool WhaleyDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("WHALEY");
}

Common::Error WhaleyDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	WhaleyRoomDialogueState &state = _state;
	Common::String whaleyTopicBuffer;
	int whaleyTopicBufferLineIndex = -1;

	auto assignWhaleyTopicBuffer = [&](int responseLineIndex) {
		runtime.assignTopicBuffer(whaleyTopicBuffer, whaleyTopicBufferLineIndex,
			responseLineIndex, "Whaley topic buffer");
	};
	auto clearWhaleyTopicBuffer = [&]() {
		whaleyTopicBuffer.clear();
		whaleyTopicBufferLineIndex = -1;
	};
	auto playWhaleyLine = [&](int wavId, int headVariant) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, "WHALEY", headVariant);
	};
	auto playHerrillLine = [&](int wavId, int headVariant) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, "HERRILL", headVariant);
	};
	auto playPcLine = [&](int wavId, int headVariant) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, "PC", headVariant);
	};
	auto playSequence = [&](const DialogueLineEntry *lines, uint count) -> Common::Error {
		return runtime.playDialogueEntrySequence(lines, count);
	};
	auto executeActionTagIfSet = [&](const char *actionTag) {
		InteractionResult interaction;
		if (runtime.executeActionTag(actionTag, interaction)) {
			runtime.applyImmediateDialogueInteractionEffects(interaction);
			runtime.queueDialogueInteractionIfNeeded(interaction);
		}
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
	auto tradePhotoForBat = [&]() {
		(void)runtime.startupScript().addRuntimeObjectToInventory(kBaseballBatObjectName);
		(void)runtime.startupScript().setRuntimeObjectVisible(
			kInventoryOwnerName, kWhaleyPhotoObjectName, false);
	};

	if (runtime.startupScript().getFlagValue("BUSTED_SCREWING_MIDGAME")) {
		Common::Error lineError = playWhaleyLine(0x143b, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x24b, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playWhaleyLine(0x1445, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			lineError = playHerrillLine(0x1449, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playPcLine(0x144d, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			if (!hasInventoryItem(kWhaleyPhotoObjectName)) {
				lineError = playSequence(kWhaleyBustedScrewingResponse2WithoutPhotoLines,
					ARRAYSIZE(kWhaleyBustedScrewingResponse2WithoutPhotoLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else {
				lineError = playHerrillLine(0x1465, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playWhaleyLine(0x1469, 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				responseIndex = 0;
				responseError = runtime.runResponseMenu(0x24c, responseIndex);
				if (responseError.getCode() != Common::kNoError)
					return responseError;

				if (responseIndex == 1) {
					lineError = playWhaleyLine(0x1476, 2);
					if (lineError.getCode() != Common::kNoError)
						return lineError;

					int nestedResponseIndex = 0;
					responseError = runtime.runResponseMenu(0x24d, nestedResponseIndex);
					if (responseError.getCode() != Common::kNoError)
						return responseError;

					if (nestedResponseIndex == 1) {
						lineError = playWhaleyLine(0x1482, 0);
						if (lineError.getCode() != Common::kNoError)
							return lineError;
						tradePhotoForBat();
					} else if (nestedResponseIndex == 2) {
						lineError = playWhaleyLine(0x1487, 2);
						if (lineError.getCode() != Common::kNoError)
							return lineError;
						lineError = playHerrillLine(0x148c, 2);
						if (lineError.getCode() != Common::kNoError)
							return lineError;
					}
				} else if (responseIndex == 2) {
					lineError = playSequence(kWhaleyBustedScrewingResponse2Menu2Lines,
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
			executeActionTagIfSet(kReverseBroomActionTag);
		}
		return Common::kNoError;
	}

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase(kWhaleyPhotoObjectName))
			return playWhaleyLine(0x1411, 2);

		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			sharedState.discussedCasketPhotoEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
			Common::Error lineError = playSequence(kWhaleyCasketPhotoLines,
				ARRAYSIZE(kWhaleyCasketPhotoLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (runtime.startupScript().getFlagValue("IF_SAW_THE_SLASHES_ON_THE_BODY")) {
				lineError = playWhaleyLine(0x1428, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = runtime.playDialogueFst(kDialogueC055FstPath);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playWhaleyLine(0x1430, 1);
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
			return playSequence(kWhaleyLedgerLines, ARRAYSIZE(kWhaleyLedgerLines));
		}

		if (usedItemName.equalsIgnoreCase("NOTE") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
			sharedState.discussedNoteCheckbookEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
			return playSequence(kWhaleyBlackmailLines, ARRAYSIZE(kWhaleyBlackmailLines));
		}

		return playWhaleyLine(0x140b, 2);
	}

	if (state.introPending) {
		state.introPending = false;
		Common::Error lineError = playSequence(kWhaleyIntroLines, ARRAYSIZE(kWhaleyIntroLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x24e, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			assignWhaleyTopicBuffer(0x24f);
			lineError = playWhaleyLine(0x1316, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			clearWhaleyTopicBuffer();
			lineError = playWhaleyLine(0x131a, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			return Common::kNoError;
		}
	} else {
		Common::Error lineError = playSequence(kWhaleyRevisitLines, ARRAYSIZE(kWhaleyRevisitLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		assignWhaleyTopicBuffer(0x250);
	}

	if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") &&
			!state.stephMidgameShown) {
		state.stephMidgameShown = true;

		Common::Error lineError = playSequence(kWhaleyStephMidgameOpeningLines,
			ARRAYSIZE(kWhaleyStephMidgameOpeningLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x251, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playSequence(kWhaleyStephMidgameResponse1Lines,
				ARRAYSIZE(kWhaleyStephMidgameResponse1Lines));
		} else if (responseIndex == 2) {
			lineError = playWhaleyLine(0x13b5, 1);
		} else {
			lineError = Common::kNoError;
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getCurrentStoryDayIndex() == 5 && !state.day5Shown) {
		state.day5Shown = true;

		Common::Error lineError = playWhaleyLine(0x13bf, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x252, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playWhaleyLine(0x13ca, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC057FstPath);
		} else if (responseIndex == 2) {
			lineError = playSequence(kWhaleyDay5Response2Lines,
				ARRAYSIZE(kWhaleyDay5Response2Lines));
		} else {
			lineError = Common::kNoError;
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("KARIN_KIDNAPED") &&
			!state.karinKidnapedShown) {
		state.karinKidnapedShown = true;
		Common::Error lineError = playWhaleyLine(0x13e5, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		sharedState.karinKidnapedDialogueState = true;
		lineError = playWhaleyLine(0x13ee, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playPcLine(0x13f3, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playWhaleyLine(0x13f8, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playWhaleyLine(0x13ff, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (sharedState.discussedWhaleyHerrillPhoto != 0 &&
			!state.whaleyHerrillPhotoFollowupShown) {
		state.whaleyHerrillPhotoFollowupShown = true;
		Common::Error lineError = playWhaleyLine(0x14dd, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("BURNED_TV_STATION") &&
			!state.burnedTvStationShown) {
		state.burnedTvStationShown = true;

		Common::Error lineError = playSequence(kWhaleyBurnedTvStationLines,
			ARRAYSIZE(kWhaleyBurnedTvStationLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueFst(kDialogueC055FstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playWhaleyLine(0x1512, 2);
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
			return playWhaleyLine(0x1373, 0);

		if (runtime.matchesResponseLine(selectedTopic, 0x255)) {
			Common::Error lineError = playSequence(kWhaleyTopic255Lines,
				ARRAYSIZE(kWhaleyTopic255Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignWhaleyTopicBuffer(0x256);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x257)) {
			sharedState.discussedLodgeTopic = true;
			Common::Error lineError = playSequence(kWhaleyLodgeTopicLines,
				ARRAYSIZE(kWhaleyLodgeTopicLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignWhaleyTopicBuffer(0x258);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x259)) {
			Common::Error lineError = playWhaleyLine(0x134e, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC053FstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			executeActionTagIfSet(kSoundAirRaidActionTag);
			lineError = playWhaleyLine(0x135f, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC052FstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			sharedState.dwayneWhaleyDisciplineFollowupState = true;
			lineError = playWhaleyLine(0x136c, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignWhaleyTopicBuffer(0x25b);
			continue;
		}

		if (runtime.matchesResponseLine(selectedTopic, 0x25c))
			continue;

		Common::Error lineError = playWhaleyLine(0x1405, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
}

} // End of namespace Harvester
