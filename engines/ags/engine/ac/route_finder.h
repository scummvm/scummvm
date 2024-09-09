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

#ifndef AGS_ENGINE_AC_ROUTEFND_H
#define AGS_ENGINE_AC_ROUTEFND_H

#include "ags/shared/ac/game_version.h"

namespace AGS3 {

// Forward declaration
namespace AGS {
namespace Shared {
class Bitmap;
} // namespace Shared
} // namespace AGS

struct MoveList;

class IRouteFinder {
public:
	virtual ~IRouteFinder() {
	}

	virtual void init_pathfinder() = 0;
	virtual void shutdown_pathfinder() = 0;
	virtual void set_wallscreen(AGS::Shared::Bitmap *wallscreen) = 0;
	virtual int can_see_from(int x1, int y1, int x2, int y2) = 0;
	virtual void get_lastcpos(int &lastcx, int &lastcy) = 0;
	virtual int find_route(short srcx, short srcy, short xx, short yy, int move_speed_x, int move_speed_y, AGS::Shared::Bitmap *onscreen, int movlst, int nocross = 0, int ignore_walls = 0) = 0;
	virtual void recalculate_move_speeds(MoveList *mlsp, int old_speed_x, int old_speed_y, int new_speed_x, int new_speed_y) = 0;
	virtual bool add_waypoint_direct(MoveList *mlsp, short x, short y, int move_speed_x, int move_speed_y) = 0;
};

void init_pathfinder(GameDataVersion game_file_version);
void shutdown_pathfinder();

void set_wallscreen(AGS::Shared::Bitmap *wallscreen);

int can_see_from(int x1, int y1, int x2, int y2);
void get_lastcpos(int &lastcx, int &lastcy);

int find_route(short srcx, short srcy, short xx, short yy, int move_speed_x, int move_speed_y, AGS::Shared::Bitmap *onscreen, int movlst, int nocross = 0, int ignore_walls = 0);
void recalculate_move_speeds(MoveList *mlsp, int old_speed_x, int old_speed_y, int new_speed_x, int new_speed_y);
// Append a waypoint to the move list, skip pathfinding
bool add_waypoint_direct(MoveList *mlsp, short x, short y, int move_speed_x, int move_speed_y);

} // namespace AGS3

#endif
