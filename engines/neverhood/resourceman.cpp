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
	uint archiveIndex = _archives.size();
	archive->open(filename);
	_archives.push_back(archive);
	debug("ResourceMan::addArchive(%s) %d files", filename.c_str(), archive->getCount());
	_entries.reserve(_entries.size() + archive->getCount());
	for (uint archiveEntryIndex = 0; archiveEntryIndex < archive->getCount(); archiveEntryIndex++) {
		BlbArchiveEntry *archiveEntry = archive->getEntry(archiveEntryIndex);
		ResourceFileEntry *entry = findEntrySimple(archiveEntry->fileHash);
		if (entry) {
			if (archiveEntry->timeStamp > _archives[entry->archiveIndex]->getEntry(entry->entryIndex)->timeStamp) {
				entry->archiveIndex = archiveIndex;
				entry->entryIndex = archiveEntryIndex;
			} 
		} else {
			ResourceFileEntry newEntry;
			newEntry.fileHash = archiveEntry->fileHash;
			newEntry.resourceHandle = -1;
			newEntry.archiveIndex = archiveIndex;
			newEntry.entryIndex = archiveEntryIndex;
			_entries.push_back(newEntry);
		}
	}
}

ResourceFileEntry *ResourceMan::findEntrySimple(uint32 fileHash) {
	for (uint i = 0; i < _entries.size(); i++) {
		if (_entries[i].fileHash == fileHash)
			return &_entries[i];
	}
	return NULL;
}

ResourceFileEntry *ResourceMan::findEntry(uint32 fileHash) {
	ResourceFileEntry *entry = findEntrySimple(fileHash);
	for (; entry && getArchiveEntry(entry)->comprType == 0x65; fileHash = getArchiveEntry(entry)->diskSize)
		entry = findEntrySimple(fileHash);
	return entry;
}

BlbArchiveEntry *ResourceMan::getArchiveEntry(ResourceFileEntry *entry) const {
	return _archives[entry->archiveIndex]->getEntry(entry->entryIndex);
}

int ResourceMan::useResource(uint32 fileHash) {
	ResourceFileEntry *entry = findEntry(fileHash);
	if (entry->resourceHandle != -1) {
		_resources[entry->resourceHandle]->useRefCount++;
	} else {
		Resource *resource = new Resource();
		resource->fileHash = entry->fileHash;
		resource->archiveIndex = entry->archiveIndex;
		resource->entryIndex = entry->entryIndex;
		resource->data = NULL;
		resource->dataRefCount = 0;
		resource->useRefCount = 1;
		entry->resourceHandle = (int)_resources.size();
		_resources.push_back(resource);
	}
	return entry->resourceHandle;
}

void ResourceMan::unuseResource(int resourceHandle) {
	Resource *resource = _resources[resourceHandle];
	if (resource->useRefCount > 0)
		resource->useRefCount--;
}

void ResourceMan::unuseResourceByHash(uint32 fileHash) {
	ResourceFileEntry *entry = findEntry(fileHash);
	if (entry->resourceHandle != -1)
		unuseResource(entry->resourceHandle);
}

int ResourceMan::getResourceHandleByHash(uint32 fileHash) {
	ResourceFileEntry *entry = findEntry(fileHash);
	return entry->resourceHandle;
}

bool ResourceMan::isResourceDataValid(int resourceHandle) const {
	return _resources[resourceHandle]->data != NULL;
}

uint32 ResourceMan::getResourceSize(int resourceHandle) const {
	Resource *resource = _resources[resourceHandle];
	return _archives[resource->archiveIndex]->getEntry(resource->entryIndex)->size;
}

byte ResourceMan::getResourceType(int resourceHandle) {
	Resource *resource = _resources[resourceHandle];
	return _archives[resource->archiveIndex]->getEntry(resource->entryIndex)->type;
}

byte ResourceMan::getResourceTypeByHash(uint32 fileHash) {
	ResourceFileEntry *entry = findEntry(fileHash);
	return getArchiveEntry(entry)->type;
}

byte *ResourceMan::getResourceExtData(int resourceHandle) {
	Resource *resource = _resources[resourceHandle];
	return _archives[resource->archiveIndex]->getEntryExtData(resource->entryIndex);
}

byte *ResourceMan::getResourceExtDataByHash(uint32 fileHash) {
	ResourceFileEntry *entry = findEntry(fileHash);
	return _archives[entry->archiveIndex]->getEntryExtData(entry->entryIndex);
}

byte *ResourceMan::loadResource(int resourceHandle, bool moveToFront) {
	Resource *resource = _resources[resourceHandle];
	if (resource->data != NULL) {
		resource->dataRefCount++;
	} else {
		BlbArchive *archive = _archives[resource->archiveIndex];
		BlbArchiveEntry *archiveEntry = archive->getEntry(resource->entryIndex);
		resource->data = new byte[archiveEntry->size];
		archive->load(resource->entryIndex, resource->data, 0);
		resource->dataRefCount = 1;
	}
	return resource->data;
}

void ResourceMan::unloadResource(int resourceHandle) {
	Resource *resource = _resources[resourceHandle];
	if (resource->dataRefCount > 0)
		resource->dataRefCount--;
}

void ResourceMan::freeResource(Resource *resource) {
	delete[] resource->data;
	resource->data = NULL;
}

} // End of namespace Neverhood
