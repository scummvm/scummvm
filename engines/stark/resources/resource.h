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

#ifndef STARK_RESOURCES_RESOURCE_H
#define STARK_RESOURCES_RESOURCE_H

#include "common/array.h"
#include "common/str.h"

namespace Stark {

class XRCReadStream;
class ResourceSerializer;

class ResourceType {
public:
	enum Type {
		kInvalid                = 0,
		kRoot                   = 1,
		kLevel                  = 2,
		kLocation               = 3,
		kLayer                  = 4,
		kCamera                 = 5,
		kFloor                  = 6,
		kFloorFace              = 7,
		kItem                   = 8,
		kScript                 = 9,
		kAnimHierarchy          = 10,
		kAnim                   = 11,
		kDirection              = 12,
		kImage                  = 13,
		kAnimScript             = 14,
		kAnimScriptItem         = 15,
		kSoundItem              = 16,
		kFloorField             = 18,
		kBookmark               = 19,
		kKnowledgeSet           = 20,
		kKnowledge              = 21,
		kCommand                = 22,
		kPATTable               = 23,
		kContainer              = 26,
		kDialog                 = 27,
		kSpeech                 = 29,
		kLight                  = 30,
		kCursor                 = 31, // Not sure about this one
		kBoneMesh               = 32,
		kScroll                 = 33,
		kFMV                    = 34,
		kLipSynch               = 35,
		kAnimScriptBonesTrigger = 36,
		kString                 = 37,
		kTextureSet             = 38
	};

	ResourceType();
	ResourceType(Type type);

	Type get();
	const char *getName();

	bool operator==(const ResourceType &other) const {
		return other._type == _type;
	}

	bool operator!=(const ResourceType &other) const {
		return other._type != _type;
	}

	bool operator==(const ResourceType::Type other) const {
		return other == _type;
	}

	bool operator!=(const ResourceType::Type other) const {
		return other != _type;
	}

private:
	Type _type;
};

/**
 * Game resource base object
 *
 * The in-game objects are represented using subclasses of this class.
 *
 * The game world is made of a tree of resources, with each level further down
 * the tree adding further details. An instance of this class is a node in that
 * tree.
 *
 * The first few tree levels are as follow:
 * - Root
 * - Level
 * - Location
 * - Layer
 *
 * The actual world tree is cut off in several sub-trees. There is one sub-tree
 * per xarc archive. For resource management reasons the sub-trees are not merged
 * in memory, the sub-trees are loaded and unloaded as needed, according to the
 * current level / location.
 *
 * The xarc archives contain each an xrc file, which is a serialized version
 * of the initial state of a resource sub-tree. The readData method is called for
 * each resource by the archive loader when a resource tree is loaded to set up
 * its initial state.
 *
 * As the game plays, modifications are made to the resources to reflect
 * the game's state. When the resource sub-trees are loaded or unloaded their
 * state is restored or persisted by the state provider. The saveLoad method
 * is called to perform the serialization / deserialization of a resource.
 * The saveLoadCurrent method is additionally called when loading or saving
 * a sub-tree corresponding to the current level / location. This allows to
 * persist additional data needed when restoring an active location.
 *
 * The OnEnterLocation and OnExitLocation methods are called by the resource
 * provider when entering or leaving a level / location.
 *
 * The OnGameLoop method is called during the game loop.
 *
 */
class Resource {
public:
	virtual ~Resource();

	ResourceType getType() const { return _type; }
	byte getSubType() const { return _subType; }
	uint16 getIndex() const { return _index; }
	Common::String getIndexAsString() const { return Common::String::format("%02x", _index); }
	Common::String getName() const { return _name; }

	/**
	 * Deserialize the resource static data and initial state.
	 */
	virtual void readData(XRCReadStream *stream);

	/**
	 * Persist / restore the resource state
	 */
	virtual void saveLoad(ResourceSerializer *serializer);

	/**
	 * Persist / restore the resource state
	 *
	 * Called only for active locations
	 */
	virtual void saveLoadCurrent(ResourceSerializer *serializer);

	/**
	 * Called when the node's initialization is complete.
	 *
	 * Allows to load additional data from file.
	 */
	virtual void onPostRead();

	/**
	 * Called when the resource sub-tree is entirely loaded.
	 *
	 * Allows to load data from other nodes.
	 */
	virtual void onAllLoaded();

	/**
	 * Called when entering a location
	 */
	virtual void onEnterLocation();

	/**
	 * Called once per game loop
	 */
	virtual void onGameLoop(uint msecs);

	/**
	 * Called when exiting a location
	 */
	virtual void onExitLocation();

	/**
	 * Called before a resource sub-tree is unloaded.
	 */
	virtual void onPreDestroy();

	template<class T>
	T *findChild(bool mustBeUnique = true);

	template<class T>
	T *findChildWithSubtype(int subType, bool mustBeUnique = true);

	template<class T>
	T *findChildWithIndex(uint16 index, int subType = -1);

	template<class T>
	Common::Array<T *> listChildren(int subType = -1);

	void addChild(Resource *child);

	void print(uint depth = 0);

protected:
	Resource(Resource *parent, byte subType, uint16 index, const Common::String &name);

	virtual void printData() = 0;

	ResourceType _type;
	byte _subType;
	uint16 _index;
	Common::String _name;

	Resource *_parent;
	Common::Array<Resource *> _children;
};

class UnimplementedResource : public Resource {
public:
	UnimplementedResource(Resource *parent, ResourceType type, byte subType, uint16 index, const Common::String &name);
	virtual ~UnimplementedResource();

protected:
	void readData(XRCReadStream *stream) override;
	void printData() override;

	uint32 _dataLength;
	byte *_data;
};

template <class T>
Common::Array<T *> Resource::listChildren(int subType) {
	Common::Array<T *> list;

	for (uint i = 0; i < _children.size(); i++) {
		if (_children[i]->getType() == T::TYPE
				&& (_children[i]->getSubType() == subType || subType == -1)) {
			// Found a matching child
			list.push_back((T *)_children[i]);
		}
	}

	return list;
}

template<>
Common::Array<Resource *> Resource::listChildren<Resource>(int subType);

template<class T>
T *Resource::findChild(bool mustBeUnique) {
	return findChildWithSubtype<T>(-1, mustBeUnique);
}

template <class T>
T *Resource::findChildWithSubtype(int subType, bool mustBeUnique) {
	Common::Array<T *> list = listChildren<T>(subType);

	if (list.empty()) {
		return nullptr;
	}

	if (list.size() > 1 && mustBeUnique) {
		error("Several children resources matching criteria type = %s, subtype = %d", ResourceType(T::TYPE).getName(), subType);
	}

	return list.front();
}

template <class T>
T *Resource::findChildWithIndex(uint16 index, int subType) {
	for (uint i = 0; i < _children.size(); i++) {
		if (_children[i]->getType() == T::TYPE
				&& (_children[i]->getSubType() == subType || subType == -1)
				&& _children[i]->getIndex() == index) {
			// Found a matching child
			return (T *)_children[i];
		}
	}

	return nullptr;
}

} // End of namespace Stark

#endif // STARK_RESOURCES_RESOURCE_H
