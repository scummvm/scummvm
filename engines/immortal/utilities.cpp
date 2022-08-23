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

#include "immortal/utilities.h"

namespace Immortal {

namespace Utilities {

void Immortal::Utilities::delay(int j) {             // Delay is measured in jiffies, which are 56.17ms
	g_system->delayMillis(j * 56);
}

void Immortal::Utilities::delay4(int j) {            // Named in source quarterClock for some reason, 1/4 jiffies are 14.04ms
	g_system->delayMillis(j * 14);
}

void Immortal::Utilities::delay8(int j) {            // 1/8 jiffies are 7.02ms
	g_system->delayMillis(j * 7);
}

bool Immortal::Utilities::inside(int x1, int y1, int a, int x2, int y2) {
	return false;
}
bool Immortal::Utilities::insideRect(int x, int y, int r) {
	return false;
}

void Immortal::Utilities::addSprite(Sprite *sprites, uint16 vpX, uint16 vpY, int num, DataSprite *d, int img, uint16 x, uint16 y, uint16 p) {
	if (num != kMaxSprites) {
		if (x >= (kResH + kMaxSpriteLeft)) {
			x |= kMaskHigh;                         // Make it negative
		}
		
		sprites[num]._X = (x << 1) + vpX;
	
		if (y >= (kMaxSpriteAbove + kResV)) {
			y |= kMaskHigh;
		}
		
		sprites[num]._Y = (y << 1) + vpY;

		if (p >= 0x80) {
			p |= kMaskHigh;
		}

		sprites[num]._priority = ((p + y) ^ 0xFFFF) + 1;
		
		sprites[num]._image = img;
		sprites[num]._dSprite = d;
		sprites[num]._on = 1;
		num += 1;

	} else {
		debug("Max sprites reached beeeeeep!!");
	}
}

}; // namespace Utilities

}; // namespace Immortal