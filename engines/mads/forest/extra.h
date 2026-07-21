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

#ifndef MADS_FOREST_EXTRA_H
#define MADS_FOREST_EXTRA_H

#include "mads/core/general.h"

namespace MADS {
namespace MADSV2 {
namespace Forest {

// Interface sprites
enum {
	fx_int_journal          = 0,
	fx_int_backpack         = 1,
	fx_int_candle           = 2,
	fx_int_exit             = 3,
	fx_int_dooropen         = 4,
	fx_int_candle_on        = 5
};

enum InterfaceButton {
	JOURNAL_FLY            = 1, 
	BP_FLY                 = 2,
	CANDLE_FLY             = 3,
	DOOR_FLY               = 4
};

/**************               NOTE:                 *********************/
/** IF following values are changed, then recompile EXTRA.C & GAME_3.C **/

#define JOURNAL_X              8     /* X for top left corner of JOURNAL */
#define JOURNAL_Y              3     /* Y for top left corner of JOURNAL */
#define BP_X                   68    /* X for top left corner of BP      */
#define BP_Y                   1     /* Y for top left corner of BP      */
#define CANDLE_X               194   /* X for top left corner of CANDLE  */
#define CANDLE_Y               4     /* Y for top left corner of CANDLE  */
#define DOOR_X                 264   /* X for top left corner of DOOR    */
#define DOOR_Y                 2     /* Y for top left corner of DOOR    */

extern int int_sprite[6];
extern bool knuthole_flag;
extern int paul_object_showing;

extern void init_extra();

inline void extra_blank_knothole() {
	knuthole_flag = 0;
}

extern void clear_selected_item();
extern void display_inventory();
extern void solve_me_selected();
extern void door_selected();

extern void load_interface();
extern void unload_interface();
extern void draw_interface();
extern void do_interface();

extern void stamp_sprite_to_interface(int x, int y, int sprite, int series);
extern void delete_sprite_in_interface(int series);
extern void inter_update_series(int series_id);
extern void open_interface(InterfaceButton button);
extern void close_interface(InterfaceButton button);
extern void extra_change_animation(int handle, int x, int y, int scale, int depth);

} // namespace Forest
} // namespace MADSV2
} // namespace MADS

#endif
