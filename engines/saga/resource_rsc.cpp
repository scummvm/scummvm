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

// RSC Resource file management module (SAGA 1, used in ITE)

#include "saga/saga.h"

#include "saga/actor.h"
#include "saga/animation.h"
#include "saga/interface.h"
#include "saga/music.h"
#include "saga/resource.h"
#include "saga/scene.h"
#include "saga/sndres.h"

#include "engines/advancedDetector.h"

namespace Saga {

struct MacResMap {
	int16 resAttr;
	int16 typeOffset;
	int16 nameOffset;
	int16 numTypes;
};

struct MacResource {
	int16 id;
	int16 nameOffset;
	byte attr;
	int32 dataOffset;
	byte name[255];
};

struct MacResType {
	uint32 id;
	int16 items;
	int16 maxItemId;
	int16 offset;
	MacResource *resources;
};

#define ID_MIDI     MKID_BE('Midi')

uint32 Resource_RSC::convertResourceId(uint32 resourceId) {

	if (_vm->isMacResources()) {
		if (resourceId > 1537) {
			return resourceId - 2;
		} else {
			if (resourceId == 1535 || resourceId == 1536) {
				error("Wrong resource number %d for Mac ITE", resourceId);
			}
		}
	}

	return resourceId;
}

bool Resource_RSC::loadMacContext(ResourceContext *context) {
	int32 macDataSize, macDataSizePad;
	int32 macResSize, macResSizePad;
	int32 macResOffset;

	uint32 macMapLength;
	uint32 macDataLength;
	uint32 macMapOffset;
	uint32 macDataOffset;

	MacResMap macResMap;
	MacResType *macResTypes;

	MacResType *macResType;
	MacResource *macResource;
	int i, j;
	byte macNameLen;
	bool notSagaContext = false;

	if (context->fileSize < RSC_MIN_FILESIZE + MAC_BINARY_HEADER_SIZE) {
		return false;
	}

	if (context->file->readByte() != 0) {
		return false;
	}
	context->file->readByte(); //MAX Name Len
	context->file->seek(74);
	if (context->file->readByte() != 0) {
		return false;
	}
	context->file->seek(82);
	if (context->file->readByte() != 0) {
		return false;
	}

	macDataSize = context->file->readSint32BE();
	macResSize = context->file->readSint32BE();
	macDataSizePad = (((macDataSize + 127) >> 7) << 7);
	macResSizePad = (((macResSize + 127) >> 7) << 7);

	macResOffset = MAC_BINARY_HEADER_SIZE + macDataSizePad;
	context->file->seek(macResOffset);

	macDataOffset = context->file->readUint32BE() + macResOffset;
	macMapOffset = context->file->readUint32BE() + macResOffset;
	macDataLength = context->file->readUint32BE();
	macMapLength = context->file->readUint32BE();

	if (macDataOffset >= (uint)context->fileSize || macMapOffset >= (uint)context->fileSize ||
		macDataLength + macMapLength > (uint)context->fileSize) {
			return false;
	}

	context->file->seek(macMapOffset + 22);

	macResMap.resAttr = context->file->readUint16BE();
	macResMap.typeOffset = context->file->readUint16BE();
	macResMap.nameOffset = context->file->readUint16BE();
	macResMap.numTypes = context->file->readUint16BE();
	macResMap.numTypes++;

	context->file->seek(macMapOffset + macResMap.typeOffset + 2);

	macResTypes = (MacResType *)calloc(macResMap.numTypes, sizeof(*macResTypes));

	for (i = macResMap.numTypes, macResType = macResTypes; i > 0; i--, macResType++) {
		macResType->id = context->file->readUint32BE();
		macResType->items = context->file->readUint16BE();
		macResType->offset = context->file->readUint16BE();
		macResType->items++;
		macResType->resources = (MacResource*)calloc(macResType->items, sizeof(*macResType->resources));
	}

	for (i = macResMap.numTypes, macResType = macResTypes; i > 0; i--, macResType++) {
		context->file->seek(macResType->offset + macMapOffset + macResMap.typeOffset);

		for (j = macResType->items, macResource = macResType->resources; j > 0; j--, macResource++) {
			macResource->id = context->file->readUint16BE();
			macResource->nameOffset = context->file->readUint16BE();
			macResource->dataOffset = context->file->readUint32BE();
			macResSize = context->file->readUint32BE();

			macResource->attr = macResource->dataOffset >> 24;
			macResource->dataOffset &= 0xFFFFFF;
			if (macResource->id > macResType->maxItemId) {
				macResType->maxItemId = macResource->id;
			}
		}

		for (j = macResType->items, macResource = macResType->resources; j > 0; j--, macResource++) {
			if (macResource->nameOffset != -1) {
				context->file->seek(macResource->nameOffset + macMapOffset + macResMap.nameOffset);
				macNameLen = context->file->readByte();
				context->file->read(macResource->name, macNameLen);
			}
		}
	}

//
	for (i = macResMap.numTypes, macResType = macResTypes; i > 0; i--, macResType++) {
		//getting offsets & sizes of midi
		if (((context->fileType & GAME_MUSICFILE_GM) > 0) && (macResType->id == ID_MIDI)) {

			context->count = macResType->maxItemId + 1;
			context->table = (ResourceData *)calloc(context->count, sizeof(*context->table));
			for (j = macResType->items, macResource = macResType->resources; j > 0; j--, macResource++) {
				context->file->seek(macDataOffset + macResource->dataOffset);
				context->table[macResource->id].size = context->file->readUint32BE();
				context->table[macResource->id].offset = context->file->pos();
			}
			notSagaContext = true;
			break;
		}
	}

//free
	for (i = 0; i < macResMap.numTypes; i++) {
		free(macResTypes[i].resources);
	}
	free(macResTypes);

	if ((!notSagaContext) && (!loadResContext(context, MAC_BINARY_HEADER_SIZE, macDataSize))) {
		return false;
	}

	return true;
}


} // End of namespace Saga
