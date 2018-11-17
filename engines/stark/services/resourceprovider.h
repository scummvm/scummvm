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

#ifndef STARK_SERVICES_RESOURCE_PROVIDER_H
#define STARK_SERVICES_RESOURCE_PROVIDER_H

#include "common/list.h"

#include "engines/stark/resourcereference.h"

namespace Stark {

namespace Resources {
class Level;
class Location;
class Object;
}

class ArchiveLoader;
class Current;
class Global;
class StateProvider;

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
	bool hasLocationChangeRequest() const { return _locationChangeRequest; }

	void setShouldRestoreCurrentState() { _restoreCurrentState = true; }

	void pushAndChangeLocation(int16 level, int16 location);
	void returnToPushedLocation();

	/** Save and restore the previous location stack */
	void readLocationStack(Common::SeekableReadStream *stream, uint32 version);
	void writeLocationStack(Common::WriteStream *stream);

	/**
	 * Apply a location change request.
	 *
	 * Update the global object with the new location.
	 * Perform the necessary resource lifecycle updates
	 */
	void performLocationChange();

	/** Set the initial position and direction for the next location change */
	void setNextLocationPosition(const ResourceReference &bookmark, int32 direction);

	/** Save the current location state to the state store. */
	void commitActiveLocationsState();

	/** Release the global and current resources */
	void shutdown();

	/** Obtain the root resource for a loaded level */
	Resources::Level *getLevel(uint16 level) const;

	/** Obtain the root resource for a loaded location */
	Resources::Location *getLocation(uint16 level, uint16 location) const;

	/** Get the parent level from a currently loaded location */
	Resources::Level *getLevelFromLocation(Resources::Location *location) const;

private:
	struct PreviousLocation {
		uint16 location;
		uint16 level;
		bool inventoryOpen;
	};

	void pushCurrentLocation();
	void popCurrentLocation();
	void saveLoadLocationStack(ResourceSerializer &serializer);
	Common::Array<PreviousLocation> _locationStack;

	typedef Common::List<Current *> CurrentList;

	Current *findLevel(uint16 level) const;
	Current *findLocation(uint16 level, uint16 location) const;

	void purgeOldLocations();

	void runLocationChangeScripts(Resources::Object *resource, uint32 scriptCallMode);
	void setAprilInitialPosition();
	void setScrollInitialPosition();

	Global *_global;
	ArchiveLoader *_archiveLoader;
	StateProvider *_stateProvider;

	bool _locationChangeRequest;
	bool _restoreCurrentState;

	CurrentList _locations;

	ResourceReference _nextPositionBookmarkReference;
	int32 _nextDirection;
};

} // End of namespace Stark

#endif // STARK_SERVICES_RESOURCE_PROVIDER_H
