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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/formats/ini-file.h"
#include "common/memstream.h"
#include "harvester/detection.h"
#include "harvester/parse_utils.h"
#include "harvester/resources.h"

namespace Harvester {

static const char *const kStartupConfigPath = "CONFIG.INI";
static const char *const kConfigSectionName = "harvester";
static const char *const kDefaultTownScript = "HARVEST.SCR";
static const char *const kDefaultVoicePath = "SOUND/VOICE/";
static const int kMaxStartupOptionLevel = 9;

static int clampStartupOptionLevel(int level) {
	if (level < 0)
		return 0;
	if (level > kMaxStartupOptionLevel)
		return kMaxStartupOptionLevel;

	return level;
}

static Common::String toNativeConfigPath(const Common::String &value) {
	Common::String result(value);
	for (uint i = 0; i < result.size(); ++i) {
		if (result[i] == '/')
			result.setChar('\\', i);
	}

	return result;
}

static Common::String dialogueTextModeToConfigValue(StartupDialogueTextMode mode) {
	switch (mode) {
	case kStartupDialogueTextNone:
		return "NO";
	case kStartupDialogueTextClick:
		return "CLICK";
	case kStartupDialogueTextYes:
	default:
		return "YES";
	}
}

bool Script::load(ResourceManager &resources) {
	_path = kDefaultTownScript;
	_data.clear();
	_entrances.clear();
	_mapEntrances.clear();
	_mapLocations.clear();
	_rooms.clear();
	_objects.clear();
	_animations.clear();
	_npcs.clear();
	_monsters.clear();
	_timers.clear();
	_regions.clear();
	_flags.clear();
	_commands.clear();
	_execLists.clear();
	_texts.clear();
	_heads.clear();
	_useItems.clear();
	_currentFlags.clear();
	_currentObjects.clear();
	_currentAnimations.clear();
	_currentNpcs.clear();
	_currentMonsters.clear();
	_currentTimers.clear();
	_currentRegions.clear();
	_playerCurrentHitPoints = Script::kDefaultPlayerHitPoints;
	_playerCombatLoadout = kDefaultPlayerCombatLoadout;
	_playerControlPaused = false;
	_fxVolumeLevel = 9;
	_musicVolumeLevel = 3;
	_gammaLevel = 0;
	_quickTipsEnabled = true;
	_goreEnabled = true;
	_voicePath = kDefaultVoicePath;
	_parentalPassword.clear();
	_savePath = "./";
	_townPath = kDefaultTownScript;
	_cdRomPath.clear();
	_drive1Path.clear();
	_drive2Path.clear();
	_drive3Path.clear();
	_dialogueTextMode = kStartupDialogueTextYes;

	loadConfig(resources);

	if (!resources.loadFile(_path, _data)) {
		warning("Harvester: unable to load town script '%s'", _path.c_str());
		return false;
	}

	decode();
	parseTownRecords(resources);
	resetRuntimeState();
	debug(1, "Harvester: loaded startup script '%s' (%u bytes)", _path.c_str(), (uint)_data.size());
	return true;
}

bool Script::loadConfig(ResourceManager &resources) {
	Common::Array<byte> configData;
	if (!resources.loadFile(kStartupConfigPath, configData)) {
		warning("Harvester: unable to load startup config '%s', using defaults", kStartupConfigPath);
		return false;
	}

	Common::MemoryReadStream stream(configData.data(), configData.size());
	Common::INIFile config;
	config.setDefaultSectionName(kConfigSectionName);
	config.requireKeyValueDelimiter();
	config.suppressValuelessLineWarning();

	if (!config.loadFromStream(stream)) {
		warning("Harvester: unable to parse startup config '%s', using defaults", kStartupConfigPath);
		return false;
	}

	Common::String townPath;
	if (config.getKey("TOWN", kConfigSectionName, townPath)) {
		_townPath = townPath;
		_path = resources.normalizeResourcePath(townPath);
	}

	Common::String savePath;
	if (config.getKey("SAVE", kConfigSectionName, savePath))
		_savePath = savePath;

	(void)config.getKey("CD_ROM", kConfigSectionName, _cdRomPath);
	(void)config.getKey("DRIVE_1", kConfigSectionName, _drive1Path);
	(void)config.getKey("DRIVE_2", kConfigSectionName, _drive2Path);
	(void)config.getKey("DRIVE_3", kConfigSectionName, _drive3Path);

	Common::String quickTipsValue;
	if (config.getKey("QUICK_TIPS", kConfigSectionName, quickTipsValue))
		_quickTipsEnabled = quickTipsValue.equalsIgnoreCase("ON");

	Common::String voicePath;
	if (config.getKey("VOICE", kConfigSectionName, voicePath))
		_voicePath = resources.normalizeResourcePath(voicePath);

	Common::String fxVolume;
	if (config.getKey("FX_VOLUME", kConfigSectionName, fxVolume))
		_fxVolumeLevel = clampStartupOptionLevel(parseAsciiIntOrZero(fxVolume));

	Common::String musicVolume;
	if (config.getKey("MUSIC_VOLUME", kConfigSectionName, musicVolume))
		_musicVolumeLevel = clampStartupOptionLevel(parseAsciiIntOrZero(musicVolume));

	Common::String gammaValue;
	if (config.getKey("GAMMA", kConfigSectionName, gammaValue))
		_gammaLevel = clampStartupOptionLevel(parseAsciiIntOrZero(gammaValue));

	Common::String textMode;
	if (config.getKey("TEXT", kConfigSectionName, textMode)) {
		if (textMode.equalsIgnoreCase("NO"))
			_dialogueTextMode = kStartupDialogueTextNone;
		else if (textMode.equalsIgnoreCase("CLICK"))
			_dialogueTextMode = kStartupDialogueTextClick;
		else
			_dialogueTextMode = kStartupDialogueTextYes;
	}

	Common::String goreValue;
	if (config.getKey("GORE", kConfigSectionName, goreValue))
		_goreEnabled = !goreValue.equalsIgnoreCase("NO") && !goreValue.equalsIgnoreCase("OFF");

	Common::String passwordValue;
	if (config.getKey("PASSWORD", kConfigSectionName, passwordValue))
		_parentalPassword = passwordValue;

	return true;
}

bool Script::saveConfig() const {
	Common::DumpFile file;
	const Common::FSNode gamePathNode(ConfMan.getPath("path"));
	const Common::FSNode configNode = gamePathNode.getChild(kStartupConfigPath);
	if (!file.open(configNode)) {
		warning("Harvester: unable to save startup config '%s'",
			configNode.getPath().toString(Common::Path::kNativeSeparator).c_str());
		return false;
	}

	auto writeKey = [&](const char *key, const Common::String &value) {
		file.writeString(Common::String::format("%s=%s\n", key, value.c_str()));
	};

	writeKey("FX_VOLUME", Common::String::format("%d", _fxVolumeLevel));
	writeKey("MUSIC_VOLUME", Common::String::format("%d", _musicVolumeLevel));
	writeKey("TEXT", dialogueTextModeToConfigValue(_dialogueTextMode));
	writeKey("GORE", _goreEnabled ? "YES" : "NO");
	writeKey("QUICK_TIPS", _quickTipsEnabled ? "ON" : "OFF");
	if (!_savePath.empty())
		writeKey("SAVE", toNativeConfigPath(_savePath));
	writeKey("VOICE", toNativeConfigPath(_voicePath));
	writeKey("GAMMA", Common::String::format("%d", _gammaLevel));
	writeKey("TOWN", toNativeConfigPath(_townPath));
	if (!_parentalPassword.empty())
		writeKey("PASSWORD", _parentalPassword);
	if (!_cdRomPath.empty())
		writeKey("CD_ROM", _cdRomPath);
	if (!_drive1Path.empty())
		writeKey("DRIVE_1", _drive1Path);
	if (!_drive2Path.empty())
		writeKey("DRIVE_2", _drive2Path);
	if (!_drive3Path.empty())
		writeKey("DRIVE_3", _drive3Path);

	file.flush();
	return !file.err();
}

void Script::setFxVolumeLevel(int level) {
	_fxVolumeLevel = clampStartupOptionLevel(level);
}

void Script::setMusicVolumeLevel(int level) {
	_musicVolumeLevel = clampStartupOptionLevel(level);
}

void Script::setGammaLevel(int level) {
	_gammaLevel = clampStartupOptionLevel(level);
}

bool Script::executeRoomEnterCommands(const Common::String &roomName,
		InteractionResult &result) {
	result = InteractionResult();

	const RoomRecord *room = findRoomRecord(roomName);
	if (!room)
		return false;

	const CommandExecutionContext context = {
		"room entry command", room->roomName, room->roomName, true
	};
	executeCommandChain(room->onEnterCommand, context, kCommandOutputsInteraction, result);
	return true;
}

bool Script::executeRoomExitCommands(const Common::String &roomName,
		InteractionResult &result) {
	result = InteractionResult();

	const RoomRecord *room = findRoomRecord(roomName);
	if (!room)
		return false;

	const CommandExecutionContext context = {
		"room exit command", room->roomName, room->roomName, false
	};
	executeCommandChain(room->onExitCommand, context, kCommandOutputsRoomExit, result);
	return true;
}

bool Script::resolveObjectInteraction(const ObjectRecord &object, InteractionResult &result,
		const ScriptExecutionContext &executionContext) {
	const Common::String roomName = executionContext.roomName();
	result = InteractionResult();
	Common::String commandRoomName = roomName;
	if (commandRoomName.empty() &&
			!object.currentOwnerOrRoom.empty() &&
			!object.currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName)) {
		commandRoomName = object.currentOwnerOrRoom;
	}

