/* Copyright (C) 1994-2005 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#ifndef	DEFS
#define	DEFS

#define	SIZE	0x10000			// 65536 items per section
#define	NuSIZE	0xffff			// & with this

// always 8 (George object used for Nico player character as well)
#define CUR_PLAYER_ID 8

// global variable references

enum {
	ID				= 0,
	RESULT				= 1,
	PLAYER_ACTION			= 2,
	// CUR_PLAYER_ID		= 3,
	PLAYER_ID			= 305,
	TALK_FLAG			= 13,

	MOUSE_X				= 4,
	MOUSE_Y				= 5,
	LEFT_BUTTON			= 109,
	RIGHT_BUTTON			= 110,
	CLICKED_ID			= 178,

	IN_SUBJECT			= 6,
	COMBINE_BASE			= 7,
	OBJECT_HELD			= 14,

	SPEECH_ID			= 9,
	INS1				= 10,
	INS2				= 11,
	INS3				= 12,
	INS4				= 60,
	INS5				= 61,
	INS_COMMAND			= 59,

	PLAYER_FEET_X			= 141,
	PLAYER_FEET_Y			= 142,
	PLAYER_CUR_DIR			= 937,

	// for debug.cpp
	LOCATION			= 62,

	// so scripts can force scroll offsets
	SCROLL_X			= 345,
	SCROLL_Y			= 346,

	EXIT_CLICK_ID			= 710,
	EXIT_FADING			= 713,

	SYSTEM_TESTING_ANIMS		= 912,
	SYSTEM_TESTING_TEXT		= 1230,
	SYSTEM_WANT_PREVIOUS_LINE	= 1245,

	// 1=on 0=off (set in fnAddHuman and fnNoHuman)
	MOUSE_AVAILABLE			= 686,

	// used in fnChoose
	AUTO_SELECTED			= 1115,

	// see fnStartConversation and fnChooser
	CHOOSER_COUNT_FLAG		= 15,

	// signifies a demo mode
	DEMO				= 1153,

	// Indicates to script whether this is the Playstation version.
	// PSXFLAG			= 1173,

	// for the poor PSX so it knows what language is running.
	// GAME_LANGUAGE		= 111,

	// 1 = dead
	DEAD				= 1256,

	// If set indicates that the speech anim is to run through only once.
	SPEECHANIMFLAG			= 1278,

	// for the engine
	SCROLL_OFFSET_X			= 1314
};

// Resource IDs

enum {
	// mouse mointers - It's pretty much safe to do it like this
	NORMAL_MOUSE_ID			= 17,
	SCROLL_LEFT_MOUSE_ID		= 1440,
	SCROLL_RIGHT_MOUSE_ID		= 1441,

	// Console Font - does not use game text - only English required
	CONSOLE_FONT_ID			= 340,

	// Speech Font
	ENGLISH_SPEECH_FONT_ID		= 341,
	FINNISH_SPEECH_FONT_ID		= 956,
	POLISH_SPEECH_FONT_ID		= 955,

	// Control Panel Font (and un-selected savegame descriptions)
	ENGLISH_CONTROLS_FONT_ID	= 2005,
	FINNISH_CONTROLS_FONT_ID	= 959,
	POLISH_CONTROLS_FONT_ID		= 3686,

	// Red Font (for selected savegame descriptions)
	// BS2 doesn't draw selected savegames in red, so I guess this is a
	// left-over from BS1
	ENGLISH_RED_FONT_ID		= 2005,		// 1998	// Redfont
	FINNISH_RED_FONT_ID		= 959,		// 960	// FinRedFn
	POLISH_RED_FONT_ID		= 3686,		// 3688	// PolRedFn

	// Control panel palette resource id
	CONTROL_PANEL_PALETTE		= 261,

	// res id's of the system menu icons
	OPTIONS_ICON			= 344,
	QUIT_ICON			= 335,
	SAVE_ICON			= 366,
	RESTORE_ICON			= 364,
	RESTART_ICON			= 342,

	// conversation exit icon, 'EXIT' menu icon (used in fnChoose)
	EXIT_ICON			= 65
};

#endif
