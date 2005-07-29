/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

// RSC Resource file management module
#include "saga/saga.h"

#include "saga/rscfile.h"
#include "saga/stream.h"

namespace Saga {

Resource::Resource(SagaEngine *vm): _vm(vm) {
	_contexts = NULL;
	_contextsCount = 0;
}

Resource::~Resource() {
	clearContexts();
}

bool Resource::loadContext(ResourceContext *context) {
	size_t i;
	int j;
	bool result;
	byte tableInfo[RSC_TABLEINFO_SIZE];
	uint32 resourceTableOffset;
	GamePatchDescription *patchDescription;
	ResourceData *resourceData;
	byte *tableBuffer;
	size_t tableSize;

	if (!context->file->open(context->fileName)) {
		return false;
	}

	context->isBigEndian = _vm->isBigEndian();

	if (context->file->size() < RSC_MIN_FILESIZE) {
		return false;
	}

	// Read resource table info from the rear end of file
	context->file->seek((long)(-RSC_TABLEINFO_SIZE), SEEK_END);

	if (context->file->read(tableInfo, RSC_TABLEINFO_SIZE) != RSC_TABLEINFO_SIZE) {
		return false;
	}

	MemoryReadStreamEndian readS(tableInfo, RSC_TABLEINFO_SIZE, context->isBigEndian);

	resourceTableOffset = readS.readUint32();
	context->count = readS.readUint32();

	// Check for sane table offset
	if (resourceTableOffset != context->file->size() - RSC_TABLEINFO_SIZE - RSC_TABLEENTRY_SIZE * context->count) {
		return false;
	}

	// Load resource table
	tableSize = RSC_TABLEENTRY_SIZE * context->count;

	tableBuffer = (byte *)malloc(tableSize);

	context->file->seek((long)resourceTableOffset, SEEK_SET);

	result = (context->file->read(tableBuffer, tableSize) == tableSize);
	if (result) {
		context->table = (ResourceData *)calloc(context->count, sizeof(*context->table));

		MemoryReadStreamEndian readS1(tableBuffer, tableSize, context->isBigEndian);

		for (i = 0; i < context->count; i++) {
			resourceData = &context->table[i];
			resourceData->offset = readS1.readUint32();
			resourceData->size = readS1.readUint32();
			//sanity check
			if ((resourceData->offset > context->file->size()) || (resourceData->size > context->file->size())) {
				result = false;
				break;
			}
		}
	}

	free(tableBuffer);

	//process patch files
	if (result) {
		for (j = 0; j < _vm->getGameDescription()->patchsCount; j++) {
			patchDescription = &_vm->getGameDescription()->patchDescriptions[j];
			if ((patchDescription->fileType & context->fileType) != 0) {
				if (patchDescription->resourceId < context->count) {
					resourceData = &context->table[patchDescription->resourceId];
					resourceData->patchData = new PatchData(patchDescription);
					if (resourceData->patchData->_patchFile->open(patchDescription->fileName)) {
						resourceData->offset = 0;
						resourceData->size = resourceData->patchData->_patchFile->size();
					} else {
						delete resourceData->patchData;
						resourceData->patchData = NULL;
					}
				}
			}
		}
	}

	return result;
}

bool Resource::createContexts() {
	int i;
	ResourceContext *context;
	_contextsCount = _vm->getGameDescription()->filesCount;
	_contexts = (ResourceContext*)calloc(_contextsCount, sizeof(*_contexts));

	for (i = 0; i < _contextsCount; i++) {
		context = &_contexts[i];
		context->file = new Common::File();
		context->fileName = _vm->getGameDescription()->filesDescriptions[i].fileName;
		context->fileType = _vm->getGameDescription()->filesDescriptions[i].fileType;
		context->serial = 0;

		// IHNM has serveral different voice files, so we need to allow
		// multiple resource contexts of the same type. We tell them
		// apart by assigning each of the duplicates an unique serial
		// number. The default behaviour when requesting a context will
		// be to look for serial number 0.

		for (int j = i - 1; j >= 0; j--) {
			if (_contexts[j].fileType & context->fileType) {
				context->serial = _contexts[j].serial + 1;
				break;
			}
		}

		if (!loadContext(context)) {
			return false;
		}
	}
	return true;
}

void Resource::clearContexts() {
	int i;
	size_t j;
	ResourceContext *context;
	if (_contexts == NULL) {
		return;
	}
	for(i = 0; i < _contextsCount; i++) {
		context = &_contexts[i];
		delete context->file;
		if (context->table != NULL) {
			for(j = 0; j < context->count; j++) {
				delete context->table[j].patchData;
			}
		}
		free(context->table);
	}
	free(_contexts);
	_contexts = NULL;
}

uint32 Resource::convertResourceId(uint32 resourceId) {

	if ((_vm->getGameType() ==  GType_ITE) && (_vm->getFeatures() & GF_MAC_RESOURCES)) {
		if (resourceId > 1537) {
			return resourceId - 2;
		} else {
			if (resourceId == 1535 || resourceId == 1536) {
				error ("Wrong resource number %d for Mac ITE", resourceId);
			}
		}
	}

	return resourceId;
}

void Resource::loadResource(ResourceContext *context, uint32 resourceId, byte*&resourceBuffer, size_t &resourceSize) {
	Common::File *file;
	uint32 resourceOffset;
	ResourceData *resourceData;

	debug(8, "loadResource %d", resourceId);

	resourceData = getResourceData(context, resourceId);

	file = context->getFile(resourceData);

	resourceOffset = resourceData->offset;
	resourceSize = resourceData->size;

	resourceBuffer = (byte*)malloc(resourceSize);

	file->seek((long)resourceOffset, SEEK_SET);

	if (file->read(resourceBuffer, resourceSize) != resourceSize) {
		error("Resource::loadResource() failed to read");
	}
}

} // End of namespace Saga
