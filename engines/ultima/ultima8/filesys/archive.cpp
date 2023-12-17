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

#include "ultima/ultima8/misc/debugger.h"
#include "ultima/ultima8/filesys/archive.h"
#include "ultima/ultima8/filesys/flex_file.h"
#include "ultima/ultima8/filesys/u8_save_file.h"

namespace Ultima {
namespace Ultima8 {

Archive::Archive() {
	_count = 0;
}

Archive::~Archive() {
	for (unsigned int i = 0; i < _sources.size(); ++i)
		delete _sources[i];
	_sources.clear();
}

Archive::Archive(Common::SeekableReadStream *rs) : _count(0) {
	addSource(rs);
}

bool Archive::addSource(FlexFile *af) {
	_sources.push_back(af);

	uint32 indexcount = af->getCount();
	if (indexcount > _count)
		_count = indexcount;

	return true;
}

bool Archive::addSource(Common::SeekableReadStream *rs) {
	if (!rs)
		return false;

	FlexFile *s = new FlexFile(rs);
	if (!s->isValid()) {
		delete s;
		return false;
	}

	return addSource(s);
}

void Archive::cache() {
	for (unsigned int i = 0; i < _count; ++i)
		cache(i);
}

void Archive::uncache() {
	for (unsigned int i = 0; i < _count; ++i)
		uncache(i);
}

uint8 *Archive::getRawObject(uint32 index, uint32 *sizep) {
	FlexFile *f = findArchiveFile(index);
	if (!f)
		return nullptr;

	return f->getObject(index, sizep);
}

uint32 Archive::getRawSize(uint32 index) const {
	FlexFile *f = findArchiveFile(index);
	if (!f) return 0;

	return f->getSize(index);
}

FlexFile *Archive::findArchiveFile(uint32 index) const {
	unsigned int n = _sources.size();
	for (unsigned int i = 1; i <= n; ++i) {
		if (_sources[n - i]->exists(index))
			return _sources[n - i];
	}

	return nullptr;
}

} // End of namespace Ultima8
} // End of namespace Ultima
