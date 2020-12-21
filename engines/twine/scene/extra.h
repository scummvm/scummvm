/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"
#include "twine/scene/actor.h"

#ifndef TWINE_EXTRA_H
#define TWINE_EXTRA_H

namespace TwinE {

#define EXTRA_MAX_ENTRIES 50

enum ExtraType {
	TIME_OUT = 1 << 0,     // 0x0001
	FLY = 1 << 1,          // 0x0002
	UNK2 = 1 << 2,         // 0x0004
	UNK3 = 1 << 3,         // 0x0008
	STOP_COL = 1 << 4,     // 0x0010
	TAKABLE = 1 << 5,      // 0x0020
	FLASH = 1 << 6,        // 0x0040
	UNK7 = 1 << 7,         // 0x0080
	UNK8 = 1 << 8,         // 0x0100
	UNK9 = 1 << 9,         // 0x0200
	TIME_IN = 1 << 10,     // 0x0400
	UNK11 = 1 << 11,       // 0x0800
	UNK12 = 1 << 12,       // 0x1000
	WAIT_NO_COL = 1 << 13, // 0x2000
	BONUS = 1 << 14,       // 0x4000
	UNK15 = 1 << 15        // 0x8000
};

struct ExtraListStruct {
	int16 info0 = 0; // field_0
	int16 x = 0;
	int16 y = 0;
	int16 z = 0;

	int16 lastX = 0; // field_8
	int16 lastY = 0; // field_A
	int16 lastZ = 0; // field_C

	ActorMoveStruct trackActorMove;

	int16 destX = 0; // field_E
	int16 destY = 0; // field_10
	int16 destZ = 0; // field_12
	uint16 type = 0; /**< ExtraType bitmask */
	int16 angle = 0; // field_16
	int32 spawnTime = 0;
	union payload { // field_ 1C
		int16 lifeTime;
		int16 actorIdx;
		int16 extraIdx;
		int16 unknown;
	} payload{0};
	int16 strengthOfHit = 0; // field_1E
	int16 info1 = 0;         // field_20
};

class TwinEEngine;

class Extra {
private:
	TwinEEngine *_engine;

	void throwExtra(ExtraListStruct *extra, int32 xAngle, int32 yAngle, int32 x, int32 extraAngle);
	void processMagicballBounce(ExtraListStruct *extra, int32 x, int32 y, int32 z);
	int32 findExtraKey();
	int32 addExtraAimingAtKey(int32 actorIdx, int32 x, int32 y, int32 z, int32 spriteIdx, int32 extraIdx);
	void drawSpecialShape(const int16 *shapeTable, int32 x, int32 y, int32 color, int32 angle, int32 size);

public:
	Extra(TwinEEngine *engine);
	ExtraListStruct extraList[EXTRA_MAX_ENTRIES];

	int32 addExtra(int32 actorIdx, int32 x, int32 y, int32 z, int32 spriteIdx, int32 targetActor, int32 maxSpeed, int32 strengthOfHit);

	/**
	 * Add extra explosion
	 * @param x Explosion X coordinate
	 * @param y Explosion Y coordinate
	 * @param z Explosion Z coordinate
	 */
	int32 addExtraExplode(int32 x, int32 y, int32 z);

	/** Reset all used extras */
	void resetExtras();

	int32 addExtraSpecial(int32 x, int32 y, int32 z, ExtraSpecialType type);
	int32 addExtraBonus(int32 x, int32 y, int32 z, int32 xAngle, int32 yAngle, int32 type, int32 bonusAmount);
	int32 addExtraThrow(int32 actorIdx, int32 x, int32 y, int32 z, int32 spriteIdx, int32 xAngle, int32 yAngle, int32 xRotPoint, int32 extraAngle, int32 strengthOfHit);
	int32 addExtraAiming(int32 actorIdx, int32 x, int32 y, int32 z, int32 spriteIdx, int32 targetActorIdx, int32 finalAngle, int32 strengthOfHit);
	void addExtraThrowMagicball(int32 x, int32 y, int32 z, int32 xAngle, int32 yAngle, int32 xRotPoint, int32 extraAngle);

	void drawExtraSpecial(int32 extraIdx, int32 x, int32 y);

	int getBonusSprite(BonusParameter bonusParameter) const;

	/** Process extras */
	void processExtras();
};

} // namespace TwinE

#endif
