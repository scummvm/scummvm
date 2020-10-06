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

#ifndef ICB_GLOBAL_OBJECTS
#define ICB_GLOBAL_OBJECTS

#include "engines/icb/p4.h"
#include "engines/icb/line_of_sight.h"
#include "engines/icb/game_script.h"
#include "engines/icb/global_objects_pc.h"

namespace ICB {

// Define the minimum distance an actor can be from the camera until he/she is clipped in cm
const int g_actor_hither_plane = 100;    // 1m
const int g_actor_far_clip_plane = 8000; // 80m

class res_man;
// this is the main resource manager object
// this remains as NULL if the engine finds no resources
// There are currently physically only 2 global res_man's and 1 private res_man inside game_session
extern res_man *rs1;
extern res_man *rs2;
extern res_man *rs3;

extern res_man *private_session_resman;

// But we have pointers to lots of others, so the PC & PSX can choose which res_man to use for
// the different resources which need to be loaded in
extern res_man *rs_bg;     // pointer to which res_man to use for background buffer
extern res_man *rs_icons;  // pointer to which res_man to use for icons
extern res_man *rs_anims;  // pointer to which res_man to use for animation data
extern res_man *rs_remora; // pointer to which res_man to use for remora graphics
extern res_man *rs_font;   // pointer to which res_man to use for font data

// global test file only in this resman
extern res_man *global_text_resman;
extern _linked_data_file *global_text;

extern text_sprite *g_text_bloc1;
extern text_sprite *g_text_bloc2;

class _mission;
// holds info about current mission
extern _mission *g_mission;

extern _stub stub;

// game script manager
extern _game_script gs;

extern _mission *g_icb_mission;
extern _game_session *g_icb_session;

// session objects
extern _barrier_handler g_icb_session_barriers;
extern _floor_world *g_icb_session_floors;
extern _logic logics[MAX_session_objects];
extern _mega megas[MAX_voxel_list];
extern _vox_image vox_images[MAX_voxel_list];

// For choosing different sessions & missions
#define NUMBER_OF_MISSIONS (11)
#define MAX_SESSIONS (29 * 2)
#define MAX_MISSIONS (NUMBER_OF_MISSIONS * 2)

extern const char *g_sessions[MAX_SESSIONS];
extern const char *g_mission_startup_names[MAX_MISSIONS];
extern const char *g_mission_names[NUMBER_OF_MISSIONS];

extern const char *g_m01;
extern const char *g_m02;
extern const char *g_m03;
extern const char *g_m04;
extern const char *g_m05;
extern const char *g_m07;
extern const char *g_m08;
extern const char *g_m09;
extern const char *g_m10;

// Prop sprite brightness to use when highlighting a prop that the player could interact with
extern int g_prop_select_r;
extern int g_prop_select_g;
extern int g_prop_select_b;

// extra ambient brightness to use when highlighting a mega that the player could interact with
extern int g_mega_select_r;
extern int g_mega_select_g;
extern int g_mega_select_b;

} // End of namespace ICB

#endif
