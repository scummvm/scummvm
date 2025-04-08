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

#include "mediastation/datum.h"
#include "mediastation/debugchannels.h"

namespace MediaStation {

Datum::Datum() {
	t = kDatumTypeEmpty;
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

	} else if (kDatumTypeUint16 == t || kDatumTypeVersion == t) {
		u.i = chunk.readUint16LE();

	} else if (kDatumTypeInt16 == t || kDatumTypeGraphicUnit == t) {
		u.i = chunk.readSint16LE();

	} else if (kDatumTypeUint32 == t || kDatumTypeInt32 == t) {
		u.i = chunk.readUint32LE();

	} else if (kDatumTypeTime == t || kDatumTypeDouble == t) {
		u.f = chunk.readDoubleLE();

	} else if (kDatumTypeString == t || kDatumTypeFilename == t) {
		// TODO: This copies the string. Can we read it directly from the chunk?
		int size = Datum(chunk, kDatumTypeUint32).u.i;
		char *buffer = new char[size + 1];
		chunk.read(buffer, size);
		buffer[size] = '\0';
		u.string = new Common::String(buffer);
		delete[] buffer;

	} else if (kDatumTypeGraphicSize == t || kDatumTypePoint == t) {
		uint16 x = Datum(chunk, kDatumTypeGraphicUnit).u.i;
		uint16 y = Datum(chunk, kDatumTypeGraphicUnit).u.i;
		u.point = new Common::Point(x, y);

	} else if (kDatumTypeRect == t) {
		Common::Point *leftTop = Datum(chunk, kDatumTypePoint).u.point;
		Common::Point *dimensions = Datum(chunk, kDatumTypeGraphicSize).u.point;
		u.bbox = new Common::Rect(*leftTop, dimensions->x, dimensions->y);
		delete leftTop;
		delete dimensions;

	} else if (kDatumTypePolygon == t) {
		uint16 totalPoints = Datum(chunk, kDatumTypeUint16).u.i;
		for (int i = 0; i < totalPoints; i++) {
			Common::Point *point = Datum(chunk, kDatumTypeGraphicSize).u.point;
			u.polygon->push_back(point);
		}

	} else if (kDatumTypePalette == t) {
		u.palette = new unsigned char[0x300];
		chunk.read(u.palette, 0x300);

	} else if (kDatumTypeChunkReference == t) {
		u.chunkRef = chunk.readUint32BE();

	} else {
		error("Unknown datum type: 0x%x (@0x%llx)", static_cast<uint>(t), static_cast<long long int>(chunk.pos()));
	}
}

} // End of namespace MediaStation
