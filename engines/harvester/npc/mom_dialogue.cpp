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

#include "harvester/npc/mom_dialogue.h"

#include "harvester/npc/dialogue_flags.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kDialogueC008AFstPath = "GRAPHIC/FST/C008A.FST";
static const char *const kDialogueC008BFstPath = "GRAPHIC/FST/C008B.FST";
static const char *const kBabyGurgleActionTag = "BABY_GURGLE";

static const DialogueLineEntry kMomIntroLines[] = {
	{ 0x1dd7, "MOM", 1 },
	{ 0x1ddc, "PC", 0 },
	{ 0x1de0, "MOM", 2 },
	{ 0x1de4, "PC", 2 }
};

static const DialogueLineEntry kMomCookiesLines[] = {
	{ 0x1e18, "PC", 0 },
	{ 0x1e1d, "MOM", 1 },
	{ 0x1e21, "PC", 0 },
	{ 0x1e25, "MOM", 0 },
	{ 0x1e29, "PC", 0 }
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

static const DialogueLineEntry kMomSparkyLines[] = {
	{ 0x1ed7, "PC", 0 },
	{ 0x1edb, "MOM", 2 },
	{ 0x1ee1, "PC", 0 },
	{ 0x1ee5, "MOM", 2 }
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

static const DialogueLineEntry kMomLodgeResponse1Lines[] = {
	{ 0x1ffe, "MOM", 1 }
};

static const DialogueLineEntry kMomLodgeResponse2Lines[] = {
	{ 0x2005, "MOM", 2 },
	{ 0x200a, "PC", 0 },
	{ 0x200e, "MOM", 0 }
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

static const DialogueLineEntry kMomMeatPlantIntroLines[] = {
	{ 0x20f7, "PC", 0 },
	{ 0x20fb, "MOM", 0 }
};

static const DialogueLineEntry kMomMeatPlantResponse1Lines[] = {
	{ 0x2106, "PC", 0 },
	{ 0x210a, "MOM", 0 },
	{ 0x210e, "PC", 0 }
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

static const DialogueLineEntry kMomGoodCauseDay5Lines[] = {
	{ 0x21a3, "PC", 0 },
	{ 0x21a8, "MOM", 1 },
	{ 0x21ad, "PC", 0 },
	{ 0x21b2, "MOM", 0 },
	{ 0x21b7, "PC", 0 }
};

static const DialogueLineEntry kMomSlaughterhouseLines[] = {
	{ 0x1faf, "PC", 4 },
	{ 0x1fb4, "MOM", 0 },
	{ 0x1fb9, "PC", 2 },
	{ 0x1fbd, "MOM", 0 }
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
static const int kMomPostIntroTopicBufferResponseLine = 0x105;
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
	auto runMomGoodbye = [&]() -> Common::Error {
		if (!runtime.startupScript().getFlagValue("DAY_FLAG"))
			return runtime.playDialogueLine(0x256e, "MOM");
		if (!runtime.startupScript().isNamedNpcDeathTypeClear("JIMMY"))
			return Common::kNoError;

		return runtime.playDialogueLine(0x2051, "MOM");
	};

	if (!usedItemName.empty()) {
		if (usedItemName.equalsIgnoreCase("NOTE") ||
				usedItemName.equalsIgnoreCase("NOTE_PHOTOCOPY") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK") ||
				usedItemName.equalsIgnoreCase("CHECKBOOK_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceOfBlackmail, true);
			return runtime.playDialogueLine(0x2317, "MOM");
		}
		if ((usedItemName.equalsIgnoreCase("LEDGER") ||
				usedItemName.equalsIgnoreCase("LEDGER2")) &&
				runtime.startupScript().getFlagValue("HAVE_BOTH_LEDGERS")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownLedgersToAnyone, true);
			return runtime.playDialogueLine(0x2320, "MOM");
		}
		if (usedItemName.equalsIgnoreCase("CASKET_PHOTO") ||
				usedItemName.equalsIgnoreCase("CASKET_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfCorpse, true);
			return runtime.playDialogueLine(0x233b, "PC");
		}
		if (usedItemName.equalsIgnoreCase("PHOTO_OF_WHALEY_HERRILL")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownPhotoOfWhaleyHerrill, true);
			return runtime.playDialogueLine(0x239c, "MOM");
		}
		if (usedItemName.equalsIgnoreCase("TV_DEED") ||
				usedItemName.equalsIgnoreCase("TV_DEED_PHOTOCOPY")) {
			(void)runtime.startupScript().setRuntimeFlagValue(DialogueFlags::kShownEvidenceSheriffOwns, true);
			return runtime.playDialogueLine(0x2382, "MOM");
		}

		return runtime.playDialogueLine(0x26cc, "MOM");
	}

	bool skipMomDefaultKeywordSeed = false;
	if (state.introPending) {
		state.introPending = false;
		state.sameDayIntroLineEnabled = true;
		state.postIntroDefaultLineEnabled = true;
		state.introDayIndex = currentStoryDayIndex;
		Common::Error lineError = runtime.playDialogueEntrySequence(kMomIntroLines, ARRAYSIZE(kMomIntroLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		assignMomTopicBuffer(kMomInitialTopicBufferResponseLine);
		skipMomDefaultKeywordSeed = true;
	}
	if (!skipMomDefaultKeywordSeed && state.sameDayIntroLineEnabled &&
			currentStoryDayIndex == state.introDayIndex) {
		Common::Error lineError = runtime.playDialogueLine(0x2047, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		assignMomTopicBuffer(kMomSameDayTopicBufferResponseLine);
		skipMomDefaultKeywordSeed = true;
	}
	if (!skipMomDefaultKeywordSeed && state.postIntroDefaultLineEnabled) {
		Common::Error lineError = runtime.playDialogueLine(0x2311, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		assignMomTopicBuffer(kMomPostIntroTopicBufferResponseLine);
	}
	if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") &&
			!state.stephMidgameShown) {
		state.stephMidgameShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x205a, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("DINER_BURNED") &&
			(runtime.startupScript().getFlagValue("KARIN_KIDNAPED") ||
				runtime.startupScript().getFlagValue("KARIN_FOUND_DEAD")) &&
			!state.dinerBurnedKarinMissingOrDeadShown) {
		state.dinerBurnedKarinMissingOrDeadShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x2456, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("BURNED_TV_STATION") &&
			!state.burnedTvStationShown) {
		state.burnedTvStationShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x2220, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("SCRATCHED_TUCKER") &&
			!state.scratchedTuckerShown) {
		state.scratchedTuckerShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x23f3, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("BARBER_POLE_STOLEN") &&
			!state.barberPoleStolenShown) {
		state.barberPoleStolenShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x23fb, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("BOLT_OF_CLOTH_TAKEN") &&
			!state.boltOfClothTakenShown) {
		state.boltOfClothTakenShown = true;
		if (runtime.startupScript().isNamedNpcDeathTypeClear("SPARKY") &&
				runtime.startupScript().isNamedNpcDeathTypeClear("FIREMAN2")) {
			Common::Error lineError = runtime.playDialogueLine(0x2416, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		Common::Error lineError = runtime.playDialogueLine(0x2420, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("DINER_BURNED") &&
			runtime.startupScript().getFlagValue("KARIN_FOUND_ALIVE") &&
			!state.dinerBurnedKarinAliveShown) {
		state.dinerBurnedKarinAliveShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x2434, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("PC_ESCAPED_JAIL") &&
			!state.escapedJailShown) {
		state.escapedJailShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x2495, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("KARIN_KIDNAPED") &&
			!runtime.startupScript().getFlagValue("KARIN_FOUND_DEAD") &&
			!runtime.startupScript().getFlagValue("KARIN_FOUND_ALIVE") &&
			!state.karinKidnapedUnresolvedShown) {
		state.karinKidnapedUnresolvedShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x24d7, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("KARIN_FOUND_ALIVE") &&
			!state.karinFoundAliveShown) {
		state.karinFoundAliveShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x2505, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("KARIN_FOUND_DEAD") &&
			!state.karinFoundDeadShown) {
		state.karinFoundDeadShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x2576, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (!runtime.startupScript().isNamedNpcDeathTypeClear("BUTCHER") &&
			!state.butcherAbsentShown) {
		state.butcherAbsentShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x2633, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (!runtime.startupScript().isNamedNpcDeathTypeClear("MOYNAHAN") &&
			!state.moynahanAbsentShown) {
		state.moynahanAbsentShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x2647, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (!runtime.startupScript().isNamedNpcDeathTypeClear("JIMMY") &&
			!state.jimmyAbsentShown) {
		state.jimmyAbsentShown = true;
		if (runtime.startupScript().getFlagValue("JIMMY_ATTACKED")) {
			Common::Error lineError = runtime.playDialogueLine(0x2659, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		Common::Error lineError = runtime.playDialogueLine(0x266c, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (!runtime.startupScript().isNamedNpcDeathTypeClear("WASP_WOMAN") &&
			!state.waspWomanAbsentShown) {
		state.waspWomanAbsentShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x2689, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("STEPHANIE_IS_DEAD") &&
			!runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED") &&
			!state.stephanieDeadPreMidgameShown) {
		state.stephanieDeadPreMidgameShown = true;
		Common::Error lineError = runtime.playDialogueLine(0x217c, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	if (runtime.startupScript().getFlagValue("DAY_5") && !state.day5Shown) {
		state.day5Shown = true;
		Common::Error lineError = runtime.playDialogueLine(0x218d, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		assignMomTopicBuffer(kMomDay5TopicBufferResponseLine);
	}
	if (runtime.startupScript().getFlagValue("DAY_6") && !state.day6Shown) {
		state.day6Shown = true;
		Common::Error lineError = runtime.playDialogueLine(0x22a8, "MOM");
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
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomCookiesLines, ARRAYSIZE(kMomCookiesLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x118);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x119)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomTopic119Lines, ARRAYSIZE(kMomTopic119Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x11a);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x11d)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomTopic11dOpeningLines, ARRAYSIZE(kMomTopic11dOpeningLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueFst(kDialogueC008AFstPath);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueEntrySequence(
				kMomTopic11dPostC008ALines, ARRAYSIZE(kMomTopic11dPostC008ALines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			StartupInteractionResult babyGurgleInteraction;
			if (runtime.startupScript().executeActionTag(kBabyGurgleActionTag, babyGurgleInteraction)) {
				runtime.applyImmediateDialogueInteractionEffects(babyGurgleInteraction);
				runtime.queueDialogueInteractionIfNeeded(babyGurgleInteraction);
			}

			lineError = runtime.playDialogueEntrySequence(
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
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomTopic121Lines, ARRAYSIZE(kMomTopic121Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x122);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x123)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomSparkyLines, ARRAYSIZE(kMomSparkyLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x124);
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMomPtaTopicResponseLines,
				ARRAYSIZE(kMomPtaTopicResponseLines))) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomPtaLines, ARRAYSIZE(kMomPtaLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(kMomPtaFollowupTopicBufferResponseLine);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x12b)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomTopic12bLines, ARRAYSIZE(kMomTopic12bLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x12c);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x12d)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomTopic12dLines, ARRAYSIZE(kMomTopic12dLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x12e);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x12f)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomTopic12fLines, ARRAYSIZE(kMomTopic12fLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x130);
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMomCookingTopicResponseLines,
				ARRAYSIZE(kMomCookingTopicResponseLines))) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomCookingLines, ARRAYSIZE(kMomCookingLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(kMomCookingFollowupTopicBufferResponseLine);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x135)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomTopic135Lines, ARRAYSIZE(kMomTopic135Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x136);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x137)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomTopic137Lines, ARRAYSIZE(kMomTopic137Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x13a)) {
			Common::Error lineError = runtime.playDialogueLine(0x1fa3, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(0x13b);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x13c)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomSlaughterhouseLines, ARRAYSIZE(kMomSlaughterhouseLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(kMomSlaughterhouseFollowupTopicBufferResponseLine);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x13f)) {
			state.fatherTopicState = true;
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomFatherLines, ARRAYSIZE(kMomFatherLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (runtime.startupScript().getFlagValue("HEARD_DAD_MOAN")) {
				lineError = runtime.playDialogueEntrySequence(
					kMomFatherHeardDadMoanLines, ARRAYSIZE(kMomFatherHeardDadMoanLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
			assignMomTopicBuffer(kMomFatherFollowupTopicBufferResponseLine);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x141)) {
			Common::Error lineError = Common::kNoError;
			if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED")) {
				lineError = runtime.playDialogueEntrySequence(
					kMomStephMidgameLodgeLines, ARRAYSIZE(kMomStephMidgameLodgeLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				continue;
			}
			sharedState.discussedLodgeTopic = true;
			lineError = runtime.playDialogueEntrySequence(kMomLodgeLines, ARRAYSIZE(kMomLodgeLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x142, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			switch (responseIndex) {
			case 1:
				lineError = runtime.playDialogueEntrySequence(
					kMomLodgeResponse1Lines, ARRAYSIZE(kMomLodgeResponse1Lines));
				break;
			case 2:
				lineError = runtime.playDialogueEntrySequence(
					kMomLodgeResponse2Lines, ARRAYSIZE(kMomLodgeResponse2Lines));
				break;
			default:
				break;
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(kMomLodgeFollowupTopicBufferResponseLine);
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMomBakeSaleTopicResponseLines,
				ARRAYSIZE(kMomBakeSaleTopicResponseLines))) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomBakeSaleLines, ARRAYSIZE(kMomBakeSaleLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(kMomBakeSaleFollowupTopicBufferResponseLine);
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x14a)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomTopic14aLines, ARRAYSIZE(kMomTopic14aLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMomPottsdamTopicResponseLines,
				ARRAYSIZE(kMomPottsdamTopicResponseLines))) {
			Common::Error lineError = Common::kNoError;
			if (runtime.startupScript().getFlagValue("STEPH_MIDGAME_PLAYED"))
				lineError = runtime.playDialogueLine(0x2086, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(kMomPottsdamFollowupTopicBufferResponseLine);
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMomMeatPlantTopicResponseLines,
				ARRAYSIZE(kMomMeatPlantTopicResponseLines))) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomMeatPlantIntroLines, ARRAYSIZE(kMomMeatPlantIntroLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(0x153, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			switch (responseIndex) {
			case 1:
				lineError = runtime.playDialogueEntrySequence(
					kMomMeatPlantResponse1Lines, ARRAYSIZE(kMomMeatPlantResponse1Lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				continue;
			case 2:
				lineError = runtime.playDialogueEntrySequence(
					kMomMeatPlantResponse2Lines, ARRAYSIZE(kMomMeatPlantResponse2Lines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				assignMomTopicBuffer(kMomMeatPlantFollowupTopicBufferResponseLine);
				continue;
			default:
				continue;
			}
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x155)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomTopic155Lines, ARRAYSIZE(kMomTopic155Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x156)) {
			Common::Error lineError = runtime.playDialogueEntrySequence(
				kMomTopic156Lines, ARRAYSIZE(kMomTopic156Lines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x15b) && currentStoryDayIndex == 5) {
			Common::Error lineError = Common::kNoError;
			if (state.goodCauseDay5State) {
				lineError = runtime.playDialogueEntrySequence(
					kMomGoodCauseDay5Lines, ARRAYSIZE(kMomGoodCauseDay5Lines));
			} else {
				lineError = runtime.playDialogueLine(0x21dd, "PC");
			}
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			assignMomTopicBuffer(kMomGoodCauseDay5FollowupTopicBufferResponseLine);
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMomMoynahanTopicResponseLines,
				ARRAYSIZE(kMomMoynahanTopicResponseLines))) {
			Common::Error lineError = runtime.playDialogueLine(0x220a, "PC");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesAnyResponseLine(selectedTopic, kMomNewspaperFireTopicResponseLines,
				ARRAYSIZE(kMomNewspaperFireTopicResponseLines))) {
			Common::Error lineError = runtime.playDialogueLine(0x229e, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x16d)) {
			Common::Error lineError = runtime.playDialogueLine(0x2696, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x16e)) {
			Common::Error lineError = runtime.playDialogueLine(0x26a3, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}
		if (runtime.matchesResponseLine(selectedTopic, 0x170)) {
			Common::Error lineError = runtime.playDialogueLine(0x26bc, "MOM");
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			continue;
		}

		Common::Error lineError = runtime.playDialogueLine(0x26c6, "MOM");
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		return runMomGoodbye();
	}
}

} // End of namespace Harvester
