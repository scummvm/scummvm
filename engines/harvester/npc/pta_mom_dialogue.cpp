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
static const char *const kPtaMomSpeakerId = "PTA_MOM1";
static const int kPtaMomResponseLineIndex = 0x297;

static const DialogueLineEntry kPtaMomResponse2Lines[] = {
	{ 0x320f, kPtaMomSpeakerId, 0 },
	{ 0x3213, kPtaMomSpeakerId, 0 }
};

static const DialogueLineEntry kPtaMomResponse3Lines[] = {
	{ 0x3217, kPtaMomSpeakerId, 0 },
	{ 0x321c, kPtaMomSpeakerId, 0 },
	{ 0x3221, "PC", 0 },
	{ 0x3226, kPtaMomSpeakerId, 0 }
};

} // End of namespace

bool PtaMomDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("PTA_MOM1") ||
		npcName.equalsIgnoreCase("PTA_MOM2") ||
		npcName.equalsIgnoreCase("PTA_MOM3") ||
		npcName.equalsIgnoreCase("PTA_MOM4") ||
		npcName.equalsIgnoreCase("PTA_MOM5");
}

Common::Error PtaMomDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &) {
	(void)usedItemName;

	auto playPtaMomLine = [&](int wavId, int headVariant) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, kPtaMomSpeakerId, headVariant);
	};

	if (runtime.startupScript().getFlagValue(kPtaRespondToTvFlag)) {
		Common::Error lineError = playPtaMomLine(0x3233, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		(void)runtime.startupScript().setRuntimeFlagValue(kPtaRespondToTvFlag, false);
		return Common::kNoError;
	}

	Common::Error lineError = Common::kNoError;
	switch (runtime.getRandomNumber(2)) {
	case 0:
		lineError = playPtaMomLine(0x31ee, 1);
		break;
	case 1:
		lineError = playPtaMomLine(0x31f2, 1);
		break;
	default:
		lineError = playPtaMomLine(0x31f6, 1);
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
		lineError = playPtaMomLine(0x3204, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		return playPtaMomLine(0x320b, 0);
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
