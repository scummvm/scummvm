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

// HRS Resource file management module (SAGA 2, used in DINO and FTA2)

#include "saga/saga.h"

#include "saga/actor.h"
#include "saga/animation.h"
#include "saga/interface.h"
#include "saga/music.h"
#include "saga/resource.h"
#include "saga/scene.h"
#include "saga/sndres.h"

#include "common/advancedDetector.h"
#include "common/endian.h"

namespace Saga {

bool Resource_HRS::loadResContext_v2(ResourceContext *context, uint32 contextSize) {
	ResourceData *origin = new ResourceData();
	uint32 firstEntryOffset;
	uint32 tableSize;
	int i, count;
	const uint32 resourceSize = 4 + 4 + 4;	// id, size, offset

	debug(3, "Context %s", context->fileName);
	context->file->seek(0, SEEK_SET);
	
	// Read head element (origin)
	origin->id = context->file->readUint32BE();		// this is BE on purpose
	origin->offset = context->file->readUint32LE();
	origin->size = context->file->readUint32LE();

	// Check if the file is valid
	if (origin->id != MKID_BE('HRES')) {	// header
		free(origin);
		return false;
	}

	// Read offset of first entry
	context->file->seek(origin->offset - sizeof(uint32), SEEK_SET);
	firstEntryOffset = context->file->readUint32LE();

	// Allocate buffers for table, categories and data
	context->categories = (ResourceData *) calloc(origin->size / resourceSize, sizeof(*context->categories));
	tableSize = origin->offset - firstEntryOffset - sizeof(uint32);
	context->table = (ResourceData *) calloc(tableSize / resourceSize, sizeof(*context->table));

	if (context->categories == NULL || context->table == NULL) {
		free(origin);
		return false;
	}

	// Read categories
	count = origin->size / resourceSize;
	for (i = 0; i < count; i++) {
		context->categories[i].id = context->file->readUint32BE();
		context->categories[i].offset = context->file->readUint32LE();
		context->categories[i].size = context->file->readUint32LE();
		debug(3, "Category entry: id %u, offset %u, size %u", context->categories[i].id, context->categories[i].offset, context->categories[i].size);
	}

	context->file->seek(firstEntryOffset, SEEK_SET);

	// Read table entries
	count = tableSize / resourceSize;
	for (i = 0; i < count; i++) {
		context->table[i].id = context->file->readUint32BE();
		context->table[i].offset = context->file->readUint32LE();
		context->table[i].size = context->file->readUint32LE();
		debug(3, "Table entry: id %u, offset %u, size %u", context->table[i].id, context->table[i].offset, context->table[i].size);
	}

	context->count = tableSize / resourceSize;

	free(origin);
	return true;
}

} // End of namespace Saga
