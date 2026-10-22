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

namespace Harvester {

const char *const Script::kInventoryOwnerName = "INVENTORY";
const int Script::kDefaultPlayerCombatLoadout = 0;

static bool matchesMonsterIdentity(const MonsterRecord &candidate, const MonsterRecord &monster) {
	if (!candidate.monsterName.equalsIgnoreCase(monster.monsterName))
		return false;

	if (candidate.recordIndex >= 0 && monster.recordIndex >= 0)
		return candidate.recordIndex == monster.recordIndex;

	if (!monster.roomName.empty())
		return candidate.roomName.equalsIgnoreCase(monster.roomName);

	return true;
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

static Common::String normalizeInteractionLabel(const Common::String &value) {
	Common::String label = value;
	for (uint i = 0; i < label.size(); ++i) {
		if (label[i] == '_')
			label.setChar(' ', i);
	}

	return label;
}

void Script::resetRuntimeState() {
	_currentFlags = _flags;
	_currentObjects = _objects;
	_currentAnimations = _animations;
	_currentRegions = _regions;
	_currentNpcs = _npcs;
	_currentMonsters = _monsters;
	_currentTimers = _timers;
	_nailgunAmmoCount = 0;
	_shotgunShellCount = 0;
	_nineGunBulletCount = 0;
	_thirtyEightGunBulletCount = 0;
	_chainsawFuelCount = 0;
	_playerCurrentHitPoints = Script::kDefaultPlayerHitPoints;
	_playerCombatLoadout = kDefaultPlayerCombatLoadout;
	_playerControlPaused = false;

	for (ObjectRecord &object : _currentObjects) {
		object.currentX = object.initialX;
		object.currentY = object.initialY;
		object.currentZ = object.initialZ;
		object.currentOwnerOrRoom = object.initialOwnerOrRoom;
		object.runtimeVisible = object.visible;
		object.identShown = object.identTextKey.empty();
	}

	for (AnimRecord &anim : _currentAnimations) {
		anim.runtimeActive = anim.active;
		anim.runtimeVisible = anim.visible;
		anim.runtimeState = -1;
	}

	for (NpcRecord &npc : _currentNpcs) {
		npc.runtimeSpawned = false;
		npc.runtimeState = -1;
		npc.savedVisible = npc.visible;
	}

	for (MonsterRecord &monster : _currentMonsters) {
		monster.currentHitPoints = monster.initialHitPoints;
		monster.runtimeSpawned = false;
		monster.runtimeState = -1;
		monster.savedVisible = monster.visible;
		if (monster.active)
			monster.visible = true;
	}

	for (TimerRecord &timer : _currentTimers)
		timer.currentValue = timer.initialValue;
}

void Script::getVisibleInventoryObjects(Common::Array<ObjectRecord> &objects) const {
	objects.clear();
	const char *const statusObjectName = resolveInventoryStatusObjectName(_playerCurrentHitPoints);
	const ObjectRecord *statusObject = nullptr;

	for (const ObjectRecord &object : _currentObjects) {
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

	ObjectRecord activeStatusObject = *statusObject;
	activeStatusObject.currentOwnerOrRoom = kInventoryOwnerName;
	activeStatusObject.visible = true;
	activeStatusObject.runtimeVisible = true;
	activeStatusObject.identShown = true;
	objects.push_back(Common::move(activeStatusObject));
}

bool Script::isObjectInInventory(const Common::String &objectName) const {
	if (objectName.empty())
		return false;

	for (const ObjectRecord &object : _currentObjects) {
		if (object.currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName) &&
				object.objectName.equalsIgnoreCase(objectName))
			return true;
	}

	return false;
}

void Script::markObjectIdentShown(const ObjectRecord &object) {
	if (ObjectRecord *currentObject = findRuntimeObject(object.currentOwnerOrRoom, object.objectName))
		currentObject->identShown = true;
}

const EntranceRecord *Script::findEntranceRecord(const Common::String &entranceName) const {
	if (entranceName.empty())
		return nullptr;

	for (const EntranceRecord &entrance : _entrances) {
		if (entrance.entranceName.equalsIgnoreCase(entranceName))
			return &entrance;
	}

	return nullptr;
}

const MapEntranceRecord *Script::findMapEntranceRecord(const Common::String &entryName) const {
	if (entryName.empty())
		return nullptr;

	for (const MapEntranceRecord &mapEntrance : _mapEntrances) {
		if (mapEntrance.entryName.equalsIgnoreCase(entryName))
			return &mapEntrance;
	}

	return nullptr;
}

const UseItemRecord *Script::findUseItemRecord(const Common::String &itemName,
		const ObjectRecord &target) const {
	if (itemName.empty() || target.objectName.empty())
		return nullptr;

	for (const UseItemRecord &useItem : _useItems) {
		if (!useItem.itemName.equalsIgnoreCase(itemName) ||
			!useItem.targetName.equalsIgnoreCase(target.objectName)) {
			continue;
		}

		// Native dispatch ignores the parsed owner/group string and matches only
		// the carried item name plus the overlapped target entity name.
		return &useItem;
	}

	return nullptr;
}

const RoomRecord *Script::findRoomRecord(const Common::String &roomName) const {
	if (roomName.empty())
		return nullptr;

	for (const RoomRecord &room : _rooms) {
		if (room.roomName.equalsIgnoreCase(roomName))
			return &room;
	}

	return nullptr;
}

const CommandRecord *Script::findCommandRecord(const Common::String &tag) const {
	if (tag.empty())
		return nullptr;

	for (const CommandRecord &command : _commands) {
		if (command.triggerTag.equalsIgnoreCase(tag))
			return &command;
	}

	return nullptr;
}

const ExecListRecord *Script::findExecListRecord(const Common::String &name) const {
	if (name.empty())
		return nullptr;

	for (const ExecListRecord &execList : _execLists) {
		if (execList.listName.equalsIgnoreCase(name))
			return &execList;
	}

	return nullptr;
}

const FlagRecord *Script::findRuntimeFlag(const Common::String &flagName) const {
	if (flagName.empty())
		return nullptr;

	for (const FlagRecord &flag : _currentFlags) {
		if (flag.name.equalsIgnoreCase(flagName))
			return &flag;
	}

	return nullptr;
}

FlagRecord *Script::findRuntimeFlag(const Common::String &flagName) {
	if (flagName.empty())
		return nullptr;

	for (FlagRecord &flag : _currentFlags) {
		if (flag.name.equalsIgnoreCase(flagName))
			return &flag;
	}

	return nullptr;
}

ObjectRecord *Script::findRuntimeObject(const Common::String &ownerOrRoom,
		const Common::String &objectName) {
	if (objectName.empty())
		return nullptr;

	ObjectRecord *fallback = nullptr;
	for (ObjectRecord &object : _currentObjects) {
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

AnimRecord *Script::findRuntimeAnim(const Common::String &animName) {
	if (animName.empty())
		return nullptr;

	for (AnimRecord &anim : _currentAnimations) {
		if (anim.animName.equalsIgnoreCase(animName))
			return &anim;
	}

	return nullptr;
}

RegionRecord *Script::findRuntimeRegion(const Common::String &regionName) {
	if (regionName.empty())
		return nullptr;

	for (RegionRecord &region : _currentRegions) {
		if (region.regionName.equalsIgnoreCase(regionName))
			return &region;
	}

	return nullptr;
}

NpcRecord *Script::findRuntimeNpc(const Common::String &npcName) {
	if (npcName.empty())
		return nullptr;

	for (NpcRecord &npc : _currentNpcs) {
		if (npc.npcName.equalsIgnoreCase(npcName))
			return &npc;
	}

	return nullptr;
}

MonsterRecord *Script::findRuntimeMonster(const Common::String &monsterName) {
	if (monsterName.empty())
		return nullptr;

	for (MonsterRecord &monster : _currentMonsters) {
		if (monster.monsterName.equalsIgnoreCase(monsterName))
			return &monster;
	}

	return nullptr;
}

MonsterRecord *Script::findRuntimeMonster(const MonsterRecord &monster) {
	if (monster.monsterName.empty())
		return nullptr;

	MonsterRecord *fallback = nullptr;
	for (MonsterRecord &candidate : _currentMonsters) {
		if (!candidate.monsterName.equalsIgnoreCase(monster.monsterName))
			continue;
		if (matchesMonsterIdentity(candidate, monster))
			return &candidate;
		if (monster.roomName.empty() && monster.recordIndex < 0 && !fallback)
			fallback = &candidate;
	}

	return fallback;
}

TimerRecord *Script::findRuntimeTimer(const Common::String &timerName) {
	if (timerName.empty())
		return nullptr;

	for (TimerRecord &timer : _currentTimers) {
		if (timer.timerName.equalsIgnoreCase(timerName))
			return &timer;
	}

	return nullptr;
}

const NpcRecord *Script::findRuntimeNpc(const Common::String &npcName) const {
	if (npcName.empty())
		return nullptr;

	for (const NpcRecord &npc : _currentNpcs) {
		if (npc.npcName.equalsIgnoreCase(npcName))
			return &npc;
	}

	return nullptr;
}

const MonsterRecord *Script::findRuntimeMonster(const Common::String &monsterName) const {
	if (monsterName.empty())
		return nullptr;

	for (const MonsterRecord &monster : _currentMonsters) {
		if (monster.monsterName.equalsIgnoreCase(monsterName))
			return &monster;
	}

	return nullptr;
}

const MonsterRecord *Script::findRuntimeMonster(const MonsterRecord &monster) const {
	if (monster.monsterName.empty())
		return nullptr;

	const MonsterRecord *fallback = nullptr;
	for (const MonsterRecord &candidate : _currentMonsters) {
		if (!candidate.monsterName.equalsIgnoreCase(monster.monsterName))
			continue;
		if (matchesMonsterIdentity(candidate, monster))
			return &candidate;
		if (monster.roomName.empty() && monster.recordIndex < 0 && !fallback)
			fallback = &candidate;
	}

	return fallback;
}

const MonsterRecord *Script::findBaseMonster(const MonsterRecord &monster) const {
	if (monster.monsterName.empty())
		return nullptr;

	const MonsterRecord *fallback = nullptr;
	for (const MonsterRecord &candidate : _monsters) {
		if (!candidate.monsterName.equalsIgnoreCase(monster.monsterName))
			continue;
		if (matchesMonsterIdentity(candidate, monster))
			return &candidate;
		if (monster.roomName.empty() && monster.recordIndex < 0 && !fallback)
			fallback = &candidate;
	}

	return fallback;
}

const TimerRecord *Script::findRuntimeTimer(const Common::String &timerName) const {
	if (timerName.empty())
		return nullptr;

	for (const TimerRecord &timer : _currentTimers) {
		if (timer.timerName.equalsIgnoreCase(timerName))
			return &timer;
	}

	return nullptr;
}

const TimerRecord *Script::findRuntimeTimerRecord(const Common::String &timerName) const {
	return findRuntimeTimer(timerName);
}

const NpcRecord *Script::findRuntimeNpcRecord(const Common::String &npcName) const {
	return findRuntimeNpc(npcName);
}

bool Script::addRuntimeObjectToInventory(const Common::String &objectName) {
	ObjectRecord *currentObject = findRuntimeObject(Common::String(), objectName);
	if (!currentObject)
		return false;

	const bool changed = !currentObject->currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName) ||
		!currentObject->visible || !currentObject->runtimeVisible || !currentObject->identShown;
	currentObject->currentOwnerOrRoom = kInventoryOwnerName;
	currentObject->visible = true;
	currentObject->runtimeVisible = true;
	currentObject->identShown = true;
	return changed;
}

bool Script::syncRuntimeObjectRecord(const ObjectRecord &object) {
	if (object.objectName.empty())
		return false;

	ObjectRecord *currentObject = findRuntimeObject(Common::String(), object.objectName);
	if (!currentObject)
		return false;

	const bool changed = currentObject->currentX != object.currentX ||
		currentObject->currentY != object.currentY ||
		currentObject->currentZ != object.currentZ ||
		!currentObject->currentOwnerOrRoom.equalsIgnoreCase(object.currentOwnerOrRoom) ||
		currentObject->visible != object.visible ||
		currentObject->runtimeVisible != object.runtimeVisible ||
		currentObject->identShown != object.identShown;
	*currentObject = object;
	return changed;
}

bool Script::setRuntimeObjectVisible(const Common::String &ownerOrRoom,
		const Common::String &objectName, bool visible) {
	if (objectName.empty())
		return false;

	ObjectRecord *currentObject = findRuntimeObject(ownerOrRoom, objectName);
	if (!currentObject)
		return false;

	const bool changed = currentObject->visible != visible ||
		currentObject->runtimeVisible != visible;
	currentObject->visible = visible;
	currentObject->runtimeVisible = visible;
	if (visible && currentObject->currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName))
		currentObject->identShown = true;

	return changed;
}

bool Script::syncRuntimeAnimState(const Common::String &animName, bool active, bool visible, int currentFrame) {
	AnimRecord *currentAnimation = findRuntimeAnim(animName);
	if (!currentAnimation)
		return false;

	const bool changed = currentAnimation->active != active ||
		currentAnimation->visible != visible ||
		currentAnimation->runtimeActive != active ||
		currentAnimation->runtimeVisible != visible ||
		currentAnimation->runtimeState != currentFrame;
	currentAnimation->active = active;
	currentAnimation->visible = visible;
	currentAnimation->runtimeActive = active;
	currentAnimation->runtimeVisible = visible;
	currentAnimation->runtimeState = currentFrame;
	return changed;
}

bool Script::syncRuntimeTimerRecord(const TimerRecord &timer) {
	TimerRecord *currentTimer = findRuntimeTimer(timer.timerName);
	if (!currentTimer)
		return false;

	*currentTimer = timer;
	return true;
}

bool Script::setRuntimeRegionEnabled(const Common::String &regionName, bool enabled) {
	RegionRecord *currentRegion = findRuntimeRegion(regionName);
	if (!currentRegion)
		return false;

	const bool changed = currentRegion->startEnabled != enabled;
	currentRegion->startEnabled = enabled;
	return changed;
}

bool Script::setRuntimeTimerEnabled(const Common::String &timerName, bool enabled) {
	TimerRecord *currentTimer = findRuntimeTimer(timerName);
	if (!currentTimer)
		return false;

	const bool wasEnabled = currentTimer->enabled;
	const bool changed = wasEnabled != enabled ||
		(enabled && !wasEnabled && currentTimer->currentValue != currentTimer->initialValue);
	currentTimer->enabled = enabled;
	if (enabled && !wasEnabled)
		currentTimer->currentValue = currentTimer->initialValue;
	return changed;
}

const TextRecord *Script::findTextRecord(const Common::String &key) const {
	if (key.empty())
		return nullptr;

	for (const TextRecord &textRecord : _texts) {
		if (textRecord.key.equalsIgnoreCase(key))
			return &textRecord;
	}

	return nullptr;
}

bool Script::resolveObjectInspectText(const ObjectRecord &object, ResolvedText &text) const {
	text = ResolvedText();

	const TextRecord *textRecord = findTextRecord(object.identTextKey);
	if (!textRecord)
		return false;

	text.boxName = textRecord->boxName;
	text.value = textRecord->value;
	return !text.value.empty();
}

bool Script::resolveTextRecord(const Common::String &key, ResolvedText &text) const {
	text = ResolvedText();

	const TextRecord *textRecord = findTextRecord(key);
	if (!textRecord)
		return false;

	text.boxName = textRecord->boxName;
	text.value = textRecord->value;
	return !text.value.empty();
}

Common::String Script::resolveInventoryTooltipText(const ObjectRecord &object) const {
	const TextRecord *textRecord = findTextRecord(object.inventoryTextKey);
	if (textRecord && !textRecord->value.empty())
		return textRecord->value;

	return Common::String();
}

Common::String Script::resolveObjectLabel(const ObjectRecord &object) const {
	if (!object.interactionLabel.empty() && !object.interactionLabel.equalsIgnoreCase("NULL_ID"))
		return normalizeInteractionLabel(object.interactionLabel);
	if (object.interactionLabel.equalsIgnoreCase("NULL_ID"))
		return Common::String();

	if (object.currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName)) {
		const Common::String inventoryLabel = resolveInventoryTooltipText(object);
		if (!inventoryLabel.empty())
			return inventoryLabel;
	}

	const TextRecord *textRecord = findTextRecord(object.identTextKey);
	if (textRecord && !textRecord->value.empty())
		return textRecord->value;

	Common::String label = normalizeInteractionLabel(object.objectName);
	if (!label.empty() && !label.equalsIgnoreCase("NULL ID"))
		return label;

	return Common::String();
}

Common::String Script::resolveTextValue(const Common::String &key) const {
	const TextRecord *textRecord = findTextRecord(key);
	if (textRecord && !textRecord->value.empty())
		return textRecord->value;

	return normalizeInteractionLabel(key);
}

const HeadRecord *Script::findHeadRecord(const Common::String &headId) const {
	if (headId.empty())
		return nullptr;

	for (const HeadRecord &head : _heads) {
		if (head.headId.equalsIgnoreCase(headId))
			return &head;
	}

	return nullptr;
}

bool Script::getFlagValue(const Common::String &flagName) const {
	const FlagRecord *flag = findRuntimeFlag(flagName);
	return flag && flag->value;
}

bool Script::setRuntimeFlagValue(const Common::String &flagName, bool value) {
	if (flagName.empty())
		return false;

	FlagRecord *flag = findRuntimeFlag(flagName);
	if (flag) {
		const bool changed = flag->value != value;
		const bool oldValue = flag->value;
		flag->value = value;
		debugC(1, kDebugGeneral,
			"Harvester: direct runtime flag '%s' %d -> %d existed=1 changed=%d",
			flagName.c_str(), oldValue, value, changed);
		return changed;
	}

	FlagRecord newFlag;
	newFlag.name = flagName;
	newFlag.value = value;
	_currentFlags.push_back(Common::move(newFlag));
	debugC(1, kDebugGeneral,
		"Harvester: direct runtime flag '%s' %d -> %d existed=0 changed=1",
		flagName.c_str(), 0, value);
	return true;
}

bool Script::resetRuntimeObjectToInitialState(const Common::String &objectName) {
	if (objectName.empty())
		return false;

	ObjectRecord *currentObject = findRuntimeObject(Common::String(), objectName);
	if (!currentObject)
		return false;

	const ObjectRecord *baseObject = nullptr;
	for (const ObjectRecord &object : _objects) {
		if (object.objectName.equalsIgnoreCase(objectName)) {
			baseObject = &object;
			break;
		}
	}
	if (!baseObject)
		return false;

	const bool changed = currentObject->currentX != currentObject->initialX ||
		currentObject->currentY != currentObject->initialY ||
		currentObject->currentZ != currentObject->initialZ ||
		!currentObject->currentOwnerOrRoom.equalsIgnoreCase(currentObject->initialOwnerOrRoom) ||
		currentObject->visible != baseObject->visible ||
		currentObject->runtimeVisible != baseObject->visible ||
		currentObject->identShown != baseObject->identShown;

	currentObject->currentX = currentObject->initialX;
	currentObject->currentY = currentObject->initialY;
	currentObject->currentZ = currentObject->initialZ;
	currentObject->currentOwnerOrRoom = currentObject->initialOwnerOrRoom;
	currentObject->visible = baseObject->visible;
	currentObject->runtimeVisible = baseObject->visible;
	currentObject->identShown = baseObject->identShown;
	return changed;
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
