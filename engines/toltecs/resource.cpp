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

#include "common/file.h"

#include "toltecs/toltecs.h"
#include "toltecs/resource.h"

namespace Toltecs {


/* ArchiveReader */

ArchiveReader::ArchiveReader() {
	_offsets = 0;
}

ArchiveReader::~ArchiveReader() {
	delete[] _offsets;
}

void ArchiveReader::openArchive(const char *filename) {
	open(filename);
	uint32 firstOffs = readUint32LE();
	uint count = firstOffs / 4;
	_offsets = new uint32[count];
	_offsets[0] = firstOffs;
	for (uint i = 1; i < count; i++)
		_offsets[i] = readUint32LE();
}

uint32 ArchiveReader::openResource(uint resIndex) {
	uint32 resourceSize = getResourceSize(resIndex);
	seek(_offsets[resIndex]);
	return resourceSize;
}

void ArchiveReader::closeResource() {
}

uint32 ArchiveReader::getResourceSize(uint resIndex) {
	return _offsets[resIndex + 1] - _offsets[resIndex];
}

void ArchiveReader::dump(uint resIndex) {
	int32 resourceSize = getResourceSize(resIndex);
	byte *data = new byte[resourceSize];

	Common::Path fn(Common::String::format("toltecs_res.%03d", resIndex));

	openResource(resIndex);
	read(data, resourceSize);
	closeResource();

	Common::DumpFile o;
	o.open(fn);
	o.write(data, resourceSize);
	o.finalize();
	o.close();

	delete[] data;
}

/* ResourceCache */

ResourceCache::ResourceCache(ToltecsEngine *vm) : _vm(vm) {
}

ResourceCache::~ResourceCache() {
	purgeCache();
}

void ResourceCache::purgeCache() {
	for (ResourceMap::iterator iter = _cache.begin(); iter != _cache.end(); ++iter) {
		delete[] iter->_value->data;
		delete iter->_value;
		iter->_value = 0;
	}

	_cache.clear();
}

Resource *ResourceCache::load(uint resIndex) {
	ResourceMap::iterator item = _cache.find(resIndex);
	if (item != _cache.end()) {
		debug(1, "ResourceCache::load(%d) From cache", resIndex);
		return (*item)._value;
	} else {
		debug(1, "ResourceCache::load(%d) From disk", resIndex);

		int32 curPos = _vm->_arc->pos();
		Resource *resItem = new Resource();
		resItem->size = _vm->_arc->openResource(resIndex);
		resItem->data = new byte[resItem->size];
		_vm->_arc->read(resItem->data, resItem->size);
		_vm->_arc->closeResource();
		_vm->_arc->seek(curPos);

		_cache[resIndex] = resItem;

		return resItem;

	}
}

} // End of namespace Toltecs
