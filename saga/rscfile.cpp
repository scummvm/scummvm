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

struct MacResMap {
	int16 resAttr;
	int16 typeOffset;
	int16 nameOffset;
	int16 numTypes;
};

struct MacResource {
	int16 id;
	int16 nameOffset;
	byte  attr;
	int32 dataOffset;
	byte name[255];
};

struct MacResType {
	uint32  id;
	int16 items;
	int16 maxItemId;
	int16 offset;
	MacResource *resources;
};


#define ID_MIDI     MKID_BE('Midi')

Resource::Resource(SagaEngine *vm): _vm(vm) {
	_contexts = NULL;
	_contextsCount = 0;
}

Resource::~Resource() {
	clearContexts();
}

bool Resource::loadSagaContext(ResourceContext *context, uint32 contextOffset, uint32 contextSize) {
	size_t i;
	bool result;
	byte tableInfo[RSC_TABLEINFO_SIZE];
	byte *tableBuffer;
	size_t tableSize;
	uint32 resourceTableOffset;
	ResourceData *resourceData;

	if (contextSize < RSC_MIN_FILESIZE) {
		return false;
	}

	context->file->seek(contextOffset + contextSize - RSC_TABLEINFO_SIZE);

	if (context->file->read(tableInfo, RSC_TABLEINFO_SIZE) != RSC_TABLEINFO_SIZE) {
		return false;
	}

	MemoryReadStreamEndian readS(tableInfo, RSC_TABLEINFO_SIZE, context->isBigEndian);

	resourceTableOffset = readS.readUint32();
	context->count = readS.readUint32();

	// Check for sane table offset
	if (resourceTableOffset != contextSize - RSC_TABLEINFO_SIZE - RSC_TABLEENTRY_SIZE * context->count) {
		return false;
	}

	// Load resource table
	tableSize = RSC_TABLEENTRY_SIZE * context->count;

	tableBuffer = (byte *)malloc(tableSize);

	context->file->seek(resourceTableOffset + contextOffset, SEEK_SET);

	result = (context->file->read(tableBuffer, tableSize) == tableSize);
	if (result) {
		context->table = (ResourceData *)calloc(context->count, sizeof(*context->table));

		MemoryReadStreamEndian readS1(tableBuffer, tableSize, context->isBigEndian);

		for (i = 0; i < context->count; i++) {
			resourceData = &context->table[i];
			resourceData->offset = contextOffset + readS1.readUint32();
			resourceData->size = readS1.readUint32();
			//sanity check
			if ((resourceData->offset > context->file->size()) || (resourceData->size > contextSize)) {
				result = false;
				break;
			}
		}
	}

	free(tableBuffer);
	return result;
}

bool Resource::loadMacContext(ResourceContext *context) {
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

	if (context->file->size() < RSC_MIN_FILESIZE + MAC_BINARY_HEADER_SIZE) {
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

	if (macDataOffset >= context->file->size() || macMapOffset >= context->file->size() ||
		macDataLength + macMapLength  > context->file->size()) {
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

	if ((!notSagaContext) && (!loadSagaContext(context, MAC_BINARY_HEADER_SIZE, macDataSize))) {
		return false;
	}

	return true;
}

bool Resource::loadContext(ResourceContext *context) {
	size_t i;
	int j;
	GamePatchDescription *patchDescription;
	ResourceData *resourceData;
	uint16 subjectResourceType;
	ResourceContext *subjectContext;
	uint32 subjectResourceId;
	uint32 patchResourceId;
	ResourceData *subjectResourceData;
	byte *tableBuffer;
	size_t tableSize;
	bool isMacBinary;
	
	if (!context->file->open(context->fileName)) {
		return false;
	}

	context->isBigEndian = _vm->isBigEndian();
	
	isMacBinary = (context->fileType & GAME_MACBINARY) > 0;
	context->fileType &= ~GAME_MACBINARY;
	
	if (isMacBinary) {
		if (!loadMacContext(context)) {
			return false;
		}
	} else {
		if (!loadSagaContext(context, 0, context->file->size())) {
			return false;
		}
	}
	

	//process internal patch files
	if (GAME_PATCHFILE & context->fileType) {
		subjectResourceType = ~GAME_PATCHFILE & context->fileType;
		subjectContext = getContext((GameFileTypes)subjectResourceType);
		if (subjectContext == NULL) {
			error("Resource::loadContext() Subject context not found");
		}
		loadResource(context, context->count - 1, tableBuffer, tableSize);

		MemoryReadStreamEndian readS2(tableBuffer, tableSize, context->isBigEndian);
		for (i = 0; i < tableSize / 8; i++) {
			subjectResourceId = readS2.readUint32();
			patchResourceId = readS2.readUint32();
			subjectResourceData = getResourceData(subjectContext, subjectResourceId);
			resourceData = getResourceData(context, patchResourceId);
			subjectResourceData->patchData = new PatchData(context->file);
			subjectResourceData->offset = resourceData->offset;
			subjectResourceData->size = resourceData->size;
		}

	}

	//process external patch files
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
	

	return true;
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

static int metaResourceTable[] = { 0, 326, 517, 677, 805, 968, 1165, 0, 1271 };

void Resource::loadGlobalResources(int chapter, int actorsEntrance) {
	// TODO
	//if (module.voiceLUT)
	//	free module.voiceLUT;

	// TODO: close chapeter context, or rather reassign it in our case

	ResourceContext *resourceContext;

	resourceContext = _vm->_resource->getContext(GAME_RESOURCEFILE);
	if (resourceContext == NULL) {
		error("Resource::loadGlobalResources() resource context not found");
	}

	byte *resourcePointer;
	size_t resourceLength;

	_vm->_resource->loadResource(resourceContext, metaResourceTable[chapter],
								 resourcePointer, resourceLength);

	MemoryReadStream metaS(resourcePointer, resourceLength);

	_metaResource.sceneIndex = metaS.readSint16LE();
	_metaResource.obectCount = metaS.readSint16LE();
	_metaResource.field_4 = metaS.readSint32LE();
	_metaResource.field_8 = metaS.readSint32LE();
	_metaResource.mainSpritesID = metaS.readSint32LE();
	_metaResource.objectResourceID = metaS.readSint32LE();
	_metaResource.actorCount = metaS.readSint16LE();
	_metaResource.field_16 = metaS.readSint32LE();
	_metaResource.actorsResourceID = metaS.readSint32LE();
	_metaResource.protagFaceSpritesID = metaS.readSint32LE();
	_metaResource.field_22 = metaS.readSint32LE();
	_metaResource.field_26 = metaS.readSint16LE();
	_metaResource.field_28 = metaS.readSint16LE();
	_metaResource.field_2a = metaS.readSint32LE();
	_metaResource.cutawayListResourceID = metaS.readSint32LE();
	_metaResource.songTableID = metaS.readSint32LE();

	free(resourcePointer);
}

} // End of namespace Saga
