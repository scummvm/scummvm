/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
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
#ifndef ULTIMA4_GAME_OBJECT_H
#define ULTIMA4_GAME_OBJECT_H

#include "ultima/ultima4/core/coords.h"
#include "ultima/ultima4/map/map_tile.h"
#include "ultima/ultima4/core/types.h"

namespace Ultima {
namespace Ultima4 {

class Object;

typedef Std::deque<Object *> ObjectDeque;

enum ObjectMovementBehavior {
	MOVEMENT_FIXED,
	MOVEMENT_WANDER,
	MOVEMENT_FOLLOW_AVATAR,
	MOVEMENT_ATTACK_AVATAR
};

class Object {
public:
	enum Type {
		UNKNOWN,
		CREATURE,
		PERSON
	};

	Object(Type type = UNKNOWN) :
		_tile(0),
		_prevTile(0),
		_movementBehavior(MOVEMENT_FIXED),
		_objType(type),
		_focused(false),
		_visible(true),
		_animated(true) {
	}

	virtual ~Object() {}

	// Methods
	MapTile &getTile() {
		return _tile;
	}
	MapTile &getPrevTile() {
		return _prevTile;
	}
	const Coords &getCoords() const {
		return _coords;
	}
	const Coords &getPrevCoords() const {
		return _prevCoords;
	}
	ObjectMovementBehavior getMovementBehavior() const {
		return _movementBehavior;
	}
	Type getType() const {
		return _objType;
	}
	bool hasFocus() const {
		return _focused;
	}
	bool isVisible() const {
		return _visible;
	}
	bool isAnimated() const {
		return _animated;
	}

	void setTile(MapTile t) {
		_tile = t;
	}
	void setTile(Tile *t) {
		_tile = t->getId();
	}
	void setPrevTile(MapTile t) {
		_prevTile = t;
	}
	void setCoords(Coords c) {
		_prevCoords = _coords;
		_coords = c;
	}
	void setPrevCoords(Coords c) {
		_prevCoords = c;
	}
	void setMovementBehavior(ObjectMovementBehavior b) {
		_movementBehavior = b;
	}
	void setType(Type t) {
		_objType = t;
	}
	void setFocus(bool f = true) {
		_focused = f;
	}
	void setVisible(bool v = true) {
		_visible = v;
	}
	void setAnimated(bool a = true) {
		_animated = a;
	}

	void setMap(class Map *m);
	Map *getMap();
	void remove();  /**< Removes itself from any maps that it is a part of */

	bool setDirection(Direction d);

	void animateMovement();

	// Properties
protected:
	MapTile _tile, _prevTile;
	Coords _coords, _prevCoords;
	ObjectMovementBehavior _movementBehavior;
	Type _objType;
	Std::deque<class Map *> _maps;           /**< A list of maps this object is a part of */

	bool _focused;
	bool _visible;
	bool _animated;
};

} // End of namespace Ultima4
} // End of namespace Ultima

#endif
