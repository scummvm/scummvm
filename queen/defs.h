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

// Maybe should be an inlined function somwhere else, feel free to change
#define InRange(x,l,h)     ((x)<=(h) && (x)>=(l)) /* X in [l..h] */

enum {
	COMPRESSION_NONE  = 0,
	COMPRESSION_MP3   = 1,	
	COMPRESSION_OGG   = 2
};

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


enum Direction {
	DIR_LEFT  = 1,
	DIR_RIGHT = 2,
	DIR_FRONT = 3,
	DIR_BACK  = 4
};


enum {
	INK_BG_PANEL      = 226,
	INK_JOURNAL       = 16,
	INK_CMD_SELECT    = 17,
	INK_CMD_NORMAL    = 1,
	INK_CMD_LOCK      = 234,
	INK_TALK_NORMAL   = 1,
	INK_JOE           = 17,
	INK_OUTLINED_TEXT = 16
};


enum {
	ZONE_ROOM  = 0,
	ZONE_PANEL = 1
};


//! GameState vars
enum {
	VAR_DRESSING_MODE  =  19, // 0=normal clothes, 1=underwear, 2=dress
	VAR_ROOM73_CUTAWAY =  93,
	VAR_INTRO_PLAYED   = 117
};


enum Language {
	ENGLISH  = 'E',
	FRENCH   = 'F',
	GERMAN   = 'G',
	ITALIAN  = 'I'
};


enum Verb {
	VERB_NONE = 0,

	VERB_PANEL_COMMAND_FIRST = 1,
	VERB_OPEN        = 1,
	VERB_CLOSE       = 2,
	VERB_MOVE        = 3,
	// no verb 4
	VERB_GIVE        = 5,
	VERB_USE         = 6,
	VERB_PICK_UP     = 7,
	VERB_LOOK_AT     = 9,
	VERB_TALK_TO     = 8,
	VERB_PANEL_COMMAND_LAST = 9,

	VERB_WALK_TO     = 10,
	VERB_SCROLL_UP   = 11,
	VERB_SCROLL_DOWN = 12,

	VERB_DIGIT_FIRST = 13,
	VERB_DIGIT_1   = 13,
	VERB_DIGIT_2   = 14,
	VERB_DIGIT_3   = 15,
	VERB_DIGIT_4   = 16,
	VERB_DIGIT_LAST = 16,
	
	VERB_USE_JOURNAL = 20,
	VERB_SKIP_TEXT   = 101,

	VERB_PREP_WITH = 11,
	VERB_PREP_TO   = 12
};


enum StateTalk {
	STATE_TALK_TALK,
	STATE_TALK_MUTE
};


enum StateGrab {
	STATE_GRAB_NONE,
	STATE_GRAB_DOWN,
	STATE_GRAB_UP,
	STATE_GRAB_MID
};


enum StateOn {
	STATE_ON_ON,
	STATE_ON_OFF
};


enum StateUse {
	STATE_USE,
	STATE_USE_ON
};


} // End of namespace Queen

#endif
