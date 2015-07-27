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

#ifndef ACCESS_MARTIAN_RESOURCES_H
#define ACCESS_MARTIAN_RESOURCES_H

#include "common/scummsys.h"

namespace Access {

namespace Martian {

extern const char *const FILENAMES[];

extern const int SIDEOFFR[];
extern const int SIDEOFFL[];
extern const int SIDEOFFU[];
extern const int SIDEOFFD[];

extern const byte *const CURSORS[4];

extern const int _travelPos[][2];

extern const int INVENTORY_SIZE;
extern const char *const INVENTORY_NAMES[];

extern const byte *const ROOM_TABLE[];
extern const char *const ROOM_DESCR[];
extern const int ROOM_NUMB;

extern const byte *const CHARTBL_MM[];

extern const int SIDEOFFR[];
extern const int SIDEOFFL[];
extern const int SIDEOFFU[];
extern const int SIDEOFFD[];

extern const byte CREDIT_DATA[];
extern const byte ICON_PALETTE[];

extern const int RMOUSE[10][2];

extern byte HELP[];
extern const char *const _askTBL[];
extern const char *const TRAVDATA[];

extern const byte DEATH_SCREENS[];
extern const char *const DEATHMESSAGE[];
extern const char *const SPEC7MESSAGE;

extern const byte _byte1EEB5[];
extern const int PICTURERANGE[][2];

} // End of namespace Martian
} // End of namespace Access

#endif /* ACCESS_MARTIAN_RESOURCES_H */
