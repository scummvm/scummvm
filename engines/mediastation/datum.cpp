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

#include "mediastation/mediastation.h"
#include "mediastation/chunk.h"
#include "mediastation/datum.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

Datum::Datum() {
	t = kDatumTypeInvalid;
	u.i = 0;
}

Datum::Datum(Common::SeekableReadStream &chunk) {
	t = static_cast<DatumType>(chunk.readUint16LE());
	readWithType(chunk);
}

Datum::Datum(Common::SeekableReadStream &chunk, DatumType expectedType) {
	t = static_cast<DatumType>(chunk.readUint16LE());
	if (t != expectedType) {
		error("Datum::Datum(): Expected datum type 0x%x, got 0x%x (@0x%llx)", expectedType, t, static_cast<long long int>(chunk.pos()));
	}
	readWithType(chunk);
}

void Datum::readWithType(Common::SeekableReadStream &chunk) {
	debugC(9, kDebugLoading, "Datum::Datum(): Type 0x%x (@0x%llx)", static_cast<uint>(t), static_cast<long long int>(chunk.pos()));
	if (kDatumTypeUint8 == t) {
		u.i = chunk.readByte();

	} else if (kDatumTypeUint16_1 == t || kDatumTypeUint16_2 == t) {
		u.i = chunk.readUint16LE();

	} else if (kDatumTypeInt16_1 == t || kDatumTypeInt16_2 == t) {
		u.i = chunk.readSint16LE();

	} else if (kDatumTypeUint32_1 == t || kDatumTypeUint32_2 == t) {
		u.i = chunk.readUint32LE();

	} else if (kDatumTypeFloat64_1 == t || kDatumTypeFloat64_2 == t) {
		u.f = chunk.readDoubleLE();

	} else if (kDatumTypeString == t || kDatumTypeFilename == t) {
		// TODO: This copies the string. Can we read it directly from the chunk?
		int size = Datum(chunk, kDatumTypeUint32_1).u.i;
		char *buffer = new char[size + 1];
		chunk.read(buffer, size);
		buffer[size] = '\0';
		u.string = new Common::String(buffer);
		delete[] buffer;

	} else if (kDatumTypePoint1 == t || kDatumTypePoint2 == t) {
		uint16 x = Datum(chunk, kDatumTypeInt16_2).u.i;
		uint16 y = Datum(chunk, kDatumTypeInt16_2).u.i;
		u.point = new Common::Point(x, y);

	} else if (kDatumTypeBoundingBox == t) {
		Common::Point *left_top = Datum(chunk, kDatumTypePoint2).u.point;
		Common::Point *dimensions = Datum(chunk, kDatumTypePoint1).u.point;
		u.bbox = new Common::Rect(*left_top, dimensions->x, dimensions->y);
		delete left_top;
		delete dimensions;

	} else if (kDatumTypePolygon == t) {
		uint16 total_points = Datum(chunk, kDatumTypeUint16_1).u.i;
		for (int i = 0; i < total_points; i++) {
			Common::Point *point = Datum(chunk, kDatumTypePoint1).u.point;
			u.polygon->push_back(point);
		}

	} else if (kDatumTypePalette == t) {
		u.palette = new unsigned char[0x300];
		chunk.read(u.palette, 0x300);

	} else if (kDatumTypeReference == t) {
		u.chunkRef = chunk.readUint32BE();

	} else {
		error("Unknown datum type: 0x%x (@0x%llx)", static_cast<uint>(t), static_cast<long long int>(chunk.pos()));
	}
}

} // End of namespace MediaStation
