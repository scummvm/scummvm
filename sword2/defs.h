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

#include "header.h"
#include "resman.h"

//--------------------------------------------------------------------------------------
#define	SIZE	0x10000	//65536 items per section
#define	NuSIZE	0xffff	//& with this

//--------------------------------------------------------------------------------------
//global variable references

// NB. 4 * <number from linc's Global Variables list>

#define	ID						*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader))
#define	RESULT					*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 1)	// 4 * <number from linc's Global Variables list>
#define	PLAYER_ACTION			*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 2)
//#define	CUR_PLAYER_ID			*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 3)
#define	CUR_PLAYER_ID			8	// always 8 (George object used for Nico player character as well)
#define	PLAYER_ID				*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 305)
#define	TALK_FLAG				*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 13)

#define	MOUSE_X					*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 4)
#define	MOUSE_Y					*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 5)
#define	LEFT_BUTTON				*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 109)
#define	RIGHT_BUTTON			*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 110)
#define	CLICKED_ID				*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 178)

#define	IN_SUBJECT				*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 6)
#define	COMBINE_BASE			*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 7)
#define	OBJECT_HELD				*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 14)

#define	SPEECH_ID				*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 9)
#define	INS1					*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 10)
#define	INS2					*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 11)
#define	INS3					*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 12)
#define	INS4					*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 60)
#define	INS5					*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 61)
#define	INS_COMMAND				*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 59)

#define	PLAYER_FEET_X			*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 141)
#define	PLAYER_FEET_Y			*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 142)
#define	PLAYER_CUR_DIR			*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 937)

#define	LOCATION				*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 62)	// for debug.cpp

#define	SCROLL_X				*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 345)	// so scripts can force scroll offsets
#define	SCROLL_Y				*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 346)	// so scripts can force scroll offsets

#define	EXIT_CLICK_ID			*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 710)
#define	EXIT_FADING				*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 713)

#define	SYSTEM_TESTING_ANIMS	*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 912)
#define	SYSTEM_TESTING_TEXT		*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 1230)
#define	SYSTEM_WANT_PREVIOUS_LINE	*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 1245)

#define	MOUSE_AVAILABLE			*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 686)	// 1=on 0=off (set in FN_add_human & FN_no_human)

#define	AUTO_SELECTED			*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 1115)	// used in FN_choose

#define	CHOOSER_COUNT_FLAG		*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 15)	// see FN_start_conversation & FN_chooser

#define	DEMO					*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 1153)	//signifies a demo mode

#define PSXFLAG					*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 1173) // Indicates to script whether this is the Playstation version.

#define	DEAD					*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 1256)	//=1 =dead
#define SPEECHANIMFLAG			*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 1278) // If set indicates that the speech anim is to run through only once.

#define SCROLL_OFFSET_X			*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 1314) //for the engine

#define GAME_LANGUAGE			*(uint32 *)(res_man.resList[1]->ad+sizeof(_standardHeader)+4* 111)  //for the poor PSX so it knows what language is running.
//--------------------------------------------------------------------------------------
//resource id's of pouse mointers. It's pretty much safe to do it like this

#define	NORMAL_MOUSE_ID				17
#define	SCROLL_LEFT_MOUSE_ID		1440
#define	SCROLL_RIGHT_MOUSE_ID		1441

//--------------------------------------------------------------------------------------
// Console Font - does not use game text - only English required
#define	CONSOLE_FONT_ID				340		// ConsFont

// Speech Font
#define ENGLISH_SPEECH_FONT_ID		341		// SpchFont
#define FINNISH_SPEECH_FONT_ID		956		// FinSpcFn
#define POLISH_SPEECH_FONT_ID		955		// PolSpcFn

// Control Panel Font (and un-selected savegame descriptions)
#define ENGLISH_CONTROLS_FONT_ID	2005	// Sfont
#define FINNISH_CONTROLS_FONT_ID	959		// FinSavFn
#define POLISH_CONTROLS_FONT_ID		3686	// PolSavFn

// Red Font (for selected savegame descriptions)
#define ENGLISH_RED_FONT_ID	2005	//		1998	// Redfont
#define FINNISH_RED_FONT_ID	959	//		960		// FinRedFn
#define POLISH_RED_FONT_ID		3686	//	3688	// PolRedFn

//--------------------------------------------------------------------------------------
// Control panel palette resource id

#define CONTROL_PANEL_PALETTE	261

//--------------------------------------------------------------------------------------
// res id's of the system menu icons
#define	OPTIONS_ICON	344
#define	QUIT_ICON		335
#define	SAVE_ICON		366
#define	RESTORE_ICON	364
#define	RESTART_ICON	342

//--------------------------------------------------------------------------------------
// res id of conversation exit icon

#define	EXIT_ICON	65		// 'EXIT' menu icon (used in FN_choose)

//--------------------------------------------------------------------------------------

#endif
