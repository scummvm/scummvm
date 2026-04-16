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


#include "harvester/npc/mom_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"
#include "harvester/script.h"

namespace Harvester {

namespace {

static const char *const kDialogueC008AFstPath = "GRAPHIC/FST/C008A.FST";
static const char *const kDialogueC008BFstPath = "GRAPHIC/FST/C008B.FST";
static const char *const kDialogueC009FstPath = "GRAPHIC/FST/C009.FST";
static const char *const kDialogueC010FstPath = "GRAPHIC/FST/C010.FST";
static const char *const kBabyGurgleActionTag = "BABY_GURGLE";
static const char *const kBabyScreamActionTag = "BABY_SCREAM";
static const char *const kSawSlashesOnBodyFlag = "IF_SAW_THE_SLASHES_ON_THE_BODY";

static const DialogueLineEntry kMomLedgerLines[] = {
	{ 0x2320, "MOM", 2 },
	{ 0x2326, "PC", 2 },
	{ 0x232b, "MOM", 1 },
	{ 0x2331, "PC", 0 },
	{ 0x2335, "MOM", 1 }
};

static const DialogueLineEntry kMomCasketPhotoLines[] = {
	{ 0x233b, "PC", 0 }
};

static const DialogueLineEntry kMomWhaleyPhotoLines[] = {
	{ 0x239c, "MOM", 2 }
};

static const DialogueLineEntry kMomIntroLines[] = {
	{ 0x1dd7, "MOM", 1 },
	{ 0x1ddc, "PC", 0 },
	{ 0x1de0, "MOM", 2 },
	{ 0x1de4, "PC", 2 },
	{ 0x1de8, "MOM", 0 },
	{ 0x1ded, "PC", 0 },
	{ 0x1df1, "MOM", 1 },
	{ 0x1df5, "PC", 2 },
	{ 0x1dfa, "MOM", 0 },
	{ 0x1e04, "MOM", 0 },
	{ 0x1e5d, "PC", 0 },
	{ 0x1e61, "MOM", 2 },
	{ 0x1e66, "PC", 0 },
	{ 0x1e6b, "MOM", 3 },
	{ 0x1e6f, "PC", 2 },
	{ 0x1e73, "MOM", 0 }
};

static const DialogueLineEntry kMomStephMidgameOpeningLines[] = {
	{ 0x205a, "MOM", 3 },
	{ 0x205e, "MOM", 1 }
};

static const DialogueLineEntry kMomStephMidgameClosingLines[] = {
	{ 0x2063, "PC", 3 },
	{ 0x2068, "MOM", 1 },
	{ 0x206d, "PC", 2 },
	{ 0x2071, "MOM", 0 }
};

static const DialogueLineEntry kMomBurnedTvStationOpeningLines[] = {
	{ 0x2220, "MOM", 2 },
	{ 0x2225, "PC", 0 },
	{ 0x222a, "MOM", 2 }
};

static const DialogueLineEntry kMomBurnedTvStationPreActionLines[] = {
	{ 0x223e, "MOM", 3 }
};

static const DialogueLineEntry kMomBurnedTvStationPreFstLines[] = {
	{ 0x224e, "MOM", 0 }
};

static const DialogueLineEntry kMomBurnedTvStationPostFstLines[] = {
	{ 0x2272, "MOM", 1 },
	{ 0x2257, "MOM", 2 },
	{ 0x2276, "PC", 4 },
	{ 0x227b, "MOM", 1 },
	{ 0x2282, "PC", 3 },
	{ 0x2286, "MOM", 0 },
	{ 0x228a, "MOM", 2 },
	{ 0x2293, "MOM", 0 }
};

static const DialogueLineEntry kMomBarberPoleStolenLines[] = {
	{ 0x23fb, "MOM", 1 },
	{ 0x2401, "MOM", 1 },
	{ 0x240d, "MOM", 2 }
};

static const DialogueLineEntry kMomBoltOfClothClearLines[] = {
	{ 0x2416, "MOM", 1 }
};

static const DialogueLineEntry kMomBoltOfClothDefaultLines[] = {
	{ 0x2420, "MOM", 1 },
	{ 0x2428, "PC", 0 },
	{ 0x242c, "MOM", 0 }
};

static const DialogueLineEntry kMomEscapedJailLines[] = {
	{ 0x2495, "MOM", 0 },
	{ 0x249b, "PC", 0 },
	{ 0x249f, "MOM", 0 }
};

static const DialogueLineEntry kMomKarinKidnappedResponse2Lines[] = {
	{ 0x24e8, "MOM", 0 },
	{ 0x24ed, "MOM", 2 },
	{ 0x24f3, "PC", 0 },
	{ 0x24f7, "MOM", 2 },
	{ 0x24fd, "PC", 0 },
	{ 0x2501, "MOM", 1 }
};

static const DialogueLineEntry kMomKarinFoundAliveIntroLines[] = {
	{ 0x2505, "MOM", 0 },
	{ 0x2509, "PC", 0 },
	{ 0x250d, "MOM", 2 },
	{ 0x2511, "PC", 2 },
	{ 0x2515, "MOM", 2 },
	{ 0x2519, "PC", 2 },
	{ 0x251d, "MOM", 2 }
};

static const DialogueLineEntry kMomKarinFoundAliveResponse1WithoutStephMidgameLines[] = {
	{ 0x252a, "MOM", 2 },
	{ 0x252e, "MOM", 2 },
	{ 0x2534, "PC", 0 },
	{ 0x2538, "MOM", 2 }
};

static const DialogueLineEntry kMomKarinFoundAliveResponse1WithStephMidgameLines[] = {
	{ 0x252a, "MOM", 2 },
	{ 0x2543, "MOM", 2 },
	{ 0x2549, "PC", 0 },
	{ 0x254d, "MOM", 2 },
	{ 0x2552, "PC", 2 },
	{ 0x2557, "MOM", 2 }
};

static const DialogueLineEntry kMomMoynahanAbsentLines[] = {
	{ 0x2647, "MOM", 2 },
	{ 0x264b, "PC", 0 },
	{ 0x264f, "MOM", 3 }
};

static const DialogueLineEntry kMomJimmyAttackedLines[] = {
	{ 0x2659, "MOM", 2 },
	{ 0x265f, "PC", 2 },
	{ 0x2663, "MOM", 0 }
};

static const DialogueLineEntry kMomStephanieDeadPreMidgameLines[] = {
	{ 0x217c, "MOM", 3 },
	{ 0x2180, "PC", 0 },
	{ 0x2184, "MOM", 2 }
};

static const DialogueLineEntry kMomDay5Lines[] = {
	{ 0x218d, "MOM", 1 },
	{ 0x2192, "PC", 0 },
	{ 0x2197, "MOM", 1 }
};

static const DialogueLineEntry kMomDay6Lines[] = {
	{ 0x22a8, "MOM", 3 },
	{ 0x22b1, "MOM", 3 }
};

static const DialogueLineEntry kMomCookiesLines[] = {
	{ 0x1e18, "PC", 0 },
	{ 0x1e1d, "MOM", 1 },
	{ 0x1e21, "PC", 0 },
	{ 0x1e25, "MOM", 0 },
	{ 0x1e29, "PC", 0 },
	{ 0x1e2d, "MOM", 0 },
	{ 0x1e31, "PC", 0 },
	{ 0x1e35, "MOM", 1 },
	{ 0x1e3a, "PC", 0 },
	{ 0x1e3e, "MOM", 0 }
};

static const DialogueLineEntry kMomTopic119Lines[] = {
	{ 0x1e47, "PC", 0 },
	{ 0x1e4b, "MOM", 0 },
	{ 0x1e4f, "PC", 0 },
	{ 0x1e53, "MOM", 0 },
	{ 0x1f99, "MOM", 2 }
};

static const DialogueLineEntry kMomTopic11dOpeningLines[] = {
	{ 0x1e7c, "PC", 4 },
	{ 0x1e80, "MOM", 2 }
};

static const DialogueLineEntry kMomTopic11dPostC008ALines[] = {
	{ 0x1e8e, "MOM", 2 },
	{ 0x1e94, "PC", 2 }
};

static const DialogueLineEntry kMomTopic11dClosingLines[] = {
	{ 0x1e9d, "MOM", 3 },
	{ 0x1ea3, "MOM", 3 }
};

static const DialogueLineEntry kMomTopic121Lines[] = {
	{ 0x1ec0, "PC", 0 },
	{ 0x1ec4, "MOM", 2 },
	{ 0x1ec9, "PC", 0 },
	{ 0x1ecd, "MOM", 2 }
};

static const DialogueLineEntry kMomSparkyLines[] = {
	{ 0x1ed7, "PC", 0 },
	{ 0x1edb, "MOM", 2 },
	{ 0x1ee1, "PC", 0 },
	{ 0x1ee5, "MOM", 2 }
};

static const DialogueLineEntry kMomPtaLines[] = {
	{ 0x1eee, "PC", 2 },
	{ 0x1ef2, "MOM", 0 }
};

static const DialogueLineEntry kMomTopic12bLines[] = {
	{ 0x1efd, "PC", 0 },
	{ 0x1f01, "MOM", 0 },
	{ 0x1f07, "PC", 0 },
	{ 0x1f0b, "MOM", 0 },
	{ 0x22cd, "PC", 2 },
	{ 0x22d1, "MOM", 0 }
};

static const DialogueLineEntry kMomTopic12dLines[] = {
	{ 0x1f14, "PC", 0 },
	{ 0x1f18, "MOM", 0 },
	{ 0x1f1d, "PC", 0 },
	{ 0x1f21, "MOM", 0 },
	{ 0x1f28, "PC", 0 },
	{ 0x1f2c, "MOM", 2 },
	{ 0x1f31, "PC", 2 },
	{ 0x1f35, "MOM", 2 }
};

static const DialogueLineEntry kMomTopic12fLines[] = {
	{ 0x1f41, "PC", 4 },
	{ 0x1f45, "MOM", 3 },
	{ 0x1f4b, "PC", 0 },
	{ 0x1f4f, "MOM", 2 }
};

static const DialogueLineEntry kMomCookingLines[] = {
	{ 0x1f58, "PC", 0 },
	{ 0x1f5c, "MOM", 0 },
	{ 0x1f63, "PC", 0 },
	{ 0x1f68, "MOM", 0 }
};

static const DialogueLineEntry kMomTopic135Lines[] = {
	{ 0x1f71, "PC", 0 },
	{ 0x1f75, "MOM", 2 }
};

static const DialogueLineEntry kMomTopic137Lines[] = {
	{ 0x1f7f, "PC", 2 },
	{ 0x1f84, "MOM", 3 },
	{ 0x1f8a, "PC", 2 },
	{ 0x1f8e, "MOM", 2 }
};

static const DialogueLineEntry kMomSlaughterhouseLines[] = {
	{ 0x1faf, "PC", 4 },
	{ 0x1fb4, "MOM", 0 },
	{ 0x1fb9, "PC", 2 },
	{ 0x1fbd, "MOM", 0 }
};

static const DialogueLineEntry kMomFatherLines[] = {
	{ 0x1fc8, "PC", 0 },
	{ 0x1fcc, "MOM", 0 }
};

static const DialogueLineEntry kMomFatherHeardDadMoanLines[] = {
	{ 0x1fd3, "PC", 0 },
	{ 0x1fd8, "MOM", 0 }
};

static const DialogueLineEntry kMomLodgeLines[] = {
	{ 0x1fe1, "PC", 0 },
	{ 0x1fe5, "MOM", 1 },
	{ 0x1feb, "PC", 0 },
	{ 0x1fef, "MOM", 1 }
};

static const DialogueLineEntry kMomStephMidgameLodgeLines[] = {
	{ 0x2154, "PC", 0 },
	{ 0x215a, "MOM", 0 }
};

static const DialogueLineEntry kMomBakeSaleLines[] = {
	{ 0x2017, "PC", 0 },
	{ 0x201b, "MOM", 1 },
	{ 0x2020, "PC", 0 },
	{ 0x2024, "MOM", 0 },
	{ 0x2029, "PC", 0 },
	{ 0x202d, "MOM", 0 },
	{ 0x2164, "PC", 0 },
	{ 0x2168, "MOM", 1 },
	{ 0x216d, "PC", 0 },
	{ 0x2171, "MOM", 0 }
};

static const DialogueLineEntry kMomTopic14aLines[] = {
	{ 0x2036, "MOM", 0 },
	{ 0x203a, "PC", 0 },
	{ 0x203e, "MOM", 2 }
};

static const DialogueLineEntry kMomPottsdamFirstResponse1Lines[] = {
	{ 0x2093, "MOM", 0 }
};

static const DialogueLineEntry kMomPottsdamFirstResponse2Lines[] = {
	{ 0x2097, "MOM", 0 }
};

static const DialogueLineEntry kMomPottsdamCommonFollowupLines[] = {
	{ 0x209c, "MOM", 1 }
};

static const DialogueLineEntry kMomPottsdamSecondResponse1Lines[] = {
	{ 0x20a6, "MOM", 1 },
	{ 0x20ab, "PC", 3 },
	{ 0x20b0, "MOM", 2 }
};

static const DialogueLineEntry kMomPottsdamThirdResponse1Lines[] = {
	{ 0x20bb, "MOM", 2 },
	{ 0x20bf, "PC", 2 },
	{ 0x20c4, "MOM", 1 }
};

static const DialogueLineEntry kMomPottsdamThirdResponse2Lines[] = {
	{ 0x20ce, "MOM", 1 }
};

static const DialogueLineEntry kMomPottsdamSecondResponse2Lines[] = {
	{ 0x20d5, "MOM", 0 },
	{ 0x20da, "PC", 4 },
	{ 0x20df, "MOM", 0 },
	{ 0x20e3, "PC", 3 },
	{ 0x20e8, "MOM", 1 }
};

static const DialogueLineEntry kMomMeatPlantIntroLines[] = {
	{ 0x20f7, "PC", 0 },
	{ 0x20fb, "MOM", 0 }
};

static const DialogueLineEntry kMomMeatPlantResponse1Lines[] = {
	{ 0x2106, "MOM", 0 },
	{ 0x210a, "PC", 0 },
	{ 0x210e, "MOM", 0 }
};

static const DialogueLineEntry kMomMeatPlantResponse2Lines[] = {
	{ 0x2114, "MOM", 1 },
	{ 0x2119, "PC", 0 },
	{ 0x211d, "MOM", 1 }
};

static const DialogueLineEntry kMomTopic155Lines[] = {
	{ 0x2125, "MOM", 1 },
	{ 0x212a, "PC", 0 },
	{ 0x212e, "MOM", 1 },
	{ 0x2134, "PC", 0 },
	{ 0x2138, "MOM", 1 }
};

static const DialogueLineEntry kMomTopic156Lines[] = {
	{ 0x2142, "MOM", 0 },
	{ 0x2147, "PC", 0 },
	{ 0x214b, "MOM", 0 }
};

static const DialogueLineEntry kMomGoodCauseDay5CompleteLines[] = {
	{ 0x21a3, "PC", 0 },
	{ 0x21a8, "MOM", 1 },
	{ 0x21ad, "PC", 0 },
	{ 0x21b2, "MOM", 0 },
	{ 0x21b7, "PC", 0 },
	{ 0x21bc, "MOM", 1 }
};

static const DialogueLineEntry kMomGoodCauseDay5SlashBodyLines[] = {
	{ 0x21c4, "PC", 0 },
	{ 0x21ca, "MOM", 2 },
	{ 0x21cf, "PC", 0 },
	{ 0x21d3, "MOM", 2 }
};

static const DialogueLineEntry kMomGoodCauseDay5FirstTimeLines[] = {
	{ 0x21dd, "PC", 0 },
	{ 0x21e2, "MOM", 0 }
};

static const DialogueLineEntry kMomMoynahanTopicLines[] = {
	{ 0x220a, "PC", 0 },
	{ 0x220e, "MOM", 0 },
	{ 0x2212, "PC", 0 },
	{ 0x2216, "MOM", 0 }
};

static const DialogueLineEntry kMomTopic16dLines[] = {
	{ 0x2696, "MOM", 0 },
	{ 0x2697, "MOM", 0 }
};

static const int kMomPtaTopicResponseLines[] = { 0x125, 0x126, 0x127, 0x128 };
static const int kMomCookingTopicResponseLines[] = { 0x131, 0x132, 0x133 };
static const int kMomBakeSaleTopicResponseLines[] = { 0x145, 0x146, 0x147 };
static const int kMomPottsdamTopicResponseLines[] = { 0x14b, 0x14c };
static const int kMomMeatPlantTopicResponseLines[] = { 0x151, 0x152 };
static const int kMomMoynahanTopicResponseLines[] = { 0x160, 0x161 };
static const int kMomNewspaperFireTopicResponseLines[] = { 0x162, 0x163 };
static const int kMomInitialTopicBufferResponseLine = 0x102;
static const int kMomSameDayTopicBufferResponseLine = 0x104;
static const int kMomStephMidgameTopicBufferResponseLine = 0x105;
static const int kMomDay5TopicBufferResponseLine = 0x113;
static const int kMomDay6TopicBufferResponseLine = 0x114;
static const int kMomSlaughterhouseFollowupTopicBufferResponseLine = 0x13e;
static const int kMomFatherFollowupTopicBufferResponseLine = 0x140;
static const int kMomLodgeFollowupTopicBufferResponseLine = 0x144;
static const int kMomPtaFollowupTopicBufferResponseLine = 0x12a;
static const int kMomCookingFollowupTopicBufferResponseLine = 0x134;
static const int kMomBakeSaleFollowupTopicBufferResponseLine = 0x148;
static const int kMomPottsdamFollowupTopicBufferResponseLine = 0x150;
static const int kMomMeatPlantFollowupTopicBufferResponseLine = 0x154;
static const int kMomGoodCauseDay5FollowupTopicBufferResponseLine = 0x15c;

} // End of namespace

bool MomDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("MOM");
}

