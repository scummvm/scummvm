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

#include "common/memstream.h"

#include "ultima/ultima8/misc/debugger.h"
#include "ultima/ultima8/filesys/flex_file.h"

namespace Ultima {
namespace Ultima8 {

static const int FLEX_TABLE_OFFSET = 0x80;
static const int FLEX_HDR_SIZE = 0x52;
static const char FLEX_HDR_PAD = 0x1A;

FlexFile::FlexFile(Common::SeekableReadStream *rs) : _rs(rs) {
	_valid = isFlexFile(_rs);

	if (_valid)
		_valid = readMetadata();
}

FlexFile::~FlexFile() {
	delete _rs;
}

//static
bool FlexFile::isFlexFile(Common::SeekableReadStream *rs) {
	rs->seek(0);
	int i;
	char buf[FLEX_HDR_SIZE];
	rs->read(buf, FLEX_HDR_SIZE);

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

bool FlexFile::readMetadata() {
	_rs->seek(FLEX_HDR_SIZE + 2);
	uint32 count = _rs->readUint32LE();

	if (count > 4095) {
		// In practice the largest flex in either Crusader or U8 games has
		// 3074 entries, so this seems invalid.
		warning("Flex invalid: improbable number of entries %d", count);
		return false;
	}

	if (_rs->size() < FLEX_TABLE_OFFSET + 8 * count) {
		warning("Flex invalid: stream not long enough for offset table");
		return false;
	}

	_entries.reserve(count);
	_rs->seek(FLEX_TABLE_OFFSET);
	for (unsigned int i = 0; i < count; ++i) {
		FileEntry fe;
		fe._offset = _rs->readUint32LE();
		fe._size = _rs->readUint32LE();

		_entries.push_back(fe);
	}

	return true;
}

Common::SeekableReadStream *FlexFile::getDataSource(uint32 index, bool is_text) {
	uint32 size;
	uint8 *buf = getObject(index, &size);

	if (!buf)
		return nullptr;

	return new Common::MemoryReadStream(buf, size, DisposeAfterUse::YES);
}

uint8 *FlexFile::getObject(uint32 index, uint32 *sizep) {
	if (index >= _entries.size())
		return nullptr;

	uint32 size = _entries[index]._size;
	if (size == 0)
		return nullptr;

	uint8 *object = new uint8[size];
	uint32 offset = _entries[index]._offset;

	_rs->seek(offset);
	_rs->read(object, size);

	if (sizep)
		*sizep = size;

	return object;
}

uint32 FlexFile::getSize(uint32 index) const {
	if (index >= _entries.size())
		return 0;

	return _entries[index]._size;
}

} // End of namespace Ultima8
} // End of namespace Ultima
