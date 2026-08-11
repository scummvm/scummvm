/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/core/obj_manager.h"
#include "ultima/nuvie/usecode/usecode.h"
#include "ultima/nuvie/pathfinder/u6_astar_path.h"

namespace Ultima {
namespace Nuvie {

/* Return the cost of moving one step from `c1' to `c2'.
 * Blocking objects are checked for, and doors may be passable
 * Returns -1 if c2 is blocked.
 */
sint32 U6AStarPath::step_cost(const MapCoord &c1, const MapCoord &c2) {
	Game *game = Game::get_game();
	sint32 c = 1; // final cost is not necessarily the actual move cost

	// FIXME: need an actor->check_move(loc2, loc1) to check one step only
	if (c2.distance(c1) > 1)
		return -1;
	if (!pf->check_loc(c2.x, c2.y, c2.z)) {
		// check for door
		// Door objects consist of a wall and the actual door tile.
		// We use get_objBasedAt() here since we are only interested in the latter.
		Obj *block = game->get_obj_manager()->get_objBasedAt(c2.x, c2.y, c2.z, true, false);
		if (block && game->get_usecode()->is_unlocked_door(block))
			c += 2; // cost for opening door
		else
			return -1;
	}
	// add cost of *original* step
//    c += game->get_game_map()->get_impedance(c1.x, c1.y, c1.z);

	if (c1.x != c2.x && c1.y != c2.y) // prefer non-diagonal
		c *= 2;
	return c;
}

// Possible step cost is 1 to 16.
uint32 U6AStarPath::path_cost_est(const MapCoord &s, const MapCoord &g) {
	return Path::path_cost_est(s, g);
}

} // End of namespace Nuvie
} // End of namespace Ultima
