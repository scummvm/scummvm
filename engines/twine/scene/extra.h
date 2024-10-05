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

#ifndef TWINE_SCENE_EXTRA_H
#define TWINE_SCENE_EXTRA_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "twine/scene/actor.h"

namespace TwinE {

#define EXTRA_MAX_ENTRIES 50

#define EXTRA_SPECIAL_MASK 0x8000

struct ShapeData {
	int16 x;
	int16 z;
};

struct ExtraShape {
	int n;
	const ShapeData *data;
};

enum ExtraType {
	TIME_OUT = 1 << 0,        // 0x0001
	FLY = 1 << 1,             // 0x0002
	END_OBJ = 1 << 2,         // 0x0004
	END_COL = 1 << 3,         // 0x0008
	STOP_COL = 1 << 4,        // 0x0010
	TAKABLE = 1 << 5,         // 0x0020
	FLASH = 1 << 6,           // 0x0040
	SEARCH_OBJ = 1 << 7,      // 0x0080
	IMPACT = 1 << 8,          // 0x0100
	MAGIC_BALL_KEY = 1 << 9,  // 0x0200
	TIME_IN = 1 << 10,        // 0x0400
	ONE_FRAME = 1 << 11,      // 0x0800
	EXPLOSION = 1 << 12,      // 0x1000 EXTRA_EXPLO
	WAIT_NO_COL = 1 << 13,    // 0x2000 EXTRA_WAIT_NO_COL
	WAIT_SOME_TIME = 1 << 14, // 0x4000
	COMPUTE_TRAJ = 1 << 15    // 0x8000 used in dotemu enhanced to render the magic ball trajectories
};

struct ExtraListStruct {
	int16 sprite = 0; /**< a value of -1 indicates that this instance is free to use */
	IVec3 pos;
	IVec3 lastPos;
	IVec3 destPos;

	ActorMoveStruct trackActorMove;

	uint16 type = 0; /**< ExtraType bitmask */
	int16 angle = 0; // weight
	int32 spawnTime = 0; // memo timer 50hz
	union payload { // field_ 1C
		int16 lifeTime;
		int16 actorIdx;
		int16 extraIdx;
		int16 unknown;
	} payload{0};
	int16 strengthOfHit = 0; // apply damage if != 0
	int16 info1 = 0;         // various - number for zone giver
};

class TwinEEngine;

class Extra {
private:
	TwinEEngine *_engine;

	void initFly(ExtraListStruct *extra, int32 xAngle, int32 yAngle, int32 x, int32 extraAngle);
	void bounceExtra(ExtraListStruct *extra, int32 x, int32 y, int32 z);
	int32 searchBonusKey() const;
	int32 extraSearchKey(int32 actorIdx, int32 x, int32 y, int32 z, int32 spriteIdx, int32 extraIdx);
	void aff2DShape(const ExtraShape &shapeTable, int32 x, int32 y, int32 color, int32 angle, int32 zoom, Common::Rect &renderRect);

public:
	Extra(TwinEEngine *engine);
	ExtraListStruct _extraList[EXTRA_MAX_ENTRIES];

	int32 extraSearch(int32 actorIdx, int32 x, int32 y, int32 z, int32 spriteIdx, int32 targetActor, int32 maxSpeed, int32 strengthOfHit);

	/**
	 * Add extra explosion
	 * @param x Explosion X coordinate
	 * @param y Explosion Y coordinate
	 * @param z Explosion Z coordinate
	 */
	int32 addExtraExplode(int32 x, int32 y, int32 z);

	inline int32 extraExplo(const IVec3 &pos) {
		return addExtraExplode(pos.x, pos.y, pos.z);
	}

	/** Reset all used extras */
	void resetExtras();

	int32 initSpecial(int32 x, int32 y, int32 z, ExtraSpecialType type);
	int32 addExtraBonus(int32 x, int32 y, int32 z, int32 xAngle, int32 yAngle, int32 type, int32 bonusAmount);

	inline int32 addExtraBonus(const IVec3 &pos, int32 xAngle, int32 yAngle, int32 type, int32 bonusAmount) {
		return addExtraBonus(pos.x, pos.y, pos.z, xAngle, yAngle, type, bonusAmount);
	}

	int32 throwExtra(int32 actorIdx, int32 x, int32 y, int32 z, int32 spriteIdx, int32 xAngle, int32 yAngle, int32 xRotPoint, int32 extraAngle, int32 strengthOfHit);
	int32 addExtraAiming(int32 actorIdx, int32 x, int32 y, int32 z, int32 spriteIdx, int32 targetActorIdx, int32 finalAngle, int32 strengthOfHit);
	void addExtraThrowMagicball(int32 x, int32 y, int32 z, int32 xAngle, int32 yAngle, int32 xRotPoint, int32 extraAngle);

	void affSpecial(int32 extraIdx, int32 x, int32 y, Common::Rect &renderRect);

	int getBonusSprite(BonusParameter bonusParameter) const;

	/** Process extras */
	void gereExtras();
};

} // namespace TwinE

#endif
