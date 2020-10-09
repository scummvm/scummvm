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
#include "engines/stark/resources/knowledgeset.h"
#include "engines/stark/resources/layer.h"
#include "engines/stark/resources/level.h"
#include "engines/stark/resources/location.h"
#include "engines/stark/resources/root.h"
#include "engines/stark/resources/script.h"
#include "engines/stark/resources/sound.h"

#include "engines/stark/services/services.h"
#include "engines/stark/services/archiveloader.h"
#include "engines/stark/services/global.h"
#include "engines/stark/services/stateprovider.h"
#include "engines/stark/services/userinterface.h"

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
	Resources::Root *root = _archiveLoader->useRoot<Resources::Root>("x.xarc");
	_global->setRoot(root);

	// Resources lifecycle update
	root->onAllLoaded();

	// Find the global level node
	Resources::Level *global = root->findChildWithSubtype<Resources::Level>(Resources::Level::kGlobal);

	// Load the global archive
	Common::String globalArchiveName = _archiveLoader->buildArchiveName(global);
	_archiveLoader->load(globalArchiveName);

	// Set the global tree
	global = _archiveLoader->useRoot<Resources::Level>(globalArchiveName);
	_global->setLevel(global);

	// Resources lifecycle update
	global->onAllLoaded();

	// Load the state
	_stateProvider->restoreLevelState(global);

	_global->setInventory(global->findChildWithSubtype<Resources::KnowledgeSet>(Resources::KnowledgeSet::kInventory));
	_global->setApril(global->findChildWithSubtype<Resources::GlobalItemTemplate>(Resources::Item::kItemGlobalTemplate));
}

Current *ResourceProvider::findLevel(uint16 level) const {
	for (CurrentList::const_iterator it = _locations.begin(); it != _locations.end(); it++) {
		if ((*it)->getLevel()->getIndex() == level) {
			return *it;
		}
	}

	return nullptr;
}

Current *ResourceProvider::findLocation(uint16 level, uint16 location) const {
	for (CurrentList::const_iterator it = _locations.begin(); it != _locations.end(); it++) {
		if ((*it)->getLevel()->getIndex() == level
				&& (*it)->getLocation()->getIndex() == location) {
			return *it;
		}
	}

	return nullptr;
}

Resources::Level *ResourceProvider::getLevel(uint16 level) const {
	Current *current = findLevel(level);

	if (current) {
		return current->getLevel();
	}

	return nullptr;
}

Resources::Location *ResourceProvider::getLocation(uint16 level, uint16 location) const {
	Current *current = findLocation(level, location);

	if (current) {
		return current->getLocation();
	}

	return nullptr;
}

void ResourceProvider::pushAndChangeLocation(int16 level, int16 location) {
	pushCurrentLocation();
	requestLocationChange(level, location);
}

void ResourceProvider::returnToPushedLocation() {
	popCurrentLocation();
}

void ResourceProvider::pushCurrentLocation() {
	PreviousLocation current;
	current.level = _global->getCurrent()->getLevel()->getIndex();
	current.location = _global->getCurrent()->getLocation()->getIndex();
	current.inventoryOpen = StarkUserInterface->isInventoryOpen();

	_locationStack.push_back(current);

	StarkUserInterface->inventoryOpen(false);
}

void ResourceProvider::popCurrentLocation() {
	if (_locationStack.empty()) {
		error("Empty location stack");
	} else {
		PreviousLocation previous = _locationStack.back();
		_locationStack.pop_back();

		requestLocationChange(previous.level, previous.location);
		StarkUserInterface->inventoryOpen(previous.inventoryOpen);
	}
}

