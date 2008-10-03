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

// RSC Resource file management header file

#ifndef SAGA_RSCFILE_H
#define SAGA_RSCFILE_H

#include "common/file.h"

namespace Saga {

#define MAC_BINARY_HEADER_SIZE 128
#define RSC_TABLEINFO_SIZE 8
#define RSC_TABLEENTRY_SIZE 8

#define RSC_MIN_FILESIZE (RSC_TABLEINFO_SIZE + RSC_TABLEENTRY_SIZE + 1)

struct PatchData {
	bool _deletePatchFile;
	Common::File *_patchFile;
	const GamePatchDescription *_patchDescription;

	PatchData(const GamePatchDescription *patchDescription): _patchDescription(patchDescription), _deletePatchFile(true) {
		_patchFile = new Common::File();
	}
	PatchData(Common::File *patchFile): _patchDescription(NULL), _patchFile(patchFile), _deletePatchFile(false) {
	}

	~PatchData() {
		if (_deletePatchFile) {
			delete _patchFile;
		}
	}
};

struct ResourceData {
	size_t offset;
	size_t size;
	PatchData *patchData;
	void fillSoundPatch(const GameSoundInfo *&soundInfo) {
		if (patchData != NULL) {
			if (patchData->_patchDescription != NULL) {
				if (patchData->_patchDescription->soundInfo != NULL) {
					soundInfo = patchData->_patchDescription->soundInfo;
				}
			}
		}
	}
};

struct ResourceContext {
	const char *fileName;
	uint16 fileType;
	Common::File *file;
	int serial;

	bool isBigEndian;
	ResourceData *table;
	size_t count;

	Common::File *getFile(ResourceData *resourceData) const {
		if (resourceData->patchData != NULL) {
			if (!resourceData->patchData->_patchFile->isOpen())
				resourceData->patchData->_patchFile->open(resourceData->patchData->_patchDescription->fileName);
			return resourceData->patchData->_patchFile;
		} else {
			return file;
		}
	}

	bool validResourceId(uint32 resourceId) const {
		return (resourceId < count);
	}

	ResourceData *getResourceData(uint32 resourceId) const {
		if (resourceId >= count) {
			error("ResourceContext::getResourceData() wrong resourceId %d", resourceId);
		}
		return &table[resourceId];
	}

};

struct MetaResource {
	int16 sceneIndex;
	int16 objectCount;
	int32 objectsStringsResourceID;
	int32 inventorySpritesID;
	int32 mainSpritesID;
	int32 objectsResourceID;
	int16 actorCount;
	int32 actorsStringsResourceID;
	int32 actorsResourceID;
	int32 protagFaceSpritesID;
	int32 field_22;
	int16 field_26;
	int16 protagStatesCount;
	int32 protagStatesResourceID;
	int32 cutawayListResourceID;
	int32 songTableID;

	MetaResource() {
		memset(this, 0, sizeof(*this));
	}
};

class Resource {
public:
	Resource(SagaEngine *vm);
	~Resource();
	bool createContexts();
	void clearContexts();
	void loadResource(ResourceContext *context, uint32 resourceId, byte*&resourceBuffer, size_t &resourceSize);
	uint32 convertResourceId(uint32 resourceId);

	void loadGlobalResources(int chapter, int actorsEntrance);

	ResourceContext *getContext(uint16 fileType, int serial = 0) {
		int i;
		for (i = 0; i < _contextsCount; i++) {
			if ((_contexts[i].fileType & fileType) && _contexts[i].serial == serial) {
				return &_contexts[i];
			}
		}
		return NULL;
	}

private:
	SagaEngine *_vm;
	ResourceContext *_contexts;
	int _contextsCount;
	char _voicesFileName[8][256];

	bool loadContext(ResourceContext *context);
	bool loadMacContext(ResourceContext *context);
	bool loadSagaContext(ResourceContext *context, uint32 contextOffset, uint32 contextSize);

public:
	MetaResource _metaResource;
};

} // End of namespace Saga

#endif
