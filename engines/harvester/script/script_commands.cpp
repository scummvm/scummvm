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

#include "harvester/script/script.h"

#include "common/algorithm.h"
#include "common/debug.h"
#include "harvester/detection.h"
#include "harvester/harvester.h"
#include "harvester/parse_utils.h"

namespace Harvester {

static int resolveCombatResourceLoadoutForOpcode(const Common::String &opcodeName) {
	// Confirmed from dispatch_room_event_actions and HARVEST.SCR.
	if (opcodeName.equalsIgnoreCase("ADD_NAILS"))
		return 2;
	if (opcodeName.equalsIgnoreCase("ADD_SHOTGUN_SHELLS"))
		return 3;
	if (opcodeName.equalsIgnoreCase("ADD_GASCANS"))
		return 14;

	return 0;
}

static bool isCombatResourceOpcode(const Common::String &opcodeName) {
	return resolveCombatResourceLoadoutForOpcode(opcodeName) != 0;
}

static bool appendStartupAudioCommand(const CommandRecord &command, Common::Array<AudioCommand> &commands) {
	if (command.opcodeName.equalsIgnoreCase("START_WAV")) {
		AudioCommand audioCommand;
		audioCommand.type = kStartupAudioCommandStartWav;
		audioCommand.path = command.arg1;
		commands.push_back(Common::move(audioCommand));
		return true;
	}

	if (command.opcodeName.equalsIgnoreCase("START_SINGLE_WAV")) {
		AudioCommand audioCommand;
		audioCommand.type = kStartupAudioCommandStartSingleWav;
		audioCommand.path = command.arg1;
		commands.push_back(Common::move(audioCommand));
		return true;
	}

	if (command.opcodeName.equalsIgnoreCase("LOAD_WAV")) {
		AudioCommand audioCommand;
		audioCommand.type = kStartupAudioCommandLoadWav;
		audioCommand.path = command.arg1;
		audioCommand.slot = command.arg2.empty() ? -1 : parseAsciiIntOrZero(command.arg2);
		commands.push_back(Common::move(audioCommand));
		return true;
	}

	if (command.opcodeName.equalsIgnoreCase("PLAY_WAV")) {
		AudioCommand audioCommand;
		audioCommand.type = kStartupAudioCommandPlayWav;
		audioCommand.slot = command.arg1.empty() ? -1 : parseAsciiIntOrZero(command.arg1);
		commands.push_back(Common::move(audioCommand));
		return true;
	}

	if (command.opcodeName.equalsIgnoreCase("DELETE_WAV")) {
		AudioCommand audioCommand;
		audioCommand.type = kStartupAudioCommandDeleteWav;
		audioCommand.slot = command.arg1.empty() ? -1 : parseAsciiIntOrZero(command.arg1);
		commands.push_back(Common::move(audioCommand));
		return true;
	}

	return false;
}

static bool isTruthy(const Common::String &value) {
	return value.equalsIgnoreCase("T") || value.equalsIgnoreCase("ON") || value.equalsIgnoreCase("TRUE");
}

static void noteMutation(InteractionResult &result, bool changed) {
	if (changed)
		result.mutatedRuntimeState = true;
}

static void noteCurrentRoomVisualMutation(InteractionResult &result, bool changed, bool affectsCurrentRoom) {
	noteMutation(result, changed);
	if (changed && affectsCurrentRoom)
		result.visualRuntimeStateChanged = true;
}

static void noteTimerMutation(InteractionResult &result, const TimerRecord *previousTimer, bool changed) {
	if (!changed || !previousTimer || previousTimer->timerName.empty())
		return;

	for (const TimerRecord &record : result.previousTimerRecords) {
		if (record.timerName.equalsIgnoreCase(previousTimer->timerName))
			return;
	}

	result.previousTimerRecords.push_back(*previousTimer);
}

static bool belongsToContextRoom(const Common::String &contextRoom, const Common::String &roomName) {
	return !contextRoom.empty() && roomName.equalsIgnoreCase(contextRoom);
}

bool Script::hasDeferredInteractionOutputs(const InteractionResult &result, CommandOutputPolicy outputPolicy) {
	return (outputPolicy == kCommandOutputsInteraction &&
			(!result.nextRoomName.empty() || !result.deathFlicPath.empty() || result.requestMainMenu)) ||
		!result.cutscenePath.empty() || result.requestDemoEnding ||
		!result.dialogueNpcName.empty() || !result.continuationTag.empty() ||
		!result.modalText.value.empty() || result.lightingCommand != kStartupLightingCommandNone ||
		!result.moveEntityToPlayerZName.empty() || result.requestCloseupExit;
}

Script::CommandStepResult Script::executeBranchCommand(const CommandRecord &command,
		const CommandExecutionContext &context, InteractionResult &result, Common::String &nextTag) {
	if (command.opcodeName.equalsIgnoreCase("CHECK_FLAG")) {
		const FlagRecord *flag = findRuntimeFlag(command.arg1);
		const bool flagValue = flag && flag->value;
		debugC(3, kDebugScene, "Harvester: %s '%s' flag '%s' -> %d",
			context.label, context.name.c_str(), command.arg1.c_str(), flagValue);
		nextTag = flagValue ? command.arg2 : command.arg3;
		return kCommandContinue;
	}

	if (command.opcodeName.equalsIgnoreCase("CHECK_PERC")) {
		const int threshold = CLIP<int>(parseAsciiIntOrZero(command.arg1), 0, 100);
		const int roll = g_engine ? static_cast<int>(g_engine->getRandomNumber(99)) : 0;
		nextTag = roll < threshold ? command.arg2 : command.arg3;
		return kCommandContinue;
	}

	if (command.opcodeName.equalsIgnoreCase("SET_FLAG")) {
		const bool flagValue = isTruthy(command.arg2);
		FlagRecord *flag = findRuntimeFlag(command.arg1);
		bool changed = false;
		bool oldValue = false;
		const bool existed = flag != nullptr;
		if (flag) {
			oldValue = flag->value;
			changed = flag->value != flagValue;
			flag->value = flagValue;
		} else {
			FlagRecord newFlag;
			newFlag.name = command.arg1;
			newFlag.value = flagValue;
			_currentFlags.push_back(Common::move(newFlag));
			changed = true;
		}
		debugC(1, kDebugGeneral,
			"Harvester: %s '%s' SET_FLAG '%s' %d -> %d existed=%d changed=%d",
			context.label, context.name.c_str(), command.arg1.c_str(), oldValue, flagValue,
			existed, changed);
		noteMutation(result, changed);
		return kCommandContinue;
	}

	return kCommandUnhandled;
}

Script::CommandStepResult Script::executeAudioCommand(const CommandRecord &command,
		const CommandExecutionContext &context, InteractionResult &result) {
	if (command.opcodeName.equalsIgnoreCase("SPOOL_MUSIC")) {
		result.musicPath = command.arg1;
		return kCommandContinue;
	}

	if (appendStartupAudioCommand(command, result.audioCommands)) {
		return kCommandContinue;
	}

	if (command.opcodeName.equalsIgnoreCase("CHANGE_CD")) {
		const int requestedDisc = parseAsciiIntOrZero(command.arg1);
		if (requestedDisc > 0)
			result.cdChangeDisc = requestedDisc;
		else
			debug(1, "Harvester: invalid CHANGE_CD target '%s' for %s '%s'",
				command.arg1.c_str(), context.label, context.name.c_str());
		return kCommandContinue;
	}

	return kCommandUnhandled;
}

Script::CommandStepResult Script::executeObjectCommand(const CommandRecord &command,
		const CommandExecutionContext &context, InteractionResult &result) {
	if (command.opcodeName.equalsIgnoreCase("ADD") ||
		command.opcodeName.equalsIgnoreCase("DELETE")) {
		ObjectRecord *currentObject = findRuntimeObject(command.arg1, command.arg2);
		if (!currentObject) {
			debug(1, "Harvester: unresolved object for %s '%s' owner='%s' object='%s'",
				context.label, context.name.c_str(), command.arg1.c_str(), command.arg2.c_str());
			return kCommandContinue;
		}

		const bool visible = command.opcodeName.equalsIgnoreCase("ADD");
		const bool affectsCurrentRoom = belongsToContextRoom(context.roomName, currentObject->currentOwnerOrRoom);
		const bool changed = currentObject->visible != visible ||
			currentObject->runtimeVisible != visible;
		currentObject->visible = visible;
		currentObject->runtimeVisible = visible;
		if (visible && currentObject->currentOwnerOrRoom.equalsIgnoreCase("INVENTORY"))
			currentObject->identShown = true;
		noteCurrentRoomVisualMutation(result, changed, affectsCurrentRoom);
		return kCommandContinue;
	}

	if (command.opcodeName.equalsIgnoreCase("ADD2INV")) {
		const ObjectRecord *currentObject = findRuntimeObject(Common::String(), command.arg1);
		const bool affectsCurrentRoom = currentObject &&
			belongsToContextRoom(context.roomName, currentObject->currentOwnerOrRoom);
		noteCurrentRoomVisualMutation(result,
			addRuntimeObjectToInventory(command.arg1), affectsCurrentRoom);
		return kCommandContinue;
	}

	return kCommandUnhandled;
}

Script::CommandStepResult Script::executeAnimationCommand(const CommandRecord &command,
		const CommandExecutionContext &context, InteractionResult &result) {
	if (command.opcodeName.equalsIgnoreCase("SET_ANIM")) {
		AnimRecord *currentAnimation = findRuntimeAnim(command.arg1);
		if (!currentAnimation) {
			debug(1, "Harvester: unresolved anim for %s '%s' anim='%s'",
				context.label, context.name.c_str(), command.arg1.c_str());
			return kCommandContinue;
		}

		const bool wasVisible = currentAnimation->visible;
		const bool active = isTruthy(command.arg2);
		const bool visible = isTruthy(command.arg3);
		int runtimeState = currentAnimation->runtimeState;
		if (visible && (!wasVisible || (active && runtimeState < 0))) {
			runtimeState = 0;
		}
		const bool changed = currentAnimation->active != active ||
			currentAnimation->visible != visible ||
			currentAnimation->runtimeActive != active ||
			currentAnimation->runtimeVisible != visible ||
			currentAnimation->runtimeState != runtimeState;
		currentAnimation->active = active;
		currentAnimation->visible = visible;
		currentAnimation->runtimeActive = active;
		currentAnimation->runtimeVisible = visible;
		currentAnimation->runtimeState = runtimeState;
		noteCurrentRoomVisualMutation(result, changed,
			belongsToContextRoom(context.roomName, currentAnimation->roomName));
		return kCommandContinue;
	}

	return kCommandUnhandled;
}

Script::CommandStepResult Script::executeRegionCommand(const CommandRecord &command,
		const CommandExecutionContext &context, InteractionResult &result) {
	if (command.opcodeName.equalsIgnoreCase("SET_REGION")) {
		RegionRecord *currentRegion = findRuntimeRegion(command.arg1);
		if (!currentRegion) {
			debug(1, "Harvester: unresolved region for %s '%s' region='%s'",
				context.label, context.name.c_str(), command.arg1.c_str());
			return kCommandContinue;
		}

		const bool enabled = !command.arg2.equalsIgnoreCase("F");
		const bool changed = currentRegion->startEnabled != enabled;
		currentRegion->startEnabled = enabled;
		noteCurrentRoomVisualMutation(result, changed,
			belongsToContextRoom(context.roomName, currentRegion->roomName));
		return kCommandContinue;
	}

	return kCommandUnhandled;
}

Script::CommandStepResult Script::executeNpcCommand(const CommandRecord &command,
		const CommandExecutionContext &context, InteractionResult &result) {
	if (command.opcodeName.equalsIgnoreCase("SET_NPC")) {
		NpcRecord *currentNpc = findRuntimeNpc(command.arg1);
		if (!currentNpc) {
			debug(1, "Harvester: unresolved npc for %s '%s' npc='%s'",
				context.label, context.name.c_str(), command.arg1.c_str());
			return kCommandContinue;
		}

		const bool active = isTruthy(command.arg2);
		const bool visible = isTruthy(command.arg3);
		const bool changed = currentNpc->active != active || currentNpc->visible != visible;
		currentNpc->active = active;
		currentNpc->visible = visible;
		noteCurrentRoomVisualMutation(result, changed,
			belongsToContextRoom(context.roomName, currentNpc->roomName));
		return kCommandContinue;
	}

	if (command.opcodeName.equalsIgnoreCase("KILL_NPC") ||
			command.opcodeName.equalsIgnoreCase("MONSTERFY")) {
		NpcRecord *currentNpc = findRuntimeNpc(command.arg1);
		if (!currentNpc) {
			debug(1, "Harvester: unresolved npc for %s '%s' npc='%s'",
				context.label, context.name.c_str(), command.arg1.c_str());
			return kCommandContinue;
		}

		const int deathDamageType = parseDeathDamageType(command.arg2);
		const bool deferLiveNpcDeathTransition =
			_deferredLiveNpcDeathTransitionDepth > 0 &&
			!context.roomName.empty() &&
			currentNpc->roomName.equalsIgnoreCase(context.roomName) &&
			currentNpc->active &&
			currentNpc->visible &&
			!currentNpc->deathOrMonsterfyFlag &&
			currentNpc->runtimeState < 0;
		noteCurrentRoomVisualMutation(result, deferLiveNpcDeathTransition
			? queueRuntimeNpcDeathOrMonsterfy(command.arg1, deathDamageType)
			: finalizeRuntimeNpcDeathOrMonsterfy(command.arg1, deathDamageType),
			belongsToContextRoom(context.roomName, currentNpc->roomName));
		return kCommandContinue;
	}

	return kCommandUnhandled;
}

Script::CommandStepResult Script::executeMonsterCommand(const CommandRecord &command,
		const CommandExecutionContext &context, InteractionResult &result) {
	if (command.opcodeName.equalsIgnoreCase("SET_MONSTER")) {
		MonsterRecord monsterLookup;
		monsterLookup.roomName = context.roomName;
		monsterLookup.monsterName = command.arg1;
		MonsterRecord *currentMonster = findRuntimeMonster(monsterLookup);
		if (!currentMonster) {
			debug(1, "Harvester: unresolved monster for %s '%s' monster='%s'",
				context.label, context.name.c_str(), command.arg1.c_str());
			return kCommandContinue;
		}

		const bool active = isTruthy(command.arg2);
		const bool visible = isTruthy(command.arg3);
		const bool changed = currentMonster->active != active || currentMonster->visible != visible;
		currentMonster->active = active;
		currentMonster->visible = active ? true : visible;
		if (active) {
			currentMonster->runtimeSpawned = false;
			currentMonster->runtimeState = -1;
		}
		if (!currentMonster->visible) {
			currentMonster->runtimeSpawned = false;
			currentMonster->runtimeState = -1;
		}
		if (active && currentMonster->currentHitPoints <= 0)
			currentMonster->currentHitPoints = currentMonster->initialHitPoints;
		noteCurrentRoomVisualMutation(result, changed,
			belongsToContextRoom(context.roomName, currentMonster->roomName));
		return kCommandContinue;
	}

	return kCommandUnhandled;
}

Script::CommandStepResult Script::executeTimerCommand(const CommandRecord &command,
		const CommandExecutionContext &context, InteractionResult &result) {
	if (command.opcodeName.equalsIgnoreCase("SET_TIMER")) {
		const TimerRecord *currentTimer = findRuntimeTimer(command.arg1);
		TimerRecord previousTimer;
		if (currentTimer)
			previousTimer = *currentTimer;
		const bool changed =
			setRuntimeTimerEnabled(command.arg1, command.arg2.equalsIgnoreCase("ON"));
		noteCurrentRoomVisualMutation(result, changed,
			currentTimer && belongsToContextRoom(context.roomName, currentTimer->arg1));
		noteTimerMutation(result, currentTimer ? &previousTimer : nullptr, changed);
		return kCommandContinue;
	}

	if (command.opcodeName.equalsIgnoreCase("KILL_TIMER")) {
		const TimerRecord *currentTimer = findRuntimeTimer(command.arg1);
		TimerRecord previousTimer;
		if (currentTimer)
			previousTimer = *currentTimer;
		const bool changed = setRuntimeTimerEnabled(command.arg1, false);
		noteCurrentRoomVisualMutation(result, changed,
			currentTimer && belongsToContextRoom(context.roomName, currentTimer->arg1));
		noteTimerMutation(result, currentTimer ? &previousTimer : nullptr, changed);
		return kCommandContinue;
	}

	return kCommandUnhandled;
}

Script::CommandStepResult Script::executeExecListCommand(const CommandRecord &command,
		const CommandExecutionContext &context, CommandOutputPolicy outputPolicy, InteractionResult &result) {
	if (command.opcodeName.equalsIgnoreCase("EXEC_LIST")) {
		const ExecListRecord *execList = findExecListRecord(command.arg1);
		if (!execList) {
			debug(1, "Harvester: unresolved exec list for %s '%s' list='%s'",
				context.label, context.name.c_str(), command.arg1.c_str());
			return kCommandContinue;
		}

		for (const Common::String &entry : execList->entries) {
			const CommandExecutionContext entryContext = {
				"exec list entry", execList->listName, context.roomName, context.allowTransitions
			};
			executeCommandChain(entry, entryContext, outputPolicy, result);
			if (hasDeferredInteractionOutputs(result, outputPolicy))
				return kCommandStop;
		}
		return kCommandContinue;
	}

	return kCommandUnhandled;
}

Script::CommandStepResult Script::executeModalCommand(const CommandRecord &command,
		const CommandExecutionContext &context, InteractionResult &result) {
	if (command.opcodeName.equalsIgnoreCase("START_DIALOG")) {
		if (command.arg1.empty()) {
			return kCommandContinue;
		}

		result.dialogueNpcName = command.arg1;
		result.dialogueContinuationTag = command.arg4;
		return kCommandStop;
	}

	if (command.opcodeName.equalsIgnoreCase("GOFLIC")) {
		result.cutscenePath = command.arg1;
		result.continuationTag = command.arg4;
		return kCommandStop;
	}

	if (command.opcodeName.equalsIgnoreCase("SHOW_TEXT")) {
		if (!resolveTextRecord(command.arg1, result.modalText)) {
			debug(1, "Harvester: unresolved SHOW_TEXT record for %s '%s' key='%s'",
				context.label, context.name.c_str(), command.arg1.c_str());
			return kCommandContinue;
		}
		result.continuationTag = command.arg4;
		return kCommandStop;
	}

	if (command.opcodeName.equalsIgnoreCase("CHANGE_LIGHTING")) {
		if (command.arg1.equalsIgnoreCase("DIM"))
			result.lightingCommand = kStartupLightingCommandDim;
		else if (command.arg1.equalsIgnoreCase("NORMAL"))
			result.lightingCommand = kStartupLightingCommandNormal;
		else if (command.arg1.equalsIgnoreCase("NONE"))
			result.lightingCommand = kStartupLightingCommandBlack;
		else if (command.arg1.equalsIgnoreCase("FADE_IN"))
			result.lightingCommand = kStartupLightingCommandFadeIn;
		else
			debug(1, "Harvester: unsupported CHANGE_LIGHTING mode '%s' for %s '%s', preserving control flow",
				command.arg1.c_str(), context.label, context.name.c_str());
		result.continuationTag = command.arg4;
		return kCommandStop;
	}

	return kCommandUnhandled;
}

Script::CommandStepResult Script::executePlayerCommand(const CommandRecord &command,
		InteractionResult &result) {
	if (command.opcodeName.equalsIgnoreCase("HEAL_PC") ||
		command.opcodeName.equalsIgnoreCase("ADJ_HP")) {
		noteMutation(result, setPlayerCurrentHitPoints(
			_playerCurrentHitPoints + parseAsciiIntOrZero(command.arg1)));
		return kCommandContinue;
	}

	const int combatResourceLoadout = resolveCombatResourceLoadoutForOpcode(command.opcodeName);
	if (combatResourceLoadout != 0) {
		noteMutation(result, adjustPlayerCombatResourceCount(combatResourceLoadout,
			parseAsciiIntOrZero(command.arg1),
			resolveCombatResourcePickupMax(combatResourceLoadout),
			command.opcodeName));
		return kCommandContinue;
	}

	if (command.opcodeName.equalsIgnoreCase("PC_CHANGE_WEAPON")) {
		noteCurrentRoomVisualMutation(result,
			setPlayerCombatLoadout(parseAsciiIntOrZero(command.arg1)), true);
		return kCommandContinue;
	}

	if (command.opcodeName.equalsIgnoreCase("KILL_PC")) {
		result.requestPlayerDeath = true;
		result.playerDeathDamageType = parseDeathDamageType(command.arg1);
		noteMutation(result, setPlayerCurrentHitPoints(0));
		return kCommandContinue;
	}

	if (command.opcodeName.equalsIgnoreCase("PAUSE_PC")) {
		noteMutation(result, setPlayerControlPaused(true));
		return kCommandContinue;
	}

	if (command.opcodeName.equalsIgnoreCase("RESUME_PC")) {
		noteMutation(result, setPlayerControlPaused(false));
		return kCommandContinue;
	}

	return kCommandUnhandled;
}

Script::CommandStepResult Script::executeMovementCommand(const CommandRecord &command,
		InteractionResult &result) {
	if (command.opcodeName.equalsIgnoreCase("MOVE_BM2PCZ")) {
		result.moveEntityToPlayerZName = command.arg1;
		return kCommandContinue;
	}

	if (command.opcodeName.equalsIgnoreCase("PC_GOTO_XZ")) {
		result.requestPlayerGotoXZ = true;
		result.playerGotoX = parseAsciiIntOrZero(command.arg1);
		result.playerGotoZ = parseAsciiIntOrZero(command.arg2);
		return kCommandContinue;
	}

	if (command.opcodeName.equalsIgnoreCase("PC_GOTO_Z")) {
		result.requestPlayerGotoZ = true;
		result.playerGotoZ = parseAsciiIntOrZero(command.arg1);
		return kCommandContinue;
	}

	return kCommandUnhandled;
}

Script::CommandStepResult Script::executeTransitionCommand(const CommandRecord &command,
		const CommandExecutionContext &context, CommandOutputPolicy outputPolicy, InteractionResult &result) {
	if (command.opcodeName.equalsIgnoreCase("GODEATHFLIC")) {
		if (outputPolicy == kCommandOutputsRoomExit) {
			debug(1, "Harvester: unsupported startup command '%s' for %s '%s' without menu-exit context",
				command.opcodeName.c_str(), context.label, context.name.c_str());
			return kCommandStop;
		}

		if (context.allowTransitions) {
			result.deathFlicPath = command.arg1;
			result.requestMainMenu = true;
		} else {
			debugC(1, kDebugScene, "Harvester: skipped transition opcode '%s' while processing %s '%s'",
				command.opcodeName.c_str(), context.label, context.name.c_str());
		}
		return kCommandStop;
	}

	if (command.opcodeName.equalsIgnoreCase("END_DEMO")) {
		result.requestDemoEnding = true;
		return kCommandStop;
	}

	if (command.opcodeName.equalsIgnoreCase("CLOSEUP") ||
		command.opcodeName.equalsIgnoreCase("CHANGE_ROOM")) {
		if (context.allowTransitions && outputPolicy == kCommandOutputsInteraction) {
			result.nextRoomName = command.arg1;
			result.roomTransition = command.opcodeName.equalsIgnoreCase("CHANGE_ROOM")
				? kStartupRoomTransitionChangeRoom
				: kStartupRoomTransitionCloseup;
		} else {
			debugC(1, kDebugScene, "Harvester: skipped transition opcode '%s' while processing %s '%s'",
				command.opcodeName.c_str(), context.label, context.name.c_str());
		}
		return kCommandContinue;
	}

	if (command.opcodeName.equalsIgnoreCase("EXIT_CLOSEUP")) {
		result.requestCloseupExit = true;
		return kCommandContinue;
	}

	return kCommandUnhandled;
}

Script::CommandStepResult Script::executeCommand(const CommandRecord &command,
		const CommandExecutionContext &context, CommandOutputPolicy outputPolicy,
		InteractionResult &result, Common::String &nextTag) {
	CommandStepResult step = executeBranchCommand(command, context, result, nextTag);
	if (step != kCommandUnhandled)
		return step;
	step = executeAudioCommand(command, context, result);
	if (step != kCommandUnhandled)
		return step;
	step = executeObjectCommand(command, context, result);
	if (step != kCommandUnhandled)
		return step;
	step = executeAnimationCommand(command, context, result);
	if (step != kCommandUnhandled)
		return step;
	step = executeRegionCommand(command, context, result);
	if (step != kCommandUnhandled)
		return step;
	step = executeNpcCommand(command, context, result);
	if (step != kCommandUnhandled)
		return step;
	step = executeMonsterCommand(command, context, result);
	if (step != kCommandUnhandled)
		return step;
	step = executeTimerCommand(command, context, result);
	if (step != kCommandUnhandled)
		return step;
	step = executeExecListCommand(command, context, outputPolicy, result);
	if (step != kCommandUnhandled)
		return step;
	step = executeModalCommand(command, context, result);
	if (step != kCommandUnhandled)
		return step;
	step = executePlayerCommand(command, result);
	if (step != kCommandUnhandled)
		return step;
	step = executeMovementCommand(command, result);
	if (step != kCommandUnhandled)
		return step;
	step = executeTransitionCommand(command, context, outputPolicy, result);
	if (step != kCommandUnhandled)
		return step;

	debug(1, "Harvester: unsupported startup command '%s' for %s '%s', continuing",
		command.opcodeName.c_str(), context.label, context.name.c_str());
	return kCommandContinue;
}

void Script::executeCommandChain(const Common::String &initialTag, const CommandExecutionContext &context,
		CommandOutputPolicy outputPolicy, InteractionResult &result) {
	debugC(3, kDebugScene, "Harvester: command chain tag='%s' context='%s' source='%s' room='%s' transitions=%d outputs='%s'",
		initialTag.c_str(), context.label, context.name.c_str(), context.roomName.c_str(), context.allowTransitions,
		outputPolicy == kCommandOutputsRoomExit ? "room exit" : "interaction");

	Common::String currentTag = initialTag;
	for (uint step = 0; step < 128 && !currentTag.empty(); ++step) {
		const CommandRecord *command = findCommandRecord(currentTag);
		if (!command) {
			debug(1, "Harvester: unresolved %s tag '%s' for '%s'",
				context.label, currentTag.c_str(), context.name.c_str());
			break;
		}

		debugC(3, kDebugScene,
			"Harvester: %s '%s' step=%u tag='%s' opcode='%s' args=['%s','%s','%s','%s']",
			context.label, context.name.c_str(), step, currentTag.c_str(), command->opcodeName.c_str(),
			command->arg1.c_str(), command->arg2.c_str(), command->arg3.c_str(), command->arg4.c_str());

		Common::String nextTag = command->arg4;
		if (executeCommand(*command, context, outputPolicy, result, nextTag) == kCommandStop)
			return;
		currentTag = nextTag;
	}
}

bool Script::probePickupBlockingCommandChain(const Common::String &initialTag,
		const Common::String &contextName, InteractionResult &result, uint recursionDepth) const {
	if (initialTag.empty() || recursionDepth > 8)
		return false;

	Common::String currentTag = initialTag;
	for (uint step = 0; step < 128 && !currentTag.empty(); ++step) {
		const CommandRecord *command = findCommandRecord(currentTag);
		if (!command) {
			debug(1, "Harvester: unresolved pickup preflight tag '%s' for '%s'",
				currentTag.c_str(), contextName.c_str());
			return false;
		}

		if (command->opcodeName.equalsIgnoreCase("CHECK_FLAG")) {
			const FlagRecord *flag = findRuntimeFlag(command->arg1);
			currentTag = (flag && flag->value) ? command->arg2 : command->arg3;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("CHECK_PERC"))
			return false;

		if (command->opcodeName.equalsIgnoreCase("SHOW_TEXT")) {
			if (!resolveTextRecord(command->arg1, result.modalText))
				return false;
			result.continuationTag = command->arg4;
			return !result.modalText.value.empty();
		}

		if (command->opcodeName.equalsIgnoreCase("EXEC_LIST")) {
			const ExecListRecord *execList = findExecListRecord(command->arg1);
			if (!execList)
				return false;

			for (const Common::String &entry : execList->entries) {
				if (probePickupBlockingCommandChain(entry, contextName, result, recursionDepth + 1))
					return true;
			}
			return false;
		}

		Common::Array<AudioCommand> audioCommands;
		if (appendStartupAudioCommand(*command, audioCommands))
			return false;

		if (command->opcodeName.equalsIgnoreCase("SET_FLAG") ||
			command->opcodeName.equalsIgnoreCase("SPOOL_MUSIC") ||
			command->opcodeName.equalsIgnoreCase("ADD") ||
			command->opcodeName.equalsIgnoreCase("DELETE") ||
			command->opcodeName.equalsIgnoreCase("ADD2INV") ||
			isCombatResourceOpcode(command->opcodeName) ||
			command->opcodeName.equalsIgnoreCase("SET_ANIM") ||
			command->opcodeName.equalsIgnoreCase("SET_REGION") ||
			command->opcodeName.equalsIgnoreCase("SET_NPC") ||
			command->opcodeName.equalsIgnoreCase("SET_MONSTER") ||
			command->opcodeName.equalsIgnoreCase("SET_TIMER") ||
			command->opcodeName.equalsIgnoreCase("KILL_TIMER") ||
			command->opcodeName.equalsIgnoreCase("START_DIALOG") ||
			command->opcodeName.equalsIgnoreCase("GOFLIC") ||
			command->opcodeName.equalsIgnoreCase("GODEATHFLIC") ||
			command->opcodeName.equalsIgnoreCase("END_DEMO") ||
			command->opcodeName.equalsIgnoreCase("KILL_NPC") ||
			command->opcodeName.equalsIgnoreCase("MONSTERFY") ||
			command->opcodeName.equalsIgnoreCase("HEAL_PC") ||
			command->opcodeName.equalsIgnoreCase("ADJ_HP") ||
			command->opcodeName.equalsIgnoreCase("KILL_PC") ||
			command->opcodeName.equalsIgnoreCase("PAUSE_PC") ||
			command->opcodeName.equalsIgnoreCase("RESUME_PC") ||
			command->opcodeName.equalsIgnoreCase("CHANGE_CD") ||
			command->opcodeName.equalsIgnoreCase("MOVE_BM2PCZ") ||
			command->opcodeName.equalsIgnoreCase("PC_CHANGE_WEAPON") ||
			command->opcodeName.equalsIgnoreCase("PC_GOTO_XZ") ||
			command->opcodeName.equalsIgnoreCase("PC_GOTO_Z") ||
			command->opcodeName.equalsIgnoreCase("CHANGE_LIGHTING") ||
			command->opcodeName.equalsIgnoreCase("EXIT_CLOSEUP") ||
			command->opcodeName.equalsIgnoreCase("CLOSEUP") ||
			command->opcodeName.equalsIgnoreCase("CHANGE_ROOM")) {
			return false;
		}

		currentTag = command->arg4;
	}

	return false;
}

bool Script::hasActionableCommandChain(const Common::String &initialTag) const {
	Common::String currentTag = initialTag;
	for (uint step = 0; step < 128 && !currentTag.empty(); ++step) {
		const CommandRecord *command = findCommandRecord(currentTag);
		if (!command) {
			debug(1, "Harvester: unresolved interaction probe tag '%s'", currentTag.c_str());
			break;
		}

		if (command->opcodeName.equalsIgnoreCase("CHECK_FLAG")) {
			const FlagRecord *flag = findRuntimeFlag(command->arg1);
			currentTag = (flag && flag->value) ? command->arg2 : command->arg3;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("CHECK_PERC")) {
			currentTag = command->arg2.empty() ? command->arg3 : command->arg2;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("SET_FLAG") ||
			command->opcodeName.equalsIgnoreCase("SPOOL_MUSIC") ||
			command->opcodeName.equalsIgnoreCase("ADD") ||
			command->opcodeName.equalsIgnoreCase("DELETE") ||
			command->opcodeName.equalsIgnoreCase("ADD2INV") ||
			isCombatResourceOpcode(command->opcodeName) ||
			command->opcodeName.equalsIgnoreCase("SET_ANIM") ||
			command->opcodeName.equalsIgnoreCase("SET_REGION") ||
			command->opcodeName.equalsIgnoreCase("SET_NPC") ||
			command->opcodeName.equalsIgnoreCase("SET_MONSTER") ||
			command->opcodeName.equalsIgnoreCase("SET_TIMER") ||
			command->opcodeName.equalsIgnoreCase("KILL_TIMER") ||
			command->opcodeName.equalsIgnoreCase("EXEC_LIST") ||
			command->opcodeName.equalsIgnoreCase("START_DIALOG") ||
			command->opcodeName.equalsIgnoreCase("GOFLIC") ||
			command->opcodeName.equalsIgnoreCase("GODEATHFLIC") ||
			command->opcodeName.equalsIgnoreCase("END_DEMO") ||
			command->opcodeName.equalsIgnoreCase("KILL_NPC") ||
			command->opcodeName.equalsIgnoreCase("MONSTERFY") ||
			command->opcodeName.equalsIgnoreCase("SHOW_TEXT") ||
			command->opcodeName.equalsIgnoreCase("HEAL_PC") ||
			command->opcodeName.equalsIgnoreCase("ADJ_HP") ||
			command->opcodeName.equalsIgnoreCase("KILL_PC") ||
			command->opcodeName.equalsIgnoreCase("PAUSE_PC") ||
			command->opcodeName.equalsIgnoreCase("RESUME_PC") ||
			command->opcodeName.equalsIgnoreCase("CHANGE_CD") ||
			command->opcodeName.equalsIgnoreCase("MOVE_BM2PCZ") ||
			command->opcodeName.equalsIgnoreCase("PC_CHANGE_WEAPON") ||
			command->opcodeName.equalsIgnoreCase("PC_GOTO_XZ") ||
			command->opcodeName.equalsIgnoreCase("PC_GOTO_Z") ||
			command->opcodeName.equalsIgnoreCase("CHANGE_LIGHTING") ||
			command->opcodeName.equalsIgnoreCase("EXIT_CLOSEUP") ||
			command->opcodeName.equalsIgnoreCase("CLOSEUP") ||
			command->opcodeName.equalsIgnoreCase("CHANGE_ROOM")) {
			return true;
		}

		Common::Array<AudioCommand> audioCommands;
		if (appendStartupAudioCommand(*command, audioCommands))
			return true;

		currentTag = command->arg4;
	}

	return false;
}

} // End of namespace Harvester
