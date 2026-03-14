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
			if (tokens[i] == "ENTRANCE" || tokens[i] == "ROOM" || tokens[i] == "OBJECT") {
				tagIndex = i;
				break;
			}
		}
		if (tagIndex == tokens.size())
			return;

		const Common::String &tag = tokens[tagIndex];
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
		object.ownerOrRoom = tokens[tagIndex + 1];
		object.objectName = tokens[tagIndex + 2];
		object.resourcePath = resources.normalizeResourcePath(tokens[tagIndex + 3]);
		object.visible = tokens[tagIndex + 9].equalsIgnoreCase("T");
		object.active = tokens[tagIndex + 10].equalsIgnoreCase("T");
		object.identTextKey = tokens[tagIndex + 11];
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

	debug(1, "Harvester: parsed %u entrances, %u rooms, %u objects from '%s'",
		(uint)_entrances.size(), (uint)_rooms.size(), (uint)_objects.size(), _path.c_str());
}

bool StartupScript::resolveRoomSetup(const Common::String &entranceName, Common::String &roomName,
		Common::String &palettePath, Common::String &backgroundPath) const {
	roomName.clear();
	palettePath.clear();
	backgroundPath.clear();

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
			!candidate.objectName.equalsIgnoreCase(room->roomName) ||
			!candidate.resourcePath.hasPrefixIgnoreCase("GRAPHIC/ROOMS/") ||
			!candidate.resourcePath.hasSuffixIgnoreCase(".BM")) {
			continue;
		}

		background = &candidate;
		break;
	}
	if (!background)
		return false;

	roomName = room->roomName;
	palettePath = room->palettePath;
	backgroundPath = background->resourcePath;
	return true;
}

} // End of namespace Harvester
