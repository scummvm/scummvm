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

#ifndef STARK_SERVICES_GLOBAL_H
#define STARK_SERVICES_GLOBAL_H

#include "common/scummsys.h"

namespace Stark {

class Camera;
class Floor;
class ItemSub1;
class ItemSub10;
class Level;
class Location;
class Root;

/**
 * Current level / location holder object
 */
class Current {
public:
	Current() :
		_level(nullptr),
		_location(nullptr),
		_floor(nullptr),
		_camera(nullptr),
		_interactive(nullptr) {
	}

	Level *getLevel() const { return _level; }
	Location *getLocation() const { return _location; }
	Floor *getFloor() const { return _floor; }
	Camera *getCamera() const { return _camera; }
	ItemSub10 *getInteractive() const { return _interactive; }

	void setLevel(Level *level) { _level = level; }
	void setLocation(Location *location) { _location = location; }
	void setFloor(Floor *floor) { _floor = floor; }
	void setCamera(Camera *camera) { _camera = camera; }
	void setInteractive(ItemSub10 *interactive) { _interactive = interactive; }

private:
	Level *_level;
	Location *_location;
	ItemSub10 *_interactive;
	Floor *_floor;
	Camera *_camera;
};

/**
 * Global resources holder object
 */
class Global {
public:
	Global();

	Root *getRoot() const { return _root; }
	Level *getLevel() const { return _level; }
	Current *getCurrent() const { return _current; }
	bool isDebug() const { return _debug; }
	bool isFastForward() const { return _fastForward; }
	uint getMillisecondsPerGameloop() const { return _millisecondsPerGameloop; }
	ItemSub1 *getApril() const { return _april; }

	void setRoot(Root *root) { _root = root; }
	void setLevel(Level *level) { _level = level; }
	void setCurrent(Current *current) { _current = current; }
	void setDebug(bool debug) { _debug = debug; }
	void setFastForward(bool fastForward) { _fastForward = fastForward; }
	void setMillisecondsPerGameloop(uint millisecondsPerGameloop) { _millisecondsPerGameloop = millisecondsPerGameloop; }
	void setApril(ItemSub1 *april) { _april = april; }

	/** Retrieve the current chapter number from the global resource tree */
	int32 getCurrentChapter();

	/** Change the current chapter */
	void setCurrentChapter(int32 value);
private:
    uint _millisecondsPerGameloop;
    Root *_root;
    Level *_level;
    /* Inventory *_inventory; */
    ItemSub1 *_april;
    Current *_current;
    bool _debug;
    bool _fastForward;
};

} // End of namespace Stark

#endif // STARK_SERVICES_GLOBAL_H
