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

#ifndef SAGA_RESOURCE_H
#define SAGA_RESOURCE_H

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
	uint32 id;		// SAGA2
	size_t offset;
	size_t size;
	PatchData *patchData;

	bool isExternal() { return ((offset & (1L<<31)) != 0L); }	// SAGA2
};

struct ResourceContext {
	const char *fileName;
	uint16 fileType;
	Common::File *file;
	int32 fileSize;
	int serial;			// IHNM speech files

	bool isCompressed;
	bool isBigEndian;
	ResourceData *table;
	size_t count;
	ResourceData *categories;		// SAGA2

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

	// SAGA 2
	int32 getEntryNum(uint32 id) {
		for (int32 i = 0; i < (int32)count; i++) {
			if (table[i].id == id) {
				return i;
			}
		}
		return -1;
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
	virtual ~Resource();
	bool createContexts();
	void clearContexts();
	void loadResource(ResourceContext *context, uint32 resourceId, byte*&resourceBuffer, size_t &resourceSize);

	virtual uint32 convertResourceId(uint32 resourceId) = 0;
	virtual void loadGlobalResources(int chapter, int actorsEntrance) = 0;

	ResourceContext *getContext(uint16 fileType, int serial = 0) {
		for (int i = 0; i < _contextsCount; i++) {
			if ((_contexts[i].fileType & fileType) && _contexts[i].serial == serial) {
				return &_contexts[i];
			}
		}
		return NULL;
	}

protected:
	SagaEngine *_vm;
	ResourceContext *_contexts;
	int _contextsCount;
	char _voicesFileName[8][256];
	char _musicFileName[256];
	char _soundFileName[256];

	bool loadContext(ResourceContext *context);
	virtual bool loadMacContext(ResourceContext *context) = 0;
	virtual bool loadResContext(ResourceContext *context, uint32 contextOffset, uint32 contextSize) = 0;
	bool loadResContext_v1(ResourceContext *context, uint32 contextOffset, uint32 contextSize);
public:
	virtual MetaResource* getMetaResource() = 0;
};

// ITE
class Resource_RSC : public Resource {
public:
	Resource_RSC(SagaEngine *vm) : Resource(vm) {}
	virtual uint32 convertResourceId(uint32 resourceId);
	virtual void loadGlobalResources(int chapter, int actorsEntrance) {}
	virtual MetaResource* getMetaResource() {
		MetaResource *dummy = 0;
		return dummy;
	}
private:
	virtual bool loadMacContext(ResourceContext *context);
	virtual bool loadResContext(ResourceContext *context, uint32 contextOffset, uint32 contextSize) {
		return loadResContext_v1(context, contextOffset, contextSize);
	}
};

#ifdef ENABLE_IHNM
// IHNM
class Resource_RES : public Resource {
public:
	Resource_RES(SagaEngine *vm) : Resource(vm) {}
	virtual uint32 convertResourceId(uint32 resourceId) { return resourceId; }
	virtual void loadGlobalResources(int chapter, int actorsEntrance);
	virtual MetaResource* getMetaResource() { return &_metaResource; };
private:
	virtual bool loadMacContext(ResourceContext *context) { return false; }
	virtual bool loadResContext(ResourceContext *context, uint32 contextOffset, uint32 contextSize) {
		return loadResContext_v1(context, 0, contextSize);
	}
	MetaResource _metaResource;
};
#endif

#ifdef ENABLE_SAGA2
// DINO, FTA2
class Resource_HRS : public Resource {
public:
	Resource_HRS(SagaEngine *vm) : Resource(vm) {}
	virtual uint32 convertResourceId(uint32 resourceId) { return resourceId; }
	virtual void loadGlobalResources(int chapter, int actorsEntrance) {}
	virtual MetaResource* getMetaResource() {
		MetaResource *dummy = 0;
		return dummy;
	}
private:
	virtual bool loadMacContext(ResourceContext *context) { return false; }
	virtual bool loadResContext(ResourceContext *context, uint32 contextOffset, uint32 contextSize) {
		return loadResContext_v2(context, contextSize);
	}
	bool loadResContext_v2(ResourceContext *context, uint32 contextSize);
};
#endif

} // End of namespace Saga

#endif
