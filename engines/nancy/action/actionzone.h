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

// The zone's subtype, stored in the low byte of its leading int32. It selects both
// what the zone does at runtime and how much trailing data follows the shared base.
// Subtypes whose meaning has not been worked out yet are named after their value;
// those still read (or skip) the right number of bytes.
enum ActionZoneType : byte {
	kZoneSpecialEffect	= 0x00,	// carries a scene id plus an optional fade
	kZoneBaseOnly		= 0x01,	// base fields only, no behavior of its own
	kZoneTeleport		= 0x02,	// pipe: swallows the ball and re-emits it elsewhere
	kZoneTerrain		= 0x03,	// sand trap / mud puddle: slows whatever is inside
	kZoneSlope			= 0x04,	// hill: a velocity kick while crossing the zone
	kZoneUnknown05		= 0x05,	// base fields only
	kZoneEventFlag		= 0x0b,	// checkpoint / sewing seam: sets an event flag
	kZoneSceneChange	= 0x0c,	// finish line / golf cup: fade + target scene + flag
	kZoneOverlay		= 0x0d,	// cosmetic overlay sprite
	kZoneUnknown0E		= 0x0e,
	kZoneUnknown0F		= 0x0f,
	kZoneUnknown10		= 0x10,	// base fields only
	kZoneDestination	= 0x11,	// driving: a location entrance the car can park in
	kZoneUnknown12		= 0x12,	// another special-effect variant
	kZoneUnknown13		= 0x13,	// another special-effect variant
	kZoneBoundary		= 0x14,	// play-area wall
	kZoneUnknown15		= 0x15,	// special effect + int32; a damage range in Nancy13
	kZoneBumper			= 0x16,	// Nancy12: an overlay variant; Nancy13: a pachinko hole
	kZoneFlatTire		= 0x17	// pothole: damages the car driving over it
};

// A polymorphic "ActionZone" record, embedded as a count-prefixed array inside
// every Nancy12 PuzzleBase puzzle (Driving/Minigolf/MirrorLight/BoardGame/Mind/
// Chase). Each subtype reads a different amount of trailing data; readData()
// consumes exactly the right number of bytes so the surrounding chunk stays in sync.
struct ActionZone {
	ActionZoneType type = kZoneSpecialEffect;
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
	// special-effect subtypes when the effect byte is not the terminator).
	uint16 specialEffectId = 0;
	bool hasSpecialEffect = false;
	byte seType = 0;				// 1 = blackout, 2 = cross-dissolve, 3 = through-black
	uint16 seTotalTime = 0;
	uint16 seFadeToBlackTime = 0;
	Common::Rect seRect;

	// int16 + byte trailer carried by kZoneEventFlag and kZoneSceneChange. For the
	// former it is an event-flag id + on/off; for the latter a target scene id + a
	// flag. Left at their defaults for the other subtypes.
	int16 tailId = 0;
	byte tailFlag = 0;

	// kZoneTeleport: the ball entering this zone is held for teleportDelay ms, then
	// re-emerges at exitRect travelling at exitSpeed along exitAngle (degrees).
	Common::Rect exitRect;
	int32 teleportDelay = 0;
	int16 exitAngle = 0;
	int16 exitSpeed = 0;

	// kZoneTerrain: extra deceleration added to the moving object's friction while
	// it is inside this zone.
	double terrainDecel = 0.0;

	// kZoneFlatTire: entering the zone adds a random amount of damage in
	// [flatTireMin, flatTireMax].
	int32 flatTireMin = 0;
	int32 flatTireMax = 0;

	// kZoneSlope: a velocity kick of slopeForce along slopeAngle (degrees), applied
	// on entering the zone and removed on leaving.
	double slopeForce = 0.0;
	int16 slopeAngle = 0;

	// kZoneOverlay (and kZoneBumper in Nancy12)
	Common::String overlayName;
	Common::Array<Common::Rect> overlaySrcRects;
	Common::Rect overlayDestRect;
	int32 overlayLayer = 0;	// draw pass: 0 renders under the car, 1 over it

	// The Nancy13 pinball layout (AR 175) differs from the Nancy12 one: the base carries an
	// extra int32 before the sound block, and the overlay/unknown-0x15/bumper subtypes have
	// different trailers. Pass isNancy13 = true to parse it; the default keeps Nancy12.
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
