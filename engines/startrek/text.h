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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef STARTREK_TEXT_H
#define STARTREK_TEXT_H

#include "common/language.h"

namespace StarTrek {

// The type to use for text references (values of "GameStringIDs" enum).
// NOTE: if this typedef is changed, certain lines in "saveload.cpp" would also need to be
// changed. Better to leave this as-is.
typedef int32 TextRef;

// Text that's loaded from "GROUND.TXT". First 0x40 pieces of text are for items.
// TODO: Floppy version has different numbers for this.
enum GroundTextIDs {
	// Generic "perform undefined action" text (ie. look at nothing, talk to wall)
	GROUNDTX_LOOK_KIRK = 0x49,
	GROUNDTX_LOOK_SPOCK = 0x4a,
	GROUNDTX_LOOK_MCCOY = 0x4b,
	GROUNDTX_LOOK_REDSHIRT = 0x4c,
	GROUNDTX_LOOK_ANYWHERE = 0x4d,
	GROUNDTX_TALK_TO_CREWMAN = 0x4e,
	GROUNDTX_NO_RESPONSE = 0x4f,

	GROUNDTX_KIRK_USE = 0x50,
	GROUNDTX_SPOCK_USE = 0x51,
	GROUNDTX_MCCOY_USE = 0x52,
	GROUNDTX_REDSHIRT_USE = 0x53,
	GROUNDTX_SPOCK_SCAN = 0x54,
	GROUNDTX_MCCOY_SCAN = 0x55,
	GROUNDTX_USE_MEDKIT = 0x56,

	GROUNDTX_PHASER_ON_MCCOY = 0x57, // 8 variations
	GROUNDTX_PHASER_ON_SPOCK = 0x5f, // 8 variations
	GROUNDTX_PHASER_ON_REDSHIRT = 0x67, // 8 variations
	GROUNDTX_PHASER_ANYWHERE = 0x6f, // 7 variations

	GROUNDTX_USE_COMMUNICATOR = 0x76,
	GROUNDTX_NOTHING_HAPPENS = 0x77,
	GROUNDTX_FAIL_TO_OBTAIN_ANYTHING = 0x78
};

enum GlobalGameStringIDs {
	TX_END = -2,
	TX_EMPTY = -1
};

Common::String patchRoomMessage(const char *text);

struct RoomTextOffsets {
	int16 id;
	uint16 offsetEnglishCD;
	uint16 offsetEnglishFloppy;
	uint16 offsetFrenchCD;
	uint16 offsetGermanCD;
};

struct RoomText {
	int16 id;
	Common::Language language;
	const char *text;
};

// defined in text.cpp
extern const char *const g_gameStrings[];

} // End of namespace StarTrek

#endif
