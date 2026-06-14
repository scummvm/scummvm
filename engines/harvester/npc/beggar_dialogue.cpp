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


#include "harvester/npc/beggar_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kBeggarNpc = "BEGGAR";
static const char *const kPcSpeaker = "PC";
static const char *const kBeggarInterruptFlag = "BEGGAR_INTERRUPT_CONVERSATION";
static const char *const kBeggarDialog3ActionTag = "BEGGAR_DIALOG_3";
static const int kBeggarInitialResponseLine = 0x0;
static const int kBeggarFollowupResponseLine = 0x1;

} // End of namespace

bool BeggarDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase(kBeggarNpc);
}

Common::Error BeggarDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &, DialogueSharedState &) {
	auto playBeggarLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, kBeggarNpc, headVariant);
	};
	auto playPcLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, kPcSpeaker, headVariant);
	};
	auto queueBeggarMonsterfyTransition = [&]() {
		InteractionResult interaction;
		if (runtime.startupScript().queueRuntimeNpcDeathOrMonsterfy(kBeggarNpc)) {
			interaction.mutatedRuntimeState = true;
			interaction.visualRuntimeStateChanged = true;
		}
		runtime.queueDialogueInteractionIfNeeded(interaction);
	};

	if (!runtime.startupScript().getFlagValue(kBeggarInterruptFlag))
		return Common::kNoError;

	InteractionResult interaction;
	if (runtime.executeActionTag(kBeggarDialog3ActionTag, interaction)) {
		runtime.applyImmediateDialogueInteractionEffects(interaction);
		runtime.queueDialogueInteractionIfNeeded(interaction);
	}

	Common::Error lineError = playBeggarLine(0x1276, 1);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	lineError = playPcLine(0x127a, 2);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	lineError = playBeggarLine(0x127e);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	int responseIndex = 0;
	Common::Error responseError = runtime.runResponseMenu(
		kBeggarInitialResponseLine, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 1) {
		lineError = playBeggarLine(0x128d, 1);
	} else if (responseIndex == 2) {
		lineError = playBeggarLine(0x1291, 3);
	} else {
		queueBeggarMonsterfyTransition();
		return Common::kNoError;
	}
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	lineError = playBeggarLine(0x1295);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	responseIndex = 0;
	responseError = runtime.runResponseMenu(
		kBeggarFollowupResponseLine, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 1) {
		lineError = playBeggarLine(0x12ab, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	} else if (responseIndex == 2) {
		lineError = playBeggarLine(0x12a5, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	queueBeggarMonsterfyTransition();
	return Common::kNoError;
}

} // End of namespace Harvester
