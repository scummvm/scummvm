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

#ifndef MADS_PHANTOM_MAIN_MENU_H
#define MADS_PHANTOM_MAIN_MENU_H

#include "common/str.h"

namespace MADS {
namespace MADSV2 {
namespace Phantom {

#define COMMAND_LINE_MAX        10

#define FRAME_RATE              1
#define MENU_FRAME_RATE         3

#define NUM_MENU_ITEMS          7

#define MENU_APPEARING          0
#define MENU_ACCEPTING_COMMANDS 1
#define MENU_DISAPPEARING       2

#define MENU_HIGH_SPRITE        15

typedef struct {
	int handle;           /* Sprite series handle */
	int active;           /* Menu item is active  */
	int status;           /* Current status       */
} MenuItem;

extern bool new_background;
extern int selected_item;

extern void menu_control();

} // namespace Phantom
} // namespace MADSV2
} // namespace MADS

#endif
