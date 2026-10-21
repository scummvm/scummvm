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
#include "harvester/parse_utils.h"
#include "harvester/patches.h"
#include "harvester/resources.h"

namespace Harvester {

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

int Script::parseDeathDamageType(const Common::String &value) {
	if (value.equalsIgnoreCase("BLUDGE"))
		return 1;
	if (value.equalsIgnoreCase("SLASH"))
		return 2;
	if (value.equalsIgnoreCase("PROJ"))
		return 4;

	return 0;
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
			bool hasEmbeddedQuote = false;
			while (i < line.size()) {
				const bool isClosingQuote = line[i] == '"' &&
					(i + 1 == line.size() || line[i + 1] == ' ' || line[i + 1] == '\t');
				if (isClosingQuote)
					break;

				// French TEXT records contain quotes inside underscore-separated values.
				// The native word reader keeps them and strips only the outer quote pair.
				if (line[i] == '"')
					hasEmbeddedQuote = true;
				token += line[i++];
			}
			if (i < line.size() && line[i] == '"')
				++i;
			if (hasEmbeddedQuote)
				debugC(3, kDebugGeneral, "Harvester: TownScript token contains embedded quote value='%s'",
					token.c_str());
			tokens.push_back(Common::move(token));
			continue;
		}

		Common::String token;
		while (i < line.size() && line[i] != ' ' && line[i] != '\t')
			token += line[i++];
		tokens.push_back(Common::move(token));
	}
}

void Script::decode() {
	for (byte &value : _data) {
		if (value == '\r' || value == '\n')
			continue;

		value ^= kTownScriptXorKey;
	}
}

void Script::parseFlagRecord(const Common::Array<Common::String> &tokens, uint tagIndex) {
	if (tokens.size() < tagIndex + 3)
		return;

	FlagRecord flag;
	flag.name = tokens[tagIndex + 1];
	flag.value = tokens[tagIndex + 2].equalsIgnoreCase("T");
	if (!flag.name.empty())
		_flags.push_back(Common::move(flag));
}

void Script::parseCommandRecord(ResourceManager &resources,
		const Common::Array<Common::String> &tokens, uint tagIndex) {
	if (tokens.size() < tagIndex + 6)
		return;

	CommandRecord command;
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
		_commands.push_back(Common::move(command));
}

void Script::parseExecListRecord(const Common::Array<Common::String> &tokens, uint tagIndex) {
	if (tokens.size() < tagIndex + 2)
		return;

	ExecListRecord execList;
	execList.listName = tokens[tagIndex + 1];
	for (uint i = tagIndex + 2; i < tokens.size(); ++i) {
		if (tokens[i].empty())
			break;
		execList.entries.push_back(tokens[i]);
	}
	if (!execList.listName.empty())
		_execLists.push_back(Common::move(execList));
}

void Script::parseTextRecord(const Common::Array<Common::String> &tokens, uint tagIndex) {
	if (tokens.size() < tagIndex + 4)
		return;

	TextRecord textRecord;
	textRecord.key = tokens[tagIndex + 1];
	textRecord.boxName = tokens[tagIndex + 2];
	textRecord.value = tokens[tagIndex + 3];
	for (uint i = 0; i < textRecord.value.size(); ++i) {
		if (textRecord.value[i] == '_')
			textRecord.value.setChar(' ', i);
	}
	if (!textRecord.key.empty())
		_texts.push_back(Common::move(textRecord));
}

void Script::parseHeadRecord(ResourceManager &resources,
		const Common::Array<Common::String> &tokens, uint tagIndex) {
	if (tokens.size() < tagIndex + 3)
		return;

	HeadRecord head;
	head.headId = tokens[tagIndex + 1];
	head.portraitPath = resources.normalizeResourcePath(tokens[tagIndex + 2]);
	if (!head.headId.empty() && !head.portraitPath.empty())
		_heads.push_back(Common::move(head));
}

void Script::parseUseItemRecord(const Common::Array<Common::String> &tokens, uint tagIndex) {
	if (tokens.size() < tagIndex + 5)
		return;

	UseItemRecord useItem;
	useItem.itemName = tokens[tagIndex + 1];
	useItem.ownerOrRoom = tokens[tagIndex + 2];
	useItem.targetName = tokens[tagIndex + 3];
	useItem.actionTag = tokens[tagIndex + 4];
	if (!useItem.itemName.empty() && !useItem.targetName.empty())
		_useItems.push_back(Common::move(useItem));
}

