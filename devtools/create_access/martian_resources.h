/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#ifndef __MARTIAN_RESOURCES_H__
#define __MARTIAN_RESOURCES_H__

#include "common/scummsys.h"

namespace Martian {

#define MARTIAN_NUM_CURSORS 4
extern const byte *const CURSORS[MARTIAN_NUM_CURSORS];
extern const uint CURSOR_SIZES[MARTIAN_NUM_CURSORS];

extern const char *const ROOM_DESCR[48];

extern const char *const DEATH_TEXT_ENG[20];
extern const byte DEATH_SCREENS_ENG[20];
extern const char *const INVENTORY_NAMES_ENG[55];
extern const int COMBO_TABLE[85][4];

extern const char *const NO_HELP_MESSAGE_ENG;
extern const char *const NO_HINTS_MESSAGE_ENG;
extern const char *const RIVER_HIT1_ENG;
extern const char *const RIVER_HIT2_ENG;
extern const char *const BAR_MESSAGE_ENG;
extern const char *const HELPLVLTXT_ENG[3];
extern const char *const IQLABELS_ENG[9];
extern const char *const CANT_GET_THERE_ENG;

} // End of namespace Amazon

#endif