	if (isPickupObject(object)) {
		if (ObjectRecord *currentObject = findRuntimeObject(object.currentOwnerOrRoom, object.objectName)) {
			if (!currentObject->currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName)) {
				const bool affectsCurrentRoom = !commandRoomName.empty() &&
					currentObject->currentOwnerOrRoom.equalsIgnoreCase(commandRoomName);
				currentObject->currentOwnerOrRoom = kInventoryOwnerName;
				currentObject->identShown = true;
				result.mutatedRuntimeState = true;
				result.visualRuntimeStateChanged = affectsCurrentRoom;
			}
		}
	}
	if (object.actionTag.empty())
		return result.mutatedRuntimeState;

	const CommandExecutionContext context = {
		"interaction command", object.objectName, commandRoomName, true
	};
	executeCommandChain(object.actionTag, context, kCommandOutputsInteraction, result);

	return !result.nextRoomName.empty() || !result.cutscenePath.empty() ||
		!result.deathFlicPath.empty() || result.requestMainMenu || result.requestDemoEnding ||
		!result.dialogueNpcName.empty() || !result.musicPath.empty() || !result.audioCommands.empty() ||
		result.cdChangeDisc > 0 || !result.continuationTag.empty() || !result.modalText.value.empty() ||
		result.lightingCommand != kStartupLightingCommandNone ||
		!result.moveEntityToPlayerZName.empty() || result.requestPlayerGotoXZ ||
		result.requestPlayerGotoZ ||
		result.requestPlayerDeath ||
		result.requestCloseupExit || result.mutatedRuntimeState ||
		hasActionableCommandChain(object.actionTag);
}

