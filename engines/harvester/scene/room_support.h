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
#include "harvester/script/script.h"

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
struct MenuTextConfig;

struct SceneState {
	ScriptExecutionContext executionContext() const { return ScriptExecutionContext(state.roomName); }

	RoomSetupState state;
	byte palette[256 * 3] = { 0 };
	Common::Array<ObjectRecord> sceneObjects;
	Common::Array<AnimRecord> sceneAnimations;
	Common::Array<RegionRecord> sceneRegions;
	float targetPaletteBrightness = 1.0f;
};

struct RoomHoverState {
	const ObjectRecord *object = nullptr;
	const NpcRecord *npc = nullptr;
	const RegionRecord *region = nullptr;
	const Entity *playerEntity = nullptr;
	Common::String promptText;
	int cursorSequence = 7;
};

bool sameMonsterEntityState(const MonsterRecord &lhs, const MonsterRecord &rhs);
bool applyRoomActorPlacement(const RoomSetupState &state, Entity &entity,
	int centerX, int bottomY, float z, const Common::String *entranceName = nullptr);
void logScenePaletteSummary(const char *label, const SceneState &scene, float brightness);
bool captureScreenBackdrop(const Graphics::Screen &screen, IndexedBitmap &bitmap);
bool shouldDisplaySceneAnimation(const AnimRecord &anim);
bool shouldRunStartupRoomProbe();
bool doesPlayerFacingMatchRegion(int playerFacing, const RegionRecord &region);
bool doesPlayerOverlapRegion(const Entity &playerEntity, const RegionRecord &region);

} // End of namespace Harvester

#endif // HARVESTER_ROOM_SUPPORT_H
