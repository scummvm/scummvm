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

#include "mediastation/datafile.h"

namespace MediaStation {

enum DatumType {
	kDatumTypeEmpty = 0x00,
	kDatumTypeUint8 = 0x02,
	kDatumTypeUint16 = 0x03,
	kDatumTypeUint32 = 0x04,
	kDatumTypeInt8 = 0x05,
	kDatumTypeInt16 = 0x06,
	kDatumTypeInt32 = 0x07,
	kDatumTypeFloat = 0x08,
	kDatumTypeDouble = 0x09,
	kDatumTypeFilename = 0x0a,
	kDatumTypeRect = 0x0d,
	kDatumTypePoint = 0x0e,
	kDatumTypeGraphicSize = 0x0f,
	kDatumTypeGraphicUnit = 0x10,
	kDatumTypeTime = 0x11,
	kDatumTypeString = 0x12,
	kDatumTypeVersion = 0x13,
	kDatumTypeChunkReference = 0x1b,
	kDatumTypePolygon = 0x1d,
	kDatumTypePalette = 0x05aa
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
