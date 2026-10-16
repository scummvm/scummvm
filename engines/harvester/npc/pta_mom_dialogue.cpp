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


#include "harvester/npc/pta_mom_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kPtaRespondToTvFlag = "PTA_RESPOND_TO_TV";
static const int kPtaMomResponseLineIndex = 0x297;

static const DialogueLineEntry kPtaMomResponse2Lines[] = {
	{ 0x320f, "PTA_MOM1", 0 },
	{ 0x3213, "PTA_MOM1", 0 }
};

static const DialogueLineEntry kPtaMomResponse3Lines[] = {
	{ 0x3217, "PTA_MOM1", 0 },
	{ 0x321c, "PTA_MOM1", 0 },
	{ 0x3221, "PC", 0 },
	{ 0x3226, "PTA_MOM1", 0 }
};

} // End of namespace

Common::Error PtaMomDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &) {
	(void)usedItemName;

	if (runtime.startupScript().getFlagValue(kPtaRespondToTvFlag)) {
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x3233, "PTA_MOM1", 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		(void)runtime.startupScript().setRuntimeFlagValue(kPtaRespondToTvFlag, false);
		return Common::kNoError;
	}

	Common::Error lineError = Common::kNoError;
	switch (runtime.getRandomNumber(2)) {
	case 0:
		lineError = runtime.playDialogueLineWithVariant(0x31ee, "PTA_MOM1", 1);
		break;
	case 1:
		lineError = runtime.playDialogueLineWithVariant(0x31f2, "PTA_MOM1", 1);
		break;
	default:
		lineError = runtime.playDialogueLineWithVariant(0x31f6, "PTA_MOM1", 1);
		break;
	}
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	int responseIndex = 0;
	Common::Error responseError = runtime.runResponseMenu(kPtaMomResponseLineIndex, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	switch (responseIndex) {
	case 1: {
		lineError = runtime.playDialogueLineWithVariant(0x3204, "PTA_MOM1", 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		return runtime.playDialogueLineWithVariant(0x320b, "PTA_MOM1", 0);
	}
	case 2:
		return runtime.playDialogueEntrySequence(kPtaMomResponse2Lines, ARRAYSIZE(kPtaMomResponse2Lines));
	case 3:
		return runtime.playDialogueEntrySequence(kPtaMomResponse3Lines, ARRAYSIZE(kPtaMomResponse3Lines));
	default:
		return Common::kNoError;
	}
}

} // End of namespace Harvester
