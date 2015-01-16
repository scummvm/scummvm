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

#include "engines/stark/services/resourceprovider.h"

#include "engines/stark/resources/bookmark.h"
#include "engines/stark/resources/camera.h"
#include "engines/stark/resources/floor.h"
#include "engines/stark/resources/item.h"
#include "engines/stark/resources/layer.h"
#include "engines/stark/resources/level.h"
#include "engines/stark/resources/location.h"
#include "engines/stark/resources/root.h"
#include "engines/stark/resources/script.h"
#include "engines/stark/services/archiveloader.h"
#include "engines/stark/services/global.h"
#include "engines/stark/services/stateprovider.h"

namespace Stark {

ResourceProvider::ResourceProvider(ArchiveLoader *archiveLoader, StateProvider *stateProvider, Global *global) :
		_archiveLoader(archiveLoader),
		_stateProvider(stateProvider),
		_global(global),
		_locationChangeRequest(false),
		_restoreCurrentState(false),
		_nextDirection(0) {
}

void ResourceProvider::initGlobal() {
	// Load the root archive
	_archiveLoader->load("x.xarc");

	// Set the root tree
	Root *root = _archiveLoader->useRoot<Root>("x.xarc");
	_global->setRoot(root);

	// Resource lifecycle update
	root->onAllLoaded();

	// Find the global level node
	Level *global = root->findChildWithSubtype<Level>(1);

	// Load the global archive
	Common::String globalArchiveName = _archiveLoader->buildArchiveName(global);
	_archiveLoader->load(globalArchiveName);

	// Set the global tree
	global = _archiveLoader->useRoot<Level>(globalArchiveName);
	_stateProvider->restoreLevelState(global);
	_global->setLevel(global);

	// Resource lifecycle update
	global->onAllLoaded();

	//TODO: Retrieve the inventory from the global tree
	_global->setApril(global->findChildWithSubtype<ItemSub1>(Item::kItemSub1));
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

Level *ResourceProvider::getLevel(uint16 level) {
	Current *current = findLevel(level);

	if (current) {
		return current->getLevel();
	}

	return nullptr;
}

Location *ResourceProvider::getLocation(uint16 level, uint16 location) {
	Current *current = findLocation(level, location);

	if (current) {
		return current->getLocation();
	}

	return nullptr;
}

void ResourceProvider::requestLocationChange(uint16 level, uint16 location) {
	Current *currentLocation = new Current();
	_locations.push_back(currentLocation);

	// Retrieve the level archive name
	Root *root = _global->getRoot();
	Level *rootLevelResource = root->findChildWithIndex<Level>(level);
	Common::String levelArchive = _archiveLoader->buildArchiveName(rootLevelResource);

	// Load the archive, and get the resource sub-tree root
	bool newlyLoaded = _archiveLoader->load(levelArchive);
	currentLocation->setLevel(_archiveLoader->useRoot<Level>(levelArchive));

	// If we just loaded a resource tree, restore its state
	if (newlyLoaded) {
		_stateProvider->restoreLevelState(currentLocation->getLevel());
		currentLocation->getLevel()->onAllLoaded();
	}

	// Retrieve the location archive name
	Level *levelResource = currentLocation->getLevel();
	Location *levelLocationResource = levelResource->findChildWithIndex<Location>(location);
	Common::String locationArchive = _archiveLoader->buildArchiveName(levelResource, levelLocationResource);

	// Load the archive, and get the resource sub-tree root
	newlyLoaded = _archiveLoader->load(locationArchive);
	currentLocation->setLocation(_archiveLoader->useRoot<Location>(locationArchive));

	if (currentLocation->getLocation()->has3DLayer()) {
		Layer3D *layer = currentLocation->getLocation()->findChildWithSubtype<Layer3D>(Layer::kLayer3D);
		currentLocation->setFloor(layer->findChild<Floor>());
		currentLocation->setCamera(layer->findChild<Camera>());
	} else {
		currentLocation->setFloor(nullptr);
		currentLocation->setCamera(nullptr);
	}

	// If we just loaded a resource tree, restore its state
	if (newlyLoaded) {
		_stateProvider->restoreLocationState(currentLocation->getLevel(), currentLocation->getLocation());
		currentLocation->getLocation()->onAllLoaded();
	}

	_locationChangeRequest = true;
}

void ResourceProvider::performLocationChange() {
	if (_global->getCurrent()) {
		// Exit the previous location
		Current *previous = _global->getCurrent();

		// Trigger location change scripts
		runLocationChangeScripts(previous->getLevel(), Script::kCallModeExitLocation);
		runLocationChangeScripts(previous->getLocation(), Script::kCallModeExitLocation);

		// Resource lifecycle update
		previous->getLocation()->onExitLocation();
		previous->getLevel()->onExitLocation();
		_global->getLevel()->onExitLocation();
	}

	// Set the new current location
	Current *current = _locations.back();
	_global->setCurrent(current);

	if (_restoreCurrentState) {
		_stateProvider->restoreGlobalState(_global->getLevel());
		_stateProvider->restoreCurrentLevelState(current->getLevel());
		_stateProvider->restoreCurrentLocationState(current->getLevel(), current->getLocation());
		_restoreCurrentState = false;
	}

	// Resource lifecycle update
	_global->getLevel()->onEnterLocation();
	current->getLevel()->onEnterLocation();
	current->getLocation()->onEnterLocation();

	if (current->getLocation()->has3DLayer()) {
		// Fetch the scene item for April
		current->setInteractive(Resource::cast<ItemSub10>(_global->getApril()->getSceneInstance()));
	}

	setAprilInitialPosition();

	// Trigger location change scripts
	runLocationChangeScripts(current->getLevel(), Script::kCallModeEnterLocation);
	runLocationChangeScripts(current->getLocation(), Script::kCallModeEnterLocation);

	purgeOldLocations();

	_locationChangeRequest = false;
}

void ResourceProvider::runLocationChangeScripts(Resource *resource, uint32 scriptCallMode) {
	Common::Array<Script *> script = resource->listChildrenRecursive<Script>();

	if (scriptCallMode == Script::kCallModeEnterLocation) {
		for (uint i = 0; i < script.size(); i++) {
			script[i]->reset();
		}
	}

	for (uint i = 0; i < script.size(); i++) {
		script[i]->execute(scriptCallMode);
	}
}

void ResourceProvider::setNextLocationPosition(const ResourceReference &bookmark, int32 direction) {
	_nextPositionBookmarkReference = bookmark;
	_nextDirection = direction;
}

void ResourceProvider::setAprilInitialPosition() {
	if (_nextPositionBookmarkReference.empty()) {
		return; // No target location
	}

	Current *current = _global->getCurrent();
	ItemSub10 *april = current->getInteractive();
	if (!april) {
		return; // No character
	}

	// Set the initial location for April
	Bookmark *position = _nextPositionBookmarkReference.resolve<Bookmark>();

	april->placeOnBookmark(position);
	april->setDirection(_nextDirection);

	_nextPositionBookmarkReference = ResourceReference();
	_nextDirection = 0;
}

void ResourceProvider::purgeOldLocations() {
	while (_locations.size() >= 2) {
		Current *location = _locations.front();

		_stateProvider->saveLocationState(location->getLevel(), location->getLocation());
		_stateProvider->saveLevelState(location->getLevel());

		_archiveLoader->returnRoot(_archiveLoader->buildArchiveName(location->getLevel(), location->getLocation()));
		_archiveLoader->returnRoot(_archiveLoader->buildArchiveName(location->getLevel()));

		delete location;

		_locations.pop_front();
	}

	_archiveLoader->unloadUnused();
}

void ResourceProvider::commitActiveLocationsState() {
	// Save active location states
	for (CurrentList::const_iterator it = _locations.begin(); it != _locations.end(); it++) {
		_stateProvider->saveLocationState((*it)->getLevel(), (*it)->getLocation());
		_stateProvider->saveLevelState((*it)->getLevel());
	}

	_stateProvider->saveLevelState(_global->getLevel());

	// Save the current location "extended" state, to be able to restore them to the exact same state.
	Current *location = _global->getCurrent();
	_stateProvider->saveCurrentLocationState(location->getLevel(), location->getLocation());
	_stateProvider->saveCurrentLevelState(location->getLevel());

	_stateProvider->saveGlobalState(_global->getLevel());
}

void ResourceProvider::shutdown() {
	// Flush the locations list
	for (CurrentList::const_iterator it = _locations.begin(); it != _locations.end(); it++) {
		Current *location = *it;

		_stateProvider->saveLocationState(location->getLevel(), location->getLocation());
		_stateProvider->saveLevelState(location->getLevel());

		_archiveLoader->returnRoot(_archiveLoader->buildArchiveName(location->getLevel(), location->getLocation()));
		_archiveLoader->returnRoot(_archiveLoader->buildArchiveName(location->getLevel()));

		delete location;
	}
	_locations.clear();

	// Return the global resources
	_stateProvider->saveLevelState(_global->getLevel());

	_archiveLoader->returnRoot(_archiveLoader->buildArchiveName(_global->getLevel()));
	_archiveLoader->returnRoot("x.xarc");

	_global->setLevel(nullptr);
	_global->setRoot(nullptr);
	_global->setCurrent(nullptr);

	_archiveLoader->unloadUnused();
}

} // End of namespace Stark
