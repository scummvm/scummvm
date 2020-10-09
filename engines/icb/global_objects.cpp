/* ResidualVM - A 3D game interpreter
 *
 * ResidualVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the AUTHORS
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1999-2000 Revolution Software Ltd.
 * This code is based on source code created by Revolution Software,
 * used with permission.
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

// global objects are in here - this makes it easier to see what there is
#include "engines/icb/global_objects.h"
#include "engines/icb/sound_logic.h"

namespace ICB {

int globalCharSpacing = 0;

// Prop sprite brightness to use when highlighting a prop that the player could interact with
int g_prop_select_r = 200;
int g_prop_select_g = 200;
int g_prop_select_b = 200;

// extra ambient brightness to use when highlighting a mega that the player could interact with
int g_mega_select_r = 200;
int g_mega_select_g = 200;
int g_mega_select_b = 200;

// holds info about current mission
_mission *g_mission = NULL;

// main buffer object
// if this remains NULL then the system knows that there are no resources present
// this should be used as a high level indication of the situation
res_man *rs1 = NULL;
res_man *rs2 = NULL;
res_man *rs3 = NULL;

res_man *private_session_resman = NULL;

res_man *rs_bg = NULL;     // pointer to which res_man to use for background buffer
res_man *rs_icons = NULL;  // pointer to which res_man to use for icons
res_man *rs_anims = NULL;  // pointer to which res_man to use for animation data
res_man *rs_remora = NULL; // pointer to which res_man to use for remora graphics
res_man *rs_font = NULL;   // pointer to which res_man to use for font data

// global test only
res_man *global_text_resman = NULL;
_linked_data_file *global_text;

// route-manager service object
// doesnt depend on session specific extrernal data so we make it global service
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
_logic logics[MAX_session_objects];
_mega megas[MAX_voxel_list];
_vox_image vox_images[MAX_voxel_list];

// master modes
_stub stub;

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

// For test_mission
const char *test_mission = "test_mission";
const char *test_session = "test_session";

const char *test_session_2 = "test_session_2";

// For m01_investigate_mine
const char *g_m01 = "m01_investigate_mine";
const char *office_building = "office_building";

const char *mine = "mine";
const char *mineworkings = "mineworkings";

// For m02_security_hq
const char *g_m02 = "m02_security_hq";
const char *basement = "basement";

const char *ground_floor = "ground_floor";
const char *first_floor = "first_floor";

// For m03_land_train
const char *g_m03 = "m03_land_train";
const char *car_5 = "car_5";

const char *car_1 = "car_1";
const char *car_2 = "car_2";
const char *car_3 = "car_3";
const char *car_4 = "car_4";

// For m04_raid_on_containment
const char *g_m04 = "m04_raid_on_containment";
const char *level_alphabeta = "level_alphabeta";

const char *level_kappa = "level_kappa";
const char *level_omega = "level_omega";

// For m05_sabotage_refinery
const char *g_m05 = "m05_sabotage_refinery";
const char *m5_mine = "m5_mine";

const char *m5_office_building = "m5_office_building";
const char *m5_refinery = "refinery";

// For m07_mainland_base
const char *g_m07 = "m07_mainland_base";
const char *compound = "compound";

const char *monorail_station = "monorail__station";
const char *cable_car_building = "cable_car_building";

// For m08_entering_island_base
const char *g_m08 = "m08_entering_island_base";
const char *cable_car_dock = "cable_car_dock";

// Mission 9 which is mission 8 but with a global set
const char *g_m09 = "m09_escape_island_base";
const char *security = "security";

const char *manufacturing = "manufacturing";
const char *quayside = "quayside";
const char *reactor = "reactor";

const char *m_ed = "m_ed";
const char *a2_ship = "2_ship";

// For m10_submarine
const char *g_m10 = "m10_submarine";
const char *submarine = "submarine";

// The collection of possible missions including the session to start for each mission
const char *g_mission_startup_names[MAX_MISSIONS] = {g_m01, office_building, g_m02, basement, g_m03, car_5,     g_m04, level_alphabeta, g_m05,        m5_mine,     g_m07, compound,
                                                     g_m08, cable_car_dock,  g_m08, security, g_m10, submarine, m_ed,  a2_ship,         test_mission, test_session};

const char *g_mission_names[NUMBER_OF_MISSIONS] = {g_m01, g_m02, g_m03, g_m04, g_m05, g_m07, g_m08, g_m08, g_m10, m_ed, test_mission};

const char *g_sessions[MAX_SESSIONS] = {g_m01,        mine,
                                        g_m01,        mineworkings,
                                        g_m01,        office_building,
                                        g_m02,        basement,
                                        g_m02,        ground_floor,
                                        g_m02,        first_floor,
                                        g_m03,        car_1,
                                        g_m03,        car_2,
                                        g_m03,        car_3,
                                        g_m03,        car_4,
                                        g_m03,        car_5,
                                        g_m04,        level_alphabeta,
                                        g_m04,        level_kappa,
                                        g_m04,        level_omega,
                                        g_m05,        m5_mine,
                                        g_m05,        m5_office_building,
                                        g_m05,        m5_refinery,
                                        g_m07,        compound,
                                        g_m07,        monorail_station,
                                        g_m07,        cable_car_building,
                                        g_m08,        cable_car_dock,
                                        g_m08,        manufacturing,
                                        g_m08,        quayside,
                                        g_m08,        reactor,
                                        g_m08,        security,
                                        g_m10,        submarine,
                                        test_mission, test_session,
                                        test_mission, test_session_2,
                                        m_ed,         a2_ship};

} // End of namespace ICB
