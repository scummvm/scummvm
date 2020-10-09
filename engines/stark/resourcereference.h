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

#ifndef STARK_RESOURCES_RESOURCE_REFERENCE_H
#define STARK_RESOURCES_RESOURCE_REFERENCE_H

#include "common/array.h"
#include "common/stream.h"

#include "engines/stark/resources/object.h"

namespace Stark {

/**
 * A reference to a resource.
 *
 * Internally, the referenced resource is designed by its path
 * in the resource tree.
 *
 */
class ResourceReference {
public:
	ResourceReference();

	Common::String describe() const;

	/** Read the reference from a stream */
	void loadFromStream(Common::ReadStream *stream);

	/** Write the reference to a stream */
	void saveToStream(Common::WriteStream *stream);

	/** Make the reference point to the specified object */
	void buildFromResource(Resources::Object *resource);

	/** Resolve the reference to the actual resource */
	template <class T>
	T* resolve() const;

	/** Return true if this reference is a null pointer */
	bool empty() const;

	/** Can this reference be resolved using currently loaded archives? */
	bool canResolve() const;
private:
	void addPathElement(Resources::Type type, uint16 index);
	Resources::Object *resolve() const;

	class PathElement {
	public:
		PathElement(Resources::Type type, uint16 index);
		Common::String describe() const;

		Resources::Type getType() const { return _type; }
		uint16 getIndex() const { return _index; }

	private:
		Resources::Type _type;
		uint16 _index;
	};

	Common::Array<PathElement> _path;
};

template<class T>
T* ResourceReference::resolve() const {
	return Resources::Object::cast<T>(resolve());
}

} // End of namespace Stark

#endif // STARK_RESOURCES_RESOURCE_REFERENCE_H
