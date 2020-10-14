/** @file keyboard.cpp
	@brief
	This file contains movies routines

	TwinEngine: a Little Big Adventure engine

	Copyright (C) 2013 The TwinEngine team
	Copyright (C) 2008-2013 Prequengine team
	Copyright (C) 2002-2007 The TwinEngine team

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "keyboard.h"

/** Initialize engine auxiliar keymap */
uint8 pressedKeyMap[29] = {
	0x48, // 0
	0x50,
	0x4B,
	0x4D,
	0x47,
	0x49,
	0x51,
	0x4F, // 7

	0x39, // 8
	0x1C,
	0x1D,
	0x38,
	0x53,
	0x2A,
	0x36, // 14

	0x3B, // 15
	0x3C,
	0x3D,
	0x3E,
	0x3F,
	0x40, // LBAKEY_F6
	0x41,
	0x42,
	0x43,
	0x44,
	0x57,
	0x58,
	0x2A,
	0x0, // 28
};

uint16 pressedKeyCharMap[31] = {
	0x0100, // up
	0x0200, // down
	0x0400, // left
	0x0800, // right
	0x0500, // home
	0x0900, // pageup
	0x0A00, // pagedown
	0x0600, // end

	0x0101, // space bar
	0x0201, // enter
	0x0401,  // ctrl
	0x0801,  // alt
	0x1001,  // del
	0x2001,  // left shift
	0x2001,  // right shift

	0x0102,  // F1
	0x0202,  // F2
	0x0402,  // F3
	0x0802,  // F4
	0x1002,  // F5
	0x2002,  // F6
	0x4002,  // F7
	0x8002,  // F8

	0x0103,  // F9
	0x0203,  // F10
	0x0403,  // ?
	0x0803,  // ?
	0x00FF,  // left shift
	0x00FF,
	0x0,
	0x0,
};
