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

#ifndef IMMORTAL_BITMASK_H
#define IMMORTAL_BITMASK_H

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

} // namespace immortal

#endif