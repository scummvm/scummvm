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

#ifndef MADS_PHANTOM_CATACOMBS_H
#define MADS_PHANTOM_CATACOMBS_H

#include "mads/madsv2/core/vocabh.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {

constexpr int NORTH = 0;
constexpr int EAST = 1;
constexpr int SOUTH = 2;
constexpr int WEST = 3;

constexpr int MAZE_PUDDLE             = 0x0001;        /* 401,404         */
constexpr int MAZE_RAT_NEST           = 0x0002;        /* 401,403,453,404 */
constexpr int MAZE_SKULL              = 0x0004;        /* 401,403         */
constexpr int MAZE_POT                = 0x0008;        /* 401,404         */
constexpr int MAZE_BRICK              = 0x0010;        /* 401,403,453,408 */
constexpr int MAZE_HOLE               = 0x0020;        /* 403             */
constexpr int MAZE_WEB                = 0x0040;        /* 403,453,404,408 */
constexpr int MAZE_PLANK              = 0x0080;        /* 403             */
constexpr int MAZE_DRAIN              = 0x0100;        /* 453             */
constexpr int MAZE_STONE              = 0x0200;        /* 453             */
constexpr int MAZE_BLOCK              = 0x0400;        /* 404             */
constexpr int MAZE_FALLEN_BLOCK       = 0x0800;        /* 404             */

extern void global_catacombs_setup();
extern void global_catacombs_init();
extern void global_enter_catacombs(int special);
extern int global_catacombs_exit(int exit);
extern void global_catacombs_move(int exit);

} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
