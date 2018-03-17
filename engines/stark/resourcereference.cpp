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
#include "engines/stark/services/global.h"
#include "engines/stark/services/resourceprovider.h"
#include "engines/stark/services/staticprovider.h"
#include "engines/stark/services/services.h"

namespace Stark {


ResourceReference::PathElement::PathElement(Resources::Type type, uint16 index) :
		_type(type), _index(index) {
}

Common::String ResourceReference::PathElement::describe() const {
	return  Common::String::format("(%s idx %d)", _type.getName(), _index);
}

ResourceReference::ResourceReference() {
}

void ResourceReference::addPathElement(Resources::Type type, uint16 index) {
	_path.push_back(PathElement(type, index));
}

Resources::Object *ResourceReference::resolve() const {
	Resources::Object *level = nullptr;
	Resources::Object *resource = nullptr;
	for (uint i = 0; i < _path.size(); i++) {
		const PathElement &element = _path[i];

		switch (element.getType().get()) {
		case Resources::Type::kLevel:
			if (StarkStaticProvider->isStaticLocation()) {
				resource = level = StarkStaticProvider->getLevel();
				assert(resource->getIndex() == element.getIndex());
			} else if (element.getIndex()) {
				resource = level = StarkResourceProvider->getLevel(element.getIndex());
			} else {
				resource = level = StarkGlobal->getLevel();
			}

			if (!level) {
				error("Level '%d' not found", element.getIndex());
			}

			break;
		case Resources::Type::kLocation:
			if (!level) {
				error("Cannot resolve location '%d' without resolving a level first", element.getIndex());
			}

			if (StarkStaticProvider->isStaticLocation()) {
				resource = StarkStaticProvider->getLocation();
				assert(resource->getIndex() == element.getIndex());
			} else {
				resource = StarkResourceProvider->getLocation(level->getIndex(), element.getIndex());
			}

			if (!resource) {
				error("Location '%d' not found in level '%d'", element.getIndex(), level->getIndex());
			}

			break;
		default:
			assert(resource);
			resource = resource->findChildWithIndex(element.getType(), element.getIndex());
			break;
		}
	}

	return resource;
}

bool ResourceReference::canResolve() const {
	if (empty()) {
		return false;
	}

	Resources::Object *level = nullptr;
	for (uint i = 0; i < _path.size(); i++) {
		const PathElement &element = _path[i];

		switch (element.getType().get()) {
			case Resources::Type::kLevel:
				if (element.getIndex()) {
					level = StarkResourceProvider->getLevel(element.getIndex());
				} else {
					level = StarkGlobal->getLevel();
				}

				if (!level) {
					return false;
				}

				break;
			case Resources::Type::kLocation: {
				if (!level) {
					return false;
				}

				Resources::Object *location = StarkResourceProvider->getLocation(level->getIndex(), element.getIndex());

				if (!location) {
					return false;
				}

				break;
			}
			default:
				return true;
		}
	}

	return true;
}

bool ResourceReference::empty() const {
	return _path.empty();
}

Common::String ResourceReference::describe() const {
	Common::String desc;

	for (uint i = 0; i < _path.size(); i++) {
		desc += _path[i].describe();

		if (i != _path.size() - 1) {
			desc += " ";
		}
	}

	return desc;
}

void ResourceReference::buildFromResource(Resources::Object *resource) {
	Common::Array<PathElement> reversePath;
	while (resource && resource->getType() != Resources::Type::kRoot) {
		reversePath.push_back(PathElement(resource->getType(), resource->getIndex()));

		switch (resource->getType().get()) {
			case Resources::Type::kLocation: {
				Resources::Location *location = Resources::Object::cast<Resources::Location>(resource);
				resource = StarkResourceProvider->getLevelFromLocation(location);
				break;
			}
			default:
				resource = resource->findParent<Resources::Object>();
				break;
		}
	}

	_path.clear();
	for (int i = reversePath.size() - 1; i >= 0; i--) {
		_path.push_back(reversePath[i]);
	}
}

void ResourceReference::loadFromStream(Common::ReadStream *stream) {
	_path.clear();

	uint32 pathSize = stream->readUint32LE();
	for (uint i = 0; i < pathSize; i++) {
		byte rawType = stream->readByte();
		Resources::Type type = Resources::Type((Resources::Type::ResourceType) (rawType));
		uint16 index = stream->readUint16LE();

		addPathElement(type, index);
	}
}

void ResourceReference::saveToStream(Common::WriteStream *stream) {
	stream->writeUint32LE(_path.size());
	for (uint i = 0; i < _path.size(); i++) {
		byte rawType = _path[i].getType().get();
		uint16 index = _path[i].getIndex();

		stream->writeByte(rawType);
		stream->writeUint16LE(index);
	}
}

} // End of namespace Stark
