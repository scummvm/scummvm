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

#ifndef ULTIMA_ULTIMA6_ACTORS_U6_WORK_TYPES_H
#define ULTIMA_ULTIMA6_ACTORS_U6_WORK_TYPES_H

// Worktype codes
#define WORKTYPE_U6_MOTIONLESS 0x0
#define WORKTYPE_U6_IN_PARTY 0x1
#define WORKTYPE_U6_PLAYER 0x2

#define WORKTYPE_U6_COMBAT_COMMAND 0x2
#define WORKTYPE_U6_COMBAT_FRONT 0x3
#define WORKTYPE_U6_COMBAT_REAR 0x4
#define WORKTYPE_U6_COMBAT_FLANK 0x5
#define WORKTYPE_U6_COMBAT_BERSERK 0x6
#define WORKTYPE_U6_COMBAT_RETREAT 0x7
#define WORKTYPE_U6_COMBAT_ASSAULT 0x8
#define WORKTYPE_U6_COMBAT_WILD 0x8
#define WORKTYPE_U6_COMBAT_SHY 0x9
#define WORKTYPE_U6_COMBAT_LIKE 0xa
#define WORKTYPE_U6_COMBAT_UNFRIENDLY 0xb

#define WORKTYPE_U6_ANIMAL_WANDER 0xc
#define WORKTYPE_U6_TANGLE   0xd
#define WORKTYPE_U6_IMMOBILE 0xe
#define WORKTYPE_U6_GUARD_WALK_EAST_WEST 0xf
#define WORKTYPE_U6_GUARD_WALK_NORTH_SOUTH 0x10

#define WORKTYPE_U6_LOOKOUT          0x11 // just a guess
#define WORKTYPE_U6_WALK_TO_LOCATION 0x86

#define WORKTYPE_U6_FACE_NORTH 0x87
#define WORKTYPE_U6_FACE_EAST 0x88
#define WORKTYPE_U6_FACE_SOUTH 0x89
#define WORKTYPE_U6_FACE_WEST 0x8a

#define WORKTYPE_U6_WALK_NORTH_SOUTH 0x8b
#define WORKTYPE_U6_WALK_EAST_WEST 0x8c

#define WORKTYPE_U6_WANDER_AROUND 0x8f
#define WORKTYPE_U6_WORK 0x90
#define WORKTYPE_U6_SLEEP 0x91
#define WORKTYPE_U6_PLAY_LUTE 0x95
#define WORKTYPE_U6_BEG 0x96
#define WORKTYPE_U6_ATTACK_PARTY 0x9b

#endif
