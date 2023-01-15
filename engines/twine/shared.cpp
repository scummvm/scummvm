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

#include "twine/shared.h"

namespace TwinE {

int32 LBAAngles::ANGLE_360;
int32 LBAAngles::ANGLE_351;
int32 LBAAngles::ANGLE_334;
int32 LBAAngles::ANGLE_315;
int32 LBAAngles::ANGLE_270;
int32 LBAAngles::ANGLE_225;
int32 LBAAngles::ANGLE_210;
int32 LBAAngles::ANGLE_180;
int32 LBAAngles::ANGLE_157_5;
int32 LBAAngles::ANGLE_140;
int32 LBAAngles::ANGLE_135;
int32 LBAAngles::ANGLE_90;
int32 LBAAngles::ANGLE_70;
int32 LBAAngles::ANGLE_63;
int32 LBAAngles::ANGLE_45;
int32 LBAAngles::ANGLE_22_5;
int32 LBAAngles::ANGLE_17;
int32 LBAAngles::ANGLE_11_25;
int32 LBAAngles::ANGLE_2;
int32 LBAAngles::ANGLE_1;
int32 LBAAngles::ANGLE_0;

void LBAAngles::init(int factor) {
	ANGLE_360 = 1024 * factor;
	ANGLE_351 = 1000 * factor;
	ANGLE_334 = 950 * factor;
	ANGLE_315 = 896 * factor;
	ANGLE_270 = 768 * factor;
	ANGLE_225 = 640 * factor;
	ANGLE_210 = 600 * factor;
	ANGLE_180 = 512 * factor;
	ANGLE_157_5 = 448 * factor;
	ANGLE_140 = 400 * factor;
	ANGLE_135 = 384 * factor;
	ANGLE_90 = 256 * factor;
	ANGLE_70 = 200 * factor;
	ANGLE_63 = 180 * factor;
	ANGLE_45 = 128 * factor;
	ANGLE_22_5 = 64 * factor;
	ANGLE_17 = 50 * factor;
	ANGLE_11_25 = 32 * factor;
	ANGLE_2 = 8 * factor;
	ANGLE_1 = 5 * factor;
	ANGLE_0 = 0 * factor;
}

void LBAAngles::lba1() {
	init(1);
}

void LBAAngles::lba2() {
	init(4);
}


int32 getDistance2D(int32 x1, int32 z1, int32 x2, int32 z2) {
	return (int32)sqrt((float)((x2 - x1) * (x2 - x1) + (z2 - z1) * (z2 - z1)));
}

int32 getDistance2D(const IVec3 &v1, const IVec3 &v2) {
	return (int32)sqrt((float)((v2.x - v1.x) * (v2.x - v1.x) + (v2.z - v1.z) * (v2.z - v1.z)));
}

int32 getDistance3D(int32 x1, int32 y1, int32 z1, int32 x2, int32 y2, int32 z2) {
	return (int32)sqrt((float)((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1) + (z2 - z1) * (z2 - z1)));
}

int32 getDistance3D(const IVec3 &v1, const IVec3 &v2) {
	return (int32)sqrt((float)((v2.x - v1.x) * (v2.x - v1.x) + (v2.y - v1.y) * (v2.y - v1.y) + (v2.z - v1.z) * (v2.z - v1.z)));
}

}
