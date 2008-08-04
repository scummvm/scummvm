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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#include "common/events.h"
#include "common/keyboard.h"
#include "common/file.h"
#include "common/savefile.h"
#include "common/config-manager.h"

#include "base/plugins.h"
#include "base/version.h"

#include "sound/mixer.h"

#include "picture/picture.h"
#include "picture/resource.h"

namespace Picture {


/* ArchiveReader */

ArchiveReader::ArchiveReader() {
}

ArchiveReader::~ArchiveReader() {
}

void ArchiveReader::openArchive(const char *filename) {
	open(filename);
	for (uint i = 0; i < 10000; i++)
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

void ArchiveReader::dump(uint resIndex, const char *prefix) {
	int32 resourceSize = getResourceSize(resIndex);
	byte *data = new byte[resourceSize];

	char fn[256];
	
	if (prefix)
		snprintf(fn, 256, "%s_%04X.0", prefix, resIndex);
	else
		snprintf(fn, 256, "%04X.0", resIndex);

	openResource(resIndex);
	read(data, resourceSize);
	closeResource();

	FILE *o = fopen(fn, "wb");
	fwrite(data, resourceSize, 1, o);
	fclose(o);

	delete[] data;
}

/* ResourceCache */

ResourceCache::ResourceCache(PictureEngine *vm) : _vm(vm) {

	_base = new byte[kMaxCacheSize];
	_bytesUsed = 0;

	memset(_cache, 0, sizeof(_cache));
	_cacheCount = 0;

	_curItemOffset = 0;
	_curItemSize = 0;

}

ResourceCache::~ResourceCache() {
	delete[] _base;
}

byte *ResourceCache::load(uint resIndex) {
	byte *data = NULL;
	if (existsItem(resIndex)) {
		debug(1, "ResourceCache::load(%d) From cache", resIndex);
		data = _base + _curItemOffset;
	} else {
		debug(1, "ResourceCache::load(%d) From disk", resIndex);
		uint32 size = _vm->_arc->openResource(resIndex);
		data = addItem(resIndex, size);
		_vm->_arc->read(data, size);
		_vm->_arc->closeResource();
	}
	return data;
}

bool ResourceCache::existsItem(uint resIndex) {
	for (uint i = 0; i < _cacheCount; i++) {
		if (_cache[i].resIndex == resIndex) {
			_curItemOffset = _cache[i].offset;
			_curItemSize = _cache[i].size;
			return true;
		}
	}
	return false;
}

byte *ResourceCache::addItem(uint resIndex, uint32 size) {

	checkCapacity(size);

	_curItemOffset = _bytesUsed;
	_curItemSize = size;

	_cache[_cacheCount].resIndex = resIndex;
	_cache[_cacheCount].offset = _curItemOffset;
	_cache[_cacheCount].size = _curItemSize;
	_cacheCount++;

	_bytesUsed += size;

	return _base + _curItemOffset;

}

void ResourceCache::checkCapacity(uint32 size) {
	if (_cacheCount > kMaxCacheItems || _bytesUsed + size > kMaxCacheSize) {
		_cacheCount = 0;
		_bytesUsed = 0;
	}
}

} // End of namespace Picture
