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


#include "harvester/npc/sergeant_dialogue.h"

#include "common/array.h"
#include "harvester/npc/dialogue_runtime.h"

namespace Harvester {

namespace {

static const char *const kDialogueC149FstPath = "GRAPHIC/FST/C149.FST";
static const char *const kDialogueC150FstPath = "GRAPHIC/FST/C150.FST";
static const char *const kDialogueC076FstPath = "GRAPHIC/FST/C076.FST";
static const char *const kGoDay2ActionTag = "GO_DAY_2";
static const char *const kGoDay3ActionTag = "GO_DAY_3";
static const char *const kRahRoomName = "RAH";
static const int kSlashDeathDamageType = 2;
static const int kSergeantRemainsResponseLine = 0x2ad;
static const int kSergeantInviteResponseLine = 0x2ae;
static const int kSergeantDinerBurnedResponseLine = 0x2af;
static const int kSergeantDinerBurnedFollowupResponseLine = 0x2b0;
static const int kSergeantBarberPoleResponseLine = 0x2b1;
static const int kSergeantBarberPoleFollowupResponseLine = 0x2b2;
static const int kSergeantBoltClothResponseLine = 0x2b3;
static const int kSergeantSecondTaskResponseLine = 0x2b4;
static const int kSergeantCompletedApplicationResponseLine = 0x2b6;
static const int kSergeantIntroResponseLine = 0x2b7;
static const int kSergeantApplicationResponseLine = 0x2b8;
static const int kSergeantAmnesiaResponseLine = 0x2b9;
static const int kSergeantReminderResponseLine = 0x2bb;

static void resetAndSetObjectVisible(DialogueRuntime &runtime, const char *ownerOrRoom,
		const char *objectName, bool visible) {
	(void)runtime.startupScript().resetRuntimeObjectToInitialState(objectName);
	(void)runtime.startupScript().setRuntimeObjectVisible(ownerOrRoom, objectName, visible);
}

static void returnItemToRah(DialogueRuntime &runtime, const char *objectName, bool visible) {
	resetAndSetObjectVisible(runtime, kRahRoomName, objectName, visible);
}

static Common::Error executeActionTagIfSet(DialogueRuntime &runtime, const char *actionTag) {
	runtime.executeAndQueueAction(actionTag);
	return Common::kNoError;
}

static void queueNpcSlashTransition(DialogueRuntime &runtime, const char *npcName) {
	InteractionResult interaction;
	if (runtime.startupScript().finalizeRuntimeNpcDeathOrMonsterfy(
			npcName, kSlashDeathDamageType)) {
		interaction.mutatedRuntimeState = true;
	}
	runtime.queueDialogueInteractionIfNeeded(interaction);
}

} // End of namespace

Common::Error SergeantDialogueHandler::runDay5ExitIfNeeded(DialogueRuntime &runtime) {
	if (!runtime.startupScript().getFlagValue("DAY_5") || _state.day5ExitLinePlayed)
		return Common::kNoError;

	_state.day5ExitLinePlayed = true;
	return runtime.playDialogueLineWithVariant(0x4324, "SERGEANT", 1);
}

Common::Error SergeantDialogueHandler::handleRemainsBranch(DialogueRuntime &runtime,
		DialogueSharedState &sharedState) {
	sharedState.dialogueStateD2f08 = true;
	sharedState.dialogueStateD2f00 = true;
	_state.talkStateBlock = true;
	_state.dialogueStateD2d50 = true;
	_state.postApplicationBriefingCompleted = true;
	_state.dialogueStateD2d58 = true;
	_state.dialogueStateD2d5c = true;
	_state.dialogueStateD2d60 = true;
	_state.dialogueStateD2d64 = true;
	_state.dialogueStateD2d68 = true;
	_state.dialogueStateD2d6c = true;
	returnItemToRah(runtime, "REMAINS", false);

	Common::Error lineError = runtime.playDialogueLineWithVariant(0x4333, "SERGEANT", 1);
	if (lineError.getCode() != Common::kNoError)
		return lineError;
	lineError = runtime.playDialogueFst(kDialogueC150FstPath);
	if (lineError.getCode() != Common::kNoError)
		return lineError;
	const DialogueLineEntry remainsIntroLines[] = {
		{ 0x433f, "SERGEANT", 0 },
		{ 0x4344, "PC", 2 },
		{ 0x4348, "SERGEANT", 0 },
		{ 0x434f, "PC", 0 },
		{ 0x4353, "SERGEANT", 1 }
	};
	lineError = runtime.playDialogueEntrySequence(remainsIntroLines, ARRAYSIZE(remainsIntroLines));
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	int responseIndex = 0;
	Common::Error responseError = runtime.runResponseMenu(
		kSergeantRemainsResponseLine, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 1) {
		lineError = runtime.playDialogueLineWithVariant(0x435f, "SERGEANT", 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	} else if (responseIndex == 2) {
		lineError = runtime.playDialogueLineWithVariant(0x4364, "SERGEANT", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	const DialogueLineEntry remainsExitLines[] = {
		{ 0x4368, "SERGEANT", 0 },
		{ 0x4372, "PC", 2 },
		{ 0x4376, "SERGEANT", 0 },
		{ 0x4381, "PC", 2 },
		{ 0x4385, "SERGEANT", 0 }
	};
	lineError = runtime.playDialogueEntrySequence(remainsExitLines, ARRAYSIZE(remainsExitLines));
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	lineError = runtime.clearScreenToBlack();
	if (lineError.getCode() != Common::kNoError)
		return lineError;
	lineError = runtime.playDialogueFst(kDialogueC076FstPath);
	if (lineError.getCode() != Common::kNoError)
		return lineError;
	lineError = runtime.showCdChangePrompt(3);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	// Native prompt_for_cdrom_disc_change(3, 0) latches a main-loop room rebuild.
	InteractionResult interaction;
	interaction.requestRoomRestart = true;
	interaction.roomRestartTargetName = "START";
	runtime.queueDialogueInteractionIfNeeded(interaction);
	return Common::kNoError;
}

Common::Error SergeantDialogueHandler::handleInviteBranch(DialogueRuntime &runtime,
		DialogueSharedState &sharedState) {
	sharedState.dialogueStateD2f08 = true;
	sharedState.dialogueStateD2f00 = true;
	_state.talkStateBlock = true;
	_state.dialogueStateD2d50 = true;
	_state.postApplicationBriefingCompleted = true;
	_state.dialogueStateD2d58 = true;
	_state.dialogueStateD2d5c = true;
	_state.dialogueStateD2d60 = true;
	_state.dialogueStateD2d64 = true;
	_state.dialogueStateD2d68 = true;
	resetAndSetObjectVisible(runtime, "ST_BEDRM", "STEFSKULL", true);
	returnItemToRah(runtime, "INVITE", false);
	(void)runtime.startupScript().setRuntimeFlagValue("TAKEN_INVITE_TO_SERGEANT", true);
	(void)runtime.startupScript().setRuntimeFlagValue("NEED_REMAINS_FOR_LODGE", true);
	(void)runtime.startupScript().setRuntimeNpcState("DWAYNE_ST_BEDRM", false, false);
	(void)runtime.startupScript().setRuntimeNpcState("MRS_POTTS_ST_BEDRM", false, false);
	(void)runtime.startupScript().setRuntimeNpcState("MRS_POTTS", true, true);

	Common::Error lineError = runtime.playDialogueFst(kDialogueC149FstPath);
	if (lineError.getCode() != Common::kNoError)
		return lineError;
	const DialogueLineEntry inviteLines[] = {
		{ 0x4305, "PC", 0 },
		{ 0x4309, "SERGEANT", 2 }
	};
	lineError = runtime.playDialogueEntrySequence(inviteLines, ARRAYSIZE(inviteLines));
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	int responseIndex = 0;
	Common::Error responseError = runtime.runResponseMenu(
		kSergeantInviteResponseLine, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 1) {
		lineError = runtime.playDialogueLineWithVariant(0x4314, "SERGEANT", 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	} else if (responseIndex == 2) {
		lineError = runtime.playDialogueLineWithVariant(0x4319, "SERGEANT", 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	lineError = runtime.playDialogueLineWithVariant(0x431d, "SERGEANT", 0);
	if (lineError.getCode() != Common::kNoError)
		return lineError;
	return runDay5ExitIfNeeded(runtime);
}

Common::Error SergeantDialogueHandler::handleBarberPoleBranch(DialogueRuntime &runtime,
		DialogueSharedState &sharedState) {
	sharedState.dialogueStateD2f08 = true;
	sharedState.dialogueStateD2f00 = true;
	_state.talkStateBlock = true;
	_state.dialogueStateD2d50 = true;
	_state.postApplicationBriefingCompleted = true;
	_state.dialogueStateD2d58 = true;
	_state.dialogueStateD2d5c = true;
	_state.dialogueStateD2d60 = true;
	returnItemToRah(runtime, "BARBER_POLE", false);
	(void)runtime.startupScript().setRuntimeFlagValue("ASSIGNED_DNA_TASK", true);

	Common::Error lineError = runtime.playDialogueLineWithVariant(0x426f, "SERGEANT", 1);
	if (lineError.getCode() != Common::kNoError)
		return lineError;
	lineError = runtime.playDialogueLineWithVariant(0x4274, "SERGEANT", 0);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	int responseIndex = 0;
	Common::Error responseError = runtime.runResponseMenu(
		kSergeantBarberPoleResponseLine, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 1) {
		lineError = runtime.playDialogueLineWithVariant(0x4281, "SERGEANT", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	} else if (responseIndex == 2) {
		lineError = runtime.playDialogueLineWithVariant(0x4289, "SERGEANT", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	lineError = runtime.playDialogueLineWithVariant(0x428d, "SERGEANT", 0);
	if (lineError.getCode() != Common::kNoError)
		return lineError;
	lineError = runtime.playDialogueLineWithVariant(0x4299, "SERGEANT", 0);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	responseIndex = 0;
	responseError = runtime.runResponseMenu(
		kSergeantBarberPoleFollowupResponseLine, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 1) {
		lineError = runtime.playDialogueLineWithVariant(0x42a4, "SERGEANT", 3);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (runtime.startupScript().getFlagValue("PC_KILLED_ANYONE")) {
			lineError = runtime.playDialogueLineWithVariant(0x42a8, "SERGEANT", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}
	} else if (responseIndex == 2) {
		lineError = runtime.playDialogueLineWithVariant(0x42ad, "SERGEANT", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	return runDay5ExitIfNeeded(runtime);
}

Common::Error SergeantDialogueHandler::handleBoltClothBranch(DialogueRuntime &runtime,
		DialogueSharedState &sharedState) {
	sharedState.dialogueStateD2f08 = true;
	sharedState.dialogueStateD2f00 = true;
	_state.talkStateBlock = true;
	_state.dialogueStateD2d50 = true;
	_state.postApplicationBriefingCompleted = true;
	_state.dialogueStateD2d58 = true;
	_state.dialogueStateD2d5c = true;

	const DialogueLineEntry boltClothLines[] = {
		{ 0x4241, "SERGEANT", 1 },
		{ 0x4245, "SERGEANT", 1 },
		{ 0x4252, "SERGEANT", 0 }
	};
	Common::Error lineError = runtime.playDialogueEntrySequence(boltClothLines, ARRAYSIZE(boltClothLines));
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	int responseIndex = 0;
	Common::Error responseError = runtime.runResponseMenu(
		kSergeantBoltClothResponseLine, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 1) {
		lineError = runtime.playDialogueLineWithVariant(0x425e, "SERGEANT", 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	} else if (responseIndex == 2) {
		lineError = runtime.playDialogueLineWithVariant(0x4266, "SERGEANT", 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	returnItemToRah(runtime, "BOLTCLTH", false);
	(void)runtime.startupScript().setRuntimeFlagValue("FIREMEN_SUICIDE", true);
	(void)runtime.startupScript().setRuntimeFlagValue("ASSIGNED_POLE_TASK", true);
	queueNpcSlashTransition(runtime, "SPARKY");
	queueNpcSlashTransition(runtime, "NUDE_MAN");
	queueNpcSlashTransition(runtime, "FIREMAN2");
	return runDay5ExitIfNeeded(runtime);
}

Common::Error SergeantDialogueHandler::handleCompletedFirstTaskBranch(DialogueRuntime &runtime,
		DialogueSharedState &sharedState) {
	sharedState.dialogueStateD2f08 = true;
	sharedState.dialogueStateD2f00 = true;
	sharedState.sergeantCompletedFirstTaskState = 1;
	_state.talkStateBlock = true;
	_state.dialogueStateD2d50 = true;
	_state.postApplicationBriefingCompleted = true;
	_state.completedFirstTaskState = true;

	Common::Error lineError = runtime.playDialogueLineWithVariant(0x420d, "SERGEANT", 1);
	if (lineError.getCode() != Common::kNoError)
		return lineError;
	lineError = runtime.playDialogueLineWithVariant(0x421e, "SERGEANT", 0);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	int responseIndex = 0;
	Common::Error responseError = runtime.runResponseMenu(
		kSergeantSecondTaskResponseLine, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 1) {
		lineError = runtime.playDialogueLineWithVariant(0x422a, "SERGEANT", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	} else if (responseIndex == 2) {
		const DialogueLineEntry reluctantTaskLines[] = {
			{ 0x422f, "SERGEANT", 2 },
			{ 0x4239, "SERGEANT", 3 }
		};
		lineError = runtime.playDialogueEntrySequence(reluctantTaskLines, ARRAYSIZE(reluctantTaskLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	if (!_state.dialogueStateD2d58) {
		_state.dialogueStateD2d58 = true;
		Common::Error actionError = executeActionTagIfSet(runtime, kGoDay3ActionTag);
		if (actionError.getCode() != Common::kNoError)
			return actionError;
	}
	return runDay5ExitIfNeeded(runtime);
}

Common::Error SergeantDialogueHandler::handleCompletedApplicationBranch(DialogueRuntime &runtime,
		DialogueSharedState &sharedState) {
	sharedState.dialogueStateD2f08 = true;
	sharedState.dialogueStateD2f00 = true;
	_state.talkStateBlock = true;
	_state.dialogueStateD2d50 = true;
	returnItemToRah(runtime, "COMPLETED_LODGE_APPLICATION", false);
	if (!runtime.startupScript().getFlagValue("QUEST_1")) {
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x41cd, "SERGEANT", 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = runtime.playDialogueLineWithVariant(0x4292, "SERGEANT", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		(void)runtime.startupScript().setRuntimeFlagValue("QUEST_1", true);
		return executeActionTagIfSet(runtime, kGoDay2ActionTag);
	}
	if (!_state.postApplicationBriefingCompleted) {
		Common::Error lineError = runtime.playDialogueLineWithVariant(0x41d2, "SERGEANT", 1);
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
			_state.postApplicationBriefingCompleted = true;
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
			_state.postApplicationBriefingCompleted = false;
		}

		return runDay5ExitIfNeeded(runtime);
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
	return runDay5ExitIfNeeded(runtime);
}

Common::Error SergeantDialogueHandler::runIntroOrReminderBranch(DialogueRuntime &runtime,
		DialogueSharedState &sharedState) {
	if (!_state.talkStateBlock) {
		sharedState.dialogueStateD2f08 = true;
		_state.talkStateBlock = true;

		const DialogueLineEntry introLines[] = {
			{ 0x413f, "SERGEANT", 2 },
			{ 0x4144, "PC", 0 },
			{ 0x4145, "SERGEANT", 0 },
			{ 0x4146, "PC", 0 },
			{ 0x4148, "SERGEANT", 2 }
		};
		Common::Error lineError = runtime.playDialogueEntrySequence(introLines, ARRAYSIZE(introLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(
			kSergeantIntroResponseLine, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = runtime.playDialogueLineWithVariant(0x4153, "SERGEANT", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			lineError = runtime.playDialogueLineWithVariant(0x4157, "SERGEANT", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		lineError = runtime.playDialogueLineWithVariant(0x415d, "SERGEANT", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		responseIndex = 0;
		responseError = runtime.runResponseMenu(
			kSergeantApplicationResponseLine, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			const DialogueLineEntry applicationLines[] = {
				{ 0x4168, "SERGEANT", 1 },
				{ 0x416c, "PC", 0 },
				{ 0x4170, "SERGEANT", 1 }
			};
			lineError = runtime.playDialogueEntrySequence(applicationLines, ARRAYSIZE(applicationLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			responseIndex = 0;
			responseError = runtime.runResponseMenu(
				kSergeantAmnesiaResponseLine, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = runtime.playDialogueLineWithVariant(0x417c, "SERGEANT", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else if (responseIndex == 2) {
				lineError = runtime.playDialogueLineWithVariant(0x4182, "SERGEANT", 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
		} else {
			const DialogueLineEntry applicationLines[] = {
				{ 0x4188, "SERGEANT", 0 },
				{ 0x418e, "PC", 0 },
				{ 0x4192, "SERGEANT", 0 }
			};
			lineError = runtime.playDialogueEntrySequence(applicationLines, ARRAYSIZE(applicationLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		return runDay5ExitIfNeeded(runtime);
	}

	_state.talkStateBlock = true;
	Common::Error lineError = runtime.playDialogueLineWithVariant(0x41ac, "SERGEANT", 0);
	if (lineError.getCode() != Common::kNoError)
		return lineError;
	if (!runtime.startupScript().getFlagValue("HAVE_LODGE_APP")) {
		lineError = runtime.playDialogueLineWithVariant(0x41b0, "PC", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}
	lineError = runtime.playDialogueLineWithVariant(0x41b4, "SERGEANT", 3);
	if (lineError.getCode() != Common::kNoError)
		return lineError;

	int responseIndex = 0;
	Common::Error responseError = runtime.runResponseMenu(
		kSergeantReminderResponseLine, responseIndex);
	if (responseError.getCode() != Common::kNoError)
		return responseError;

	if (responseIndex == 1) {
		lineError = runtime.playDialogueLineWithVariant(0x41bf, "SERGEANT", 2);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		_state.dialogueStateD2d50 = true;
	} else if (responseIndex == 2) {
		lineError = runtime.playDialogueLineWithVariant(0x41c4, "SERGEANT", 0);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
	}

	return runDay5ExitIfNeeded(runtime);
}

Common::Error SergeantDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	SergeantRoomDialogueState &state = _state;

	auto hasInventoryItem = [&](const char *objectName) {
		Common::Array<ObjectRecord> inventoryObjects;
		runtime.startupScript().getVisibleInventoryObjects(inventoryObjects);
		for (const ObjectRecord &inventoryObject : inventoryObjects) {
			if (inventoryObject.objectName.equalsIgnoreCase(objectName))
				return true;
		}

		return false;
	};

	if (usedItemName.empty()) {
		if (runtime.startupScript().getFlagValue("GOT_REMAINS_FOR_LODGE"))
			return handleRemainsBranch(runtime, sharedState);
		if (runtime.startupScript().getFlagValue("STEPHANIE_IS_DEAD") &&
				(hasInventoryItem("INVITE") ||
					runtime.startupScript().getFlagValue("TAKEN_INVITE_TO_SERGEANT"))) {
			return handleInviteBranch(runtime, sharedState);
		}
		if (runtime.startupScript().getFlagValue("DINER_BURNED")) {
			sharedState.dialogueStateD2f08 = true;
			sharedState.dialogueStateD2f00 = true;
			state.talkStateBlock = true;
			state.dialogueStateD2d50 = true;
			state.postApplicationBriefingCompleted = true;
			state.dialogueStateD2d58 = true;
			state.dialogueStateD2d5c = true;
			state.dialogueStateD2d60 = true;
			state.dialogueStateD2d64 = true;

			Common::Error lineError = runtime.playDialogueLineWithVariant(0x42b5, "SERGEANT", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = runtime.playDialogueLineWithVariant(0x42b9, "SERGEANT", 0);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(
				kSergeantDinerBurnedResponseLine, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = runtime.playDialogueLineWithVariant(0x42c5, "SERGEANT", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else if (responseIndex == 2) {
				lineError = runtime.playDialogueLineWithVariant(0x42cb, "SERGEANT", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			lineError = runtime.playDialogueLineWithVariant(0x42d2, "SERGEANT", 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			responseIndex = 0;
			responseError = runtime.runResponseMenu(
				kSergeantDinerBurnedFollowupResponseLine, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = runtime.playDialogueLineWithVariant(0x42de, "SERGEANT", 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else if (responseIndex == 2) {
				lineError = runtime.playDialogueLineWithVariant(0x42e3, "SERGEANT", 0);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			const DialogueLineEntry dinerBurnedTailLines[] = {
				{ 0x42e9, "SERGEANT", 0 },
				{ 0x42ef, "PC", 0 },
				{ 0x42f3, "SERGEANT", 0 }
			};
			lineError = runtime.playDialogueEntrySequence(dinerBurnedTailLines, ARRAYSIZE(dinerBurnedTailLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			resetAndSetObjectVisible(runtime, "ST_BEDRM", "INVITE", true);
			resetAndSetObjectVisible(runtime, "ST_BEDRM", "STEFSKULL", true);
			(void)runtime.startupScript().setRuntimeNpcState("DWAYNE_ST_BEDRM", true, true);
			(void)runtime.startupScript().setRuntimeFlagValue("STEPHANIE_IS_DEAD", true);
			(void)runtime.startupScript().setRuntimeFlagValue(
				"STEPHANIE_DEAD_IN_HALL_JUST_OUTSIDE", true);
			return runDay5ExitIfNeeded(runtime);
		}
		if (runtime.startupScript().getFlagValue("BARBER_POLE_STOLEN"))
			return handleBarberPoleBranch(runtime, sharedState);
		if (runtime.startupScript().getFlagValue("BOLT_OF_CLOTH_TAKEN"))
			return handleBoltClothBranch(runtime, sharedState);
		if (runtime.startupScript().getFlagValue("SCRATCHED_TUCKER")) {
			return handleCompletedFirstTaskBranch(runtime, sharedState);
		}
		if (runtime.startupScript().getFlagValue("HAVE_COMPLETED_LODGE_APP"))
			return handleCompletedApplicationBranch(runtime, sharedState);
		return runIntroOrReminderBranch(runtime, sharedState);
	}

	if (usedItemName.equalsIgnoreCase("BARBER_POLE"))
		return handleBarberPoleBranch(runtime, sharedState);
	if (usedItemName.equalsIgnoreCase("BOLTCLTH"))
		return handleBoltClothBranch(runtime, sharedState);
	if (usedItemName.equalsIgnoreCase("COMPLETED_LODGE_APPLICATION"))
		return handleCompletedApplicationBranch(runtime, sharedState);
	if (usedItemName.equalsIgnoreCase("INVITE"))
		return handleInviteBranch(runtime, sharedState);
	if (usedItemName.equalsIgnoreCase("REMAINS"))
		return handleRemainsBranch(runtime, sharedState);

	return runtime.playDialogueLineWithVariant(0x42fa, "SERGEANT", 0);
}

} // End of namespace Harvester
