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

namespace Stark {

class Resource;
class Level;
class Location;

class ResourceSerializer : public Common::Serializer {
public:
	ResourceSerializer(Common::SeekableReadStream *in, Common::WriteStream *out) :
		Common::Serializer(in, out) {

	}
};

/**
 * Resource state provider.
 *
 * Maintains a serialized version of the state of the resource trees.
 */
class StateProvider {
public:
	~StateProvider();

	void restoreLevelState(Level *level);
	void restoreCurrentLevelState(Level *level);
	void restoreLocationState(Level *level, Location *location);
	void restoreCurrentLocationState(Level *level, Location *location);
	void restoreGlobalState(Level *level);

	void saveLevelState(Level *level);
	void saveCurrentLevelState(Level *level);
	void saveLocationState(Level *level, Location *location);
	void saveCurrentLocationState(Level *level, Location *location);
	void saveGlobalState(Level *level);

	/** Replace the current states by those read from the stream */
	void readStateFromStream(Common::SeekableReadStream *stream);

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

	void restoreResourceTreeState(Common::String storeKey, Resource *root, bool current);
	void saveResourceTreeState(Common::String storeKey, Resource *root, bool current);

	void readResourceTree(Resource *resource, Common::SeekableReadStream *stream, bool current);
	void writeResourceTree(Resource *resource, Common::WriteStream *stream, bool current);

	void clear();

	Common::String readString(Common::ReadStream *stream);

	ResourceTreeStateMap _stateStore;
};

} // End of namespace Stark

#endif // STARK_STATE_PROVIDER_H
