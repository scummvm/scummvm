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
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#ifndef STATICDISPLAY_H
#define STATICDISPLAY_H

#define SIZE_PAL_ARRAY 64

// Color table of standard 16 VGA colors
// Values from "Programmers guide to EGA/VGA cards" Ferraro, p303
#define V1 168                                      // Low intensity value
#define V2 255                                      // High intensity value
#define V3 87                                       // Special for Brown/Gray
#define V4 32                                       // De-saturate hi intensity


byte _palette[SIZE_PAL_ARRAY] = {
	0,   0,   0, 0,                               // BLACK
	0,   0,  V1, 0,                               // BLUE
	0,  V1,   0, 0,                               // GREEN
	0,  V1,  V1, 0,                               // CYAN
	V1,   0,   0, 0,                               // RED
	V1,   0,  V1, 0,                               // MAGENTA
	V1,  V3,   0, 0,                               // BROWN
	V1,  V1,  V1, 0,                               // WHITE (LIGHT GRAY)
	V3,  V3,  V3, 0,                               // GRAY  (DARK GRAY)
	V4,  V4,  V2, 0,                               // LIGHTBLUE
	V4,  V2,  V4, 0,                               // LIGHTGREEN
	V4,  V2,  V2, 0,                               // LIGHTCYAN
	V2,  V4,  V4, 0,                               // LIGHTRED
	V2,  V4,  V2, 0,                               // LIGHTMAGENTA
	V2,  V2,  V4, 0,                               // YELLOW
	V2,  V2,  V2, 0                                // BRIGHTWHITE
};

#endif
