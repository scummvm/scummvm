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
	uint32 firstGroupOffset;
	uint32 size;
	int i, count;

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

	// Read first group offset
	context->file->seek(origin->offset - sizeof(uint32), SEEK_SET);
	firstGroupOffset = context->file->readUint32LE();

	// Allocate buffers for root/base node, groups and data
	context->base = (ResourceData *) calloc(origin->size, sizeof(*context->base));
	size = origin->offset - firstGroupOffset - sizeof(uint32);
	context->groups = (ResourceData *) calloc(size / sizeof(*context->groups), sizeof(*context->groups));

	if (context->base == NULL || context->groups == NULL) {
		free(origin);
		return false;
	}

	// Read base
	count = origin->size / sizeof(*context->base);
	for (i = 0; i < count; i++) {
		context->base[i].id = context->file->readUint32LE();
		context->base[i].offset = context->file->readUint32LE();
		context->base[i].size = context->file->readUint32LE();
	}

	context->file->seek(firstGroupOffset, SEEK_SET);

	// Read groups
	count = size / sizeof(*context->groups);
	for (i = 0; i < count; i++) {
		context->groups[i].id = context->file->readUint32LE();
		context->groups[i].offset = context->file->readUint32LE();
		context->groups[i].size = context->file->readUint32LE();
	}

	context->count = origin->size / sizeof(*origin);

	free(origin);
	return true;
}

} // End of namespace Saga
