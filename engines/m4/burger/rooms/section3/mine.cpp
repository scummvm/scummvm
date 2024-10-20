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

#include "m4/burger/rooms/section3/mine.h"
#include "m4/burger/vars.h"

namespace M4 {
namespace Burger {
namespace Rooms {

#define MINE_END 39

const char *Mine::SAID[][4] = {
	{ "TUNNEL",  "311w007", "311w007z", nullptr    },
	{ "DEBRIS",  nullptr,   "311w010",  "311w011"  },
	{ "GROUND",  "311w012", "311w007z", "311w007z" },
	{ "WALL",    "311w012", "311w007z", "311w007z" },
	{ "CEILING", "311w012", "311w007z", "311w007z" },
	{ nullptr, nullptr, nullptr, nullptr }
};

const int16 Mine::MINE_SCENE_NUMBERS[] = {
	305, 310, 311, 312, 313, 314, 315, 316, 317, 318, 319
};

const MineRoom Mine::MINE_INFO[] = {
  //                              Links                      Doors
  //                      ---------------------   --------------------------     Correct
  // Room#   Scene ID      Back Front Left Right   Back   Front  Left   Right      Link    Check
  // -----   ----------    ---- ----- ---- -----   -----  -----  -----  -----     -------  -----
      { 0,   SCENE_305, {  6,   -1,   -1,  -1 },   { FRONT, NONE,  NONE,  NONE },   BACK,      0},  // mine entrance
      { 1,   SCENE_313, { -1,   -1,    2,   6 },   { NONE,  NONE,  RIGHT, LEFT },   RIGHT,     0},
      { 2,   SCENE_316, { -1,   11,   13,   1 },   { NONE,  BACK,  RIGHT, LEFT },   LEFT,      0},
      { 3,   SCENE_318, {  9,   14,    5,   4 },   { FRONT, BACK,  RIGHT, LEFT },   FRONT,     0},
      { 4,   SCENE_315, {  5,   -1,    3,  10 },   { FRONT, NONE,  RIGHT, LEFT },   LEFT,      0},
      { 5,   SCENE_317, { 18,    4,    9,   3 },   { FRONT, BACK,  RIGHT, LEFT },   RIGHT,     0},
      { 6,   SCENE_318, { 11,    0,    1,   7 },   { FRONT, BACK,  RIGHT, LEFT },   RIGHT,     0},
      { 7,   SCENE_317, { 13,   14,    6,  18 },   { FRONT, RIGHT, RIGHT, LEFT },   FRONT,     0},
      { 8,   SCENE_313, { -1,   -1,   10,   9 },   { NONE,  NONE,  RIGHT, LEFT },   RIGHT,     0},
      { 9,   SCENE_318, { 10,    3,    8,   5 },   { FRONT, BACK,  RIGHT, LEFT },   FRONT,     0},
  // -----  ----------   ---- ----- ---- -----      -----  -----  -----  -----     ----     ---}--
      {10,   SCENE_316, { -1,    9,    4,   8 },   { NONE,  BACK,  RIGHT, LEFT },   LEFT,      0},
      {11,   SCENE_317, {  2,    6,   11,  11 },   { FRONT, BACK,  RIGHT, LEFT },   FRONT,     0},
      {12,   SCENE_319, { -1,   -1,   -1,  13 },   { NONE,  NONE,  NONE,  LEFT },   RIGHT,     0},
      {13,   SCENE_316, { -1,    7,   12,   2 },   { NONE,  BACK,  RIGHT, LEFT },   FRONT,     0},
      {14,   SCENE_315, {  3,   -1,   26,   7 },   { FRONT, NONE,  BACK,  FRONT},   LEFT,      0},
      {15,   SCENE_317, { 17,   16,   17,  20 },   { FRONT, BACK,  RIGHT, LEFT },   RIGHT,     0},
      {16,   SCENE_311, { 15,   -1,   -1,  -1 },   { FRONT, NONE,  NONE,  NONE },   BACK,      0},
      {17,   SCENE_318, { 22,   15,   20,  15 },   { FRONT, BACK,  RIGHT, LEFT },   LEFT,      0},
      {18,   SCENE_316, { -1,    5,    7,  19 },   { NONE,  BACK,  RIGHT, LEFT },   LEFT,      0},
      {19,   SCENE_312, { -1,   -1,   18,  -1 },   { NONE,  NONE,  RIGHT, NONE },   LEFT,      0},
  // -----   ----------   ---- ----- ---- -----      -----  -----  ------ -----     ----     ---}--
      {20,   SCENE_315, { 26,   -1,   15,  17 },   { FRONT, NONE,  RIGHT, LEFT },   BACK,      0},
      {21,   SCENE_319, { -1,   -1,   -1,  22 },   { NONE,  NONE,  NONE,  LEFT },   RIGHT,     0},
      {22,   SCENE_316, { -1,   17,   21,  23 },   { NONE,  BACK,  RIGHT, LEFT },   FRONT,     0},
      {23,   SCENE_312, { -1,   -1,   22,  -1 },   { NONE,  NONE,  RIGHT, NONE },   LEFT,      0},
      {24,   SCENE_318, { 35,   29,   33,  25 },   { FRONT, BACK,  RIGHT, LEFT },   RIGHT,     0},
      {25,   SCENE_314, { -1,   -1,   24,  26 },   { NONE,  NONE,  RIGHT, LEFT },   RIGHT,     0},
      {26,   SCENE_317, { 14,   20,   25,  38 },   { LEFT,  BACK,  RIGHT, LEFT },   RIGHT,     0},
      {27,   SCENE_318, { 38,   38,   37,  28 },   { FRONT, BACK,  RIGHT, LEFT },   BACK,      0},
      {28,   SCENE_312, { -1,   -1,   27,  -1 },   { NONE,  NONE,  RIGHT, NONE },   LEFT,      0},
      {29,   SCENE_317, { 24,   30,   35,  31 },   { FRONT, BACK,  RIGHT, LEFT },   BACK,      0},
  // -----   ----------   ---- ----- ---- -----      -----  -----  ------ -----     ----     -----
      {30,   SCENE_311, { 29,   -1,   -1,  -1 },   { FRONT, NONE,  NONE,  NONE },   BACK,      0},
      {31,   SCENE_312, { -1,   -1,   29,  -1 },   { NONE,  NONE,  RIGHT, NONE },   LEFT,      0},
      {32,   SCENE_319, { -1,   -1,   -1,  33 },   { NONE,  NONE,  NONE,  LEFT },   RIGHT,     0},
      {33,   SCENE_314, { -1,   -1,   32,  24 },   { NONE,  NONE,  RIGHT, LEFT },   RIGHT,     0},
      {34,   SCENE_319, { -1,   -1,   -1,  35 },   { NONE,  NONE,  NONE,  LEFT },   RIGHT,     0},
      {35,   SCENE_316, { -1,   24,   34,  29 },   { NONE,  BACK,  RIGHT, LEFT },   FRONT,     0},
      {36,   SCENE_319, { -1,   -1,   -1,  37 },   { NONE,  NONE,  NONE,  LEFT },   RIGHT,     0},
      {37,   SCENE_314, { -1,   -1,   36,  27 },   { NONE,  NONE,  RIGHT, LEFT },   RIGHT,     0},
      {38,   SCENE_318, { 27,   27,   26,  39 },   { FRONT, BACK,  RIGHT, LEFT },   RIGHT,     0},
      {39,   SCENE_310, { -1,   -1,   38,  -1 },   { NONE,  NONE,  RIGHT, NONE },   NONE,      0}, // treasure
  // -----   ----------   ---- ----- ---- -----      -----  -----  -----  -----    -------  -----
      {40,   NO_SCENE,  { -1,   -1,   -1,  -1 },   { NONE,  NONE,  NONE,  NONE },   NONE,	    0}
};

const EntranceInfo Mine::ENTRANCE_INFO[MAX_SCENE_TYPES][4] = {
	//
	// ( offscreen_x, offscreen_y, enter_facing, home_x, home_y, home_facing ) x 4 possible entrances
	//
	//	      Back				 	      Front					      Left					       Right
	//-----------------------    -------------------------   -------------------------   -------------------------
   	{{285, 240, 5, 320, 290, 5}, {  0,   0, 0,   0,   0, 0}, {  0,   0, 0,   0,   0, 0}, {  0,   0, 0,   0,   0, 0}}, // 305
	{{  0,   0, 0,   0,   0, 0}, {  0,   0, 0,   0,   0, 0}, { 55, 245, 3, 225, 275, 3}, {  0,   0, 0,   0,   0, 0}}, // 310
	{{315, 225, 7, 310, 275, 7}, {  0,   0, 0,   0,   0, 0}, {  0,   0, 0,   0,   0, 0}, {  0,   0, 0,   0,   0, 0}}, // 311
	{{  0,   0, 0,   0,   0, 0}, {  0,   0, 0,   0,   0, 0}, { 90, 240, 3, 220, 270, 4}, {  0,   0, 0,   0,   0, 0}}, // 312
	{{  0,   0, 0,   0,   0, 0}, {  0,   0, 0,   0,   0, 0}, { 65, 230, 3, 215, 270, 4}, {550, 240, 8, 400, 270, 8}}, // 313
	{{  0,   0, 0,   0,   0, 0}, {  0,   0, 0,   0,   0, 0}, { 65, 240, 3, 190, 270, 4}, {570, 250, 8, 440, 280, 8}}, // 314
	{{320, 210, 7, 315, 270, 7}, {  0,   0, 0,   0,   0, 0}, { 60, 230, 3, 190, 270, 4}, {560, 250, 9, 400, 270, 8}}, // 315
	{{  0,   0, 0,   0,   0, 0}, {400, 373,11, 335, 300, 7}, { 60, 220, 3, 200, 270, 4}, {570, 240, 8, 400, 270, 7}}, // 316
	{{320, 215, 7, 315, 270, 7}, {230, 373, 1, 330, 300, 7}, { 60, 220, 3, 200, 270, 4}, {570, 240, 8, 400, 270, 7}}, // 317
	{{320, 215, 5, 315, 270, 7}, {350, 373,11, 330, 300, 7}, { 60, 240, 3, 190, 270, 4}, {560, 235, 9, 420, 270, 8}}, // 318
	{{  0,   0, 0,   0,   0, 0}, {  0,   0, 0,   0,   0, 0}, {  0,   0, 0,   0,   0, 0}, {530, 240, 9, 420, 270, 7}}  // 319
};


const Rectangle Mine::FADE_DOWN_INFO[MAX_SCENE_TYPES][4] = {
	//
	// ( x1, y1, x2, y2) for four directions
	//
	//  	   Back		  		 	 Front	     		 	Left 		   	       Right
	//-------------------    -------------------    -------------------    -------------------
	{{255,   0, 315, 250},  {  0,   0,   0,   0},  {  0,   0,   0,   0},  {  0,   0,   0,   0}},  // 305
	{{  0,   0,   0,   0},  {  0,   0,   0,   0},  {  0,   0, 170, 270},  {  0,   0,   0,   0}},  // 310
	{{270,   0, 365, 255},  {  0,   0,   0,   0},  {  0,   0,   0,   0},  {  0,   0,   0,   0}},  // 311
	{{  0,   0,   0,   0},  {  0,   0,   0,   0},  {  0,   0, 175, 268},  {  0,   0,   0,   0}},  // 312
	{{  0,   0,   0,   0},  {  0,   0,   0,   0},  {  0,   0, 172, 256},  {470,   0, 639, 265}},  // 313
	{{  0,   0,   0,   0},  {  0,   0,   0,   0},  {  0,   0, 172, 265},  {470,   0, 639, 280}},  // 314
	{{280,   0, 365, 245},  {  0,   0,   0,   0},  {  0,   0, 165, 257},  {470,   0, 639, 271}},  // 315
	{{  0,   0,   0,   0},  {325, 350, 500, 374},  {  0,   0, 178, 265},  {470,   0, 639, 265}},  // 316
	{{275,   0, 365, 255},  {136, 344, 360, 374},  {  0,   0, 165, 268},  {470,   0, 639, 270}},  // 317
	{{275,   0, 365, 252},  {185, 344, 500, 374},  {  0,   0, 165, 265},  {470,   0, 639, 270}},  // 318
	{{  0,   0,   0,   0},  {  0,   0,   0,   0},  {  0,   0,   0,   0},  {470,   0, 639, 270}}   // 319
};

int16 Mine::_entranceDoor;

void MineRoom::clear() {
	roomNumber = 0;
	scene_id = 0;
	Common::fill(link, link + 4, 0);
	Common::fill(door, door + 4, 0);
	correctLink = 0;
	check = 0;
}

Mine::Mine() : Section3Room() {
	_mineRoomInfo.clear();
}

void Mine::preload() {
	Section3Room::preload();
	_mineCtr = 0;
	_G(player).walker_type = 0;
	_G(player).shadow_type = 0;
}

void Mine::init() {
	setupDigi();

	if (_G(flags)[kTrufflesInMine]) {
		for (int i = 0; i < 6; ++i)
			digi_preload(Common::String::format("300t001%c", 'a' + i));
	}

	_fade_down_rect_active = false;
	set_palette_brightness(30);
	_G(kernel).call_daemon_every_loop = true;
	_mineCtr = 0;

	const int32 &mineRoomIndex = _G(flags)[kMineRoomIndex];
	_mineRoomInfo = MINE_INFO[mineRoomIndex];		// Get this mine room info
	_presentSceneID = _mineRoomInfo.scene_id;		// Set the scene ID

	switch (_G(game).previous_room) {
	case KERNEL_RESTORING_GAME:
		player_set_commands_allowed(true);
		break;

	default:
		player_set_commands_allowed(false);
		ws_demand_location(-50, 200);
		kernel_trigger_dispatch_now(301);
		break;
	}
}

void Mine::daemon() {
	switch (_G(kernel).trigger) {
	case 301:
	case 302: {
		const EntranceInfo &ei = ENTRANCE_INFO[_presentSceneID][_entranceDoor];

		if (_G(kernel).trigger == 301)
			ws_demand_location(ei.offscreen_x, ei.offscreen_y, ei.enter_facing);

		player_set_commands_allowed(false);
		ws_walk(ei.home_x, ei.home_y, nullptr, 303, ei.home_facing);
		term_message("Mine entry %d at (%d,%d) to (%d,%d, %d)",
			_presentSceneID,
			ei.offscreen_x, ei.offscreen_y,
			ei.home_x, ei.home_y, ei.home_facing);
		break;
	}

	case 303:
		player_set_commands_allowed(true);

		if (_G(game).room_id == 310) {
			if (_G(flags)[V141]) {
				_G(wilbur_should) = 10002;
			} else {
				_G(flags)[V141] = 1;
				_G(wilbur_should) = 408;
			}

			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		} else if (_G(game).room_id != 305) {
			if (!_G(flags)[kEnteredMine]) {
				_G(flags)[kEnteredMine] = 1;
				_G(wilbur_should) = _G(flags)[kTrufflesInMine] ? 402 : 401;
			} else if (!imath_rand_bool(3)) {
				_G(wilbur_should) = 10002;
			} else if (!_G(flags)[kTrufflesInMine]) {
				_G(wilbur_should) = 404;
			} else {
				_G(wilbur_should) = inv_player_has("WHISTLE") ? 406 : 405;
			}

			kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
		}
		break;

	case 304:
		digi_play("300_007", 2);
		break;

	case 305:
		_volume = 160 - getTreasureDistance() * 25;
		_random1 = imath_ranged_rand(0, 5);
		digi_play(Common::String::format("300t001%c", 'a' + _random1).c_str(), 3, _volume);
		break;

	case 10008:
		switch (_G(game).room_id) {
		case 305:
		case 310:
			_G(kernel).continue_handling_trigger = true;
			break;

		default:
			if (_G(flags)[kTrufflesInMine] && !_G(flags)[kTrufflesRanAway])
				kernel_timing_trigger(15, 305);
			break;
		}
		break;

	case kCHANGE_WILBUR_ANIMATION:
		switch (_G(wilbur_should)) {
		case 401:
			wilbur_speech("311w001");
			break;

		case 402:
			player_set_commands_allowed(false);
			_G(wilbur_should) = 403;
			wilbur_speech("311w002", kCHANGE_WILBUR_ANIMATION);
			break;

		case 403:
			player_set_commands_allowed(true);
			wilbur_speech("311w003");
			break;

		case 404:
			player_set_commands_allowed(true);
			_random2 = imath_ranged_rand(0, 5);
			wilbur_speech(Common::String::format("311w004%c", 'a' + _random2).c_str());
			break;

		case 405:
			term_message("Wilbur enters the mine with no whistle!");
			player_set_commands_allowed(true);
			_random2 = imath_ranged_rand(0, 3);
			digi_play(Common::String::format("311w005%c", 'a' + _random2).c_str(), 3, _volume);
			break;

		case 406:
			_random2 = imath_ranged_rand(0, 2);
			wilbur_speech(Common::String::format("311w006%c", 'a' + _random2).c_str());
			break;

		case 407:
			if (_G(flags)[V136]) {
				_random2 = imath_ranged_rand(0, 6);
				wilbur_speech(Common::String::format("311w009%c", 'a' + _random2).c_str());
			} else {
				wilbur_speech("311w008");
			}
			break;

		case 408:
			wilbur_speech("310w001");
			break;

		case kWILBUR_SPEECH_STARTED:
			player_set_commands_allowed(true);
			ws_unhide_walker();
			_G(wilbur_should) = 10002;
			term_message("Mine whistle daemon thing");

			switch (_G(game).room_id) {
			case 305:
				if (_G(flags)[kTrufflesInMine]) {
					_G(flags)[V002] = 1;
					wilbur_speech("300w060");
				}
				break;

			case 310:
				if (_G(flags)[kTrufflesInMine] && !_G(flags)[kTrufflesRanAway]) {
					_G(flags)[kTrufflesRanAway] = 1;
				} else {
					wilbur_speech("300w069");
				}
				break;

			default:
				if (!_G(flags)[kTrufflesInMine] && _G(flags)[kTrufflesRanAway]) {
					// Truffles is too far away
					wilbur_speech("300w061");
				} else if (!_G(flags)[V148]) {
					_G(flags)[V148] = 1;
					_treasureDistance = getTreasureDistance();

					switch (_treasureDistance) {
					case 1:
						wilbur_speech("300w062");
						break;
					case 2:
						wilbur_speech("300w063");
						break;
					case 3:
					case 4:
						wilbur_speech("300w064");
						break;
					case 5:
					case 6:
						wilbur_speech("300w065");
						break;
					default:
						break;
					}

					_previousDistance = _treasureDistance;
				} else {
					_treasureDistance = getTreasureDistance();

					if (_treasureDistance < _previousDistance) {
						wilbur_speech("300w068");
					} else if (_treasureDistance == _previousDistance) {
						wilbur_speech("300w067");
					} else {
						wilbur_speech("300w066");
					}

					_previousDistance = _treasureDistance;
				}
				break;
			}
			break;

		default:
			_G(kernel).continue_handling_trigger = true;
			break;
		}
		break;

	case kCALLED_EACH_LOOP:
		if (_fade_down_rect_active) {
			player_update_info();

			if (_G(player_info).x >= _fade_down_rect.x1 &&
				_G(player_info).x <= _fade_down_rect.x2 &&
				_G(player_info).y >= _fade_down_rect.y1 &&
				_G(player_info).y <= _fade_down_rect.y2) {
				if (player_commands_allowed()) {
					pal_fade_set_start(100);
					pal_fade_init(_G(kernel).first_fade, 255, 0, 40, -1);
				}

				player_set_commands_allowed(false);
			}
		}
		break;

	default:
		_G(kernel).continue_handling_trigger = true;
		break;
	}
}

void Mine::pre_parser() {
	_G(kernel).trigger_mode = KT_DAEMON;
	_fade_down_rect_active = false;

	if (player_said("tunnel") && player_said_any("walk through", "GEAR")) {
		if (_G(player).click_y > 300)
			set_fade_down_rect(FRONT);
		else if (_G(player).click_x < 200)
			set_fade_down_rect(LEFT);
		else if (_G(player).click_x < 400)
			set_fade_down_rect(BACK);
		else
			set_fade_down_rect(RIGHT);

		// Turn on the fade down area
		_fade_down_rect_active = true;
	}
}

void Mine::parser() {
	_G(kernel).trigger_mode = KT_DAEMON;

	if (player_said("LOOK AT") && player_said_any("WALL", "CEILING", "GROUND")) {
		term_message("Room #: %d", _G(flags)[kMineRoomIndex]);
		term_message("Distance from pig: %d", getTreasureDistance());

		_mineCtr = (_mineCtr + 1) % 5;

		if (_mineCtr == 0) {
			wilbur_speech("311w012");
			_G(player).command_ready = false;
			return;
		}
	}

	if (_G(walker).wilbur_said(SAID)) {
		// Already handled
	} else if (player_said("tunnel") && player_said_any("walk through", "GEAR")) {
		pal_fade_set_start(0);

		if (_G(player).click_y > 300)
			mine_travel_link(FRONT);
		else if (_G(player).click_x < 200)
			mine_travel_link(LEFT);
		else if (_G(player).click_x > 400)
			mine_travel_link(RIGHT);
		else
			mine_travel_link(BACK);
	} else if (player_said("LOOK AT", "DEBRIS") && _G(game).room_id != 305) {
		_G(wilbur_should) = 407;
		kernel_trigger_dispatch_now(kCHANGE_WILBUR_ANIMATION);
	} else {
		return;
	}

	_G(player).command_ready = false;
}

int Mine::getTreasureDistance() const {
	int distance = 0;

	for (int index = _G(flags)[kMineRoomIndex]; index != MINE_END; ++distance) {
		const MineRoom &me = MINE_INFO[index];
		index = me.link[me.correctLink];
	}

	return distance;
}

void Mine::mine_travel_link(int16 takeLink) {
	int32 &mineRoomIndex = _G(flags)[kMineRoomIndex];

	_mineRoomInfo = MINE_INFO[mineRoomIndex];		// Get this mine room info
	_entranceDoor = _mineRoomInfo.door[takeLink];	// Get which door to enter from in new room
	mineRoomIndex = _mineRoomInfo.link[takeLink];	// Get which link to take
	assert(mineRoomIndex != -1);

	_mineRoomInfo = MINE_INFO[mineRoomIndex];		// Get new mine room info from new index
	_presentSceneID = _mineRoomInfo.scene_id;		// Set the scene ID
	_G(game).new_room = MINE_SCENE_NUMBERS[_presentSceneID]; // Go to the corresponding scene number
	_G(kernel).force_restart = true; 				// Makes the scene start over even if new_room = present room
}

void Mine::set_fade_down_rect(MineDoors exit_door) {
	_fade_down_rect.x1 = FADE_DOWN_INFO[_presentSceneID][exit_door].x1;
	_fade_down_rect.y1 = FADE_DOWN_INFO[_presentSceneID][exit_door].y1;
	_fade_down_rect.x2 = FADE_DOWN_INFO[_presentSceneID][exit_door].x2;
	_fade_down_rect.y2 = FADE_DOWN_INFO[_presentSceneID][exit_door].y2;
}


} // namespace Rooms
} // namespace Burger
} // namespace M4
