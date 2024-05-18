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

#include "stream-helper.h"

#include "common/textconsole.h"

using namespace Common;

namespace Alcachofa {

bool readBool(ReadStream &stream) {
	return stream.readByte() != 0;
}

Point readPoint(ReadStream &stream) {
	return { (int16)stream.readSint32LE(), (int16)stream.readSint32LE() };
}

static uint32 readVarInt(ReadStream &stream) {
	uint32 length = stream.readByte();
	if (length != 0xFF)
		return length;
	length = stream.readUint16LE();
	if (length != 0xFFFF)
		return length;
	return stream.readUint32LE();
}

String readVarString(ReadStream &stream) {
	uint32 length = readVarInt(stream);
	if (length == 0)
		return Common::String();

	// TODO: Being able to resize a string would avoid the double-allocation :/
	char *buffer = new char[length];
	if (buffer == nullptr)
		error("Out of memory in readVarString");
	if (stream.read(buffer, length) != length)
		error("Could not read all %u bytes in readVarString", length);

	String result(buffer, buffer + length);
	delete[] buffer;
	return result;
}

void skipVarString(SeekableReadStream &stream) {
	stream.skip(readVarInt(stream));
}

void syncPoint(Serializer &serializer, Point &point) {
	serializer.syncAsSint32LE(point.x);
	serializer.syncAsSint32LE(point.y);
}

}