void Script::parseEntranceRecord(const Common::Array<Common::String> &tokens, uint tagIndex) {
	if (tokens.size() < tagIndex + 4)
		return;

	EntranceRecord entrance;
	if (tagIndex >= 3) {
		entrance.posX = parseAsciiIntOrZero(tokens[0]);
		entrance.posY = parseAsciiIntOrZero(tokens[1]);
		entrance.posZ = parseAsciiIntOrZero(tokens[2]);
	}
	entrance.direction = tokens[tagIndex + 1];
	entrance.facing = parseEntranceFacing(entrance.direction);
	entrance.roomName = tokens[tagIndex + 2];
	entrance.entranceName = tokens[tagIndex + 3];
	if (!entrance.roomName.empty() && !entrance.entranceName.empty())
		_entrances.push_back(Common::move(entrance));
}

void Script::parseMapEntranceRecord(const Common::Array<Common::String> &tokens, uint tagIndex) {
	if (tokens.size() < tagIndex + 2)
		return;

	MapEntranceRecord mapEntrance;
	if (tagIndex >= 3) {
		mapEntrance.mapX = parseAsciiIntOrZero(tokens[0]);
		mapEntrance.mapY = parseAsciiIntOrZero(tokens[1]);
		mapEntrance.initialPanelIndex = parseAsciiIntOrZero(tokens[2]);
	}
	mapEntrance.entryName = tokens[tagIndex + 1];
	if (!mapEntrance.entryName.empty())
		_mapEntrances.push_back(Common::move(mapEntrance));
}

void Script::parseMapLocationRecord(const Common::Array<Common::String> &tokens, uint tagIndex) {
	if (tokens.size() < tagIndex + 3)
		return;

	MapLocationRecord mapLocation;
	if (tagIndex >= 7) {
		mapLocation.minX = parseAsciiIntOrZero(tokens[0]);
		mapLocation.minY = parseAsciiIntOrZero(tokens[1]);
		mapLocation.maxX = parseAsciiIntOrZero(tokens[2]);
		mapLocation.maxY = parseAsciiIntOrZero(tokens[3]);
		mapLocation.panelIndex = parseAsciiIntOrZero(tokens[4]);
		mapLocation.labelX = parseAsciiIntOrZero(tokens[5]);
		mapLocation.labelY = parseAsciiIntOrZero(tokens[6]);
	}
	mapLocation.labelText = tokens[tagIndex + 1];
	for (uint i = 0; i < mapLocation.labelText.size(); ++i) {
		if (mapLocation.labelText[i] == '_')
			mapLocation.labelText.setChar(' ', i);
	}
	mapLocation.destinationEntranceName = tokens[tagIndex + 2];
	if (!mapLocation.destinationEntranceName.empty())
		_mapLocations.push_back(Common::move(mapLocation));
}

void Script::parseRoomRecord(ResourceManager &resources,
		const Common::Array<Common::String> &tokens, uint tagIndex) {
	if (tokens.size() < tagIndex + 10)
		return;

	RoomRecord room;
	if (tagIndex >= 6) {
		room.minZ = parseAsciiIntOrZero(tokens[0]);
		room.maxZ = parseAsciiIntOrZero(tokens[1]);
		room.maxZScreenY = parseAsciiIntOrZero(tokens[2]);
		room.minZScreenY = parseAsciiIntOrZero(tokens[3]);
		room.fullScaleZ = parseAsciiIntOrZero(tokens[4]);
		room.maxZScalePercent = parseAsciiIntOrZero(tokens[5]);
		if (room.maxZ != room.fullScaleZ) {
			room.perspectiveScale = ((100.0f - static_cast<float>(room.maxZScalePercent)) /
				static_cast<float>(room.maxZ - room.fullScaleZ)) * 0.01f;
		}
	}
	room.roomName = tokens[tagIndex + 1];
	room.musicPath = resources.normalizeResourcePath(tokens[tagIndex + 2]);
	room.reservedString38 = tokens[tagIndex + 3];
	room.reservedString3c = tokens[tagIndex + 4];
	room.reservedString40 = tokens[tagIndex + 5];
	room.palettePath = resources.normalizeResourcePath(tokens[tagIndex + 6]);
	room.dimmable = tokens[tagIndex + 7].equalsIgnoreCase("T");
	room.onEnterCommand = tokens[tagIndex + 8];
	room.onExitCommand = tokens[tagIndex + 9];
	if (!room.roomName.empty())
		_rooms.push_back(Common::move(room));
}

