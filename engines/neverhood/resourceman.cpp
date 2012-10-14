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
	debug("_entries.size() = %d", _entries.size());
}

ResourceFileEntry *ResourceMan::findEntrySimple(uint32 fileHash) {
	EntriesMap::iterator p = _entries.find(fileHash);
	return p != _entries.end() ? &(*p)._value : NULL;
}

ResourceFileEntry *ResourceMan::findEntry(uint32 fileHash) {
	ResourceFileEntry *entry = findEntrySimple(fileHash);
	for (; entry && entry->archiveEntry->comprType == 0x65; fileHash = entry->archiveEntry->diskSize)
		entry = findEntrySimple(fileHash);
	return entry;
}

int ResourceMan::useResource(uint32 fileHash) {
	ResourceFileEntry *entry = findEntry(fileHash);
	if (!entry)
		return -1;
	if (entry->resourceHandle != -1) {
		_resources[entry->resourceHandle]->useRefCount++;
	} else {
		Resource *resource = new Resource();
		resource->entry = entry;
		resource->useRefCount = 1;
		entry->resourceHandle = (int)_resources.size();
		_resources.push_back(resource);
	}
	return entry->resourceHandle;
}

void ResourceMan::unuseResource(int resourceHandle) {
	if (resourceHandle < 0)
		return;
	Resource *resource = _resources[resourceHandle];
	if (resource->useRefCount > 0)
		resource->useRefCount--;
}

uint32 ResourceMan::getResourceSize(int resourceHandle) const {
	if (resourceHandle < 0)
		return 0;
	Resource *resource = _resources[resourceHandle];
	return resource->entry->archiveEntry->size;
}

byte ResourceMan::getResourceType(int resourceHandle) {
	if (resourceHandle < 0)
		return 0;
	Resource *resource = _resources[resourceHandle];
	return resource->entry->archiveEntry->type;
}

byte ResourceMan::getResourceTypeByHash(uint32 fileHash) {
	ResourceFileEntry *entry = findEntry(fileHash);
	return entry->archiveEntry->type;
}

byte *ResourceMan::getResourceExtData(int resourceHandle) {
	if (resourceHandle < 0)
		return NULL;
	Resource *resource = _resources[resourceHandle];
	return resource->entry->archive->getEntryExtData(resource->entry->archiveEntry);
}

byte *ResourceMan::getResourceExtDataByHash(uint32 fileHash) {
	ResourceFileEntry *entry = findEntrySimple(fileHash);
	return entry ? entry->archive->getEntryExtData(entry->archiveEntry) : NULL;
}

byte *ResourceMan::loadResource(int resourceHandle, bool moveToFront) {
	if (resourceHandle < 0)
		return NULL;
	Resource *resource = _resources[resourceHandle];
	ResourceData *resourceData = _data[resource->entry->archiveEntry->fileHash];
	if (!resourceData) {
		resourceData = new ResourceData();
		_data[resource->entry->archiveEntry->fileHash] = resourceData;
	}
	if (resourceData->data != NULL) {
		resourceData->dataRefCount++;
	} else {
		resourceData->data = new byte[resource->entry->archiveEntry->size];
		resource->entry->archive->load(resource->entry->archiveEntry, resourceData->data, 0);
		resourceData->dataRefCount = 1;
	}
	return resourceData->data;
}

void ResourceMan::unloadResource(int resourceHandle) {
	if (resourceHandle < 0)
		return;
	Resource *resource = _resources[resourceHandle];
	ResourceData *resourceData = _data[resource->entry->archiveEntry->fileHash];
	if (resourceData && resourceData->dataRefCount > 0)
		resourceData->dataRefCount--;
}

Common::SeekableReadStream *ResourceMan::createStream(uint32 fileHash) {
	ResourceFileEntry *entry = findEntry(fileHash);
	return entry->archive->createStream(entry->archiveEntry);
}

} // End of namespace Neverhood
