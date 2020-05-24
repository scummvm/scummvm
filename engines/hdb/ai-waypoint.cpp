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
#include "hdb/ai.h"
#include "hdb/gfx.h"
#include "hdb/map.h"
#include "hdb/sound.h"

namespace HDB {

void AI::addWaypoint(int px, int py, int x, int y, int level) {
	// at the max yet?
	if (_numWaypoints >= kMaxWaypoints || (playerOnIce() && _player->goalX))
		return;

	// first, let's see if this is a duplicate waypoint
	int i;
	for (i = 0; i < _numWaypoints; i++)
		if (_waypoints[i].x == x && _waypoints[i].y == y)
			return;

	// check to make sure the path is clear...
	// if it's not, don't add to waypoint list!
	// The destination x,y might be modified, so
	// we'll pass in the address to them...
	int nx = x;
	int ny = y;
	if (!_numWaypoints) {
		// if player is already moving and this is the first waypoint, forget it.
		// player is using the keyboard and must be fully stopped before laying
		// the first waypoint
		if (_player->goalX)
			return;

		// trace from player to new spot
		// return value of 0 only means a diagonal was selected!
		if (!traceStraightPath(px, py, &nx, &ny, &level)) {
			int	tx, ty, tx2, ty2;

			// it didn't work, so player is probably trying some diagonal movement.
			// let's break it down into two waypoints: one horz, one vert
			tx = nx;
			ty = py;
			tx2 = nx;
			ty2 = ny;
			int	lvl1, lvl2;

			lvl1 = lvl2 = level;

			if (traceStraightPath(px, py, &tx, &ty, &lvl1)) {
				if (tx != nx || ty != py)
					goto newpath;

				traceStraightPath(tx, ty, &tx2, &ty2, &lvl2);

				if (tx2 != nx || ty2 != ny)
					goto newpath;

				_waypoints[_numWaypoints].x = tx;
				_waypoints[_numWaypoints].y = ty;
				_waypoints[_numWaypoints].level = lvl1;
				_numWaypoints++;
				_waypoints[_numWaypoints].x = nx;
				_waypoints[_numWaypoints].y = ny;
				_waypoints[_numWaypoints].level = lvl2;
				_numWaypoints++;
				g_hdb->_sound->playSound(SND_MENU_SLIDER);

				if (onEvenTile(_player->x, _player->y))
					setEntityGoal(_player, tx, ty);
			} else {
newpath:
				tx = px;
				ty = ny;
				tx2 = nx;
				ty2 = ny;

				lvl1 = lvl2 = level;

				if (traceStraightPath(px, py, &tx, &ty, &lvl1)) {
					if (tx != px || ty != ny)
						return;

					traceStraightPath(tx, ty, &nx, &ny, &lvl2);

					if (tx2 != nx || ty2 != ny)
						return;

					_waypoints[_numWaypoints].x = tx;
					_waypoints[_numWaypoints].y = ty;
					_waypoints[_numWaypoints].level = lvl1;
					_numWaypoints++;
					_waypoints[_numWaypoints].x = nx;
					_waypoints[_numWaypoints].y = ny;
					_waypoints[_numWaypoints].level = lvl2;
					_numWaypoints++;
					g_hdb->_sound->playSound(SND_MENU_SLIDER);

					if (onEvenTile(_player->x, _player->y))
						setEntityGoal(_player, tx, ty);
				}
			}
			return;
		}

		// create a waypoint @ the player x,y? NO!
		if ((nx != px || ny != py) && onEvenTile(_player->x, _player->y))
			setEntityGoal(_player, nx, ny);
		else
			return;
	} else {
		// trace from last waypoint to new spot
		level = _waypoints[_numWaypoints - 1].level;

		if (!traceStraightPath(_waypoints[_numWaypoints - 1].x, _waypoints[_numWaypoints - 1].y, &nx, &ny, &level)) {
			int		tx, ty, tx2, ty2;

			// it didn't work, so player is probably trying some diagonal movement.
			// let's break it down into two waypoints: one horz, one vert
			px = _waypoints[_numWaypoints - 1].x;
			py = _waypoints[_numWaypoints - 1].y;
			level = _waypoints[_numWaypoints - 1].level;

			tx = nx;
			ty = py;
			tx2 = nx;
			ty2 = ny;
			int	lvl1, lvl2;

			lvl1 = lvl2 = level;

			if (traceStraightPath(px, py, &tx, &ty, &lvl1)) {
				if (tx != nx || ty != py)
					goto newpath2;

				traceStraightPath(tx, ty, &tx2, &ty2, &lvl2);

				if (tx2 != nx || ty2 != ny)
					goto newpath2;

				if (_numWaypoints < kMaxWaypoints) {
					_waypoints[_numWaypoints].x = tx;
					_waypoints[_numWaypoints].y = ty;
					_waypoints[_numWaypoints].level = lvl1;
					_numWaypoints++;
					g_hdb->_sound->playSound(SND_MENU_SLIDER);
				}

				if (_numWaypoints < kMaxWaypoints) {
					_waypoints[_numWaypoints].x = nx;
					_waypoints[_numWaypoints].y = ny;
					_waypoints[_numWaypoints].level = lvl2;
					_numWaypoints++;
					g_hdb->_sound->playSound(SND_MENU_SLIDER);
				}
			} else {
newpath2:
				tx = px;
				ty = ny;
				tx2 = nx;
				ty2 = ny;

				lvl1 = lvl2 = level;

				if (traceStraightPath(px, py, &tx, &ty, &lvl1)) {
					if (tx != px || ty != ny)
						return;

					traceStraightPath(tx, ty, &nx, &ny, &lvl2);

					if (tx2 != nx || ty2 != ny)
						return;

					if (_numWaypoints < kMaxWaypoints) {
						_waypoints[_numWaypoints].x = tx;
						_waypoints[_numWaypoints].y = ty;
						_waypoints[_numWaypoints].level = lvl1;
						_numWaypoints++;
						g_hdb->_sound->playSound(SND_MENU_SLIDER);
					}

					if (_numWaypoints < kMaxWaypoints) {
						_waypoints[_numWaypoints].x = nx;
						_waypoints[_numWaypoints].y = ny;
						_waypoints[_numWaypoints].level = lvl2;
						_numWaypoints++;
						g_hdb->_sound->playSound(SND_MENU_SLIDER);
					}
				}
			}
			return;
		}

		// create a waypoint @ the player x,y? NO!
		if (nx == px && ny == py)
			return;

		// make sure potential waypoint isn't on other waypoints!
		for (i = 0; i < _numWaypoints; i++)
			if (_waypoints[i].x == nx && _waypoints[i].y == ny)
				return;
	}

	if (_numWaypoints < kMaxWaypoints) {
		_waypoints[_numWaypoints].x = nx;
		_waypoints[_numWaypoints].y = ny;
		_waypoints[_numWaypoints].level = level;
		_numWaypoints++;
		g_hdb->_sound->playSound(SND_MENU_SLIDER);
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
	for (uint8 i = 0; i < ARRAYSIZE(_waypoints); i++) {
		_waypoints[i].reset();
	}
	_numWaypoints = 0;
}

bool AI::traceStraightPath(int x1, int y1, int *x2, int *y2, int *level) {
	// this checks to make sure we're only going vert or horz
	if (x1 != *x2 && y1 != *y2)
		return false;

	// this sets a -1, 0, or 1 step value
	int xVel = *x2 - x1;
	if (xVel < 0)
		xVel = -1;
	if (xVel > 0)
		xVel = 1;

	int yVel = *y2 - y1;
	if (yVel < 0)
		yVel = -1;
	if (yVel > 0)
		yVel = 1;

	AIEntity *e;
	while (1) {
		// clear tile ahead?
		bool entOK = false;
		bool ok = false;

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
					((flags & kFlagRadFloor) == kFlagRadFloor);
				e = findEntity(x1, y1);
				if (e && g_hdb->_ai->walkThroughEnt(e->type))
					entOK = true;
				else if (ok && e && (e->state == STATE_FLOATING || e->state == STATE_MELTED || e == _player))
					entOK = ok = true;
				else
					ok = false;
			} else if (ok &&
				((flags & kFlagWater) == kFlagWater ||
				(flags & kFlagSlime) == kFlagSlime)) {
				// if it's non-blocking, is there water or slime?
				e = findEntity(x1, y1);
				if (e && g_hdb->_ai->walkThroughEnt(e->type))
					entOK = true;
				else
					if (e && (e->state == STATE_FLOATING || e->state == STATE_MELTED || e == _player))
						entOK = ok = true;
					else
						ok = false;
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
						((flags & kFlagRadFloor) == kFlagRadFloor);
					e = findEntity(x1, y1);
					if (e && g_hdb->_ai->walkThroughEnt(e->type))
						entOK = true;
					else if (ok && e && (e->state == STATE_FLOATING || e->state == STATE_MELTED || e == _player))
							entOK = ok = true;
					else
						ok = false;
				} else if (ok &&
					((flags & kFlagWater) == kFlagWater ||
					(flags & kFlagSlime) == kFlagSlime)) {
					// if it's non-blocking, is there water or slime?
					e = findEntity(x1, y1);
					if (e && g_hdb->_ai->walkThroughEnt(e->type))
						entOK = true;
					else
						if (e && (e->state == STATE_FLOATING || e->state == STATE_MELTED || e == _player))
							entOK = ok = true;
						else
							ok = false;
				}
			}
		}

