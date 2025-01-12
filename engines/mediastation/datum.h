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

#ifndef MEDIASTATION_DATUM_H
#define MEDIASTATION_DATUM_H

#include "common/str.h"
#include "common/array.h"
#include "common/rect.h"
#include "common/stream.h"

#include "mediastation/chunk.h"

namespace MediaStation {

enum DatumType {
	// This type isn't a type we see in data files; it is just a
	// default initialization value.
	kDatumTypeInvalid = 0x0000,

	kDatumTypeUint8 = 0x0002,
	// TODO: Understand why there are different (u)int16 type codes.
	kDatumTypeUint16_1 = 0x0003,
	kDatumTypeUint16_2 = 0x0013,
	kDatumTypeInt16_1 = 0x0006,
	kDatumTypeInt16_2 = 0x0010,
	// TODO: Understand why there are two different uint32 type codes.
	kDatumTypeUint32_1 = 0x0004,
	kDatumTypeUint32_2 = 0x0007,
	// TODO: Understand why there are two different float64 type codes.
	kDatumTypeFloat64_1 = 0x0011,
	kDatumTypeFloat64_2 = 0x0009,
	kDatumTypeString = 0x0012,
	kDatumTypeFilename = 0x000a,
	kDatumTypePoint1 = 0x000f,
	kDatumTypePoint2 = 0x000e,
	kDatumTypeBoundingBox = 0x000d,
	kDatumTypePolygon = 0x001d,
	// These are other types.
	kDatumTypePalette = 0x05aa,
	kDatumTypeReference = 0x001b
};

// It is the caller's responsibility to delete any heap items
// that are created as part of a datum. The datum is really
// just a container.
class Datum {
public:
	DatumType t;
	union {
		int i;
		double f;
		uint32 chunkRef;
		Common::String *string;
		Common::Array<Common::Point *> *polygon;
		Common::Point *point;
		Common::Rect *bbox;
		unsigned char *palette;
	} u;

	Datum();
	Datum(Common::SeekableReadStream &chunk);
	Datum(Common::SeekableReadStream &chunk, DatumType expectedType);

private:
	void readWithType(Common::SeekableReadStream &chunk);
};

} // End of namespace MediaStation

#endif
