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

#ifndef MADS_CORE_EXTRA_H
#define MADS_CORE_EXTRA_H

#include "mads/madsv2/core/general.h"

namespace MADS {
namespace MADSV2 {


#define JOURNAL_FLY            1 
#define BP_FLY                 2
#define CANDLE_FLY             3
#define DOOR_FLY               4

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

extern void fly_on_screen(int flying_object);
extern void fly_off_screen(int flying_object);

extern void display_inventory(void);
extern void display_journal(void);
extern void leave_journal(void);
extern void solve_me_selected(void);
extern void door_selected(void);

extern void extra_spinning_object(void);
extern void extra_inven_preserve_palette(void);
extern void stamp_sprite_to_interface(int x, int y, int sprite, int series);
extern void delete_sprite_in_interface(int series);
extern void extra_change_animation(int handle, int x, int y, byte scale, byte depth);
extern void extra_shift_animation(int handle, int x, int y, byte scale);
extern void extra_blank_knothole(void);

} // namespace MADSV2
} // namespace MADS

#endif
