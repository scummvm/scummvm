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

#include "illusions/illusions.h"
#include "illusions/resources/scriptresource.h"

namespace Illusions {

// ScriptResourceLoader

void ScriptResourceLoader::load(Resource *resource) {
	ScriptInstance *scriptInstance = new ScriptInstance(_vm);
	scriptInstance->load(resource);
	resource->_instance = scriptInstance;
}

bool ScriptResourceLoader::isFlag(int flag) {
	return
		flag == kRlfLoadFile;
}

// Properties

Properties::Properties()
	: _count(0), _properties(0) {
}

void Properties::init(uint count, byte *properties) {
	_count = count;
	_properties = properties;
}

void Properties::clear() {
	uint32 size = getSize();
	for (uint32 i = 0; i < size; ++i) {
		_properties[i] = 0;
	}
}

bool Properties::get(uint32 propertyId) {
	uint index;
	byte mask;
	getProperyPos(propertyId, index, mask);
	return (_properties[index] & mask) != 0;
}

void Properties::set(uint32 propertyId, bool value) {
	uint index;
	byte mask;
	getProperyPos(propertyId, index, mask);
	if (value)
		_properties[index] |= mask;
	else
		_properties[index] &= ~mask;
}

uint32 Properties::getSize() {
	return (_count >> 3) + 1;
}

void Properties::writeToStream(Common::WriteStream *out) {
	const uint32 size = getSize();
	out->writeUint32LE(size);
	out->write(_properties, size);
}

bool Properties::readFromStream(Common::ReadStream *in) {
	uint32 size = in->readUint32LE();
	if (size != getSize())
		return false;
	in->read(_properties, size);
	return true;
}

void Properties::getProperyPos(uint32 propertyId, uint &index, byte &mask) {
	propertyId &= 0xFFFF;
	index = propertyId >> 3;
	mask = 1 << (propertyId & 7);
}

// BlockCounters

BlockCounters::BlockCounters()
	: _count(0), _blockCounters(0) {
}

void BlockCounters::init(uint count, byte *blockCounters) {
	_count = count;
	_blockCounters = blockCounters;
}

void BlockCounters::clear() {
	for (uint i = 0; i < _count; ++i) {
		_blockCounters[i] = 0;
	}
}

byte BlockCounters::get(uint index) {
	return _blockCounters[index - 1] & 0x3F;
}

void BlockCounters::set(uint index, byte value) {
	_blockCounters[index - 1] ^= (_blockCounters[index - 1] ^ value) & 0x3F;
}

byte BlockCounters::getC0(uint index) {
	return _blockCounters[index - 1] & 0xC0;
}

void BlockCounters::setC0(uint index, byte value) {
	byte oldValue = _blockCounters[index - 1] & 0x3F;
	if (value & 0x80)
		value = value & 0xBF;
	_blockCounters[index - 1] = oldValue | (value & 0xC0);
}

uint32 BlockCounters::getSize() {
	return _count;
}

void BlockCounters::writeToStream(Common::WriteStream *out) {
	const uint32 size = getSize();
	out->writeUint32LE(size);
	out->write(_blockCounters, size);
}

bool BlockCounters::readFromStream(Common::ReadStream *in) {
	uint32 size = in->readUint32LE();
	if (size != getSize())
		return false;
	in->read(_blockCounters, size);
	return true;
}

// TriggerCause

void TriggerCause::load(Common::SeekableReadStream &stream) {
	_verbId = stream.readUint32LE();
	_objectId2 = stream.readUint32LE();
	_codeOffs = stream.readUint32LE();

	debug(2, "TriggerCause::load() _verbId: %08X; _objectId2: %08X; _codeOffs: %08X",
		_verbId, _objectId2, _codeOffs);
}

// TriggerObject

TriggerObject::TriggerObject()
	: _causesCount(0), _causes(0) {
}

TriggerObject::~TriggerObject() {
	delete[] _causes;
}

void TriggerObject::load(byte *dataStart, Common::SeekableReadStream &stream) {
	_objectId = stream.readUint32LE();
	_causesCount = stream.readUint16LE();
	stream.skip(2); // Skip padding
	debug(2, "TriggerObject::load() _objectId: %08X; _causesCount: %d",
		_objectId, _causesCount);
	_causes = new TriggerCause[_causesCount];
	for (uint i = 0; i < _causesCount; ++i) {
		_causes[i].load(stream);
	}
}

bool TriggerObject::findTriggerCause(uint32 verbId, uint32 objectId2, uint32 &codeOffs) {
	if ((verbId & 0xFFFF0000) == 0) {
		for (uint i = 0; i < _causesCount; ++i) {
			if ((verbId == 7 && ((_causes[i]._verbId == 7 && _causes[i]._objectId2 == objectId2) || _causes[i]._verbId == 8)) ||
				(verbId != 7 && verbId == _causes[i]._verbId)) {
				codeOffs = _causes[i]._codeOffs;
				return true;
			}
		}
	} else {
		for (uint i = 0; i < _causesCount; ++i) {
			if (_causes[i]._verbId == verbId && _causes[i]._objectId2 == objectId2) {
				codeOffs = _causes[i]._codeOffs;
				return true;
			}
		}
	}
	return false;
}

void TriggerObject::fixupSceneInfosDuckman() {
	for (uint i = 0; i < _causesCount; ++i) {
		_causes[i]._verbId &= 0xFFFF;
	}
}

// SceneInfo

SceneInfo::SceneInfo()
	: _triggerObjectsCount(0), _triggerObjects(0),
	_resourcesCount(0), _resources(0) {
}

SceneInfo::~SceneInfo() {
	delete[] _triggerObjects;
	delete[] _resources;
}

void SceneInfo::load(byte *dataStart, Common::SeekableReadStream &stream) {
	_id = stream.readUint16LE();
	_unk = stream.readUint16LE();
	_name = (uint16 *)(dataStart + stream.pos());
	swapBytesInWideString((byte *)_name);
	stream.skip(128);
	_triggerObjectsCount = stream.readUint16LE();
	_resourcesCount = stream.readUint16LE();
	debug(2, "\nSceneInfo::load() _id: %d; _unk: %d; _name: [%s]",
		_id, _unk, debugW2I(_name));
	uint32 triggerObjectsListOffs = stream.readUint32LE();
	if (_resourcesCount > 0) {
		_resources = new uint32[_resourcesCount];
		for (uint i = 0; i < _resourcesCount; ++i) {
			_resources[i] = stream.readUint32LE();
		}
	}
	if (_triggerObjectsCount > 0) {
		_triggerObjects = new TriggerObject[_triggerObjectsCount];
		for (uint i = 0; i < _triggerObjectsCount; ++i) {
			stream.seek(triggerObjectsListOffs + i * 4);
			uint32 triggerObjectOffs = stream.readUint32LE();
			stream.seek(triggerObjectOffs);
			_triggerObjects[i].load(dataStart, stream);
		}
	}
}

bool SceneInfo::findTriggerCause(uint32 verbId, uint32 objectId2, uint32 objectId, uint32 &codeOffs) {
	TriggerObject *triggerObject = findTriggerObject(objectId);
	if (triggerObject)
		return triggerObject->findTriggerCause(verbId, objectId2, codeOffs);
	return false;
}

void SceneInfo::getResources(uint &resourcesCount, uint32 *&resources) {
	resourcesCount = _resourcesCount;
	resources = _resources;
}

TriggerObject *SceneInfo::findTriggerObject(uint32 objectId) {
	for (uint i = 0; i < _triggerObjectsCount; ++i) {
		if (_triggerObjects[i]._objectId == objectId)
			return &_triggerObjects[i];
	}
	return 0;
}

void SceneInfo::fixupSceneInfosDuckman() {
	for (uint i = 0; i < _triggerObjectsCount; ++i) {
		_triggerObjects[i].fixupSceneInfosDuckman();
	}
}

// ScriptResource

ScriptResource::ScriptResource()
	: _codeOffsets(0), _objectMap(0) {
}

ScriptResource::~ScriptResource() {
	delete[] _codeOffsets;
	delete[] _objectMap;
}

void ScriptResource::load(Resource *resource) {
	_data = resource->_data;
	_dataSize = resource->_dataSize;

	Common::MemoryReadStream stream(_data, _dataSize, DisposeAfterUse::NO);

	uint32 objectMapOffs = 0, sceneInfosOffs = 0;
	_objectMapCount = 0;

	if (resource->_gameId == kGameIdBBDOU) {
		sceneInfosOffs = 0x18;
	} else if (resource->_gameId == kGameIdDuckman) {
		for (uint i = 0; i < 27; ++i) {
			_soundIds[i] = stream.readUint32LE();
		}
		sceneInfosOffs = 0x8C;
	}

	stream.skip(4); // Skip unused

	// Read item counts
	uint propertiesCount = stream.readUint16LE();
	uint blockCountersCount = stream.readUint16LE();
	if (resource->_gameId == kGameIdDuckman)
		_objectMapCount = stream.readUint16LE();
	_codeCount = stream.readUint16LE();
	_sceneInfosCount = stream.readUint16LE();
	if (resource->_gameId == kGameIdDuckman)
		stream.readUint16LE();//Unused?

	// Read item offsets
	uint32 propertiesOffs = stream.readUint32LE();
	uint32 blockCountersOffs = stream.readUint32LE();
	if (resource->_gameId == kGameIdDuckman)
		objectMapOffs = stream.readUint32LE(); //TODO Is this needed for BBDOU?
	uint32 codeTblOffs = stream.readUint32LE();

	debug(2, "ScriptResource::load() propertiesCount: %d; blockCountersCount: %d; _codeCount: %d; _sceneInfosCount: %d; _objectMapCount: %d",
		propertiesCount, blockCountersCount, _codeCount, _sceneInfosCount, _objectMapCount);
	debug(2, "ScriptResource::load() propertiesOffs: %08X; blockCountersOffs: %08X; codeTblOffs: %08X; objectMapOffs: %08X",
		propertiesOffs, blockCountersOffs, codeTblOffs, objectMapOffs);
	// Init properties
	_properties.init(propertiesCount, _data + propertiesOffs);

	// Init blockcounters
	_blockCounters.init(blockCountersCount, _data + blockCountersOffs);

	_codeOffsets = new uint32[_codeCount];
	stream.seek(codeTblOffs);
	for (uint i = 0; i < _codeCount; ++i) {
		_codeOffsets[i] = stream.readUint32LE();
	}

	_sceneInfos = new SceneInfo[_sceneInfosCount];
	for (uint i = 0; i < _sceneInfosCount; ++i) {
		stream.seek(sceneInfosOffs + i * 4);
		uint32 sceneInfoOffs = stream.readUint32LE();
		stream.seek(sceneInfoOffs);
		_sceneInfos[i].load(_data, stream);
	}

	if (_objectMapCount > 0) {
		_objectMap = new uint32[_objectMapCount];
		stream.seek(objectMapOffs);
		for (uint i = 0; i < _objectMapCount; ++i) {
			_objectMap[i] = stream.readUint32LE();
			stream.skip(4);
		}
	}

	if (resource->_gameId == kGameIdDuckman) {
		stream.seek(0x6C);
		_mainActorObjectId = stream.readUint32LE();
	} else if (resource->_gameId == kGameIdBBDOU) {
		stream.seek(0);
		_mainActorObjectId = stream.readUint32LE();
	}

	if (resource->_gameId == kGameIdDuckman)
		fixupSceneInfosDuckman();

}

byte *ScriptResource::getThreadCode(uint32 threadId) {
	return _data + _codeOffsets[(threadId & 0xFFFF) - 1];
}

byte *ScriptResource::getCode(uint32 codeOffs) {
	return _data + codeOffs;
}

SceneInfo *ScriptResource::getSceneInfo(uint32 index) {
	if (index > 0 && index <= _sceneInfosCount)
		return &_sceneInfos[index - 1];
	return 0;
}

uint32 ScriptResource::getObjectActorTypeId(uint32 objectId) {
	return _objectMap[(objectId & 0xFFFF) - 1];
}

void ScriptResource::fixupSceneInfosDuckman() {
	for (uint i = 0; i < _sceneInfosCount; ++i) {
		_sceneInfos[i].fixupSceneInfosDuckman();
	}
}

// ScriptInstance

ScriptInstance::ScriptInstance(IllusionsEngine *vm)
	: _vm(vm) {
}

void ScriptInstance::load(Resource *resource) {
	_vm->_scriptResource = new ScriptResource();
	_vm->_scriptResource->load(resource);
}

void ScriptInstance::unload() {
	delete _vm->_scriptResource;
	_vm->_scriptResource = 0;
}

} // End of namespace Illusions
