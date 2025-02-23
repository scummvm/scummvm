
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

#ifndef BAGEL_BAGLIB_RES_H
#define BAGEL_BAGLIB_RES_H

namespace Bagel {

// This section should not change, the first cursor is always invalid
#define BOF_INVALID 0

// OBJ
// Lists object types
#define BOF_BMP_OBJ 1
#define BOF_SPRITE_OBJ 2
#define BOF_BUTTON_OBJ 3
#define BOF_SOUND_OBJ 4
#define BOF_LINK_OBJ 5
#define BOF_TEXT_OBJ 6
#define BOF_CHAR_OBJ 7
#define BOF_VAR_OBJ 8
#define BOF_AREA_OBJ 9
#define BOF_MOVIE_OBJ 11
#define BOF_COMM_OBJ 12
#define BOF_EXPRESS_OBJ 13
#define BOF_THING_OBJ 14
#define BOF_RESPRNT_OBJ 15
#define BOF_DOSSIER_OBJ 16

// PAN
// These CANNOT change

#define BOF_LT_CURSOR 14
#define BOF_UP_CURSOR 15
#define BOF_RT_CURSOR 16
#define BOF_DN_CURSOR 17
#define BOF_TAKE_HAND 18
#define BOF_HAND 19

// WLD
#define INV_WLD "INV_WLD"
#define LOG_WLD "LOG_WLD"
#define LOGZ_WLD "LOGZ_WLD"
#define MAP_WLD "MAP_WLD"
#define MOO_WLD "MOO_WLD"
#define PDA_WLD "BPDA_WLD"
#define PDAZ_WLD "BPDAZ_WLD"
#define THUD_WLD "THUD_WLD"
#define WIELD_WLD "BWIELD_WLD"

} // namespace Bagel

#endif
