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

#ifndef NANCY_ACTION_ACTIONZONE_H
#define NANCY_ACTION_ACTIONZONE_H

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

#include "engines/nancy/commontypes.h"

namespace Common {
class SeekableReadStream;
}

namespace Nancy {
namespace Action {

// A polymorphic "ActionZone" record, embedded as a count-prefixed array inside
// every Nancy12 PuzzleBase puzzle (Driving/Minigolf/MirrorLight/BoardGame/Mind/
// Chase). The zone's type is the low byte of its leading int32; each subtype
// reads a different amount of trailing data. readData() consumes exactly the
// right number of bytes for the type so the surrounding chunk stays in sync.
struct ActionZone {
	byte type = 0;
	int32 typeField = 0;	// full leading int32; low byte == type

	Common::Rect rect;
	Common::String ovlName;	// OVL sprite name ("" / sentinel == none)

	int32 pointA = 0;		// 8 bytes (a point) at base+0x39
	int32 pointB = 0;
	int32 valC = 0;			// 8 bytes at base+0x41
	int32 valD = 0;
	int16 val49 = 0;
	byte val4b = 0;

	RandomSoundBlock _sound;

	// Special Effect (an embedded 21-byte SpecialEffect record, present on the
	// special-effect subtypes when the effect byte is not the 0xff terminator).
	uint16 specialEffectId = 0;
	bool hasSpecialEffect = false;
	byte seType = 0;				// 1 = blackout, 2 = cross-dissolve, 3 = through-black
	uint16 seTotalTime = 0;
	uint16 seFadeToBlackTime = 0;
	Common::Rect seRect;

	// int16 + byte trailer carried by the collision (0x0b) and trigger (0x0c)
	// subtypes. For 0x0b it is an event-flag id + on/off; for 0x0c it is a target
	// scene id + a flag. Left at their defaults for the other subtypes.
	int16 tailId = 0;
	byte tailFlag = 0;

	// OverlayZone subtypes (0x0d / 0x16)
	Common::String overlayName;
	Common::Array<Common::Rect> overlaySrcRects;
	Common::Rect overlayDestRect;

	// The Nancy13 pinball layout (AR 175) differs from the Nancy12 one: the base carries an
	// extra int32 before the sound block, and subtypes 0x0d/0x15/0x16 have different trailers.
	// Pass isNancy13 = true to parse it; the default keeps the Nancy12 behaviour.
	void readData(Common::SeekableReadStream &stream, bool isNancy13 = false);

private:
	void readSpecialEffect(Common::SeekableReadStream &stream);
	void readOverlayZone(Common::SeekableReadStream &stream, bool isNancy13);
	void readSubtype(Common::SeekableReadStream &stream, bool isNancy13);
};

// Reads an int16 count, then that many ActionZones.
void readActionZoneArray(Common::SeekableReadStream &stream, Common::Array<ActionZone> &out, bool isNancy13 = false);

} // End of namespace Action
} // End of namespace Nancy

#endif // NANCY_ACTION_ACTIONZONE_H
