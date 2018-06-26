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
#include "common/array.h"

namespace Stark {

namespace Resources {
class Camera;
class Floor;
class GlobalItemTemplate;
class ModelItem;
class KnowledgeSet;
class Level;
class Location;
class Root;
}

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

	Resources::Level *getLevel() const { return _level; }
	Resources::Location *getLocation() const { return _location; }
	Resources::Floor *getFloor() const { return _floor; }
	Resources::Camera *getCamera() const { return _camera; }
	Resources::ModelItem *getInteractive() const { return _interactive; }

	void setLevel(Resources::Level *level) { _level = level; }
	void setLocation(Resources::Location *location) { _location = location; }
	void setFloor(Resources::Floor *floor) { _floor = floor; }
	void setCamera(Resources::Camera *camera) { _camera = camera; }
	void setInteractive(Resources::ModelItem *interactive) { _interactive = interactive; }

private:
	Resources::Level *_level;
	Resources::Location *_location;
	Resources::ModelItem *_interactive;
	Resources::Floor *_floor;
	Resources::Camera *_camera;
};

/**
 * Global resources holder object
 */
class Global {
public:
	Global();

	Resources::Root *getRoot() const { return _root; }
	Resources::Level *getLevel() const { return _level; }
	Current *getCurrent() const { return _current; }
	bool isFastForward() const { return _fastForward; }
	uint getMillisecondsPerGameloop() const { return _millisecondsPerGameloop; }
	Resources::GlobalItemTemplate *getApril() const { return _april; }
	Resources::KnowledgeSet *getInventory() const { return _inventory; }

	void setRoot(Resources::Root *root) { _root = root; }
	void setLevel(Resources::Level *level) { _level = level; }
	void setCurrent(Current *current) { _current = current; }
	void setFastForward() { _fastForward = true; }
	void setNormalSpeed() { _fastForward = false; }
	void setMillisecondsPerGameloop(uint millisecondsPerGameloop) { _millisecondsPerGameloop = millisecondsPerGameloop; }
	void setApril(Resources::GlobalItemTemplate *april) { _april = april; }
	void setInventory(Resources::KnowledgeSet *inventory) { _inventory = inventory; }

	/** Retrieve the current chapter number from the global resource tree */
	int32 getCurrentChapter();

	/** Check if the player has an inventory item using its name  */
	bool hasInventoryItem(const Common::String &itemName) const;

	/** Change the current chapter */
	void setCurrentChapter(int32 value);

	/** Get the name of a character by its id */
	Common::String getCharacterName(int32 id);

private:
	uint _millisecondsPerGameloop;
	Resources::Root *_root;
	Resources::Level *_level;
	Resources::KnowledgeSet *_inventory;
	Resources::GlobalItemTemplate *_april;
	Current *_current;
	bool _fastForward;
};

} // End of namespace Stark

#endif // STARK_SERVICES_GLOBAL_H
