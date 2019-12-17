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

#ifndef ULTIMA6_PATHFINDER_ACTOR_PATH_FINDER_H
#define ULTIMA6_PATHFINDER_ACTOR_PATH_FINDER_H

#include "PathFinder.h"
#include "DirFinder.h"

namespace Ultima {
namespace Ultima6 {

class Actor;

class ActorPathFinder: public PathFinder, public DirFinder {
protected:
	Actor *actor;

public:
	ActorPathFinder(Actor *a, MapCoord g);
	virtual ~ActorPathFinder();
	void set_actor(Actor *a);

	virtual bool update_location(); /* get location from actor (use any time) */
	virtual void actor_moved(); /* the actor moved ON PATH...
                                   (use after get_next_move()) */

	virtual bool check_loc(const MapCoord &loc);

	void get_closest_dir(MapCoord &rel_step); // relative dir loc->goal
	virtual bool get_next_move(MapCoord &step);

protected:
	bool search_towards_target(const MapCoord &g, MapCoord &rel_step);
	bool check_dir(const MapCoord &loc, MapCoord &rel, sint8 rot = 0);
	bool check_dir_and_distance(MapCoord loc, MapCoord g, MapCoord &rel_step, sint8 rotate);
};

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
