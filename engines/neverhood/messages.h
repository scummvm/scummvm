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

#ifndef NEVERHOOD_MESSAGES_H
#define NEVERHOOD_MESSAGES_H

namespace Neverhood {

enum NeverhoodMessage {
	NM_KEYPRESS_SPACE           = 0x0009,
	NM_KEYPRESS_ESC             = 0x000C,
	NM_ANIMATION_START          = 0x100D,
	NM_ANIMATION_UPDATE         = 0x2000,
	NM_ANIMATION_STOP           = 0x3002,
	NM_LEVER_UP                 = 0x4807,
	NM_DOOR_OPEN                = 0x4808,
	NM_DOOR_CLOSE               = 0x4809,
	NM_LEVER_DOWN               = 0x480F,
	NM_KLAYMEN_PICKUP			= 0x4812,
	NM_KLAYMEN_PRESS_BUTTON		= 0x4816,
	NM_KLAYMEN_INSERT_DISK		= 0x481A,
	NM_KLAYMEN_RELEASE_LEVER	= 0x4827
};

} // End of namespace Neverhood

#endif /* NEVERHOOD_MESSAGES_H */
