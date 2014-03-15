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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "illusions/illusions.h"
#include "illusions/fixedpoint.h"

namespace Illusions {

FP16 floatToFixed(float value) {
	return value * 65536.0;
}

float fixedToFloat(FP16 value) {
	return value / 65536.0;
}

FP16 fixedMul(FP16 a, FP16 b) {
	return ((float)a * b) / 65536.0;
}

FP16 fixedDiv(FP16 a, FP16 b) {
	return ((float)a / b) * 65536.0;
}

int fixedTrunc(FP16 value) {
	// CHECKME Not sure if this correct
	int result = value >> 16;
	if ((value & 0xFFFF) >= 0x8000)
		++result;
	return result;
}

FP16 fixedDistance(FP16 x1, FP16 y1, FP16 x2, FP16 y2) {
	float xd = ABS(fixedToFloat(x1) - fixedToFloat(x2));
	float yd = ABS(fixedToFloat(y1) - fixedToFloat(y2));
	if (xd != 0.0 || yd != 0.0)
		return floatToFixed(sqrt(xd * xd + yd * yd));
	return 0;
}

} // End of namespace Illusions
