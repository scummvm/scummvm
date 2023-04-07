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

#include "common/system.h"

namespace Immortal {

// The source uses nK many times throughout, which seems to be a compiler macro for n * 1024, ie. Kb
enum Kilobyte {
	k1K  = 0x400,			// 1024
	k2K  = 0x800,			// 2048
	k3K  = 0xC00, 			// 3072
	k4K  = 0x1000,			// 4096
	k6K  = 0x1800,			// 6144
	k8K  = 0x2000,			// 8192
	k10K = 0x2800,			// 10240
	k16K = 0x4000			// 16384
};

enum BitMask16 : uint16 {
	kMaskLow   = 0x00FF,
	kMaskHigh  = 0xFF00,
	kMaskLast  = 0xF000,
	kMaskFirst = 0x000F,
	kMaskHLow  = 0x0F00,
	kMaskLHigh = 0x00F0,
	kMaskNeg   = 0x8000,
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

// Other
void delay(int j);                                  // Delay engine by j jiffies (from driver originally, but makes more sense grouped with misc)
void delay4(int j);                                 // || /4
void delay8(int j);                                 // || /8
bool inside(uint8 dist, uint8 centX, uint8 centY, uint8 pointX, uint8 pointY);
bool insideRect(uint8 rectX, uint8 rectY, uint8 w, uint8 h, uint8 pointX, uint8 pointY);

} // namespace Utilities

} // namespace Immortal

#endif