Common::Error MomDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	const int currentStoryDayIndex = runtime.startupScript().getCurrentStoryDayIndex();
	MomRoomDialogueState &state = _state;
	Common::String momTopicBuffer;
	int momTopicBufferLineIndex = -1;
	auto assignMomTopicBuffer = [&](int responseLineIndex) {
		runtime.assignTopicBuffer(momTopicBuffer, momTopicBufferLineIndex,
			responseLineIndex, "Mom topic buffer");
	};
	auto playMomLine = [&](int wavId, int headVariant) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, "MOM", headVariant);
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
	auto hasInventoryObject = [&](const char *objectName) -> bool {
		Common::Array<ObjectRecord> inventoryObjects;
		runtime.startupScript().getVisibleInventoryObjects(inventoryObjects);
		for (const ObjectRecord &inventoryObject : inventoryObjects) {
			if (inventoryObject.objectName.equalsIgnoreCase(objectName))
				return true;
		}
		return false;
	};
	auto runMomGoodbye = [&]() -> Common::Error {
		if (!runtime.startupScript().getFlagValue("DAY_FLAG"))
			return playMomLine(0x256e, 1);
		if (!runtime.startupScript().isNamedNpcDeathTypeClear("JIMMY"))
			return Common::kNoError;

		return playMomLine(0x2051, 1);
	};

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("NOTE") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
			sharedState.discussedNoteCheckbookEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
			return playMomLine(0x2317, 1);
		}
		if ((usedItemName.equalsIgnoreCase("LEDGER") ||
				usedItemName.equalsIgnoreCase("LEDGER2")) &&
				runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
			sharedState.discussedLedgerEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownLedgersToAnyone, true);
			return playSequence(kMomLedgerLines, ARRAYSIZE(kMomLedgerLines));
		}
		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			sharedState.discussedCasketPhotoEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
			return playSequence(kMomCasketPhotoLines, ARRAYSIZE(kMomCasketPhotoLines));
		}
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			sharedState.discussedWhaleyHerrillPhoto = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
			return playSequence(kMomWhaleyPhotoLines, ARRAYSIZE(kMomWhaleyPhotoLines));
		}
		if (usedItemName.equalsIgnoreCase("TV_DEED") ||
				usedItemName.equalsIgnoreCase("TV_DEED_PHOTOCOPY")) {
			sharedState.discussedTvDeedEvidence = 1;
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceSheriffOwns, true);
			return playMomLine(0x2382, 1);
		}

		return playMomLine(0x26cc, 2);
	}

	if (state.introPending) {
		state.introPending = false;
		state.sameDayIntroLineEnabled = true;
		state.postIntroDefaultLineEnabled = true;
		state.introDayIndex = currentStoryDayIndex;
		sharedState.dialogueStateD2ef4 = true;

		Common::Error lineError = playSequence(kMomIntroLines, ARRAYSIZE(kMomIntroLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		assignMomTopicBuffer(kMomInitialTopicBufferResponseLine);
	} else if (state.sameDayIntroLineEnabled && currentStoryDayIndex == state.introDayIndex) {
		Common::Error lineError = playMomLine(0x2047, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		assignMomTopicBuffer(kMomSameDayTopicBufferResponseLine);
	} else if (state.postIntroDefaultLineEnabled) {
		Common::Error lineError = playMomLine(0x2311, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") &&
			!state.stephMidgameShown) {
		state.stephMidgameShown = true;

		Common::Error lineError = playSequence(
			kMomStephMidgameOpeningLines, ARRAYSIZE(kMomStephMidgameOpeningLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueFst(kDialogueC009FstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playSequence(
			kMomStephMidgameClosingLines, ARRAYSIZE(kMomStephMidgameClosingLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		assignMomTopicBuffer(kMomStephMidgameTopicBufferResponseLine);
	}

	if (runtime.startupScript().getFlagValue("DINER_BURNED") &&
			(runtime.startupScript().getFlagValue("KARIN_KIDNAPED") ||
				runtime.startupScript().getFlagValue("KARIN_FOUND_DEAD")) &&
			!state.dinerBurnedKarinMissingOrDeadShown) {
		state.dinerBurnedKarinMissingOrDeadShown = true;

		Common::Error lineError = playMomLine(0x2456, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (sharedState.dialogueStateD2ea4) {
			lineError = playMomLine(0x245d, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x106, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			switch (responseIndex) {
			case 1:
				lineError = playMomLine(0x246b, 0);
				break;
			case 2:
				lineError = playMomLine(0x246f, 1);
				break;
			default:
				lineError = Common::kNoError;
				break;
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}

	if (runtime.startupScript().getFlagValue("BURNED_TV_STATION") &&
			!state.burnedTvStationShown) {
		state.burnedTvStationShown = true;

		Common::Error lineError = playSequence(
			kMomBurnedTvStationOpeningLines, ARRAYSIZE(kMomBurnedTvStationOpeningLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x107, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;
		if (responseIndex == 1) {
			lineError = playMomLine(0x2237, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		lineError = playSequence(
			kMomBurnedTvStationPreActionLines, ARRAYSIZE(kMomBurnedTvStationPreActionLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		executeActionTagIfSet(kBabyScreamActionTag);
		lineError = playSequence(
			kMomBurnedTvStationPreFstLines, ARRAYSIZE(kMomBurnedTvStationPreFstLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueFst(kDialogueC010FstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playSequence(
			kMomBurnedTvStationPostFstLines, ARRAYSIZE(kMomBurnedTvStationPostFstLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("SCRATCHED_TUCKER") &&
			!state.scratchedTuckerShown) {
		state.scratchedTuckerShown = true;
		Common::Error lineError = playMomLine(0x23f3, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("BARBER_POLE_STOLEN") &&
			!state.barberPoleStolenShown) {
		state.barberPoleStolenShown = true;
		Common::Error lineError = playSequence(
			kMomBarberPoleStolenLines, ARRAYSIZE(kMomBarberPoleStolenLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("BOLT_OF_CLOTH_TAKEN") &&
			!state.boltOfClothTakenShown) {
		state.boltOfClothTakenShown = true;

		Common::Error lineError = runtime.startupScript().isNamedNpcDeathTypeClear("SPARKY") &&
				runtime.startupScript().isNamedNpcDeathTypeClear("FIREMAN2")
			? playSequence(kMomBoltOfClothClearLines, ARRAYSIZE(kMomBoltOfClothClearLines))
			: playSequence(kMomBoltOfClothDefaultLines, ARRAYSIZE(kMomBoltOfClothDefaultLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("DINER_BURNED") &&
			runtime.startupScript().getFlagValue("KARIN_FOUND_ALIVE") &&
			!state.dinerBurnedKarinAliveShown) {
		state.dinerBurnedKarinAliveShown = true;

		Common::Error lineError = playMomLine(0x2434, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (sharedState.dialogueStateD2ea4) {
			lineError = playMomLine(0x2439, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x109, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			switch (responseIndex) {
			case 1:
				lineError = playMomLine(0x2448, 0);
				break;
			case 2:
				lineError = playMomLine(0x244d, 1);
				break;
			default:
				lineError = Common::kNoError;
				break;
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}

	if (runtime.startupScript().getFlagValue("PC_ESCAPED_JAIL") &&
			!state.escapedJailShown) {
		state.escapedJailShown = true;
		Common::Error lineError = playSequence(
			kMomEscapedJailLines, ARRAYSIZE(kMomEscapedJailLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("KARIN_KIDNAPED") &&
			!runtime.startupScript().getFlagValue("KARIN_FOUND_DEAD") &&
			!runtime.startupScript().getFlagValue("KARIN_FOUND_ALIVE") &&
			!state.karinKidnapedUnresolvedShown) {
		state.karinKidnapedUnresolvedShown = true;

		Common::Error lineError = playMomLine(0x24d7, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		sharedState.karinKidnapedDialogueState = true;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x10d, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		switch (responseIndex) {
		case 1:
			lineError = playMomLine(0x24e3, 0);
			break;
		case 2:
			lineError = playSequence(
				kMomKarinKidnappedResponse2Lines,
				ARRAYSIZE(kMomKarinKidnappedResponse2Lines));
			break;
		default:
			lineError = Common::kNoError;
			break;
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("KARIN_FOUND_ALIVE") &&
			!state.karinFoundAliveShown) {
		state.karinFoundAliveShown = true;

		Common::Error lineError = playSequence(
			kMomKarinFoundAliveIntroLines, ARRAYSIZE(kMomKarinFoundAliveIntroLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x10e, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		switch (responseIndex) {
		case 1:
			lineError = runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED")
				? playSequence(kMomKarinFoundAliveResponse1WithStephMidgameLines,
					ARRAYSIZE(kMomKarinFoundAliveResponse1WithStephMidgameLines))
				: playSequence(kMomKarinFoundAliveResponse1WithoutStephMidgameLines,
					ARRAYSIZE(kMomKarinFoundAliveResponse1WithoutStephMidgameLines));
			break;
		case 2:
			lineError = playMomLine(0x2560, 2);
			break;
		default:
			lineError = Common::kNoError;
			break;
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("KARIN_FOUND_DEAD") &&
			!state.karinFoundDeadShown) {
		state.karinFoundDeadShown = true;

		Common::Error lineError = playMomLine(0x2576, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = hasInventoryObject("K_PURSE")
			? playPcLine(0x257b, 0)
			: playPcLine(0x2580, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playPcLine(0x2585, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playMomLine(0x258b, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (currentStoryDayIndex <= 5) {
			lineError = playMomLine(0x258f, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playPcLine(0x2594, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playMomLine(0x2598, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		if (runtime.startupScript().getFlagValue("DINER_BURNED") &&
				!runtime.startupScript().isNamedNpcDeathTypeClear("EDNA")) {
			lineError = playMomLine(0x25a0, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}

	if (!runtime.startupScript().isNamedNpcDeathTypeClear("BUTCHER") &&
			!state.butcherAbsentShown) {
		state.butcherAbsentShown = true;
		Common::Error lineError = playMomLine(0x2633, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (!runtime.startupScript().isNamedNpcDeathTypeClear("MOYNAHAN") &&
			!state.moynahanAbsentShown) {
		state.moynahanAbsentShown = true;
		Common::Error lineError = playSequence(
			kMomMoynahanAbsentLines, ARRAYSIZE(kMomMoynahanAbsentLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (!runtime.startupScript().isNamedNpcDeathTypeClear("JIMMY")) {
		Common::Error lineError = runtime.startupScript().getFlagValue("JIMMY_ATTACKED")
			? playSequence(kMomJimmyAttackedLines, ARRAYSIZE(kMomJimmyAttackedLines))
			: playMomLine(0x266c, 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (!runtime.startupScript().getFlagValue("JIMMY_ATTACKED")) {
			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x111, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			switch (responseIndex) {
			case 1:
				lineError = playMomLine(0x2677, 0);
				break;
			case 2:
				lineError = playMomLine(0x267b, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playMomLine(0x267f, 3);
				break;
			default:
				lineError = Common::kNoError;
				break;
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	}

	if (!runtime.startupScript().isNamedNpcDeathTypeClear("WASP_WOMAN") &&
			!state.waspWomanAbsentShown) {
		state.waspWomanAbsentShown = true;
		Common::Error lineError = playMomLine(0x2689, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("STEPHANIE_IS_DEAD") &&
			!runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") &&
			!state.stephanieDeadPreMidgameShown) {
		state.stephanieDeadPreMidgameShown = true;
		Common::Error lineError = playSequence(
			kMomStephanieDeadPreMidgameLines,
			ARRAYSIZE(kMomStephanieDeadPreMidgameLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (runtime.startupScript().getFlagValue("DAY_5") && !state.day5Shown) {
		state.day5Shown = true;

		Common::Error lineError = playSequence(kMomDay5Lines, ARRAYSIZE(kMomDay5Lines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(0x112, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		switch (responseIndex) {
		case 1:
			lineError = playMomLine(0x20ce, 1);
			break;
		case 2:
			lineError = playMomLine(0x2201, 0);
			break;
		default:
			lineError = Common::kNoError;
			break;
		}
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		assignMomTopicBuffer(kMomDay5TopicBufferResponseLine);
	}

	if (runtime.startupScript().getFlagValue("DAY_6") && !state.day6Shown) {
		state.day6Shown = true;

		Common::Error lineError = playSequence(kMomDay6Lines, ARRAYSIZE(kMomDay6Lines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		assignMomTopicBuffer(kMomDay6TopicBufferResponseLine);
	}

	for (;;) {
		Common::String selectedTopic;
		Common::Error menuError = runtime.runKeywordMenu(
			momTopicBuffer, momTopicBufferLineIndex, selectedTopic);
		if (menuError.getCode() != Common::kNoError)
			return menuError;
		if (selectedTopic.empty())
			return Common::kNoError;
		if (selectedTopic.equalsIgnoreCase(runtime.genericByeTopic()))
			return runMomGoodbye();

		if (runtime.matchesResponseLine(selectedTopic, 0x116)) {
			Common::Error lineError = playSequence(kMomCookiesLines, ARRAYSIZE(kMomCookiesLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x118);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x119)) {
			Common::Error lineError = playSequence(kMomTopic119Lines, ARRAYSIZE(kMomTopic119Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x11a);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x11d)) {
			Common::Error lineError = playSequence(
				kMomTopic11dOpeningLines, ARRAYSIZE(kMomTopic11dOpeningLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC008AFstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playSequence(
				kMomTopic11dPostC008ALines, ARRAYSIZE(kMomTopic11dPostC008ALines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			executeActionTagIfSet(kBabyGurgleActionTag);
			lineError = playSequence(
				kMomTopic11dClosingLines, ARRAYSIZE(kMomTopic11dClosingLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC008BFstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x120);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x121)) {
			Common::Error lineError = playSequence(kMomTopic121Lines, ARRAYSIZE(kMomTopic121Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			sharedState.waspWomanDialogueState = true;
			assignMomTopicBuffer(0x122);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x123)) {
			Common::Error lineError = playSequence(kMomSparkyLines, ARRAYSIZE(kMomSparkyLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x124);
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMomPtaTopicResponseLines,
				ARRAYSIZE(kMomPtaTopicResponseLines))) {
			Common::Error lineError = playSequence(kMomPtaLines, ARRAYSIZE(kMomPtaLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(kMomPtaFollowupTopicBufferResponseLine);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x12b)) {
			Common::Error lineError = playSequence(kMomTopic12bLines, ARRAYSIZE(kMomTopic12bLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x12c);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x12d)) {
			Common::Error lineError = playSequence(kMomTopic12dLines, ARRAYSIZE(kMomTopic12dLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x12e);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x12f)) {
			Common::Error lineError = playSequence(kMomTopic12fLines, ARRAYSIZE(kMomTopic12fLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x130);
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMomCookingTopicResponseLines,
				ARRAYSIZE(kMomCookingTopicResponseLines))) {
			Common::Error lineError = playSequence(kMomCookingLines, ARRAYSIZE(kMomCookingLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(kMomCookingFollowupTopicBufferResponseLine);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x135)) {
			Common::Error lineError = playSequence(kMomTopic135Lines, ARRAYSIZE(kMomTopic135Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x136);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x137)) {
			Common::Error lineError = playSequence(kMomTopic137Lines, ARRAYSIZE(kMomTopic137Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x13a)) {
			Common::Error lineError = playMomLine(0x1fa3, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x13b);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x13c)) {
			Common::Error lineError = playSequence(
				kMomSlaughterhouseLines, ARRAYSIZE(kMomSlaughterhouseLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(kMomSlaughterhouseFollowupTopicBufferResponseLine);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x13f)) {
			sharedState.momFatherTopicState = 1;
			Common::Error lineError = playSequence(kMomFatherLines, ARRAYSIZE(kMomFatherLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (runtime.startupScript().getFlagValue("HEARD_DAD_MOAN")) {
				lineError = playSequence(
					kMomFatherHeardDadMoanLines, ARRAYSIZE(kMomFatherHeardDadMoanLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			assignMomTopicBuffer(kMomFatherFollowupTopicBufferResponseLine);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x141)) {
			if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED")) {
				Common::Error lineError = playSequence(
					kMomStephMidgameLodgeLines, ARRAYSIZE(kMomStephMidgameLodgeLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				continue;
			}

			sharedState.discussedLodgeTopic = true;
			Common::Error lineError = playSequence(kMomLodgeLines, ARRAYSIZE(kMomLodgeLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x142, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			switch (responseIndex) {
			case 1:
				lineError = playMomLine(0x1ffe, 1);
				break;
			case 2:
				lineError = playMomLine(0x2005, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playPcLine(0x200a, 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				lineError = playMomLine(0x200e, 0);
				break;
			default:
				lineError = Common::kNoError;
				break;
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(kMomLodgeFollowupTopicBufferResponseLine);
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMomBakeSaleTopicResponseLines,
				ARRAYSIZE(kMomBakeSaleTopicResponseLines))) {
			Common::Error lineError = playSequence(kMomBakeSaleLines, ARRAYSIZE(kMomBakeSaleLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(kMomBakeSaleFollowupTopicBufferResponseLine);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x14a)) {
			Common::Error lineError = playSequence(kMomTopic14aLines, ARRAYSIZE(kMomTopic14aLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMomPottsdamTopicResponseLines,
				ARRAYSIZE(kMomPottsdamTopicResponseLines)) &&
				runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED")) {
			Common::Error lineError = playMomLine(0x2086, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x14d, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			switch (responseIndex) {
			case 1:
				lineError = playSequence(
					kMomPottsdamFirstResponse1Lines,
					ARRAYSIZE(kMomPottsdamFirstResponse1Lines));
				break;
			case 2:
				lineError = playSequence(
					kMomPottsdamFirstResponse2Lines,
					ARRAYSIZE(kMomPottsdamFirstResponse2Lines));
				break;
			default:
				lineError = Common::kNoError;
				break;
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			lineError = playSequence(
				kMomPottsdamCommonFollowupLines,
				ARRAYSIZE(kMomPottsdamCommonFollowupLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			responseIndex = 0;
			responseError = runtime.runResponseMenu(0x14e, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			switch (responseIndex) {
			case 1:
				lineError = playSequence(
					kMomPottsdamSecondResponse1Lines,
					ARRAYSIZE(kMomPottsdamSecondResponse1Lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				responseIndex = 0;
				responseError = runtime.runResponseMenu(0x14f, responseIndex);
				if (responseError.getCode() != Common::kNoError)
					return responseError;

				switch (responseIndex) {
				case 1:
					lineError = playSequence(
						kMomPottsdamThirdResponse1Lines,
						ARRAYSIZE(kMomPottsdamThirdResponse1Lines));
					break;
				case 2:
					lineError = playSequence(
						kMomPottsdamThirdResponse2Lines,
						ARRAYSIZE(kMomPottsdamThirdResponse2Lines));
					break;
				default:
					lineError = Common::kNoError;
					break;
				}
				break;
			case 2:
				lineError = playSequence(
					kMomPottsdamSecondResponse2Lines,
					ARRAYSIZE(kMomPottsdamSecondResponse2Lines));
				break;
			default:
				lineError = Common::kNoError;
				break;
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			assignMomTopicBuffer(kMomPottsdamFollowupTopicBufferResponseLine);
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMomMeatPlantTopicResponseLines,
				ARRAYSIZE(kMomMeatPlantTopicResponseLines))) {
			Common::Error lineError = playSequence(
				kMomMeatPlantIntroLines, ARRAYSIZE(kMomMeatPlantIntroLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x153, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			switch (responseIndex) {
			case 1:
				lineError = playSequence(
					kMomMeatPlantResponse1Lines,
					ARRAYSIZE(kMomMeatPlantResponse1Lines));
				break;
			case 2:
				lineError = playSequence(
					kMomMeatPlantResponse2Lines,
					ARRAYSIZE(kMomMeatPlantResponse2Lines));
				break;
			default:
				lineError = Common::kNoError;
				break;
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (responseIndex == 2)
				assignMomTopicBuffer(kMomMeatPlantFollowupTopicBufferResponseLine);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x155)) {
			Common::Error lineError = playSequence(kMomTopic155Lines, ARRAYSIZE(kMomTopic155Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x156)) {
			Common::Error lineError = playSequence(kMomTopic156Lines, ARRAYSIZE(kMomTopic156Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x15b) && currentStoryDayIndex == 5) {
			Common::Error lineError = sharedState.momGoodCauseDay5State
				? playSequence(kMomGoodCauseDay5CompleteLines,
					ARRAYSIZE(kMomGoodCauseDay5CompleteLines))
				: playSequence(kMomGoodCauseDay5FirstTimeLines,
					ARRAYSIZE(kMomGoodCauseDay5FirstTimeLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			if (sharedState.momGoodCauseDay5State &&
					runtime.startupScript().getFlagValue(kSawSlashesOnBodyFlag)) {
				lineError = playSequence(
					kMomGoodCauseDay5SlashBodyLines,
					ARRAYSIZE(kMomGoodCauseDay5SlashBodyLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			if (!sharedState.momGoodCauseDay5State)
				assignMomTopicBuffer(kMomGoodCauseDay5FollowupTopicBufferResponseLine);
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMomMoynahanTopicResponseLines,
				ARRAYSIZE(kMomMoynahanTopicResponseLines))) {
			Common::Error lineError = playSequence(
				kMomMoynahanTopicLines, ARRAYSIZE(kMomMoynahanTopicLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMomNewspaperFireTopicResponseLines,
				ARRAYSIZE(kMomNewspaperFireTopicResponseLines))) {
			Common::Error lineError = playMomLine(0x229e, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x16d)) {
			Common::Error lineError = playSequence(kMomTopic16dLines, ARRAYSIZE(kMomTopic16dLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x16e)) {
			Common::Error lineError = playMomLine(0x26a3, 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x16f, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			switch (responseIndex) {
			case 1:
				lineError = playMomLine(0x26ae, 0);
				break;
			case 2:
				lineError = playMomLine(0x26b3, 2);
				break;
			default:
				lineError = Common::kNoError;
				break;
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			return runMomGoodbye();
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x170)) {
			Common::Error lineError = playMomLine(0x26bc, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x171))
			continue;

		Common::Error lineError = playMomLine(0x26c6, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		continue;
	}
}

} // End of namespace Harvester
