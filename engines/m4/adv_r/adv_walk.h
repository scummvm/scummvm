
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

#ifndef M4_ADV_R_ADV_WALK_H
#define M4_ADV_R_ADV_WALK_H

#include "m4/m4_types.h"
#include "m4/adv_r/adv.h"
#include "m4/wscript/ws_machine.h"

namespace M4 {

class Walker {
public:
	virtual ~Walker() {}

	virtual bool walk_load_walker_and_shadow_series() = 0;
	virtual machine *walk_initialize_walker() = 0;
};

void set_walker_scaling(SceneDef *rdef);

/**
 * Called every time s/he hits a node
 */
bool walker_has_walk_finished(machine *sender);
void ws_demand_location(machine *myWalker, int32 x, int32 y, int facing = -1);
void ws_demand_facing(machine *myWalker, int32 newFacing);
void ws_turn_to_face(machine *myWalker, int32 facing, int32 trigger = -1);
void ws_hide_walker(machine *myWalker);
void ws_unhide_walker(machine *myWalker);
void ws_walk(machine *myWalker, int32 x, int32 y, GrBuff **, int16 trigger, int32 finalFacing, bool complete_walk = true);

void ws_demand_location(int32 x, int32 y, int facing = -1);
void ws_demand_facing(int32 newFacing);
void ws_turn_to_face(int32 facing, int32 trigger = -1);
void ws_hide_walker();
void ws_unhide_walker();
void ws_walk(int32 x, int32 y, GrBuff **buffer, int16 trigger, int32 finalFacing = -1, bool complete_walk = true);

void ws_get_walker_info(machine *myWalker, int32 *x, int32 *y, int32 *s, int32 *layer, int32 *facing);

bool ws_walk_init_system();

bool ws_walk_load_series(const int16 *dir_array, const char *name_array[], bool shadow_flag, bool load_palette);
bool ws_walk_load_walker_series(const int16 *dir_array, const char *name_array[], bool load_palette = false);
bool ws_walk_load_shadow_series(const int16 *dir_array, const char *name_array[]);

void ws_walk_dump_series(int16 num_directions, int16 start_hash);
#define ws_walk_dump_walker_series(xx, yy) (ws_walk_dump_series (xx, yy))
#define ws_walk_dump_shadow_series(xx, yy) (ws_walk_dump_series (xx, yy))

#define player_walk(xx, yy, ff, tt)    (ws_walk(_G(my_walker), xx, yy, nullptr, tt, ff, true))
#define player_walk_no_finish(xx, yy, ff, tt)      (ws_walk(_G(my_walker), xx, yy, nullptr, tt, ff, FALSE))
#define player_demand_facing(dd)       (ws_demand_facing(_G(my_walker), dd))
#define player_demand_location(xx, yy) (ws_demand_location(_G(my_walker), xx, yy))
#define player_turn_to_face(dd, tt) (ws_turn_to_face(_G(my_walker), dd, tt))
#define player_hide()                  (ws_hide_walker(_G(my_walker)))
#define player_unhide()                (ws_unhide_walker(_G(my_walker)))
#define player_get_info()              (player_update_info(_G(my_walker), &_G(player_info)))


// New walking stuff
void ws_custom_walk(machine *myWalker, int32 finalFacing, int32 trigger, bool complete_walk = true);
#define adv_walker_custom_walk(ww, ff, tt)      (ws_custom_walk(ww, ff, tt, true))
#define adv_walker_custom_walk_no_finish(ww, ff, tt)     (ws_custom_walk(ww, ff, tt, FALSE))
#define adv_walker_walk(ww, xx, yy, ff, tt)     (ws_walk(ww, xx, yy, nullptr, tt, ff, true))
#define adv_walker_walk_no_finish(ww, xx, yy, ff, tt)    (ws_walk(ww, xx, yy, nullptr, tt, ff, FALSE))
#define adv_walker_face(ww, dd)              (ws_demand_facing(ww, dd))
#define adv_walker_turn_to_face(ww, dd, tt)  (ws_turn_to_face(ww, dd, tt))
#define adv_walker_move(ww, xx, yy)          (ws_demand_location(ww, xx, yy))
#define adv_walker_hide(ww)                  (ws_hide_walker(ww))
#define adv_walker_unhide(ww)                (ws_unhide_walker(ww))
bool adv_walker_path_exists(machine *myWalker, int32 x, int32 y);

void adv_hyperwalk_to_final_destination(void *, void *);

} // End of namespace M4

#endif
