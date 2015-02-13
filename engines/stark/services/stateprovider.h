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

#ifndef STARK_STATE_PROVIDER_H
#define STARK_STATE_PROVIDER_H

#include "common/hashmap.h"
#include "common/serializer.h"
#include "common/hash-str.h"
#include "common/stream.h"
#include "common/substream.h"

namespace Stark {

namespace Resources {
class Resource;
class Level;
class Location;
}

class StateReadStream : public Common::SeekableSubReadStream {
public:
	StateReadStream(Common::SeekableReadStream *parentStream, DisposeAfterUse::Flag disposeParentStream = DisposeAfterUse::YES);
	virtual ~StateReadStream();

	Common::String readString();
};

class ResourceSerializer : public Common::Serializer {
public:
	ResourceSerializer(Common::SeekableReadStream *in, Common::WriteStream *out) :
		Common::Serializer(in, out) {

	}
};

/**
 * Resources::Resource state provider.
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
	void readStateFromStream(StateReadStream*stream);

	/** Write the states in the store to a stream */
	void writeStateToStream(Common::WriteStream *stream);

private:
	class ResourceTreeState {
	public:
		ResourceTreeState();
		ResourceTreeState(uint32 size, byte *data);
		~ResourceTreeState();

		uint32 getSize() const { return _size; }
		byte *getData() const { return _data; }
	private:
		uint32 _size;
		byte *_data;
	};

	typedef Common::HashMap<Common::String, ResourceTreeState *> ResourceTreeStateMap;

	void restoreResourceTreeState(Common::String storeKey, Resources::Resource *root, bool current);
	void saveResourceTreeState(Common::String storeKey, Resources::Resource *root, bool current);

	void readResourceTree(Resources::Resource *resource, Common::SeekableReadStream *stream, bool current);
	void writeResourceTree(Resources::Resource *resource, Common::WriteStream *stream, bool current);

	void clear();

	ResourceTreeStateMap _stateStore;
};

} // End of namespace Stark

#endif // STARK_STATE_PROVIDER_H
