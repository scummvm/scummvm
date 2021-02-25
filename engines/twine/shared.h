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

#ifndef TWINE_SHARED_H
#define TWINE_SHARED_H

#include "common/scummsys.h"

#define NUM_GAME_FLAGS 255
#define NUM_INVENTORY_ITEMS 28

namespace TwinE {

struct Vec3 {
	Vec3() : x(0), y(0), z(0) {}
	Vec3(int32 _x, int32 _y, int32 _z) : x(_x), y(_y), z(_z) {}
	int32 x;
	int32 y;
	int32 z;

	inline Vec3& operator+=(const Vec3 &other) {
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	inline Vec3& operator-=(const Vec3 &other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}
};

inline Vec3 operator+(const Vec3 &lhs, const Vec3 &rhs) {
	return Vec3{lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z};
}

inline Vec3 operator-(const Vec3 &lhs, const Vec3 &rhs) {
	return Vec3{lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z};
}

struct BoundingBox {
	Vec3 mins;
	Vec3 maxs;
};

struct ActorBoundingBox {
	/** Bottom left X coordinate */
	int16 bottomLeftX = 0;
	/** Bottom left Y coordinate */
	int16 bottomLeftY = 0;
	/** Bottom left Z coordinate */
	int16 bottomLeftZ = 0;
	/** Top left X coordinate */
	int16 topRightX = 0;
	/** Top left Y coordinate */
	int16 topRightY = 0;
	/** Top left Z coordinate */
	int16 topRightZ = 0;
	bool hasBoundingBox = false;
};

enum ActionType {
	ACTION_NOP = 0,
	ACTION_BODY = 1,
	ACTION_BODP = 2,
	ACTION_ANIM = 3,
	ACTION_ANIP = 4,
	ACTION_HITTING = 5,
	ACTION_SAMPLE = 6,
	ACTION_SAMPLE_FREQ = 7,
	ACTION_THROW_EXTRA_BONUS = 8,
	ACTION_THROW_MAGIC_BALL = 9,
	ACTION_SAMPLE_REPEAT = 10,
	ACTION_THROW_SEARCH = 11,
	ACTION_THROW_ALPHA = 12,
	ACTION_SAMPLE_STOP = 13,
	ACTION_ZV = 14,
	ACTION_LEFT_STEP = 15,
	ACTION_RIGHT_STEP = 16,
	ACTION_HERO_HITTING = 17,
	ACTION_THROW_3D = 18,
	ACTION_THROW_3D_ALPHA = 19,
	ACTION_THROW_3D_SEARCH = 20,
	ACTION_UNKNOWN_21 = 21,
	ACTION_LAST
};

enum class ShapeType {
	kNone = 0,
	kSolid = 1,
	kStairsTopLeft = 2,
	kStairsTopRight = 3,
	kStairsBottomLeft = 4,
	kStairsBottomRight = 5,
	kDoubleSideStairsTop1 = 6,
	kDoubleSideStairsBottom1 = 7,
	kDoubleSideStairsLeft1 = 8,
	kDoubleSideStairsRight1 = 9,
	kDoubleSideStairsTop2 = 10,
	kDoubleSideStairsBottom2 = 11,
	kDoubleSideStairsLeft2 = 12,
	kDoubleSideStairsRight2 = 13,
	kFlatBottom1 = 14,
	kFlatBottom2 = 15
};

/** Control mode types */
enum class ControlMode {
	kNoMove = 0,
	kManual = 1,
	kFollow = 2,
	kTrack = 3,
	kFollow2 = 4,
	kTrackAttack = 5,
	kSameXZ = 6,
	kRandom = 7
};

enum class AnimationTypes {
	kAnimNone = -1,
	kStanding = 0,
	kForward = 1,
	kBackward = 2,
	kTurnLeft = 3,
	kTurnRight = 4,
	kHit = 5,
	kBigHit = 6,
	kFall = 7,
	kLanding = 8,
	kLandingHit = 9,
	kLandDeath = 10,
	kAction = 11,
	kClimbLadder = 12,
	kTopLadder = 13,
	kJump = 14,
	kThrowBall = 15,
	kHide = 16,
	kKick = 17,
	kRightPunch = 18,
	kLeftPunch = 19,
	kFoundItem = 20,
	kDrawn = 21,
	kHit2 = 22,
	kSabreAttack = 23,
	kSabreUnknown = 24,
	kCarStarting = 303,
	kCarDriving = 304,
	kCarDrivingBackwards = 305,
	kCarStopping = 306,
	kCarFrozen = 307,
	kAnimInvalid = 255
};

/** Hero behaviour
 * <li> NORMAL: Talk / Read / Search / Use
 * <li> ATHLETIC: Jump
 * <li> AGGRESSIVE:
 * Auto mode   : Fight
 * Manual mode : While holding the spacebar down
 * 			UP / RIGHT / LEFT will manually select
 * 			different punch/kick options
 * <li> DISCREET: Kneel down to hide
 *
 * @note The values must match the @c TextId indices
 */
enum class HeroBehaviourType {
	kNormal = 0,
	kAthletic = 1,
	kAggressive = 2,
	kDiscrete = 3,
	kProtoPack = 4
};

/**
 * 0: tunic + medallion
 * 1: tunic
 * 2: tunic + medallion + sword
 * 3: prison suit
 * 4: nurse outfit
 * 5: tunic + medallion + horn
 * 6: snowboard (WARNING, this can crash the game when you change behavior)
 */
enum class BodyType {
	btNone = -1,
	btNormal = 0,
	btTunic = 1,
	btSabre = 2,
	btPrisonSuit = 3,
	btNurseSuit = 4,
	btHorn = 5,
	btSnowboard = 6
};

enum class ExtraSpecialType {
	kHitStars = 0,
	kExplodeCloud = 1
};

// lba2 does from 0 to 0x1000
// lba1 angles
// TODO: wrap in a class to be able to handle lba1 and lba2
#define ANGLE_360 1024
#define ANGLE_351 1000
#define ANGLE_334 950
#define ANGLE_315 896
#define ANGLE_270 768
#define ANGLE_225 640
#define ANGLE_210 600
#define ANGLE_180 512
#define ANGLE_140 400
#define ANGLE_135 384
#define ANGLE_90 256
#define ANGLE_70 200
#define ANGLE_63 180
#define ANGLE_45 128
#define ANGLE_22_5 64
#define ANGLE_17 50
#define ANGLE_11_25 32
#define ANGLE_2 8 // 1.67
#define ANGLE_1 5 // 1.75
#define ANGLE_0 0

inline int32 NormalizeAngle(int32 angle) {
	if (angle < -ANGLE_180) {
		angle += ANGLE_360;
	} else if (angle > ANGLE_180) {
		angle -= ANGLE_360;
	}
	return angle;
}

/**
 * @param[in] angle The angle as input from game data
 * @return The value as it is used at runtime
 */
inline constexpr int32 ToAngle(int32 angle) {
	return angle;
}

/**
 * @param[in] angle The angle as used at runtime
 * @return The value as it should be used for storing in game data
 */
inline constexpr int32 FromAngle(int32 angle) {
	return angle;
}

inline constexpr double AngleToRadians(int32 angle) {
	return 2.0 * M_PI * angle / (double)ANGLE_360;
}

inline constexpr int32 ClampAngle(int32 angle) {
	return angle & (ANGLE_360 - 1);
}

template<typename T>
inline constexpr T bits(T value, uint8 offset, uint8 bits) {
	return (((1 << bits) - 1) & (value >> offset));
}

}

#endif
