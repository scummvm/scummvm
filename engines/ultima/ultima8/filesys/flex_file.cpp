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

#include "ultima/ultima8/misc/pent_include.h"
#include "ultima/ultima8/filesys/flex_file.h"

namespace Ultima {
namespace Ultima8 {

static const int FLEX_TABLE_OFFSET = 0x80;
static const int FLEX_HDR_SIZE = 0x52;
static const char FLEX_HDR_PAD = 0x1A;

FlexFile::FlexFile(Common::SeekableReadStream *rs) : _rs(rs), _count(0) {
	_valid = isFlexFile(_rs);

	if (_valid) {
		_rs->seek(FLEX_HDR_SIZE + 2);
		_count = _rs->readUint32LE();
	}
	if (_count > 4095) {
		// In practice the largest flex in either Crusader or U8 games has
		// 3074 entries, so this seems invalid.
		warning("Flex invalid: improbable number of entries %d", _count);
		_valid = false;
		_count = 0;
	}
	if (rs->size() < FLEX_TABLE_OFFSET + 8 * _count) {
		warning("Flex invalid: stream not long enough for offset table");
		_valid = false;
		_count = 0;
	}
}

FlexFile::~FlexFile() {
	delete _rs;
}

//static
bool FlexFile::isFlexFile(Common::SeekableReadStream *_rs) {
	_rs->seek(0);
	int i;
	char buf[FLEX_HDR_SIZE];
	_rs->read(buf, FLEX_HDR_SIZE);

	for (i = 0; i < FLEX_HDR_SIZE; ++i) {
		if (buf[i] == FLEX_HDR_PAD) break;
	}

	if (i < FLEX_HDR_SIZE) {
		for (++i; i < FLEX_HDR_SIZE; ++i) {
			if (buf[i] != FLEX_HDR_PAD) return false;
		}
		return true;
	}
	return false;
}

uint32 FlexFile::getOffset(uint32 index) {
	_rs->seek(FLEX_TABLE_OFFSET + 8 * index);
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

	_rs->seek(FLEX_TABLE_OFFSET + 4 + 8 * index);
	uint32 length = _rs->readUint32LE();

	return length;
}

bool FlexFile::nameToIndex(const Std::string &name, uint32 &index) const {
	return extractIndexFromName(name, index);
}

} // End of namespace Ultima8
} // End of namespace Ultima
