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

FixedPoint16 floatToFixed(float value) {
	return value * 65536.0;
}

float fixedToFloat(FixedPoint16 value) {
	return value / 65536.0;
}

FixedPoint16 fixedMul(FixedPoint16 a, FixedPoint16 b) {
	return ((float)a * b) / 65536.0;
}

FixedPoint16 fixedDiv(FixedPoint16 a, FixedPoint16 b) {
	return ((float)a / b) * 65536.0;
}

int16 fixedTrunc(FixedPoint16 value) {
	// CHECKME Not sure if this correct
	int16 result = (value >> 16) & 0xFFFF;
	if ((value & 0xFFFF) >= 0x8000)
		++result;
	return result;
}

FixedPoint16 fixedDistance(FixedPoint16 x1, FixedPoint16 y1, FixedPoint16 x2, FixedPoint16 y2) {
	float xd = fixedToFloat(x1) - fixedToFloat(x2);
	float yd = fixedToFloat(y1) - fixedToFloat(y2);
	if (xd != 0.0f || yd != 0.0f)
		return floatToFixed(sqrt(xd * xd + yd * yd));
	return 0;
}

FixedPoint16 fixedAtan(FixedPoint16 value) {
	//return floatToFixed(atan2(1.0, fixedToFloat(value)));
	return floatToFixed(atan(fixedToFloat(value)));
}

FixedPoint16 fixedCos(FixedPoint16 value) {
	return floatToFixed(cos(fixedToFloat(value)));
}

FixedPoint16 fixedSin(FixedPoint16 value) {
	return floatToFixed(sin(fixedToFloat(value)));
}

} // End of namespace Illusions
