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


#include "harvester/npc/madam_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kHookerNpc = "HOOKER";
static const char *const kBarCash200ItemName = "BARCASH200";
static const char *const kInventoryOwnerName = "INVENTORY";
static const char *const kMadamInterruptFlag = "MADAM_INTERRUPT_CONVERSATION";
static const char *const kHurtPcActionTag = "HURT_PC_0";
static const char *const kActivateLustExitActionTag = "ACTV_LUST_EXIT";
static const char *const kDialogueHoarloveFstPath = "GRAPHIC/FST/HOARLOVE.FST";
static const char *const kDialogueC117FstPath = "GRAPHIC/FST/C117.FST";
static const int kMadamInitialResponseLine = 0xfb;
static const int kMadamPaymentResponseLine = 0xfc;

} // End of namespace

Common::Error MadamDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &, DialogueSharedState &) {
	auto queueHookerMonsterfyTransition = [&]() {
		InteractionResult interaction;
		if (runtime.startupScript().queueRuntimeNpcDeathOrMonsterfy(kHookerNpc)) {
			interaction.mutatedRuntimeState = true;
			interaction.visualRuntimeStateChanged = true;
		}
		runtime.queueDialogueInteractionIfNeeded(interaction);
	};
	auto executeActionTagIfSet = [&](const char *actionTag, bool mutatedRuntimeState) {
		InteractionResult interaction;
		interaction.mutatedRuntimeState = mutatedRuntimeState;
		if (runtime.executeActionTag(actionTag, interaction)) {
			runtime.applyImmediateDialogueInteractionEffects(interaction);
			runtime.queueDialogueInteractionIfNeeded(interaction);
		} else if (interaction.mutatedRuntimeState) {
			runtime.queueDialogueInteractionIfNeeded(interaction);
		}
	};

	if (!runtime.startupScript().getFlagValue(kMadamInterruptFlag))
		return Common::kNoError;

	(void)runtime.startupScript().setRuntimeFlagValue(kMadamInterruptFlag, false);

	Common::Error lineError = runtime.playDialogueLineWithVariant(0x1537, "MADAM", 1);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	int responseIndex = 0;
	Common::Error responseError = runtime.runResponseMenu(kMadamInitialResponseLine, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex != 1) {
		lineError = runtime.playDialogueLineWithVariant(0x1577, "MADAM", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = runtime.playDialogueLineWithVariant(0x157b, "MADAM", 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		queueHookerMonsterfyTransition();
		return Common::kNoError;
	}

	lineError = runtime.playDialogueLineWithVariant(0x1542, "MADAM", 1);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	if (!runtime.startupScript().isObjectInInventory(kBarCash200ItemName)) {
		lineError = runtime.playDialogueLineWithVariant(0x156e, "PC", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = runtime.playDialogueLineWithVariant(0x1572, "MADAM", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = runtime.playDialogueLineWithVariant(0x157b, "MADAM", 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		queueHookerMonsterfyTransition();
		return Common::kNoError;
	}

	responseIndex = 0;
	responseError = runtime.runResponseMenu(kMadamPaymentResponseLine, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 1) {
		lineError = runtime.playDialogueLineWithVariant(0x154e, "MADAM", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = runtime.playDialogueLineWithVariant(0x157b, "MADAM", 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		queueHookerMonsterfyTransition();
		return Common::kNoError;
	}
	if (responseIndex != 2)
		return Common::kNoError;

	lineError = runtime.playDialogueLineWithVariant(0x1555, "MADAM", 1);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	const bool removedBarCash200 = runtime.startupScript().setRuntimeObjectVisible(
		kInventoryOwnerName, kBarCash200ItemName, false);

	lineError = runtime.playDialogueFst(kDialogueHoarloveFstPath);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	lineError = runtime.playDialogueLineWithVariant(0x155a, "MADAM", 0);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	lineError = runtime.playDialogueLineWithVariant(0x1563, "MADAM", 0);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	lineError = runtime.playDialogueFst(kDialogueC117FstPath);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	executeActionTagIfSet(kHurtPcActionTag, removedBarCash200);
	executeActionTagIfSet(kActivateLustExitActionTag, false);
	return Common::kNoError;
}

} // End of namespace Harvester
