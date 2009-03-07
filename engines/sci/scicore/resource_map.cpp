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

#include "sci/sci_memory.h"
#include "sci/scicore/resource.h"
#include "sci/tools.h"

#include "common/util.h"
#include "common/file.h"

namespace Sci {

#define SCI0_RESMAP_ENTRIES_SIZE 6
#define SCI1_RESMAP_ENTRIES_SIZE 6
#define SCI11_RESMAP_ENTRIES_SIZE 5

int ResourceManager::readResourceMapSCI0(ResourceSource *map) {
	Common::File file;
	Resource *res;
	ResourceType type;
	uint16 number, id;
	uint32 offset;

	if (!file.open(map->location_name))
		return SCI_ERROR_RESMAP_NOT_FOUND;

	file.seek(0, SEEK_SET);

	byte bMask = _mapVersion == SCI_VERSION_01_VGA_ODD ? 0xF0: 0xFC;
	byte bShift = _mapVersion == SCI_VERSION_01_VGA_ODD ? 28 : 26;

	do {
		id = file.readUint16LE();
		offset = file.readUint32LE();

		if (file.ioFailed()) {
			warning("Error while reading %s: ", map->location_name.c_str());
			perror("");
			return SCI_ERROR_RESMAP_NOT_FOUND;
		}
		if(offset == 0xFFFFFFFF)
			break;

		type = (ResourceType)(id >> 11);
		number = id & 0x7FF;
		uint32 resId = RESOURCE_HASH(type, number);
		// adding a new resource
		if (_resMap.contains(resId) == false) {
			res = new Resource;
			res->id = id;
			res->file_offset = offset & (((~bMask) << 24) | 0xFFFFFF);
			res->number = number;
			res->type = type;
			res->source = getVolume(map, offset >> bShift);
			_resMap.setVal(resId, res);
		}
	} while (!file.eos());
	return 0;
}

int ResourceManager::readResourceMapSCI1(ResourceSource *map, ResourceSource *vol) {
	Common::File file;
	Resource *res;
	if (!file.open(map->location_name))
		return SCI_ERROR_RESMAP_NOT_FOUND;

	resource_index_t resMap[kResourceTypeInvalid];
	memset(resMap, 0, sizeof(resource_index_t) * kResourceTypeInvalid);
	byte type = 0, prevtype = 0;
	byte nEntrySize = _mapVersion == SCI_VERSION_1_1 ? SCI11_RESMAP_ENTRIES_SIZE : SCI1_RESMAP_ENTRIES_SIZE;
	uint32 resId;

	// Read resource type and offsets to resource offsets block from .MAP file
	// The last entry has type=0xFF (0x1F) and offset equals to map file length
	do {
		type = file.readByte() & 0x1F;
		resMap[type].wOffset = file.readUint16LE();
		resMap[prevtype].wSize = (resMap[type].wOffset
				- resMap[prevtype].wOffset) / nEntrySize;
		prevtype = type;
	} while (type != 0x1F); // the last entry is FF

	// reading each type's offsets
	uint32 off = 0;
	for (type = 0; type < kResourceTypeInvalid; type++) {
		if (resMap[type].wOffset == 0) // this resource does not exist in map
			continue;
		file.seek(resMap[type].wOffset);
		for (int i = 0; i < resMap[type].wSize; i++) {
			uint16 number = file.readUint16LE();
			file.read(&off, nEntrySize - 2);
			if (file.ioFailed()) {
				warning("Error while reading %s: ", map->location_name.c_str());
				perror("");
				return SCI_ERROR_RESMAP_NOT_FOUND;
			}
			resId = RESOURCE_HASH(type, number);
			// adding new resource only if it does not exist
			if (_resMap.contains(resId) == false) {
				res = new Resource;
				_resMap.setVal(resId, res);
				res->type = (ResourceType)type;
				res->number = number;
				res->id = res->number | (res->type << 16);
				res->source = _mapVersion < SCI_VERSION_1_1 ? getVolume(map, off  >> 28) : vol;
				res->file_offset = _mapVersion < SCI_VERSION_1_1 ? off & 0x0FFFFFFF : off << 1; 
			}
		}
	}
	return 0;
}

} // End of namespace Sci
