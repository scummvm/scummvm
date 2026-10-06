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

// Hopkins-specific ACK data types and constants.

#ifndef HOPKINS_BASE_TYPES_H
#define HOPKINS_BASE_TYPES_H

#include "common/array.h"
#include "common/scummsys.h"

namespace Hopkins {

static const int kBaseMapWidth = 64;
static const int kBaseMapHeight = 64;
static const int kBaseMapCellCount = kBaseMapWidth * kBaseMapHeight;
static const int kBaseAckGridArray = (kBaseMapWidth + 2) * (kBaseMapHeight + 2);
static const int kBaseCellSize = 64;
static const int kBaseCellShift = 6;
static const int kBaseGridMask = 0xffc0;
static const int kBaseWorldExtent = kBaseMapWidth * kBaseCellSize;

static const int kBaseFixedShift = 16;
static const int32 kBaseFixedOne = 1 << kBaseFixedShift;

// Hopkins' ACK fork uses 1920 angular units per revolution, not 1800.
static const int kBaseAngleCount = 1920;
static const int kBaseAngleHalfFov = 160;
static const int kBaseQuarterTurn = 480;
static const int kBaseHalfTurn = 960;
static const int kBaseThreeQuarterTurn = 1440;

static const int kBaseFrameWidth = 320;
static const int kBaseFrameHeight = 200;
static const int kBaseViewWidth = 320;
static const int kBaseViewHeight = 180;
static const int kBaseViewHalfWidth = 160;
static const int kBaseHorizon = 90;
static const int kBaseMaximumDistance = 2048;
static const int kBaseMinimumWallHeight = 8;
static const int kBaseMaximumWallHeight = 960;

static const int kBaseMaxObjects = 60;
static const int kBaseMaxDoors = 20;

static const uint16 kBaseWallTransparent = 0x0800;
static const uint16 kBaseWallMulti = 0x0400;
static const uint16 kBaseWallUpper = 0x0200;
static const uint16 kBaseWallPass = 0x0100;

static const uint16 kBaseDoorSecret = 0x8000;
static const uint16 kBaseDoorLocked = 0x4000;
static const uint16 kBaseDoorSlide = 0x2000;
static const uint16 kBaseDoorSplit = 0x1000;
static const uint16 kBaseDoorOpening = 0x0080;
static const uint16 kBaseDoorClosing = 0x0040;
static const uint16 kBaseDoorXCode = 60;
static const uint16 kBaseDoorSideCode = 59;
static const uint16 kBaseDoorYCode = 62;
static const uint16 kBaseExitBitmap = 15;

inline int normalizeBaseAngle(int angle) {
	angle %= kBaseAngleCount;
	if (angle < 0)
		angle += kBaseAngleCount;
	return angle;
}

inline int baseMapIndex(int x, int y) {
	return y * kBaseMapWidth + x;
}

inline int baseWorldMapIndex(int x, int y) {
	return (y & kBaseGridMask) + (x >> kBaseCellShift);
}

enum BaseObjectMode {
	kBaseObjectChase = 1,
	kBaseObjectFiring = 2,
	kBaseObjectFlee = 3,
	kBaseObjectDying = 4,
	kBaseObjectDead = 5
};

enum BaseRayAxis {
	kBaseRayNone = 0,
	kBaseRayX = 1,
	kBaseRayY = 2
};

struct BaseBitmap {
	uint16 width;
	uint16 height;
	bool columnMajor;
	Common::Array<byte> pixels;
	Common::Array<byte> blankColumns;

	BaseBitmap() : width(0), height(0), columnMajor(false) {}

	bool valid() const {
		return width && height && pixels.size() == (uint32)width * height;
	}

	byte sample(uint x, uint y) const {
		if (!valid())
			return 0;
		x %= width;
		y %= height;
		return columnMajor ? pixels[x * height + y] : pixels[y * width + x];
	}
};

struct BaseDoor {
	int16 mPos;
	int16 mPos1;
	uint16 mCode;
	uint16 mCode1;
	int16 offset;
	int8 speed;
	byte type;
	uint16 flags;

	BaseDoor() : mPos(-1), mPos1(-1), mCode(0), mCode1(0), offset(0),
		speed(0), type(0), flags(0) {}
};

struct BaseObject {
	bool active;
	bool passable;
	int16 direction;
	int16 x;
	int16 y;
	int16 mapPos;
	byte id;
	byte bitmap;
	byte animationTick;
	byte mode;
	int16 timer;
	int16 oldX;
	int16 oldY;

	BaseObject() : active(false), passable(false), direction(0), x(0), y(0),
		mapPos(0), id(0), bitmap(1), animationTick(0), mode(kBaseObjectChase),
		timer(0), oldX(0), oldY(0) {}
};

struct BaseRayHit {
	bool hit;
	BaseRayAxis axis;
	uint16 code;
	int mapPos;
	int textureColumn;
	int32 distance;
	int32 rawDistance;
	int32 worldX;
	int32 worldY;

	BaseRayHit() : hit(false), axis(kBaseRayNone), code(0), mapPos(-1),
		textureColumn(0), distance(kBaseMaximumDistance - 1),
		rawDistance(0x7fffffff), worldX(0), worldY(0) {}
};

struct BaseEntryPoint {
	int entryId;
	int16 playerX;
	int16 playerY;
	int16 playerAngle;
	int mapIndex;
	int returnId;
};

} // End of namespace Hopkins

#endif // HOPKINS_BASE_TYPES_H
