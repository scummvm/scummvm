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


#include "harvester/npc/dark_woman_dialogue.h"

#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kDarkWomanNpc = "DARK_WOMAN";
static const char *const kHandMirrorItemName = "HANDMIRROR";
static const char *const kInventoryOwnerName = "INVENTORY";
static const char *const kSetDarkWomanFlagActionTag = "SET_D_W_FLAG";
static const char *const kDialogueC109FstPath = "GRAPHIC/FST/C109.FST";
static const int kDarkWomanInitialResponseLine = 0x4b;
static const int kDarkWomanSecondResponseLine = 0x4c;
static const int kDarkWomanThirdResponseLine = 0x4d;
static const int kDarkWomanHasMirrorResponseLine = 0x4e;
static const int kDarkWomanNeedsMirrorResponseLine = 0x4f;

} // End of namespace

bool DarkWomanDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase(kDarkWomanNpc);
}

Common::Error DarkWomanDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &) {
	auto playDarkWomanLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, kDarkWomanNpc, headVariant);
	};
	auto queueDarkWomanMonsterfyTransition = [&]() {
		InteractionResult interaction;
		if (runtime.startupScript().queueRuntimeNpcDeathOrMonsterfy(kDarkWomanNpc)) {
			interaction.mutatedRuntimeState = true;
			interaction.visualRuntimeStateChanged = true;
		}
		runtime.queueDialogueInteractionIfNeeded(interaction);
	};
	auto runDarkWomanMonsterfyBranch = [&]() -> Common::Error {
		Common::Error lineError = playDarkWomanLine(0x4ce2, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		queueDarkWomanMonsterfyTransition();
		return Common::kNoError;
	};
	auto runMirrorRevealBranch = [&]() -> Common::Error {
		Common::Error lineError = playDarkWomanLine(0x4d03, 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = runtime.playDialogueFst(kDialogueC109FstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		lineError = playDarkWomanLine(0x4d0d, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		const bool changed = runtime.startupScript().setRuntimeObjectVisible(
			kInventoryOwnerName, kHandMirrorItemName, false);
		InteractionResult interaction;
		if (runtime.executeActionTag(kSetDarkWomanFlagActionTag, interaction)) {
			interaction.mutatedRuntimeState |= changed;
			runtime.applyImmediateDialogueInteractionEffects(interaction);
		} else {
			interaction.mutatedRuntimeState = changed;
		}
		runtime.queueDialogueInteractionIfNeeded(interaction);
		return Common::kNoError;
	};

	if (usedItemName.equalsIgnoreCase(kHandMirrorItemName))
		return runMirrorRevealBranch();

	if (!_state.talkStatePending)
		return Common::kNoError;

	_state.talkStatePending = false;

	Common::Error lineError = playDarkWomanLine(0x4cd5, 1);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	int responseIndex = 0;
	Common::Error responseError = runtime.runResponseMenu(
		kDarkWomanInitialResponseLine, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 1)
		return runDarkWomanMonsterfyBranch();
	if (responseIndex != 2)
		return Common::kNoError;

	lineError = playDarkWomanLine(0x4ce6, 4);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	responseIndex = 0;
	responseError = runtime.runResponseMenu(
		kDarkWomanSecondResponseLine, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 1)
		return runDarkWomanMonsterfyBranch();
	if (responseIndex != 2)
		return Common::kNoError;

	lineError = playDarkWomanLine(0x4ce6, 4);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	responseIndex = 0;
	responseError = runtime.runResponseMenu(
		kDarkWomanThirdResponseLine, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 1)
		return runDarkWomanMonsterfyBranch();
	if (responseIndex != 2)
		return Common::kNoError;

	lineError = playDarkWomanLine(0x4ce6, 4);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	const bool hasHandMirror = runtime.startupScript().isObjectInInventory(kHandMirrorItemName);
	responseIndex = 0;
	responseError = runtime.runResponseMenu(
		hasHandMirror ? kDarkWomanHasMirrorResponseLine : kDarkWomanNeedsMirrorResponseLine,
		responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 1)
		return runDarkWomanMonsterfyBranch();
	if (!hasHandMirror)
		return playDarkWomanLine(0x4cfc, 2);

	return runMirrorRevealBranch();
}

} // End of namespace Harvester
