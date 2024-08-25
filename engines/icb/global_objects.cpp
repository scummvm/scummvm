/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

// global objects are in here - this makes it easier to see what there is
#include "engines/icb/global_objects.h"
#include "engines/icb/sound_logic.h"

namespace ICB {

int32 globalCharSpacing = 0;

// Prop sprite brightness to use when highlighting a prop that the player could interact with
int32 g_prop_select_r = 200;
int32 g_prop_select_g = 200;
int32 g_prop_select_b = 200;

// extra ambient brightness to use when highlighting a mega that the player could interact with
int32 g_mega_select_r = 200;
int32 g_mega_select_g = 200;
int32 g_mega_select_b = 200;

// holds info about current mission
_mission *g_mission = nullptr;

// main buffer object
// if this remains NULL then the system knows that there are no resources present
// this should be used as a high level indication of the situation
res_man *rs1 = nullptr;
res_man *rs2 = nullptr;
res_man *rs3 = nullptr;

res_man *private_session_resman = nullptr;

res_man *rs_bg = nullptr;     // pointer to which res_man to use for background buffer
res_man *rs_icons = nullptr;  // pointer to which res_man to use for icons
res_man *rs_anims = nullptr;  // pointer to which res_man to use for animation data
res_man *rs_remora = nullptr; // pointer to which res_man to use for remora graphics
res_man *rs_font = nullptr;   // pointer to which res_man to use for font data

// global test only
res_man *global_text_resman = nullptr;
LinkedDataFile *global_text;

// route-manager service object
// doesn't depend on session specific external data so we make it global service
// gets reset by the session constructor
// the service exists to facilitate and coordinate the movement of non player characters - the player object has its own stuff
//_route_manager        router;

// speech
text_sprite *g_text_bloc1;

// pc has extra bloc for remora, etc.
text_sprite *g_text_bloc2;

_mission *g_icb_mission;
_game_session *g_icb_session;

// session objects
_barrier_handler g_icb_session_barriers;
_floor_world *g_icb_session_floors;
_logic *g_logics[MAX_session_objects];
_mega *g_megas[MAX_voxel_list];
_vox_image *g_vox_images[MAX_voxel_list];

// master modes
_stub *g_stub;

// game script manager
_game_script gs;

class _remora;
class _icon_menu;
class _icon_list_manager;
// These objects are now all declared in one place because the order of creation matters.
_event_manager *g_oEventManager;
_line_of_sight *g_oLineOfSight;
_icon_menu *g_oIconMenu;
_icon_list_manager *g_oIconListManager;
_remora *g_oRemora;
_sound_logic *g_oSoundLogicEngine;

// Mission strings for the options menus (harmonised for PC and PSX)

const char *g_m01 = "m01_investigate_mine";
const char *g_m02 = "m02_security_hq";
const char *g_m03 = "m03_land_train";
const char *g_m04 = "m04_raid_on_containment";
const char *g_m05 = "m05_sabotage_refinery";
const char *g_m07 = "m07_mainland_base";
const char *g_m08 = "m08_entering_island_base";
const char *g_m09 = "m09_escape_island_base";
const char *g_m10 = "m10_submarine";

// The collection of possible missions including the session to start for each mission
const char *g_mission_startup_names[MAX_MISSIONS] = {
	"m01_investigate_mine", "office_building",
	"m02_security_hq", "basement",
	"m03_land_train", "car_5",
	"m04_raid_on_containment", "level_alphabeta",
	"m05_sabotage_refinery", "m5_mine",
	"m07_mainland_base", "compound",
	"m08_entering_island_base", "cable_car_dock",
	"m08_entering_island_base", "security",
	"m10_submarine", "submarine",
	"m_ed", "2_ship",
	"test_mission", "test_session"
};

const char *g_mission_names[NUMBER_OF_MISSIONS] = {
	"m01_investigate_mine",
	"m02_security_hq",
	"m03_land_train",
	"m04_raid_on_containment",
	"m05_sabotage_refinery",
	"m07_mainland_base",
	"m08_entering_island_base",
	"m08_entering_island_base",
	"m10_submarine",
	"m_ed",
	"test_mission"
};

const char *g_sessions[MAX_SESSIONS] = {"m01_investigate_mine",     "mine",
										"m01_investigate_mine",     "mineworkings",
										"m01_investigate_mine",     "office_building",
										"m02_security_hq",          "basement",
										"m02_security_hq",          "ground_floor",
										"m02_security_hq",          "first_floor",
										"m03_land_train",           "car_1",
										"m03_land_train",           "car_2",
										"m03_land_train",           "car_3",
										"m03_land_train",           "car_4",
										"m03_land_train",           "car_5",
										"m04_raid_on_containment",  "level_alphabeta",
										"m04_raid_on_containment",  "level_kappa",
										"m04_raid_on_containment",  "level_omega",
										"m05_sabotage_refinery",    "m5_mine",
										"m05_sabotage_refinery",    "m5_office_building",
										"m05_sabotage_refinery",    "refinery",
										"m07_mainland_base",        "compound",
										"m07_mainland_base",        "monorail__station",
										"m07_mainland_base",        "cable_car_building",
										"m08_entering_island_base", "cable_car_dock",
										"m08_entering_island_base", "manufacturing",
										"m08_entering_island_base", "quayside",
										"m08_entering_island_base", "reactor",
										"m08_entering_island_base", "security",
										"m10_submarine",            "submarine",
										"test_mission",             "test_mission",
										"test_mission",             "test_session_2",
										"m_ed",                     "2_ship"};

} // End of namespace ICB
