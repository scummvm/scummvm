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

#include "engines/stark/resources/resource.h"

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

	Common::String describe();

	void addPathElement(ResourceType type, uint16 index);
	Resource *resolve();

	template <class T>
	T* resolve() {
		Resource *resource = resolve();

		if (resource && resource->getType() != T::TYPE) {
			error("Unexpected resource type when resolving reference %s instad of %s",
					resource->getType().getName(), ResourceType(T::TYPE).getName());
		}

		return (T *) resource;
	}

private:
	class PathElement {
	public:
		PathElement(ResourceType type, uint16 index);
		Common::String describe();

		ResourceType getType() const { return _type; }
		uint16 getIndex() const { return _index; }

	private:
		ResourceType _type;
		uint16 _index;
	};

	Common::Array<PathElement> _path;
};

} // End of namespace Stark

#endif // STARK_RESOURCES_RESOURCE_REFERENCE_H
