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

#ifndef STARK_FORMATS_BIFF_H
#define STARK_FORMATS_BIFF_H

#include "common/array.h"
#include "common/scummsys.h"

namespace Stark {

class ArchiveReadStream;

namespace Formats {

class BiffObject;

/**
 * A tree-style container for BiffObjects
 *
 * Users of this class must provide a factory method for the BiffObject subclasses
 * contained in the archive. This class can only read the archive's structure
 * and not specific object types.
 */
class BiffArchive {
public:
	typedef BiffObject *(*ObjectBuilder)(uint32 type);

	BiffArchive(ArchiveReadStream *stream, ObjectBuilder objectBuilder);
	~BiffArchive();

	/** List the objects at the root level of the archive */
	Common::Array<BiffObject *> listObjects();

	/** List objects recursively matching the template parameter type */
	template<class T>
	Common::Array<T *> listObjectsRecursive();

private:
	void read(ArchiveReadStream *stream);
	BiffObject *readObject(ArchiveReadStream *stream, BiffObject *parent);

	ObjectBuilder _objectBuilder;
	uint32 _version;

	Common::Array<BiffObject *> _rootObjects;
};

/**
 * An object which can be read from a BiffArchive
 *
 * Each object has a list of children objects, resulting in a tree structure
 */
class BiffObject {
public:
	BiffObject();
	virtual ~BiffObject();

	/**
	 * Used to read the object data from the stream
	 */
	virtual void readData(ArchiveReadStream *stream, uint32 dataLength) = 0;

	/** Get the object type */
	uint32 getType() const;

	/** List children recursively matching the template parameter type */
	template<class T>
	Common::Array<T *> listChildrenRecursive();

	/** Add an object to the children list */
	void addChild(BiffObject *child);

protected:
	uint32 _type;
	uint32 _u3;
	uint32 _version;

	BiffObject *_parent;
	Common::Array<BiffObject *> _children;

	friend class BiffArchive;
};

template<class T>
Common::Array<T *> BiffArchive::listObjectsRecursive() {
	Common::Array<BiffObject *> objects = listObjects();

	Common::Array<T *> array;
	for (uint i = 0; i < objects.size(); i++) {
		array.push_back(objects[i]->listChildrenRecursive<T>());
	}

	return array;
}

template<class T>
Common::Array<T *> BiffObject::listChildrenRecursive() {
	Common::Array<T *> list;

	for (uint i = 0; i < _children.size(); i++) {
		if (_children[i]->getType() == T::TYPE) {
			// Found a matching child
			list.push_back(static_cast<T *>(_children[i]));
		}

		// Look for matching resources in the child's children
		list.push_back(_children[i]->listChildrenRecursive<T>());
	}

	return list;
}

} // End of namespace Formats
} // End of namespace Stark

#endif // STARK_FORMATS_BIFF_H
