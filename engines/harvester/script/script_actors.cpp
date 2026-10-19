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
#include "harvester/player/player.h"

namespace Harvester {

static const int kMaxPlayerCombatLoadout = 0x14;

struct InventoryCombatLoadoutEntry {
	int loadoutId;
	const char *objectName;
};

static const InventoryCombatLoadoutEntry kInventoryCombatLoadoutMap[] = {
	{ 1, "CLEAVER" },
	{ 2, "NAILGUN" },
	{ 3, "SHOTGUN" },
	{ 4, "9GUN" },
	{ 5, "38GUN" },
	{ 6, "TOMAHAWK" },
	{ 7, "KNIFE" },
	{ 8, "FLAIL" },
	{ 9, "HANDAXE" },
	{ 10, "WRENCH" },
	{ 11, "PITCHFORK" },
	{ 12, "SCYTHE" },
	{ 13, "SWORD" },
	{ 14, "CHAINSAW" },
	{ 15, "HARVEST_BLADE" },
	{ 16, "SHOVEL" },
	{ 17, "FIREAXE" },
	{ 18, "BAT" },
	{ 19, "RAZOR" },
	{ 20, "POOLSTICK" }
};

int Script::clampPlayerHitPoints(int hitPoints) {
	if (hitPoints < 0)
		return 0;
	if (hitPoints > Script::kDefaultPlayerHitPoints)
		return Script::kDefaultPlayerHitPoints;

	return hitPoints;
}

int Script::clampPlayerCombatLoadout(int loadout) {
	return CLIP<int>(loadout, 0, kMaxPlayerCombatLoadout);
}

static int resolveCombatResourceDisplayMax(int loadout) {
	switch (loadout) {
	case 2:
	case 3:
	case 14:
		return 16;
	case 4:
		return 8;
	case 5:
		return 6;
	default:
		return 0;
	}
}

int Script::clampCombatResourceCount(int loadout, int count) {
	const int maxCount = resolveCombatResourceDisplayMax(loadout);
	if (maxCount <= 0)
		return 0;

	return CLIP<int>(count, 0, maxCount);
}

int Script::resolveCombatResourcePickupMax(int loadout) {
	switch (loadout) {
	case 2:
	case 3:
	case 14:
		return 15;
	case 4:
		return 7;
	case 5:
		return 5;
	default:
		return 0;
	}
}

static const char *resolveCombatLoadoutInventoryObjectName(int loadout) {
	for (const InventoryCombatLoadoutEntry &entry : kInventoryCombatLoadoutMap) {
		if (entry.loadoutId == loadout)
			return entry.objectName;
	}

	return nullptr;
}

int Script::validatePlayerCombatLoadoutAgainstInventory(const Common::Array<ObjectRecord> &currentObjects,
		int loadout, const char *contextLabel) {
	const int clampedLoadout = clampPlayerCombatLoadout(loadout);
	if (clampedLoadout == kDefaultPlayerCombatLoadout)
		return clampedLoadout;

	const char *requiredObjectName = resolveCombatLoadoutInventoryObjectName(clampedLoadout);
	if (!requiredObjectName)
		return clampedLoadout;

	for (const ObjectRecord &object : currentObjects) {
		if (object.currentOwnerOrRoom.equalsIgnoreCase(kInventoryOwnerName) &&
				object.objectName.equalsIgnoreCase(requiredObjectName))
			return clampedLoadout;
	}

	debugC(1, kDebugGeneral,
		"Harvester: cleared player combat loadout %d('%s') after %s because inventory object '%s' is absent",
		clampedLoadout, Player::describeCombatLoadout(clampedLoadout),
		contextLabel ? contextLabel : "runtime restore", requiredObjectName);
	return kDefaultPlayerCombatLoadout;
}

bool Script::setRuntimeNpcState(const Common::String &npcName, bool active, bool visible) {
	if (npcName.empty())
		return false;

	NpcRecord *currentNpc = findRuntimeNpc(npcName);
	if (!currentNpc)
		return false;

	const bool changed = currentNpc->active != active || currentNpc->visible != visible;
	currentNpc->active = active;
	currentNpc->visible = visible;
	return changed;
}

bool Script::setPlayerCurrentHitPoints(int hitPoints) {
	const int clampedHitPoints = clampPlayerHitPoints(hitPoints);
	const bool changed = _playerCurrentHitPoints != clampedHitPoints;
	_playerCurrentHitPoints = clampedHitPoints;
	return changed;
}

bool Script::adjustPlayerCurrentHitPoints(int delta) {
	return setPlayerCurrentHitPoints(_playerCurrentHitPoints + delta);
}

int Script::getPlayerCombatResourceCount(int loadout) const {
	const int *count = getPlayerCombatResourceCountPtr(loadout);
	return count ? *count : 0;
}

bool Script::consumePlayerCombatResourceUnit(int loadout) {
	const int maxCount = resolveCombatResourcePickupMax(loadout);
	if (maxCount <= 0)
		return true;

	if (getPlayerCombatResourceCount(loadout) < 1)
		return false;

	return adjustPlayerCombatResourceCount(loadout, -1, maxCount, "PLAYER_ATTACK");
}

bool Script::setPlayerCombatResourceCount(int loadout, int count) {
	int *currentCount = getPlayerCombatResourceCountPtr(loadout);
	const int maxCount = resolveCombatResourceDisplayMax(loadout);
	if (!currentCount || maxCount <= 0)
		return false;

	const int clampedCount = CLIP<int>(count, 0, maxCount);
	const bool changed = *currentCount != clampedCount;
	*currentCount = clampedCount;
	return changed;
}

int *Script::getPlayerCombatResourceCountPtr(int loadout) {
	switch (loadout) {
	case 2:
		return &_nailgunAmmoCount;
	case 3:
		return &_shotgunShellCount;
	case 4:
		return &_nineGunBulletCount;
	case 5:
		return &_thirtyEightGunBulletCount;
	case 14:
		return &_chainsawFuelCount;
	default:
		return nullptr;
	}
}

const int *Script::getPlayerCombatResourceCountPtr(int loadout) const {
	switch (loadout) {
	case 2:
		return &_nailgunAmmoCount;
	case 3:
		return &_shotgunShellCount;
	case 4:
		return &_nineGunBulletCount;
	case 5:
		return &_thirtyEightGunBulletCount;
	case 14:
		return &_chainsawFuelCount;
	default:
		return nullptr;
	}
}

bool Script::adjustPlayerCombatResourceCount(int loadout, int delta, int maxCount,
		const Common::String &reason) {
	int *count = getPlayerCombatResourceCountPtr(loadout);
	if (!count || maxCount <= 0)
		return false;

	const int oldCount = *count;
	const int newCount = CLIP<int>(oldCount + delta, 0, maxCount);
	*count = newCount;
	debugC(1, kDebugGeneral,
		"Harvester: %s player combat resource loadout=%d('%s') delta=%d count=%d->%d max=%d changed=%d",
		reason.c_str(), loadout, Player::describeCombatLoadout(loadout),
		delta, oldCount, newCount, maxCount, oldCount != newCount);
	return oldCount != newCount;
}

bool Script::setPlayerCombatLoadout(int loadout) {
	const int clampedLoadout = clampPlayerCombatLoadout(loadout);
	const bool changed = _playerCombatLoadout != clampedLoadout;
	_playerCombatLoadout = clampedLoadout;
	return changed;
}

bool Script::setPlayerControlPaused(bool paused) {
	const bool changed = _playerControlPaused != paused;
	_playerControlPaused = paused;
	return changed;
}

bool Script::syncRuntimeMonsterRecord(const MonsterRecord &monster) {
	MonsterRecord *currentMonster = findRuntimeMonster(monster);
	if (!currentMonster)
		return false;

	*currentMonster = monster;
	return true;
}

bool Script::queueRuntimeNpcDeathOrMonsterfy(const Common::String &npcName, int deathDamageType) {
	if (npcName.empty())
		return false;

	NpcRecord *currentNpc = findRuntimeNpc(npcName);
	if (!currentNpc || currentNpc->deathOrMonsterfyFlag)
		return false;

	// Native live MONSTERFY queues actor state 0x35 without a damage argument.
	// Use bludge as the queued animation selector when scripts leave it blank.
	const int queuedDeathDamageType = deathDamageType != 0 ? deathDamageType : 1;
	if (currentNpc->deathDamageType == queuedDeathDamageType)
		return false;

	currentNpc->deathDamageType = queuedDeathDamageType;
	return true;
}

bool Script::finalizeRuntimeNpcDeathOrMonsterfy(const Common::String &npcName, int deathDamageType,
		bool preserveCorpse, int corpseFrame, int monsterfyPosZ) {
	if (npcName.empty())
		return false;

	NpcRecord *currentNpc = findRuntimeNpc(npcName);
	if (!currentNpc)
		return false;

	bool monsterChanged = false;
	if (!currentNpc->monsterfyTargetName.empty()) {
		MonsterRecord monsterLookup;
		monsterLookup.roomName = currentNpc->roomName;
		monsterLookup.monsterName = currentNpc->monsterfyTargetName;
		MonsterRecord *currentMonster = findRuntimeMonster(monsterLookup);
		if (currentMonster) {
			const bool overrideMonsterPosZ = monsterfyPosZ != kNoMonsterfyPosZOverride;
			monsterChanged = !currentMonster->active || !currentMonster->visible ||
				(overrideMonsterPosZ && currentMonster->posZ != monsterfyPosZ);
			if (overrideMonsterPosZ)
				currentMonster->posZ = monsterfyPosZ;
			currentMonster->active = true;
			currentMonster->visible = true;
			currentMonster->runtimeSpawned = false;
			currentMonster->runtimeState = -1;
			if (currentMonster->currentHitPoints <= 0)
				currentMonster->currentHitPoints = currentMonster->initialHitPoints;
		} else {
			debug(1, "Harvester: unresolved monsterfy target for dialogue npc='%s' target='%s'",
				currentNpc->npcName.c_str(), currentNpc->monsterfyTargetName.c_str());
		}
	}

	const bool changed = !currentNpc->deathOrMonsterfyFlag ||
		currentNpc->active ||
		currentNpc->visible ||
		currentNpc->runtimeSpawned ||
		currentNpc->runtimeState != (preserveCorpse && corpseFrame >= 0 ? corpseFrame : -1) ||
		(deathDamageType != 0 && currentNpc->deathDamageType != deathDamageType);
	currentNpc->active = false;
	currentNpc->visible = preserveCorpse && corpseFrame >= 0 && currentNpc->monsterfyTargetName.empty();
	currentNpc->savedVisible = currentNpc->visible;
	currentNpc->deathOrMonsterfyFlag = !currentNpc->visible;
	currentNpc->runtimeSpawned = currentNpc->visible;
	currentNpc->runtimeState = currentNpc->visible ? corpseFrame : -1;
	if (deathDamageType != 0)
		currentNpc->deathDamageType = deathDamageType;
	return changed || monsterChanged;
}

void Script::pushDeferredLiveNpcDeathTransitions() {
	++_deferredLiveNpcDeathTransitionDepth;
}

void Script::popDeferredLiveNpcDeathTransitions() {
	if (_deferredLiveNpcDeathTransitionDepth > 0)
		--_deferredLiveNpcDeathTransitionDepth;
}

bool Script::isNamedNpcDeathTypeClear(const Common::String &npcName) const {
	const NpcRecord *npc = findRuntimeNpc(npcName);
	return npc && npc->deathDamageType == 0;
}

} // End of namespace Harvester
