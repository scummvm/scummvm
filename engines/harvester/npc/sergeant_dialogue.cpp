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

#include "harvester/npc/sergeant_dialogue.h"

#include "common/array.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kDialogueC149FstPath = "GRAPHIC/FST/C149.FST";

} // End of namespace

bool SergeantDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("SERGEANT");
}

Common::Error SergeantDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	SergeantRoomDialogueState &state = _state;
	auto playSergeantLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, "SERGEANT", headVariant);
	};
	auto hasInventoryItem = [&](const char *objectName) {
		Common::Array<StartupObjectRecord> inventoryObjects;
		runtime.startupScript().getVisibleInventoryObjects(inventoryObjects);
		for (const StartupObjectRecord &inventoryObject : inventoryObjects) {
			if (inventoryObject.objectName.equalsIgnoreCase(objectName))
				return true;
		}

		return false;
	};
	auto markSergeantProgress = [&]() {
		sharedState.dialogueStateD2f08 = true;
		sharedState.dialogueStateD2f00 = true;
		state.introPending = false;
		state.dialogueStateD2d54 = true;
	};
	auto restoreItemToRah = [&](const char *objectName) {
		(void)runtime.startupScript().resetRuntimeObjectToInitialState(objectName);
		(void)runtime.startupScript().setRuntimeObjectVisible("RAH", objectName, true);
	};
	auto handleRemainsBranch = [&]() -> Common::Error {
		markSergeantProgress();
		restoreItemToRah("REMAINS");
		return playSergeantLine(0x4333);
	};
	auto handleInviteBranch = [&]() -> Common::Error {
		markSergeantProgress();
		(void)runtime.startupScript().resetRuntimeObjectToInitialState("STEFSKULL");
		(void)runtime.startupScript().setRuntimeObjectVisible("ST_BEDRM", "STEFSKULL", true);
		restoreItemToRah("INVITE");
		(void)runtime.startupScript().setRuntimeFlagValue("TAKEN_INVITE_TO_SERGEANT", true);
		(void)runtime.startupScript().setRuntimeFlagValue("NEED_REMAINS_FOR_LODGE", true);
		(void)runtime.startupScript().setRuntimeNpcState("DWAYNE_ST_BEDRM", false, false);
		(void)runtime.startupScript().setRuntimeNpcState("MRS_POTTS_ST_BEDRM", false, false);
		(void)runtime.startupScript().setRuntimeNpcState("MRS_POTTS", true, true);

		Common::Error lineError = runtime.playDialogueFst(kDialogueC149FstPath);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		return runtime.playDialogueLine(0x4305, "PC");
	};
	auto handleBarberPoleBranch = [&]() -> Common::Error {
		markSergeantProgress();
		restoreItemToRah("BARBER_POLE");
		(void)runtime.startupScript().setRuntimeFlagValue("ASSIGNED_DNA_TASK", true);
		return playSergeantLine(0x426f);
	};
	auto handleBoltClothBranch = [&]() -> Common::Error {
		markSergeantProgress();
		restoreItemToRah("BOLTCLTH");
		return playSergeantLine(0x4241);
	};
	auto handleCompletedApplicationBranch = [&]() -> Common::Error {
		sharedState.dialogueStateD2f08 = true;
		sharedState.dialogueStateD2f00 = true;
		state.introPending = false;
		restoreItemToRah("COMPLETED_LODGE_APPLICATION");
		if (!runtime.startupScript().getFlagValue("QUEST_1"))
			return playSergeantLine(0x41cd);
		if (!state.dialogueStateD2d54)
			return playSergeantLine(0x41d2);
		return playSergeantLine(0x41f0);
	};

	if (usedItemName.empty()) {
		if (runtime.startupScript().getFlagValue("GOT_REMAINS_FOR_LODGE"))
			return handleRemainsBranch();
		if (runtime.startupScript().getFlagValue("STEPHANIE_IS_DEAD") &&
				(hasInventoryItem("INVITE") ||
					runtime.startupScript().getFlagValue("TAKEN_INVITE_TO_SERGEANT"))) {
			return handleInviteBranch();
		}
		if (runtime.startupScript().getFlagValue("DINER_BURNED")) {
			markSergeantProgress();
			return playSergeantLine(0x42b5, 1);
		}
		if (runtime.startupScript().getFlagValue("BARBER_POLE_STOLEN"))
			return handleBarberPoleBranch();
		if (runtime.startupScript().getFlagValue("BOLT_OF_CLOTH_TAKEN"))
			return handleBoltClothBranch();
		if (runtime.startupScript().getFlagValue("SCRATCHED_TUCKER")) {
			sharedState.dialogueStateD2f08 = true;
			sharedState.dialogueStateD2f00 = true;
			state.introPending = false;
			state.dialogueStateD2d54 = true;
			state.completedFirstTaskState = true;
			return playSergeantLine(0x420d);
		}
		if (runtime.startupScript().getFlagValue("HAVE_COMPLETED_LODGE_APP"))
			return handleCompletedApplicationBranch();
		if (state.introPending) {
			sharedState.dialogueStateD2f08 = true;
			state.introPending = false;
			return playSergeantLine(0x413f);
		}
		return playSergeantLine(0x41ac);
	}

	if (usedItemName.equalsIgnoreCase("BARBER_POLE"))
		return handleBarberPoleBranch();
	if (usedItemName.equalsIgnoreCase("BOLTCLTH"))
		return handleBoltClothBranch();
	if (usedItemName.equalsIgnoreCase("COMPLETED_LODGE_APPLICATION"))
		return handleCompletedApplicationBranch();
	if (usedItemName.equalsIgnoreCase("INVITE"))
		return handleInviteBranch();
	if (usedItemName.equalsIgnoreCase("REMAINS"))
		return handleRemainsBranch();

	return playSergeantLine(0x42fa);
}

} // End of namespace Harvester
