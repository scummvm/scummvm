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

namespace Formats {
class XRCReadStream;
}
class ResourceSerializer;

namespace Resources {

class Type {
public:
	enum ResourceType {
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
		kPath                   = 17,
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
		kBonesMesh              = 32,
		kScroll                 = 33,
		kFMV                    = 34,
		kLipSync                = 35,
		kAnimSoundTrigger       = 36,
		kString                 = 37,
		kTextureSet             = 38
	};

	Type();
	Type(ResourceType type);

	ResourceType get() const;
	const char *getName() const;

	bool operator==(const Type &other) const {
		return other._type == _type;
	}

	bool operator!=(const Type &other) const {
		return other._type != _type;
	}

	bool operator==(const Type::ResourceType other) const {
		return other == _type;
	}

	bool operator!=(const Type::ResourceType other) const {
		return other != _type;
	}

private:
	ResourceType _type;
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
class Object {
public:
	virtual ~Object();

	/** Get the resource type */
	Type getType() const { return _type; }

	/** Get the resource sub type */
	byte getSubType() const { return _subType; }

	/** Get the resource index */
	uint16 getIndex() const { return _index; }

	/** Get the resource index as a string */
	Common::String getIndexAsString() const { return Common::String::format("%02x", _index); }

	/** Get the name of the resource */
	Common::String getName() const { return _name; }

	/**
	 * Deserialize the resource static data and initial state.
	 */
	virtual void readData(Formats::XRCReadStream *stream);

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
	virtual void onGameLoop();

	/**
	 * Called when ScummVM pauses or resumes the engine
	 */
	virtual void onEnginePause(bool pause);

	/**
	 * Called when exiting a location
	 */
	virtual void onExitLocation();

	/**
	 * Called before a resource sub-tree is unloaded.
	 */
	virtual void onPreDestroy();

	/**
	 * Cast a resource, performing a type check
	 */
	template<class T>
	static T *cast(Object *resource);

	/** Find the first parent resource with the specified type */
	template<class T>
	T *findParent();

	/** Find a child resource matching the specified type, index and subtype */
	Object *findChildWithIndex(Type type, uint16 index, int subType = -1) const;

	/** Find a child resource matching the specified type, order in the children list and subtype */
	Object *findChildWithOrder(Type type, uint16 order, int subType = -1) const;

	/** Find a child resource matching the specified type, name and subtype */
	Object *findChildWithName(Type type, const Common::String &name, int subType = -1) const;

	/** Find a child matching the template parameter type */
	template<class T>
	T *findChild(bool mustBeUnique = true) const;

	/** Find a child matching the template parameter type and the specified subtype */
	template<class T>
	T *findChildWithSubtype(int subType, bool mustBeUnique = true) const;

	/** Find a child matching the template parameter type and the specified index */
	template<class T>
	T *findChildWithIndex(uint16 index, int subType = -1) const;

	/** Find a child matching the template parameter type, order in the children list and subtype */
	template<class T>
	T *findChildWithOrder(uint16 order, int subType = -1) const;

	/** Find a child matching the template parameter type, name and subtype */
	template<class T>
	T *findChildWithName(const Common::String &name, int subType = -1) const;

	/** List children matching the template parameter type and the specified subtype */
	template<class T>
	Common::Array<T *> listChildren(int subType = -1) const;

	/** List children recursively matching the template parameter type and the specified subtype */
	template<class T>
	Common::Array<T *> listChildrenRecursive(int subType = -1);

	/** Add a resource to the children list */
	void addChild(Object *child);

	/** Print debug information for the resource */
	virtual void print(uint depth = 0);

protected:
	Object(Object *parent, byte subType, uint16 index, const Common::String &name);

	void printWithDepth(uint depth, const Common::String &string) const;
	void printDescription(uint depth) const;
	virtual void printData();

	Type _type;
	byte _subType;
	uint16 _index;
	Common::String _name;

	Object *_parent;
	Common::Array<Object *> _children;
};

/**
 * An unimplemented resource type.
 *
 * Used to display the raw resource data when dumping a resource tree.
 * To be removed once all the resource types are implemented.
 */
class UnimplementedResource : public Object {
public:
	UnimplementedResource(Object *parent, Type type, byte subType, uint16 index, const Common::String &name);
	virtual ~UnimplementedResource();

protected:
	void readData(Formats::XRCReadStream *stream) override;
	void printData() override;

	uint32 _dataLength;
	byte *_data;
};

template <class T>
T* Object::cast(Object *resource) {
	if (resource && resource->_type != T::TYPE) {
		error("Unexpected resource type when casting resource %s instead of %s",
				resource->_type.getName(), Type(T::TYPE).getName());
	}

	return (T *) resource;
}

template<>
Object *Object::cast<Object>(Object *resource);

template<class T>
T *Object::findParent() {
	if (getType() == T::TYPE) {
		return cast<T>(this);
	} else if (!_parent) {
		return nullptr;
	} else {
		return _parent->findParent<T>();
	}
}

template<>
Object *Object::findParent();

template <class T>
Common::Array<T *> Object::listChildren(int subType) const {
	Common::Array<T *> list;

	for (uint i = 0; i < _children.size(); i++) {
		if (_children[i]->getType() == T::TYPE
				&& (_children[i]->getSubType() == subType || subType == -1)) {
			// Found a matching child
			list.push_back(Object::cast<T>(_children[i]));
		}
	}

	return list;
}

template<class T>
Common::Array<T *> Object::listChildrenRecursive(int subType) {
	Common::Array<T *> list;

	for (uint i = 0; i < _children.size(); i++) {
		if (_children[i]->getType() == T::TYPE
				&& (_children[i]->getSubType() == subType || subType == -1)) {
			// Found a matching child
			list.push_back(Object::cast<T>(_children[i]));
		}

		// Look for matching resources in the child's children
		list.push_back(_children[i]->listChildrenRecursive<T>(subType));
	}

	return list;
}

template<>
Common::Array<Object *> Object::listChildren<Object>(int subType) const;

template<class T>
T *Object::findChild(bool mustBeUnique) const {
	return findChildWithSubtype<T>(-1, mustBeUnique);
}

template <class T>
T *Object::findChildWithSubtype(int subType, bool mustBeUnique) const {
	Common::Array<T *> list = listChildren<T>(subType);

	if (list.empty()) {
		return nullptr;
	}

	if (list.size() > 1 && mustBeUnique) {
		error("Several children resources matching criteria type = %s, subtype = %d", Type(T::TYPE).getName(), subType);
	}

	return list.front();
}

template <class T>
T *Object::findChildWithIndex(uint16 index, int subType) const {
	return Object::cast<T>(findChildWithIndex(T::TYPE, index, subType));
}

template <class T>
T *Object::findChildWithOrder(uint16 order, int subType) const {
	return Object::cast<T>(findChildWithOrder(T::TYPE, order, subType));
}

template<class T>
T *Object::findChildWithName(const Common::String &name, int subType) const {
	return Object::cast<T>(findChildWithName(T::TYPE, name, subType));
}

} // End of namespace Resources
} // End of namespace Stark

#endif // STARK_RESOURCES_RESOURCE_H
