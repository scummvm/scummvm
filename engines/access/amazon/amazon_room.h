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

#ifndef ACCESS_AMAZON_ROOM_H
#define ACCESS_AMAZON_ROOM_H

#include "common/scummsys.h"
#include "access/room.h"

namespace Access {

class AccessEngine;

namespace Amazon {

class AmazonEngine;

class AmazonRoom : public Room {
private:
	AmazonEngine *_game;
	bool _antOutFlag;
	const byte *_icon;

	void roomSet();
protected:
	virtual void loadRoom(int roomNumber);

	virtual void reloadRoom();

	virtual void reloadRoom1();

	virtual void setupRoom();

	virtual void mainAreaClick();

	virtual void clearRoom();

	virtual void walkCursor();
public:
	AmazonRoom(AccessEngine *vm);

	virtual ~AmazonRoom();

	virtual void init4Quads();

	virtual void roomMenu();
};

} // End of namespace Amazon

} // End of namespace Access

#endif /* ACCESS_AMAZON_ROOM_H */
