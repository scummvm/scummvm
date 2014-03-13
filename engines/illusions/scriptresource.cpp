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
#include "illusions/scriptresource.h"

namespace Illusions {

// ScriptResourceLoader

void ScriptResourceLoader::load(Resource *resource) {
	debug("ScriptResourceLoader::load() Loading script %08X from %s...", resource->_resId, resource->_filename.c_str());

	ScriptResource *scriptResource = new ScriptResource();
	scriptResource->load(resource->_data, resource->_dataSize);
	
	
	
	_vm->_scriptResource = scriptResource;
	
}

void ScriptResourceLoader::unload(Resource *resource) {
}

void ScriptResourceLoader::buildFilename(Resource *resource) {
	resource->_filename = Common::String::format("%08X.scr", resource->_resId);
}

bool ScriptResourceLoader::isFlag(int flag) {
	return
		flag == kRlfLoadFile;
}

Properties::Properties()
	: _count(0), _properties(0) {
}

void Properties::init(uint count, byte *properties) {
	_count = count;
	_properties = properties;
}

// BlockCounters

BlockCounters::BlockCounters()
	: _count(0), _blockCounters(0) {
}

void BlockCounters::init(uint count, byte *blockCounters) {
	_count = count;
	_blockCounters = blockCounters;
}


// TriggerCause

void TriggerCause::load(Common::SeekableReadStream &stream) {
	_verbId = stream.readUint32LE();
	_objectId2 = stream.readUint32LE();
	_codeOffs = stream.readUint32LE();
	
	debug("TriggerCause::load() _verbId: %08X; _objectId2: %08X; _codeOffs: %08X",
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
	debug("TriggerObject::load() _objectId: %08X; _causesCount: %d",
		_objectId, _causesCount);
	_causes = new TriggerCause[_causesCount];
	for (uint i = 0; i < _causesCount; ++i)
		_causes[i].load(stream);
}

// ProgInfo

ProgInfo::ProgInfo()
	: _triggerObjectsCount(0), _triggerObjects(0) {
}

ProgInfo::~ProgInfo() {
	delete[] _triggerObjects;
}

char *debugW2I(byte *wstr) {
	static char buf[65];
	char *p = buf;
	while (*wstr != 0) {
		*p++ = *wstr;
		wstr += 2;
	}
	*p = 0;
	return buf;
}

void ProgInfo::load(byte *dataStart, Common::SeekableReadStream &stream) {
	_id = stream.readUint16LE();
	_unk = stream.readUint16LE();
	_name = dataStart + stream.pos();
	stream.skip(128);
	_triggerObjectsCount = stream.readUint16LE();
	stream.skip(2); // Skip padding
	debug("\nProgInfo::load() _id: %d; _unk: %d; _name: [%s]",
		_id, _unk, debugW2I(_name));
	uint32 triggerObjectsListOffs = stream.readUint32LE();
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

// ScriptResource

ScriptResource::ScriptResource()
	: _codeOffsets(0) {
}

ScriptResource::~ScriptResource() {
	delete[] _codeOffsets;
}

void ScriptResource::load(byte *data, uint32 dataSize) {
	Common::MemoryReadStream stream(data, dataSize, DisposeAfterUse::NO);
	
	stream.skip(4); // Skip unused
	uint propertiesCount = stream.readUint16LE();
	uint blockCountersCount = stream.readUint16LE();
	uint codeCount = stream.readUint16LE();
	_progInfosCount = stream.readUint16LE();
	uint32 propertiesOffs = stream.readUint32LE();
	uint32 blockCountersOffs = stream.readUint32LE();
	uint32 codeTblOffs = stream.readUint32LE();
	
	// Init properties
	_properties.init(propertiesCount, data + propertiesOffs);
	
	// Init blockcounters
	_blockCounters.init(blockCountersCount, data + blockCountersOffs);
	
	_codeOffsets = new uint32[codeCount];
	stream.seek(codeTblOffs);
	for (uint i = 0; i < codeCount; ++i)
		_codeOffsets[i] = stream.readUint32LE();

	_progInfos = new ProgInfo[_progInfosCount];
	for (uint i = 0; i < _progInfosCount; ++i) {
		stream.seek(0x18 + i * 4);
		uint32 progInfoOffs = stream.readUint32LE();
		stream.seek(progInfoOffs);
		_progInfos[i].load(data, stream);
	}

	debug("ScriptResource::load() propertiesCount: %d; blockCountersCount: %d; codeCount: %d; _progInfosCount: %d",
		propertiesCount, blockCountersCount, codeCount, _progInfosCount);
	debug("ScriptResource::load() propertiesOffs: %08X; blockCountersOffs: %08X; codeTblOffs: %08X",
		propertiesOffs, blockCountersOffs, codeTblOffs);

}

} // End of namespace Illusions
