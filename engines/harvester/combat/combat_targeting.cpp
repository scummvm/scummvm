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

#include "harvester/combat/combat.h"

#include "common/debug.h"
#include "common/util.h"
#include "harvester/detection.h"
#include "harvester/player/player.h"
#include "harvester/entity.h"

namespace Harvester {

const int kNativeAttackSideWindow = 30;

namespace {

static MonsterRecord *findRoomMonsterRecordByName(RoomSetupState &state, const Common::String &name) {
	for (MonsterRecord &monster : state.roomMonsters) {
		if (monster.monsterName.equalsIgnoreCase(name))
			return &monster;
	}
	return nullptr;
}

static bool isKillableMonster(const MonsterRecord *monster) {
	return monster && monster->active && monster->visible && monster->currentHitPoints > 0;
}

enum NativeAttackSide {
	kNativeAttackSideNone,
	kNativeAttackSideLeft,
	kNativeAttackSideRight
};

enum NativeAttackBand {
	kNativeAttackBandMid,
	kNativeAttackBandUpper,
	kNativeAttackBandLower
};

static bool resolveNativePlayerAttackGeometry(int attackFirstFrame, NativeAttackSide &attackSide, NativeAttackBand &attackBand) {
	attackSide = kNativeAttackSideNone;
	attackBand = kNativeAttackBandMid;

	switch (attackFirstFrame) {
	case 0x50:
		attackSide = kNativeAttackSideLeft;
		attackBand = kNativeAttackBandUpper;
		return true;
	case 0x5a:
		attackSide = kNativeAttackSideLeft;
		attackBand = kNativeAttackBandMid;
		return true;
	case 0x64:
		attackSide = kNativeAttackSideLeft;
		attackBand = kNativeAttackBandLower;
		return true;
	case 0x6e:
		attackSide = kNativeAttackSideRight;
		attackBand = kNativeAttackBandUpper;
		return true;
	case 0x78:
		attackSide = kNativeAttackSideRight;
		attackBand = kNativeAttackBandMid;
		return true;
	case 0x82:
		attackSide = kNativeAttackSideRight;
		attackBand = kNativeAttackBandLower;
		return true;
	default:
		return false;
	}
}

struct NativeAttackGeometry {
	NativeAttackSide side = kNativeAttackSideNone;
	NativeAttackBand band = kNativeAttackBandMid;
};

static NativeAttackGeometry resolveKeyboardAttackGeometry(int facing,
		bool attackLeft, bool attackRight, bool attackUp, bool attackDown) {
	NativeAttackGeometry geometry;
	if (attackRight) {
		geometry.side = kNativeAttackSideRight;
		geometry.band = kNativeAttackBandMid;
	} else if (attackLeft) {
		geometry.side = kNativeAttackSideLeft;
		geometry.band = kNativeAttackBandMid;
	} else if (attackUp) {
		if (facing == 2) {
			geometry.side = kNativeAttackSideRight;
			geometry.band = kNativeAttackBandUpper;
		} else if (facing == 1) {
			geometry.side = kNativeAttackSideLeft;
			geometry.band = kNativeAttackBandUpper;
		}
	} else if (attackDown) {
		if (facing == 2) {
			geometry.side = kNativeAttackSideRight;
			geometry.band = kNativeAttackBandLower;
		} else if (facing == 1) {
			geometry.side = kNativeAttackSideLeft;
			geometry.band = kNativeAttackBandLower;
		}
	}
	return geometry;
}

static bool isBetterDirectionalAttackTarget(const Common::Rect &playerRect, int playerCenterX,
		const Common::Rect &targetRect, NativeAttackSide attackSide, NativeAttackBand attackBand,
		int bestLeadingEdge) {
	const int playerLeft = playerRect.left;
	const int playerRight = playerRect.right;
	const int targetLeft = targetRect.left;
	const int targetCenterX = targetLeft + targetRect.width() / 2;
	bool matchesDirection = false;
	if (attackSide == kNativeAttackSideLeft) {
		if (attackBand == kNativeAttackBandMid)
			matchesDirection = targetLeft < playerCenterX;
		else
			matchesDirection =
				targetLeft < playerCenterX && targetCenterX > playerLeft - kNativeAttackSideWindow;
		if (!matchesDirection || targetLeft <= bestLeadingEdge)
			return false;
	} else {
		if (attackBand == kNativeAttackBandMid)
			matchesDirection = playerCenterX < targetLeft;
		else
			matchesDirection =
				playerCenterX < targetLeft && targetCenterX < playerRight + kNativeAttackSideWindow;
		if (!matchesDirection || targetLeft >= bestLeadingEdge)
			return false;
	}
	return true;
}

} // End of anonymous namespace

Entity *Combat::findSceneRuntimeEntity(EntityManager *entityManager, const Common::String &name) {
	return entityManager ? entityManager->findSceneEntityByName(name) : nullptr;
}

NpcRecord *Combat::findRoomNpcRecordByName(RoomSetupState &state, const Common::String &name) {
	for (NpcRecord &npc : state.roomNpcs) {
		if (npc.npcName.equalsIgnoreCase(name))
			return &npc;
	}
	return nullptr;
}

Entity *Combat::selectMonsterEntity(EntityManager *entityManager, const MonsterRecord &monster) {
	Entity *entity = findSceneRuntimeEntity(entityManager, monster.monsterName);
	return (entity && entity->isVisible()) ? entity : nullptr;
}

MonsterRecord *Combat::findMonsterTargetAtPoint(const Common::Point &point) const {
	MonsterRecord *bestMonster = nullptr;
	int bestZ = -0x7fffffff;
	int bestTop = -0x7fffffff;
	for (MonsterRecord &monster : _scene.state.roomMonsters) {
		if (!monster.active || !monster.visible || monster.currentHitPoints <= 0)
			continue;

		Entity *entity = findSceneRuntimeEntity(_entityManager, monster.monsterName);
		if (!entity || !entity->isVisible())
			continue;

		const Common::Rect entityRect = entity->getScreenRect();
		if (!entityRect.contains(point))
			continue;

		if (!bestMonster || monster.posZ > bestZ ||
				(monster.posZ == bestZ && entityRect.top > bestTop)) {
			bestMonster = &monster;
			bestZ = monster.posZ;
			bestTop = entityRect.top;
		}
	}

	return bestMonster;
}

MonsterRecord *Combat::findOverlappingMonsterTarget() const {
	const RoomPlayerState &playerState = _player.getState();
	if (!playerState.entity)
		return nullptr;

	for (MonsterRecord &monster : _scene.state.roomMonsters) {
		if (!monster.active || !monster.visible || monster.currentHitPoints <= 0)
			continue;

		Entity *entity = findSceneRuntimeEntity(_entityManager, monster.monsterName);
		if (entity && areCombatantsWithinRoomCombatReach(_scene.state,
				*playerState.entity, playerState.z, *entity, static_cast<float>(monster.posZ), monster.engageDistance))
			return &monster;
	}

	return nullptr;
}

NpcRecord *Combat::findOverlappingNpcTarget() const {
	const RoomPlayerState &playerState = _player.getState();
	if (!playerState.entity)
		return nullptr;

	for (NpcRecord &npc : _scene.state.roomNpcs) {
		if (!npc.visible || npc.deathOrMonsterfyFlag || npc.deathDamageType != 0)
			continue;

		Entity *entity = findSceneRuntimeEntity(_entityManager, npc.npcName);
		if (entity && areCombatantsWithinRoomCombatReach(_scene.state,
				*playerState.entity, playerState.z, *entity, entity->getZ(), 0))
			return &npc;
	}

	return nullptr;
}

bool Combat::playerAttackCanReachTarget(Entity *targetEntity, int engageDistance) const {
	const RoomPlayerState &playerState = _player.getState();
	if (!playerState.entity || !targetEntity)
		return false;
	if (Player::isProjectileCombatLoadout(playerState.combatLoadout))
		return true;

	return areCombatantsWithinRoomCombatReach(_scene.state,
		*playerState.entity, playerState.z, *targetEntity, targetEntity->getZ(), engageDistance);
}

MonsterRecord *Combat::findProjectileContactMonsterTarget() const {
	const RoomPlayerState &playerState = _player.getState();
	if (!playerState.entity)
		return nullptr;

	NativeAttackSide attackSide = kNativeAttackSideNone;
	NativeAttackBand attackBand = kNativeAttackBandMid;
	if (!resolveNativePlayerAttackGeometry(playerState.attackFirstFrame, attackSide, attackBand))
		return nullptr;

	const Common::Rect playerRect = playerState.entity->getScreenRect();
	const int playerLeft = playerRect.left;
	const int playerCenterX = playerLeft + playerRect.width() / 2;
	MonsterRecord *bestMonster = nullptr;
	Entity *bestEntity = nullptr;
	int bestLeadingEdge = attackSide == kNativeAttackSideLeft ? -0x7fffffff : 0x7fffffff;

	for (MonsterRecord &monster : _scene.state.roomMonsters) {
		if (!monster.active || !monster.visible || monster.currentHitPoints <= 0)
			continue;

		Entity *entity = findSceneRuntimeEntity(_entityManager, monster.monsterName);
		if (!entity || !entity->isVisible() ||
				!doRuntimeEntityDepthExtentsOverlap(*playerState.entity, *entity)) {
			continue;
		}

		const Common::Rect targetRect = entity->getScreenRect();
		const int targetLeft = targetRect.left;
		if (!isBetterDirectionalAttackTarget(playerRect, playerCenterX,
				targetRect, attackSide, attackBand, bestLeadingEdge))
			continue;

		bestMonster = &monster;
		bestEntity = entity;
		bestLeadingEdge = targetLeft;
	}

	if (bestMonster && bestEntity) {
		debugC(1, kDebugCombat,
			"Harvester: combat player projectile contact fallback target='%s' attack_frame=%d player_rect=(%d,%d)-(%d,%d) target_rect=(%d,%d)-(%d,%d)",
			bestMonster->monsterName.c_str(), playerState.attackFirstFrame,
			playerRect.left, playerRect.top, playerRect.right, playerRect.bottom,
			bestEntity->getScreenRect().left, bestEntity->getScreenRect().top,
			bestEntity->getScreenRect().right, bestEntity->getScreenRect().bottom);
	}

	return bestMonster;
}

void Combat::capturePlayerAttackTargetForKeyboard(bool attackLeft, bool attackRight, bool attackUp, bool attackDown) const {
	const RoomPlayerState &playerState = _player.getState();
	_player.clearAttackTarget();
	if (!playerState.entity)
		return;

	const NativeAttackGeometry geometry = resolveKeyboardAttackGeometry(
		playerState.facing, attackLeft, attackRight, attackUp, attackDown);
	if (geometry.side == kNativeAttackSideNone) {
		debugC(1, kDebugCombat,
			"Harvester: combat player keyboard target capture class='none' input=(L=%d R=%d U=%d D=%d) facing=%d reason='no native attack family'",
			attackLeft, attackRight, attackUp, attackDown, playerState.facing);
		return;
	}

	const Common::Rect playerRect = playerState.entity->getScreenRect();
	// Keyboard selection uses the cached center; projectile fallback uses the frame rectangle.
	const int playerCenterX = playerState.centerX;
	Entity *bestEntity = nullptr;
	Common::String bestName;
	int bestClassId = -1;
	int bestLeadingEdge = geometry.side == kNativeAttackSideLeft ? -0x7fffffff : 0x7fffffff;

	auto considerTarget = [&](Entity *targetEntity, const Common::String &targetName, int classId) {
		if (!targetEntity || !targetEntity->isVisible() ||
				!doRuntimeEntityDepthExtentsOverlap(*playerState.entity, *targetEntity)) {
			return;
		}

		const Common::Rect targetRect = targetEntity->getScreenRect();
		const int targetLeft = targetRect.left;
		if (!isBetterDirectionalAttackTarget(playerRect, playerCenterX,
				targetRect, geometry.side, geometry.band, bestLeadingEdge))
			return;

		bestEntity = targetEntity;
		bestName = targetName;
		bestClassId = classId;
		bestLeadingEdge = targetLeft;
	};

	for (NpcRecord &npc : _scene.state.roomNpcs) {
		if (!npc.visible || npc.deathOrMonsterfyFlag || npc.deathDamageType != 0)
			continue;
		considerTarget(findSceneRuntimeEntity(_entityManager, npc.npcName), npc.npcName, kEntityClassNpc);
	}
	for (MonsterRecord &monster : _scene.state.roomMonsters) {
		if (!monster.active || !monster.visible || monster.currentHitPoints <= 0)
			continue;
		considerTarget(findSceneRuntimeEntity(_entityManager, monster.monsterName), monster.monsterName, kEntityClassMonster);
	}

	if (!bestEntity) {
		debugC(1, kDebugCombat,
			"Harvester: combat player keyboard target capture class='none' input=(L=%d R=%d U=%d D=%d) facing=%d reason='no directional target'",
			attackLeft, attackRight, attackUp, attackDown, playerState.facing);
		return;
	}

	_player.captureAttackTarget(bestName, bestClassId);
	debugC(1, kDebugCombat,
		"Harvester: combat player keyboard target capture class='%s' name='%s' input=(L=%d R=%d U=%d D=%d) facing=%d target_rect=(%d,%d)-(%d,%d)",
		describeCombatTargetClass(bestClassId), bestName.c_str(),
		attackLeft, attackRight, attackUp, attackDown, playerState.facing,
		bestEntity->getScreenRect().left, bestEntity->getScreenRect().top,
		bestEntity->getScreenRect().right, bestEntity->getScreenRect().bottom);
}

MonsterRecord *Combat::resolveKillTarget(const Common::Point &mousePos) const {
	const RoomPlayerState &playerState = _player.getState();
	if (playerState.attackTargetClassId == kEntityClassMonster) {
		MonsterRecord *monster = findRoomMonsterRecordByName(_scene.state, playerState.attackTargetName);
		if (isKillableMonster(monster) && selectMonsterEntity(_entityManager, *monster))
			return monster;
	}

	if (MonsterRecord *monster = findMonsterTargetAtPoint(mousePos)) {
		if (isKillableMonster(monster) && selectMonsterEntity(_entityManager, *monster))
			return monster;
	}

	if (playerState.entity) {
		for (uint i = 0; i < _scene.state.roomMonsters.size() && i < _monsterCombatStates.size(); ++i) {
			MonsterRecord &monster = _scene.state.roomMonsters[i];
			const MonsterCombatState &combatState = _monsterCombatStates[i];
			if (!combatState.attackActive || !isKillableMonster(&monster) || !selectMonsterEntity(_entityManager, monster))
				continue;
			if (!combatState.attackTargetName.empty() &&
					playerState.entity->getName().equalsIgnoreCase(combatState.attackTargetName)) {
				return &monster;
			}
		}
	}

	if (!playerState.entity)
		return nullptr;

	const Common::Rect playerRect = playerState.entity->getScreenRect();
	const int playerCenterX = playerRect.left + playerRect.width() / 2;
	MonsterRecord *bestMonster = nullptr;
	int bestScore = 0x7fffffff;
	for (MonsterRecord &monster : _scene.state.roomMonsters) {
		if (!isKillableMonster(&monster))
			continue;

		Entity *entity = selectMonsterEntity(_entityManager, monster);
		if (!entity)
			continue;

		const Common::Rect monsterRect = entity->getScreenRect();
		const int monsterCenterX = monsterRect.left + monsterRect.width() / 2;
		const int score = ABS(monsterCenterX - playerCenterX) +
			ABS(monster.posZ - roundRoomCombatFloat(playerState.z)) * 8;
		if (!bestMonster || score < bestScore) {
			bestMonster = &monster;
			bestScore = score;
		}
	}

	return bestMonster;
}

void Combat::capturePlayerAttackTarget(const Common::Point &mousePos, const NpcRecord *hoverNpc) const {
	const RoomPlayerState &playerState = _player.getState();
	if (hoverNpc) {
		_player.captureAttackTarget(hoverNpc->npcName, kEntityClassNpc);
		debugC(1, kDebugCombat,
			"Harvester: combat player target capture class='npc' name='%s' cursor=(%d,%d)",
			playerState.attackTargetName.c_str(), mousePos.x, mousePos.y);
		return;
	}

	if (MonsterRecord *monster = findMonsterTargetAtPoint(mousePos)) {
		_player.captureAttackTarget(monster->monsterName, kEntityClassMonster);
		debugC(1, kDebugCombat,
			"Harvester: combat player target capture class='monster' name='%s' cursor=(%d,%d)",
			playerState.attackTargetName.c_str(), mousePos.x, mousePos.y);
		return;
	}

	debugC(1, kDebugCombat,
		"Harvester: combat player target capture class='none' cursor=(%d,%d)",
		mousePos.x, mousePos.y);
}

void Combat::captureOverlappingNpcAttackTarget() const {
	const RoomPlayerState &playerState = _player.getState();
	if (playerState.attackTargetClassId < 0 && !Player::isProjectileCombatLoadout(playerState.combatLoadout)) {
		if (NpcRecord *fallbackNpc = findOverlappingNpcTarget()) {
			_player.captureAttackTarget(fallbackNpc->npcName, kEntityClassNpc);
		}
	}
}

NpcRecord *Combat::selectNpcAttackContactTarget(Entity *&npcEntity) const {
	const RoomPlayerState &playerState = _player.getState();
	NpcRecord *npc = findRoomNpcRecordByName(_scene.state, playerState.attackTargetName);
	npcEntity = npc ? findSceneRuntimeEntity(_entityManager, npc->npcName) : nullptr;
	if (!npc || !npcEntity || !playerAttackCanReachTarget(npcEntity))
		npc = Player::isProjectileCombatLoadout(playerState.combatLoadout) ? nullptr : findOverlappingNpcTarget();
	npcEntity = npc ? findSceneRuntimeEntity(_entityManager, npc->npcName) : nullptr;
	return npc;
}

MonsterRecord *Combat::selectMonsterAttackContactTarget(Entity *&monsterEntity) const {
	const RoomPlayerState &playerState = _player.getState();
	MonsterRecord *monster = nullptr;
	if (playerState.attackTargetClassId == kEntityClassMonster)
		monster = findRoomMonsterRecordByName(_scene.state, playerState.attackTargetName);
	monsterEntity = monster ? findSceneRuntimeEntity(_entityManager, monster->monsterName) : nullptr;
	if (!monster || !monsterEntity || !playerAttackCanReachTarget(monsterEntity, monster->engageDistance)) {
		if (!Player::isProjectileCombatLoadout(playerState.combatLoadout)) {
			monster = findOverlappingMonsterTarget();
		} else if (playerState.attackTargetClassId < 0) {
			monster = findProjectileContactMonsterTarget();
			if (monster) {
				_player.captureAttackTarget(monster->monsterName, kEntityClassMonster);
			}
		} else {
			monster = nullptr;
		}
		monsterEntity = monster ? findSceneRuntimeEntity(_entityManager, monster->monsterName) : nullptr;
	}
	return monster;
}

} // End of namespace Harvester
