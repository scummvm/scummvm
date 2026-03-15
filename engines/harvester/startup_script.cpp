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

#include "harvester/startup_script.h"

#include <cstdlib>

#include "common/debug.h"
#include "common/formats/ini-file.h"
#include "common/memstream.h"
#include "harvester/detection.h"
#include "harvester/resources.h"

namespace Harvester {

static const char *const kStartupConfigPath = "CONFIG.INI";
static const char *const kConfigSectionName = "harvester";
static const char *const kDefaultTownScript = "HARVEST.SCR";
static const byte kTownScriptXorKey = 0xaa;
static const float kDefaultPaletteBrightness = 1.0f;
static const float kDimmedPaletteBrightness = 0.6f;

static Common::String trimAsciiLine(const Common::String &value) {
	uint start = 0;
	uint end = value.size();

	while (start < end && (value[start] == ' ' || value[start] == '\t'))
		++start;
	while (end > start && (value[end - 1] == ' ' || value[end - 1] == '\t' || value[end - 1] == '\r'))
		--end;

	return value.substr(start, end - start);
}

static Common::String normalizeInteractionLabel(const Common::String &value) {
	Common::String label = value;
	for (uint i = 0; i < label.size(); ++i) {
		if (label[i] == '_')
			label.setChar(' ', i);
	}

	return label;
}

static int parseEntranceFacing(const Common::String &direction) {
	if (direction.equalsIgnoreCase("FRONT"))
		return 0;
	if (direction.equalsIgnoreCase("LEFT"))
		return 1;
	if (direction.equalsIgnoreCase("RIGHT"))
		return 2;
	if (direction.equalsIgnoreCase("BACK"))
		return 3;

	return -1;
}

static bool appendStartupAudioCommand(const StartupCommandRecord &command, Common::Array<StartupAudioCommand> &commands) {
	if (command.opcodeName.equalsIgnoreCase("START_WAV")) {
		StartupAudioCommand audioCommand;
		audioCommand.type = kStartupAudioCommandStartWav;
		audioCommand.path = command.arg1;
		commands.push_back(audioCommand);
		return true;
	}

	if (command.opcodeName.equalsIgnoreCase("START_SINGLE_WAV")) {
		StartupAudioCommand audioCommand;
		audioCommand.type = kStartupAudioCommandStartSingleWav;
		audioCommand.path = command.arg1;
		commands.push_back(audioCommand);
		return true;
	}

	if (command.opcodeName.equalsIgnoreCase("LOAD_WAV")) {
		StartupAudioCommand audioCommand;
		audioCommand.type = kStartupAudioCommandLoadWav;
		audioCommand.path = command.arg1;
		audioCommand.slot = command.arg2.empty() ? -1 : atoi(command.arg2.c_str());
		commands.push_back(audioCommand);
		return true;
	}

	if (command.opcodeName.equalsIgnoreCase("PLAY_WAV")) {
		StartupAudioCommand audioCommand;
		audioCommand.type = kStartupAudioCommandPlayWav;
		audioCommand.slot = command.arg1.empty() ? -1 : atoi(command.arg1.c_str());
		commands.push_back(audioCommand);
		return true;
	}

	if (command.opcodeName.equalsIgnoreCase("DELETE_WAV")) {
		StartupAudioCommand audioCommand;
		audioCommand.type = kStartupAudioCommandDeleteWav;
		audioCommand.slot = command.arg1.empty() ? -1 : atoi(command.arg1.c_str());
		commands.push_back(audioCommand);
		return true;
	}

	return false;
}

static void tokenizeTownScriptLine(const Common::String &line, Common::Array<Common::String> &tokens) {
	tokens.clear();

	for (uint i = 0; i < line.size();) {
		while (i < line.size() && (line[i] == ' ' || line[i] == '\t'))
			++i;
		if (i >= line.size())
			break;

		if (line[i] == '"') {
			++i;
			Common::String token;
			while (i < line.size() && line[i] != '"')
				token += line[i++];
			if (i < line.size() && line[i] == '"')
				++i;
			tokens.push_back(token);
			continue;
		}

		Common::String token;
		while (i < line.size() && line[i] != ' ' && line[i] != '\t')
			token += line[i++];
		tokens.push_back(token);
	}
}

bool StartupScript::load(ResourceManager &resources) {
	_path = kDefaultTownScript;
	_data.clear();
	_entrances.clear();
	_rooms.clear();
	_objects.clear();
	_animations.clear();
	_flags.clear();
	_commands.clear();
	_texts.clear();
	_quickTipsEnabled = true;

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

bool StartupScript::loadConfig(ResourceManager &resources) {
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
	if (config.getKey("TOWN", kConfigSectionName, townPath))
		_path = resources.normalizeResourcePath(townPath);

	Common::String quickTipsValue;
	if (config.getKey("QUICK_TIPS", kConfigSectionName, quickTipsValue))
		_quickTipsEnabled = quickTipsValue.equalsIgnoreCase("ON");

	return true;
}

void StartupScript::decode() {
	for (byte &value : _data) {
		if (value == '\r' || value == '\n')
			continue;

		value ^= kTownScriptXorKey;
	}
}

void StartupScript::parseTownRecords(ResourceManager &resources) {
	_entrances.clear();
	_rooms.clear();
	_objects.clear();
	_animations.clear();
	_flags.clear();
	_commands.clear();
	_texts.clear();

	auto parseLine = [&](const Common::String &rawLine) {
		const Common::String line = trimAsciiLine(rawLine);
		if (line.empty() || line[0] == '{')
			return;

		Common::Array<Common::String> tokens;
		tokenizeTownScriptLine(line, tokens);
		if (tokens.empty())
			return;

		uint tagIndex = tokens.size();
		for (uint i = 0; i < tokens.size(); ++i) {
			if (tokens[i] == "ANIM" || tokens[i] == "ENTRANCE" || tokens[i] == "ROOM" || tokens[i] == "OBJECT" ||
				tokens[i] == "FLAG" || tokens[i] == "COMMAND" || tokens[i] == "TEXT") {
				tagIndex = i;
				break;
			}
		}
		if (tagIndex == tokens.size())
			return;

		const Common::String &tag = tokens[tagIndex];
		if (tag == "FLAG") {
			if (tokens.size() < tagIndex + 3)
				return;

			StartupFlagRecord flag;
			flag.name = tokens[tagIndex + 1];
			flag.value = tokens[tagIndex + 2].equalsIgnoreCase("T");
			if (!flag.name.empty())
				_flags.push_back(flag);
			return;
		}

		if (tag == "COMMAND") {
			if (tokens.size() < tagIndex + 6)
				return;

			StartupCommandRecord command;
			command.triggerTag = tokens[tagIndex + 1];
			command.opcodeName = tokens[tagIndex + 2];
			command.arg1 = resources.normalizeResourcePath(tokens[tagIndex + 3]);
			command.arg2 = tokens[tagIndex + 4];
			command.arg3 = tokens[tagIndex + 5];
			command.arg4 = tokens.size() > tagIndex + 6 ? tokens[tagIndex + 6] : Common::String();

			// Only path-like operands should be normalized.
			if (!command.opcodeName.equalsIgnoreCase("SPOOL_MUSIC") &&
				!command.opcodeName.equalsIgnoreCase("GOFLIC") &&
				!command.opcodeName.equalsIgnoreCase("START_WAV") &&
				!command.opcodeName.equalsIgnoreCase("LOAD_WAV") &&
				!command.opcodeName.equalsIgnoreCase("START_SINGLE_WAV")) {
				command.arg1 = tokens[tagIndex + 3];
			}

			if (!command.triggerTag.empty() && !command.opcodeName.empty())
				_commands.push_back(command);
			return;
		}

		if (tag == "TEXT") {
			if (tokens.size() < tagIndex + 4)
				return;

			StartupTextRecord textRecord;
			textRecord.key = tokens[tagIndex + 1];
			textRecord.boxName = tokens[tagIndex + 2];
			textRecord.value = tokens[tagIndex + 3];
			for (uint i = 0; i < textRecord.value.size(); ++i) {
				if (textRecord.value[i] == '_')
					textRecord.value.setChar(' ', i);
			}
			if (!textRecord.key.empty())
				_texts.push_back(textRecord);
			return;
		}

		if (tag == "ENTRANCE") {
			if (tokens.size() < tagIndex + 4)
				return;

			StartupEntranceRecord entrance;
			if (tagIndex >= 3) {
				entrance.posX = atoi(tokens[0].c_str());
				entrance.posY = atoi(tokens[1].c_str());
				entrance.posZ = atoi(tokens[2].c_str());
			}
			entrance.direction = tokens[tagIndex + 1];
			entrance.facing = parseEntranceFacing(entrance.direction);
			entrance.roomName = tokens[tagIndex + 2];
			entrance.entranceName = tokens[tagIndex + 3];
			if (!entrance.roomName.empty() && !entrance.entranceName.empty())
				_entrances.push_back(entrance);
			return;
		}

		if (tag == "ROOM") {
			if (tokens.size() < tagIndex + 10)
				return;

			StartupRoomRecord room;
			if (tagIndex >= 6) {
				room.minZ = atoi(tokens[0].c_str());
				room.maxZ = atoi(tokens[1].c_str());
				room.maxZScreenY = atoi(tokens[2].c_str());
				room.minZScreenY = atoi(tokens[3].c_str());
				room.fullScaleZ = atoi(tokens[4].c_str());
				room.maxZScalePercent = atoi(tokens[5].c_str());
				if (room.maxZ != room.fullScaleZ) {
					room.perspectiveScale = ((100.0f - (float)room.maxZScalePercent) /
						(float)(room.maxZ - room.fullScaleZ)) * 0.01f;
				}
			}
			room.roomName = tokens[tagIndex + 1];
			room.musicPath = resources.normalizeResourcePath(tokens[tagIndex + 2]);
			room.field38 = tokens[tagIndex + 3];
			room.field3c = tokens[tagIndex + 4];
			room.field40 = tokens[tagIndex + 5];
			room.palettePath = resources.normalizeResourcePath(tokens[tagIndex + 6]);
			room.dimmable = tokens[tagIndex + 7].equalsIgnoreCase("T");
			room.onEnterCommand = tokens[tagIndex + 8];
			room.onExitCommand = tokens[tagIndex + 9];
			if (!room.roomName.empty())
				_rooms.push_back(room);
			return;
		}

			if (tag == "ANIM") {
				if (tokens.size() < tagIndex + 10)
					return;

				StartupAnimRecord anim;
				if (tagIndex >= 4) {
					anim.x = atoi(tokens[0].c_str());
					anim.y = atoi(tokens[1].c_str());
					anim.z = atoi(tokens[2].c_str());
					anim.frameDelay = atoi(tokens[3].c_str());
				}
				anim.roomName = tokens[tagIndex + 1];
				anim.resourcePath = resources.normalizeResourcePath(tokens[tagIndex + 2]);
				anim.animName = tokens[tagIndex + 3];
				anim.active = tokens[tagIndex + 4].equalsIgnoreCase("T");
				anim.visible = tokens[tagIndex + 5].equalsIgnoreCase("T");
				anim.looping = tokens[tagIndex + 6].equalsIgnoreCase("T");
				anim.backward = tokens[tagIndex + 7].equalsIgnoreCase("T");
				anim.pingPong = tokens[tagIndex + 8].equalsIgnoreCase("T");
				anim.remove = tokens[tagIndex + 9].equalsIgnoreCase("T");
				anim.runtimeActive = anim.active;
				anim.runtimeVisible = anim.visible;
				if (!anim.roomName.empty() && !anim.resourcePath.empty() && !anim.animName.empty())
					_animations.push_back(anim);
				return;
			}

			if (tokens.size() < tagIndex + 13)
				return;

			StartupObjectRecord object;
			if (tagIndex >= 6) {
				object.initialX = atoi(tokens[0].c_str());
				object.initialY = atoi(tokens[1].c_str());
				object.boundsX2 = atoi(tokens[2].c_str());
				object.boundsY2 = atoi(tokens[3].c_str());
				object.initialZ = atoi(tokens[4].c_str());
				object.zExtent = atoi(tokens[5].c_str());
			}
			object.currentX = object.initialX;
			object.currentY = object.initialY;
			object.currentZ = object.initialZ;
			object.initialOwnerOrRoom = tokens[tagIndex + 1];
			object.objectName = tokens[tagIndex + 2];
			object.spritePath = resources.normalizeResourcePath(tokens[tagIndex + 3]);
			object.altSpritePath = resources.normalizeResourcePath(tokens[tagIndex + 4]);
			object.field40 = tokens[tagIndex + 5];
			object.inventoryTextKey = tokens[tagIndex + 6];
			object.field34 = tokens[tagIndex + 7];
			object.identTextKey = tokens[tagIndex + 8];
			object.operatable = tokens[tagIndex + 9].equalsIgnoreCase("T");
			object.visible = tokens[tagIndex + 10].equalsIgnoreCase("T");
			object.actionTag = tokens[tagIndex + 11];
			object.interactionLabel = tokens[tagIndex + 12];
			object.currentOwnerOrRoom = object.initialOwnerOrRoom;
			object.runtimeVisible = object.visible;
			object.identShown = object.identTextKey.empty();
			if (!object.initialOwnerOrRoom.empty() && !object.objectName.empty())
				_objects.push_back(object);
		};

	Common::String line;
	for (uint i = 0; i < _data.size(); ++i) {
		const char ch = (char)_data[i];
		if (ch == '\r')
			continue;
		if (ch == '\n') {
			parseLine(line);
			line.clear();
			continue;
		}

		line += ch;
	}

	parseLine(line);

	debug(1, "Harvester: parsed %u entrances, %u rooms, %u objects, %u anims, %u flags, %u commands, %u texts from '%s'",
		(uint)_entrances.size(), (uint)_rooms.size(), (uint)_objects.size(), (uint)_animations.size(),
		(uint)_flags.size(), (uint)_commands.size(), (uint)_texts.size(), _path.c_str());
}

bool StartupScript::resolveRoomSetupState(const Common::String &entranceName, StartupRoomSetupState &state,
		ResourceManager &resources) {
	(void)resources;
	state = StartupRoomSetupState();

	const StartupEntranceRecord *entrance = findEntranceRecord(entranceName);
	const Common::String resolvedRoomName = entrance ? entrance->roomName : entranceName;
	const StartupRoomRecord *room = findRoomRecord(resolvedRoomName);
	if (!room)
		return false;

	Common::String musicPath;
	Common::Array<StartupAudioCommand> audioCommands;
	bool mutatedRuntimeState = false;
	executeCommandChain(room->onEnterCommand, "room setup command", room->roomName, false,
		&musicPath, &audioCommands, nullptr, &mutatedRuntimeState);

	if (!buildRuntimeRoomState(*room, entrance, state))
		return false;

	state.audioCommands = audioCommands;
	if (!musicPath.empty())
		state.musicPath = musicPath;

	debugC(1, kDebugGeneral,
		"Harvester: resolveRoomSetupState('%s') -> room='%s' entrance='%s' spawn=(%d,%d,%d) facing=%d palette='%s' background='%s' music='%s' brightness=%.2f roomObjects=%u activeObjects=%u roomAnims=%u mutated=%d",
		entranceName.c_str(), state.roomName.c_str(), state.entranceName.c_str(),
		state.playerSpawnX, state.playerSpawnY, state.playerSpawnZ, state.playerFacing,
		state.palettePath.c_str(), state.backgroundPath.c_str(), state.musicPath.c_str(),
		(double)state.paletteBrightness, (uint)state.roomObjects.size(),
		(uint)state.activeObjects.size(), (uint)state.roomAnimations.size(), mutatedRuntimeState);

	return true;
}

void StartupScript::resetRuntimeState() {
	_runtimeFlags = _flags;
	_runtimeObjects = _objects;
	_runtimeAnimations = _animations;

	for (StartupObjectRecord &object : _runtimeObjects) {
		object.currentX = object.initialX;
		object.currentY = object.initialY;
		object.currentZ = object.initialZ;
		object.currentOwnerOrRoom = object.initialOwnerOrRoom;
		object.runtimeVisible = object.visible;
		object.identShown = object.identTextKey.empty();
	}

	for (StartupAnimRecord &anim : _runtimeAnimations) {
		anim.runtimeActive = anim.active;
		anim.runtimeVisible = anim.visible;
		anim.runtimeState = -1;
	}
}

bool StartupScript::materializeRoomState(const Common::String &entranceName, const Common::String &roomName,
		StartupRoomSetupState &state) const {
	state = StartupRoomSetupState();

	const StartupEntranceRecord *entrance = findEntranceRecord(entranceName);
	const Common::String resolvedRoomName = !roomName.empty()
		? roomName
		: (entrance ? entrance->roomName : Common::String());
	if (resolvedRoomName.empty())
		return false;

	const StartupRoomRecord *room = findRoomRecord(resolvedRoomName);
	if (!room)
		return false;

	return buildRuntimeRoomState(*room, entrance, state);
}

bool StartupScript::executeRoomExitCommands(const Common::String &roomName,
		Common::Array<StartupAudioCommand> &audioCommands) {
	audioCommands.clear();

	const StartupRoomRecord *room = findRoomRecord(roomName);
	if (!room)
		return false;

	bool mutatedRuntimeState = false;
	executeCommandChain(room->onExitCommand, "room exit command", room->roomName, false,
		nullptr, &audioCommands, nullptr, &mutatedRuntimeState);
	return true;
}

bool StartupScript::resolveObjectInteraction(const StartupObjectRecord &object, StartupInteractionResult &result) {
	result = StartupInteractionResult();
	if (object.actionTag.empty())
		return false;

	executeCommandChain(object.actionTag, "interaction command", object.objectName, true,
		&result.musicPath, &result.audioCommands, &result.nextRoomName, &result.mutatedRuntimeState);

	return !result.nextRoomName.empty() || !result.musicPath.empty() || !result.audioCommands.empty() ||
		result.mutatedRuntimeState || hasActionableCommandChain(object.actionTag);
}

bool StartupScript::hasObjectInteraction(const StartupObjectRecord &object) const {
	if (object.actionTag.empty())
		return false;

	return hasActionableCommandChain(object.actionTag);
}

void StartupScript::markObjectIdentShown(const StartupObjectRecord &object) {
	if (StartupObjectRecord *runtimeObject = findRuntimeObject(object.currentOwnerOrRoom, object.objectName))
		runtimeObject->identShown = true;
}

const StartupEntranceRecord *StartupScript::findEntranceRecord(const Common::String &entranceName) const {
	if (entranceName.empty())
		return nullptr;

	for (const StartupEntranceRecord &entrance : _entrances) {
		if (entrance.entranceName.equalsIgnoreCase(entranceName))
			return &entrance;
	}

	return nullptr;
}

const StartupRoomRecord *StartupScript::findRoomRecord(const Common::String &roomName) const {
	if (roomName.empty())
		return nullptr;

	for (const StartupRoomRecord &room : _rooms) {
		if (room.roomName.equalsIgnoreCase(roomName))
			return &room;
	}

	return nullptr;
}

const StartupCommandRecord *StartupScript::findCommandRecord(const Common::String &tag) const {
	if (tag.empty())
		return nullptr;

	for (const StartupCommandRecord &command : _commands) {
		if (command.triggerTag.equalsIgnoreCase(tag))
			return &command;
	}

	return nullptr;
}

const StartupFlagRecord *StartupScript::findRuntimeFlag(const Common::String &flagName) const {
	if (flagName.empty())
		return nullptr;

	for (const StartupFlagRecord &flag : _runtimeFlags) {
		if (flag.name.equalsIgnoreCase(flagName))
			return &flag;
	}

	return nullptr;
}

StartupFlagRecord *StartupScript::findRuntimeFlag(const Common::String &flagName) {
	if (flagName.empty())
		return nullptr;

	for (StartupFlagRecord &flag : _runtimeFlags) {
		if (flag.name.equalsIgnoreCase(flagName))
			return &flag;
	}

	return nullptr;
}

StartupObjectRecord *StartupScript::findRuntimeObject(const Common::String &ownerOrRoom,
		const Common::String &objectName) {
	if (objectName.empty())
		return nullptr;

	StartupObjectRecord *fallback = nullptr;
	for (StartupObjectRecord &object : _runtimeObjects) {
		if (!object.objectName.equalsIgnoreCase(objectName))
			continue;

		if (ownerOrRoom.empty())
			return &object;

		if (object.currentOwnerOrRoom.equalsIgnoreCase(ownerOrRoom) ||
			object.initialOwnerOrRoom.equalsIgnoreCase(ownerOrRoom)) {
			return &object;
		}

		if (!fallback)
			fallback = &object;
	}

	return fallback;
}

StartupAnimRecord *StartupScript::findRuntimeAnim(const Common::String &animName) {
	if (animName.empty())
		return nullptr;

	for (StartupAnimRecord &anim : _runtimeAnimations) {
		if (anim.animName.equalsIgnoreCase(animName))
			return &anim;
	}

	return nullptr;
}

bool StartupScript::buildRuntimeRoomState(const StartupRoomRecord &room, const StartupEntranceRecord *entrance,
		StartupRoomSetupState &state) const {
	state = StartupRoomSetupState();

	const StartupObjectRecord *background = nullptr;
	for (const StartupObjectRecord &candidate : _runtimeObjects) {
		if (!candidate.currentOwnerOrRoom.equalsIgnoreCase(room.roomName) ||
			!candidate.spritePath.hasPrefixIgnoreCase("GRAPHIC/ROOMS/") ||
			!candidate.spritePath.hasSuffixIgnoreCase(".BM")) {
			continue;
		}

		background = &candidate;
		break;
	}
	if (!background)
		return false;

	state.roomName = room.roomName;
	state.palettePath = room.palettePath;
	state.backgroundPath = background->spritePath;
	state.roomMinZ = room.minZ;
	state.roomMaxZ = room.maxZ;
	state.roomMaxZScreenY = room.maxZScreenY;
	state.roomMinZScreenY = room.minZScreenY;
	state.roomFullScaleZ = room.fullScaleZ;
	state.roomMaxZScalePercent = room.maxZScalePercent;
	state.roomPerspectiveScale = room.perspectiveScale;
	state.roomZVelocityStep = room.zVelocityStep;
	state.musicPath = room.musicPath;
	if (entrance && entrance->roomName.equalsIgnoreCase(room.roomName)) {
		state.entranceName = entrance->entranceName;
		state.hasEntrance = true;
		state.playerSpawnX = entrance->posX;
		state.playerSpawnY = entrance->posY;
		state.playerSpawnZ = entrance->posZ;
		state.playerFacing = entrance->facing;
	}
	for (const StartupObjectRecord &object : _runtimeObjects) {
		if (object.currentOwnerOrRoom.equalsIgnoreCase(room.roomName))
			state.roomObjects.push_back(object);
	}
	for (const StartupAnimRecord &anim : _runtimeAnimations) {
		if (anim.roomName.equalsIgnoreCase(room.roomName))
			state.roomAnimations.push_back(anim);
	}

	const StartupFlagRecord *dayFlag = findRuntimeFlag("DAY_FLAG");
	state.paletteBrightness = (room.dimmable && (!dayFlag || !dayFlag->value))
		? kDimmedPaletteBrightness
		: kDefaultPaletteBrightness;

	for (const StartupObjectRecord &object : state.roomObjects) {
		debugC(1, kDebugScene,
			"Harvester: materialized room object room='%s' object='%s' owner='%s' visible=%d runtimeVisible=%d sprite='%s' alt='%s' pos=(%d,%d,%d) bounds=(%d,%d)-(%d,%d) action='%s'",
			state.roomName.c_str(), object.objectName.c_str(), object.currentOwnerOrRoom.c_str(),
			object.visible, object.runtimeVisible, object.spritePath.c_str(), object.altSpritePath.c_str(),
			object.currentX, object.currentY, object.currentZ,
			object.currentX, object.currentY, object.boundsX2, object.boundsY2, object.actionTag.c_str());
	}

	debugC(1, kDebugGeneral,
		"Harvester: materializeRoomState room='%s' entrance='%s' spawn=(%d,%d,%d) facing=%d palette='%s' background='%s' music='%s' brightness=%.2f roomObjects=%u roomAnims=%u",
		state.roomName.c_str(), state.entranceName.c_str(), state.playerSpawnX, state.playerSpawnY,
		state.playerSpawnZ, state.playerFacing, state.palettePath.c_str(), state.backgroundPath.c_str(),
		state.musicPath.c_str(), (double)state.paletteBrightness,
		(uint)state.roomObjects.size(), (uint)state.roomAnimations.size());

	return true;
}

void StartupScript::executeCommandChain(const Common::String &initialTag, const char *contextLabel,
		const Common::String &contextName, bool allowTransitions, Common::String *musicPath,
		Common::Array<StartupAudioCommand> *audioCommands, Common::String *nextRoomName,
		bool *mutatedRuntimeState) {
	auto isTruthy = [](const Common::String &value) {
		return value.equalsIgnoreCase("T") || value.equalsIgnoreCase("ON") || value.equalsIgnoreCase("TRUE");
	};
	auto noteMutation = [&](bool changed) {
		if (changed && mutatedRuntimeState)
			*mutatedRuntimeState = true;
	};

	Common::String currentTag = initialTag;
	for (uint step = 0; step < 128 && !currentTag.empty(); ++step) {
		const StartupCommandRecord *command = findCommandRecord(currentTag);
		if (!command) {
			debug(1, "Harvester: unresolved %s tag '%s' for '%s'",
				contextLabel, currentTag.c_str(), contextName.c_str());
			break;
		}

		debugC(1, kDebugScene,
			"Harvester: %s '%s' step=%u tag='%s' opcode='%s' args=['%s','%s','%s','%s']",
			contextLabel, contextName.c_str(), step, currentTag.c_str(), command->opcodeName.c_str(),
			command->arg1.c_str(), command->arg2.c_str(), command->arg3.c_str(), command->arg4.c_str());

		if (command->opcodeName.equalsIgnoreCase("CHECK_FLAG")) {
			const StartupFlagRecord *flag = findRuntimeFlag(command->arg1);
			const bool flagValue = flag && flag->value;
			debugC(1, kDebugScene, "Harvester: %s '%s' flag '%s' -> %d",
				contextLabel, contextName.c_str(), command->arg1.c_str(), flagValue);
			currentTag = flagValue ? command->arg2 : command->arg3;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("SET_FLAG")) {
			const bool flagValue = isTruthy(command->arg2);
			StartupFlagRecord *flag = findRuntimeFlag(command->arg1);
			bool changed = false;
			if (flag) {
				changed = flag->value != flagValue;
				flag->value = flagValue;
			} else {
				StartupFlagRecord newFlag;
				newFlag.name = command->arg1;
				newFlag.value = flagValue;
				_runtimeFlags.push_back(newFlag);
				changed = true;
			}
			noteMutation(changed);
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("SPOOL_MUSIC")) {
			if (musicPath)
				*musicPath = command->arg1;
			currentTag = command->arg4;
			continue;
		}

		if (audioCommands) {
			if (appendStartupAudioCommand(*command, *audioCommands)) {
				currentTag = command->arg4;
				continue;
			}
		} else {
			Common::Array<StartupAudioCommand> ignoredAudioCommands;
			if (appendStartupAudioCommand(*command, ignoredAudioCommands)) {
				currentTag = command->arg4;
				continue;
			}
		}

		if (command->opcodeName.equalsIgnoreCase("ADD") ||
			command->opcodeName.equalsIgnoreCase("DELETE")) {
			StartupObjectRecord *runtimeObject = findRuntimeObject(command->arg1, command->arg2);
			if (!runtimeObject) {
				debug(1, "Harvester: unresolved object for %s '%s' owner='%s' object='%s'",
					contextLabel, contextName.c_str(), command->arg1.c_str(), command->arg2.c_str());
				currentTag = command->arg4;
				continue;
			}

			const bool visible = command->opcodeName.equalsIgnoreCase("ADD");
			const bool changed = runtimeObject->visible != visible ||
				runtimeObject->runtimeVisible != visible;
			runtimeObject->visible = visible;
			runtimeObject->runtimeVisible = visible;
			if (visible && runtimeObject->currentOwnerOrRoom.equalsIgnoreCase("INVENTORY"))
				runtimeObject->identShown = true;
			noteMutation(changed);
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("SET_ANIM")) {
			StartupAnimRecord *runtimeAnim = findRuntimeAnim(command->arg1);
			if (!runtimeAnim) {
				debug(1, "Harvester: unresolved anim for %s '%s' anim='%s'",
					contextLabel, contextName.c_str(), command->arg1.c_str());
				currentTag = command->arg4;
				continue;
			}

			const bool active = isTruthy(command->arg2);
			const bool visible = isTruthy(command->arg3);
			const bool changed = runtimeAnim->active != active ||
				runtimeAnim->visible != visible ||
				runtimeAnim->runtimeActive != active ||
				runtimeAnim->runtimeVisible != visible;
			runtimeAnim->active = active;
			runtimeAnim->visible = visible;
			runtimeAnim->runtimeActive = active;
			runtimeAnim->runtimeVisible = visible;
			noteMutation(changed);
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("CLOSEUP") ||
			command->opcodeName.equalsIgnoreCase("CHANGE_ROOM")) {
			if (allowTransitions && nextRoomName)
				*nextRoomName = command->arg1;
			else
				debugC(1, kDebugScene, "Harvester: skipped transition opcode '%s' while processing %s '%s'",
					command->opcodeName.c_str(), contextLabel, contextName.c_str());
			return;
		}

		debug(1, "Harvester: unsupported startup command '%s' for %s '%s', continuing",
			command->opcodeName.c_str(), contextLabel, contextName.c_str());
		currentTag = command->arg4;
	}
}

bool StartupScript::hasActionableCommandChain(const Common::String &initialTag) const {
	Common::String currentTag = initialTag;
	for (uint step = 0; step < 128 && !currentTag.empty(); ++step) {
		const StartupCommandRecord *command = findCommandRecord(currentTag);
		if (!command) {
			debug(1, "Harvester: unresolved interaction probe tag '%s'", currentTag.c_str());
			break;
		}

		if (command->opcodeName.equalsIgnoreCase("CHECK_FLAG")) {
			const StartupFlagRecord *flag = findRuntimeFlag(command->arg1);
			currentTag = (flag && flag->value) ? command->arg2 : command->arg3;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("SET_FLAG") ||
			command->opcodeName.equalsIgnoreCase("SPOOL_MUSIC") ||
			command->opcodeName.equalsIgnoreCase("ADD") ||
			command->opcodeName.equalsIgnoreCase("DELETE") ||
			command->opcodeName.equalsIgnoreCase("SET_ANIM") ||
			command->opcodeName.equalsIgnoreCase("CLOSEUP") ||
			command->opcodeName.equalsIgnoreCase("CHANGE_ROOM")) {
			return true;
		}

		Common::Array<StartupAudioCommand> audioCommands;
		if (appendStartupAudioCommand(*command, audioCommands))
			return true;

		currentTag = command->arg4;
	}

	return false;
}

const StartupTextRecord *StartupScript::findTextRecord(const Common::String &key) const {
	if (key.empty())
		return nullptr;

	for (const StartupTextRecord &textRecord : _texts) {
		if (textRecord.key.equalsIgnoreCase(key))
			return &textRecord;
	}

	return nullptr;
}

bool StartupScript::resolveObjectInspectText(const StartupObjectRecord &object, StartupResolvedText &text) const {
	text = StartupResolvedText();

	const StartupTextRecord *textRecord = findTextRecord(object.identTextKey);
	if (!textRecord)
		return false;

	text.boxName = textRecord->boxName;
	text.value = textRecord->value;
	return !text.value.empty();
}

Common::String StartupScript::resolveObjectLabel(const StartupObjectRecord &object) const {
	if (!object.interactionLabel.empty() && !object.interactionLabel.equalsIgnoreCase("NULL_ID"))
		return normalizeInteractionLabel(object.interactionLabel);

	const StartupTextRecord *textRecord = findTextRecord(object.identTextKey);
	if (textRecord && !textRecord->value.empty())
		return textRecord->value;

	Common::String label = normalizeInteractionLabel(object.objectName);
	if (!label.empty() && !label.equalsIgnoreCase("NULL ID"))
		return label;

	return Common::String();
}

Common::String StartupScript::resolveTextValue(const Common::String &key) const {
	const StartupTextRecord *textRecord = findTextRecord(key);
	if (textRecord && !textRecord->value.empty())
		return textRecord->value;

	return normalizeInteractionLabel(key);
}

} // End of namespace Harvester
