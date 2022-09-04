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

#ifndef IMMORTAL_UTIL_H
#define IMMORTAL_UTIL_H

#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/system.h"
#include "immortal/sprite_list.h"

namespace Immortal {

enum BitMask16 : uint16 {
	kMaskLow   = 0x00FF,
	kMaskHigh  = 0xFF00,
	kMaskLast  = 0xF000,
	kMaskFirst = 0x000F,
	kMaskHLow  = 0x0F00,
	kMaskLHigh = 0x00F0,
	kMaskNeg   = 0x8000,
	kMask12Bit = 0x0F9F                                 // Compression code (pos, 00, len) is stored in lower 12 bits of word
};

enum BitMask8 : uint8 {
	kMaskASCII = 0x7F,                                  // The non-extended ASCII table uses 7 bits, this makes a couple of things easier
	kMask8High = 0xF0,
	kMask8Low  = 0x0F
};

enum ColourBitMask : uint16 {
	kMaskRed   = 0x0F00,
	kMaskGreen = 0x00F0,
	kMaskBlue  = 0x000F
};

enum ChrMask : uint16 {
	kChr0  = 0x0000,
	kChrL  = 0x0001,
	kChrR  = 0xFFFF,
	kChrLD = 0x0002,
	kChrRD = 0xFFFE
};

enum Screen {                                           // These are constants that are used for defining screen related arrays
	kResH           = 320,
	kResV           = 200,
	kMaxSprites     = 32,                               // Number of sprites allowed at once
	kViewPortCW     = 256 / 64,
	kViewPortCH     = 128 / kMaxSprites,
	kMaxDrawItems   = kViewPortCH + 1 + kMaxSprites,
	kMaxSpriteAbove = 48,                               // Maximum sprite extents from center
	kMaxSpriteBelow = 16,
	kMaxSpriteLeft  = 16,
	kMaxSpriteRight = 16
};

namespace Utilities {

void delay(int j);                                  // Delay engine by j jiffies (from driver originally, but makes more sense grouped with misc)
void delay4(int j);                                 // || /4
void delay8(int j);                                 // || /8
bool inside(int x1, int y1, int a, int x2, int y2);
bool insideRect(int x, int y, int r);
void addSprite(Sprite *sprites, uint16 vpX, uint16 vpY, int *num, DataSprite *d, int img, uint16 x, uint16 y, uint16 p);

}; // namespace Util

}; // namespace Immortal

#endif