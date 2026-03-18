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

#ifndef HARVESTER_STARTUP_ROOM_SUPPORT_H
#define HARVESTER_STARTUP_ROOM_SUPPORT_H

#include "common/array.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "harvester/startup_script.h"

namespace Graphics {
class Font;
class Screen;
}

namespace Harvester {

class HarvesterEngine;
class ResourceManager;
class RuntimeEntity;
class Art;
struct IndexedBitmap;

struct StartupRoomSceneResources {
	StartupRoomSetupState state;
	byte palette[256 * 3] = { 0 };
	Common::Array<StartupObjectRecord> sceneObjects;
	Common::Array<StartupAnimRecord> sceneAnimations;
	Common::Array<StartupRegionRecord> sceneRegions;
	float targetPaletteBrightness = 1.0f;
};

struct StartupRoomPlayerState {
	RuntimeEntity *entity = nullptr;
	int centerX = 0;
	int bottomY = 0;
	float z = 0.0f;
	int facing = -1;
	uint32 nextMovementTick = 0;
	bool hasMoveTarget = false;
	int targetX = 0;
	float targetZ = 0.0f;
	bool turnActive = false;
	int turnTargetFacing = -1;
	int turnFirstFrame = -1;
	int turnLastFrame = -1;
	int turnEndFrame = -1;
	bool turnPlayBackwards = false;
};

struct StartupRoomHoverState {
	const StartupObjectRecord *object = nullptr;
	const StartupNpcRecord *npc = nullptr;
	const StartupRegionRecord *region = nullptr;
	const RuntimeEntity *playerEntity = nullptr;
	Common::String promptText;
	int cursorSequence = 7;
};

struct StartupRoomIdleAnimationState {
	RuntimeEntity *entity = nullptr;
	int restoreFacing = -1;
	uint32 activityTick = 0;
	uint32 resetTick = 0;
	uint32 triggerTick = 0;
	bool active = false;
	bool loopStarted = false;
	bool exiting = false;
};

bool applyRoomActorPlacement(const StartupRoomSetupState &state, RuntimeEntity &entity,
	int centerX, int bottomY, float z, const Common::String *entranceName = nullptr);
void setPlayerMoveTarget(const StartupRoomSetupState &state, StartupRoomPlayerState &playerState,
	int targetX, float targetZ);
void setPlayerMoveTargetFromScreenPoint(const StartupRoomSetupState &state,
	StartupRoomPlayerState &playerState, int targetX, int targetBottomY);
int resolveRegionTargetX(const StartupRegionRecord &region, const StartupRoomPlayerState &playerState);
float resolveRegionTargetZ(const StartupRegionRecord &region);
uint32 getRuntimeClockTicks();
bool isIdleAnimationExcludedRoom(const Common::String &roomName);
void updatePlayerIdleTrigger(StartupRoomIdleAnimationState &idleState);
void logScenePaletteSummary(const char *label, const StartupRoomSceneResources &scene, float brightness);
bool captureScreenBackdrop(const Graphics::Screen &screen, IndexedBitmap &bitmap);
bool loadRoomSceneResources(const StartupRoomSetupState &state, ResourceManager &resources, StartupRoomSceneResources &scene);
bool shouldRunStartupRoomProbe();
void drawRoomScene(HarvesterEngine &engine, Graphics::Screen &screen, const StartupRoomSceneResources &scene,
	float brightness);
const StartupObjectRecord *findSceneObjectByName(const Common::Array<StartupObjectRecord> &objects,
	const Common::String &objectName);
const StartupRegionRecord *findSceneRegionByName(const Common::Array<StartupRegionRecord> &regions,
	const Common::String &regionName);
StartupObjectRecord *findSceneObjectByName(Common::Array<StartupObjectRecord> &objects,
	const Common::String &objectName);
const IndexedBitmap *resolveInspectTextboxBitmap(const Art &art, const StartupResolvedText &text);
void drawRoomInspectText(Graphics::Screen &screen, const Art &art, const Graphics::Font &font,
	const StartupResolvedText &inspectText);
bool unlocksRoomObjectInteractionAfterInitialExamine(const StartupObjectRecord &object,
	Script &startupScript);
bool doesPlayerFacingMatchRegion(int playerFacing, const StartupRegionRecord &region);
bool doesPlayerOverlapRegion(const RuntimeEntity &playerEntity, const StartupRegionRecord &region);
StartupRoomHoverState resolveRoomHoverState(HarvesterEngine &engine, const StartupRoomSetupState &state,
	const Common::Array<StartupObjectRecord> &sceneObjects,
	const Common::Array<StartupNpcRecord> &npcs,
	const Common::Array<StartupRegionRecord> &regions,
	const Common::Point &mousePos);
bool setPlayerIdleAnimation(StartupRoomPlayerState &playerState, int facing);
bool startPlayerTurnAnimation(StartupRoomPlayerState &playerState, int targetFacing);
bool updatePlayerTurnAnimationState(StartupRoomPlayerState &playerState);
bool stepPlayerMoveTarget(HarvesterEngine &engine, const StartupRoomSetupState &state,
	const Common::Array<StartupObjectRecord> &sceneObjects,
	const Common::Array<StartupAnimRecord> &sceneAnimations,
	StartupRoomPlayerState &playerState);
bool stepPlayerKeyboardMovement(HarvesterEngine &engine, const StartupRoomSetupState &state,
	const Common::Array<StartupObjectRecord> &sceneObjects,
	const Common::Array<StartupAnimRecord> &sceneAnimations,
	StartupRoomPlayerState &playerState, bool moveLeft, bool moveRight, bool moveUp, bool moveDown);
bool requestPlayerIdleAnimationExit(const StartupRoomSetupState &state,
	StartupRoomPlayerState &playerState, StartupRoomIdleAnimationState &idleState);
bool startPlayerIdleAnimation(HarvesterEngine &engine, const StartupRoomSetupState &state,
	StartupRoomPlayerState &playerState, StartupRoomIdleAnimationState &idleState);
bool updatePlayerIdleAnimation(const StartupRoomSetupState &state, StartupRoomPlayerState &playerState,
	StartupRoomIdleAnimationState &idleState);
void logStartupRoomProbe(HarvesterEngine &engine, const StartupRoomSceneResources &scene,
	const Common::String &entranceName, Common::Point &mousePos);

} // End of namespace Harvester

#endif // HARVESTER_STARTUP_ROOM_SUPPORT_H
