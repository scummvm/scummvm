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
#include "harvester/resources.h"

namespace Harvester {

static const char *const kStartupConfigPath = "CONFIG.INI";
static const char *const kConfigSectionName = "harvester";
static const char *const kDefaultTownScript = "HARVEST.SCR";
static const byte kTownScriptXorKey = 0xaa;

static Common::String trimAsciiLine(const Common::String &value) {
	uint start = 0;
	uint end = value.size();

	while (start < end && (value[start] == ' ' || value[start] == '\t'))
		++start;
	while (end > start && (value[end - 1] == ' ' || value[end - 1] == '\t' || value[end - 1] == '\r'))
		--end;

	return value.substr(start, end - start);
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
			if (tokens[i] == "ENTRANCE" || tokens[i] == "ROOM" || tokens[i] == "OBJECT" ||
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
			entrance.direction = tokens[tagIndex + 1];
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
			room.roomName = tokens[tagIndex + 1];
			room.palettePath = resources.normalizeResourcePath(tokens[tagIndex + 6]);
			room.dimmable = tokens[tagIndex + 7].equalsIgnoreCase("T");
			room.onEnterCommand = tokens[tagIndex + 8];
			room.onExitCommand = tokens[tagIndex + 9];
			if (!room.roomName.empty())
				_rooms.push_back(room);
			return;
		}

		if (tokens.size() < tagIndex + 13)
			return;

		StartupObjectRecord object;
		if (tagIndex >= 4) {
			object.left = atoi(tokens[0].c_str());
			object.top = atoi(tokens[1].c_str());
			object.right = atoi(tokens[2].c_str());
			object.bottom = atoi(tokens[3].c_str());
		}
		object.ownerOrRoom = tokens[tagIndex + 1];
		object.objectName = tokens[tagIndex + 2];
		object.resourcePath = resources.normalizeResourcePath(tokens[tagIndex + 3]);
		object.inventoryBitmapPath = resources.normalizeResourcePath(tokens[tagIndex + 4]);
		object.shortTextKey = tokens[tagIndex + 6];
		object.identTextKey = tokens[tagIndex + 8];
		object.visible = tokens[tagIndex + 9].equalsIgnoreCase("T");
		object.active = tokens[tagIndex + 10].equalsIgnoreCase("T");
		object.interactionCommandTag = tokens[tagIndex + 11];
		object.displayName = tokens[tagIndex + 12];
		if (!object.ownerOrRoom.empty() && !object.objectName.empty())
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

	debug(1, "Harvester: parsed %u entrances, %u rooms, %u objects, %u flags, %u commands, %u texts from '%s'",
		(uint)_entrances.size(), (uint)_rooms.size(), (uint)_objects.size(),
		(uint)_flags.size(), (uint)_commands.size(), (uint)_texts.size(), _path.c_str());
}

bool StartupScript::resolveRoomSetupState(const Common::String &entranceName, StartupRoomSetupState &state,
		ResourceManager &resources) const {
	state = StartupRoomSetupState();

	const StartupEntranceRecord *entrance = nullptr;
	for (const StartupEntranceRecord &candidate : _entrances) {
		if (candidate.entranceName.equalsIgnoreCase(entranceName)) {
			entrance = &candidate;
			break;
		}
	}

	Common::String resolvedRoomName;
	if (entrance)
		resolvedRoomName = entrance->roomName;
	else
		resolvedRoomName = entranceName;

	const StartupRoomRecord *room = nullptr;
	for (const StartupRoomRecord &candidate : _rooms) {
		if (candidate.roomName.equalsIgnoreCase(resolvedRoomName)) {
			room = &candidate;
			break;
		}
	}
	if (!room)
		return false;

	const StartupObjectRecord *background = nullptr;
	for (const StartupObjectRecord &candidate : _objects) {
		if (!candidate.ownerOrRoom.equalsIgnoreCase(room->roomName) ||
			!candidate.resourcePath.hasPrefixIgnoreCase("GRAPHIC/ROOMS/") ||
			!candidate.resourcePath.hasSuffixIgnoreCase(".BM")) {
			continue;
		}

		background = &candidate;
		break;
	}
	if (!background)
		return false;

	state.roomName = room->roomName;
	state.palettePath = room->palettePath;
	state.backgroundPath = background->resourcePath;
	for (const StartupObjectRecord &object : _objects) {
		if (object.ownerOrRoom.equalsIgnoreCase(room->roomName))
			state.roomObjects.push_back(object);
	}

	Common::Array<StartupFlagRecord> resolvedFlags = _flags;
	auto getFlagValue = [&](const Common::String &flagName) {
		for (const StartupFlagRecord &flag : resolvedFlags) {
			if (flag.name.equalsIgnoreCase(flagName))
				return flag.value;
		}
		return false;
	};
	auto setFlagValue = [&](const Common::String &flagName, bool value) {
		for (StartupFlagRecord &flag : resolvedFlags) {
			if (flag.name.equalsIgnoreCase(flagName)) {
				flag.value = value;
				return;
			}
		}

		StartupFlagRecord flag;
		flag.name = flagName;
		flag.value = value;
		resolvedFlags.push_back(flag);
	};
	auto addObject = [&](const Common::String &ownerOrRoom, const Common::String &objectName) {
		for (const StartupObjectRecord &candidate : _objects) {
			if (!candidate.ownerOrRoom.equalsIgnoreCase(ownerOrRoom) ||
				!candidate.objectName.equalsIgnoreCase(objectName) ||
				candidate.resourcePath.empty() ||
				!candidate.resourcePath.hasSuffixIgnoreCase(".BM")) {
				continue;
			}

			for (const StartupObjectRecord &activeObject : state.activeObjects) {
				if (activeObject.ownerOrRoom.equalsIgnoreCase(candidate.ownerOrRoom) &&
					activeObject.objectName.equalsIgnoreCase(candidate.objectName)) {
					return;
				}
			}

			state.activeObjects.push_back(candidate);
			return;
		}
	};
	auto removeObject = [&](const Common::String &ownerOrRoom, const Common::String &objectName) {
		for (uint i = 0; i < state.activeObjects.size(); ++i) {
			if (state.activeObjects[i].ownerOrRoom.equalsIgnoreCase(ownerOrRoom) &&
				state.activeObjects[i].objectName.equalsIgnoreCase(objectName)) {
				state.activeObjects.remove_at(i);
				return;
			}
		}
	};
	auto findCommand = [&](const Common::String &tag) -> const StartupCommandRecord * {
		for (const StartupCommandRecord &command : _commands) {
			if (command.triggerTag.equalsIgnoreCase(tag))
				return &command;
		}
		return nullptr;
	};

	Common::String currentTag = room->onEnterCommand;
	for (uint step = 0; step < 128 && !currentTag.empty(); ++step) {
		const StartupCommandRecord *command = findCommand(currentTag);
		if (!command) {
			debug(1, "Harvester: unresolved startup command tag '%s' for room '%s'",
				currentTag.c_str(), room->roomName.c_str());
			break;
		}

		if (command->opcodeName.equalsIgnoreCase("CHECK_FLAG")) {
			currentTag = getFlagValue(command->arg1) ? command->arg2 : command->arg3;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("SET_FLAG")) {
			setFlagValue(command->arg1, command->arg2.equalsIgnoreCase("T"));
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("SPOOL_MUSIC")) {
			state.musicPath = resources.normalizeResourcePath(command->arg1);
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("ADD")) {
			addObject(command->arg1, command->arg2);
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("DELETE")) {
			removeObject(command->arg1, command->arg2);
			currentTag = command->arg4;
			continue;
		}

		debug(1, "Harvester: unsupported startup command '%s' for tag '%s'",
			command->opcodeName.c_str(), command->triggerTag.c_str());
		break;
	}

	return true;
}

bool StartupScript::resolveObjectInteraction(const StartupObjectRecord &object, StartupInteractionResult &result) const {
	result = StartupInteractionResult();
	if (object.interactionCommandTag.empty())
		return false;

	Common::Array<StartupFlagRecord> resolvedFlags = _flags;
	auto getFlagValue = [&](const Common::String &flagName) {
		for (const StartupFlagRecord &flag : resolvedFlags) {
			if (flag.name.equalsIgnoreCase(flagName))
				return flag.value;
		}
		return false;
	};
	auto setFlagValue = [&](const Common::String &flagName, bool value) {
		for (StartupFlagRecord &flag : resolvedFlags) {
			if (flag.name.equalsIgnoreCase(flagName)) {
				flag.value = value;
				return;
			}
		}

		StartupFlagRecord flag;
		flag.name = flagName;
		flag.value = value;
		resolvedFlags.push_back(flag);
	};
	auto findCommand = [&](const Common::String &tag) -> const StartupCommandRecord * {
		for (const StartupCommandRecord &command : _commands) {
			if (command.triggerTag.equalsIgnoreCase(tag))
				return &command;
		}
		return nullptr;
	};

	Common::String currentTag = object.interactionCommandTag;
	for (uint step = 0; step < 128 && !currentTag.empty(); ++step) {
		const StartupCommandRecord *command = findCommand(currentTag);
		if (!command) {
			debug(1, "Harvester: unresolved interaction command tag '%s' for object '%s'",
				currentTag.c_str(), object.objectName.c_str());
			break;
		}

		if (command->opcodeName.equalsIgnoreCase("CHECK_FLAG")) {
			currentTag = getFlagValue(command->arg1) ? command->arg2 : command->arg3;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("SET_FLAG")) {
			setFlagValue(command->arg1, command->arg2.equalsIgnoreCase("T"));
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("START_WAV") ||
			command->opcodeName.equalsIgnoreCase("LOAD_WAV") ||
			command->opcodeName.equalsIgnoreCase("START_SINGLE_WAV")) {
			if (result.soundPath.empty())
				result.soundPath = command->arg1;
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("SPOOL_MUSIC")) {
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("CLOSEUP") ||
			command->opcodeName.equalsIgnoreCase("CHANGE_ROOM")) {
			result.nextRoomName = command->arg1;
			return !result.nextRoomName.empty() || !result.soundPath.empty();
		}

		debug(1, "Harvester: unsupported interaction command '%s' for tag '%s'",
			command->opcodeName.c_str(), command->triggerTag.c_str());
		break;
	}

	return !result.nextRoomName.empty() || !result.soundPath.empty();
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
	const StartupTextRecord *textRecord = findTextRecord(object.shortTextKey);
	if (textRecord && !textRecord->value.empty())
		return textRecord->value;

	Common::String label = object.displayName;
	for (uint i = 0; i < label.size(); ++i) {
		if (label[i] == '_')
			label.setChar(' ', i);
	}
	if (!label.empty())
		return label;

	textRecord = findTextRecord(object.identTextKey);
	if (textRecord)
		return textRecord->value;

	return Common::String();
}

} // End of namespace Harvester
