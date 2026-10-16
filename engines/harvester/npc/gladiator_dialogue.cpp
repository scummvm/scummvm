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


#include "harvester/npc/gladiator_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kGladiatorInterruptFlag = "GLADIATOR_INTERRUPT_CONVERSATION";
static const char *const kStartMercyTimerActionTag = "START_MERCY_TIMR";
static const char *const kDialogueC110FstPath = "GRAPHIC/FST/C110.FST";
static const int kGladiatorInitialResponseLine = 0xc3;
static const int kGladiatorFollowupResponseLine = 0xc4;

} // End of namespace

Common::Error GladiatorDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &, DialogueSharedState &) {
	auto queueGladiatorMonsterfyTransition = [&]() {
		InteractionResult interaction;
		if (runtime.startupScript().queueRuntimeNpcDeathOrMonsterfy("GLADIATOR")) {
			interaction.mutatedRuntimeState = true;
			interaction.visualRuntimeStateChanged = true;
		}
		runtime.queueDialogueInteractionIfNeeded(interaction);
	};
	auto runGladiatorDeathBranch = [&](int leadWavId) -> Common::Error {
		Common::Error lineError = runtime.playDialogueLineWithVariant(leadWavId, "GLADIATOR", 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = runtime.playDialogueLineWithVariant(0x6cd, "GLADIATOR", 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		queueGladiatorMonsterfyTransition();
		return Common::kNoError;
	};

	if (!runtime.startupScript().getFlagValue(kGladiatorInterruptFlag))
		return Common::kNoError;

	(void)runtime.startupScript().setRuntimeFlagValue(kGladiatorInterruptFlag, false);

	Common::Error lineError = runtime.playDialogueLineWithVariant(0x69c, "GLADIATOR", 1);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	lineError = runtime.playDialogueLineWithVariant(0x6a3, "GLADIATOR", 0);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	int responseIndex = 0;
	Common::Error responseError = runtime.runResponseMenu(
		kGladiatorInitialResponseLine, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 1)
		return runGladiatorDeathBranch(0x6af);
	if (responseIndex != 2)
		return Common::kNoError;

	lineError = runtime.playDialogueLineWithVariant(0x6b3, "GLADIATOR", 0);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	lineError = runtime.playDialogueFst(kDialogueC110FstPath);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	responseIndex = 0;
	responseError = runtime.runResponseMenu(
		kGladiatorFollowupResponseLine, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 1)
		return runGladiatorDeathBranch(0x6c3);
	if (responseIndex != 2)
		return Common::kNoError;

	lineError = runtime.playDialogueLineWithVariant(0x6c7, "GLADIATOR", 1);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	lineError = runtime.playDialogueLineWithVariant(0x6cd, "GLADIATOR", 0);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	InteractionResult interaction;
	if (runtime.executeActionTag(kStartMercyTimerActionTag, interaction)) {
		runtime.applyImmediateDialogueInteractionEffects(interaction);
		runtime.queueDialogueInteractionIfNeeded(interaction);
	}

	return Common::kNoError;
}

} // End of namespace Harvester
