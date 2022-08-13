/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "tetraedge/te/te_resource.h"
#include "tetraedge/te/te_resource_manager.h"

namespace Tetraedge {

TeResourceManager::TeResourceManager() {
}

TeResourceManager::~TeResourceManager() {
	// Remove resources one at a time as they may be inter-dependant,
	// causing removals during iteration.
	while (_resources.size()) {
		_resources.erase(_resources.end() - 1);
	}
}

void TeResourceManager::addResource(const TeIntrusivePtr<TeResource> &resource) {
	_resources.insert_at(0, resource);
}

void TeResourceManager::addResource(TeResource *resource) {
	_resources.insert_at(0, TeIntrusivePtr<TeResource>(resource));
}

bool TeResourceManager::exists(const Common::Path &path) {
	Common::String pathstr = path.toString();
	for (auto &resource : _resources) {
		if (resource->getAccessName() == pathstr)
			return true;
	}
	return false;
}

void TeResourceManager::removeResource(const TeIntrusivePtr<TeResource> &resource) {
	for (uint i = 0; i < _resources.size(); i++) {
		if (_resources[i] == resource) {
			_resources.remove_at(i);
			break;
		}
	}
}

void TeResourceManager::removeResource(const TeResource *resource) {
	// We want to hold a new reference before removing the old one, as
	// this could cause the object's destruction - which triggers it
	// being removed from the list.
	TeIntrusivePtr<TeResource> ptr;
	uint i = 0;
	for (; i < _resources.size(); i++) {
		if (_resources[i].get() == resource) {
			ptr = _resources[i];
			break;
		}
	}
	if (i < _resources.size())
		_resources.remove_at(i);

	// now we can let the other pointer go.  It could cause another
	// removeResource request but now it's not in the list any more.
}

} // end namespace Tetraedge
