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

#ifndef NUVIE_SAVE_OBJ_LIST_H
#define NUVIE_SAVE_OBJ_LIST_H

namespace Ultima {
namespace Nuvie {

#define OBJLIST_OFFSET_PARTY_NAMES     0xf00
#define OBJLIST_OFFSET_PARTY_ROSTER    0xfe0
#define OBJLIST_OFFSET_NUM_IN_PARTY    0xff0

//U6 specific offsets
#define OBJLIST_OFFSET_U6_MOVEMENT_POINTS 0x14f1
#define OBJLIST_OFFSET_U6_TALK_FLAGS   0x17f1
#define OBJLIST_OFFSET_U6_MOVEMENT_FLAGS 0x19f1
#define OBJLIST_OFFSET_U6_QUEST_FLAG   0x1bf1
#define OBJLIST_OFFSET_U6_REST_COUNTER 0x1bf2
#define OBJLIST_OFFSET_U6_GAMETIME     0x1bf3
#define OBJLIST_OFFSET_U6_KARMA        0x1bf9

#define OBJLIST_OFFSET_U6_WIND_DIR     0x1bfa
#define OBJLIST_OFFSET_U6_TIMERS       0x1c03
#define OBJLIST_OFFSET_U6_ECLIPSE      0x1c12
#define OBJLIST_OFFSET_U6_ALCOHOL      0x1c17
#define OBJLIST_OFFSET_U6_MOONSTONES   0x1c1b
#define OBJLIST_OFFSET_U6_GARGISH_LANG 0x1c5f

#define OBJLIST_OFFSET_U6_COMBAT_MODE  0x1c69
#define OBJLIST_OFFSET_U6_SOLO_MODE    0x1c6a
#define OBJLIST_OFFSET_U6_COMMAND_BAR  0x1c6c
#define OBJLIST_OFFSET_U6_GENDER       0x1c71

//MD specific offsets

#define OBJLIST_OFFSET_MD_MOVEMENT_POINTS    0x15f1
#define OBJLIST_OFFSET_MD_TALK_FLAGS         0x18f1
#define OBJLIST_OFFSET_MD_MOVEMENT_FLAGS     0x1af1
#define OBJLIST_OFFSET_MD_BLUE_BERRY_COUNTER 0x1d05
#define OBJLIST_OFFSET_MD_COMMAND_BAR        0x1d26
#define OBJLIST_OFFSET_MD_GENDER             0x1d27
#define OBJLIST_OFFSET_MD_BERRY_TIMERS       0x1d2f

//WOU specific offsets (MD and SE)

#define OBJLIST_OFFSET_WOU_GAMETIME     0x1cf3

#define OBJLIST_PARTY_MODE 0xff

//u6 wind direction
#define OBJLIST_U6_WIND_DIR_N  0
#define OBJLIST_U6_WIND_DIR_NE 1
#define OBJLIST_U6_WIND_DIR_E  2
#define OBJLIST_U6_WIND_DIR_SE 3
#define OBJLIST_U6_WIND_DIR_S  4
#define OBJLIST_U6_WIND_DIR_SW 5
#define OBJLIST_U6_WIND_DIR_W  6
#define OBJLIST_U6_WIND_DIR_NW 7
#define OBJLIST_U6_WIND_DIR_C  0xff

#define OBJLIST_OFFSET_SE_COMMAND_BAR  0x1d6c

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
