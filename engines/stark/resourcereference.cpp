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

#include "engines/stark/resourcereference.h"

#include "engines/stark/debug.h"
#include "engines/stark/resources/level.h"
#include "engines/stark/resources/location.h"
#include "engines/stark/services/resourceprovider.h"
#include "engines/stark/stark.h"

namespace Stark {


ResourceReference::PathElement::PathElement(ResourceType type, uint16 index) :
		_type(type), _index(index) {
}

Common::String ResourceReference::PathElement::describe() {
	return  Common::String::format("(%s idx %d)", _type.getName(), _index);
}

ResourceReference::ResourceReference() {
}

void ResourceReference::addPathElement(ResourceType type, uint16 index) {
	_path.push_back(PathElement(type, index));
}

Resource *ResourceReference::resolve() const {
	ResourceProvider *resourceProvider = StarkServices::instance().resourceProvider;
	Global *global = StarkServices::instance().global;

	Resource *resource = nullptr;
	for (uint i = 0; i < _path.size(); i++) {
		PathElement element = _path[i];

		switch (element.getType().get()) {
		case ResourceType::kLevel:
			if (element.getIndex()) {
				resource = resourceProvider->getLevel(element.getIndex());
			} else {
				resource = global->getLevel();
			}
			break;
		case ResourceType::kLocation:
			resource = resourceProvider->getLocation(resource->getIndex(), element.getIndex());
			break;
		default:
			resource = resource->findChildWithIndex(element.getType(), element.getIndex());
			break;
		}
	}

	return resource;
}

Common::String ResourceReference::describe() {
	Common::String desc;

	for (uint i = 0; i < _path.size(); i++) {
		desc += _path[i].describe() + " ";
	}

	return desc;
}

} // End of namespace Stark
