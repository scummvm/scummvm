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

#ifdef ENABLE_SAGA2

// HRS Resource file management module (SAGA 2, used in DINO and FTA2)

#include "saga/saga.h"

#include "saga/actor.h"
#include "saga/animation.h"
#include "saga/interface.h"
#include "saga/music.h"
#include "saga/resource.h"
#include "saga/scene.h"
#include "saga/sndres.h"

#include "common/endian.h"

namespace Saga {

void ResourceContext_HRS::readCategory(ResourceData &element) {
	element.id = _file.readUint32BE();
	element.offset = _file.readUint32LE();
	element.size = _file.readUint32LE();
	element.category = 0;
	debug(3, "Category: id %u, offset %u, size %u", element.id, (uint)element.offset, (uint)element.size);
}

void ResourceContext_HRS::readEntry(ResourceData &element) {
	element.id = _file.readUint32BE();
	element.offset = _file.readUint32LE();
	element.size = _file.readUint32LE();
	element.category = getCategory(_file.pos());
	debug(3, "Entry: id %u, offset %u, size %u", element.id, (uint)element.offset, (uint)element.size);
}

uint32 ResourceContext_HRS::getCategory(uint32 resourceOffset) {
	for (int i = _categories.size() - 1; i >= 0; --i) {
		if (resourceOffset >= _categories[i].offset)
			return _categories[i].id;
	}

	error("Unknown category for offset %d", resourceOffset);
}

static bool categorySortHelper(const ResourceData &r1, const ResourceData &r2) {
	return r1.offset < r2.offset;
}

bool ResourceContext_HRS::loadResV2(uint32 contextSize) {
	ResourceData origin;
	uint32 firstEntryOffset;
	uint32 tableSize;
	int i, count;
	const uint32 resourceSize = 4 + 4 + 4;	// id, size, offset

	debug(3, "Context %s =====", _fileName);
	_file.seek(0, SEEK_SET);

	readCategory(origin);

	// Check if the file is valid
	if (origin.id != MKTAG('H','R','E','S')) {	// header
		return false;
	}

	// Read offset of first entry
	_file.seek(origin.offset - sizeof(uint32), SEEK_SET);
	firstEntryOffset = _file.readUint32LE();

	// Allocate buffers for table, categories and data
	_categories.resize(origin.size / resourceSize);
	tableSize = origin.offset - firstEntryOffset - sizeof(uint32);
	_table.resize(tableSize / resourceSize);

	// Read categories
	count = origin.size / resourceSize;
	debug(3, "File: %s, categories: %d =====", _file.getName(), count);
	for (i = 0; i < count; i++) {
		readCategory(_categories[i]);
		//uint32 id = _categories[i].id;
		//debug("%i: %c%c%c%c, offset: %d, size: %d", i, (id >> 24),  (id >> 16) & 0xFF, (id >> 8) & 0xFF, id & 0xFF, _categories[i].offset, _categories[i].size);
	}

	Common::sort(_categories.begin(), _categories.end(), categorySortHelper);

	_file.seek(firstEntryOffset, SEEK_SET);

	// Read table entries
	count = tableSize / resourceSize;
	debug(3, "File: %s, entries: %d =====", _file.getName(), count);
	for (i = 0; i < count; i++) {
		readEntry(_table[i]);
		//debug("%i: offset: %d, size: %d", i, _table[i].offset, _table[i].size);
	}
	return true;
}

} // End of namespace Saga

#endif
