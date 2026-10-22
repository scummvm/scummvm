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

#include "harvester/flow/flow.h"

#include "common/debug.h"
#include "common/util.h"
#include "harvester/detection.h"
#include "harvester/entity.h"
#include "harvester/harvester.h"
#include "harvester/player/player.h"
#include "harvester/scene/scene_entities.h"

namespace Harvester {

namespace {

static const char *const kPlayerActorEntityName = "PLAYER";

struct RoomActorFrameMetrics {
	int width = 0;
	int height = 0;
	int xOffset = 0;
	int yOffset = 0;
};

static void setRoomActorScreenPosition(Entity &entity, int centerX, int bottomY, float z,
		const RoomActorFrameMetrics &metrics) {
	entity.setPosition(centerX - metrics.xOffset - metrics.width / 2, bottomY - metrics.height - metrics.yOffset, z);
}

static float computeRoomActorRenderZ(float z, const Entity &entity) {
	// Native actor spawns lower the render-list depth anchor by half the z extent
	// before insertion. Without that adjustment, cemetery Karin lands one layer
	// too far back and gets occluded by the grave mask in CEM10.
	return z - floorf(MAX<float>(entity.getZExtent(), 0.0f) * 0.5f);
}

static bool applyStartupActorPlacement(const RoomSetupState &state, Entity &entity) {
	return applyRoomActorPlacement(state, entity,
		state.playerSpawnX, state.playerSpawnY, (float)state.playerSpawnZ, &state.entranceName);
}

static void populateRoomTimers(const RoomSetupState &state, EntityManager &entityManager) {
	for (const TimerRecord &timer : state.roomTimers) {
		Entity *existingTimer = entityManager.findSceneEntityByName(timer.timerName);
		if (existingTimer && existingTimer->getClassId() == kEntityClassTimer) {
			debugC(1, kDebugRoom,
				"Harvester: scene timer reused room='%s' timer='%s' current=%d initial=%d enabled=%d loop=%d global=%d",
				state.roomName.c_str(), timer.timerName.c_str(), existingTimer->getTimerCurrentValue(),
				existingTimer->getTimerInitialValue(), existingTimer->isTimerEnabled(),
				existingTimer->isTimerLooping(), existingTimer->isTimerGlobal());
			continue;
		}

		if (!entityManager.spawnSceneTimerEntity(timer.timerName,
				timer.initialValue, timer.currentValue, timer.enabled, timer.looping, timer.global)) {
			debug(1, "Harvester: unable to spawn room timer entity '%s'",
				timer.timerName.c_str());
			continue;
		}

		debugC(1, kDebugRoom,
			"Harvester: scene timer spawned room='%s' timer='%s' current=%d initial=%d enabled=%d loop=%d global=%d",
			state.roomName.c_str(), timer.timerName.c_str(), timer.currentValue, timer.initialValue,
			timer.enabled, timer.looping, timer.global);
	}
}

static void populateRoomAnimations(EntityManager &entityManager,
		const Common::Array<AnimRecord> &drawableAnimations) {
	for (const AnimRecord &anim : drawableAnimations) {
		const bool displayVisible = shouldDisplaySceneAnimation(anim);
		if (!entityManager.spawnSceneAnimationEntity(anim.animName, anim.resourcePath,
				Common::Point(anim.x, anim.y), (float)anim.z, anim.frameDelay, anim.active, displayVisible,
				anim.looping, anim.backward, anim.pingPong, anim.runtimeState)) {
			debug(1, "Harvester: unable to spawn room anim entity '%s' from '%s'",
				anim.animName.c_str(), anim.resourcePath.c_str());
		}
	}
}

static void populateRoomPlayer(HarvesterEngine &engine, RoomSetupState &state,
		EntityManager &entityManager, Entity *preservedPlayer,
		const Common::Array<ObjectRecord> &drawableObjects,
		const Common::Array<AnimRecord> &drawableAnimations) {
	if (state.hasEntrance) {
		const int playerFrame = Player::resolveFacingFrame(state.playerFacing);
		Script *script = engine.getScript();
		const int playerCombatLoadout = script ? script->getPlayerCombatLoadout() : 0;
		const Common::String playerResourcePath =
			Player::resolveCombatLoadoutResourcePath(playerCombatLoadout);
		const bool reusedPlayer = preservedPlayer != nullptr;
		Entity *player = preservedPlayer;
		if (!reusedPlayer) {
			player = entityManager.spawnSceneActorEntity(kPlayerActorEntityName,
				playerResourcePath, Common::Point(state.playerSpawnX, state.playerSpawnY),
				(float)state.playerSpawnZ, playerFrame);
		}
		if (!player) {
			debug(1, "Harvester: unable to spawn startup player actor from '%s'", playerResourcePath.c_str());
		} else {
			player->setClassId(kEntityClassPlayer);
			player->setAnchorMode(kEntityAnchorTopLeft);
			player->setHitTestMode(kEntityHitTestOpaquePixels);
			player->setVisible(true);
			player->setAnimationRate(0);
			player->setAnimationFrameRange(playerFrame, playerFrame, false);
			player->setCurrentFrame(playerFrame);
			if (!applyStartupActorPlacement(state, *player)) {
				debug(1, "Harvester: unable to apply startup player placement for entrance '%s'",
					state.entranceName.c_str());
			}
			if (reusedPlayer) {
				Player loadoutPlayer;
				loadoutPlayer.initialize(state, player, -1);
				(void)loadoutPlayer.syncCombatLoadoutVisual(engine, state, playerCombatLoadout);
			}
			Player spawnPlayer;
			spawnPlayer.initialize(state, player, 0);
			if (spawnPlayer.resolveBlockedStartupSpawn(engine, state, drawableObjects, drawableAnimations)) {
				state.playerSpawnX = spawnPlayer.getState().centerX;
				state.playerSpawnY = spawnPlayer.getState().bottomY;
				state.playerSpawnZ = (int)spawnPlayer.getState().z;
			}
			if (reusedPlayer)
				entityManager.adoptSceneEntity(player);
			else
				entityManager.reinsertSceneEntity(player);
			debugC(1, kDebugPlayer,
				"Harvester: %s startup player actor '%s' entrance='%s' pos=(%d,%d,%d) facing=%d frame=%d",
				reusedPlayer ? "reused" : "spawned", playerResourcePath.c_str(), state.entranceName.c_str(),
				state.playerSpawnX, state.playerSpawnY, state.playerSpawnZ,
				state.playerFacing, playerFrame);
		}
	} else if (preservedPlayer) {
		delete preservedPlayer;
	}
}

} // End of anonymous namespace

bool applyRoomActorPlacement(const RoomSetupState &state, Entity &entity,
		int centerX, int bottomY, float z, const Common::String *entranceName) {
	RoomActorFrameMetrics metrics;
	if (!entity.getCurrentFrameMetrics(metrics.width, metrics.height, metrics.xOffset, metrics.yOffset))
		return false;

	const float renderZ = computeRoomActorRenderZ(z, entity);
	entity.setAnchorMode(kEntityAnchorTopLeft);
	setRoomActorScreenPosition(entity, centerX, bottomY, renderZ, metrics);

	entity.setDepthScale(Player::computeDepthScale(state, renderZ));
	if (!entity.getCurrentFrameMetrics(metrics.width, metrics.height, metrics.xOffset, metrics.yOffset))
		return false;
	setRoomActorScreenPosition(entity, centerX, bottomY, renderZ, metrics);

	return true;
}

bool Flow::populateRoomSceneEntities(RoomSetupState &state,
		const Common::Array<ObjectRecord> &drawableObjects,
		const Common::Array<AnimRecord> &drawableAnimations) {
	EntityManager *entityManager = _engine.getRuntimeEntities();
	if (!entityManager)
		return false;

	SceneEntities sceneRuntime(state, _engine.getScript(), _engine.getResources(), entityManager);
	Entity *preservedPlayer = entityManager->detachSceneEntityByName(kPlayerActorEntityName);
	entityManager->clearSceneEntities(true);
	sceneRuntime.populateRoomRegions();
	populateRoomTimers(state, *entityManager);
	sceneRuntime.populateRoomObjects(drawableObjects);
	populateRoomAnimations(*entityManager, drawableAnimations);
	sceneRuntime.populateRoomNpcs();
	sceneRuntime.populateRoomMonsters();
	populateRoomPlayer(_engine, state, *entityManager, preservedPlayer, drawableObjects, drawableAnimations);

	return true;
}

} // End of namespace Harvester
