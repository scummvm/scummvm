/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ULTIMA8_FILESYS_ODATASOURCE_H
#define ULTIMA8_FILESYS_ODATASOURCE_H

#include "ultima/ultima8/misc/pent_include.h"
#include "common/stream.h"

namespace Ultima {
namespace Ultima8 {

class ODataSource : public Common::SeekableWriteStream {
public:
	ODataSource() {}
	virtual ~ODataSource() {}

	virtual void writeUint24LE(uint32 val) {
		writeByte(static_cast<byte>(val & 0xff));
		writeByte(static_cast<byte>((val >> 8) & 0xff));
		writeByte(static_cast<byte>((val >> 16) & 0xff));
	}

	void writeX(uint32 val, uint32 num_bytes) {
		assert(num_bytes > 0 && num_bytes <= 4);
		if (num_bytes == 1) writeByte(static_cast<byte>(val));
		else if (num_bytes == 2) writeUint16LE(static_cast<uint16>(val));
		else if (num_bytes == 3) writeUint24LE(val);
		else writeUint32LE(val);
	}

	virtual void skip(int32 delta) {
		seek(delta, SEEK_CUR);
	};
};

} // End of namespace Ultima8
} // End of namespace Ultima

#endif