		if (ok) {
			e = findEntity(x1, y1);
			if (e == _player)
				e = nullptr;
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
	default:
		break;
	}
	return e->standdownGfx[0];
}

void AI::drawWayPoints() {
	static int anim = 0;
	static uint32 delay = g_hdb->getTimeSlice();
	static int alpha = 255;
	static int aVel = -4;

	int	mapX, mapY;
	g_hdb->_map->getMapXY(&mapX, &mapY);

	for (int i = 0; i < _numWaypoints; i++) {
		int	x = _waypoints[i].x * kTileWidth;
		int	y = _waypoints[i].y * kTileHeight;

		if (x > mapX - 32 && (x < (mapX + g_hdb->_screenWidth)) &&
			y > mapY - 32 && (y < (mapY + g_hdb->_screenHeight)))
			_waypointGfx[anim]->drawMasked(x - mapX, y - mapY, alpha);
	}

	// vary the alpha blending
	alpha = (alpha + aVel);
	if (alpha < 64) {
		alpha = 64;
		aVel = -aVel;
	}

	if (alpha > 200) {
		alpha = 200;
		aVel = -aVel;
	}

	// don't animate every single game frame...
	if (delay > g_hdb->getTimeSlice())
		return;

	delay = g_hdb->getTimeSlice() + 100;

	// cycle the waypoint gfx animation
	anim++;
	if (anim == 4)
		anim = 0;
}

} // End of Namespace
