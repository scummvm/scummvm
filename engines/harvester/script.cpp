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

#include "harvester/script.h"

#include <cstdlib>

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/formats/ini-file.h"
#include "common/memstream.h"
#include "common/serializer.h"
#include "harvester/detection.h"
#include "harvester/resources.h"

namespace Harvester {

static const char *const kStartupConfigPath = "CONFIG.INI";
static const char *const kConfigSectionName = "harvester";
static const char *const kDefaultTownScript = "HARVEST.SCR";
static const char *const kInventoryOwnerName = "INVENTORY";
static const char *const kDefaultVoicePath = "SOUND/VOICE/";
static const byte kTownScriptXorKey = 0xaa;
static const float kDefaultPaletteBrightness = 1.0f;
static const float kDimmedPaletteBrightness = 0.6f;
static const int kDefaultPlayerHitPoints = 30;
static const int kMaxStartupOptionLevel = 9;

static void syncBool(Common::Serializer &s, bool &value) {
	byte serialized = value ? 1 : 0;
	s.syncAsByte(serialized);
	if (s.isLoading())
		value = serialized != 0;
}

template <typename RecordType, typename SyncFn>
static void syncRecordArray(Common::Serializer &s, Common::Array<RecordType> &records, SyncFn syncFn) {
	uint32 count = (uint32)records.size();
	s.syncAsUint32LE(count);
	if (s.isLoading())
		records.resize(count);
	for (uint32 i = 0; i < count; ++i)
		syncFn(s, records[i]);
}

static void syncStartupFlagRecord(Common::Serializer &s, StartupFlagRecord &record) {
	s.syncString(record.name);
	syncBool(s, record.value);
}

static void syncStartupObjectRecord(Common::Serializer &s, StartupObjectRecord &record) {
	s.syncAsSint32LE(record.initialX);
	s.syncAsSint32LE(record.initialY);
	s.syncAsSint32LE(record.initialZ);
	s.syncAsSint32LE(record.currentX);
	s.syncAsSint32LE(record.currentY);
	s.syncAsSint32LE(record.currentZ);
	s.syncAsSint32LE(record.boundsX2);
	s.syncAsSint32LE(record.boundsY2);
	s.syncAsSint32LE(record.zExtent);
	s.syncString(record.actionTag);
	s.syncString(record.spritePath);
	s.syncString(record.altSpritePath);
	s.syncString(record.objectName);
	s.syncString(record.field34);
	s.syncString(record.identTextKey);
	s.syncString(record.currentOwnerOrRoom);
	s.syncString(record.field40);
	s.syncString(record.inventoryTextKey);
	s.syncString(record.initialOwnerOrRoom);
	s.syncString(record.interactionLabel);
	syncBool(s, record.operatable);
	syncBool(s, record.visible);
	syncBool(s, record.runtimeVisible);
	syncBool(s, record.identShown);
}

static void syncStartupAnimRecord(Common::Serializer &s, StartupAnimRecord &record) {
	s.syncAsSint32LE(record.x);
	s.syncAsSint32LE(record.y);
	s.syncAsSint32LE(record.z);
	s.syncAsSint32LE(record.frameDelay);
	s.syncString(record.roomName);
	s.syncString(record.resourcePath);
	s.syncString(record.animName);
	syncBool(s, record.active);
	syncBool(s, record.visible);
	syncBool(s, record.looping);
	syncBool(s, record.backward);
	syncBool(s, record.pingPong);
	syncBool(s, record.remove);
	syncBool(s, record.runtimeActive);
	syncBool(s, record.runtimeVisible);
	s.syncAsSint32LE(record.runtimeState);
}

static void syncStartupNpcRecord(Common::Serializer &s, StartupNpcRecord &record) {
	s.syncAsSint32LE(record.posX);
	s.syncAsSint32LE(record.posY);
	s.syncAsSint32LE(record.posZ);
	s.syncAsSint32LE(record.frameDelay);
	s.syncString(record.onDeathActionTag);
	s.syncString(record.modelPath);
	s.syncString(record.npcName);
	s.syncString(record.monsterfyTargetName);
	s.syncString(record.roomName);
	syncBool(s, record.deathOrMonsterfyFlag);
	syncBool(s, record.runtimeSpawned);
	syncBool(s, record.active);
	syncBool(s, record.visible);
	syncBool(s, record.savedVisible);
	s.syncAsSint32LE(record.deathDamageType);
	s.syncString(record.audioPath);
	s.syncString(record.entityInitArg);
}

static void syncStartupMonsterRecord(Common::Serializer &s, StartupMonsterRecord &record) {
	s.syncAsSint32LE(record.posX);
	s.syncAsSint32LE(record.posY);
	s.syncAsSint32LE(record.posZ);
	s.syncAsSint32LE(record.initialFacing);
	s.syncString(record.roomName);
	s.syncString(record.monsterName);
	s.syncString(record.modelPath);
	s.syncString(record.onDeathActionTag);
	syncBool(s, record.active);
	syncBool(s, record.visible);
}

static void syncStartupRegionRecord(Common::Serializer &s, StartupRegionRecord &record) {
	s.syncAsSint32LE(record.left);
	s.syncAsSint32LE(record.top);
	s.syncAsSint32LE(record.right);
	s.syncAsSint32LE(record.bottom);
	s.syncAsSint32LE(record.minZ);
	s.syncAsSint32LE(record.maxZ);
	s.syncAsSint32LE(record.desiredFacing);
	s.syncString(record.regionName);
	s.syncString(record.direction);
	s.syncString(record.roomName);
	s.syncString(record.actionTag);
	syncBool(s, record.startEnabled);
	syncBool(s, record.cursorEnabled);
}

static int clampStartupOptionLevel(int level) {
	if (level < 0)
		return 0;
	if (level > kMaxStartupOptionLevel)
		return kMaxStartupOptionLevel;

	return level;
}

static int clampPlayerHitPoints(int hitPoints) {
	if (hitPoints < 0)
		return 0;
	if (hitPoints > kDefaultPlayerHitPoints)
		return kDefaultPlayerHitPoints;

	return hitPoints;
}

static const char *resolveInventoryStatusObjectName(int hitPoints) {
	if (hitPoints < 8)
		return "INV_STAT4";
	if (hitPoints < 15)
		return "INV_STAT3";
	if (hitPoints < 23)
		return "INV_STAT2";

	return "INV_STAT1";
}

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

static int parseNpcDeathDamageType(const Common::String &value) {
	if (value.equalsIgnoreCase("BLUDGE"))
		return 1;
	if (value.equalsIgnoreCase("SLASH"))
		return 2;
	if (value.equalsIgnoreCase("PROJ"))
		return 4;

	return 0;
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

static bool loadBitmapDimensions(ResourceManager &resources, const Common::String &path,
		uint32 &width, uint32 &height) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 8)
		return false;

