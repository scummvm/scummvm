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


#include "harvester/npc/valet_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kValetSpeakerId = "VALET";
static const char *const kPcSpeakerId = "PC";
static const char *const kValetCutscenePath = "GRAPHIC/FST/MIDGET02.FST";
static const int kValetResponseLineIndex = 0x2fe;
static const int kValetFirstResponseIndex = 1;
static const int kValetSecondResponseIndex = 2;

static const DialogueLineEntry kValetIntroLines[] = {
	{ 0xf2c, kValetSpeakerId, 1 },
	{ 0xf31, kPcSpeakerId, 0 },
	{ 0xf35, kValetSpeakerId, 0 },
	{ 0xf39, kPcSpeakerId, 0 }
};

static const DialogueLineEntry kValetFirstResponseLine = { 0xf4a, kValetSpeakerId, 0 };
static const DialogueLineEntry kValetSecondResponseLine = { 0xf50, kValetSpeakerId, 2 };
static const DialogueLineEntry kValetExitLine = { 0xf56, kValetSpeakerId, 0 };

} // End of namespace

bool ValetDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("VALET");
}

Common::Error ValetDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &, DialogueSharedState &) {
	auto playValetLine = [&](const DialogueLineEntry &line) -> Common::Error {
		return runtime.playDialogueLineWithVariant(line.wavId, line.speakerId, line.headVariant);
	};

	Common::Error lineError = runtime.playDialogueEntrySequence(kValetIntroLines, ARRAYSIZE(kValetIntroLines));
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	lineError = runtime.playDialogueFst(kValetCutscenePath);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	int responseIndex = 0;
	Common::Error responseError = runtime.runResponseMenu(kValetResponseLineIndex, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == kValetFirstResponseIndex) {
		lineError = playValetLine(kValetFirstResponseLine);
	} else if (responseIndex == kValetSecondResponseIndex) {
		lineError = playValetLine(kValetSecondResponseLine);
	}
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	return playValetLine(kValetExitLine);
}

} // End of namespace Harvester
