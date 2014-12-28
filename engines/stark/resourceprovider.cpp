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

#include "engines/stark/resourceprovider.h"

#include "engines/stark/archiveloader.h"
#include "engines/stark/resources/root.h"
#include "engines/stark/resources/level.h"
#include "engines/stark/resources/location.h"

namespace Stark {

ResourceProvider::ResourceProvider(ArchiveLoader *archiveLoader, Global *global) :
		_archiveLoader(archiveLoader),
		_global(global),
		_locationChangeRequest(false) {
}

void ResourceProvider::initGlobal() {
	// Load the root archive
	_archiveLoader->load("x.xarc");

	// Set the root tree
	Root *root = static_cast<Root *>(_archiveLoader->useRoot("x.xarc"));
	_global->setRoot(root);

	// Find the global level node
	Level *global = static_cast<Level *>(root->findChild(ResourceType::kLevel, 1));

	// Load the global archive
	Common::String globalArchiveName = _archiveLoader->buildArchiveName(global);
	_archiveLoader->load(globalArchiveName);

	// Set the global tree
	global = static_cast<Level *>(_archiveLoader->useRoot(globalArchiveName));
	_global->setLevel(global);

	//TODO: Retrieve the inventory and April from the global tree
}

Current *ResourceProvider::findLevel(uint16 level) {
	for (CurrentList::const_iterator it = _locations.begin(); it != _locations.end(); it++) {
		if ((*it)->getLevel()->getIndex() == level) {
			return *it;
		}
	}

	return nullptr;
}

Current *ResourceProvider::findLocation(uint16 level, uint16 location) {
	for (CurrentList::const_iterator it = _locations.begin(); it != _locations.end(); it++) {
		if ((*it)->getLevel()->getIndex() == level
				&& (*it)->getLocation()->getIndex() == location) {
			return *it;
		}
	}

	return nullptr;
}

void ResourceProvider::requestLocationChange(uint16 level, uint16 location) {
	Current *currentLocation = new Current();

	// Retrieve the level archive name
	Root *root = _global->getRoot();
	Level *rootLevelResource = (Level *) root->findChildWithIndex(ResourceType::kLevel, -1, level);
	Common::String levelArchive = _archiveLoader->buildArchiveName(rootLevelResource);

	// Load the archive, and get the resource sub-tree root
	_archiveLoader->load(levelArchive);
	currentLocation->setLevel(static_cast<Level *>(_archiveLoader->useRoot(levelArchive)));

	// Retrieve the location archive name
	Level *levelResource = currentLocation->getLevel();
	Location *levelLocationResource = (Location *) levelResource->findChildWithIndex(ResourceType::kLocation, -1, location);
	Common::String locationArchive = _archiveLoader->buildArchiveName(levelResource, levelLocationResource);

	// Load the archive, and get the resource sub-tree root
	_archiveLoader->load(locationArchive);
	currentLocation->setLocation(static_cast<Location *>(_archiveLoader->useRoot(locationArchive)));

	_locations.push_back(currentLocation);

	_locationChangeRequest = true;
}

void ResourceProvider::performLocationChange() {
	if (_global->getCurrent()) {
		// Exit the previous location
		Current *previous = _global->getCurrent();

		// Resource lifecycle update
		previous->getLocation()->onExitLocation();
		previous->getLevel()->onExitLocation();
		_global->getLevel()->onExitLocation();
	}

	// Set the new current location
	Current *current = _locations.back();
	_global->setCurrent(current);

	// Resource lifecycle update
	_global->getLevel()->onExitLocation();
	current->getLevel()->onExitLocation();
	current->getLocation()->onExitLocation();

	purgeOldLocations();

	_locationChangeRequest = false;
}

void ResourceProvider::purgeOldLocations() {
	while (_locations.size() >= 2) {
		Current *location = _locations.front();

		_archiveLoader->returnRoot(_archiveLoader->buildArchiveName(location->getLevel(), location->getLocation()));
		_archiveLoader->returnRoot(_archiveLoader->buildArchiveName(location->getLevel()));

		delete location;

		_locations.pop_front();
	}

	_archiveLoader->unloadUnused();
}

void ResourceProvider::shutdown() {
	// Flush the locations list
	for (CurrentList::const_iterator it = _locations.begin(); it != _locations.end(); it++) {
		Current *location = *it;

		_archiveLoader->returnRoot(_archiveLoader->buildArchiveName(location->getLevel(), location->getLocation()));
		_archiveLoader->returnRoot(_archiveLoader->buildArchiveName(location->getLevel()));

		delete location;
	}
	_locations.clear();

	// Return the global resources
	_archiveLoader->returnRoot(_archiveLoader->buildArchiveName(_global->getLevel()));
	_archiveLoader->returnRoot("x.xarc");

	_global->setLevel(nullptr);
	_global->setRoot(nullptr);

	_archiveLoader->unloadUnused();
}

} // End of namespace Stark
