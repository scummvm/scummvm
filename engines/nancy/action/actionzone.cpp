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

#include "common/stream.h"
#include "common/util.h"

#include "engines/nancy/util.h"
#include "engines/nancy/action/actionzone.h"

namespace Nancy {
namespace Action {

// Terminator in place of a special effect's type byte, meaning "no effect follows"
static const byte kNoSpecialEffect = 0xff;

void ActionZone::readData(Common::SeekableReadStream &stream, bool isNancy13) {
	// Base ActionZone fields, shared by every subtype.
	typeField = stream.readSint32LE();
	type = (ActionZoneType)(typeField & 0xFF);

	readRect(stream, rect);
	readFilename(stream, ovlName);

	pointA = stream.readSint32LE();
	pointB = stream.readSint32LE();
	valC = stream.readSint32LE();
	valD = stream.readSint32LE();
	val49 = stream.readSint16LE();
	val4b = stream.readByte();

	// The Nancy13 base carries an extra int32 here (before the sound block).
	if (isNancy13) {
		stream.skip(4);
	}

	// Random-sound block: count, then that many 33-byte names + 8 bytes of params.
	_sound.readData(stream);

	readSubtype(stream, isNancy13);
}

// Subtype-specific trailing data. Fields not yet needed are skipped to keep the stream
// aligned. The Nancy12 and Nancy13 layouts are identical apart from three subtypes
// (overlay, unknown 0x15 and bumper), which branch on isNancy13.
void ActionZone::readSubtype(Common::SeekableReadStream &stream, bool isNancy13) {
	switch (type) {
	case kZoneSpecialEffect:
	case kZoneDestination:
	case kZoneUnknown12:
	case kZoneUnknown13:
		readSpecialEffect(stream);
		break;
	case kZoneBaseOnly:
	case kZoneUnknown05:
	case kZoneUnknown10:
	case kZoneBoundary:
		// No trailing data.
		break;
	case kZoneTeleport:
		readRect(stream, exitRect);
		teleportDelay = stream.readSint32LE();
		exitAngle = stream.readSint16LE();
		exitSpeed = stream.readSint16LE();
		break;
	case kZoneTerrain:
		terrainDecel = stream.readDoubleLE();
		break;
	case kZoneSlope:
		slopeForce = stream.readDoubleLE();
		slopeAngle = stream.readSint16LE();
		break;
	case kZoneEventFlag:
		tailId = stream.readSint16LE();
		tailFlag = stream.readByte();
		break;
	case kZoneSceneChange:
		readSpecialEffect(stream);
		tailId = stream.readSint16LE();
		tailFlag = stream.readByte();
		break;
	case kZoneOverlay:
		readOverlayZone(stream, isNancy13);
		break;
	case kZoneUnknown0E:
		stream.skip(2);		// int16
		break;
	case kZoneUnknown0F:
		stream.skip(4);		// int16 + int16
		break;
	case kZoneUnknown15:
		// Nancy12: special effect + a trailing int32. Nancy13: a damage range
		// (min/max int32), with no special effect.
		if (isNancy13) {
			stream.skip(8);
		} else {
			readSpecialEffect(stream);
			stream.skip(4);
		}
		break;
	case kZoneBumper:
		// Nancy12: an OverlayZone + int32. Nancy13: a short record (two bytes + int16).
		if (isNancy13) {
			stream.skip(4);
		} else {
			readOverlayZone(stream, false);
			stream.skip(4);
		}
		break;
	case kZoneFlatTire:		// Nancy12 only
		flatTireMin = stream.readSint32LE();
		flatTireMax = stream.readSint32LE();
		if (flatTireMax < flatTireMin) {
			SWAP(flatTireMin, flatTireMax);
		}
		break;
	default:
		warning("ActionZone: unknown type %d - chunk may desync", type);
		break;
	}
}

// Special Effect block: an int16 id (a target scene on transition zones), then the
// effect type byte. If the type is the terminator the effect is absent; otherwise a
// 21-byte SpecialEffect record follows (type + totalTime + fadeToBlackTime + Rect),
// matching the standalone SpecialEffect action record.
void ActionZone::readSpecialEffect(Common::SeekableReadStream &stream) {
	specialEffectId = stream.readUint16LE();
	seType = stream.readByte();
	if (seType == kNoSpecialEffect) {
		seType = 0;
		return;
	}

	hasSpecialEffect = true;
	seTotalTime = stream.readUint16LE();
	seFadeToBlackTime = stream.readUint16LE();
	readRect(stream, seRect);
}

void ActionZone::readOverlayZone(Common::SeekableReadStream &stream, bool isNancy13) {
	readFilename(stream, overlayName);

	int16 numSrcRects = stream.readSint16LE();
	if (numSrcRects > 0) {
		overlaySrcRects.resize(numSrcRects);
		for (int i = 0; i < numSrcRects; ++i) {
			readRect(stream, overlaySrcRects[i]);
		}
	}

	readRect(stream, overlayDestRect);
	if (isNancy13) {
		stream.skip(4);	// extra int32 vs Nancy12
	}
	stream.skip(4);		// int32
	stream.skip(1);		// byte (loop/play mode)
	overlayLayer = stream.readSint32LE();
}

void readActionZoneArray(Common::SeekableReadStream &stream, Common::Array<ActionZone> &out, bool isNancy13) {
	int16 count = stream.readSint16LE();
	if (count <= 0) {
		return;
	}

	out.resize(count);
	for (int i = 0; i < count; ++i) {
		out[i].readData(stream, isNancy13);
	}
}

} // End of namespace Action
} // End of namespace Nancy