void ResourceProvider::requestLocationChange(uint16 level, uint16 location) {
	Current *currentLocation = new Current();
	_locations.push_back(currentLocation);

	// Retrieve the level archive name
	Resources::Root *root = _global->getRoot();
	Resources::Level *rootLevelResource = root->findChildWithIndex<Resources::Level>(level);
	Common::String levelArchive = _archiveLoader->buildArchiveName(rootLevelResource);

	// Load the archive, and get the resource sub-tree root
	bool newlyLoaded = _archiveLoader->load(levelArchive);
	currentLocation->setLevel(_archiveLoader->useRoot<Resources::Level>(levelArchive));

	// If we just loaded a resource tree, restore its state
	if (newlyLoaded) {
		currentLocation->getLevel()->onAllLoaded();
		_stateProvider->restoreLevelState(currentLocation->getLevel());
	}

	// Retrieve the location archive name
	Resources::Level *levelResource = currentLocation->getLevel();
	Resources::Location *levelLocationResource = levelResource->findChildWithIndex<Resources::Location>(location);
	Common::String locationArchive = _archiveLoader->buildArchiveName(levelResource, levelLocationResource);

	// Load the archive, and get the resource sub-tree root
	newlyLoaded = _archiveLoader->load(locationArchive);
	currentLocation->setLocation(_archiveLoader->useRoot<Resources::Location>(locationArchive));

	if (currentLocation->getLocation()->has3DLayer()) {
		Resources::Layer3D *layer = currentLocation->getLocation()->findChildWithSubtype<Resources::Layer3D>(Resources::Layer::kLayer3D);
		currentLocation->setFloor(layer->findChild<Resources::Floor>());
		currentLocation->setCamera(layer->findChild<Resources::Camera>());
	} else {
		currentLocation->setFloor(nullptr);
		currentLocation->setCamera(nullptr);
	}

	// If we just loaded a resource tree, restore its state
	if (newlyLoaded) {
		currentLocation->getLocation()->onAllLoaded();
		_stateProvider->restoreLocationState(currentLocation->getLevel(), currentLocation->getLocation());
	}

	_locationChangeRequest = true;
}

void ResourceProvider::performLocationChange() {
	Current *current = _locations.back();
	Current *previous = _global->getCurrent();
	bool levelChanged = !previous || previous->getLevel() != current->getLevel();

	// Exit the previous location
	if (previous) {
		// Trigger location change scripts
		if (levelChanged) {
			runLocationChangeScripts(previous->getLevel(), Resources::Script::kCallModeExitLocation);
		}
		runLocationChangeScripts(previous->getLocation(), Resources::Script::kCallModeExitLocation);

		// Resources lifecycle update
		previous->getLocation()->onExitLocation();
		previous->getLevel()->onExitLocation();
		_global->getLevel()->onExitLocation();
	}

	// Clear all pointers to location objects in the UI instances
	StarkUserInterface->clearLocationDependentState();

	// Set the new current location
	_global->setCurrent(current);

	// Resources lifecycle update
	_global->getLevel()->onEnterLocation();
	current->getLevel()->onEnterLocation();
	current->getLocation()->onEnterLocation();

	if (current->getLocation()->has3DLayer()) {
		// Fetch the scene item for April
		current->setInteractive(Resources::Object::cast<Resources::ModelItem>(_global->getApril()->getSceneInstance()));
	}

	if (_restoreCurrentState) {
		_stateProvider->restoreGlobalState(_global->getLevel());
		_stateProvider->restoreCurrentLevelState(current->getLevel());
		_stateProvider->restoreCurrentLocationState(current->getLevel(), current->getLocation());
		_restoreCurrentState = false;
	} else {
		setAprilInitialPosition();
		setScrollInitialPosition();

		// Trigger location change scripts
		if (levelChanged) {
			runLocationChangeScripts(current->getLevel(), Resources::Script::kCallModeEnterLocation);
		}
		runLocationChangeScripts(current->getLocation(), Resources::Script::kCallModeEnterLocation);
	}

	current->getLocation()->resetAnimationBlending();
	purgeOldLocations();

	_locationChangeRequest = false;
}

