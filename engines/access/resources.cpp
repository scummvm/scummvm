/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, 0xwhose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, 0xor (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, 0xwrite to the Free Software
 * Foundation, 0xInc., 0x51 Franklin Street, 0xFifth Floor, 0xBoston, 0xMA 02110-1301, 0xUSA.
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

const int SIDEOFFR[] = { 5, 5, 5, 5, 5, 5, 5, 5, 0 };
const int SIDEOFFL[] = { 5, 5, 5, 5, 5, 5, 5, 5, 0 };
const int SIDEOFFU[] = { 2, 2, 2, 2, 2, 2, 2, 2, 0 };
const int SIDEOFFD[] = { 2, 2, 2, 2, 2, 2, 2, 2, 0 };
const int DIAGOFFURX[] = { 4, 5, 2, 2, 3, 4, 2, 2, 0 };
const int DIAGOFFURY[] = { 2, 3, 2, 2, 2, 3, 1, 1, 0 };
const int DIAGOFFDRX[] = { 4, 5, 4, 3, 5, 4, 5, 1, 0 };
const int DIAGOFFDRY[] = { 3, 2, 1, 2, 2, 1, 2, 1, 0 };
const int DIAGOFFULX[] = { 4, 5, 4, 3, 3, 2, 2, 2, 0 };
const int DIAGOFFULY[] = { 3, 3, 1, 2, 2, 1, 1, 1, 0 };
const int DIAGOFFDLX[] = { 4, 5, 3, 3, 5, 4, 6, 1, 0 };
const int DIAGOFFDLY[] = { 2, 2, 1, 2, 3, 1, 2, 1, 0 };

const int RMOUSE[10][2] = {
	{ 0, 35 }, { 0, 0 }, { 36, 70 }, { 71, 106 }, { 107, 141 },
	{ 142, 177 }, { 178, 212 }, { 213, 248 }, { 249, 283 }, { 284, 318 }
};

const char *LOOK_MESSAGE = "LOOKING THERE REVEALS NOTHING OF INTEREST.";
const char *GET_MESSAGE = "YOU CAN'T TAKE THAT.";
const char *OPEN_MESSAGE = "THAT DOESN'T OPEN.";
const char *MOVE_MESSAGE = "THAT WON'T MOVE.";
const char *USE_MESSAGE = "THAT DOESN'T SEEM TO WORK.";
const char *GO_MESSAGE = "YOU CAN'T CLIMB THAT.";
const char *HELP_MESSAGE = "THIS OBJECT REQUIRES NO HINTS";
const char *TALK_MESSAGE      = "THERE SEEMS TO BE NO RESPONSE.";
const char *const GENERAL_MESSAGES[] = {
	LOOK_MESSAGE, OPEN_MESSAGE, MOVE_MESSAGE, GET_MESSAGE, USE_MESSAGE,
	GO_MESSAGE, TALK_MESSAGE, HELP_MESSAGE, HELP_MESSAGE, USE_MESSAGE
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
