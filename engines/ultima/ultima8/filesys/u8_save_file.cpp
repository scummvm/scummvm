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

#include "ultima/ultima8/filesys/u8_save_file.h"
#include "ultima/ultima8/filesys/idata_source.h"

namespace Ultima {
namespace Ultima8 {

U8SaveFile::U8SaveFile(Common::SeekableReadStream *rs) : _rs(rs), _count(0) {
	_valid = isU8SaveFile(_rs);

	if (_valid)
		_valid = readMetadata();
}

U8SaveFile::~U8SaveFile() {
	delete _rs;
}

//static
bool U8SaveFile::isU8SaveFile(Common::SeekableReadStream *_rs) {
	_rs->seek(0);
	char buf[24];
	_rs->read(buf, 23);
	buf[23] = '\0';

	return (Std::strncmp(buf, "Ultima 8 SaveGame File.", 23) == 0);
}

bool U8SaveFile::readMetadata() {
	_rs->seek(0x18);
	_count = _rs->readUint16LE();

	_offsets.resize(_count);
	_sizes.resize(_count);

	for (unsigned int i = 0; i < _count; ++i) {
		uint32 namelen = _rs->readUint32LE();
		char *buf = new char[namelen];
		_rs->read(buf, static_cast<int32>(namelen));
		Std::string filename = buf;
		_indices[filename] = i;
		storeIndexedName(filename);
		delete[] buf;
		_sizes[i] = _rs->readUint32LE();
		_offsets[i] = _rs->pos();
		_rs->skip(_sizes[i]); // skip data
	}

	return true;
}

bool U8SaveFile::findIndex(const Std::string &name, uint32 &index) const {
	Std::map<Common::String, uint32>::const_iterator iter;
	iter = _indices.find(name);
	if (iter == _indices.end()) return false;
	index = iter->_value;
	return true;
}

bool U8SaveFile::exists(const Std::string &name) {
	uint32 index;
	return findIndex(name, index);
}

uint8 *U8SaveFile::getObject(const Std::string &name, uint32 *sizep) {
	uint32 index;
	if (!findIndex(name, index))
		return nullptr;

	uint32 size = _sizes[index];
	if (size == 0)
		return nullptr;

	uint8 *object = new uint8[size];
	uint32 offset = _offsets[index];

	_rs->seek(offset);
	_rs->read(object, size);

	if (sizep) *sizep = size;

	return object;
}


uint32 U8SaveFile::getSize(const Std::string &name) const {
	uint32 index;
	if (!findIndex(name, index))
		return 0;

	return _sizes[index];
}

} // End of namespace Ultima8
} // End of namespace Ultima