bool Script::resolveRegionInteraction(const RegionRecord &region, InteractionResult &result,
		const ScriptExecutionContext &executionContext) {
	const Common::String roomName = executionContext.roomName();
	result = InteractionResult();
	if (region.actionTag.empty())
		return false;
	const Common::String &commandRoomName = roomName.empty() ? region.roomName : roomName;

	const CommandExecutionContext context = {
		"region command", region.regionName, commandRoomName, true
	};
	executeCommandChain(region.actionTag, context, kCommandOutputsInteraction, result);
	return !result.nextRoomName.empty() || !result.cutscenePath.empty() ||
		!result.deathFlicPath.empty() || result.requestMainMenu || result.requestDemoEnding ||
		!result.dialogueNpcName.empty() || !result.musicPath.empty() || !result.audioCommands.empty() ||
		result.cdChangeDisc > 0 || !result.continuationTag.empty() || !result.modalText.value.empty() ||
		result.lightingCommand != kStartupLightingCommandNone ||
		!result.moveEntityToPlayerZName.empty() || result.requestPlayerGotoXZ ||
		result.requestPlayerGotoZ ||
		result.requestPlayerDeath ||
		result.requestCloseupExit || result.mutatedRuntimeState ||
		hasActionableCommandChain(region.actionTag);
}

bool Script::resolveUseItemInteraction(const Common::String &itemName, const ObjectRecord &target,
		InteractionResult &result, const ScriptExecutionContext &executionContext) {
	const Common::String roomName = executionContext.roomName();
	result = InteractionResult();

	const UseItemRecord *useItem = findUseItemRecord(itemName, target);
	if (!useItem)
		return false;
	Common::String commandRoomName = roomName;
	if (commandRoomName.empty() &&
			!target.currentOwnerOrRoom.empty() &&
			!target.currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName)) {
		commandRoomName = target.currentOwnerOrRoom;
	}

	const Common::String contextName = Common::String::format("%s -> %s",
		itemName.c_str(), target.objectName.c_str());
	const CommandExecutionContext context = {
		"useitem command", contextName, commandRoomName, true
	};
	executeCommandChain(useItem->actionTag, context, kCommandOutputsInteraction, result);
	return true;
}

