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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ACCESS_ROOM_H
#define ACCESS_ROOM_H

#include "common/scummsys.h"

namespace Access {

class AccessEngine;

class Room {
private:
	void roomLoop();
protected:
	AccessEngine *_vm;

	void setupRoom();
	void setWallCodes();
	void buildScreen();

	virtual void reloadRoom() = 0;

	virtual void reloadRoom1() = 0;
public:
	int _function;
	int _roomFlag;
public:
	Room(AccessEngine *vm);

	void doRoom();

	void doCommands();

	/**
	 * Clear all the data used by the room
	 */
	void clearRoom();

	void loadRoom(int room);
};

} // End of namespace Access

#endif /* ACCESS_ROOM_H */
