/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

// RSC Resource file management header file

#ifndef SAGA_RESOURCE_H
#define SAGA_RESOURCE_H

#include "common/array.h"
#include "common/file.h"
#include "common/list.h"

namespace Saga {

#define MAC_BINARY_HEADER_SIZE 128
#define RSC_TABLEINFO_SIZE 8
#define RSC_TABLEENTRY_SIZE 8

#define RSC_MIN_FILESIZE (RSC_TABLEINFO_SIZE + RSC_TABLEENTRY_SIZE + 1)

class SagaEngine;
class ByteArray;

struct PatchData {
	Common::File *_patchFile;
	const char *_fileName;
	bool _deletePatchFile;

	PatchData(const char *fileName): _fileName(fileName), _deletePatchFile(true) {
		_patchFile = new Common::File();
	}
	PatchData(Common::File *patchFile, const char *fileName): _patchFile(patchFile), _fileName(fileName), _deletePatchFile(false) {
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
	int diskNum;
	PatchData *patchData;

	ResourceData() :
		offset(0), size(0), patchData(NULL), diskNum(-1) {
	}

	~ResourceData() {
		if (patchData) {
			delete patchData;
			patchData = NULL;
		}
	}
};

typedef Common::Array<ResourceData> ResourceDataArray;

class ResourceContext {
friend class Resource;
public:

	ResourceContext():
		_fileName(NULL), _fileType(0), _isCompressed(false), _serial(0),
		_isBigEndian(false),
		_fileSize(0) {
	}

	virtual ~ResourceContext() { }

	bool isCompressed() const {	return _isCompressed; }
	uint16 fileType() const { return _fileType; }
	int32 fileSize() const { return _fileSize; }
	int serial() const { return _serial; }
	bool isBigEndian() const { return _isBigEndian; }
	const char * fileName() const {	return _fileName; }

	Common::File *getFile(ResourceData *resourceData) {
		Common::File *file;
		const char * fn;
		if (resourceData && resourceData->patchData != NULL) {
			file = resourceData->patchData->_patchFile;
			fn = resourceData->patchData->_fileName;
		} else {
			file = &_file;
			fn = _fileName;
		}
		if (!file->isOpen())
			file->open(fn);
		return file;
	}

	bool validResourceId(uint32 resourceId) const {
		return (resourceId < _table.size());
	}

	ResourceData *getResourceData(uint32 resourceId) {
		if (resourceId >= _table.size()) {
			error("ResourceContext::getResourceData() wrong resourceId %d", resourceId);
		}
		return &_table[resourceId];
	}

protected:
	const char *_fileName;
	uint16 _fileType;
	bool _isCompressed;
	int _serial;					// IHNM speech files

	bool _isBigEndian;
	ResourceDataArray _table;
	Common::File _file;
	int32 _fileSize;

	bool load(SagaEngine *_vm, Resource *resource);
	bool loadResV1(uint32 contextOffset, uint32 contextSize);
	bool loadResIteAmiga(uint32 contextOffset, uint32 contextSize, int type);

	virtual bool loadMacMIDI() { return false; }
	virtual bool loadRes(uint32 contextOffset, uint32 contextSize, int type) = 0;
	virtual void processPatches(Resource *resource, const GamePatchDescription *patchFiles) { }
};

typedef Common::List<ResourceContext *> ResourceContextList;

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
	void loadResource(ResourceContext *context, uint32 resourceId, ByteArray &resourceBuffer);

	virtual uint32 convertResourceId(uint32 resourceId) = 0;
	virtual void loadGlobalResources(int chapter, int actorsEntrance) = 0;

	ResourceContext *getContext(uint16 fileType, int serial = 0);
	virtual MetaResource* getMetaResource() = 0;
protected:
	SagaEngine *_vm;
	ResourceContextList _contexts;
	char _voicesFileName[8][256];
	char _musicFileName[256];
	char _soundFileName[256];

	void addContext(const char *fileName, uint16 fileType, bool isCompressed = false, int serial = 0);
	virtual ResourceContext *createContext() = 0;
};

// ITE
class ResourceContext_RSC: public ResourceContext {
protected:
	bool loadMacMIDI() override;
	bool loadRes(uint32 contextOffset, uint32 contextSize, int type) override {
		return loadResV1(contextOffset, contextSize);
	}
	void processPatches(Resource *resource, const GamePatchDescription *patchFiles) override;
};

class ResourceContext_RSC_ITE_Amiga: public ResourceContext {
protected:
	bool loadRes(uint32 contextOffset, uint32 contextSize, int type) override {
		return loadResIteAmiga(contextOffset, contextSize, type);
	}
};

class Resource_RSC : public Resource {
public:
	Resource_RSC(SagaEngine *vm) : Resource(vm) {}
	uint32 convertResourceId(uint32 resourceId) override {
		return _vm->isMacResources() ? resourceId - 2 : resourceId;
	}
	void loadGlobalResources(int chapter, int actorsEntrance) override {}
	MetaResource* getMetaResource() override {
		MetaResource *dummy = 0;
		return dummy;
	}
protected:
	ResourceContext *createContext() override {
		if (_vm->getPlatform() == Common::kPlatformAmiga && _vm->getGameId() == GID_ITE)
			return new ResourceContext_RSC_ITE_Amiga();
		return new ResourceContext_RSC();
	}
};

#ifdef ENABLE_IHNM
// IHNM
class ResourceContext_RES: public ResourceContext {
protected:
	bool loadRes(uint32 contextOffset, uint32 contextSize, int type) override {
		return loadResV1(0, contextSize);
	}

	void processPatches(Resource *resource, const GamePatchDescription *patchFiles) override;
};

// TODO: move load routines from sndres
class VoiceResourceContext_RES: public ResourceContext {
protected:
	bool loadRes(uint32 contextOffset, uint32 contextSize, int type) override {
		return false;
	}
public:
	VoiceResourceContext_RES() : ResourceContext() {
		_fileType = GAME_VOICEFILE;
		_isBigEndian = true;
	}
};

class Resource_RES : public Resource {
public:
	Resource_RES(SagaEngine *vm) : Resource(vm) {}
	uint32 convertResourceId(uint32 resourceId) override { return resourceId; }
	void loadGlobalResources(int chapter, int actorsEntrance) override;
	MetaResource* getMetaResource() override { return &_metaResource; }
protected:
	ResourceContext *createContext() override {
		return new ResourceContext_RES();
	}
private:
	MetaResource _metaResource;
};
#endif

} // End of namespace Saga

#endif