bool Script::executeDebugCommand(const CommandRecord &command,
		InteractionResult &result, bool allowTransitions) {
	result = InteractionResult();
	if (command.opcodeName.empty())
		return false;

	// Reuse the normal action-tag executor by injecting a one-off command node and
	// then removing it once the synthetic debug command has been evaluated.
	Common::String debugTag;
	for (uint suffix = 0;; ++suffix) {
		debugTag = Common::String::format("__DEBUG_COMMAND_%u", suffix);
		if (!findCommandRecord(debugTag))
			break;
	}

	CommandRecord debugCommand = command;
	debugCommand.triggerTag = debugTag;
	_commands.push_back(Common::move(debugCommand));
	const bool handled = executeActionTag(debugTag, result, allowTransitions, ScriptExecutionContext());
	_commands.pop_back();
	return handled;
}

bool Script::executeActionTag(const Common::String &tag, InteractionResult &result,
		bool allowTransitions, const ScriptExecutionContext &executionContext) {
	const Common::String roomName = executionContext.roomName();
	result = InteractionResult();
	if (tag.empty())
		return false;

	const CommandExecutionContext context = {
		"action tag", tag, roomName, allowTransitions
	};
	executeCommandChain(tag, context, kCommandOutputsInteraction, result);

	return !result.nextRoomName.empty() || !result.cutscenePath.empty() ||
		!result.deathFlicPath.empty() || result.requestMainMenu || result.requestDemoEnding ||
		!result.dialogueNpcName.empty() || !result.musicPath.empty() || !result.audioCommands.empty() ||
		result.cdChangeDisc > 0 || !result.continuationTag.empty() || !result.modalText.value.empty() ||
		result.lightingCommand != kStartupLightingCommandNone ||
		!result.moveEntityToPlayerZName.empty() || result.requestPlayerGotoXZ ||
		result.requestPlayerGotoZ ||
		result.requestPlayerDeath ||
		result.requestCloseupExit || result.mutatedRuntimeState ||
		hasActionableCommandChain(tag);
}

bool Script::executeTimerAction(const Common::String &timerName, InteractionResult &result,
		bool allowTransitions, const ScriptExecutionContext &executionContext) {
	const Common::String roomName = executionContext.roomName();
	result = InteractionResult();
	if (timerName.empty())
		return false;

	const TimerRecord *timer = findRuntimeTimer(timerName);
	if (!timer) {
		debug(1, "Harvester: unresolved timer record '%s'", timerName.c_str());
		return false;
	}
	if (timer->arg2.empty()) {
		debug(1, "Harvester: timer '%s' has no action tag", timerName.c_str());
		return false;
	}
	const Common::String &commandRoomName = roomName.empty() ? timer->arg1 : roomName;

	const CommandExecutionContext context = {
		"timer command", timer->timerName, commandRoomName, allowTransitions
	};
	executeCommandChain(timer->arg2, context, kCommandOutputsInteraction, result);

	return !result.nextRoomName.empty() || !result.cutscenePath.empty() ||
		!result.deathFlicPath.empty() || result.requestMainMenu || result.requestDemoEnding ||
		!result.dialogueNpcName.empty() || !result.musicPath.empty() || !result.audioCommands.empty() ||
		result.cdChangeDisc > 0 || !result.continuationTag.empty() || !result.modalText.value.empty() ||
		result.lightingCommand != kStartupLightingCommandNone ||
		!result.moveEntityToPlayerZName.empty() || result.requestPlayerGotoXZ ||
		result.requestPlayerGotoZ ||
		result.requestPlayerDeath ||
		result.requestCloseupExit || result.mutatedRuntimeState ||
		hasActionableCommandChain(timer->arg2);
}

bool Script::executeNestedActionTag(const Common::String &tag, InteractionResult &result,
		bool allowTransitions) {
	const bool handled = executeActionTag(tag, result, allowTransitions, ScriptExecutionContext());
	if (handled)
		result.abortRemainingCommandChain = true;

	return handled;
}

bool Script::isPickupObject(const ObjectRecord &object) const {
	return !object.altSpritePath.empty() &&
		!object.currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName);
}

bool Script::isPickupBlockedByAction(const ObjectRecord &object,
		InteractionResult *result) const {
	if (result)
		*result = InteractionResult();
	if (!isPickupObject(object) || object.actionTag.empty())
		return false;
	if (!result) {
		InteractionResult ignoredResult;
		return probePickupBlockingCommandChain(object.actionTag, object.objectName, ignoredResult, 0);
	}

	return probePickupBlockingCommandChain(object.actionTag, object.objectName, *result, 0);
}

bool Script::hasObjectInteraction(const ObjectRecord &object) const {
	if (isPickupObject(object))
		return true;
	if (object.actionTag.empty())
		return false;

	return hasActionableCommandChain(object.actionTag);
}

bool Script::hasUseItemInteraction(const Common::String &itemName, const ObjectRecord &target) const {
	return findUseItemRecord(itemName, target) != nullptr;
}

} // End of namespace Harvester
