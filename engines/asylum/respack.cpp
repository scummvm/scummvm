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

#include "asylum/respack.h"

namespace Asylum {

ResourcePack::ResourcePack(const char *resourceFile) {
	init(resourceFile);
}

ResourcePack::ResourcePack(int resourceIndex) {
	// We don't use the file number part of resource IDs
	//uint32 fileNum = (resourceID >> 16) & 0x7FFF;
	char filename[20];
	sprintf(filename, "res.%03d", resourceIndex);
	init(filename);
}

ResourcePack::~ResourcePack() {
	for (uint32 i = 0; i < _resources.size(); i++)
		delete [] _resources[i].data;

	_resources.clear();
	_packFile.close();
}

void ResourcePack::init(const char *resourceFile) {
	_packFile.open(resourceFile);

	uint32 entryCount = _packFile.readUint32LE();
	_resources.resize(entryCount);

	uint32 prevOffset = _packFile.readUint32LE();
	uint32 nextOffset = 0;

	for (uint32 i = 0; i < entryCount; i++) {
		ResourceEntry entry;
		entry.offset = prevOffset;

		// Read the offset of the next entry to determine the size of this one
		nextOffset = (i < entryCount - 1) ? _packFile.readUint32LE() : _packFile.size();
		entry.size = (nextOffset > 0) ? nextOffset - prevOffset : _packFile.size() - prevOffset;
		entry.data = 0;

		_resources[i] = entry;

		prevOffset = nextOffset;
	}
}

ResourceEntry *ResourcePack::getResource(uint32 resourceId) {
	uint16 index = RESIDX(resourceId);
	if (!_resources[index].data) {
		// Load the requested resource if it's not loaded already
		_packFile.seek(_resources[index].offset, SEEK_SET);
		_resources[index].data = new byte[_resources[index].size];
		_packFile.read(_resources[index].data, _resources[index].size);
	}

	return &_resources[index];
}

} // end of namespace Asylum