void ResourceProvider::runLocationChangeScripts(Resources::Object *resource, uint32 scriptCallMode) {
	Common::Array<Resources::Script *> scripts = resource->listChildrenRecursive<Resources::Script>();

	if (scriptCallMode == Resources::Script::kCallModeEnterLocation) {
		for (uint i = 0; i < scripts.size(); i++) {
			scripts[i]->reset();
		}
	}

	for (uint i = 0; i < scripts.size(); i++) {
		scripts[i]->execute(scriptCallMode);
	}

	if (scriptCallMode == Resources::Script::kCallModeExitLocation) {
		Common::Array<Resources::Sound *> sounds = resource->listChildrenRecursive<Resources::Sound>();
		for (uint i = 0; i < sounds.size(); i++) {
			sounds[i]->stop();
		}
	}
}

void ResourceProvider::setNextLocationPosition(const ResourceReference &bookmark, int32 direction) {
	_nextPositionBookmarkReference = bookmark;
	_nextDirection = direction;
}

void ResourceProvider::setAprilInitialPosition() {
	Current *current = _global->getCurrent();
	Resources::ModelItem *april = current->getInteractive();
	if (!april) {
		return; // No character
	}

	// Set the initial position for April
	if (!_nextPositionBookmarkReference.empty()) {
		Resources::Bookmark *position = _nextPositionBookmarkReference.resolve<Resources::Bookmark>();
		april->placeOnBookmark(position);

		Resources::Camera *camera = current->getCamera();
		Math::Angle cameraAngle = camera->getHorizontalAngle();
		april->setDirection(_nextDirection + cameraAngle);
	} else if (april->getFloorFaceIndex() <= 0) {
		// No target location provided, place April on the first floor face
		april->placeDefaultPosition();
	}

	_nextPositionBookmarkReference = ResourceReference();
	_nextDirection = 0;
}

void ResourceProvider::setScrollInitialPosition() {
	Current *current = _global->getCurrent();
	Resources::Location *location = current->getLocation();
	location->scrollToCharacterImmediate();
}

void ResourceProvider::purgeOldLocations() {
	while (_locations.size() > 2) {
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
	_stateProvider->clear();

	_locationStack.clear();

	// Flush the locations list
	for (CurrentList::const_iterator it = _locations.begin(); it != _locations.end(); it++) {
		Current *location = *it;

		_archiveLoader->returnRoot(_archiveLoader->buildArchiveName(location->getLevel(), location->getLocation()));
		_archiveLoader->returnRoot(_archiveLoader->buildArchiveName(location->getLevel()));

		delete location;
	}
	_locations.clear();

	// Return the global resources
	if (_global->getLevel()) {
		_archiveLoader->returnRoot(_archiveLoader->buildArchiveName(_global->getLevel()));
		_global->setLevel(nullptr);
	}

	if (_global->getRoot()) {
		_archiveLoader->returnRoot("x.xarc");
		_global->setRoot(nullptr);
	}

	_global->setCurrent(nullptr);
	_global->setInventory(nullptr);
	_global->setApril(nullptr);

	_archiveLoader->unloadUnused();
}

Resources::Level *ResourceProvider::getLevelFromLocation(Resources::Location *location) const {
	for (CurrentList::const_iterator it = _locations.begin(); it != _locations.end(); it++) {
		if ((*it)->getLocation() == location) {
			return (*it)->getLevel();
		}
	}

	return nullptr;
}

void ResourceProvider::readLocationStack(Common::SeekableReadStream *stream, uint32 version) {
	ResourceSerializer serializer(stream, nullptr, version);
	saveLoadLocationStack(serializer);
}

void ResourceProvider::writeLocationStack(Common::WriteStream *stream) {
	ResourceSerializer serializer(nullptr, stream, StateProvider::kSaveVersion);
	saveLoadLocationStack(serializer);
}

void ResourceProvider::saveLoadLocationStack(ResourceSerializer &serializer) {
	serializer.syncArraySize(_locationStack, 12);

	for (uint i = 0; i < _locationStack.size(); i++) {
		serializer.syncAsUint16LE(_locationStack[i].level);
		serializer.syncAsUint16LE(_locationStack[i].location);
		serializer.syncAsUint32LE(_locationStack[i].inventoryOpen);
	}
}

} // End of namespace Stark