void Script::parseNpcRecord(ResourceManager &resources,
		const Common::Array<Common::String> &tokens, uint tagIndex) {
	if (tokens.size() < tagIndex + 10)
		return;

	NpcRecord npc;
	if (tagIndex >= 4) {
		npc.posX = parseAsciiIntOrZero(tokens[0]);
		npc.posY = parseAsciiIntOrZero(tokens[1]);
		npc.posZ = parseAsciiIntOrZero(tokens[2]);
		npc.frameDelay = parseAsciiIntOrZero(tokens[3]);
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
		_npcs.push_back(Common::move(npc));
}

void Script::parseMonsterRecord(ResourceManager &resources,
		const Common::Array<Common::String> &tokens, uint tagIndex) {
	if (tokens.size() < tagIndex + 24)
		return;

	MonsterRecord monster;
	if (tagIndex >= 6) {
		monster.posX = parseAsciiIntOrZero(tokens[0]);
		monster.posY = parseAsciiIntOrZero(tokens[1]);
		monster.posZ = parseAsciiIntOrZero(tokens[2]);
		// Native MonsterRecord numeric order is engage distance, initial HP, then damage.
		monster.engageDistance = parseAsciiIntOrZero(tokens[3]);
		monster.initialHitPoints = parseAsciiIntOrZero(tokens[4]);
		monster.damageAmount = parseAsciiIntOrZero(tokens[5]);
	}
	if (tagIndex >= 10) {
		monster.attackSoundTriggerFrame = parseAsciiIntOrZero(tokens[6]);
		monster.hitSoundTriggerFrame = parseAsciiIntOrZero(tokens[7]);
		monster.footstepSoundTriggerFrame = parseAsciiIntOrZero(tokens[8]);
		monster.deathSoundTriggerFrame = parseAsciiIntOrZero(tokens[9]);
	}
	monster.roomName = tokens[tagIndex + 1];
	monster.monsterName = tokens[tagIndex + 2];
	monster.modelPath = resources.normalizeResourcePath(tokens[tagIndex + 3]);
	monster.reservedString44 = tokens[tagIndex + 4];
	monster.reservedString48 = tokens[tagIndex + 5];
	monster.reservedString38 = tokens[tagIndex + 6];
	monster.reservedString3c = tokens[tagIndex + 7];
	monster.facing = parseEntranceFacing(tokens[tagIndex + 8]);
	monster.damageType = parseDeathDamageType(tokens[tagIndex + 9]);
	monster.attackSound1 = resources.normalizeResourcePath(tokens[tagIndex + 10]);
	monster.attackSound2 = resources.normalizeResourcePath(tokens[tagIndex + 11]);
	monster.attackSound3 = resources.normalizeResourcePath(tokens[tagIndex + 12]);
	monster.hitSound1 = resources.normalizeResourcePath(tokens[tagIndex + 13]);
	monster.hitSound2 = resources.normalizeResourcePath(tokens[tagIndex + 14]);
	monster.hitSound3 = resources.normalizeResourcePath(tokens[tagIndex + 15]);
	monster.footstepSoundLeft = resources.normalizeResourcePath(tokens[tagIndex + 16]);
	monster.footstepSoundRight = resources.normalizeResourcePath(tokens[tagIndex + 17]);
	monster.deathSound = resources.normalizeResourcePath(tokens[tagIndex + 18]);
	monster.active = tokens[tagIndex + 19].equalsIgnoreCase("T");
	monster.visible = tokens[tagIndex + 20].equalsIgnoreCase("T");
	monster.onDeathActionTag = tokens[tagIndex + 21];
	if (tokens.size() > tagIndex + 22 && !tokens[tagIndex + 22].empty())
		monster.screenMinXBound = parseAsciiIntOrZero(tokens[tagIndex + 22]);
	if (tokens.size() > tagIndex + 23 && !tokens[tagIndex + 23].empty())
		monster.screenMaxXBound = parseAsciiIntOrZero(tokens[tagIndex + 23]);
	monster.currentHitPoints = monster.initialHitPoints;
	monster.savedVisible = monster.visible;
	if (monster.active)
		monster.visible = true;
	if (!monster.roomName.empty() && !monster.monsterName.empty() && !monster.modelPath.empty()) {
		monster.recordIndex = static_cast<int>(_monsters.size());
		_monsters.push_back(Common::move(monster));
	}
}

void Script::parseTimerRecord(const Common::Array<Common::String> &tokens, uint tagIndex) {
	if (tokens.size() < tagIndex + 7)
		return;

	TimerRecord timer;
	if (tagIndex >= 1) {
		timer.initialValue = parseAsciiIntOrZero(tokens[0]);
		timer.currentValue = timer.initialValue;
	}
	timer.timerName = tokens[tagIndex + 1];
	timer.arg1 = tokens[tagIndex + 2];
	timer.arg2 = tokens[tagIndex + 3];
	timer.enabled = tokens[tagIndex + 4].equalsIgnoreCase("T");
	timer.looping = tokens[tagIndex + 5].equalsIgnoreCase("T");
	timer.global = tokens[tagIndex + 6].equalsIgnoreCase("T");
	if (!timer.timerName.empty())
		_timers.push_back(Common::move(timer));
}

void Script::parseRegionRecord(const Common::Array<Common::String> &tokens, uint tagIndex) {
	if (tokens.size() < tagIndex + 7)
		return;

	RegionRecord region;
	if (tagIndex >= 6) {
		region.left = parseAsciiIntOrZero(tokens[0]);
		region.top = parseAsciiIntOrZero(tokens[1]);
		region.right = parseAsciiIntOrZero(tokens[2]);
		region.bottom = parseAsciiIntOrZero(tokens[3]);
		region.minZ = parseAsciiIntOrZero(tokens[4]);
		region.maxZ = parseAsciiIntOrZero(tokens[5]);
	}
	region.regionName = tokens[tagIndex + 1];
	region.direction = tokens[tagIndex + 2];
	region.desiredFacing = parseEntranceFacing(region.direction);
	region.roomName = tokens[tagIndex + 3];
	region.actionTag = tokens[tagIndex + 4];
	region.startEnabled = tokens[tagIndex + 5].equalsIgnoreCase("T");
	region.cursorEnabled = tokens[tagIndex + 6].equalsIgnoreCase("T");
	if (!region.roomName.empty() && !region.regionName.empty())
		_regions.push_back(Common::move(region));
}

void Script::parseAnimRecord(ResourceManager &resources,
		const Common::Array<Common::String> &tokens, uint tagIndex) {
	if (tokens.size() < tagIndex + 10)
		return;

	AnimRecord anim;
	if (tagIndex >= 4) {
		anim.x = parseAsciiIntOrZero(tokens[0]);
		anim.y = parseAsciiIntOrZero(tokens[1]);
		anim.z = parseAsciiIntOrZero(tokens[2]);
		anim.frameDelay = parseAsciiIntOrZero(tokens[3]);
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
		_animations.push_back(Common::move(anim));
}

void Script::parseObjectRecord(ResourceManager &resources,
		const Common::Array<Common::String> &tokens, uint tagIndex, uint32 &anonymousObjectId) {
	if (tokens.size() < tagIndex + 13)
		return;

	ObjectRecord object;
	if (tagIndex >= 6) {
		object.initialX = parseAsciiIntOrZero(tokens[0]);
		object.initialY = parseAsciiIntOrZero(tokens[1]);
		object.boundsX2 = parseAsciiIntOrZero(tokens[2]);
		object.boundsY2 = parseAsciiIntOrZero(tokens[3]);
		object.initialZ = parseAsciiIntOrZero(tokens[4]);
		object.zExtent = parseAsciiIntOrZero(tokens[5]);
	}
	object.currentX = object.initialX;
	object.currentY = object.initialY;
	object.currentZ = object.initialZ;
	object.initialOwnerOrRoom = tokens[tagIndex + 1];
	object.objectName = tokens[tagIndex + 2];
	object.spritePath = resources.normalizeResourcePath(tokens[tagIndex + 3]);
	object.altSpritePath = resources.normalizeResourcePath(tokens[tagIndex + 4]);
	object.reservedString40 = tokens[tagIndex + 5];
	object.inventoryTextKey = tokens[tagIndex + 6];
	object.reservedXFlag = tokens[tagIndex + 7];
	object.identTextKey = tokens[tagIndex + 8];
	object.operatable = tokens[tagIndex + 9].equalsIgnoreCase("T");
	object.visible = tokens[tagIndex + 10].equalsIgnoreCase("T");
	object.actionTag = tokens[tagIndex + 11];
	object.interactionLabel = tokens[tagIndex + 12];
	object.currentOwnerOrRoom = object.initialOwnerOrRoom;
	object.runtimeVisible = object.visible;
	object.identShown = object.identTextKey.empty();
	applyObjectPatches(object);
	if (object.objectName.empty()) {
		// Room scripts use unnamed OBJECT entries as blocker rectangles.
		object.objectName = Common::String::format("__ANON_OBJECT_%u", anonymousObjectId++);
	}
	if (!object.initialOwnerOrRoom.empty())
		_objects.push_back(Common::move(object));
}

void Script::parseTownRecordLine(ResourceManager &resources,
		const Common::String &rawLine, uint32 &anonymousObjectId) {
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
			tokens[i] == "NPC" || tokens[i] == "MONSTER" || tokens[i] == "REGION" ||
			tokens[i] == "HEAD" || tokens[i] == "FLAG" || tokens[i] == "COMMAND" ||
			tokens[i] == "EXEC_LIST" || tokens[i] == "TEXT" || tokens[i] == "TIMER" ||
			tokens[i] == "USEITEM") {
			tagIndex = i;
			break;
		}
	}
	if (tagIndex == tokens.size())
		return;

	const Common::String &tag = tokens[tagIndex];
	if (tag == "FLAG")
		parseFlagRecord(tokens, tagIndex);
	else if (tag == "COMMAND")
		parseCommandRecord(resources, tokens, tagIndex);
	else if (tag == "EXEC_LIST")
		parseExecListRecord(tokens, tagIndex);
	else if (tag == "TEXT")
		parseTextRecord(tokens, tagIndex);
	else if (tag == "HEAD")
		parseHeadRecord(resources, tokens, tagIndex);
	else if (tag == "USEITEM")
		parseUseItemRecord(tokens, tagIndex);
	else if (tag == "ENTRANCE")
		parseEntranceRecord(tokens, tagIndex);
	else if (tag == "MAP_ENTRANCE")
		parseMapEntranceRecord(tokens, tagIndex);
	else if (tag == "MAP_LOCATION")
		parseMapLocationRecord(tokens, tagIndex);
	else if (tag == "ROOM")
		parseRoomRecord(resources, tokens, tagIndex);
	else if (tag == "NPC")
		parseNpcRecord(resources, tokens, tagIndex);
	else if (tag == "MONSTER")
		parseMonsterRecord(resources, tokens, tagIndex);
	else if (tag == "TIMER")
		parseTimerRecord(tokens, tagIndex);
	else if (tag == "REGION")
		parseRegionRecord(tokens, tagIndex);
	else if (tag == "ANIM")
		parseAnimRecord(resources, tokens, tagIndex);
	else
		parseObjectRecord(resources, tokens, tagIndex, anonymousObjectId);
}

void Script::parseTownRecords(ResourceManager &resources) {
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

	uint32 anonymousObjectId = 0;

	Common::String line;
	for (uint i = 0; i < _data.size(); ++i) {
		const char ch = static_cast<char>(_data[i]);
		if (ch == '\r')
			continue;
		if (ch == '\n') {
			parseTownRecordLine(resources, line, anonymousObjectId);
			line.clear();
			continue;
		}

		line += ch;
	}

	parseTownRecordLine(resources, line, anonymousObjectId);

	debug(1, "Harvester: parsed %u entrances, %u map entrances, %u map locations, %u rooms, %u objects, %u anims, %u npcs, %u regions, %u flags, %u commands, %u exec_lists, %u texts, %u heads, %u useitems from '%s'",
		(uint)_entrances.size(), (uint)_mapEntrances.size(), (uint)_mapLocations.size(),
		(uint)_rooms.size(), (uint)_objects.size(), (uint)_animations.size(),
		(uint)_npcs.size(), (uint)_regions.size(),
		(uint)_flags.size(), (uint)_commands.size(), (uint)_execLists.size(),
		(uint)_texts.size(), (uint)_heads.size(),
		(uint)_useItems.size(), _path.c_str());
}

} // End of namespace Harvester
