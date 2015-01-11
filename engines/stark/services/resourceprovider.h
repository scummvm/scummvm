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

#ifndef STARK_RESOURCE_PROVIDER_H
#define STARK_RESOURCE_PROVIDER_H

#include "common/list.h"

namespace Stark {

class Camera;
class Floor;
class Level;
class Location;
class Resource;
class Root;

class ArchiveLoader;
class StateProvider;

/**
 * Current level / location holder object
 */
class Current {
public:
	Current() :
		_level(nullptr),
		_location(nullptr),
		_floor(nullptr),
		_camera(nullptr) {
	}

	Level *getLevel() const { return _level; }
	Location *getLocation() const { return _location; }
	Floor *getFloor() const { return _floor; }
	Camera *getCamera() const { return _camera; }

	void setLevel(Level *level) { _level = level; }
	void setLocation(Location *location) { _location = location; }
	void setFloor(Floor *floor) { _floor = floor; }
	void setCamera(Camera *camera) { _camera = camera; }

private:
	Level *_level;
	Location *_location;
	/* Item *_interactive; */
	Floor *_floor;
	Camera *_camera;
};

/**
 * Global resources holder object
 */
class Global {
public:
	Global() :
		_millisecondsPerGameloop(0),
		_root(nullptr),
		_level(nullptr),
		_current(nullptr),
		_debug(false),
		_fastForward(false) {
	}

	Root *getRoot() const { return _root; }
	Level *getLevel() const { return _level; }
	Current *getCurrent() const { return _current; }
	bool isDebug() const { return _debug; }
	bool isFastForward() const { return _fastForward; }
	uint getMillisecondsPerGameloop() const { return _millisecondsPerGameloop; }

	void setRoot(Root *root) { _root = root; }
	void setLevel(Level *level) { _level = level; }
	void setCurrent(Current *current) { _current = current; }
	void setDebug(bool debug) { _debug = debug; }
	void setFastForward(bool fastForward) { _fastForward = fastForward; }
	void setMillisecondsPerGameloop(uint millisecondsPerGameloop) { _millisecondsPerGameloop = millisecondsPerGameloop; }

private:
    uint _millisecondsPerGameloop;
    Root *_root;
    Level *_level;
    /* Inventory *_inventory; */
    /* ItemVis3D *_april; */
    Current *_current;
    bool _debug;
    bool _fastForward;
};

/**
 * Game Resource provider.
 *
 * Maintains a list of resource trees.
 * Maintained trees are the global and the current ones.
 */
class ResourceProvider {
public:
	ResourceProvider(ArchiveLoader *archiveLoader, StateProvider *stateProvider, Global *global);

	/** Load the global archives and fill the global object */
	void initGlobal();

	/** Load the resources for the specified location */
	void requestLocationChange(uint16 level, uint16 location);

	/** Is a location change pending? */
	bool hasLocationChangeRequest() { return _locationChangeRequest; }

	void setShouldRestoreCurrentState() { _restoreCurrentState = true; }

	/**
	 * Apply a location change request.
	 *
	 * Update the global object with the new location.
	 * Perform the necessary resource lifecycle updates
	 */
	void performLocationChange();

	/** Save the current location state to the state store. */
	void commitActiveLocationsState();

	/** Release the global and current resources */
	void shutdown();

	/** Obtain the root resource for a loaded level */
	Level *getLevel(uint16 level);

	/** Obtain the root resource for a loaded location */
	Location *getLocation(uint16 level, uint16 location);

private:
	typedef Common::List<Current *> CurrentList;

	Current *findLevel(uint16 level);
	Current *findLocation(uint16 level, uint16 location);

	void purgeOldLocations();

	void runLocationChangeScripts(Resource *resource, uint32 scriptCallMode);

	Global *_global;
	ArchiveLoader *_archiveLoader;
	StateProvider *_stateProvider;

	bool _locationChangeRequest;
	bool _restoreCurrentState;

	CurrentList _locations;
};

} // End of namespace Stark

#endif // STARK_RESOURCE_PROVIDER_H
