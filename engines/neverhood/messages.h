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

#ifndef NEVERHOOD_MESSAGES_H
#define NEVERHOOD_MESSAGES_H

namespace Neverhood {

enum NeverhoodMessage {
	NM_MOUSE_MOVE               = 0x0000,
	NM_MOUSE_CLICK              = 0x0001,
	NM_MOUSE_RELEASE            = 0x0002,
	NM_MOUSE_HIDE               = 0x101D,
	NM_MOUSE_SHOW               = 0x101E,
	NM_KEYPRESS_SPACE           = 0x0009,
	NM_KEYPRESS_ESC             = 0x000C,
	NM_ANIMATION_START          = 0x100D,
	NM_SCENE_LEAVE              = 0x1019,
	NM_PRIORITY_CHANGE          = 0x1022,
	NM_ANIMATION_UPDATE         = 0x2000,
	NM_POSITION_CHANGE          = 0x2002,
	NM_KLAYMEN_CLIMB_LADDER     = 0x2005,
	NM_KLAYMEN_STOP_CLIMBING    = 0x2006,

	NM_CAR_MOVE_TO_PREV_POINT   = 0x2007,
	NM_CAR_MOVE_TO_NEXT_POINT   = 0x2008,
	NM_CAR_ENTER                = 0x2009,
	NM_CAR_LEAVE                = 0x200A,
	NM_CAR_TURN                 = 0x200E,
	NM_CAR_AT_HOME              = 0x200F,

	NM_ANIMATION_STOP           = 0x3002,

	NM_KLAYMEN_STAND_IDLE       = 0x4004,
	NM_KLAYMEN_USE_OBJECT       = 0x4806,
	NM_KLAYMEN_RAISE_LEVER      = 0x4807,
	NM_KLAYMEN_OPEN_DOOR        = 0x4808,
	NM_KLAYMEN_CLOSE_DOOR       = 0x4809,
	NM_KLAYMEN_MOVE_OBJECT      = 0x480A,
	NM_KLAYMEN_LOWER_LEVER      = 0x480F,
	NM_KLAYMEN_PICKUP           = 0x4812,
	NM_KLAYMEN_PRESS_BUTTON     = 0x4816,
	NM_KLAYMEN_INSERT_DISK      = 0x481A,
	NM_KLAYMEN_TURN_TO_USE      = 0x481D,
	NM_KLAYMEN_RETURN_FROM_USE  = 0x481E,
	NM_KLAYMEN_RELEASE_LEVER    = 0x4827,

	NM_MOVE_TO_BACK             = 0x482A,
	NM_MOVE_TO_FRONT            = 0x482B,

	// New to ScummVM
	NM_MOUSE_WHEELUP            = 0xF000,
	NM_MOUSE_WHEELDOWN          = 0xF001
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MESSAGES_H */
