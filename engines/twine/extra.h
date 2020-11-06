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

#include "twine/actor.h"
#include "common/scummsys.h"

#ifndef TWINE_EXTRA_H
#define TWINE_EXTRA_H

namespace TwinE {

#define EXTRA_MAX_ENTRIES 50

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
	int16 type = 0;  // field_14
	int16 angle = 0; // field_16
	int32 lifeTime = 0;
	int16 actorIdx = 0;      // field_ 1C
	int16 strengthOfHit = 0; // field_1E
	int16 info1 = 0;         // field_20
};

enum ExtraSpecialType {
	kHitStars = 0,
	kExplodeCloud = 1
};

class TwinEEngine;

class Extra {
private:
	TwinEEngine *_engine;

	void throwExtra(ExtraListStruct *extra, int32 var1, int32 var2, int32 var3, int32 var4);
	void processMagicballBounce(ExtraListStruct *extra, int32 x, int32 y, int32 z);
	int32 findExtraKey();
	int32 addExtraAimingAtKey(int32 actorIdx, int32 x, int32 y, int32 z, int32 spriteIdx, int32 extraIdx);
	void drawSpecialShape(const int16 *shapeTable, int32 x, int32 y, int32 color, int32 angle, int32 size);

public:
	Extra(TwinEEngine *engine);
	ExtraListStruct extraList[EXTRA_MAX_ENTRIES];

	int32 addExtra(int32 actorIdx, int32 x, int32 y, int32 z, int32 info0, int32 targetActor, int32 maxSpeed, int32 strengthOfHit);

	/**
	 * Add extra explosion
	 * @param x Explostion X coordinate
	 * @param y Explostion Y coordinate
	 * @param z Explostion Z coordinate
	 */
	int32 addExtraExplode(int32 x, int32 y, int32 z);

	/** Reset all used extras */
	void resetExtras();

	void addExtraSpecial(int32 x, int32 y, int32 z, ExtraSpecialType type);
	int32 addExtraBonus(int32 x, int32 y, int32 z, int32 param, int32 angle, int32 type, int32 bonusAmount);
	int32 addExtraThrow(int32 actorIdx, int32 x, int32 y, int32 z, int32 sprite, int32 var2, int32 var3, int32 var4, int32 var5, int32 strengthOfHit);
	int32 addExtraAiming(int32 actorIdx, int32 x, int32 y, int32 z, int32 spriteIdx, int32 targetActorIdx, int32 maxSpeed, int32 strengthOfHit);
	void addExtraThrowMagicball(int32 x, int32 y, int32 z, int32 param1, int32 angle, int32 param2, int32 param3);

	void drawExtraSpecial(int32 extraIdx, int32 x, int32 y);

	/** Process extras */
	void processExtras();
};

} // namespace TwinE

#endif
