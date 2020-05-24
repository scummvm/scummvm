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

#ifndef ULTIMA_SHARED_MAPS_MAP_WIDGET_H
#define ULTIMA_SHARED_MAPS_MAP_WIDGET_H

#include "common/events.h"
#include "common/ptr.h"
#include "common/serializer.h"
#include "common/str.h"
#include "ultima/shared/core/message_target.h"
#include "ultima/shared/core/rect.h"

namespace Ultima {
namespace Shared {

class Game;
	
namespace Maps {

enum Direction {
	DIR_NONE = 0,
	DIR_LEFT = 1, DIR_RIGHT = 2, DIR_UP = 3, DIR_DOWN = 4,
	DIR_WEST = 1, DIR_EAST = 2, DIR_NORTH = 3, DIR_SOUTH = 4
};

class MapBase;

/**
 * Base class for things that appear within a map, such as monsters, transports, or people
 */
class MapWidget : public MessageTarget {
	DECLARE_MESSAGE_MAP;
protected:
	Game *_game;						// Game reference
	MapBase *_map;						// Map reference
public:
	Point _position;					// Position within the map
	Direction _direction;				// Direction
	Common::String _name;				// Name of widget
public:
	/**
	 * Support method to get a direction from a keycode
	 */
	static Direction directionFromKey(Common::KeyCode keycode);
public:
	CLASSDEF;

	/**
	 * Constructor
	 */
	MapWidget(Game *game, MapBase *map) : _game(game), _map(map), _direction(DIR_NONE) {}
	MapWidget(Game *game, MapBase *map, const Point &pt, Direction dir = DIR_NONE) : _game(game), _map(map), _position(pt), _direction(dir) {}
	MapWidget(Game *game, MapBase *map, const Common::String &name, const Point &pt, Direction dir = DIR_NONE) :
		_game(game), _map(map), _name(name), _position(pt), _direction(dir) {}

	/**
	 * Destructor
	 */
	~MapWidget() override {}

	/**
	 * Return a name for a widget class if it can be synchronized to savegames
	 */
	virtual const char *getClassName() const { return nullptr; }

	/**
	 * Handles loading and saving game data
	 */
	virtual void synchronize(Common::Serializer &s);

	/**
	 * Adds a text string to the info area
	 * @param text			Text to add
	 * @param newLine		Whether to apply a newline at the end
	 * @param replaceLine	If true, replaces the current last line
	 */
	void addInfoMsg(const Common::String &text, bool newLine = true, bool replaceLine = false);

	/**
	 * Get the tile for the widget
	 */
	virtual uint getTileNum() const { return 0; }

	/**
	 * Returns true if the player can move onto a tile the widget occupies
	 */
	virtual bool isBlocking() const { return true; }

	/**
	 * Called to update the widget at the end of a turn
	 * @param isPreUpdate		Update is called twice in succesion during the end of turn update.
	 *		Once with true for all widgets, then with it false
	 */
	virtual void update(bool isPreUpdate) {}

	enum CanMove { UNSET = 0, YES = 1, NO = 2 };

	/**
	 * Returns true if the given widget can move to a given position on the map
	 */
	virtual CanMove canMoveTo(const Point &destPos);

	/**
	 * Moves the widget to a given position
	 * @param destPos		Specified new position
	 * @param dir			Optional explicit direction to set. If not specified,
	 *		the direction will be set relative to the position moved from
	 */
	virtual void moveTo(const Point &destPos, Direction dir = DIR_NONE);
};

typedef Common::SharedPtr<MapWidget> MapWidgetPtr;

} // End of namespace Maps
} // End of namespace Shared
} // End of namespace Ultima

#endif
