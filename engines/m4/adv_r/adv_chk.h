
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

#ifndef M4_ADV_R_ADV_CHK_H
#define M4_ADV_R_ADV_CHK_H

#include "m4/m4_types.h"
#include "m4/adv_r/adv.h"

namespace M4 {

/**
 * Reads the .DEF file for the specified room into the "room" structure.
 * @param room_code		room number
 * @param rdef			Output def to populate
 * @returns				Returns 0 if successful, or -1 for error
 */
int db_def_chk_read(int16 room_code, SceneDef *rdef);

} // End of namespace M4

#endif
