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

#include "ags/engine/ac/route_finder.h"
#include "ags/engine/ac/route_finder_impl.h"
#include "ags/engine/ac/route_finder_impl_legacy.h"
#include "ags/shared/debugging/out.h"
#include "ags/globals.h"

namespace AGS3 {

using AGS::Shared::Bitmap;

class AGSRouteFinder : public IRouteFinder {
public:
	virtual ~AGSRouteFinder() {}

	void init_pathfinder() override {
		AGS::Engine::RouteFinder::init_pathfinder();
	}
	void shutdown_pathfinder() override {
		AGS::Engine::RouteFinder::shutdown_pathfinder();
	}
	void set_wallscreen(Bitmap *wallscreen) override {
		AGS::Engine::RouteFinder::set_wallscreen(wallscreen);
	}
	int can_see_from(int x1, int y1, int x2, int y2) override {
		return AGS::Engine::RouteFinder::can_see_from(x1, y1, x2, y2);
	}
	void get_lastcpos(int &lastcx, int &lastcy) override {
		AGS::Engine::RouteFinder::get_lastcpos(lastcx, lastcy);
	}
	void set_route_move_speed(int speed_x, int speed_y) override {
		AGS::Engine::RouteFinder::set_route_move_speed(speed_x, speed_y);
	}
	int find_route(short srcx, short srcy, short xx, short yy, Bitmap *onscreen, int movlst, int nocross = 0, int ignore_walls = 0) override {
		return AGS::Engine::RouteFinder::find_route(srcx, srcy, xx, yy, onscreen, movlst, nocross, ignore_walls);
	}
	void calculate_move_stage(MoveList *mlsp, int aaa) override {
		AGS::Engine::RouteFinder::calculate_move_stage(mlsp, aaa);
	}
};

class AGSLegacyRouteFinder : public IRouteFinder {
public:
	virtual ~AGSLegacyRouteFinder() {}

	void init_pathfinder() override {
		AGS::Engine::RouteFinderLegacy::init_pathfinder();
	}
	void shutdown_pathfinder() override {
		AGS::Engine::RouteFinderLegacy::shutdown_pathfinder();
	}
	void set_wallscreen(Bitmap *wallscreen) override {
		AGS::Engine::RouteFinderLegacy::set_wallscreen(wallscreen);
	}
	int can_see_from(int x1, int y1, int x2, int y2) override {
		return AGS::Engine::RouteFinderLegacy::can_see_from(x1, y1, x2, y2);
	}
	void get_lastcpos(int &lastcx, int &lastcy) override {
		AGS::Engine::RouteFinderLegacy::get_lastcpos(lastcx, lastcy);
	}
	void set_route_move_speed(int speed_x, int speed_y) override {
		AGS::Engine::RouteFinderLegacy::set_route_move_speed(speed_x, speed_y);
	}
	int find_route(short srcx, short srcy, short xx, short yy, Bitmap *onscreen, int movlst, int nocross = 0, int ignore_walls = 0) override {
		return AGS::Engine::RouteFinderLegacy::find_route(srcx, srcy, xx, yy, onscreen, movlst, nocross, ignore_walls);
	}
	void calculate_move_stage(MoveList *mlsp, int aaa) override {
		AGS::Engine::RouteFinderLegacy::calculate_move_stage(mlsp, aaa);
	}
};

void init_pathfinder(GameDataVersion game_file_version) {
	if (game_file_version >= kGameVersion_350) {
		AGS::Shared::Debug::Printf(AGS::Shared::MessageType::kDbgMsg_Info, "Initialize path finder library");
		_GP(route_finder_impl).reset(new AGSRouteFinder());
	} else {
		AGS::Shared::Debug::Printf(AGS::Shared::MessageType::kDbgMsg_Info, "Initialize legacy path finder library");
		_GP(route_finder_impl).reset(new AGSLegacyRouteFinder());
	}

	_GP(route_finder_impl)->init_pathfinder();
}

void shutdown_pathfinder() {
	if (_GP(route_finder_impl))
		_GP(route_finder_impl)->shutdown_pathfinder();
}

void set_wallscreen(Bitmap *wallscreen) {
	_GP(route_finder_impl)->set_wallscreen(wallscreen);
}

int can_see_from(int x1, int y1, int x2, int y2) {
	return _GP(route_finder_impl)->can_see_from(x1, y1, x2, y2);
}

void get_lastcpos(int &lastcx, int &lastcy) {
	_GP(route_finder_impl)->get_lastcpos(lastcx, lastcy);
}

void set_route_move_speed(int speed_x, int speed_y) {
	_GP(route_finder_impl)->set_route_move_speed(speed_x, speed_y);
}

int find_route(short srcx, short srcy, short xx, short yy, Bitmap *onscreen, int movlst, int nocross, int ignore_walls) {
	return _GP(route_finder_impl)->find_route(srcx, srcy, xx, yy, onscreen, movlst, nocross, ignore_walls);
}

void calculate_move_stage(MoveList *mlsp, int aaa) {
	_GP(route_finder_impl)->calculate_move_stage(mlsp, aaa);
}

} // namespace AGS3
