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

#include "common/debug.h"
#include "common/endian.h"
#include "harvester/detection.h"
#include "harvester/player/player.h"
#include "harvester/resources.h"

namespace Harvester {

static const float kDefaultPaletteBrightness = 1.0f;
static const float kDimmedPaletteBrightness = 0.6f;

static bool loadBitmapDimensions(ResourceManager &resources, const Common::String &path,
		uint32 &width, uint32 &height) {
	Common::Array<byte> data;
	if (!resources.loadFile(path, data) || data.size() < 8)
		return false;

	width = READ_LE_UINT32(data.data());
	height = READ_LE_UINT32(data.data() + 4);
	return true;
}

static bool isRoomBackgroundSpriteCandidate(const ObjectRecord &candidate,
		const RoomRecord &room) {
	if (!candidate.currentOwnerOrRoom.equalsIgnoreCase(room.roomName) ||
			candidate.spritePath.empty() ||
			candidate.initialX != 0 || candidate.initialY != 0) {
		return false;
	}

	return true;
}

static bool isFullscreenRoomBackgroundObject(const ObjectRecord &candidate,
		ResourceManager &resources) {
	if (candidate.spritePath.empty())
		return false;

	uint32 width = 0;
	uint32 height = 0;
	if (!loadBitmapDimensions(resources, candidate.spritePath, width, height))
		return false;

	return width == 640 && height == 480;
}

static void initializeRoomSetupState(const RoomRecord &room, const EntranceRecord *entrance,
		const ObjectRecord &background, ResourceManager &resources, RoomSetupState &state) {
	state.roomName = room.roomName;
	state.backgroundObjectName = background.objectName;
	state.palettePath = room.palettePath;
	state.backgroundPath = background.spritePath;
	state.discNumber = resources.getCurrentDisc();
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
}

static void logRuntimeRoomState(const RoomSetupState &state) {
	for (const ObjectRecord &object : state.roomObjects) {
		debugC(3, kDebugRoom,
			"Harvester: materialized room object room='%s' object='%s' owner='%s' visible=%d runtimeVisible=%d sprite='%s' alt='%s' pos=(%d,%d,%d) bounds=(%d,%d)-(%d,%d) action='%s'",
			state.roomName.c_str(), object.objectName.c_str(), object.currentOwnerOrRoom.c_str(),
			object.visible, object.runtimeVisible, object.spritePath.c_str(), object.altSpritePath.c_str(),
			object.currentX, object.currentY, object.currentZ,
			object.currentX, object.currentY, object.boundsX2, object.boundsY2, object.actionTag.c_str());
	}
	for (const NpcRecord &npc : state.roomNpcs) {
		debugC(3, kDebugRoom,
			"Harvester: materialized room npc room='%s' npc='%s' visible=%d active=%d spawned=%d runtimeState=%d damage_type=%d pos=(%d,%d,%d) frame_delay=%d model='%s' on_death='%s' audio='%s'",
			state.roomName.c_str(), npc.npcName.c_str(), npc.visible, npc.active,
			npc.runtimeSpawned, npc.runtimeState, npc.deathDamageType,
			npc.posX, npc.posY, npc.posZ, npc.frameDelay,
			npc.modelPath.c_str(), npc.onDeathActionTag.c_str(), npc.audioPath.c_str());
	}
	for (const MonsterRecord &monster : state.roomMonsters) {
		debugC(3, kDebugRoom,
			"Harvester: materialized room monster room='%s' monster='%s' visible=%d active=%d spawned=%d runtimeState=%d pos=(%d,%d,%d) facing=%d hp=%d/%d damage=%d engage=%d damage_type='%s' model='%s' on_death='%s'",
			state.roomName.c_str(), monster.monsterName.c_str(), monster.visible, monster.active,
			monster.runtimeSpawned, monster.runtimeState,
			monster.posX, monster.posY, monster.posZ, monster.facing,
			monster.currentHitPoints, monster.initialHitPoints,
			monster.damageAmount, monster.engageDistance, Player::describeCombatDamageType(monster.damageType),
			monster.modelPath.c_str(), monster.onDeathActionTag.c_str());
	}
	for (const TimerRecord &timer : state.roomTimers) {
		debugC(3, kDebugRoom,
			"Harvester: materialized room timer room='%s' timer='%s' current=%d initial=%d enabled=%d loop=%d global=%d",
			state.roomName.c_str(), timer.timerName.c_str(), timer.currentValue, timer.initialValue,
			timer.enabled, timer.looping, timer.global);
	}

	debugC(3, kDebugRoom,
		"Harvester: materializeRoomState room='%s' entrance='%s' spawn=(%d,%d,%d) facing=%d palette='%s' background='%s' music='%s' brightness=%.2f roomObjects=%u roomAnims=%u roomNpcs=%u roomMonsters=%u roomTimers=%u roomRegions=%u",
		state.roomName.c_str(), state.entranceName.c_str(), state.playerSpawnX, state.playerSpawnY,
		state.playerSpawnZ, state.playerFacing, state.palettePath.c_str(), state.backgroundPath.c_str(),
		state.musicPath.c_str(), static_cast<double>(state.paletteBrightness),
		(uint)state.roomObjects.size(), (uint)state.roomAnimations.size(),
		(uint)state.roomNpcs.size(), (uint)state.roomMonsters.size(),
		(uint)state.roomTimers.size(), (uint)state.roomRegions.size());
}

bool Script::resolveRoomSetupState(const Common::String &entranceName, RoomSetupState &state,
		ResourceManager &resources) {
	state = RoomSetupState();

	const EntranceRecord *entrance = findEntranceRecord(entranceName);
	const Common::String resolvedRoomName = entrance ? entrance->roomName : entranceName;
	const RoomRecord *room = findRoomRecord(resolvedRoomName);
	if (!room) {
		warning("Harvester: unresolved room setup target '%s' (resolved room '%s')",
			entranceName.c_str(), resolvedRoomName.c_str());
		return false;
	}

	if (!buildRuntimeRoomState(*room, entrance, resources, state))
		return false;

	debugC(3, kDebugRoom,
		"Harvester: resolveRoomSetupState('%s') -> room='%s' entrance='%s' spawn=(%d,%d,%d) facing=%d palette='%s' background='%s' music='%s' brightness=%.2f roomObjects=%u activeObjects=%u roomAnims=%u roomNpcs=%u roomMonsters=%u roomTimers=%u roomRegions=%u",
		entranceName.c_str(), state.roomName.c_str(), state.entranceName.c_str(),
		state.playerSpawnX, state.playerSpawnY, state.playerSpawnZ, state.playerFacing,
		state.palettePath.c_str(), state.backgroundPath.c_str(), state.musicPath.c_str(),
		static_cast<double>(state.paletteBrightness), (uint)state.roomObjects.size(),
		(uint)state.activeObjects.size(), (uint)state.roomAnimations.size(), (uint)state.roomNpcs.size(),
		(uint)state.roomMonsters.size(), (uint)state.roomTimers.size(),
		(uint)state.roomRegions.size());

	return true;
}

bool Script::resolveRoomSetupStateByRoomName(const Common::String &roomName, RoomSetupState &state,
		ResourceManager &resources) {
	state = RoomSetupState();

	const RoomRecord *room = findRoomRecord(roomName);
	if (!room) {
		warning("Harvester: unresolved room setup room '%s'", roomName.c_str());
		return false;
	}

	const EntranceRecord *entrance = nullptr;
	for (const EntranceRecord &candidate : _entrances) {
		if (!candidate.roomName.equalsIgnoreCase(room->roomName))
			continue;

		entrance = &candidate;
		break;
	}

	if (!buildRuntimeRoomState(*room, entrance, resources, state))
		return false;

	debugC(3, kDebugRoom,
		"Harvester: resolveRoomSetupStateByRoomName('%s') -> room='%s' entrance='%s' spawn=(%d,%d,%d) facing=%d palette='%s' background='%s' music='%s' brightness=%.2f roomObjects=%u activeObjects=%u roomAnims=%u roomNpcs=%u roomMonsters=%u roomTimers=%u roomRegions=%u",
		roomName.c_str(), state.roomName.c_str(), state.entranceName.c_str(),
		state.playerSpawnX, state.playerSpawnY, state.playerSpawnZ, state.playerFacing,
		state.palettePath.c_str(), state.backgroundPath.c_str(), state.musicPath.c_str(),
		static_cast<double>(state.paletteBrightness), (uint)state.roomObjects.size(),
		(uint)state.activeObjects.size(), (uint)state.roomAnimations.size(), (uint)state.roomNpcs.size(),
		(uint)state.roomMonsters.size(), (uint)state.roomTimers.size(),
		(uint)state.roomRegions.size());

	return true;
}

bool Script::materializeRoomState(const Common::String &entranceName, const Common::String &roomName,
		RoomSetupState &state, ResourceManager &resources) const {
	state = RoomSetupState();

	const EntranceRecord *entrance = findEntranceRecord(entranceName);
	const Common::String resolvedRoomName = !roomName.empty()
		? roomName
		: (entrance ? entrance->roomName : Common::String());
	if (resolvedRoomName.empty())
		return false;

	const RoomRecord *room = findRoomRecord(resolvedRoomName);
	if (!room) {
		warning("Harvester: unresolved materialized room target entrance='%s' room='%s'",
			entranceName.c_str(), resolvedRoomName.c_str());
		return false;
	}

	return buildRuntimeRoomState(*room, entrance, resources, state);
}

bool Script::hasRoomSetupTarget(const Common::String &entranceName,
		const Common::String &roomName) const {
	const EntranceRecord *entrance = findEntranceRecord(entranceName);
	const Common::String resolvedRoomName = !roomName.empty()
		? roomName
		: (entrance ? entrance->roomName : Common::String());
	if (resolvedRoomName.empty())
		return false;

	return findRoomRecord(resolvedRoomName) != nullptr;
}

const ObjectRecord *Script::findRoomBackground(const RoomRecord &room,
		ResourceManager &resources) const {
	const ObjectRecord *background = nullptr;
	const ObjectRecord *backgroundFallback = nullptr;
	bool usedBackgroundFallback = false;
	for (const ObjectRecord &candidate : _currentObjects) {
		if (!isRoomBackgroundSpriteCandidate(candidate, room))
			continue;

		if (!backgroundFallback)
			backgroundFallback = &candidate;
		if (isFullscreenRoomBackgroundObject(candidate, resources)) {
			background = &candidate;
			break;
		}
	}
	if (!background && backgroundFallback) {
		background = backgroundFallback;
		usedBackgroundFallback = true;
	}
	if (!background) {
		warning("Harvester: no room background object found for room '%s'", room.roomName.c_str());
		return nullptr;
	}
	if (usedBackgroundFallback) {
		debugC(1, kDebugRoom,
			"Harvester: using origin room sprite as background room='%s' object='%s' sprite='%s'",
			room.roomName.c_str(), background->objectName.c_str(), background->spritePath.c_str());
	}

	return background;
}

void Script::collectRuntimeRoomRecords(const RoomRecord &room, RoomSetupState &state) const {
	for (const ObjectRecord &object : _currentObjects) {
		if (object.visible && object.currentOwnerOrRoom.equalsIgnoreCase(room.roomName))
			state.roomObjects.push_back(object);
	}
	if (!state.hasEntrance) {
		static const char *const kCloseupExitObjects[] = { "EXIT_BM", "EXIT_HS" };
		for (const char *name : kCloseupExitObjects) {
			for (const ObjectRecord &object : _currentObjects) {
				if (object.objectName.equalsIgnoreCase(name)) {
					state.roomObjects.push_back(object);
					break;
				}
			}
		}
	}
	for (const AnimRecord &anim : _currentAnimations) {
		if (anim.roomName.equalsIgnoreCase(room.roomName))
			state.roomAnimations.push_back(anim);
	}
	for (const NpcRecord &npc : _currentNpcs) {
		if (!npc.roomName.equalsIgnoreCase(room.roomName) || !npc.visible)
			continue;
		if (npc.deathOrMonsterfyFlag) {
			debugC(1, kDebugRoom,
				"Harvester: suppressed room npc room='%s' npc='%s' death_or_monsterfy=%d damage_type=%d model='%s' monsterfy_target='%s'",
				room.roomName.c_str(), npc.npcName.c_str(), npc.deathOrMonsterfyFlag, npc.deathDamageType,
				npc.modelPath.c_str(), npc.monsterfyTargetName.c_str());
			continue;
		}

		state.roomNpcs.push_back(npc);
	}
	for (const MonsterRecord &monster : _currentMonsters) {
		if (!monster.roomName.equalsIgnoreCase(room.roomName))
			continue;
		if (!monster.active && !monster.visible)
			continue;

		state.roomMonsters.push_back(monster);
	}
	for (const TimerRecord &timer : _currentTimers) {
		if (timer.arg1.equalsIgnoreCase(room.roomName))
			state.roomTimers.push_back(timer);
	}
	for (const RegionRecord &region : _currentRegions) {
		if (region.roomName.equalsIgnoreCase(room.roomName) && region.startEnabled)
			state.roomRegions.push_back(region);
	}
}

bool Script::buildRuntimeRoomState(const RoomRecord &room, const EntranceRecord *entrance,
		ResourceManager &resources, RoomSetupState &state) const {
	state = RoomSetupState();

	const ObjectRecord *background = findRoomBackground(room, resources);
	if (!background)
		return false;

	initializeRoomSetupState(room, entrance, *background, resources, state);
	collectRuntimeRoomRecords(room, state);

	const FlagRecord *dayFlag = findRuntimeFlag("DAY_FLAG");
	state.paletteBrightness = (room.dimmable && (!dayFlag || !dayFlag->value))
		? kDimmedPaletteBrightness
		: kDefaultPaletteBrightness;

	logRuntimeRoomState(state);
	return true;
}

} // End of namespace Harvester
