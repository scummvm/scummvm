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
 */

#ifndef ILLUSIONS_SCRIPTRESOURCE_H
#define ILLUSIONS_SCRIPTRESOURCE_H

#include "illusions/resourcesystem.h"
#include "common/file.h"

namespace Illusions {

class IllusionsEngine;

class ScriptResourceLoader : public BaseResourceLoader {
public:
	ScriptResourceLoader(IllusionsEngine *vm) : _vm(vm) {}
	~ScriptResourceLoader() override {}
	void load(Resource *resource) override;
	bool isFlag(int flag) override;
protected:
	IllusionsEngine *_vm;
};

class Properties {
public:
	Properties();
	void init(uint count, byte *properties);
	void clear();
	bool get(uint32 propertyId);
	void set(uint32 propertyId, bool value);
	uint32 getSize();
	void writeToStream(Common::WriteStream *out);
	bool readFromStream(Common::ReadStream *in);
public:
	uint _count;
	byte *_properties;
	void getProperyPos(uint32 propertyId, uint &index, byte &mask);
};

class BlockCounters {
public:
	BlockCounters();
	void init(uint count, byte *blockCounters);
	void clear();
	byte get(uint index);
	void set(uint index, byte value);
	byte getC0(uint index);
	void setC0(uint index, byte value);
	uint32 getSize();
	void writeToStream(Common::WriteStream *out);
	bool readFromStream(Common::ReadStream *in);
public:
	uint _count;
	byte *_blockCounters;
};

struct TriggerCause {
	uint32 _verbId;
	uint32 _objectId2;
	uint32 _codeOffs;
	void load(Common::SeekableReadStream &stream);
};

class TriggerObject {
public:
	TriggerObject();
	~TriggerObject();
	void load(byte *dataStart, Common::SeekableReadStream &stream);
	bool findTriggerCause(uint32 verbId, uint32 objectId2, uint32 &codeOffs);
	void fixupSceneInfosDuckman();
public:
	uint32 _objectId;
	uint _causesCount;
	TriggerCause *_causes;
};

class SceneInfo {
public:
	SceneInfo();
	~SceneInfo();
	void load(byte *dataStart, Common::SeekableReadStream &stream);
	bool findTriggerCause(uint32 verbId, uint32 objectId2, uint32 objectId, uint32 &codeOffs);
	void getResources(uint &resourcesCount, uint32 *&resources);
	void fixupSceneInfosDuckman();
protected:
	uint16 _id;
	uint16 _unk;
	uint16 *_name;
	uint _triggerObjectsCount;
	TriggerObject *_triggerObjects;
	uint _resourcesCount;
	uint32 *_resources;
	TriggerObject *findTriggerObject(uint32 objectId);
};

class ScriptResource {
public:
	ScriptResource();
	~ScriptResource();
	void load(Resource *resource);
	byte *getThreadCode(uint32 threadId);
	byte *getCode(uint32 codeOffs);
	SceneInfo *getSceneInfo(uint32 index);
	uint32 getObjectActorTypeId(uint32 objectId);
	uint32 getMainActorObjectId() const { return _mainActorObjectId; }
public:
	byte *_data;
	uint32 _dataSize;
	Properties _properties;
	BlockCounters _blockCounters;
	uint _codeCount;
	uint32 *_codeOffsets;
	uint _sceneInfosCount;
	SceneInfo *_sceneInfos;
	// Duckman specific
	uint32 _soundIds[27];
	uint _objectMapCount;
	uint32 *_objectMap;
	uint32 _mainActorObjectId;
	void fixupSceneInfosDuckman();
};

class ScriptInstance : public ResourceInstance {
public:
	ScriptInstance(IllusionsEngine *vm);
	void load(Resource *resource) override;
	void unload() override;
public:
	IllusionsEngine *_vm;
};

} // End of namespace Illusions

#endif // ILLUSIONS_ACTORRESOURCE_H
