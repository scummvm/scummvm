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

#ifndef TITANIC_TIMER_H
#define TITANIC_TIMER_H

#include "titanic/core/list.h"

namespace Titanic {

class CProjectItem;

class CTimer : public ListItem {
private:
	static int _v1;
public:
	uint _id;
	bool _done;
	uint _field44;
public:
	CTimer();

	/**
	 * Called after loading a game has finished
	 */
	void postLoad(uint ticks, CProjectItem *project);

	/**
	 * Called when a game is about to be saved
	 */
	void preSave();

	/**
	 * Called when a game has finished being saved
	 */
	void postSave();

	bool update(uint ticks);

	void set44(uint val) { _field44 = val; }
};

class CTimerList : public List<CTimer> {
public:
	/**
	 * Called after loading a game has finished
	 */
	void postLoad(uint ticks, CProjectItem *project);

	/**
	 * Called when a game is about to be saved
	 */
	void preSave();

	/**
	 * Called when a game has finished being saved
	 */
	void postSave();

	/**
	 * Handles an update
	 */
	void update(uint ticks);

	/**
	 * Remove an item with the given Id
	 */
	void stop(uint id);

	void set44(uint id, uint val);
};

} // End of namespace Titanic

#endif /* TITANIC_TIMER_H */
