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

#ifndef STATICUTIL_H
#define STATICUTIL_H

#define NUM_UTIL_TEXT 8

const char *textUtil[NUM_UTIL_TEXT] = {
	"\n\nPlease read the supplied 'technote' file which may contain information on this problem.",
	"File not found: ",
	"Unable to write file.\nDisk full or perhaps read-only?\n",
	"Bad data file format:\n",
	"Insufficient memory to run game.\n",
	"Sound missing from sound file:\n",
	"An error has occurred.\n",
	"I'm afraid all you can do at this point is:\n\n-  Load a saved game (Ctrl+L)\n-  Start a new game (Ctrl+N)\n-  Quit! (Alt+F4)"
//	"No timers available, try again later.\n",
//	"Unable to find or load VBX file:\n"
};

#endif //STATICENGINE_H
