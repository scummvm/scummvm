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
 *
 * Based on the original sources
 *   Faery Tale II -- The Halls of the Dead
 *   (c) 1993-1996 The Wyrmkeep Entertainment Co.
 */

#ifndef SAGA2_BEEGEE_H
#define SAGA2_BEEGEE_H

namespace Saga2 {

enum {
	kNoEnemy = -1,
	kMaxThemes = 16
};

/* ===================================================================== *
   Types
 * ===================================================================== */

//-----------------------------------------------------------------------
// Music selection brain

class Deejay {
private:
	int     _enemy;
	bool    _aggr;
	bool    _day;
	bool    _ugd;
	bool    _susp;

	int _current;
	int _currentID;

public:
	Deejay() {
		_enemy = -1;
		_aggr = false;
		_day = true;
		_susp = false;
		_ugd = false;
		_current = 0;
		_currentID = 0;
	}
	~Deejay() {}

private:
	void select(void);

public:
	void setEnemy(int16 enemyType = -1) {
		_enemy = enemyType;
		select();
	}
	void setAggression(bool aggressive) {
		_aggr = aggressive;
		select();
	}
	void setDaytime(bool daytime) {
		_day = daytime;
		select();
	}
	void setSuspend(bool suspended) {
		_susp = suspended;
		select();
	}
	void setWorld(bool underground) {
		_ugd = underground;
		select();
	}
};

} // end of namespace Saga2

#endif
