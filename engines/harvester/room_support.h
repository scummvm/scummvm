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

#ifndef HARVESTER_ROOM_SUPPORT_H
#define HARVESTER_ROOM_SUPPORT_H

#include "common/array.h"
#include "common/rect.h"
#include "common/scummsys.h"
#include "common/str.h"
#include "harvester/script.h"

namespace Graphics {
class Font;
class Screen;
}

namespace Harvester {

class HarvesterEngine;
class ResourceManager;
class Entity;
class Art;
class DialogueSystem;
struct IndexedBitmap;

struct RoomSceneResources {
	RoomSetupState state;
	byte palette[256 * 3] = { 0 };
	Common::Array<ObjectRecord> sceneObjects;
	Common::Array<AnimRecord> sceneAnimations;
	Common::Array<RegionRecord> sceneRegions;
	float targetPaletteBrightness = 1.0f;
};

struct RoomPlayerState {
	Entity *entity = nullptr;
	int centerX = 0;
	int bottomY = 0;
	float z = 0.0f;
	int facing = -1;
	// Mirrors native RenderEntityRuntime.player_combat_loadout_id (+0x11bc).
	int combatLoadout = 0;
	uint32 nextMovementTick = 0;
	bool hasMoveTarget = false;
	int targetX = 0;
	int targetBottomY = 0;
	float targetZ = 0.0f;
	bool moveTargetRequiresScreenY = false;
	bool moveTargetXReached = false;
	bool moveTargetZReached = false;
	Common::String currentBlockerName;
	// Native RenderEntityRuntime keeps these directional blocker-history
	// pointers at +0x108c..+0x1098 and clears them when no current
	// opaque blocker overlaps the actor.
	Common::String upwardBlockerHistory;
	Common::String downwardBlockerHistory;
	Common::String leftwardBlockerHistory;
	Common::String rightwardBlockerHistory;
	bool turnActive = false;
	int turnTargetFacing = -1;
	int turnFirstFrame = -1;
	int turnLastFrame = -1;
	int turnEndFrame = -1;
	bool turnPlayBackwards = false;
	bool attackActive = false;
	int attackFirstFrame = -1;
	int attackLastFrame = -1;
	int attackContactFrame = -1;
	int attackResumeFacing = -1;
	bool attackSoundPlayed = false;
	int attackSoundPlaybackFrame = -1;
	bool attackContactResolved = false;
	// Native room combat tracks the live chase/attack focus through
	// RenderEntityRuntime.combat_target_entity (+0x11a4).
	Common::String attackTargetName;
	int attackTargetClassId = -1;
	bool hitActive = false;
	int hitFirstFrame = -1;
	int hitLastFrame = -1;
	int hitResumeFacing = -1;
	int hitKnockbackRemainingX = 0;
	int hitKnockbackDecayStep = 0;
	bool deathActive = false;
	int deathFirstFrame = -1;
	int deathLastFrame = -1;
	int deathDamageType = 0;
};

struct RoomHoverState {
	const ObjectRecord *object = nullptr;
	const NpcRecord *npc = nullptr;
	const RegionRecord *region = nullptr;
	const Entity *playerEntity = nullptr;
	Common::String promptText;
	int cursorSequence = 7;
};

struct RoomIdleAnimationState {
	Entity *entity = nullptr;
	int restoreFacing = -1;
	uint32 activityTick = 0;
	uint32 resetTick = 0;
	uint32 triggerTick = 0;
	bool active = false;
	bool loopStarted = false;
	bool exiting = false;
};

bool applyRoomActorPlacement(const RoomSetupState &state, Entity &entity,
	int centerX, int bottomY, float z, const Common::String *entranceName = nullptr);
void logScenePaletteSummary(const char *label, const RoomSceneResources &scene, float brightness);
bool captureScreenBackdrop(const Graphics::Screen &screen, IndexedBitmap &bitmap);
Common::Rect getRoomObjectHotspotBounds(const ObjectRecord &object);
bool shouldDisplaySceneAnimation(const AnimRecord &anim);
bool loadRoomSceneResources(const RoomSetupState &state, ResourceManager &resources, RoomSceneResources &scene);
bool shouldRunStartupRoomProbe();
void drawRoomScene(HarvesterEngine &engine, Graphics::Screen &screen, const RoomSceneResources &scene,
	float brightness);
const ObjectRecord *findSceneObjectByName(const Common::Array<ObjectRecord> &objects,
	const Common::String &objectName);
const RegionRecord *findSceneRegionByName(const Common::Array<RegionRecord> &regions,
	const Common::String &regionName);
ObjectRecord *findSceneObjectByName(Common::Array<ObjectRecord> &objects,
	const Common::String &objectName);
const IndexedBitmap *resolveInspectTextboxBitmap(const Art &art, const ResolvedText &text);
void drawRoomInspectText(Graphics::Screen &screen, const Art &art, const Graphics::Font &font,
	const ResolvedText &inspectText, bool useNativeFont);
bool unlocksRoomObjectInteractionAfterInitialExamine(const ObjectRecord &object,
	Script &script);
bool doesPlayerFacingMatchRegion(int playerFacing, const RegionRecord &region);
bool doesPlayerOverlapRegion(const Entity &playerEntity, const RegionRecord &region);
RoomHoverState resolveRoomHoverState(HarvesterEngine &engine, const RoomSetupState &state,
	const Common::Array<ObjectRecord> &sceneObjects,
	const Common::Array<NpcRecord> &npcs,
	const Common::Array<RegionRecord> &regions,
	const Common::Point &mousePos, const DialogueSystem *dialogue = nullptr);
void logStartupRoomProbe(HarvesterEngine &engine, const RoomSceneResources &scene,
	const Common::String &entranceName, Common::Point &mousePos);

} // End of namespace Harvester

#endif // HARVESTER_ROOM_SUPPORT_H
