/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef QUEENDEFS_H
#define QUEENDEFS_H

namespace Queen {


enum {
	GAME_SCREEN_WIDTH  = 320,
	GAME_SCREEN_HEIGHT = 200,
	ROOM_ZONE_HEIGHT   = 150,
	PANEL_ZONE_HEIGHT  =  50
};


enum {
	FRAMES_JOE      = 36,
	FRAMES_JOE_XTRA = 2,
	FRAMES_JOURNAL  = 40
};


enum {
	DIR_LEFT  = 1,
	DIR_RIGHT = 2,
	DIR_FRONT = 3,
	DIR_BACK  = 4
};


enum {
	INK_BG_PANEL      = 0xE2,
	INK_SELECT        = 17,
	INK_JOURNAL       = 16,
	INK_NORMAL        = 1,
	INK_TALK_NORMAL   = 1,
	INK_JOE           = 17,
	INK_OUTLINED_TEXT = 16
};


enum {
	ZONE_ROOM  = 0,
	ZONE_PANEL = 1
};


} // End of namespace Queen

#endif
