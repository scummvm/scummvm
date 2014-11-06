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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef ACCESS_AMAZON_RESOURCES_H
#define ACCESS_AMAZON_RESOURCES_H

#include "common/scummsys.h"

namespace Access {

namespace Amazon {

extern const char *const FILENAMES[];

extern const byte *CURSORS[10];

extern const int TRAVEL_POS[][2];

extern const byte *ROOM_TABLE[];

extern const byte *CHARTBL[];

extern const char *const INVENTORY_NAMES[];

extern const int FONT2_INDEX[];

extern const byte FONT2_DATA[];

extern const int FONT6x6_INDEX[];

extern const byte FONT6x6_DATA[];

extern const char *const NO_HELP_MESSAGE;
extern const char *const NO_HINTS_MESSAGE;

extern const byte DEATH_SCREENS[58];

extern const char *const DEATH_TEXT[58];

extern const int DEATH_CELLS[12][3];

extern const int CHAPTER_CELLS[17][3];

extern const int CHAPTER_TABLE[14][5];

extern const int CHAPTER_JUMP[14];

extern const int COMBO_TABLE[85][4];

} // End of namespace Amazon

} // End of namespace Access

#endif /* ACCESS_AMAZON_RESOURCES_H */
