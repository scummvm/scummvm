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

#include "harvester/monster.h"

#include "harvester/runtime_entity.h"

namespace Harvester {

namespace {

struct MonsterAnimationRange {
	MonsterAnimationRange() {}
	MonsterAnimationRange(int walkFirstFrame, int walkLastFrame, int idleFrame)
		: walkFirstFrame(walkFirstFrame), walkLastFrame(walkLastFrame), idleFrame(idleFrame) {}

	int walkFirstFrame = 0;
	int walkLastFrame = 0;
	int idleFrame = 0;
};

static MonsterAnimationRange resolveAnimationRange(int facing) {
	switch (facing) {
	case 0:
		return MonsterAnimationRange(0x00, 0x09, 0x3b);
	case 1:
		return MonsterAnimationRange(0x0f, 0x18, 0x0e);
	case 2:
		return MonsterAnimationRange(0x2d, 0x36, 0x2c);
	case 3:
		return MonsterAnimationRange(0x1e, 0x27, 0x28);
	default:
		return MonsterAnimationRange(0x00, 0x09, 0x3b);
	}
}

static int resolveStandingFrame(const Entity &entity, const MonsterAnimationRange &range) {
	if (entity.hasOpaqueFramesInRange(range.idleFrame, range.idleFrame))
		return range.idleFrame;
	if (entity.hasOpaqueFramesInRange(range.walkFirstFrame, range.walkFirstFrame))
		return range.walkFirstFrame;
	return CLIP<int>(range.idleFrame, 0, MAX(0, entity.getLastFrame()));
}

} // End of anonymous namespace

int Monster::resolveFacingFrame(int facing) {
	return resolveAnimationRange(facing).idleFrame;
}

void Monster::applyAnimation(Entity &entity, const MonsterRecord &monster) {
	entity.setVisible(monster.visible);
	if (!monster.visible)
		return;

	// Native monsters keep the last death frame on screen after the death bank
	// completes instead of snapping back to an idle facing frame.
	if (!monster.active &&
			monster.currentHitPoints <= 0 &&
			monster.runtimeSpawned &&
			monster.runtimeState >= 0) {
		int corpseFrame = monster.runtimeState;
		if (corpseFrame > entity.getLastFrame())
			corpseFrame = entity.getLastFrame();
		entity.setAnimationRate(0);
		entity.setAnimationFrameRange(corpseFrame, corpseFrame, false);
		entity.setCurrentFrame(corpseFrame);
		entity.setAnimationEnabled(false);
		return;
	}

	const MonsterAnimationRange range = resolveAnimationRange(monster.facing);
	// Native active monster spawn starts on a standing facing frame; chase code switches to walking.
	const int standingFrame = resolveStandingFrame(entity, range);
	entity.setAnimationRate(0);
	entity.setAnimationFrameRange(standingFrame, standingFrame, false);
	entity.setCurrentFrame(standingFrame);
	entity.setAnimationEnabled(false);
}

} // End of namespace Harvester
