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

} // End of namespace

bool SergeantDialogueHandler::matchesNpc(const Common::String &npcName) const {
	return npcName.equalsIgnoreCase("SERGEANT");
}

Common::Error SergeantDialogueHandler::handleDialogue(DialogueRuntime &runtime,
		const Common::String &usedItemName, DialogueSharedState &sharedState) {
	SergeantRoomDialogueState &state = _state;
	auto playLine = [&](int wavId, const char *speakerId, int headVariant = 0) -> Common::Error {
		return runtime.playDialogueLineWithVariant(wavId, speakerId, headVariant);
	};
	auto playSergeantLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return playLine(wavId, "SERGEANT", headVariant);
	};
	auto playPcLine = [&](int wavId, int headVariant = 0) -> Common::Error {
		return playLine(wavId, "PC", headVariant);
	};
	auto playSequence = [&](const DialogueLineEntry *lines, uint count) -> Common::Error {
		return runtime.playDialogueEntrySequence(lines, count);
	};
	auto hasInventoryItem = [&](const char *objectName) {
		Common::Array<ObjectRecord> inventoryObjects;
		runtime.startupScript().getVisibleInventoryObjects(inventoryObjects);
		for (const ObjectRecord &inventoryObject : inventoryObjects) {
			if (inventoryObject.objectName.equalsIgnoreCase(objectName))
				return true;
		}

		return false;
	};
	auto resetAndSetObjectVisible = [&](const char *ownerOrRoom, const char *objectName, bool visible) {
		(void)runtime.startupScript().resetRuntimeObjectToInitialState(objectName);
		(void)runtime.startupScript().setRuntimeObjectVisible(ownerOrRoom, objectName, visible);
	};
	auto returnItemToRah = [&](const char *objectName, bool visible) {
		resetAndSetObjectVisible(kRahRoomName, objectName, visible);
	};
	auto executeActionTagIfSet = [&](const char *actionTag) -> Common::Error {
		InteractionResult interaction;
		if (!runtime.executeActionTag(actionTag, interaction))
			return Common::kNoError;

		runtime.applyImmediateDialogueInteractionEffects(interaction);
		runtime.queueDialogueInteractionIfNeeded(interaction);
		return Common::kNoError;
	};
	auto queueNpcSlashTransition = [&](const char *npcName) {
		InteractionResult interaction;
		if (runtime.startupScript().finalizeRuntimeNpcDeathOrMonsterfy(
				npcName, kSlashDeathDamageType)) {
			interaction.mutatedRuntimeState = true;
		}
		runtime.queueDialogueInteractionIfNeeded(interaction);
	};
	auto runDay5ExitIfNeeded = [&]() -> Common::Error {
		if (!runtime.startupScript().getFlagValue("DAY_5") || state.day5ExitLinePlayed)
			return Common::kNoError;

		state.day5ExitLinePlayed = true;
		return playSergeantLine(0x4324, 1);
	};
	auto handleRemainsBranch = [&]() -> Common::Error {
		sharedState.dialogueStateD2f08 = true;
		sharedState.dialogueStateD2f00 = true;
		state.talkStateBlock = true;
		state.dialogueStateD2d50 = true;
		state.postApplicationBriefingCompleted = true;
		state.dialogueStateD2d58 = true;
		state.dialogueStateD2d5c = true;
		state.dialogueStateD2d60 = true;
		state.dialogueStateD2d64 = true;
		state.dialogueStateD2d68 = true;
		state.dialogueStateD2d6c = true;
		returnItemToRah("REMAINS", false);

		Common::Error lineError = playSergeantLine(0x4333, 1);
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
		lineError = playSequence(remainsIntroLines, ARRAYSIZE(remainsIntroLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(
			kSergeantRemainsResponseLine, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playSergeantLine(0x435f, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			lineError = playSergeantLine(0x4364);
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
		lineError = playSequence(remainsExitLines, ARRAYSIZE(remainsExitLines));
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
	};
	auto handleInviteBranch = [&]() -> Common::Error {
		sharedState.dialogueStateD2f08 = true;
		sharedState.dialogueStateD2f00 = true;
		state.talkStateBlock = true;
		state.dialogueStateD2d50 = true;
		state.postApplicationBriefingCompleted = true;
		state.dialogueStateD2d58 = true;
		state.dialogueStateD2d5c = true;
		state.dialogueStateD2d60 = true;
		state.dialogueStateD2d64 = true;
		state.dialogueStateD2d68 = true;
		resetAndSetObjectVisible("ST_BEDRM", "STEFSKULL", true);
		returnItemToRah("INVITE", false);
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
		lineError = playSequence(inviteLines, ARRAYSIZE(inviteLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(
			kSergeantInviteResponseLine, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playSergeantLine(0x4314, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			lineError = playSergeantLine(0x4319, 2);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		lineError = playSergeantLine(0x431d);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		return runDay5ExitIfNeeded();
	};
	auto handleBarberPoleBranch = [&]() -> Common::Error {
		sharedState.dialogueStateD2f08 = true;
		sharedState.dialogueStateD2f00 = true;
		state.talkStateBlock = true;
		state.dialogueStateD2d50 = true;
		state.postApplicationBriefingCompleted = true;
		state.dialogueStateD2d58 = true;
		state.dialogueStateD2d5c = true;
		state.dialogueStateD2d60 = true;
		returnItemToRah("BARBER_POLE", false);
		(void)runtime.startupScript().setRuntimeFlagValue("ASSIGNED_DNA_TASK", true);

		Common::Error lineError = playSergeantLine(0x426f, 1);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playSergeantLine(0x4274);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(
			kSergeantBarberPoleResponseLine, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playSergeantLine(0x4281);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			lineError = playSergeantLine(0x4289);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		lineError = playSergeantLine(0x428d);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		lineError = playSergeantLine(0x4299);
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		responseIndex = 0;
		responseError = runtime.runResponseMenu(
			kSergeantBarberPoleFollowupResponseLine, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playSergeantLine(0x42a4, 3);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			if (runtime.startupScript().getFlagValue("PC_KILLED_ANYONE")) {
				lineError = playSergeantLine(0x42a8);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}
		} else if (responseIndex == 2) {
			lineError = playSergeantLine(0x42ad);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		return runDay5ExitIfNeeded();
	};
	auto handleBoltClothBranch = [&]() -> Common::Error {
		sharedState.dialogueStateD2f08 = true;
		sharedState.dialogueStateD2f00 = true;
		state.talkStateBlock = true;
		state.dialogueStateD2d50 = true;
		state.postApplicationBriefingCompleted = true;
		state.dialogueStateD2d58 = true;
		state.dialogueStateD2d5c = true;

		const DialogueLineEntry boltClothLines[] = {
			{ 0x4241, "SERGEANT", 1 },
			{ 0x4245, "SERGEANT", 1 },
			{ 0x4252, "SERGEANT", 0 }
		};
		Common::Error lineError = playSequence(boltClothLines, ARRAYSIZE(boltClothLines));
		if (lineError.getCode() != Common::kNoError)
			return lineError;

		int responseIndex = 0;
		Common::Error responseError = runtime.runResponseMenu(
			kSergeantBoltClothResponseLine, responseIndex);
		if (responseError.getCode() != Common::kNoError)
			return responseError;

		if (responseIndex == 1) {
			lineError = playSergeantLine(0x425e, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			lineError = playSergeantLine(0x4266, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		returnItemToRah("BOLTCLTH", false);
		(void)runtime.startupScript().setRuntimeFlagValue("FIREMEN_SUICIDE", true);
		(void)runtime.startupScript().setRuntimeFlagValue("ASSIGNED_POLE_TASK", true);
		queueNpcSlashTransition("SPARKY");
		queueNpcSlashTransition("NUDE_MAN");
		queueNpcSlashTransition("FIREMAN2");
		return runDay5ExitIfNeeded();
	};
	auto handleCompletedFirstTaskBranch = [&]() -> Common::Error {
		sharedState.dialogueStateD2f08 = true;
		sharedState.dialogueStateD2f00 = true;
		sharedState.sergeantCompletedFirstTaskState = 1;
		state.talkStateBlock = true;
		state.dialogueStateD2d50 = true;
		state.postApplicationBriefingCompleted = true;
		state.completedFirstTaskState = true;

		Common::Error lineError = playSergeantLine(0x420d, 1);
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

		if (responseIndex == 1) {
			lineError = playSergeantLine(0x422a);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		} else if (responseIndex == 2) {
			const DialogueLineEntry reluctantTaskLines[] = {
				{ 0x422f, "SERGEANT", 2 },
				{ 0x4239, "SERGEANT", 3 }
			};
			lineError = playSequence(
				reluctantTaskLines, ARRAYSIZE(reluctantTaskLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		if (!state.dialogueStateD2d58) {
			state.dialogueStateD2d58 = true;
			Common::Error actionError = executeActionTagIfSet(kGoDay3ActionTag);
			if (actionError.getCode() != Common::kNoError)
				return actionError;
		}
		return runDay5ExitIfNeeded();
	};
	auto handleCompletedApplicationBranch = [&]() -> Common::Error {
		sharedState.dialogueStateD2f08 = true;
		sharedState.dialogueStateD2f00 = true;
		state.talkStateBlock = true;
		state.dialogueStateD2d50 = true;
		returnItemToRah("COMPLETED_LODGE_APPLICATION", false);
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
		if (!state.postApplicationBriefingCompleted) {
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
				state.postApplicationBriefingCompleted = true;
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
				state.postApplicationBriefingCompleted = false;
			}

			return runDay5ExitIfNeeded();
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
		return runDay5ExitIfNeeded();
	};
	auto runIntroOrReminderBranch = [&]() -> Common::Error {
		if (!state.talkStateBlock) {
			sharedState.dialogueStateD2f08 = true;
			state.talkStateBlock = true;

			const DialogueLineEntry introLines[] = {
				{ 0x413f, "SERGEANT", 2 },
				{ 0x4144, "PC", 0 },
				{ 0x4145, "SERGEANT", 0 },
				{ 0x4146, "PC", 0 },
				{ 0x4148, "SERGEANT", 2 }
			};
			Common::Error lineError = playSequence(introLines, ARRAYSIZE(introLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(
				kSergeantIntroResponseLine, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = playSergeantLine(0x4153);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else if (responseIndex == 2) {
				lineError = playSergeantLine(0x4157, 1);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			lineError = playSergeantLine(0x415d);
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
				lineError = playSequence(applicationLines, ARRAYSIZE(applicationLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;

				responseIndex = 0;
				responseError = runtime.runResponseMenu(
					kSergeantAmnesiaResponseLine, responseIndex);
				if (responseError.getCode() != Common::kNoError)
					return responseError;

				if (responseIndex == 1) {
					lineError = playSergeantLine(0x417c);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				} else if (responseIndex == 2) {
					lineError = playSergeantLine(0x4182, 2);
					if (lineError.getCode() != Common::kNoError)
						return lineError;
				}
			} else {
				const DialogueLineEntry applicationLines[] = {
					{ 0x4188, "SERGEANT", 0 },
					{ 0x418e, "PC", 0 },
					{ 0x4192, "SERGEANT", 0 }
				};
				lineError = playSequence(applicationLines, ARRAYSIZE(applicationLines));
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			return runDay5ExitIfNeeded();
		}

		state.talkStateBlock = true;
		Common::Error lineError = playSergeantLine(0x41ac);
		if (lineError.getCode() != Common::kNoError)
			return lineError;
		if (!runtime.startupScript().getFlagValue("HAVE_LODGE_APP")) {
			lineError = playPcLine(0x41b0);
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
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			state.dialogueStateD2d50 = true;
		} else if (responseIndex == 2) {
			lineError = playSergeantLine(0x41c4);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
		}

		return runDay5ExitIfNeeded();
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
			sharedState.dialogueStateD2f08 = true;
			sharedState.dialogueStateD2f00 = true;
			state.talkStateBlock = true;
			state.dialogueStateD2d50 = true;
			state.postApplicationBriefingCompleted = true;
			state.dialogueStateD2d58 = true;
			state.dialogueStateD2d5c = true;
			state.dialogueStateD2d60 = true;
			state.dialogueStateD2d64 = true;

			Common::Error lineError = playSergeantLine(0x42b5, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;
			lineError = playSergeantLine(0x42b9);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			int responseIndex = 0;
			Common::Error responseError = runtime.runResponseMenu(
				kSergeantDinerBurnedResponseLine, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = playSergeantLine(0x42c5);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else if (responseIndex == 2) {
				lineError = playSergeantLine(0x42cb);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			lineError = playSergeantLine(0x42d2, 1);
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			responseIndex = 0;
			responseError = runtime.runResponseMenu(
				kSergeantDinerBurnedFollowupResponseLine, responseIndex);
			if (responseError.getCode() != Common::kNoError)
				return responseError;

			if (responseIndex == 1) {
				lineError = playSergeantLine(0x42de, 2);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			} else if (responseIndex == 2) {
				lineError = playSergeantLine(0x42e3);
				if (lineError.getCode() != Common::kNoError)
					return lineError;
			}

			const DialogueLineEntry dinerBurnedTailLines[] = {
				{ 0x42e9, "SERGEANT", 0 },
				{ 0x42ef, "PC", 0 },
				{ 0x42f3, "SERGEANT", 0 }
			};
			lineError = playSequence(dinerBurnedTailLines, ARRAYSIZE(dinerBurnedTailLines));
			if (lineError.getCode() != Common::kNoError)
				return lineError;

			resetAndSetObjectVisible("ST_BEDRM", "INVITE", true);
			resetAndSetObjectVisible("ST_BEDRM", "STEFSKULL", true);
			(void)runtime.startupScript().setRuntimeNpcState("DWAYNE_ST_BEDRM", true, true);
			(void)runtime.startupScript().setRuntimeFlagValue("STEPHANIE_IS_DEAD", true);
			(void)runtime.startupScript().setRuntimeFlagValue(
				"STEPHANIE_DEAD_IN_HALL_JUST_OUTSIDE", true);
			return runDay5ExitIfNeeded();
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
		return runIntroOrReminderBranch();
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