	width = READ_LE_UINT32(data.data());
	height = READ_LE_UINT32(data.data() + 4);
	return true;
}

static bool isRoomBackgroundObject(const StartupObjectRecord &candidate, const StartupRoomRecord &room,
		ResourceManager &resources) {
	if (!candidate.currentOwnerOrRoom.equalsIgnoreCase(room.roomName) ||
			candidate.spritePath.empty() ||
			candidate.initialX != 0 || candidate.initialY != 0) {
		return false;
	}

	uint32 width = 0;
	uint32 height = 0;
	if (!loadBitmapDimensions(resources, candidate.spritePath, width, height))
		return false;

	return width == 640 && height == 480;
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
	_regions.clear();
	_flags.clear();
	_commands.clear();
	_texts.clear();
	_heads.clear();
	_useItems.clear();
	_runtimeRegions.clear();
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
		_fxVolumeLevel = clampStartupOptionLevel(atoi(fxVolume.c_str()));

	Common::String musicVolume;
	if (config.getKey("MUSIC_VOLUME", kConfigSectionName, musicVolume))
		_musicVolumeLevel = clampStartupOptionLevel(atoi(musicVolume.c_str()));

	Common::String gammaValue;
	if (config.getKey("GAMMA", kConfigSectionName, gammaValue))
		_gammaLevel = clampStartupOptionLevel(atoi(gammaValue.c_str()));

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
	const Common::Path configPath = ConfMan.getPath("path").join(kStartupConfigPath, Common::Path::kNoSeparator);
	if (!file.open(configPath)) {
		warning("Harvester: unable to save startup config '%s'", configPath.toString(Common::Path::kNativeSeparator).c_str());
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

void Script::decode() {
	for (byte &value : _data) {
		if (value == '\r' || value == '\n')
			continue;

		value ^= kTownScriptXorKey;
	}
}

void Script::parseTownRecords(ResourceManager &resources) {
	_entrances.clear();
	_mapEntrances.clear();
	_mapLocations.clear();
	_rooms.clear();
	_objects.clear();
	_animations.clear();
	_npcs.clear();
	_regions.clear();
	_flags.clear();
	_commands.clear();
	_texts.clear();
	_heads.clear();
	_useItems.clear();

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
			if (tokens[i] == "ANIM" || tokens[i] == "ENTRANCE" || tokens[i] == "MAP_ENTRANCE" ||
				tokens[i] == "MAP_LOCATION" || tokens[i] == "ROOM" || tokens[i] == "OBJECT" ||
				tokens[i] == "NPC" || tokens[i] == "REGION" || tokens[i] == "HEAD" ||
				tokens[i] == "FLAG" || tokens[i] == "COMMAND" || tokens[i] == "TEXT" || tokens[i] == "USEITEM") {
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

		if (tag == "HEAD") {
			if (tokens.size() < tagIndex + 3)
				return;

			StartupHeadRecord head;
			head.headId = tokens[tagIndex + 1];
			head.portraitPath = resources.normalizeResourcePath(tokens[tagIndex + 2]);
			if (!head.headId.empty() && !head.portraitPath.empty())
				_heads.push_back(head);
			return;
		}

		if (tag == "USEITEM") {
			if (tokens.size() < tagIndex + 5)
				return;

			StartupUseItemRecord useItem;
			useItem.itemName = tokens[tagIndex + 1];
			useItem.ownerOrRoom = tokens[tagIndex + 2];
			useItem.targetName = tokens[tagIndex + 3];
			useItem.actionTag = tokens[tagIndex + 4];
			if (!useItem.itemName.empty() && !useItem.targetName.empty())
				_useItems.push_back(useItem);
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

		if (tag == "MAP_ENTRANCE") {
			if (tokens.size() < tagIndex + 2)
				return;

			StartupMapEntranceRecord mapEntrance;
			if (tagIndex >= 3) {
				mapEntrance.field0 = atoi(tokens[0].c_str());
				mapEntrance.field4 = atoi(tokens[1].c_str());
				mapEntrance.initialPanelIndex = atoi(tokens[2].c_str());
			}
			mapEntrance.entryName = tokens[tagIndex + 1];
			if (!mapEntrance.entryName.empty())
				_mapEntrances.push_back(mapEntrance);
			return;
		}

		if (tag == "MAP_LOCATION") {
			if (tokens.size() < tagIndex + 3)
				return;

			StartupMapLocationRecord mapLocation;
			if (tagIndex >= 7) {
				mapLocation.minX = atoi(tokens[0].c_str());
				mapLocation.minY = atoi(tokens[1].c_str());
				mapLocation.maxX = atoi(tokens[2].c_str());
				mapLocation.maxY = atoi(tokens[3].c_str());
				mapLocation.panelIndex = atoi(tokens[4].c_str());
				mapLocation.labelX = atoi(tokens[5].c_str());
				mapLocation.labelY = atoi(tokens[6].c_str());
			}
			mapLocation.labelText = tokens[tagIndex + 1];
			for (uint i = 0; i < mapLocation.labelText.size(); ++i) {
				if (mapLocation.labelText[i] == '_')
					mapLocation.labelText.setChar(' ', i);
			}
			mapLocation.destinationEntranceName = tokens[tagIndex + 2];
			if (!mapLocation.destinationEntranceName.empty())
				_mapLocations.push_back(mapLocation);
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

		if (tag == "NPC") {
			if (tokens.size() < tagIndex + 10)
				return;

			StartupNpcRecord npc;
			if (tagIndex >= 4) {
				npc.posX = atoi(tokens[0].c_str());
				npc.posY = atoi(tokens[1].c_str());
				npc.posZ = atoi(tokens[2].c_str());
				npc.frameDelay = atoi(tokens[3].c_str());
			}
			npc.roomName = tokens[tagIndex + 1];
			npc.modelPath = resources.normalizeResourcePath(tokens[tagIndex + 2]);
			npc.npcName = tokens[tagIndex + 3];
			npc.monsterfyTargetName = tokens[tagIndex + 4];
			npc.active = tokens[tagIndex + 5].equalsIgnoreCase("T");
			npc.visible = tokens[tagIndex + 6].equalsIgnoreCase("T");
			if (npc.active)
				npc.visible = true;
			npc.savedVisible = npc.visible;
			npc.onDeathActionTag = tokens[tagIndex + 7];
			npc.audioPath = resources.normalizeResourcePath(tokens[tagIndex + 8]);
			npc.entityInitArg = tokens[tagIndex + 9];
			if (!npc.roomName.empty() && !npc.modelPath.empty() && !npc.npcName.empty())
				_npcs.push_back(npc);
			return;
		}

		if (tag == "MONSTER") {
			if (tokens.size() < tagIndex + 24)
				return;

			StartupMonsterRecord monster;
			if (tagIndex >= 3) {
				monster.posX = atoi(tokens[0].c_str());
				monster.posY = atoi(tokens[1].c_str());
				monster.posZ = atoi(tokens[2].c_str());
			}
			monster.roomName = tokens[tagIndex + 1];
			monster.monsterName = tokens[tagIndex + 2];
			monster.modelPath = resources.normalizeResourcePath(tokens[tagIndex + 3]);
			monster.initialFacing = parseEntranceFacing(tokens[tagIndex + 8]);
			monster.active = tokens[tagIndex + 19].equalsIgnoreCase("T");
			monster.visible = tokens[tagIndex + 20].equalsIgnoreCase("T");
			monster.onDeathActionTag = tokens[tagIndex + 21];
			if (!monster.roomName.empty() && !monster.monsterName.empty() && !monster.modelPath.empty())
				_monsters.push_back(monster);
			return;
		}

		if (tag == "REGION") {
			if (tokens.size() < tagIndex + 7)
				return;

			StartupRegionRecord region;
			if (tagIndex >= 6) {
				region.left = atoi(tokens[0].c_str());
				region.top = atoi(tokens[1].c_str());
				region.right = atoi(tokens[2].c_str());
				region.bottom = atoi(tokens[3].c_str());
				region.minZ = atoi(tokens[4].c_str());
				region.maxZ = atoi(tokens[5].c_str());
			}
			region.regionName = tokens[tagIndex + 1];
			region.direction = tokens[tagIndex + 2];
			region.desiredFacing = parseEntranceFacing(region.direction);
			region.roomName = tokens[tagIndex + 3];
			region.actionTag = tokens[tagIndex + 4];
			region.startEnabled = tokens[tagIndex + 5].equalsIgnoreCase("T");
			region.cursorEnabled = tokens[tagIndex + 6].equalsIgnoreCase("T");
			if (!region.roomName.empty() && !region.regionName.empty())
				_regions.push_back(region);
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

	debug(1, "Harvester: parsed %u entrances, %u map entrances, %u map locations, %u rooms, %u objects, %u anims, %u npcs, %u regions, %u flags, %u commands, %u texts, %u heads, %u useitems from '%s'",
		(uint)_entrances.size(), (uint)_mapEntrances.size(), (uint)_mapLocations.size(),
		(uint)_rooms.size(), (uint)_objects.size(), (uint)_animations.size(),
		(uint)_npcs.size(), (uint)_regions.size(),
		(uint)_flags.size(), (uint)_commands.size(), (uint)_texts.size(), (uint)_heads.size(),
		(uint)_useItems.size(), _path.c_str());
}

bool Script::resolveRoomSetupState(const Common::String &entranceName, StartupRoomSetupState &state,
		ResourceManager &resources) {
	state = StartupRoomSetupState();

	const StartupEntranceRecord *entrance = findEntranceRecord(entranceName);
	const Common::String resolvedRoomName = entrance ? entrance->roomName : entranceName;
	const StartupRoomRecord *room = findRoomRecord(resolvedRoomName);
	if (!room) {
		warning("Harvester: unresolved room setup target '%s' (resolved room '%s')",
			entranceName.c_str(), resolvedRoomName.c_str());
		return false;
	}

	Common::String musicPath;
	Common::Array<StartupAudioCommand> audioCommands;
	bool mutatedRuntimeState = false;
	executeCommandChain(room->onEnterCommand, "room setup command", room->roomName, false,
		&musicPath, &audioCommands, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		&mutatedRuntimeState);

	if (!buildRuntimeRoomState(*room, entrance, resources, state))
		return false;

	state.audioCommands = audioCommands;
	if (!musicPath.empty())
		state.musicPath = musicPath;

	debugC(1, kDebugGeneral,
		"Harvester: resolveRoomSetupState('%s') -> room='%s' entrance='%s' spawn=(%d,%d,%d) facing=%d palette='%s' background='%s' music='%s' brightness=%.2f roomObjects=%u activeObjects=%u roomAnims=%u roomNpcs=%u roomMonsters=%u roomRegions=%u mutated=%d",
		entranceName.c_str(), state.roomName.c_str(), state.entranceName.c_str(),
		state.playerSpawnX, state.playerSpawnY, state.playerSpawnZ, state.playerFacing,
		state.palettePath.c_str(), state.backgroundPath.c_str(), state.musicPath.c_str(),
		(double)state.paletteBrightness, (uint)state.roomObjects.size(),
		(uint)state.activeObjects.size(), (uint)state.roomAnimations.size(), (uint)state.roomNpcs.size(),
		(uint)state.roomMonsters.size(), (uint)state.roomRegions.size(), mutatedRuntimeState);

	return true;
}

void Script::resetRuntimeState() {
	_runtimeFlags = _flags;
	_runtimeObjects = _objects;
	_runtimeAnimations = _animations;
	_runtimeRegions = _regions;
	_runtimeNpcs = _npcs;
	_runtimeMonsters = _monsters;
	_playerCurrentHitPoints = kDefaultPlayerHitPoints;

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

	for (StartupNpcRecord &npc : _runtimeNpcs) {
		npc.runtimeSpawned = false;
		npc.savedVisible = npc.visible;
	}
}

void Script::syncRuntimeSaveState(Common::Serializer &s) {
	syncRecordArray(s, _runtimeFlags, syncStartupFlagRecord);
	syncRecordArray(s, _runtimeObjects, syncStartupObjectRecord);
	syncRecordArray(s, _runtimeAnimations, syncStartupAnimRecord);
	syncRecordArray(s, _runtimeRegions, syncStartupRegionRecord);
	syncRecordArray(s, _runtimeNpcs, syncStartupNpcRecord);
	syncRecordArray(s, _runtimeMonsters, syncStartupMonsterRecord);
	s.syncAsSint32LE(_playerCurrentHitPoints);
	if (s.isLoading())
		_playerCurrentHitPoints = clampPlayerHitPoints(_playerCurrentHitPoints);
}

bool Script::materializeRoomState(const Common::String &entranceName, const Common::String &roomName,
		StartupRoomSetupState &state, ResourceManager &resources) const {
	state = StartupRoomSetupState();

	const StartupEntranceRecord *entrance = findEntranceRecord(entranceName);
	const Common::String resolvedRoomName = !roomName.empty()
		? roomName
		: (entrance ? entrance->roomName : Common::String());
	if (resolvedRoomName.empty())
		return false;

	const StartupRoomRecord *room = findRoomRecord(resolvedRoomName);
	if (!room) {
		warning("Harvester: unresolved materialized room target entrance='%s' room='%s'",
			entranceName.c_str(), resolvedRoomName.c_str());
		return false;
	}

	return buildRuntimeRoomState(*room, entrance, resources, state);
}

bool Script::executeRoomExitCommands(const Common::String &roomName,
		Common::Array<StartupAudioCommand> &audioCommands) {
	audioCommands.clear();

	const StartupRoomRecord *room = findRoomRecord(roomName);
	if (!room)
		return false;

	bool mutatedRuntimeState = false;
	executeCommandChain(room->onExitCommand, "room exit command", room->roomName, false,
		nullptr, &audioCommands, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		&mutatedRuntimeState);
	return true;
}

bool Script::resolveObjectInteraction(const StartupObjectRecord &object, StartupInteractionResult &result) {
	result = StartupInteractionResult();

	if (isPickupObject(object)) {
		if (StartupObjectRecord *runtimeObject = findRuntimeObject(object.currentOwnerOrRoom, object.objectName)) {
			if (!runtimeObject->currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName)) {
				runtimeObject->currentOwnerOrRoom = kInventoryOwnerName;
				runtimeObject->identShown = true;
				result.mutatedRuntimeState = true;
			}
		}
	}
	if (object.actionTag.empty())
		return result.mutatedRuntimeState;

	executeCommandChain(object.actionTag, "interaction command", object.objectName, true,
		&result.musicPath, &result.audioCommands, &result.nextRoomName, &result.roomTransition,
		&result.deathFlicPath, &result.requestMainMenu,
		&result.dialogueNpcName, &result.dialogueContinuationTag, &result.mutatedRuntimeState);

	return !result.nextRoomName.empty() || !result.deathFlicPath.empty() || result.requestMainMenu ||
		!result.dialogueNpcName.empty() || !result.musicPath.empty() || !result.audioCommands.empty() ||
		result.mutatedRuntimeState || hasActionableCommandChain(object.actionTag);
}

bool Script::resolveRegionInteraction(const StartupRegionRecord &region, StartupInteractionResult &result) {
	result = StartupInteractionResult();
	if (region.actionTag.empty())
		return false;

	executeCommandChain(region.actionTag, "region command", region.regionName, true,
		&result.musicPath, &result.audioCommands, &result.nextRoomName, &result.roomTransition,
		&result.deathFlicPath, &result.requestMainMenu,
		&result.dialogueNpcName, &result.dialogueContinuationTag, &result.mutatedRuntimeState);
	return !result.nextRoomName.empty() || !result.deathFlicPath.empty() || result.requestMainMenu ||
		!result.dialogueNpcName.empty() || !result.musicPath.empty() || !result.audioCommands.empty() ||
		result.mutatedRuntimeState || hasActionableCommandChain(region.actionTag);
}

bool Script::resolveUseItemInteraction(const Common::String &itemName, const StartupObjectRecord &target,
		StartupInteractionResult &result) {
	result = StartupInteractionResult();

	const StartupUseItemRecord *useItem = findUseItemRecord(itemName, target);
	if (!useItem)
		return false;

	executeCommandChain(useItem->actionTag, "useitem command",
		Common::String::format("%s -> %s", itemName.c_str(), target.objectName.c_str()), true,
		&result.musicPath, &result.audioCommands, &result.nextRoomName, &result.roomTransition,
		&result.deathFlicPath, &result.requestMainMenu,
		&result.dialogueNpcName, &result.dialogueContinuationTag, &result.mutatedRuntimeState);
	return true;
}

bool Script::executeActionTag(const Common::String &tag, StartupInteractionResult &result,
		bool allowTransitions) {
	result = StartupInteractionResult();
	if (tag.empty())
		return false;

	executeCommandChain(tag, "action tag", tag, allowTransitions,
		&result.musicPath, &result.audioCommands, &result.nextRoomName, &result.roomTransition,
		&result.deathFlicPath, &result.requestMainMenu,
		&result.dialogueNpcName, &result.dialogueContinuationTag, &result.mutatedRuntimeState);

	return !result.nextRoomName.empty() || !result.deathFlicPath.empty() || result.requestMainMenu ||
		!result.dialogueNpcName.empty() || !result.musicPath.empty() || !result.audioCommands.empty() ||
		result.mutatedRuntimeState || hasActionableCommandChain(tag);
}

bool Script::executeNestedActionTag(const Common::String &tag, StartupInteractionResult &result,
		bool allowTransitions) {
	const bool handled = executeActionTag(tag, result, allowTransitions);
	if (handled)
		result.abortRemainingCommandChain = true;

	return handled;
}

bool Script::isPickupObject(const StartupObjectRecord &object) const {
	return !object.altSpritePath.empty() &&
		!object.currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName);
}

bool Script::hasObjectInteraction(const StartupObjectRecord &object) const {
	if (isPickupObject(object))
		return true;
	if (object.actionTag.empty())
		return false;

	return hasActionableCommandChain(object.actionTag);
}

bool Script::hasUseItemInteraction(const Common::String &itemName, const StartupObjectRecord &target) const {
	return findUseItemRecord(itemName, target) != nullptr;
}

void Script::getVisibleInventoryObjects(Common::Array<StartupObjectRecord> &objects) const {
	objects.clear();
	const char *const statusObjectName = resolveInventoryStatusObjectName(_playerCurrentHitPoints);
	const StartupObjectRecord *statusObject = nullptr;

	for (const StartupObjectRecord &object : _runtimeObjects) {
		if (object.objectName.hasPrefixIgnoreCase("INV_STAT")) {
			if (object.objectName.equalsIgnoreCase(statusObjectName))
				statusObject = &object;
			continue;
		}

		if (object.currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName) && object.visible)
			objects.push_back(object);
	}

	if (!statusObject)
		return;

	StartupObjectRecord activeStatusObject = *statusObject;
	activeStatusObject.currentOwnerOrRoom = kInventoryOwnerName;
	activeStatusObject.visible = true;
	activeStatusObject.runtimeVisible = true;
	activeStatusObject.identShown = true;
	objects.push_back(activeStatusObject);
}

void Script::markObjectIdentShown(const StartupObjectRecord &object) {
	if (StartupObjectRecord *runtimeObject = findRuntimeObject(object.currentOwnerOrRoom, object.objectName))
		runtimeObject->identShown = true;
}

const StartupEntranceRecord *Script::findEntranceRecord(const Common::String &entranceName) const {
	if (entranceName.empty())
		return nullptr;

	for (const StartupEntranceRecord &entrance : _entrances) {
		if (entrance.entranceName.equalsIgnoreCase(entranceName))
			return &entrance;
	}

	return nullptr;
}

const StartupMapEntranceRecord *Script::findMapEntranceRecord(const Common::String &entryName) const {
	if (entryName.empty())
		return nullptr;

	for (const StartupMapEntranceRecord &mapEntrance : _mapEntrances) {
		if (mapEntrance.entryName.equalsIgnoreCase(entryName))
			return &mapEntrance;
	}

	return nullptr;
}

const StartupUseItemRecord *Script::findUseItemRecord(const Common::String &itemName,
		const StartupObjectRecord &target) const {
	if (itemName.empty() || target.objectName.empty())
		return nullptr;

	for (const StartupUseItemRecord &useItem : _useItems) {
		if (!useItem.itemName.equalsIgnoreCase(itemName) ||
			!useItem.targetName.equalsIgnoreCase(target.objectName)) {
			continue;
		}

		if (!useItem.ownerOrRoom.empty() &&
			!useItem.ownerOrRoom.equalsIgnoreCase(target.currentOwnerOrRoom)) {
			continue;
		}

		return &useItem;
	}

	return nullptr;
}

const StartupRoomRecord *Script::findRoomRecord(const Common::String &roomName) const {
	if (roomName.empty())
		return nullptr;

	for (const StartupRoomRecord &room : _rooms) {
		if (room.roomName.equalsIgnoreCase(roomName))
			return &room;
	}

	return nullptr;
}

const StartupCommandRecord *Script::findCommandRecord(const Common::String &tag) const {
	if (tag.empty())
		return nullptr;

	for (const StartupCommandRecord &command : _commands) {
		if (command.triggerTag.equalsIgnoreCase(tag))
			return &command;
	}

	return nullptr;
}

const StartupFlagRecord *Script::findRuntimeFlag(const Common::String &flagName) const {
	if (flagName.empty())
		return nullptr;

	for (const StartupFlagRecord &flag : _runtimeFlags) {
		if (flag.name.equalsIgnoreCase(flagName))
			return &flag;
	}

	return nullptr;
}

StartupFlagRecord *Script::findRuntimeFlag(const Common::String &flagName) {
	if (flagName.empty())
		return nullptr;

	for (StartupFlagRecord &flag : _runtimeFlags) {
		if (flag.name.equalsIgnoreCase(flagName))
			return &flag;
	}

	return nullptr;
}

StartupObjectRecord *Script::findRuntimeObject(const Common::String &ownerOrRoom,
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

StartupAnimRecord *Script::findRuntimeAnim(const Common::String &animName) {
	if (animName.empty())
		return nullptr;

	for (StartupAnimRecord &anim : _runtimeAnimations) {
		if (anim.animName.equalsIgnoreCase(animName))
			return &anim;
	}

	return nullptr;
}

StartupRegionRecord *Script::findRuntimeRegion(const Common::String &regionName) {
	if (regionName.empty())
		return nullptr;

	for (StartupRegionRecord &region : _runtimeRegions) {
		if (region.regionName.equalsIgnoreCase(regionName))
			return &region;
	}

	return nullptr;
}

StartupNpcRecord *Script::findRuntimeNpc(const Common::String &npcName) {
	if (npcName.empty())
		return nullptr;

	for (StartupNpcRecord &npc : _runtimeNpcs) {
		if (npc.npcName.equalsIgnoreCase(npcName))
			return &npc;
	}

	return nullptr;
}

StartupMonsterRecord *Script::findRuntimeMonster(const Common::String &monsterName) {
	if (monsterName.empty())
		return nullptr;

	for (StartupMonsterRecord &monster : _runtimeMonsters) {
		if (monster.monsterName.equalsIgnoreCase(monsterName))
			return &monster;
	}

	return nullptr;
}

const StartupNpcRecord *Script::findRuntimeNpc(const Common::String &npcName) const {
	if (npcName.empty())
		return nullptr;

	for (const StartupNpcRecord &npc : _runtimeNpcs) {
		if (npc.npcName.equalsIgnoreCase(npcName))
			return &npc;
	}

	return nullptr;
}

const StartupMonsterRecord *Script::findRuntimeMonster(const Common::String &monsterName) const {
	if (monsterName.empty())
		return nullptr;

	for (const StartupMonsterRecord &monster : _runtimeMonsters) {
		if (monster.monsterName.equalsIgnoreCase(monsterName))
			return &monster;
	}

	return nullptr;
}

const StartupNpcRecord *Script::findRuntimeNpcRecord(const Common::String &npcName) const {
	return findRuntimeNpc(npcName);
}

bool Script::addRuntimeObjectToInventory(const Common::String &objectName) {
	StartupObjectRecord *runtimeObject = findRuntimeObject(Common::String(), objectName);
	if (!runtimeObject)
		return false;

	const bool changed = !runtimeObject->currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName) ||
		!runtimeObject->visible || !runtimeObject->runtimeVisible || !runtimeObject->identShown;
	runtimeObject->currentOwnerOrRoom = kInventoryOwnerName;
	runtimeObject->visible = true;
	runtimeObject->runtimeVisible = true;
	runtimeObject->identShown = true;
	return changed;
}

bool Script::setRuntimeObjectVisible(const Common::String &ownerOrRoom,
		const Common::String &objectName, bool visible) {
	if (objectName.empty())
		return false;

	StartupObjectRecord *runtimeObject = findRuntimeObject(ownerOrRoom, objectName);
	if (!runtimeObject)
		return false;

	const bool changed = runtimeObject->visible != visible ||
		runtimeObject->runtimeVisible != visible;
	runtimeObject->visible = visible;
	runtimeObject->runtimeVisible = visible;
	if (visible && runtimeObject->currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName))
		runtimeObject->identShown = true;

	return changed;
}

bool Script::setRuntimeNpcState(const Common::String &npcName, bool active, bool visible) {
	if (npcName.empty())
		return false;

	StartupNpcRecord *runtimeNpc = findRuntimeNpc(npcName);
	if (!runtimeNpc)
		return false;

	const bool changed = runtimeNpc->active != active || runtimeNpc->visible != visible;
	runtimeNpc->active = active;
	runtimeNpc->visible = visible;
	return changed;
}

bool Script::setPlayerCurrentHitPoints(int hitPoints) {
	const int clampedHitPoints = clampPlayerHitPoints(hitPoints);
	const bool changed = _playerCurrentHitPoints != clampedHitPoints;
	_playerCurrentHitPoints = clampedHitPoints;
	return changed;
}

bool Script::triggerRuntimeNpcDeathOrMonsterfy(const Common::String &npcName) {
	if (npcName.empty())
		return false;

	StartupNpcRecord *runtimeNpc = findRuntimeNpc(npcName);
	if (!runtimeNpc)
		return false;

	bool monsterChanged = false;
	if (!runtimeNpc->monsterfyTargetName.empty()) {
		StartupMonsterRecord *runtimeMonster = findRuntimeMonster(runtimeNpc->monsterfyTargetName);
		if (runtimeMonster) {
			monsterChanged = !runtimeMonster->active || !runtimeMonster->visible;
			runtimeMonster->active = true;
			runtimeMonster->visible = true;
		} else {
			debug(1, "Harvester: unresolved monsterfy target for dialogue npc='%s' target='%s'",
				runtimeNpc->npcName.c_str(), runtimeNpc->monsterfyTargetName.c_str());
		}
	}

	const bool changed = !runtimeNpc->deathOrMonsterfyFlag;
	runtimeNpc->deathOrMonsterfyFlag = true;
	return changed || monsterChanged;
}

bool Script::buildRuntimeRoomState(const StartupRoomRecord &room, const StartupEntranceRecord *entrance,
		ResourceManager &resources, StartupRoomSetupState &state) const {
	state = StartupRoomSetupState();

	const StartupObjectRecord *background = nullptr;
	for (const StartupObjectRecord &candidate : _runtimeObjects) {
		if (isRoomBackgroundObject(candidate, room, resources)) {
			background = &candidate;
			break;
		}
	}
	if (!background) {
		warning("Harvester: no fullscreen background object found for room '%s'", room.roomName.c_str());
		return false;
	}

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
	if (!state.hasEntrance) {
		static const char *const kCloseupExitObjects[] = { "EXIT_BM", "EXIT_HS" };
		for (const char *name : kCloseupExitObjects) {
			for (const StartupObjectRecord &object : _runtimeObjects) {
				if (object.objectName.equalsIgnoreCase(name)) {
					state.roomObjects.push_back(object);
					break;
				}
			}
		}
	}
	for (const StartupAnimRecord &anim : _runtimeAnimations) {
		if (anim.roomName.equalsIgnoreCase(room.roomName))
			state.roomAnimations.push_back(anim);
	}
	for (const StartupNpcRecord &npc : _runtimeNpcs) {
		if (!npc.roomName.equalsIgnoreCase(room.roomName) || !npc.visible)
			continue;
		if (npc.deathOrMonsterfyFlag) {
			debugC(1, kDebugScene,
				"Harvester: suppressed room npc room='%s' npc='%s' death_or_monsterfy=%d damage_type=%d model='%s' monsterfy_target='%s'",
				room.roomName.c_str(), npc.npcName.c_str(), npc.deathOrMonsterfyFlag, npc.deathDamageType,
				npc.modelPath.c_str(), npc.monsterfyTargetName.c_str());
			continue;
		}

		state.roomNpcs.push_back(npc);
	}
	for (const StartupMonsterRecord &monster : _runtimeMonsters) {
		if (!monster.roomName.equalsIgnoreCase(room.roomName))
			continue;
		if (!monster.active && !monster.visible)
			continue;

		state.roomMonsters.push_back(monster);
	}
	for (const StartupRegionRecord &region : _runtimeRegions) {
		if (region.roomName.equalsIgnoreCase(room.roomName) && region.startEnabled)
			state.roomRegions.push_back(region);
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
	for (const StartupNpcRecord &npc : state.roomNpcs) {
		debugC(1, kDebugScene,
			"Harvester: materialized room npc room='%s' npc='%s' visible=%d active=%d pos=(%d,%d,%d) frame_delay=%d model='%s' on_death='%s' audio='%s'",
			state.roomName.c_str(), npc.npcName.c_str(), npc.visible, npc.active,
			npc.posX, npc.posY, npc.posZ, npc.frameDelay,
			npc.modelPath.c_str(), npc.onDeathActionTag.c_str(), npc.audioPath.c_str());
	}
	for (const StartupMonsterRecord &monster : state.roomMonsters) {
		debugC(1, kDebugScene,
			"Harvester: materialized room monster room='%s' monster='%s' visible=%d active=%d pos=(%d,%d,%d) facing=%d model='%s' on_death='%s'",
			state.roomName.c_str(), monster.monsterName.c_str(), monster.visible, monster.active,
			monster.posX, monster.posY, monster.posZ, monster.initialFacing,
			monster.modelPath.c_str(), monster.onDeathActionTag.c_str());
	}

	debugC(1, kDebugGeneral,
		"Harvester: materializeRoomState room='%s' entrance='%s' spawn=(%d,%d,%d) facing=%d palette='%s' background='%s' music='%s' brightness=%.2f roomObjects=%u roomAnims=%u roomNpcs=%u roomMonsters=%u roomRegions=%u",
		state.roomName.c_str(), state.entranceName.c_str(), state.playerSpawnX, state.playerSpawnY,
		state.playerSpawnZ, state.playerFacing, state.palettePath.c_str(), state.backgroundPath.c_str(),
		state.musicPath.c_str(), (double)state.paletteBrightness,
		(uint)state.roomObjects.size(), (uint)state.roomAnimations.size(),
		(uint)state.roomNpcs.size(), (uint)state.roomMonsters.size(), (uint)state.roomRegions.size());

	return true;
}

void Script::executeCommandChain(const Common::String &initialTag, const char *contextLabel,
		const Common::String &contextName, bool allowTransitions, Common::String *musicPath,
		Common::Array<StartupAudioCommand> *audioCommands, Common::String *nextRoomName,
		StartupRoomTransitionKind *roomTransition,
		Common::String *deathFlicPath, bool *requestMainMenu, Common::String *dialogueNpcName,
		Common::String *dialogueContinuationTag,
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

		if (command->opcodeName.equalsIgnoreCase("ADD2INV")) {
			noteMutation(addRuntimeObjectToInventory(command->arg1));
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

		if (command->opcodeName.equalsIgnoreCase("SET_REGION")) {
			StartupRegionRecord *runtimeRegion = findRuntimeRegion(command->arg1);
			if (!runtimeRegion) {
				debug(1, "Harvester: unresolved region for %s '%s' region='%s'",
					contextLabel, contextName.c_str(), command->arg1.c_str());
				currentTag = command->arg4;
				continue;
			}

			const bool enabled = !command->arg2.equalsIgnoreCase("F");
			const bool changed = runtimeRegion->startEnabled != enabled;
			runtimeRegion->startEnabled = enabled;
			noteMutation(changed);
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("SET_NPC")) {
			StartupNpcRecord *runtimeNpc = findRuntimeNpc(command->arg1);
			if (!runtimeNpc) {
				debug(1, "Harvester: unresolved npc for %s '%s' npc='%s'",
					contextLabel, contextName.c_str(), command->arg1.c_str());
				currentTag = command->arg4;
				continue;
			}

			const bool active = isTruthy(command->arg2);
			const bool visible = isTruthy(command->arg3);
			const bool changed = runtimeNpc->active != active || runtimeNpc->visible != visible;
			runtimeNpc->active = active;
			runtimeNpc->visible = visible;
			noteMutation(changed);
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("SET_MONSTER")) {
			StartupMonsterRecord *runtimeMonster = findRuntimeMonster(command->arg1);
			if (!runtimeMonster) {
				debug(1, "Harvester: unresolved monster for %s '%s' monster='%s'",
					contextLabel, contextName.c_str(), command->arg1.c_str());
				currentTag = command->arg4;
				continue;
			}

			const bool active = isTruthy(command->arg2);
			const bool visible = isTruthy(command->arg3);
			const bool changed = runtimeMonster->active != active || runtimeMonster->visible != visible;
			runtimeMonster->active = active;
			runtimeMonster->visible = visible;
			noteMutation(changed);
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("START_DIALOG")) {
			if (!dialogueNpcName || !dialogueContinuationTag) {
				debug(1, "Harvester: unsupported startup command '%s' for %s '%s' without dialogue context",
					command->opcodeName.c_str(), contextLabel, contextName.c_str());
				return;
			}
			if (command->arg1.empty()) {
				currentTag = command->arg4;
				continue;
			}

			*dialogueNpcName = command->arg1;
			*dialogueContinuationTag = command->arg4;
			return;
		}

		if (command->opcodeName.equalsIgnoreCase("GODEATHFLIC")) {
			if (!deathFlicPath || !requestMainMenu) {
				debug(1, "Harvester: unsupported startup command '%s' for %s '%s' without menu-exit context",
					command->opcodeName.c_str(), contextLabel, contextName.c_str());
				return;
			}

			if (allowTransitions) {
				*deathFlicPath = command->arg1;
				*requestMainMenu = true;
			} else {
				debugC(1, kDebugScene, "Harvester: skipped transition opcode '%s' while processing %s '%s'",
					command->opcodeName.c_str(), contextLabel, contextName.c_str());
			}
			return;
		}

		if (command->opcodeName.equalsIgnoreCase("KILL_NPC") ||
				command->opcodeName.equalsIgnoreCase("MONSTERFY")) {
			StartupNpcRecord *runtimeNpc = findRuntimeNpc(command->arg1);
			if (!runtimeNpc) {
				debug(1, "Harvester: unresolved npc for %s '%s' npc='%s'",
					contextLabel, contextName.c_str(), command->arg1.c_str());
				currentTag = command->arg4;
				continue;
			}

			const int deathDamageType = parseNpcDeathDamageType(command->arg2);
			bool monsterChanged = false;
			if (command->opcodeName.equalsIgnoreCase("MONSTERFY") && !runtimeNpc->monsterfyTargetName.empty()) {
				StartupMonsterRecord *runtimeMonster = findRuntimeMonster(runtimeNpc->monsterfyTargetName);
				if (runtimeMonster) {
					monsterChanged = !runtimeMonster->active || !runtimeMonster->visible;
					runtimeMonster->active = true;
					runtimeMonster->visible = true;
				} else {
					debug(1, "Harvester: unresolved monsterfy target for %s '%s' npc='%s' target='%s'",
						contextLabel, contextName.c_str(), command->arg1.c_str(),
						runtimeNpc->monsterfyTargetName.c_str());
				}
			}

			const bool changed = !runtimeNpc->deathOrMonsterfyFlag ||
				(deathDamageType != 0 && runtimeNpc->deathDamageType != deathDamageType);
			runtimeNpc->deathOrMonsterfyFlag = true;
			if (deathDamageType != 0)
				runtimeNpc->deathDamageType = deathDamageType;
			noteMutation(changed || monsterChanged);
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("HEAL_PC") ||
			command->opcodeName.equalsIgnoreCase("ADJ_HP")) {
			noteMutation(setPlayerCurrentHitPoints(
				_playerCurrentHitPoints + atoi(command->arg1.c_str())));
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("KILL_PC")) {
			noteMutation(setPlayerCurrentHitPoints(0));
			currentTag = command->arg4;
			continue;
		}

		if (command->opcodeName.equalsIgnoreCase("CLOSEUP") ||
			command->opcodeName.equalsIgnoreCase("CHANGE_ROOM")) {
			if (allowTransitions && nextRoomName) {
				*nextRoomName = command->arg1;
				if (roomTransition) {
					*roomTransition = command->opcodeName.equalsIgnoreCase("CHANGE_ROOM")
						? kStartupRoomTransitionChangeRoom
						: kStartupRoomTransitionCloseup;
				}
			} else {
				debugC(1, kDebugScene, "Harvester: skipped transition opcode '%s' while processing %s '%s'",
					command->opcodeName.c_str(), contextLabel, contextName.c_str());
			}
			return;
		}

		debug(1, "Harvester: unsupported startup command '%s' for %s '%s', continuing",
			command->opcodeName.c_str(), contextLabel, contextName.c_str());
		currentTag = command->arg4;
	}
}

bool Script::hasActionableCommandChain(const Common::String &initialTag) const {
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
			command->opcodeName.equalsIgnoreCase("ADD2INV") ||
			command->opcodeName.equalsIgnoreCase("SET_ANIM") ||
			command->opcodeName.equalsIgnoreCase("SET_REGION") ||
			command->opcodeName.equalsIgnoreCase("SET_NPC") ||
			command->opcodeName.equalsIgnoreCase("SET_MONSTER") ||
			command->opcodeName.equalsIgnoreCase("START_DIALOG") ||
			command->opcodeName.equalsIgnoreCase("GODEATHFLIC") ||
			command->opcodeName.equalsIgnoreCase("KILL_NPC") ||
			command->opcodeName.equalsIgnoreCase("MONSTERFY") ||
			command->opcodeName.equalsIgnoreCase("HEAL_PC") ||
			command->opcodeName.equalsIgnoreCase("ADJ_HP") ||
			command->opcodeName.equalsIgnoreCase("KILL_PC") ||
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

const StartupTextRecord *Script::findTextRecord(const Common::String &key) const {
	if (key.empty())
		return nullptr;

	for (const StartupTextRecord &textRecord : _texts) {
		if (textRecord.key.equalsIgnoreCase(key))
			return &textRecord;
	}

	return nullptr;
}

bool Script::resolveObjectInspectText(const StartupObjectRecord &object, StartupResolvedText &text) const {
	text = StartupResolvedText();

	const StartupTextRecord *textRecord = findTextRecord(object.identTextKey);
	if (!textRecord)
		return false;

	text.boxName = textRecord->boxName;
	text.value = textRecord->value;
	return !text.value.empty();
}

Common::String Script::resolveObjectLabel(const StartupObjectRecord &object) const {
	if (object.currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName)) {
		const StartupTextRecord *inventoryText = findTextRecord(object.inventoryTextKey);
		if (inventoryText && !inventoryText->value.empty())
			return inventoryText->value;
	}

	if (!object.interactionLabel.empty() && !object.interactionLabel.equalsIgnoreCase("NULL_ID"))
		return normalizeInteractionLabel(object.interactionLabel);
	if (object.interactionLabel.equalsIgnoreCase("NULL_ID"))
		return Common::String();

	const StartupTextRecord *textRecord = findTextRecord(object.identTextKey);
	if (textRecord && !textRecord->value.empty())
		return textRecord->value;

	Common::String label = normalizeInteractionLabel(object.objectName);
	if (!label.empty() && !label.equalsIgnoreCase("NULL ID"))
		return label;

	return Common::String();
}

Common::String Script::resolveTextValue(const Common::String &key) const {
	const StartupTextRecord *textRecord = findTextRecord(key);
	if (textRecord && !textRecord->value.empty())
		return textRecord->value;

	return normalizeInteractionLabel(key);
}

const StartupHeadRecord *Script::findHeadRecord(const Common::String &headId) const {
	if (headId.empty())
		return nullptr;

	for (const StartupHeadRecord &head : _heads) {
		if (head.headId.equalsIgnoreCase(headId))
			return &head;
	}

	return nullptr;
}

bool Script::getFlagValue(const Common::String &flagName) const {
	const StartupFlagRecord *flag = findRuntimeFlag(flagName);
	return flag && flag->value;
}

bool Script::setRuntimeFlagValue(const Common::String &flagName, bool value) {
	if (flagName.empty())
		return false;

	StartupFlagRecord *flag = findRuntimeFlag(flagName);
	if (flag) {
		const bool changed = flag->value != value;
		flag->value = value;
		return changed;
	}

	StartupFlagRecord newFlag;
	newFlag.name = flagName;
	newFlag.value = value;
	_runtimeFlags.push_back(newFlag);
	return true;
}

bool Script::resetRuntimeObjectToInitialState(const Common::String &objectName) {
	if (objectName.empty())
		return false;

	StartupObjectRecord *runtimeObject = findRuntimeObject(Common::String(), objectName);
	if (!runtimeObject)
		return false;

	const StartupObjectRecord *baseObject = nullptr;
	for (const StartupObjectRecord &object : _objects) {
		if (object.objectName.equalsIgnoreCase(objectName)) {
			baseObject = &object;
			break;
		}
	}
	if (!baseObject)
		return false;

	const bool changed = runtimeObject->currentX != runtimeObject->initialX ||
		runtimeObject->currentY != runtimeObject->initialY ||
		runtimeObject->currentZ != runtimeObject->initialZ ||
		!runtimeObject->currentOwnerOrRoom.equalsIgnoreCase(runtimeObject->initialOwnerOrRoom) ||
		runtimeObject->visible != baseObject->visible ||
		runtimeObject->runtimeVisible != baseObject->visible ||
		runtimeObject->identShown != baseObject->identShown;

	runtimeObject->currentX = runtimeObject->initialX;
	runtimeObject->currentY = runtimeObject->initialY;
	runtimeObject->currentZ = runtimeObject->initialZ;
	runtimeObject->currentOwnerOrRoom = runtimeObject->initialOwnerOrRoom;
	runtimeObject->visible = baseObject->visible;
	runtimeObject->runtimeVisible = baseObject->visible;
	runtimeObject->identShown = baseObject->identShown;
	return changed;
}

bool Script::isNamedNpcDeathTypeClear(const Common::String &npcName) const {
	const StartupNpcRecord *npc = findRuntimeNpc(npcName);
	return npc && npc->deathDamageType == 0;
}

int Script::getCurrentStoryDayIndex() const {
	if (getFlagValue("DAY_1"))
		return 1;
	if (getFlagValue("DAY_2") || getFlagValue("NIGHT_2"))
		return 2;
	if (getFlagValue("DAY_3") || getFlagValue("NIGHT_3"))
		return 3;
	if (getFlagValue("DAY_4") || getFlagValue("NIGHT_4"))
		return 4;
	if (getFlagValue("DAY_5") || getFlagValue("NIGHT_5"))
		return 5;
	if (getFlagValue("DAY_6"))
		return 6;

	return 0;
}

} // End of namespace Harvester
