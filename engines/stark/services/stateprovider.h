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

#ifndef STARK_SERVICES_STATE_PROVIDER_H
#define STARK_SERVICES_STATE_PROVIDER_H

#include "common/hashmap.h"
#include "common/serializer.h"
#include "common/hash-str.h"
#include "common/stream.h"
#include "common/substream.h"

#include "math/mathfwd.h"

#include "engines/stark/resourcereference.h"

namespace Stark {

namespace Resources {
class Object;
class Level;
class Location;
}

class StateReadStream : public Common::SeekableSubReadStream {
public:
	explicit StateReadStream(Common::SeekableReadStream *parentStream, DisposeAfterUse::Flag disposeParentStream = DisposeAfterUse::YES);
	virtual ~StateReadStream();

	Common::String readString();
};

class ResourceSerializer : public Common::Serializer {
public:
	ResourceSerializer(Common::SeekableReadStream *in, Common::WriteStream *out, uint32 version);

	void syncAsFloat(float &value);
	void syncAsVector3d(Math::Vector3d &value);
	void syncAsResourceReference(ResourceReference &reference);
	void syncAsString32(Common::String &string);

	template<typename T>
	void syncAsResourceReference(T **object, Version minVersion = 0, Version maxVersion = kLastVersion);

	template<typename T>
	void syncArraySize(Common::Array<T> &array, Version minVersion = 0, Version maxVersion = kLastVersion);
};

template<typename T>
void ResourceSerializer::syncAsResourceReference(T **object, Version minVersion, Version maxVersion) {
	assert(object);

	if (_version < minVersion || _version > maxVersion)
		return;	// Ignore anything which is not supposed to be present in this save game version

	if (isLoading()) {
		ResourceReference reference;
		reference.loadFromStream(_loadStream);
		*object = reference.resolve<T>();
	} else {
		ResourceReference reference;
		reference.buildFromResource(*object);
		reference.saveToStream(_saveStream);
	}
}

template<typename T>
void ResourceSerializer::syncArraySize(Common::Array<T> &array, Version minVersion, Version maxVersion) {
	if (_version < minVersion || _version > maxVersion)
		return;	// Ignore anything which is not supposed to be present in this save game version

	uint32 size = array.size();
	syncAsUint32LE(size);

	if (isLoading()) {
		array.resize(size);
	}
}

/**
 * Resource state provider.
 *
 * Maintains a serialized version of the state of the resource trees.
 */
class StateProvider {
public:
	~StateProvider();

	void restoreLevelState(Resources::Level *level);
	void restoreCurrentLevelState(Resources::Level *level);
	void restoreLocationState(Resources::Level *level, Resources::Location *location);
	void restoreCurrentLocationState(Resources::Level *level, Resources::Location *location);
	void restoreGlobalState(Resources::Level *level);

	void saveLevelState(Resources::Level *level);
	void saveCurrentLevelState(Resources::Level *level);
	void saveLocationState(Resources::Level *level, Resources::Location *location);
	void saveCurrentLocationState(Resources::Level *level, Resources::Location *location);
	void saveGlobalState(Resources::Level *level);

	/** Replace the current states by those read from the stream */
	void readStateFromStream(StateReadStream *stream, uint saveVersion);

	/** Write the states in the store to a stream */
	void writeStateToStream(Common::WriteStream *stream);

	/** Clear all the state, effectively preparing to start a new game */
	void clear();

	static const uint kMinSaveVersion = 6;
	static const uint kSaveVersion = 13;

private:
	class ResourceTreeState {
	public:
		ResourceTreeState(uint32 size, byte *data, uint32 version);
		~ResourceTreeState();

		uint32 getVersion() const { return _version; }
		uint32 getSize() const { return _size; }
		byte *getData() const { return _data; }

	private:
		uint32 _version;
		uint32 _size;
		byte *_data;
	};

	typedef Common::HashMap<Common::String, ResourceTreeState *> ResourceTreeStateMap;

	void restoreResourceTreeState(const Common::String &storeKey, Resources::Object *root, bool current);
	void saveResourceTreeState(const Common::String &storeKey, Resources::Object *root, bool current);

	void readResourceTree(Resources::Object *resource, Common::SeekableReadStream *stream, bool current, uint32 version);
	void writeResourceTree(Resources::Object *resource, Common::WriteStream *stream, bool current);

	ResourceTreeStateMap _stateStore;
};

} // End of namespace Stark

#endif // STARK_SERVICES_STATE_PROVIDER_H
