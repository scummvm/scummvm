/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001/2004 The ScummVM project
 * Copyright (C) 2002 Ph0x - GP32 Backend
 * Copyright (C) 2003/2004 DJWillis - GP32 Backend
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef SETUP_H
#define SETUP_H

#ifndef NULL
	#define NULL 0
#endif

/////////////////////////////////////////////////////////////////////////////
//
// ScummVM/GP32 - Configuration front-end launcher and splash-screen stuff.
//				  Also contains GP32 start-up code.
//                Not part of ScummVM backend as such.
//
/////////////////////////////////////////////////////////////////////////////

#define MENU_MUSICDRV	0
#define MENU_SCREENPOS	1
#define MENU_CPUSPEED	2

// OLD CONFIG
// NULL marks end (FIXME)
typedef struct {
	const char *option;
	const char **submenu;
	int index;
} tmenu;

const char *sautorun[] = {
"NOT IMPLEMENTED",
/*
"Off",
"On",
*/
//NULL
};

const char *ssounddrv[] = {
"-eadlib",
"-enull",
NULL
};

const char *sscreenpos[] = {
"Top",
"Center",
NULL
};

const char *scpuspeed[] = {
"66",
"100",
"120",
"132",
"156",
"166",
"180",
"200",
NULL
};

const char *sbrightness[] = {
"Normal",
"Lighter",
"Lightest",
"Darker",
"Darkest",
NULL
};

tmenu menu[] = {
{"Music Driver", ssounddrv, 0},
{"Screen Position", sscreenpos, 1},
{"CPU Speed (MHz)", scpuspeed, 3},
};

#else
	#warning GP32 LAUNCHER.H Called more then once.
#endif /* SETUP_H */
