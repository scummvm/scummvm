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

#ifndef M4_BURGER_ROOMS_SECTION3_MINE_ROOM_H
#define M4_BURGER_ROOMS_SECTION3_MINE_ROOM_H

#include "m4/burger/rooms/section3/section3_room.h"

namespace M4 {
namespace Burger {
namespace Rooms {

enum MineDoors {
	BACK, FRONT, LEFT, RIGHT, MAX_DOORS, NONE
};

enum MineScenes {
	SCENE_305,	 // treasure
	SCENE_310,	 // mine entrance
	SCENE_311,	 // one door (a)
	SCENE_312,	 // one door (b)
	SCENE_313,	 // two doors (a)
	SCENE_314,	 // two doors (b)
	SCENE_315,	 // three doors (a)
	SCENE_316,	 // three doors (b)
	SCENE_317,	 // four doors (a)
	SCENE_318,	 // four doors (b)
	SCENE_319,	 // one door (c)
	MAX_SCENE_TYPES,
	TREASURE_SCENE = SCENE_310,
	ENTRANCE_SCENE = SCENE_305,
	NO_SCENE = MAX_SCENE_TYPES
};


struct MineRoom {
	int16 roomNumber;    	  	// The mine room number
	int16 scene_id;             // The scene id (indexes for mine_scene_numbers)
	int16 link[4];              // Links to other mine rooms
	int16 door[4];              // Specify doors to use
	int16 correctLink;          // The correct link to take to get to the treasure
	int16 check;            	// Used to check the maze sanity

	void clear();
};

struct EntranceInfo {
	int16 offscreen_x;
	int16 offscreen_y;
	int16 enter_facing;
	int16 home_x;
	int16 home_y;
	int16 home_facing;
};

struct Rectangle {
	int16 x1;
	int16 y1;
	int16 x2;
	int16 y2;
};

class Mine : public Section3Room {
private:
	static const char *SAID[][4];
	static const int16 MINE_SCENE_NUMBERS[];
	static const MineRoom MINE_INFO[];

	/**
	 * Coords for starting fade down for exits
	 */
	static const Rectangle FADE_DOWN_INFO[MAX_SCENE_TYPES][4];

	/**
	 * Room coords for entrance walk-ins
	 */
	static const EntranceInfo ENTRANCE_INFO[MAX_SCENE_TYPES][4];

	int _mineCtr = 0;
	int16 _presentSceneID = 0;
	static int16 _entranceDoor;		// Shared between cellar and mine room classes
	MineRoom _mineRoomInfo;
	bool _fade_down_rect_active = false;	// True if a fade down should occur when walker in the fade_down_rect
	Rectangle _fade_down_rect;				// If the walker is in his rectum and its active, turn off the lights
	int _volume = 0;
	int _random1 = 0;
	int _random2 = 0;
	int _treasureDistance = 0;
	int _previousDistance = 0;

	int getTreasureDistance() const;

protected:
	const char *getDigi() override {
		return "300_004";
	}

	void mine_travel_link(int16 takeLink);
	void set_fade_down_rect(MineDoors exit_door);

public:
	Mine();
	virtual ~Mine() {}

	void preload() override;
	void init() override;
	void daemon() override;
	void pre_parser() override;
	void parser() override;
};

} // namespace Rooms
} // namespace Burger
} // namespace M4

#endif
