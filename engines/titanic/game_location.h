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

#ifndef TITANIC_GAME_LOCATION_H
#define TITANIC_GAME_LOCATION_H

#include "titanic/support/simple_file.h"
#include "titanic/core/node_item.h"
#include "titanic/core/room_item.h"
#include "titanic/core/view_item.h"

namespace Titanic {

enum PassengerClass {
	UNCHECKED = 4, THIRD_CLASS = 3, SECOND_CLASS = 2, FIRST_CLASS = 1,
	NO_CLASS = 0
};

class CGameState;

class CGameLocation {
private:
	CGameState *_gameState;
	CViewItem *_view;

	int _roomNumber;
	int _nodeNumber;
	int _viewNumber;
public:
	CGameLocation(CGameState *owner);

	/**
	 * Save the data for the class to file
	 */
	void save(SimpleFile *file) const;

	/**
	 * Load the data for the class from file
	 */
	void load(SimpleFile *file);

	/**
	 * Set the current view
	 */
	void setView(CViewItem *view);

	/**
	 * Get the current view
	 */
	CViewItem *getView();

	/**
	 * Get the current node
	 */
	CNodeItem *getNode();

	/**
	 * Get the current room
	 */
	CRoomItem *getRoom();
};

} // End of namespace Titanic

#endif /* TITANIC_GAME_LOCATION_H */
