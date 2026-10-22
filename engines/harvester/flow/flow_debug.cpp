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

#include "common/config-manager.h"
#include "common/debug.h"
#include "common/util.h"
#include "harvester/detection.h"
#include "harvester/entity.h"
#include "harvester/harvester.h"
#include "harvester/scene/input.h"
#include "harvester/player/player.h"
#include "harvester/scene/scene_entities.h"

namespace Harvester {

namespace {

static const char *const kPlayerActorEntityName = "PLAYER";
static const int kCursorSequenceWalk = 0;

static void logStartupRoomEntities(EntityManager &entityManager, const SceneState &scene,
		const Common::String &entranceName) {
	if (const Entity *cursor = entityManager.getCursorEntity()) {
		uint32 framePixels = 0;
		uint32 transparentPixels = 0;
		uint32 preservedPixels = 0;
		if (cursor->measureCurrentFrameTransparency(framePixels, transparentPixels, preservedPixels)) {
			debugC(1, kDebugCursor,
				"Harvester: startup probe cursor sequence=%d frame=%d pixels=%u transparent_pixels=%u preserved_pixels=%u",
				cursor->getAnimationSequence(), cursor->getCurrentFrame(), framePixels, transparentPixels, preservedPixels);
		}
	}

	if (const Entity *player = entityManager.findSceneEntityByName(kPlayerActorEntityName)) {
		uint32 framePixels = 0;
		uint32 transparentPixels = 0;
		uint32 preservedPixels = 0;
		(void)player->measureCurrentFrameTransparency(framePixels, transparentPixels, preservedPixels);
		const Common::Rect playerRect = player->getScreenRect();
		const Common::Rect screenRect(0, 0, 640, 480);
		debugC(1, kDebugPlayer,
			"Harvester: startup probe player room='%s' entrance='%s' frame=%d rect=(%d,%d)-(%d,%d) visible=%d intersects_screen=%d opaque_pixels=%u",
			scene.state.roomName.c_str(), entranceName.c_str(), player->getCurrentFrame(),
			playerRect.left, playerRect.top, playerRect.right, playerRect.bottom,
			player->isVisible(), playerRect.intersects(screenRect), framePixels - transparentPixels);
	}
}

static bool findRoomObjectProbePoint(HarvesterEngine &engine, const Common::Array<ObjectRecord> &sceneObjects,
		const ObjectRecord &object, Common::Point &probePoint) {
	const Common::Rect bounds = SceneEntities::getSceneObjectBounds(object);
	if (bounds.isEmpty())
		return false;

	const Common::Point center((bounds.left + bounds.right - 1) / 2, (bounds.top + bounds.bottom - 1) / 2);
	const Common::Point corners[] = {
		center,
		Common::Point(bounds.left, bounds.top),
		Common::Point(bounds.right - 1, bounds.top),
		Common::Point(bounds.left, bounds.bottom - 1),
		Common::Point(bounds.right - 1, bounds.bottom - 1)
	};
	for (const Common::Point &candidate : corners) {
		const ObjectRecord *hit = Input::findRoomObjectAtPoint(engine, sceneObjects, candidate);
		if (hit && hit->objectName.equalsIgnoreCase(object.objectName)) {
			probePoint = candidate;
			return true;
		}
	}

	const int xStep = MAX<int>(1, bounds.width() / 6);
	const int yStep = MAX<int>(1, bounds.height() / 6);
	for (int y = bounds.top; y < bounds.bottom; y += yStep) {
		for (int x = bounds.left; x < bounds.right; x += xStep) {
			const Common::Point candidate(x, y);
			const ObjectRecord *hit = Input::findRoomObjectAtPoint(engine, sceneObjects, candidate);
			if (hit && hit->objectName.equalsIgnoreCase(object.objectName)) {
				probePoint = candidate;
				return true;
			}
		}
	}

	return false;
}

static void logStartupRoomMovement(HarvesterEngine &engine, const SceneState &scene,
		const MenuTextConfig &menuTextConfig, EntityManager &entityManager) {
	if (Player::supportsMovementBand(scene.state)) {
		Common::Point floorProbe;
		bool foundFloorProbe = false;
		for (int y = scene.state.roomMaxZScreenY; y <= scene.state.roomMinZScreenY && !foundFloorProbe; y += 12) {
			for (int x = 48; x < 592; x += 16) {
				const RoomHoverState candidateHover = Input::resolveRoomHoverState(
					engine, scene.state, scene.sceneObjects, scene.state.roomNpcs, scene.sceneRegions,
					Common::Point(x, y), menuTextConfig);
				if (candidateHover.cursorSequence == kCursorSequenceWalk) {
					floorProbe = Common::Point(x, y);
					foundFloorProbe = true;
					break;
				}
			}
		}

		const RoomHoverState floorHover = foundFloorProbe
			? Input::resolveRoomHoverState(engine, scene.state, scene.sceneObjects, scene.state.roomNpcs,
				scene.sceneRegions, floorProbe, menuTextConfig)
			: RoomHoverState();
		debugC(1, kDebugRoom,
			"Harvester: startup probe floor room='%s' point=(%d,%d) found=%d cursor_sequence=%d prompt='%s'",
			scene.state.roomName.c_str(), floorProbe.x, floorProbe.y, foundFloorProbe,
			floorHover.cursorSequence, floorHover.promptText.c_str());

		Entity *player = entityManager.findSceneEntityByName(kPlayerActorEntityName);
		if (player && foundFloorProbe && floorHover.cursorSequence == kCursorSequenceWalk) {
			Player probePlayer;
			probePlayer.initialize(scene.state, player, 0);
			probePlayer.setMoveTargetFromScreenPoint(scene.state, floorProbe.x, floorProbe.y);
			for (int i = 0; i < 32 && probePlayer.getState().hasMoveTarget; ++i)
				(void)probePlayer.stepMoveTarget(engine, scene.state, scene.sceneObjects, scene.sceneAnimations);

			const Common::Rect movedRect = player->getScreenRect();
			debugC(1, kDebugPlayer,
				"Harvester: startup probe movement room='%s' target=(%d,%d) final_rect=(%d,%d)-(%d,%d) z=%.2f",
				scene.state.roomName.c_str(), floorProbe.x, floorProbe.y,
				movedRect.left, movedRect.top, movedRect.right, movedRect.bottom, (double)probePlayer.getState().z);

			player->setCurrentFrame(Player::resolveFacingFrame(scene.state.playerFacing));
			(void)applyRoomActorPlacement(scene.state, *player,
				scene.state.playerSpawnX, scene.state.playerSpawnY, (float)scene.state.playerSpawnZ);
		}
	}
}

} // End of anonymous namespace

void logScenePaletteSummary(const char *label, const SceneState &scene, float brightness) {
	debugC(2, kDebugRoom, "Harvester: %s room='%s' palette='%s' brightness=%.2f",
		label, scene.state.roomName.c_str(), scene.state.palettePath.c_str(), (double)brightness);
}

bool shouldRunStartupRoomProbe() {
	return ConfMan.hasKey("harvester_debug_probe_startup_room") &&
		ConfMan.getBool("harvester_debug_probe_startup_room");
}

void Flow::logStartupRoomProbe(const SceneState &scene, const Common::String &entranceName) {
	EntityManager *entityManager = _engine.getRuntimeEntities();
	Script *script = _engine.getScript();
	if (!entityManager || !script)
		return;

	logStartupRoomEntities(*entityManager, scene, entranceName);

	for (const ObjectRecord &object : scene.sceneObjects) {
		if (SceneEntities::getSceneObjectBounds(object).isEmpty())
			continue;

		Common::Point probePoint;
		if (!findRoomObjectProbePoint(_engine, scene.sceneObjects, object, probePoint))
			continue;

		const ObjectRecord *hoveredObject = Input::findRoomObjectAtPoint(_engine, scene.sceneObjects, probePoint);
		if (!hoveredObject)
			continue;

		_mousePos = probePoint;
		const Common::String objectLabel = script->resolveObjectLabel(*hoveredObject);
		ResolvedText inspectText;
		const RoomHoverState hoverState = Input::resolveRoomHoverState(
			_engine, scene.state, scene.sceneObjects, scene.state.roomNpcs, scene.sceneRegions,
			probePoint, _menuTextConfig);
		const bool hasInteraction = script->hasObjectInteraction(*hoveredObject);
		const bool hasInspectText = script->resolveObjectInspectText(*hoveredObject, inspectText);
		debugC(1, kDebugRoom,
			"Harvester: startup probe hotspot room='%s' object='%s' point=(%d,%d) label='%s' prompt='%s' cursor_sequence=%d action_tag='%s' interaction=%d next_room='%s' inspect=%d",
			scene.state.roomName.c_str(), hoveredObject->objectName.c_str(), probePoint.x, probePoint.y,
			objectLabel.c_str(), hoverState.promptText.c_str(), hoverState.cursorSequence,
			hoveredObject->actionTag.c_str(), hasInteraction,
			"", hasInspectText);

		logStartupRoomMovement(_engine, scene, _menuTextConfig, *entityManager);
		return;
	}

	debugC(1, kDebugRoom, "Harvester: startup probe found no interactive hotspot in room '%s'",
		scene.state.roomName.c_str());
}

} // End of namespace Harvester
