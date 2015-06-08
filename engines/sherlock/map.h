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

#ifndef SHERLOCK_MAP_H
#define SHERLOCK_MAP_H

#include "sherlock/objects.h"
#include "sherlock/saveload.h"

namespace Sherlock {

class SherlockEngine;

class Map {
protected:
	SherlockEngine *_vm;

	Map(SherlockEngine *vm);
public:
	Point32 _overPos;
	Point32 _bigPos;
	int _charPoint, _oldCharPoint;
	bool _active;
	bool _frameChangeFlag;
public:
	static Map *init(SherlockEngine *vm);
	virtual ~Map() {}

	/**
	 * Show the map
	 */
	virtual int show() = 0;

	/**
	 * Synchronize the data for a savegame
	 */
	void synchronize(Serializer &s);
};

} // End of namespace Sherlock

#endif
