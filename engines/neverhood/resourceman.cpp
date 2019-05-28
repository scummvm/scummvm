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

struct EntrySizeFix {
	uint32 fileHash;
	uint32 offset;
	uint32 diskSize;
	uint32 size;
	uint32 fixedSize;
};

static const EntrySizeFix entrySizeFixes[] = {
	//  fileHash    offset   diskSize  size  fixedSize
	// Fixes for the Russian "Dyadyushka Risech" version
	{ 0x041137051,   667019,  23391,  41398,  29191 },	// "Options" menu header text
	{ 0x00f960021,   402268,   1704,   4378,   1870 },	// "Save" menu
	{ 0x01301a7ea,  1220008,   2373,   4146,   2877 },	// "Load" menu
	{ 0x084181e81,   201409,   1622,   5058,   1833 },	// "Delete" menu
	{ 0x0C10B2015,   690410,   5850,  11162,   7870 },	// Menu text
	{ 0x008C0AC24,  1031009,   3030,   6498,   3646 },	// Overwrite dialog
	{ 0x0c6604282, 12813649,  19623,  35894,  30370 },	// One of the fonts when reading Willie's notes
	{ 0x080283101, 13104841,   1961,   3712,   3511 },	// First message from Willie
	{ 0x058208810, 46010519,  24852, 131874, 114762 },  // Entry to hut with musical lock
	{ 0x000918480, 17676417,    581,    916,    706 },	// First wall in the museum
	{ 0x00800090C, 16064875,  19555,  38518,  30263 },	// First wall in the museum
	{ 0x00008E486, 39600019,    240,    454,    271 },  // Second wall in the museum
	{ 0x003086004, 39621755,    482,    614,    600 },  // Second wall in the museum
	{ 0x02008048E, 39611075,   3798,  21089,   6374 },  // Next walls in the museum
	{ 0x008586283, 39587864,  12155,  29731,  20582 },  // Next walls in the museum
	{ 0x030A84C80, 39606142,   4933,  16305,   8770 },  // Next walls in the museum
	{ 0x000C9A480, 39614873,   6882,  23915,  11571 },  // Next walls in the museum
	{ 0x000098880, 39603114,   3028,  10860,   4762 },  // Next walls in the museum
	{ 0x040080183, 39600259,   2855,  13400,   4305 },  // Next walls in the museum
	{ 0x004290188, 39580567,   7297,  27131,  12322 },  // Next walls in the museum
	{ 0x0283CE401, 12795150,  18499,  36658,  29166 },  // Late-game notes

	// Fixes for the Russian "Fargus" version
	{ 0x041137051,   758264,  29037,  49590,  49591 },	// "Options" menu header text
	{ 0x0c10b2015,   787304,   4414,  15848,  15853 },	// Text on option buttons
	{ 0x006802920,  1076824,   1010,   5642,   1546 },	// Crash in front of the Aqua House
	//
	{          0,        0,         0,     0,         0 }
};

void ResourceMan::loadResource(ResourceHandle &resourceHandle, bool applyResourceFixes) {
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
			BlbArchiveEntry *entry = resourceHandle._resourceFileEntry->archiveEntry;

			// Apply fixes for broken resources in Russian versions
			if (applyResourceFixes) {
				for (const EntrySizeFix *cur = entrySizeFixes; cur->fileHash > 0; ++cur) {
					if (entry->fileHash == cur->fileHash && entry->offset == cur->offset &&
						entry->diskSize == cur->diskSize && entry->size == cur->size)
						entry->size = cur->fixedSize;
				}
			}

			resourceData->data = new byte[entry->size];
			resourceHandle._resourceFileEntry->archive->load(entry, resourceData->data, 0);
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
