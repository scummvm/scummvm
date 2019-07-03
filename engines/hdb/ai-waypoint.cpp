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
	for (int i = 0; i < _numWaypoints; i++)
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
	for (i = 0; i < _numWaypoints; i++) {
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

bool AI::traceStraightPath(int x1, int y1, int *x2, int *y2, int *level) {
	int	xVel, yVel, ok, entOK;
	AIEntity *e;

	// this checks to make sure we're only going vert or horz
	if (x1 != *x2 && y1 != *y2)
		return false;

	// this sets a -1, 0, or 1 step value
	xVel = *x2 - x1;
	if (xVel < 0)
		xVel = -1;
	if (xVel > 0)
		xVel = 1;

	yVel = *y2 - y1;
	if (yVel < 0)
		yVel = -1;
	if (yVel > 0)
		yVel = 1;

	while (1) {
		// clear tile ahead?
		entOK = ok = 0;
		uint32 flags = g_hdb->_map->getMapBGTileFlags(x1, y1);
		if (flags & kFlagStairTop)
			*level = 2;
		else if (flags & kFlagStairBot)
			*level = 1;

		// Floor level 1
		if (*level < 2) {
			ok = !(flags & (kFlagPlayerBlock | kFlagMonsterBlock));
			// if it's blocking, is it rad or plasma? (might be melted stuff on it)
			if (!ok) {
				ok = ((flags & kFlagPlasmaFloor) == kFlagPlasmaFloor) +
					((flags & kFlagPlasmaFloor) == kFlagRadFloor);
				e = findEntity(x1, y1);
				if (e && g_hdb->_ai->walkThroughEnt(e->type))
					entOK = 1;
				else if (ok && e && (e->state == STATE_FLOATING || e->state == STATE_MELTED || e == _player))
					entOK = ok = 1;
				else
					ok = 0;
			} else if (ok &&
				((flags & kFlagWater) == kFlagWater ||
				(flags & kFlagSlime) == kFlagSlime)) {
				// if it's non-blocking, is there water or slime?
				e = findEntity(x1, y1);
				if (e && g_hdb->_ai->walkThroughEnt(e->type))
					entOK = 1;
				else
					if (e && (e->state == STATE_FLOATING || e->state == STATE_MELTED || e == _player))
						entOK = ok = 1;
					else
						ok = 0;
				}
		} else {
			// Floor level 2
			if (g_hdb->_map->getMapFGTileIndex(x1, y1) >= 0)			// is there a foregnd tile? its flags take precedence on Level 2
				ok = !(g_hdb->_map->getMapFGTileFlags(x1, y1) & (kFlagPlayerBlock | kFlagMonsterBlock));
			else {
				flags = g_hdb->_map->getMapBGTileFlags(x1, y1);
				ok = !(flags & (kFlagPlayerBlock | kFlagMonsterBlock));
				// if it's blocking, is it rad or plasma? (might be melted stuff on it)
				if (!ok) {
					ok = ((flags & kFlagPlasmaFloor) == kFlagPlasmaFloor) +
						((flags & kFlagPlasmaFloor) == kFlagRadFloor);
					e = findEntity(x1, y1);
					if (e && g_hdb->_ai->walkThroughEnt(e->type))
						entOK = 1;
					else if (ok && e && (e->state == STATE_FLOATING || e->state == STATE_MELTED || e == _player))
							entOK = ok = 1;
					else
						ok = 0;
				} else if (ok &&
					((flags & kFlagWater) == kFlagWater ||
					(flags & kFlagSlime) == kFlagSlime)) {
					// if it's non-blocking, is there water or slime?
					e = findEntity(x1, y1);
					if (e && g_hdb->_ai->walkThroughEnt(e->type))
						entOK = 1;
					else
						if (e && (e->state == STATE_FLOATING || e->state == STATE_MELTED || e == _player))
							entOK = ok = 1;
						else
							ok = 0;
				}
			}
		}

		if (ok) {
			e = findEntity(x1, y1);
			if (e == _player)
				e = NULL;
			else if (g_hdb->_map->laserBeamExist(x1, y1)) {
				*x2 = x1 - xVel;
				*y2 = y1 - yVel;
				return true;
			} else if (e && (e->level != _player->level) && (g_hdb->_map->getMapFGTileFlags(e->tileX, e->tileY) & kFlagGrating)) {
				// on the same level????
				entOK = 1;
			}

			if (e && !entOK) {
				if (g_hdb->_ai->walkThroughEnt(e->type)) {
					// yes! are we at desired location?
					if (x1 == *x2 && y1 == *y2)
						return true;
				} else {
					// solid tile! back up one and return!
					*x2 = x1 - xVel;
					*y2 = y1 - yVel;
					return true;
				}
			} else if (x1 == *x2 && y1 == *y2) {
				// yes! are we at desired location?
				return true;
			}
		} else	{
			// solid tile! back up one and return!
			*x2 = x1 - xVel;
			*y2 = y1 - yVel;

			return true;
		}

		x1 += xVel;
		y1 += yVel;
	}

	return true;
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

void AI::drawWayPoints() {
	debug(9, "STUB: AI::drawWayPoints()");
}

} // End of Namespace
