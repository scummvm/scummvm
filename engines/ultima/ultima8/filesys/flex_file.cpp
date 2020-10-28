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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/filesys/flex_file.h"
#include "ultima/ultima8/filesys/idata_source.h"

namespace Ultima {
namespace Ultima8 {

FlexFile::FlexFile(Common::SeekableReadStream *rs) : _rs(rs), _count(0) {
	_valid = isFlexFile(_rs);

	if (_valid) {
		_rs->seek(0x54);
		_count = _rs->readUint32LE();
	}
}

FlexFile::~FlexFile() {
	delete _rs;
}

//static
bool FlexFile::isFlexFile(Common::SeekableReadStream *_rs) {
	_rs->seek(0);
	int i;
	char buf[0x52];
	_rs->read(buf, 0x52);

	for (i = 0; i < 0x52; ++i) {
		if (buf[i] == 0x1A) break;
	}

	if (i < 0x52) {
		for (++i; i < 0x52; ++i) {
			if (buf[i] != 0x1A) return false;
		}
		return true;
	}
	return false;
}

uint32 FlexFile::getOffset(uint32 index) {
	_rs->seek(0x80 + 8 * index);
	return _rs->readUint32LE();
}

uint8 *FlexFile::getObject(uint32 index, uint32 *sizep) {
	if (index >= _count)
		return nullptr;

	uint32 size = getSize(index);
	if (size == 0)
		return nullptr;

	uint8 *object = new uint8[size];
	uint32 offset = getOffset(index);

	_rs->seek(offset);
	_rs->read(object, size);

	if (sizep) *sizep = size;

	return object;
}

uint32 FlexFile::getSize(uint32 index) const {
	if (index >= _count) return 0;

	_rs->seek(0x84 + 8 * index);
	uint32 length = _rs->readUint32LE();

	return length;
}

bool FlexFile::nameToIndex(const Std::string &name, uint32 &index) const {
	return extractIndexFromName(name, index);
}

} // End of namespace Ultima8
} // End of namespace Ultima
