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

#ifndef MADS_CORE_CURSOR_H
#define MADS_CORE_CURSOR_H

#include "mads/madsv2/core/general.h"

namespace MADS {
namespace MADSV2 {

#define CURSOR_OFF                   0       /* Cursor off                     */
#define CURSOR_ON                    1       /* Cursor on                      */
#define CURSOR_INSERT                2       /* Cursor in insert mode          */
#define CURSOR_OVERWRITE             3       /* Cursor in overwrite mode       */

extern int cursor_mode;          /* global cursor insert mode flag */
extern int cursor_follow;        /* global cursor follow flag      */

/**
 * Sets text cursor size
 */
extern void cursor_set_size(short start, short finish);

/**
 * Sets text cursor position
 */
extern void cursor_set_pos(short x, short y);

extern void cursor_get_pos(int *x, int *y);
extern void cursor_set_mode(int my_type);
extern void cursor_toggle_insert();
extern void cursor_set_follow(int follow);

} // namespace MADSV2
} // namespace MADS

#endif
