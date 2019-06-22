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

#include "hdb/hdb.h"

namespace HDB {

void AI::addWaypoint(int px, int py, int x, int y, int level) {
	if (_numWaypoints >= kMaxWaypoints || (playerOnIce() && _player->goalX))
		return;

	// Check for duplicates
	for (int i = 0;i < _numWaypoints;i++)
		if (_waypoints[i].x == x && _waypoints[i].y == y)
			return;

	// Check if path is clear
	int nx = x;
	int ny = y;
	if (!_numWaypoints) {
		warning("STUB: addWaypoint: Setup First Waypoint");
	} else {
		warning("STUB: addWaypoint: Setup additional Waypoints");
	}

	if (_numWaypoints < kMaxWaypoints) {
		_waypoints[_numWaypoints].x = nx;
		_waypoints[_numWaypoints].y = ny;
		_waypoints[_numWaypoints].level = level;
		_numWaypoints++;
		warning("STUB: addWaypoint: Play SND_MENU_SLIDER");
	}
}

void AI::removeFirstWaypoint() {
	if (!_numWaypoints)
		return;

	int i;
	for (i = 0;i < _numWaypoints;i++) {
		_waypoints[i] = _waypoints[i + 1];
	}

	_waypoints[i].x = 0;
	_waypoints[i].y = 0;
	_waypoints[i].level = 0;
	_numWaypoints--;
}

void AI::clearWaypoints() {
	memset(&_waypoints[0], 0, sizeof(_waypoints));
	_numWaypoints = 0;
}

Tile *AI::getStandFrameDir(AIEntity *e) {
	switch (e->dir) {
	case DIR_DOWN:
		if (e->standdownFrames)
			return e->standdownGfx[0];
		else if (e->movedownFrames)
			return e->movedownGfx[0];
		break;
	case DIR_UP:
		if (e->standupFrames)
			return e->standupGfx[0];
		else if (e->moveupFrames)
			return e->moveupGfx[0];
		break;
	case DIR_LEFT:
		if (e->standleftFrames)
			return e->standleftGfx[0];
		else if (e->moveleftFrames)
			return e->moveleftGfx[0];
		break;
	case DIR_RIGHT:
		if (e->standrightFrames)
			return e->standrightGfx[0];
		else if (e->moverightFrames)
			return e->moverightGfx[0];
		break;
	case DIR_NONE:
		warning("AI-WAYPOINT: getStandFrameDir: DIR_NONE found");
		break;
	}
	return e->standdownGfx[0];
}

} // End of Namespace
