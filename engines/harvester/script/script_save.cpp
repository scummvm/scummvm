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
#include "common/serializer.h"
#include "harvester/detection.h"
#include "harvester/resources.h"

namespace Harvester {

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

static void syncStartupFlagRecord(Common::Serializer &s, FlagRecord &record) {
	s.syncString(record.name);
	syncBool(s, record.value);
}

static void syncStartupObjectRecord(Common::Serializer &s, ObjectRecord &record) {
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
	s.syncString(record.reservedXFlag);
	s.syncString(record.identTextKey);
	s.syncString(record.currentOwnerOrRoom);
	s.syncString(record.reservedString40);
	s.syncString(record.inventoryTextKey);
	s.syncString(record.initialOwnerOrRoom);
	s.syncString(record.interactionLabel);
	syncBool(s, record.operatable);
	syncBool(s, record.visible);
	syncBool(s, record.runtimeVisible);
	syncBool(s, record.identShown);
}

static void syncStartupAnimRecord(Common::Serializer &s, AnimRecord &record) {
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

static void syncStartupNpcRecord(Common::Serializer &s, NpcRecord &record) {
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
	s.syncAsSint32LE(record.runtimeState);
	syncBool(s, record.active);
	syncBool(s, record.visible);
	syncBool(s, record.savedVisible);
	s.syncAsSint32LE(record.deathDamageType);
	s.syncString(record.audioPath);
	s.syncString(record.entityInitArg);
}

static void syncStartupMonsterRecord(Common::Serializer &s, MonsterRecord &record) {
	s.syncAsSint32LE(record.posX);
	s.syncAsSint32LE(record.posY);
	s.syncAsSint32LE(record.posZ);
	s.syncAsSint32LE(record.facing);
	s.syncString(record.roomName);
	s.syncString(record.monsterName);
	s.syncString(record.modelPath);
	s.syncString(record.onDeathActionTag);
	syncBool(s, record.active);
	syncBool(s, record.visible);
	s.syncAsSint32LE(record.initialHitPoints);
	s.syncAsSint32LE(record.currentHitPoints);
	s.syncAsSint32LE(record.damageAmount);
	s.syncAsSint32LE(record.engageDistance);
	s.syncAsSint32LE(record.damageType);
	s.syncString(record.reservedString38);
	s.syncString(record.reservedString3c);
	s.syncString(record.reservedString44);
	s.syncString(record.reservedString48);
	s.syncString(record.attackSound1);
	s.syncString(record.attackSound2);
	s.syncString(record.attackSound3);
	s.syncString(record.hitSound1);
	s.syncString(record.hitSound2);
	s.syncString(record.hitSound3);
	s.syncString(record.footstepSoundLeft);
	s.syncString(record.footstepSoundRight);
	s.syncString(record.deathSound);
	syncBool(s, record.savedVisible);
	syncBool(s, record.runtimeSpawned);
	s.syncAsSint32LE(record.screenMinXBound);
	s.syncAsSint32LE(record.screenMaxXBound);
	s.syncAsSint32LE(record.attackSoundTriggerFrame);
	s.syncAsSint32LE(record.hitSoundTriggerFrame);
	s.syncAsSint32LE(record.footstepSoundTriggerFrame);
	s.syncAsSint32LE(record.deathSoundTriggerFrame);
	s.syncAsSint32LE(record.runtimeState);
}

static void restoreMonsterRecordIndices(const Common::Array<MonsterRecord> &baseMonsters,
		Common::Array<MonsterRecord> &runtimeMonsters) {
	for (uint i = 0; i < runtimeMonsters.size(); ++i) {
		if (i < baseMonsters.size() &&
				baseMonsters[i].monsterName.equalsIgnoreCase(runtimeMonsters[i].monsterName) &&
				baseMonsters[i].roomName.equalsIgnoreCase(runtimeMonsters[i].roomName)) {
			runtimeMonsters[i].recordIndex = baseMonsters[i].recordIndex;
			continue;
		}

		runtimeMonsters[i].recordIndex = -1;
		for (const MonsterRecord &baseMonster : baseMonsters) {
			if (!baseMonster.monsterName.equalsIgnoreCase(runtimeMonsters[i].monsterName) ||
					!baseMonster.roomName.equalsIgnoreCase(runtimeMonsters[i].roomName)) {
				continue;
			}

			runtimeMonsters[i].recordIndex = baseMonster.recordIndex;
			break;
		}
	}
}

static void syncStartupTimerRecord(Common::Serializer &s, TimerRecord &record) {
	s.syncAsSint32LE(record.initialValue);
	s.syncAsSint32LE(record.currentValue);
	s.syncString(record.timerName);
	s.syncString(record.arg1);
	s.syncString(record.arg2);
	syncBool(s, record.enabled);
	syncBool(s, record.looping);
	syncBool(s, record.global);
}

static void syncStartupRegionRecord(Common::Serializer &s, RegionRecord &record) {
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

void Script::restoreRuntimeFlags(const Common::Array<FlagRecord> &currentFlags) {
	for (const FlagRecord &flag : currentFlags) {
		FlagRecord *currentFlag = findRuntimeFlag(flag.name);
		if (currentFlag) {
			currentFlag->value = flag.value;
			continue;
		}

		_currentFlags.push_back(flag);
	}
}

void Script::restoreRuntimeObjects(const Common::Array<ObjectRecord> &currentObjects) {
	for (const ObjectRecord &object : currentObjects) {
		ObjectRecord *currentObject = findRuntimeObject(object.initialOwnerOrRoom, object.objectName);
		if (!currentObject)
			currentObject = findRuntimeObject(Common::String(), object.objectName);
		if (!currentObject) {
			_currentObjects.push_back(object);
			continue;
		}

		currentObject->currentX = object.currentX;
		currentObject->currentY = object.currentY;
		currentObject->currentZ = object.currentZ;
		currentObject->currentOwnerOrRoom = object.currentOwnerOrRoom;
		currentObject->visible = object.visible;
		currentObject->runtimeVisible = object.runtimeVisible;
		currentObject->identShown = object.identShown;
	}
}

void Script::restoreRuntimeAnimations(const Common::Array<AnimRecord> &currentAnimations) {
	for (const AnimRecord &anim : currentAnimations) {
		AnimRecord *currentAnimation = findRuntimeAnim(anim.animName);
		if (!currentAnimation) {
			_currentAnimations.push_back(anim);
			continue;
		}

		currentAnimation->active = anim.active;
		currentAnimation->visible = anim.visible;
		currentAnimation->runtimeActive = anim.runtimeActive;
		currentAnimation->runtimeVisible = anim.runtimeVisible;
		currentAnimation->runtimeState = anim.runtimeState;
	}
}

void Script::restoreRuntimeRegions(const Common::Array<RegionRecord> &currentRegions) {
	for (const RegionRecord &region : currentRegions) {
		RegionRecord *currentRegion = findRuntimeRegion(region.regionName);
		if (!currentRegion) {
			_currentRegions.push_back(region);
			continue;
		}

		currentRegion->startEnabled = region.startEnabled;
	}
}

void Script::restoreRuntimeNpcs(const Common::Array<NpcRecord> &currentNpcs) {
	for (const NpcRecord &npc : currentNpcs) {
		NpcRecord *currentNpc = findRuntimeNpc(npc.npcName);
		if (!currentNpc) {
			_currentNpcs.push_back(npc);
			continue;
		}

		currentNpc->active = npc.active;
		currentNpc->visible = npc.visible;
		currentNpc->savedVisible = npc.savedVisible;
		currentNpc->runtimeSpawned = npc.runtimeSpawned;
		currentNpc->runtimeState = npc.runtimeState;
		currentNpc->deathOrMonsterfyFlag = npc.deathOrMonsterfyFlag;
		currentNpc->deathDamageType = npc.deathDamageType;
	}
}

void Script::restoreRuntimeMonsters(const Common::Array<MonsterRecord> &currentMonsters) {
	for (const MonsterRecord &monster : currentMonsters) {
		MonsterRecord *currentMonster = findRuntimeMonster(monster);
		if (!currentMonster) {
			_currentMonsters.push_back(monster);
			continue;
		}

		currentMonster->posX = monster.posX;
		currentMonster->posY = monster.posY;
		currentMonster->posZ = monster.posZ;
		currentMonster->facing = monster.facing;
		currentMonster->active = monster.active;
		currentMonster->visible = monster.visible;
		currentMonster->savedVisible = monster.savedVisible;
		currentMonster->runtimeSpawned = monster.runtimeSpawned;
		currentMonster->runtimeState = monster.runtimeState;
		currentMonster->currentHitPoints = monster.currentHitPoints;
		currentMonster->screenMinXBound = monster.screenMinXBound;
		currentMonster->screenMaxXBound = monster.screenMaxXBound;
	}
}

void Script::restoreRuntimeTimers(const Common::Array<TimerRecord> &currentTimers) {
	for (const TimerRecord &timer : currentTimers) {
		TimerRecord *currentTimer = findRuntimeTimer(timer.timerName);
		if (!currentTimer) {
			_currentTimers.push_back(timer);
			continue;
		}

		currentTimer->initialValue = timer.initialValue;
		currentTimer->currentValue = timer.currentValue;
		currentTimer->enabled = timer.enabled;
		currentTimer->looping = timer.looping;
		currentTimer->global = timer.global;
	}
}

bool Script::reloadTownWorld(ResourceManager &resources) {
	Common::Array<byte> reloadedData;
	if (!resources.loadFile(_path, reloadedData)) {
		warning("Harvester: unable to reload town script '%s'", _path.c_str());
		return false;
	}

	const Common::Array<FlagRecord> currentFlags = _currentFlags;
	const Common::Array<ObjectRecord> currentObjects = _currentObjects;
	const Common::Array<AnimRecord> currentAnimations = _currentAnimations;
	const Common::Array<RegionRecord> currentRegions = _currentRegions;
	const Common::Array<NpcRecord> currentNpcs = _currentNpcs;
	const Common::Array<MonsterRecord> currentMonsters = _currentMonsters;
	const Common::Array<TimerRecord> currentTimers = _currentTimers;
	const int nailgunAmmoCount = _nailgunAmmoCount;
	const int shotgunShellCount = _shotgunShellCount;
	const int nineGunBulletCount = _nineGunBulletCount;
	const int thirtyEightGunBulletCount = _thirtyEightGunBulletCount;
	const int chainsawFuelCount = _chainsawFuelCount;
	const int playerCurrentHitPoints = _playerCurrentHitPoints;
	const int playerCombatLoadout = _playerCombatLoadout;
	const bool playerControlPaused = _playerControlPaused;

	_data = Common::move(reloadedData);
	decode();
	parseTownRecords(resources);
	resetRuntimeState();

	restoreRuntimeFlags(currentFlags);
	restoreRuntimeObjects(currentObjects);
	restoreRuntimeAnimations(currentAnimations);
	restoreRuntimeRegions(currentRegions);
	restoreRuntimeNpcs(currentNpcs);
	restoreRuntimeMonsters(currentMonsters);
	restoreRuntimeTimers(currentTimers);

	_playerCurrentHitPoints = clampPlayerHitPoints(playerCurrentHitPoints);
	_nailgunAmmoCount = clampCombatResourceCount(2, nailgunAmmoCount);
	_shotgunShellCount = clampCombatResourceCount(3, shotgunShellCount);
	_nineGunBulletCount = clampCombatResourceCount(4, nineGunBulletCount);
	_thirtyEightGunBulletCount = clampCombatResourceCount(5, thirtyEightGunBulletCount);
	_chainsawFuelCount = clampCombatResourceCount(14, chainsawFuelCount);
	_playerCombatLoadout = validatePlayerCombatLoadoutAgainstInventory(
		_currentObjects, playerCombatLoadout, "town reload");
	_playerControlPaused = playerControlPaused;

	debugC(1, kDebugGeneral,
		"Harvester: reloaded town script '%s' for disc %d while preserving runtime state",
		_path.c_str(), resources.getCurrentDisc());
	return true;
}

void Script::logRuntimeSaveFlags(const char *operation) const {
	for (const FlagRecord &flag : _currentFlags) {
		const FlagRecord *baseFlag = nullptr;
		for (const FlagRecord &candidate : _flags) {
			if (candidate.name.equalsIgnoreCase(flag.name)) {
				baseFlag = &candidate;
				break;
			}
		}
		if (!baseFlag || flag.value != baseFlag->value) {
			debugC(3, kDebugGeneral,
				"Harvester: %s runtime flag '%s' value=%d base=%d defined_in_script=%d",
				operation, flag.name.c_str(), flag.value, baseFlag ? baseFlag->value : 0,
				baseFlag != nullptr);
		}
	}
}

void Script::logRuntimeSaveObjects(const char *operation) const {
	for (const ObjectRecord &object : _currentObjects) {
		const ObjectRecord *baseObject = nullptr;
		for (const ObjectRecord &candidate : _objects) {
			if (candidate.objectName.equalsIgnoreCase(object.objectName)) {
				baseObject = &candidate;
				break;
			}
		}

		const Common::String &defaultOwner = baseObject ? baseObject->initialOwnerOrRoom : object.initialOwnerOrRoom;
		const int defaultX = baseObject ? baseObject->initialX : object.initialX;
		const int defaultY = baseObject ? baseObject->initialY : object.initialY;
		const int defaultZ = baseObject ? baseObject->initialZ : object.initialZ;
		const bool defaultRuntimeVisible = baseObject ? baseObject->visible : object.visible;
		const bool defaultVisible = baseObject ? baseObject->visible : object.visible;
		const bool defaultIdentShown = baseObject ? baseObject->identTextKey.empty() : object.identTextKey.empty();
		if (!baseObject ||
				!object.currentOwnerOrRoom.equalsIgnoreCase(defaultOwner) ||
				object.currentX != defaultX ||
				object.currentY != defaultY ||
				object.currentZ != defaultZ ||
				object.runtimeVisible != defaultRuntimeVisible ||
				object.visible != defaultVisible ||
				object.identShown != defaultIdentShown) {
			debugC(3, kDebugGeneral,
				"Harvester: %s runtime object '%s' owner='%s' pos=(%d,%d,%d) visible=%d runtimeVisible=%d identShown=%d base_owner='%s' base_pos=(%d,%d,%d) base_visible=%d base_runtimeVisible=%d base_identShown=%d",
				operation, object.objectName.c_str(), object.currentOwnerOrRoom.c_str(),
				object.currentX, object.currentY, object.currentZ,
				object.visible, object.runtimeVisible, object.identShown,
				defaultOwner.c_str(), defaultX, defaultY, defaultZ,
				defaultVisible, defaultRuntimeVisible, defaultIdentShown);
		}
	}
}

void Script::logRuntimeSaveNpcs(const char *operation) const {
	for (const NpcRecord &npc : _currentNpcs) {
		const NpcRecord *baseNpc = nullptr;
		for (const NpcRecord &candidate : _npcs) {
			if (candidate.npcName.equalsIgnoreCase(npc.npcName)) {
				baseNpc = &candidate;
				break;
			}
		}

		const bool defaultActive = baseNpc ? baseNpc->active : npc.active;
		const bool defaultVisible = baseNpc ? baseNpc->visible : npc.visible;
		const bool defaultSavedVisible = baseNpc ? baseNpc->visible : npc.visible;
		const bool defaultDeathOrMonsterfy = baseNpc ? baseNpc->deathOrMonsterfyFlag : npc.deathOrMonsterfyFlag;
		const int defaultDeathDamageType = baseNpc ? baseNpc->deathDamageType : npc.deathDamageType;
		const int defaultRuntimeState = -1;
		if (!baseNpc ||
				npc.active != defaultActive ||
				npc.visible != defaultVisible ||
				npc.savedVisible != defaultSavedVisible ||
				npc.deathOrMonsterfyFlag != defaultDeathOrMonsterfy ||
				npc.deathDamageType != defaultDeathDamageType ||
				npc.runtimeSpawned ||
				npc.runtimeState != defaultRuntimeState) {
			debugC(3, kDebugGeneral,
				"Harvester: %s runtime npc '%s' active=%d visible=%d savedVisible=%d spawned=%d runtimeState=%d deathOrMonsterfy=%d damageType=%d base_active=%d base_visible=%d base_savedVisible=%d base_spawned=%d base_runtimeState=%d base_deathOrMonsterfy=%d base_damageType=%d",
				operation, npc.npcName.c_str(), npc.active, npc.visible, npc.savedVisible,
				npc.runtimeSpawned, npc.runtimeState, npc.deathOrMonsterfyFlag, npc.deathDamageType,
				defaultActive, defaultVisible, defaultSavedVisible,
				0, defaultRuntimeState, defaultDeathOrMonsterfy, defaultDeathDamageType);
		}
	}
}

void Script::logRuntimeSaveMonsters(const char *operation) const {
	for (const MonsterRecord &monster : _currentMonsters) {
		const MonsterRecord *baseMonster = findBaseMonster(monster);

		const bool defaultActive = baseMonster ? baseMonster->active : monster.active;
		const bool defaultVisible = baseMonster
			? (baseMonster->active ? true : baseMonster->visible)
			: (monster.active ? true : monster.visible);
		const bool defaultSavedVisible = defaultVisible;
		const int defaultCurrentHitPoints = baseMonster ? baseMonster->initialHitPoints : monster.initialHitPoints;
		const int defaultScreenMinXBound = baseMonster ? baseMonster->screenMinXBound : monster.screenMinXBound;
		const int defaultScreenMaxXBound = baseMonster ? baseMonster->screenMaxXBound : monster.screenMaxXBound;
		if (!baseMonster ||
				monster.active != defaultActive ||
				monster.visible != defaultVisible ||
				monster.savedVisible != defaultSavedVisible ||
				monster.currentHitPoints != defaultCurrentHitPoints ||
				monster.runtimeSpawned ||
				monster.runtimeState != -1 ||
				monster.screenMinXBound != defaultScreenMinXBound ||
				monster.screenMaxXBound != defaultScreenMaxXBound) {
			debugC(3, kDebugGeneral,
				"Harvester: %s runtime monster '%s' active=%d visible=%d savedVisible=%d spawned=%d runtimeState=%d hp=%d/%d screen_bounds=[%d,%d] base_active=%d base_visible=%d base_savedVisible=%d base_runtimeState=%d base_hp=%d/%d base_screen_bounds=[%d,%d]",
				operation, monster.monsterName.c_str(), monster.active, monster.visible,
				monster.savedVisible, monster.runtimeSpawned, monster.runtimeState,
				monster.currentHitPoints, monster.initialHitPoints,
				monster.screenMinXBound, monster.screenMaxXBound,
				defaultActive, defaultVisible, defaultSavedVisible, -1,
				defaultCurrentHitPoints, baseMonster ? baseMonster->initialHitPoints : monster.initialHitPoints,
				defaultScreenMinXBound, defaultScreenMaxXBound);
		}
	}
}

void Script::logRuntimeSaveTimers(const char *operation) const {
	for (const TimerRecord &timer : _currentTimers) {
		const TimerRecord *baseTimer = nullptr;
		for (const TimerRecord &candidate : _timers) {
			if (candidate.timerName.equalsIgnoreCase(timer.timerName)) {
				baseTimer = &candidate;
				break;
			}
		}

		const int defaultInitialValue = baseTimer ? baseTimer->initialValue : timer.initialValue;
		const int defaultCurrentValue = defaultInitialValue;
		const bool defaultEnabled = baseTimer ? baseTimer->enabled : timer.enabled;
		const bool defaultLooping = baseTimer ? baseTimer->looping : timer.looping;
		const bool defaultGlobal = baseTimer ? baseTimer->global : timer.global;
		if (!baseTimer ||
				timer.initialValue != defaultInitialValue ||
				timer.currentValue != defaultCurrentValue ||
				timer.enabled != defaultEnabled ||
				timer.looping != defaultLooping ||
				timer.global != defaultGlobal) {
			debugC(3, kDebugGeneral,
				"Harvester: %s runtime timer '%s' current=%d initial=%d enabled=%d loop=%d global=%d base_current=%d base_initial=%d base_enabled=%d base_loop=%d base_global=%d",
				operation, timer.timerName.c_str(), timer.currentValue, timer.initialValue,
				timer.enabled, timer.looping, timer.global,
				defaultCurrentValue, defaultInitialValue, defaultEnabled, defaultLooping, defaultGlobal);
		}
	}
}

void Script::logRuntimeSaveState(const char *operation) const {
	debugC(1, kDebugGeneral,
		"Harvester: %s runtime save state flags=%u objects=%u anims=%u regions=%u npcs=%u monsters=%u timers=%u hp=%d loadout=%d paused=%d ammo=[nails:%d shells:%d bullets9:%d bullets38:%d gas:%d]",
		operation, (uint)_currentFlags.size(), (uint)_currentObjects.size(),
		(uint)_currentAnimations.size(), (uint)_currentRegions.size(),
		(uint)_currentNpcs.size(), (uint)_currentMonsters.size(),
		(uint)_currentTimers.size(), _playerCurrentHitPoints, _playerCombatLoadout,
		_playerControlPaused, _nailgunAmmoCount, _shotgunShellCount,
		_nineGunBulletCount, _thirtyEightGunBulletCount, _chainsawFuelCount);

	logRuntimeSaveFlags(operation);
	logRuntimeSaveObjects(operation);
	logRuntimeSaveNpcs(operation);
	logRuntimeSaveMonsters(operation);
	logRuntimeSaveTimers(operation);
}

void Script::syncRuntimeSaveState(Common::Serializer &s) {
	if (s.isSaving())
		logRuntimeSaveState("saving");

	syncRecordArray(s, _currentFlags, syncStartupFlagRecord);
	syncRecordArray(s, _currentObjects, syncStartupObjectRecord);
	syncRecordArray(s, _currentAnimations, syncStartupAnimRecord);
	syncRecordArray(s, _currentRegions, syncStartupRegionRecord);
	syncRecordArray(s, _currentNpcs, syncStartupNpcRecord);
	syncRecordArray(s, _currentMonsters, syncStartupMonsterRecord);
	if (s.isLoading())
		restoreMonsterRecordIndices(_monsters, _currentMonsters);
	s.syncAsSint32LE(_playerCurrentHitPoints);
	syncRecordArray(s, _currentTimers, syncStartupTimerRecord);
	s.syncAsSint32LE(_playerCombatLoadout);
	syncBool(s, _playerControlPaused);
	s.syncAsSint32LE(_nailgunAmmoCount);
	s.syncAsSint32LE(_shotgunShellCount);
	s.syncAsSint32LE(_nineGunBulletCount);
	s.syncAsSint32LE(_thirtyEightGunBulletCount);
	s.syncAsSint32LE(_chainsawFuelCount);
	if (s.isLoading()) {
		_playerCurrentHitPoints = clampPlayerHitPoints(_playerCurrentHitPoints);
		_playerCombatLoadout = clampPlayerCombatLoadout(_playerCombatLoadout);
		_nailgunAmmoCount = clampCombatResourceCount(2, _nailgunAmmoCount);
		_shotgunShellCount = clampCombatResourceCount(3, _shotgunShellCount);
		_nineGunBulletCount = clampCombatResourceCount(4, _nineGunBulletCount);
		_thirtyEightGunBulletCount = clampCombatResourceCount(5, _thirtyEightGunBulletCount);
		_chainsawFuelCount = clampCombatResourceCount(14, _chainsawFuelCount);
		_playerCombatLoadout = validatePlayerCombatLoadoutAgainstInventory(
			_currentObjects, _playerCombatLoadout, "save load");
		logRuntimeSaveState("loaded");
	}
}

} // End of namespace Harvester
