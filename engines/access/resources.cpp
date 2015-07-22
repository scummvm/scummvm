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

#include "access/resources.h"
#include "access/access.h"

namespace Access {

const byte INITIAL_PALETTE[18 * 3] = {
	0x00, 0x00, 0x00,
	0xff, 0xff, 0xff,
	0xf0, 0xf0, 0xf0,
	0xe0, 0xe0, 0xe0,
	0xd0, 0xd0, 0xd0,
	0xc0, 0xc0, 0xc0,
	0xb0, 0xb0, 0xb0,
	0xa0, 0xa0, 0xa0,
	0x90, 0x90, 0x90,
	0x80, 0x80, 0x80,
	0x70, 0x70, 0x70,
	0x60, 0x60, 0x60,
	0x50, 0x50, 0x50,
	0x40, 0x40, 0x40,
	0x30, 0x30, 0x30,
	0x20, 0x20, 0x20,
	0x10, 0x10, 0x10,
	0x00, 0x00, 0x00
};

const char *const GENERAL_MESSAGES[] = {
	"LOOKING THERE REVEALS NOTHING OF INTEREST.", // LOOK_MESSAGE
	"THAT DOESN'T OPEN.",               // OPEN_MESSAGE
	"THAT WON'T MOVE."                  // MOVE_MESSAGE
	"YOU CAN'T TAKE THAT.",             // GET_MESSAGE
	"THAT DOESN'T SEEM TO WORK.",       // USE_MESSAGE
	"YOU CAN'T CLIMB THAT.",            // GO_MESSAGE
	"THERE SEEMS TO BE NO RESPONSE.",   // TALK_MESSAGE
	"THIS OBJECT REQUIRES NO HINTS",    // HELP_MESSAGE
	"THIS OBJECT REQUIRES NO HINTS",    // HELP_MESSAGE
	"THAT DOESN'T SEEM TO WORK.",       // USE_MESSAGE
};

const int INVCOORDS[][4] = {
	{ 23, 68, 15, 49 },
	{ 69, 114, 15, 49 },
	{ 115, 160, 15, 49 },
	{ 161, 206, 15, 49 },
	{ 207, 252, 15, 49 },
	{ 253, 298, 15, 49 },
	{ 23, 68, 50, 84 },
	{ 69, 114, 50, 84 },
	{ 115, 160, 50, 84 },
	{ 161, 206, 50, 84 },
	{ 207, 252, 50, 84 },
	{ 253, 298, 50, 84 },
	{ 23, 68, 85, 119 },
	{ 69, 114, 85, 119 },
	{ 115, 160, 85, 119 },
	{ 161, 206, 85, 119 },
	{ 207, 252, 85, 119 },
	{ 253, 298, 85, 119 },
	{ 23, 68, 120, 154 },
	{ 69, 114, 120, 154 },
	{ 115, 160, 120, 154 },
	{ 161, 206, 120, 154 },
	{ 207, 252, 120, 154 },
	{ 253, 298, 120, 154 },
	{ 237, 298, 177, 193 },
	{ 25, 85, 177, 193 }
};

} // End of namespace Access
