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
	t = DatumType::INVALID;
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
	debugC(9, kDebugLoading, "Datum::Datum(): Type 0x%x (@0x%llx)", t, static_cast<long long int>(chunk.pos()));
	if (DatumType::UINT8 == t) {
		u.i = chunk.readByte();

	} else if (DatumType::UINT16_1 == t || DatumType::UINT16_2 == t) {
		u.i = chunk.readUint16LE();

	} else if (DatumType::INT16_1 == t || DatumType::INT16_2 == t) {
		u.i = chunk.readSint16LE();

	} else if (DatumType::UINT32_1 == t || DatumType::UINT32_2 == t) {
		u.i = chunk.readUint32LE();

	} else if (DatumType::FLOAT64_1 == t || DatumType::FLOAT64_2 == t) {
		u.f = chunk.readDoubleLE();

	} else if (DatumType::STRING == t || DatumType::FILENAME == t) {
		// TODO: This copies the string. Can we read it directly from the chunk?
		int size = Datum(chunk, DatumType::UINT32_1).u.i;
		char *buffer = new char[size + 1];
		chunk.read(buffer, size);
		buffer[size] = '\0';
		u.string = new Common::String(buffer);
		delete[] buffer;

	} else if (DatumType::POINT_1 == t || DatumType::POINT_2 == t) {
		uint16 x = Datum(chunk, DatumType::INT16_2).u.i;
		uint16 y = Datum(chunk, DatumType::INT16_2).u.i;
		u.point = new Common::Point(x, y);

	} else if (DatumType::BOUNDING_BOX == t) {
		Common::Point *left_top = Datum(chunk, DatumType::POINT_2).u.point;
		Common::Point *dimensions = Datum(chunk, DatumType::POINT_1).u.point;
		u.bbox = new Common::Rect(*left_top, dimensions->x, dimensions->y);
		delete left_top;
		delete dimensions;

	} else if (DatumType::POLYGON == t) {
		uint16 total_points = Datum(chunk, DatumType::UINT16_1).u.i;
		for (int i = 0; i < total_points; i++) {
			Common::Point *point = Datum(chunk, DatumType::POINT_1).u.point;
			u.polygon->push_back(point);
		}

	} else if (DatumType::PALETTE == t) {
		u.palette = new unsigned char[0x300];
		chunk.read(u.palette, 0x300);

	} else if (DatumType::REFERENCE == t) {
		u.chunkRef = chunk.readUint32BE();

	} else {
		error("Unknown datum type: 0x%x (@0x%llx)", t, static_cast<long long int>(chunk.pos()));
	}
}

} // End of namespace MediaStation
