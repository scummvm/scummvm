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

#include "startrek/startrek.h"

namespace StarTrek {

Fixed14 StarTrekEngine::sin(Angle angle) {
	int16 i = angle.raw();
	if (angle < 0) {
		i %= 0x400;
		i += 0x400;
	}
	i &= 0x3ff;

	if (i == 0x100)
		return 1.0;
	else if (i == 0x300)
		return -1.0;

	float f = _sineTable.at(i);
	return Fixed14(f);
}

Fixed14 StarTrekEngine::cos(Angle angle) {
	return sin(angle + 1.0);
}

Angle StarTrekEngine::atan2(int32 deltaX, int32 deltaY) {
	static const int16 atanTable[] = {
		0x0000, 0x0064, 0x00c9, 0x012d, 0x0192, 0x01f6, 0x025b, 0x02c0,
		0x0324, 0x0389, 0x03ee, 0x0453, 0x04b8, 0x051d, 0x0582, 0x05e8,
		0x064d, 0x06b3, 0x0718, 0x077e, 0x07e4, 0x084a, 0x08b1, 0x0917,
		0x097e, 0x09e5, 0x0a4c, 0x0ab3, 0x0b1a, 0x0b82, 0x0bea, 0x0c52,
		0x0cba, 0x0d23, 0x0d8c, 0x0df5, 0x0e5f, 0x0ec8, 0x0f32, 0x0f9d,
		0x1007, 0x1072, 0x10de, 0x114a, 0x11b6, 0x1222, 0x128f, 0x12fc,
		0x136a, 0x13d8, 0x1446, 0x14b5, 0x1524, 0x1594, 0x1604, 0x1675,
		0x16e6, 0x1757, 0x17ca, 0x183c, 0x18b0, 0x1923, 0x1998, 0x1a0c,
		0x1a82, 0x1af8, 0x1b6f, 0x1be6, 0x1c5e, 0x1cd7, 0x1d50, 0x1dca,
		0x1e45, 0x1ec0, 0x1f3c, 0x1fb9, 0x2037, 0x20b5, 0x2134, 0x21b4,
		0x2235, 0x22b7, 0x2339, 0x23bd, 0x2441, 0x24c6, 0x254c, 0x25d4,
		0x265c, 0x26e5, 0x276f, 0x27fa, 0x2887, 0x2914, 0x29a2, 0x2a32,
		0x2ac3, 0x2b55, 0x2be8, 0x2c7d, 0x2d12, 0x2da9, 0x2e42, 0x2edc,
		0x2f77, 0x3013, 0x30b1, 0x3151, 0x31f2, 0x3294, 0x3338, 0x33de,
		0x3486, 0x352f, 0x35d9, 0x3686, 0x3734, 0x37e5, 0x3897, 0x394b,
		0x3a01, 0x3ab9, 0x3b73, 0x3c30, 0x3cee, 0x3daf, 0x3e72, 0x3f38,
		0x4000
	};

	if (deltaX == 0 && deltaY == 0)
		return 0.0;

	bool deltaYNegative = deltaY <= 0;
	bool deltaXNegative = deltaX <= 0;

	deltaY = abs(deltaY);
	deltaX = abs(deltaX);

	Fixed14 ratio;
	if (deltaY > deltaX)
		ratio = Fixed14::fromRaw(((deltaX & 0xffff0000) >> 2) / deltaY);
	else
		ratio = Fixed14::fromRaw(((deltaY & 0xffff0000) >> 2) / deltaX);

	int16 endIndex = 128;
	int16 index = 0;
	int16 angle = 0;
	while (index <= endIndex) {
		angle = (index + endIndex) / 2;
		Fixed14 tableValue = Fixed14::fromRaw(atanTable[angle]);
		if (tableValue > ratio)
			endIndex = angle - 1;
		else if (tableValue < ratio)
			index = angle + 1;
		else
			break;
	}

	if (deltaY > deltaX) {
		if (deltaYNegative && deltaXNegative)
			angle = angle - 512;
		else if (deltaYNegative && !deltaXNegative)
			angle = 512 - angle;
		else if (!deltaYNegative && deltaXNegative)
			angle = -angle;
		// else if (!deltaYNegative && !deltaXNegative)
		//	angle remains the same i.e. angle = angle
	} else {
		if (deltaYNegative && deltaXNegative)
			angle = -256 - angle;
		else if (deltaYNegative && !deltaXNegative)
			angle = 256 + angle;
		else if (!deltaYNegative && deltaXNegative)
			angle = -256 + angle;
		else // if (!deltaYNegative && !deltaXNegative)
			angle = 256 - angle;
	}

	return Angle::fromRaw(angle);
}

} // End of namespace StarTrek
