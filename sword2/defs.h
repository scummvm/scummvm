/* Copyright (C) 1994-2003 Revolution Software Ltd
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

#include "bs2/header.h"
#include "bs2/resman.h"

#define	SIZE	0x10000			// 65536 items per section
#define	NuSIZE	0xffff			// & with this

// global variable references
// NB. 4 * <number from linc's Global Variables list>

#define VAR(n)				(*(uint32 *) (res_man._resList[1]->ad + sizeof(_standardHeader) + 4 * (n)))

#define ID				VAR(0)
#define RESULT				VAR(1)
#define PLAYER_ACTION			VAR(2)
// #define CUR_PLAYER_ID		VAR(3)
// always 8 (George object used for Nico player character as well)
#define	CUR_PLAYER_ID			8
#define PLAYER_ID			VAR(305)
#define TALK_FLAG			VAR(13)

#define MOUSE_X				VAR(4)
#define MOUSE_Y				VAR(5)
#define LEFT_BUTTON			VAR(109)
#define RIGHT_BUTTON			VAR(110)
#define CLICKED_ID			VAR(178)

#define IN_SUBJECT			VAR(6)
#define COMBINE_BASE			VAR(7)
#define OBJECT_HELD			VAR(14)

#define SPEECH_ID			VAR(9)
#define INS1				VAR(10)
#define INS2				VAR(11)
#define INS3				VAR(12)
#define INS4				VAR(60)
#define INS5				VAR(61)
#define INS_COMMAND			VAR(59)

#define PLAYER_FEET_X			VAR(141)
#define PLAYER_FEET_Y			VAR(142)
#define PLAYER_CUR_DIR			VAR(937)

// for debug.cpp
#define LOCATION			VAR(62)

// so scripts can force scroll offsets
#define SCROLL_X			VAR(345)
#define SCROLL_Y			VAR(346)

#define EXIT_CLICK_ID			VAR(710)
#define EXIT_FADING			VAR(713)

#define SYSTEM_TESTING_ANIMS		VAR(912)
#define SYSTEM_TESTING_TEXT		VAR(1230)
#define SYSTEM_WANT_PREVIOUS_LINE	VAR(1245)

// 1=on 0=off (set in fnAddHuman and fnNoHuman)
#define MOUSE_AVAILABLE			VAR(686)

// used in fnChoose
#define AUTO_SELECTED			VAR(1115)

// see fnStartConversation and fnChooser
#define CHOOSER_COUNT_FLAG		VAR(15)

// signifies a demo mode
#define DEMO				VAR(1153)

// Indicates to script whether this is the Playstation version.
// #define PSXFLAG			VAR(1173)

// 1 = dead
#define DEAD				VAR(1256)

 // If set indicates that the speech anim is to run through only once.
#define SPEECHANIMFLAG			VAR(1278)

// for the engine
#define SCROLL_OFFSET_X			VAR(1314)

// for the poor PSX so it knows what language is running.
// #define GAME_LANGUAGE		VAR(111)

//resource id's of pouse mointers. It's pretty much safe to do it like this

#define NORMAL_MOUSE_ID			17
#define SCROLL_LEFT_MOUSE_ID		1440
#define SCROLL_RIGHT_MOUSE_ID		1441

// Console Font - does not use game text - only English required
#define CONSOLE_FONT_ID			340		// ConsFont

// Speech Font
#define ENGLISH_SPEECH_FONT_ID		341		// SpchFont
#define FINNISH_SPEECH_FONT_ID		956		// FinSpcFn
#define POLISH_SPEECH_FONT_ID		955		// PolSpcFn

// Control Panel Font (and un-selected savegame descriptions)
#define ENGLISH_CONTROLS_FONT_ID	2005		// Sfont
#define FINNISH_CONTROLS_FONT_ID	959		// FinSavFn
#define POLISH_CONTROLS_FONT_ID		3686		// PolSavFn

// Red Font (for selected savegame descriptions)
#define ENGLISH_RED_FONT_ID		2005		// 1998	// Redfont
#define FINNISH_RED_FONT_ID		959		// 960	// FinRedFn
#define POLISH_RED_FONT_ID		3686		// 3688	// PolRedFn

// Control panel palette resource id

#define CONTROL_PANEL_PALETTE		261

// res id's of the system menu icons
#define OPTIONS_ICON			344
#define QUIT_ICON			335
#define SAVE_ICON			366
#define RESTORE_ICON			364
#define RESTART_ICON			342

// res id of conversation exit icon, 'EXIT' menu icon (used in fnChoose)
#define EXIT_ICON			65

#endif
