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

#ifndef HARVESTER_MONSTER_H
#define HARVESTER_MONSTER_H

#include "common/str.h"

namespace Harvester {

class Entity;

struct MonsterRecord {
	int posX = 0;
	int posY = 0;
	int posZ = 0;
	// Native MONSTER numeric fields after z seed the combat-runtime engage distance,
	// initial HP, and attack damage fields.
	int engageDistance = 0;
	int initialHitPoints = 0;
	int currentHitPoints = 0;
	int damageAmount = 0;
	int damageType = 0;
	int facing = -1;
	Common::String roomName;
	Common::String monsterName;
	Common::String modelPath;
	Common::String reservedString38;
	Common::String reservedString3c;
	Common::String reservedString44;
	Common::String reservedString48;
	Common::String attackSound1;
	Common::String attackSound2;
	Common::String attackSound3;
	Common::String hitSound1;
	Common::String hitSound2;
	Common::String hitSound3;
	Common::String footstepSoundLeft;
	Common::String footstepSoundRight;
	Common::String deathSound;
	Common::String onDeathActionTag;
	int recordIndex = -1;
	bool active = false;
	bool visible = false;
	bool savedVisible = false;
	bool runtimeSpawned = false;
	int runtimeState = -1;
	int screenMinXBound = 0x14;
	int screenMaxXBound = 0x26b;
	int attackSoundTriggerFrame = 0;
	int hitSoundTriggerFrame = 0;
	int footstepSoundTriggerFrame = 0;
	int deathSoundTriggerFrame = 0;
};

class Monster {
public:
	static int resolveFacingFrame(int facing);
	static void applyAnimation(Entity &entity, const MonsterRecord &monster);
};

} // End of namespace Harvester

#endif // HARVESTER_MONSTER_H
