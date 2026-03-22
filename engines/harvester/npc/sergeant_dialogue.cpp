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
static const char *const kGoDay2ActionTag = "GO_DAY_2";
static const int kSergeantSecondTaskResponseLine = 0x2b4;
static const int kSergeantCompletedApplicationResponseLine = 0x2b6;
static const int kSergeantIntroResponseLine = 0x2b7;
static const int kSergeantApplicationResponseLine = 0x2b8;
static const int kSergeantAmnesiaResponseLine = 0x2b9;
static const int kSergeantReminderResponseLine = 0x2bb;

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
		state.dialogueStateD2d50 = true;
		state.dialogueStateD2d54 = true;
	};
	auto restoreItemToRah = [&](const char *objectName) {
		(void)runtime.startupScript().resetRuntimeObjectToInitialState(objectName);
		(void)runtime.startupScript().setRuntimeObjectVisible("RAH", objectName, true);
	};
	auto executeActionTagIfSet = [&](const char *actionTag) -> Common::Error {
		StartupInteractionResult interaction;
		if (!runtime.startupScript().executeActionTag(actionTag, interaction))
			return Common::kNoError;

		runtime.applyImmediateDialogueInteractionEffects(interaction);
		runtime.queueDialogueInteractionIfNeeded(interaction);
		return Common::kNoError;
	};
	auto runDay5ReminderIfNeeded = [&]() -> Common::Error {
		if (!runtime.startupScript().getFlagValue("DAY_5") || state.day5ReminderShown)
			return Common::kNoError;

		state.day5ReminderShown = true;
		return playSergeantLine(0x4324, 1);
	};
	auto runApplicationDiscussion = [&]() -> Common::Error {
		Common::Error lineError = playSergeantLine(0x415d);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(
			kSergeantApplicationResponseLine, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			const DialogueLineEntry applicationLines[] = {
				{ 0x4168, "SERGEANT", 1 },
				{ 0x416c, "PC", 0 },
				{ 0x4170, "SERGEANT", 1 }
			};
			lineError = runtime.playDialogueEntrySequence(
				applicationLines, ARRAYSIZE(applicationLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			responseIndex = 0;
			responseError = runtime.runResponseMenu(kSergeantAmnesiaResponseLine, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1)
				lineError = playSergeantLine(0x417c);
			else if (responseIndex == 2)
				lineError = playSergeantLine(0x4182, 2);
		} else {
			const DialogueLineEntry applicationLines[] = {
				{ 0x4188, "SERGEANT", 0 },
				{ 0x418e, "PC", 0 },
				{ 0x4192, "SERGEANT", 0 }
			};
			lineError = runtime.playDialogueEntrySequence(
				applicationLines, ARRAYSIZE(applicationLines));
		}

		if (lineError.getCode() != Common::kNoError)
			return lineError;
		return runDay5ReminderIfNeeded();
	};
	auto runInitialIntroBranch = [&]() -> Common::Error {
		sharedState.dialogueStateD2f08 = true;
		state.introPending = false;

		const DialogueLineEntry introLines[] = {
			{ 0x413f, "SERGEANT", 2 },
			{ 0x4144, "PC", 0 },
			{ 0x4145, "SERGEANT", 0 },
			{ 0x4146, "PC", 0 },
			{ 0x4148, "SERGEANT", 2 }
		};
		Common::Error lineError = runtime.playDialogueEntrySequence(
			introLines, ARRAYSIZE(introLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(
			kSergeantIntroResponseLine, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1)
			lineError = playSergeantLine(0x4153);
		else if (responseIndex == 2)
			lineError = playSergeantLine(0x4157, 1);

		if (lineError.getCode() != Common::kNoError)
			return lineError;
		return runApplicationDiscussion();
	};
	auto runApplicationReminderBranch = [&]() -> Common::Error {
		Common::Error lineError = playSergeantLine(0x41ac);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (!runtime.startupScript().getFlagValue("HAVE_LODGE_APP")) {
			lineError = playSergeantLine(0x41b0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
		lineError = playSergeantLine(0x41b4, 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(
			kSergeantReminderResponseLine, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playSergeantLine(0x41bf, 2);
			state.dialogueStateD2d50 = true;
		} else if (responseIndex == 2) {
			lineError = playSergeantLine(0x41c4);
		}

		if (lineError.getCode() != Common::kNoError)
			return lineError;
		return runDay5ReminderIfNeeded();
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
	auto handleCompletedFirstTaskBranch = [&]() -> Common::Error {
		markSergeantProgress();

		Common::Error lineError = playSergeantLine(0x420d);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playSergeantLine(0x421e);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(
			kSergeantSecondTaskResponseLine, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1)
			return playSergeantLine(0x422a);

		if (responseIndex == 2) {
			const DialogueLineEntry reluctantTaskLines[] = {
				{ 0x422f, "SERGEANT", 0 },
				{ 0x4239, "SERGEANT", 0 }
			};
			return runtime.playDialogueEntrySequence(
				reluctantTaskLines, ARRAYSIZE(reluctantTaskLines));
		}

		return Common::kNoError;
	};
	auto handleCompletedApplicationBranch = [&]() -> Common::Error {
		sharedState.dialogueStateD2f08 = true;
		sharedState.dialogueStateD2f00 = true;
		state.introPending = false;
		state.dialogueStateD2d50 = true;
		restoreItemToRah("COMPLETED_LODGE_APPLICATION");
		if (!runtime.startupScript().getFlagValue("QUEST_1")) {
			Common::Error lineError = playSergeantLine(0x41cd, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playSergeantLine(0x4292);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			(void)runtime.startupScript().setRuntimeFlagValue("QUEST_1", true);
			return executeActionTagIfSet(kGoDay2ActionTag);
		}
		if (!state.dialogueStateD2d54) {
			Common::Error lineError = playSergeantLine(0x41d2, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(
				kSergeantCompletedApplicationResponseLine, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				const DialogueLineEntry acceptedTaskLines[] = {
					{ 0x41db, "SERGEANT", 0 },
					{ 0x41e2, "PC", 0 },
					{ 0x41e6, "SERGEANT", 0 },
					{ 0x41ec, "PC", 2 },
					{ 0x41f0, "SERGEANT", 0 },
					{ 0x41f6, "PC", 0 },
					{ 0x41fa, "SERGEANT", 0 }
				};
				lineError = runtime.playDialogueEntrySequence(
					acceptedTaskLines, ARRAYSIZE(acceptedTaskLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				state.dialogueStateD2d54 = true;
			} else if (responseIndex == 2) {
				const DialogueLineEntry declinedTaskLines[] = {
					{ 0x4201, "SERGEANT", 3 },
					{ 0x4202, "SERGEANT", 3 },
					{ 0x4203, "SERGEANT", 2 }
				};
				lineError = runtime.playDialogueEntrySequence(
					declinedTaskLines, ARRAYSIZE(declinedTaskLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
				state.dialogueStateD2d54 = false;
			}

			return runDay5ReminderIfNeeded();
		}

		const DialogueLineEntry repeatTaskLines[] = {
			{ 0x41f0, "SERGEANT", 0 },
			{ 0x41f6, "PC", 0 },
			{ 0x41fa, "SERGEANT", 0 }
		};
		Common::Error lineError = runtime.playDialogueEntrySequence(
			repeatTaskLines, ARRAYSIZE(repeatTaskLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		return runDay5ReminderIfNeeded();
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
			return handleCompletedFirstTaskBranch();
		}
		if (runtime.startupScript().getFlagValue("HAVE_COMPLETED_LODGE_APP"))
			return handleCompletedApplicationBranch();
		if (state.introPending)
			return runInitialIntroBranch();
		return runApplicationReminderBranch();
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
