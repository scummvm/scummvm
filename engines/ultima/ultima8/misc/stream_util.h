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

#ifndef ULTIMA8_MISC_STREAM_H
#define ULTIMA8_MISC_STREAM_H

#include "common/stream.h"

namespace Ultima {
namespace Ultima8 {

//  Read a 3-byte value, lsb first.
inline uint32 readUint24LE(Common::ReadStream &rs) {
	uint32 val = 0;
	val |= static_cast<uint32>(rs.readByte());
	val |= static_cast<uint32>(rs.readByte() << 8);
	val |= static_cast<uint32>(rs.readByte() << 16);
	return val;
}

inline uint32 readX(Common::ReadStream &rs, uint32 num_bytes) {
	assert(num_bytes > 0 && num_bytes <= 4);
	if (num_bytes == 1) return rs.readByte();
	else if (num_bytes == 2) return rs.readUint16LE();
	else if (num_bytes == 3) return readUint24LE(rs);
	else return rs.readUint32LE();
}

inline int32 readXS(Common::ReadStream &rs, uint32 num_bytes) {
	assert(num_bytes > 0 && num_bytes <= 4);
	if (num_bytes == 1) return static_cast<int8>(rs.readByte());
	else if (num_bytes == 2) return static_cast<int16>(rs.readUint16LE());
	else if (num_bytes == 3) return (((static_cast<int32>(readUint24LE(rs))) << 8) >> 8);
	else return static_cast<int32>(rs.readUint32LE());
}

inline void writeUint24LE(Common::WriteStream &ws, uint32 val) {
	ws.writeByte(static_cast<byte>(val & 0xff));
	ws.writeByte(static_cast<byte>((val >> 8) & 0xff));
	ws.writeByte(static_cast<byte>((val >> 16) & 0xff));
}

inline void writeX(Common::WriteStream &ws, uint32 val, uint32 num_bytes) {
	assert(num_bytes > 0 && num_bytes <= 4);
	if (num_bytes == 1) ws.writeByte(static_cast<byte>(val));
	else if (num_bytes == 2) ws.writeUint16LE(static_cast<uint16>(val));
	else if (num_bytes == 3) writeUint24LE(ws, val);
	else ws.writeUint32LE(val);
}

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
