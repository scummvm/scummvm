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

#include "ultima/ultima8/filesys/raw_archive.h"
#include "ultima/ultima8/filesys/idata_source.h"
#include "ultima/ultima8/filesys/archive_file.h"

namespace Ultima {
namespace Ultima8 {

RawArchive::~RawArchive() {
	Archive::uncache();
}

void RawArchive::cache(uint32 index) {
	if (index >= _count) return;
	if (_objects.empty()) _objects.resize(_count);

	if (_objects[index]) return;

	_objects[index] = getRawObject(index);
}

void RawArchive::uncache(uint32 index) {
	if (index >= _count) return;
	if (_objects.empty()) return;

	if (_objects[index]) {
		delete[] _objects[index];
		_objects[index] = nullptr;
	}
}

bool RawArchive::isCached(uint32 index) const {
	if (index >= _count) return false;
	if (_objects.empty()) return false;

	return (_objects[index] != nullptr);
}

const uint8 *RawArchive::get_object_nodel(uint32 index) {
	if (index >= _count)
		return nullptr;
	cache(index);
	return _objects[index];
}

uint8 *RawArchive::get_object(uint32 index) {
	if (index >= _count)
		return nullptr;

	if (index < _objects.size() && _objects[index]) {
		// already cached
		uint32 size = getRawSize(index);
		if (size == 0)
			return nullptr;
		uint8 *object = new uint8[size];
		Std::memcpy(object, _objects[index], size);
		return object;
	}

	return getRawObject(index);
}

uint32 RawArchive::get_size(uint32 index) {
	if (index >= _count)
		return 0;
	return getRawSize(index);
}

IDataSource *RawArchive::get_datasource(uint32 index) {
	if (index >= _count)
		return nullptr;
	cache(index);

	if (!_objects[index])
		return nullptr;

	return new IBufferDataSource(_objects[index], getRawSize(index));
}

} // End of namespace Ultima8
} // End of namespace Ultima
