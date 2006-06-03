/* ScummVM - Scumm Interpreter
 * Copyright (C) 2005-2006 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

#ifndef __luredefs_h__
#define __luredefs_h__

#include "common/stdafx.h"
#include "common/scummsys.h"
#include "common/list.h"

namespace Lure {

#define LURE_DEBUG 1

#define READ_LE_INT16(x) (int16) READ_LE_UINT16(x)
#define READ_LE_INT32(x) (int32) READ_LE_UINT32(x)

enum {
	GF_FLOPPY	= 1 <<  0,
	GF_LNGUNK	= 1 << 15
};

enum {
	GI_LURE = 0
};

enum Action {
	NONE = 0,
	GET = 1,
	DROP = 0,
	PUSH = 3,
	PULL = 4,
	OPERATE = 5,
	OPEN = 6,
	CLOSE = 7,
	LOCK = 8,
	UNLOCK = 9,
	USE = 10,
	GIVE = 11,
	TALK_TO = 12,
	TELL = 13,
	BUY = 14,
	LOOK = 15,
	LOOK_AT = 16,
	LOOK_THROUGH = 17,
	ASK = 18,
	EAT = 0,
	DRINK = 20,
	STATUS = 21,
	GO_TO = 22,
	RETURN = 23,
	BRIBE = 24,
	EXAMINE = 25,
	NPC_SET_ROOM_AND_OFFSET = 28, 
	NPC_UNKNOWN1 = 29, 
	NPC_EXEC_SCRIPT = 30, 
	NPC_UNKNOWN2 = 31,
	NPC_SET_RAND_DEST = 32, 
	NPC_WALKING_CHECK = 33, 
	NPC_SET_SUPPORT_OFFSET = 34,
	NPC_SUPPORT_OFFSET_COND = 35, 
	NPC_DISPATCH_ACTION = 36, 
	NPC_UNKNOWN3 = 37,
	NPC_UNKNOWN4 = 38, 
	NPC_START_TALKING = 39, 
	NPC_JUMP_ADDRESS = 40
};

// Basic game dimensions
#define FULL_SCREEN_WIDTH 320
#define FULL_SCREEN_HEIGHT 200
#define GAME_COLOURS 256
#define SCREEN_SIZE (FULL_SCREEN_HEIGHT * FULL_SCREEN_WIDTH)

#define SUPPORT_FILENAME "lure.dat"
#define LURE_DAT_MAJOR 1
#define LURE_DAT_MINOR 8

// Some resources include multiple packed palettes of 64 entries each
#define SUB_PALETTE_SIZE 64
// Palette resources have 220 palette entries
#define RES_PALETTE_ENTRIES 220
// Palette colour increment amouns for palette fade in/outs
#define PALETTE_FADE_INC_SIZE 4

// Palette and animation for Skorl catching player
#define SKORL_CATCH_PALETTE_ID 0x4060
#define SKORL_CATCH_ANIM_ID 0x4061

// Specifies the maximum buffer sized allocated for decoding animation data
#define MAX_ANIM_DECODER_BUFFER_SIZE 300000

#define MAX_DESC_SIZE 1024
#define MAX_HOTSPOT_NAME_SIZE 80
#define MAX_ACTION_NAME_SIZE 15

// Menubar constants
#define MENUBAR_Y_SIZE 8

// Cursor definitions
#define CURSOR_WIDTH 16
#define CURSOR_HEIGHT 16
#define CURSOR_SIZE 256
#define CURSOR_RESOURCE_ID 1
#define CURSOR_ARROW 0
#define CURSOR_DISK 1
#define CURSOR_TIME_START 2
#define CURSOR_TIME_END 9
#define CURSOR_CROSS 10
#define CURSOR_CAMERA 15
#define CURSOR_TALK 16
#define CURSOR_MENUBAR 17

// Font details
#define FONT_RESOURCE_ID 4
#define NUM_CHARS_IN_FONT 122
#define FONT_WIDTH 8
#define FONT_HEIGHT 8

// Menu constants
#define MENU_RESOURCE_ID 5
#define MENUBAR_SELECTED_COLOUR 0xf7
#define MENU_UNSELECTED_COLOUR 0xe2
#define MENU_SELECTED_COLOUR 0xe3
#define MENUITEM_NONE 0
#define MENUITEM_CREDITS 1 
#define MENUITEM_RESTART_GAME 2 
#define MENUITEM_SAVE_GAME 3 
#define MENUITEM_RESTORE_GAME 4 
#define MENUITEM_QUIT 5 
#define MENUITEM_TEXT_SPEED 6 
#define MENUITEM_SOUND 7

// Mouse change needed to change an item in a popup menu
#define POPMENU_CHANGE_SENSITIVITY 5

// Dialog related defines
#define DIALOG_EDGE_SIZE 9
#define	DIALOG_TEXT_COLOUR 0xe2
#define DIALOG_WHITE_COLOUR 0xe3
#define INFO_DIALOG_X 69
#define INFO_DIALOG_Y 61
#define INFO_DIALOG_WIDTH 191
#define TALK_DIALOG_WIDTH 128
#define TALK_DIALOG_EDGE_SIZE 3
#define TALK_DIALOG_Y 33

// Strings defines
#define STRINGS_RESOURCE_ID 0x10
#define STRINGS_2_RESOURCE_ID 0x11
#define STRINGS_3_RESOURCE_ID 0x12
#define STRING_ID_RANGE 0x7d0
#define STRING_ID_UPPER 0xfa0

// Custom resources stored in lure.dat
#define GAME_PALETTE_RESOURCE_ID 0x3f01
#define ALT_PALETTE_RESOURCE_ID 0x3f02
#define DIALOG_RESOURCE_ID 0x3f03
#define TALK_DIALOG_RESOURCE_ID 0x3f04
#define ROOM_DATA_RESOURCE_ID 0x3f05
#define NPC_SCHEDULES_RESOURCE_ID 0x3f06
#define HOTSPOT_DATA_RESOURCE_ID 0x3f07
#define HOTSPOT_OVERRIDE_DATA_RESOURCE_ID 0x3f08
#define ROOM_EXITS_RESOURCE_ID 0x3f09
#define ROOM_EXIT_JOINS_RESOURCE_ID 0x3f0a
#define ANIM_DATA_RESOURCE_ID 0x3f0b
#define SCRIPT_DATA_RESOURCE_ID 0x3f0c
#define SCRIPT2_DATA_RESOURCE_ID 0x3f0d
#define HOTSPOT_SCRIPT_LIST_RESOURCE_ID 0x3f0e
#define	MESSAGES_LIST_RESOURCE_ID 0x3f0f
#define ACTION_LIST_RESOURCE_ID 0x3f10
#define TALK_HEADER_RESOURCE_ID 0x3f11
#define TALK_DATA_RESOURCE_ID 0x3f12
#define ROOM_PATHS_RESOURCE_ID 0x3f13
#define EXIT_COORDINATES_RESOURCE_ID 0x3f14
#define EXIT_HOTSPOT_ID_LIST 0x3f15

// Script constants
#define STARTUP_SCRIPT 0x23FC

// Miscellaneous resources
#define CREDITS_RESOURCE_ID 0x7800
#define NAMES_RESOURCE_ID 9
#define NOONE_ID 0x3E7
#define PLAYER_ID 0x3E8
#define RATPOUCH_ID 0x3E9
#define SKORL_ID 0x3EA
#define FIRST_NONCHARACTER_ID 0x408
#define SACK_ID 0x40D
#define TRANSFORM_ID 0x425

#define START_NONVISUAL_HOTSPOT_ID 0x7530

// Milliseconds delay between game frames
#define GAME_FRAME_DELAY 80

// Tick proc constants
#define TALK_TICK_PROC_ID 0x8ABD
#define PLAYER_TICK_PROC_ID 0x5E44
#define VOICE_TICK_PROC_ID 0x625E

// String constants
#define STRANGER_ID 0x17A
#define TALK_MAGIC_ID 0x424
#define TALK_RESPONSE_MAGIC_ID 0x1092

// Misc constants
#define VOICE_ANIM_ID 0x5810

// Countdown for # operations in path finder before breaking until next
// tick - set it to 0 if you'd like all pathfinding to be done at once
//#define PATHFIND_COUNTDOWN 4000
#define PATHFIND_COUNTDOWN 0

// Pixel record flags
#define PIXELFLAG_HAS_TABLE 4

// Constants used to reference entries in the reworked support data entry lists
#define RETURN_SUPPORT_ID 0x100
#define EXIT_BLOCKED_SUPPORT_ID 0x200

} // End of namespace Lure

#endif
