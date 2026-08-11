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

#ifndef FREESCAPE_SID_H
#define FREESCAPE_SID_H

#include "common/scummsys.h"

namespace Freescape {

// SID register offsets (matches C64 $D400-$D418)
enum SIDRegs {
	kSIDV1FreqLo   = 0x00,
	kSIDV1FreqHi   = 0x01,
	kSIDV1PwLo     = 0x02,
	kSIDV1PwHi     = 0x03,
	kSIDV1Ctrl     = 0x04,
	kSIDV1AD       = 0x05,
	kSIDV1SR       = 0x06,

	kSIDV2FreqLo   = 0x07,
	kSIDV2FreqHi   = 0x08,
	kSIDV2PwLo     = 0x09,
	kSIDV2PwHi     = 0x0A,
	kSIDV2Ctrl     = 0x0B,
	kSIDV2AD       = 0x0C,
	kSIDV2SR       = 0x0D,

	kSIDV3FreqLo   = 0x0E,
	kSIDV3FreqHi   = 0x0F,
	kSIDV3PwLo     = 0x10,
	kSIDV3PwHi     = 0x11,
	kSIDV3Ctrl     = 0x12,
	kSIDV3AD       = 0x13,
	kSIDV3SR       = 0x14,

	kSIDFilterLo   = 0x15,
	kSIDFilterHi   = 0x16,
	kSIDFilterCtrl = 0x17,
	kSIDVolume     = 0x18
};

// 40-byte SFX descriptor used by the Wally Beben C64 SFX engine
// (shared by Dark Side, Total Eclipse, and possibly other Freescape titles).
struct C64SFXData {
	uint8 numNotes;
	uint8 repeatCount;
	uint8 reserved;
	uint8 freqWaypoints[20];
	uint8 padding;
	uint8 durations[9];
	uint8 speed;
	uint8 padding2;
	uint8 pwLo;
	uint8 pwHi;
	uint8 waveform;
	uint8 attackDecay;
	uint8 sustainRelease;
};

} // namespace Freescape

#endif // FREESCAPE_SID_H
