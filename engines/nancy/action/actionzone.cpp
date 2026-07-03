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

#include "engines/nancy/util.h"
#include "engines/nancy/action/actionzone.h"

namespace Nancy {
namespace Action {

void ActionZone::readData(Common::SeekableReadStream &stream) {
	// Base ActionZone (matches the original "Action Zone Boundary OVL" reader).
	typeField = stream.readSint32LE();
	type = typeField & 0xFF;

	readRect(stream, rect);
	readFilename(stream, ovlName);

	pointA = stream.readSint32LE();
	pointB = stream.readSint32LE();
	valC = stream.readSint32LE();
	valD = stream.readSint32LE();
	val49 = stream.readSint16LE();
	val4b = stream.readByte();

	// Random-sound block: count, then that many 33-byte names + 8 bytes of params.
	_sound.readData(stream);

	// Subtype-specific trailing data. Fields not yet needed are skipped to keep
	// the stream aligned.
	switch (type) {
	case 1:		// base only
	case 5:
	case 0x10:
	case 0x14:	// Boundary
		break;
	case 0:		// Special Effect (and movement variants) - peeked terminator
	case 0x11:
	case 0x12:
	case 0x13:
		readSpecialEffect(stream);
		break;
	case 0x0b:				// collision zone: event-flag id + on/off
		tailId = stream.readSint16LE();
		tailFlag = stream.readByte();
		break;
	case 0x0e:
		stream.skip(2);		// int16
		break;
	case 0x0f:
		stream.skip(4);		// int16 + int16
		break;
	case 3:
		stream.skip(8);		// double
		break;
	case 0x17:				// Flat Tire (min/max)
		stream.skip(8);		// int32 + int32
		break;
	case 4:
		stream.skip(10);	// double + int16
		break;
	case 2:
		stream.skip(24);	// Rect + int32 + int16 + int16
		break;
	case 0x0c:				// trigger zone: special effect + target scene id + flag
		readSpecialEffect(stream);
		tailId = stream.readSint16LE();
		tailFlag = stream.readByte();
		break;
	case 0x15:
		readSpecialEffect(stream);
		stream.skip(4);		// int32
		break;
	case 0x0d:				// OverlayZone
		readOverlayZone(stream);
		break;
	case 0x16:				// OverlayZone + int32
		readOverlayZone(stream);
		stream.skip(4);
		break;
	default:
		warning("Nancy12 ActionZone: unknown type %d - chunk may desync", type);
		break;
	}
}

// Special Effect block: an int16 id, then a byte. If the byte is the 0xff
// terminator the effect is absent; otherwise it is the first byte of an embedded
// 21-byte SpecialEffect record (5 int32 + 1 byte).
void ActionZone::readSpecialEffect(Common::SeekableReadStream &stream) {
	specialEffectId = stream.readUint16LE();
	byte b = stream.readByte();
	if (b == 0xff) {
		return;
	}

	stream.seek(-1, SEEK_CUR);
	hasSpecialEffect = true;
	for (int i = 0; i < 5; ++i) {
		specialEffect[i] = stream.readSint32LE();
	}
	specialEffectFlag = stream.readByte();
}

void ActionZone::readOverlayZone(Common::SeekableReadStream &stream) {
	readFilename(stream, overlayName);

	int16 numSrcRects = stream.readSint16LE();
	if (numSrcRects > 0) {
		overlaySrcRects.resize(numSrcRects);
		for (int i = 0; i < numSrcRects; ++i) {
			readRect(stream, overlaySrcRects[i]);
		}
	}

	readRect(stream, overlayDestRect);
	stream.skip(4);		// int32
	stream.skip(1);		// byte (loop/play mode)
	stream.skip(4);		// int32
}

void readActionZoneArray(Common::SeekableReadStream &stream, Common::Array<ActionZone> &out) {
	int16 count = stream.readSint16LE();
	if (count <= 0) {
		return;
	}

	out.resize(count);
	for (int i = 0; i < count; ++i) {
		out[i].readData(stream);
	}
}

} // End of namespace Action
} // End of namespace Nancy
