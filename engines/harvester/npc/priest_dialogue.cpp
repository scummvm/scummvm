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


#include "harvester/npc/priest_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kPcTalkedToPriestFlag = "PC_TALKED_TO_PRIEST";
static const char *const kPriestFlag = "PRIEST_FLAG";
static const char *const kFollowerMonsterfyActionTag = "FOLOWR_MNSTFY_A";
static const char *const kReligiousExitsActionTag = "RELIG_EXITS_A";
static const int kPriestInitialResponseLine = 0x27a;
static const int kPriestSecondResponseLine = 0x27b;
static const int kPriestThirdResponseLine = 0x27c;
static const int kPriestFourthResponseLine = 0x27d;
static const int kPriestFinalResponseLine = 0x27e;

} // End of namespace

Common::Error PriestDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &, DialogueSharedState &) {
	auto runFollowerMonsterfyBranch = [&]() -> Common::Error {
		Common::Error lineError = runtime.playDialogueLineWithVariant(0xcc4, "PRIEST", 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		runtime.executeAndQueueAction(kFollowerMonsterfyActionTag);
		return Common::kNoError;
	};

	if (runtime.startupScript().getFlagValue(kPcTalkedToPriestFlag))
		return Common::kNoError;

	(void)runtime.startupScript().setRuntimeFlagValue(kPcTalkedToPriestFlag, true);

	Common::Error lineError = runtime.playDialogueLineWithVariant(0xcb2, "PRIEST", 0);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	int responseIndex = 0;
	Common::Error responseError = runtime.runResponseMenu(kPriestInitialResponseLine, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 2)
		return runFollowerMonsterfyBranch();
	if (responseIndex != 1)
		return Common::kNoError;

	lineError = runtime.playDialogueLineWithVariant(0xcbf, "PRIEST", 1);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	responseIndex = 0;
	responseError = runtime.runResponseMenu(kPriestSecondResponseLine, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 2)
		return runFollowerMonsterfyBranch();
	if (responseIndex != 1)
		return Common::kNoError;

	lineError = runtime.playDialogueLineWithVariant(0xccf, "PRIEST", 1);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	responseIndex = 0;
	responseError = runtime.runResponseMenu(kPriestThirdResponseLine, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 1)
		return runFollowerMonsterfyBranch();
	if (responseIndex != 2)
		return Common::kNoError;

	lineError = runtime.playDialogueLineWithVariant(0xce5, "PRIEST", 1);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	responseIndex = 0;
	responseError = runtime.runResponseMenu(kPriestFourthResponseLine, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 2)
		return runFollowerMonsterfyBranch();
	if (responseIndex != 1)
		return Common::kNoError;

	lineError = runtime.playDialogueLineWithVariant(0xcf2, "PRIEST", 1);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	responseIndex = 0;
	responseError = runtime.runResponseMenu(kPriestFinalResponseLine, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 4) {
		lineError = runtime.playDialogueLineWithVariant(0xd0e, "PRIEST", 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		(void)runtime.startupScript().setRuntimeFlagValue(kPriestFlag, false);
		runtime.executeAndQueueAction(kReligiousExitsActionTag);
		return Common::kNoError;
	}

	lineError = runtime.playDialogueLineWithVariant(0xd13, "PRIEST", 3);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	(void)runtime.startupScript().setRuntimeFlagValue(kPriestFlag, true);
	runtime.executeAndQueueAction(kFollowerMonsterfyActionTag);
	return Common::kNoError;
}

} // End of namespace Harvester
