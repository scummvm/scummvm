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


#include "harvester/npc/inquisitor_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kInquisitorNpc = "INQUISITOR";
static const char *const kPainSolvedFlag = "PAIN_SOLVED";
static const char *const kStartInquisitorTimerActionTag = "START_INQ_TIM";
static const char *const kTorch2FstPath = "GRAPHIC/FST/TORCH2.FST";

} // End of namespace

bool InquisitorDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase(kInquisitorNpc);
}

Common::Error InquisitorDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &, DialogueSharedState &) {
	if (runtime.startupScript().getFlagValue(kPainSolvedFlag) || !_state.talkStatePending)
		return Common::kNoError;

	_state.talkStatePending = false;

	auto playInquisitorLine = [&](int wavId) -> Common::Error {
		return runtime.playDialogueLine(wavId, kInquisitorNpc);
	};

	Common::Error lineError = playInquisitorLine(0x18e7);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	lineError = playInquisitorLine(0x18e8);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	lineError = playInquisitorLine(0x18e9);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	lineError = runtime.playDialogueFst(kTorch2FstPath);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	lineError = playInquisitorLine(0x18f8);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	InteractionResult interaction;
	if (runtime.executeActionTag(kStartInquisitorTimerActionTag, interaction)) {
		runtime.applyImmediateDialogueInteractionEffects(interaction);
		runtime.queueDialogueInteractionIfNeeded(interaction);
	}

	return Common::kNoError;
}

} // End of namespace Harvester
