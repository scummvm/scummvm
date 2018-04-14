/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/stark/services/stateprovider.h"

#include "common/memstream.h"

#include "engines/stark/resources/level.h"
#include "engines/stark/resources/location.h"
#include "engines/stark/resources/object.h"

namespace Stark {

StateReadStream::StateReadStream(Common::SeekableReadStream *parentStream, DisposeAfterUse::Flag disposeParentStream) :
		SeekableSubReadStream(parentStream, 0, parentStream->size(), disposeParentStream) {
}

StateReadStream::~StateReadStream() {
}

Common::String StateReadStream::readString() {
	// Read the string length
	uint32 length = readUint32LE();

	// Read the string
	char *data = new char[length];
	read(data, length);
	Common::String string(data, length);
	delete[] data;

	return string;
}

StateProvider::ResourceTreeState::ResourceTreeState(uint32 size, byte *data, uint32 version) :
	_size(size),
	_data(data),
	_version(version) {
}

StateProvider::ResourceTreeState::~ResourceTreeState() {
	free(_data);
}

StateProvider::~StateProvider() {
	clear();
}

void StateProvider::clear() {
	for (ResourceTreeStateMap::iterator it = _stateStore.begin(); it != _stateStore.end(); it++) {
		delete it->_value;
	}
	_stateStore.clear();
}

void StateProvider::restoreLevelState(Resources::Level *level) {
	Common::String storeKey = level->getName();

	restoreResourceTreeState(storeKey, level, false);
}

void StateProvider::restoreCurrentLevelState(Resources::Level *level) {
	restoreResourceTreeState("Current", level, true);
}

void StateProvider::restoreLocationState(Resources::Level *level, Resources::Location *location) {
	Common::String storeKey = level->getName() + location->getName();

	restoreResourceTreeState(storeKey, location, false);
}

void StateProvider::restoreCurrentLocationState(Resources::Level *level, Resources::Location *location) {
	restoreResourceTreeState("CurrentCurrent", location, true);
}

void StateProvider::restoreGlobalState(Resources::Level *level) {
	restoreResourceTreeState("CurrentGlobal", level, true);
}

void StateProvider::restoreResourceTreeState(const Common::String &storeKey, Resources::Object *root, bool current) {
	if (_stateStore.contains(storeKey)) {
		ResourceTreeState *state = _stateStore[storeKey];

		Common::MemoryReadStream stream(state->getData(), state->getSize(), DisposeAfterUse::NO);
		readResourceTree(root, &stream, current, state->getVersion());
	}
}

void StateProvider::readResourceTree(Resources::Object *resource, Common::SeekableReadStream *stream, bool current, uint32 version) {
	// Read the resource to the source stream
	/* byte type = */ stream->readByte();
	/* byte subType = */ stream->readByte();
	uint32 size = stream->readUint32LE();

	if (size > 0) {
		Common::SeekableReadStream *resourceStream = stream->readStream(size);
		ResourceSerializer serializer(resourceStream, nullptr, version);

		// Deserialize the resource state from stream
		if (current) {
			resource->saveLoadCurrent(&serializer);
		} else {
			resource->saveLoad(&serializer);
		}

		delete resourceStream;
	}

	// Deserialize the resource children
	Common::Array<Resources::Object *> children = resource->listChildren<Resources::Object>();
	for (uint i = 0; i < children.size(); i++) {
		readResourceTree(children[i], stream, current, version);
	}
}

void StateProvider::saveLevelState(Resources::Level *level) {
	Common::String storeKey = level->getName();

	saveResourceTreeState(storeKey, level, false);
}

void StateProvider::saveCurrentLevelState(Resources::Level *level) {
	saveResourceTreeState("Current", level, true);
}

void StateProvider::saveLocationState(Resources::Level *level, Resources::Location *location) {
	Common::String storeKey = level->getName() + location->getName();

	saveResourceTreeState(storeKey, location, false);
}

void StateProvider::saveCurrentLocationState(Resources::Level *level, Resources::Location *location) {
	saveResourceTreeState("CurrentCurrent", location, true);
}

void StateProvider::saveGlobalState(Resources::Level *level) {
	saveResourceTreeState("CurrentGlobal", level, true);
}

void StateProvider::saveResourceTreeState(const Common::String &storeKey, Resources::Object *root, bool current) {
	// Delete any previous data
	if (_stateStore.contains(storeKey)) {
		delete _stateStore[storeKey];
		_stateStore.erase(storeKey);
	}

	// Write the tree state to memory
	Common::MemoryWriteStreamDynamic stream(DisposeAfterUse::NO);
	writeResourceTree(root, &stream, current);

	// Add the state to the store
	_stateStore[storeKey] = new ResourceTreeState(stream.size(), stream.getData(), kSaveVersion);
}

void StateProvider::writeResourceTree(Resources::Object *resource, Common::WriteStream *stream, bool current) {
	// Explicit scope to control the lifespan of the memory stream
	{
		Common::MemoryWriteStreamDynamic resourceStream(DisposeAfterUse::YES);
		ResourceSerializer serializer(nullptr, &resourceStream, kSaveVersion);

		// Serialize the resource to a memory stream
		if (current) {
			resource->saveLoadCurrent(&serializer);
		} else {
			resource->saveLoad(&serializer);
		}

		// Write the resource to the target stream
		stream->writeByte(resource->getType().get());
		stream->writeByte(resource->getSubType());
		stream->writeUint32LE(resourceStream.size());
		stream->write(resourceStream.getData(), resourceStream.size());
	}

	// Serialize the resource children
	Common::Array<Resources::Object *> children = resource->listChildren<Resources::Object>();
	for (uint i = 0; i < children.size(); i++) {
		writeResourceTree(children[i], stream, current);
	}
}

void StateProvider::readStateFromStream(StateReadStream *stream, uint saveVersion) {
	clear();

	uint32 treeCount = stream->readUint32LE();
	for (uint i = 0; i < treeCount; i++) {
		// Read the store key
		Common::String key = stream->readString();

		// Each resource tree state needs to have its own version because
		// some may never be made active again and serialized in the latest version.
		// In that case they stay in a previous version.
		uint treeVersion = 6;
		if (saveVersion > 6) {
			treeVersion = stream->readUint32LE();
		}

		// Read the data size
		uint32 dataSize = stream->readUint32LE();

		// Read the data
		byte *data = (byte *) malloc(dataSize);
		stream->read(data, dataSize);

		_stateStore[key] = new ResourceTreeState(dataSize, data, treeVersion);
	}
}

void StateProvider::writeStateToStream(Common::WriteStream *stream) {
	stream->writeUint32LE(_stateStore.size());

	for (ResourceTreeStateMap::iterator it = _stateStore.begin(); it != _stateStore.end(); it++) {
		stream->writeUint32LE(it->_key.size());
		stream->writeString(it->_key);
		stream->writeUint32LE(it->_value->getVersion());
		stream->writeUint32LE(it->_value->getSize());
		stream->write(it->_value->getData(), it->_value->getSize());
	}
}

ResourceSerializer::ResourceSerializer(Common::SeekableReadStream *in, Common::WriteStream *out, uint32 version) :
		Common::Serializer(in, out) {
	_version = version;
}

void ResourceSerializer::syncAsFloat(float &value) {
	if (isLoading()) {
		value = _loadStream->readFloatLE();
	} else {
		_saveStream->writeFloatLE(value);
	}
}

void ResourceSerializer::syncAsVector3d(Math::Vector3d &value) {
	syncAsFloat(value.x());
	syncAsFloat(value.y());
	syncAsFloat(value.z());
}

void ResourceSerializer::syncAsResourceReference(ResourceReference &reference) {
	if (isLoading()) {
		reference.loadFromStream(_loadStream);
	} else {
		reference.saveToStream(_saveStream);
	}
}

void ResourceSerializer::syncAsString32(Common::String &string) {
	if (isLoading()) {
		string.clear();

		uint32 length = _loadStream->readUint32LE();
		for (uint i = 0; i < length; i++) {
			char c = _loadStream->readByte();
			string += c;
		}

		_bytesSynced += 4 + length;
	} else {
		_saveStream->writeUint32LE(string.size());
		_saveStream->writeString(string);
		_bytesSynced += 4 + string.size();
	}
}

} // End of namespace Stark
