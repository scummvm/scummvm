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

#ifndef NUVIE_PATHFINDER_U6_ASTAR_PATH_H
#define NUVIE_PATHFINDER_U6_ASTAR_PATH_H

#include "ultima/nuvie/pathfinder/astar_path.h"

namespace Ultima {
namespace Nuvie {

/* This provides a U6-specific step_cost() method. */
class U6AStarPath: public AStarPath {
public:
	sint32 step_cost(MapCoord &c1, MapCoord &c2) override;
	uint32 path_cost_est(MapCoord &s, MapCoord &g) override;
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
