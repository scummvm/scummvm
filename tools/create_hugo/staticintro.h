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

#ifndef STATICINTRO_H
#define STATICINTRO_H

#define NUM_INTRO_TEXT 3
#define NUM_INTRO_TICK 36
// We use intro_tick as an index into the following coordinate list for the plane path.
const byte x_intro[NUM_INTRO_TICK] = {
	210, 204, 198, 192, 186, 180, 174, 168, 162, 156,
	152, 149, 152, 158, 165, 171, 170, 165, 161, 157,
	150, 144, 138, 134, 133, 134, 138, 144, 146, 142,
	137, 132, 128, 124, 120, 115
};

const byte y_intro[NUM_INTRO_TICK] = {
	61,  61,  61,  61,  61,  61,  61,  61,  61,  61,
	63,  66,  71,  74,  72,  75,  80,  82,  83,  84,
	84,  84,  85,  89,  94,  99, 103, 104, 100,  98,
	100, 103, 106, 109, 111, 112
};

const char *textIntro[NUM_INTRO_TEXT] = {
	"Hugo and Penelope are returning\nhome from their vacation at the\ncottage of Great Uncle Horace.",
	"Suddenly, a freak magnetic storm\ncauses the compass in their light\naircraft to spin wildly!  Unable\nto navigate, Hugo loses all sense\nof direction...",
	"Finally, hopelessly lost over a\nSouth American Jungle, the plane\nabout to run out of gas, Hugo\nspots a clearing just big enough\nto land it.\n\nWith fingers clenching the controls\nhe shouts:  Hold on Penelope, we're\ngoing down...!"
};

#endif
