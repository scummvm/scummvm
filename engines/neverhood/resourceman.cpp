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
 */

#include "neverhood/resourceman.h"

namespace Neverhood {

ResourceHandle::ResourceHandle()
	: _resourceFileEntry(NULL), _data(NULL) {
}

ResourceHandle::~ResourceHandle() {
}

ResourceMan::ResourceMan() {
}

ResourceMan::~ResourceMan() {
}

void ResourceMan::addArchive(const Common::String &filename) {
	BlbArchive *archive = new BlbArchive();
	archive->open(filename);
	_archives.push_back(archive);
	debug(3, "ResourceMan::addArchive(%s) %d files", filename.c_str(), archive->getCount());
	for (uint archiveEntryIndex = 0; archiveEntryIndex < archive->getCount(); archiveEntryIndex++) {
		BlbArchiveEntry *archiveEntry = archive->getEntry(archiveEntryIndex);
		ResourceFileEntry *entry = findEntrySimple(archiveEntry->fileHash);
		if (entry) {
			if (archiveEntry->timeStamp > entry->archiveEntry->timeStamp) {
				entry->archive = archive;
				entry->archiveEntry = archiveEntry;
			} 
		} else {
			ResourceFileEntry newEntry;
			newEntry.resourceHandle = -1;
			newEntry.archive = archive;
			newEntry.archiveEntry = archiveEntry;
			_entries[archiveEntry->fileHash] = newEntry;
		}
	}
}

ResourceFileEntry *ResourceMan::findEntrySimple(uint32 fileHash) {
	EntriesMap::iterator p = _entries.find(fileHash);
	return p != _entries.end() ? &(*p)._value : NULL;
}

ResourceFileEntry *ResourceMan::findEntry(uint32 fileHash, ResourceFileEntry **firstEntry) {
	ResourceFileEntry *entry = findEntrySimple(fileHash);
	if (firstEntry)
		*firstEntry = entry;
	for (; entry && entry->archiveEntry->comprType == 0x65; fileHash = entry->archiveEntry->diskSize)
		entry = findEntrySimple(fileHash);
	return entry;
}

Common::SeekableReadStream *ResourceMan::createStream(uint32 fileHash) {
	ResourceFileEntry *entry = findEntry(fileHash);
	return entry ? entry->archive->createStream(entry->archiveEntry) : NULL;
}

void ResourceMan::queryResource(uint32 fileHash, ResourceHandle &resourceHandle) {
	ResourceFileEntry *firstEntry;
	resourceHandle._resourceFileEntry = findEntry(fileHash, &firstEntry);
	resourceHandle._extData = firstEntry ? firstEntry->archiveEntry->extData : NULL;
}

void ResourceMan::loadResource(ResourceHandle &resourceHandle) {
	resourceHandle._data = NULL;
	if (resourceHandle.isValid()) {
		const uint32 fileHash = resourceHandle.fileHash();
		ResourceData *resourceData = _data[fileHash];
		if (!resourceData) {
			resourceData = new ResourceData();
			_data[fileHash] = resourceData;
		}
		if (resourceData->data != NULL) {
			resourceData->dataRefCount++;
		} else {
			resourceData->data = new byte[resourceHandle._resourceFileEntry->archiveEntry->size];
			resourceHandle._resourceFileEntry->archive->load(resourceHandle._resourceFileEntry->archiveEntry, resourceData->data, 0);
			resourceData->dataRefCount = 1;
		}
		resourceHandle._data = resourceData->data;
	}
}

void ResourceMan::unloadResource(ResourceHandle &resourceHandle) {
	if (resourceHandle.isValid()) {
		ResourceData *resourceData = _data[resourceHandle.fileHash()];
		if (resourceData && resourceData->dataRefCount > 0)
			--resourceData->dataRefCount;
		resourceHandle._resourceFileEntry = NULL;
		resourceHandle._data = NULL;
	}
}

void ResourceMan::purgeResources() {
	for (Common::HashMap<uint32, ResourceData*>::iterator it = _data.begin(); it != _data.end(); ++it) {
		ResourceData *resourceData = (*it)._value;
		if (resourceData->dataRefCount == 0) {
			delete[] resourceData->data;
			resourceData->data = NULL;
		}
	}
}

} // End of namespace Neverhood
