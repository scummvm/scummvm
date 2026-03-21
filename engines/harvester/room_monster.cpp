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

#include "harvester/room_monster.h"

#include "harvester/runtime_entity.h"

namespace Harvester {

namespace {

static const int kRoomMonsterAnimationRate = 17;

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

} // End of anonymous namespace

int RoomMonsterLogic::resolveFacingFrame(int facing) {
	return resolveAnimationRange(facing).idleFrame;
}

void RoomMonsterLogic::applyAnimation(RuntimeEntity &entity, const StartupMonsterRecord &monster) {
	entity.setVisible(monster.visible);
	if (!monster.visible)
		return;

	const MonsterAnimationRange range = resolveAnimationRange(monster.facing);
	if (monster.active) {
		entity.setAnimationRate(kRoomMonsterAnimationRate);
		entity.setAnimationFrameRange(range.walkFirstFrame, range.walkLastFrame, true);
		entity.setAnimationEnabled(true);
	} else {
		entity.setAnimationRate(0);
		entity.setAnimationFrameRange(range.idleFrame, range.idleFrame, false);
		entity.setCurrentFrame(range.idleFrame);
		entity.setAnimationEnabled(false);
	}
}

} // End of namespace Harvester
