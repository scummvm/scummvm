/* ScummVM - Scumm Interpreter
 * Copyright (C) 2001  Ludvig Strigeus
 * Copyright (C) 2001-2006 The ScummVM project
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

#ifndef AGOS_DEBUG_H
#define AGOS_DEBUG_H

namespace AGOS {

static const char *const elvira1_opcodeNameTable[300] = {
	/* 0 */
	"IJ|AT",
	"IJ|NOT_AT",
	"IJ|PRESENT",
	"IJ|NOT_PRESENT",
	/* 4 */
	"IJ|WORN",
	"IJ|NOT_WORN",
	"IJ|CARRIED",
	"IJ|NOT_CARRIED",
	/* 8 */
	"IIJ|IS_AT",
	"IIJ|IS_NOT_AT",
	"IIJ|IS_SIBLING",
	"IIJ|IS_NOT_SIBLING",
	/* 12 */
	"WJ|IS_ZERO",
	"WJ|ISNOT_ZERO",
	"WWJ|IS_EQ",
	"WWJ|IS_NEQ",
	/* 16 */
	"WWJ|IS_LE",
	"WWJ|IS_GE",
	"WWJ|IS_EQF",
	"WWJ|IS_NEQF",
	/* 20 */
	"WWJ|IS_LEF",
	"WWJ|IS_GEF",
	NULL,
	NULL,
	/* 24 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 28 */
	NULL,
	"WJ|CHANCE",
	"IJ|IS_PLAYER",
	NULL,
	/* 32 */
	"IJ|IS_ROOM",
	"IJ|IS_OBJECT",
	"IWJ|ITEM_STATE_IS",
	NULL,
	/* 36 */
	"IWJ|OBJECT_HAS_FLAG",
	"IIJ|CAN_PUT",
	NULL,
	NULL,
	/* 40 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 44 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 48 */
	"I|SET_NO_PARENT",
	NULL,
	NULL,
	"II|SET_PARENT",
	/* 52 */
	NULL,
	NULL,
	"IWV|COPY_OF",
	"WIW|COPY_FO",
	/* 56 */
	"WW|MOVE",
	"W|WHAT_O",
	NULL,
	"IW|WEIGH",
	/* 60 */
	"W|SET_FF",
	"W|ZERO",
	NULL,
	NULL,
	/* 64 */
	"WW|SET",
	"WW|ADD",
	"WW|SUB",
	"WW|ADDF",
	/* 68 */
	"WW|SUBF",
	"WW|MUL",
	"WW|DIV",
	"WW|MULF",
	/* 72 */
	"WW|DIVF",
	"WW|MOD",
	"WW|MODF",
	"WW|RANDOM",
	/* 76 */
	"W|MOVE_DIRN",
	"I|SET_A_PARENT",
	NULL,
	NULL,
	/* 80 */
	"IW|SET_CHILD2_BIT",
	"IW|CLEAR_CHILD2_BIT",
	NULL,
	NULL,
	/* 84 */
	"II|MAKE_SIBLING",
	"I|INC_STATE",
	"I|DEC_STATE",
	"IW|SET_STATE",
	/* 88 */
	NULL,
	"W|SHOW_INT",
	"|SHOW_SCORE",
	"T|SHOW_STRING_NL",
	/* 92 */
	"T|SHOW_STRING",
	NULL,
	NULL,
	NULL,
	/* 96 */
	NULL,
	"x|END",
	"x|DONE",
	NULL,
	/* 100 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 104 */
	NULL,
	"W|START_SUB",
	"IWW|DO_CLASS",
	NULL,
	/* 108 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 112 */
	"IW|PRINT_OBJ",
	NULL,
	"I|PRINT_NAME",
	"I|PRINT_CNAME",
	/* 116 */
	NULL,
	NULL,
	NULL,
	"WW|ADD_TIMEOUT",
	/* 120 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 124 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 128 */
	"J|IS_M1_EMPTY",
	"J|IS_M3_EMPTY",
	NULL,
	NULL,
	/* 132 */
	NULL,
	NULL,
	NULL,
	"ITJ|CHILD_FR2_IS",
	/* 136 */
	"IIJ|IS_ITEM_EQ",
	NULL,
	NULL,
	NULL,
	/* 140 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 144 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 148 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 152 */
	"W|DEBUG",
	NULL,
	NULL,
	NULL,
	/* 156 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 160 */
	NULL,
	NULL,
	"IWJ|IS_CFLAG",
	NULL,
	/* 164 */
	"|RESCAN",
	"wwwW|MEANS",
	NULL,
	NULL,
	/* 168 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 172 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 176 */
	"IWI|SET_USER_ITEM",
	"IWW|GET_USER_ITEM",
	"IW|CLEAR_USER_ITEM",
	NULL,
	/* 180 */
	"IWW|WHERE_TO",
	"IIW|DOOR_EXIT",
	NULL,
	NULL,
	/* 184 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 188 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 192 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 196 */
	NULL,
	NULL,
	"W|COMMENT",
	NULL,
	/* 200 */
	NULL,
	NULL,
	NULL,
	"|NOT",
	/* 204 */
	NULL,
	NULL,
	"IW|GET_PARENT",
	"IW|GET_NEXT",
	/* 208 */
	"IW|GET_CHILDREN",
	NULL,
	NULL,
	NULL,
	/* 212 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 216 */
	NULL,
	NULL,
	NULL,
	"WW|FIND_MASTER",
	/* 220 */
	"IWW|NEXT_MASTER",
	NULL,
	NULL,
	NULL,
	/* 224 */
	"WW|PICTURE",
	"W|LOAD_ZONE",
	"WWWWW|ANIMATE",
	"W|STOP_ANIMATE",
	/* 228 */
	"|KILL_ANIMATE",
	"WWWWWWW|DEFINE_WINDOW",
	"W|CHANGE_WINDOW",
	"|CLS",
	/* 232 */
	"W|CLOSE_WINDOW",
	NULL,
	NULL,
	"WWWWWIW|ADD_BOX",
	/* 236 */
	"W|DEL_BOX",
	"W|ENABLE_BOX",
	"W|DISABLE_BOX",
	"WWW|MOVE_BOX",
	/* 240 */
	NULL,
	NULL,
	"IW|DO_ICONS",
	"IWJ|IS_CLASS",
	/* 244 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 248 */
	NULL,
	"IW|SET_CLASS",
	"IW|UNSET_CLASS",
	"WW|CLEAR_BIT",
	/* 252 */
	"WW|SET_BIT",
	"WWJ|BIT_TEST",
	NULL,
	"W|WAIT_SYNC",
	/* 256 */
	"W|SYNC",
	"WI|DEF_OBJ",
	NULL,
	"|SET_TIME",
	/* 260 */
	"WJ|IF_TIME",
	"IJ|IS_SIBLING_WITH_A",
	"IWW|DO_CLASS_ICONS",
	"WW|PLAY_TUNE",
	/* 264 */
	"W|WAIT_END_TUNE",
	"W|IF_END_TUNE",
	"Www|SET_ADJ_NOUN",
	"WW|ZONE_DISK",
	/* 268 */
	"|SAVE_GAME",
	"|LOAD_GAME",
	"|PRINT_STATS",
	"|STOP_TUNE",
	/* 272 */
	"|PRINT_PLAYER_DAMAGE",
	"|PRINT_MONSTER_DAMAGE",
	"|PAUSE",
	"IW|COPY_SF",
	/* 276 */
	"W|RESTORE_ICONS",
	"|PRINT_PLAYER_HIT",
	"|PRINT_MONSTER_HIT",
	"|FREEZE_ZONES",
	/* 280 */
	"II|SET_PARENT_SPECIAL",
	"|CLEAR_TIMERS",
	"IW|SET_STORE",
	"WJ|IS_BOX",
};

static const char *const waxworks_opcodeNameTable[256] = {
	/* 0 */
	"|NOT",
	"IJ|AT",
	"IJ|NOT_AT",
	NULL,
	/* 4 */
	NULL,
	"IJ|CARRIED",
	"IJ|NOT_CARRIED",
	"IIJ|IS_AT",
	/* 8 */
	"IIJ|IS_NOT_AT",
	"IIJ|IS_SIBLING",
	"IIJ|IS_NOT_SIBLING",
	"VJ|IS_ZERO",
	/* 12 */
	"VJ|ISNOT_ZERO",
	"VWJ|IS_EQ",
	"VWJ|IS_NEQ",
	"VWJ|IS_LE",
	/* 16 */
	"VWJ|IS_GE",
	"VVJ|IS_EQF",
	"VVJ|IS_NEQF",
	"VVJ|IS_LEF",
	/* 20 */
	"VVJ|IS_GEF",
	NULL,
	NULL,
	"WJ|CHANCE",
	/* 24 */
	"IJ|IS_PLAYER",
	"IJ|IS_ROOM",
	"IJ|IS_OBJECT",
	"IWJ|ITEM_STATE_IS",
	/* 28 */
	"IBJ|OBJECT_HAS_FLAG",
	"IIJ|CAN_PUT",
	NULL,
	"I|SET_NO_PARENT",
	/* 32 */
	NULL,
	"II|SET_PARENT",
	"IBV|COPY_OF",
	"VIB|COPY_FO",
	/* 36 */
	"VV|MOVE",
	"W|WHAT_O",
	NULL,
	NULL,
	/* 40 */
	NULL,
	"V|ZERO",
	"VW|SET",
	"VW|ADD",
	/* 44 */
	"VW|SUB",
	"VV|ADDF",
	"VV|SUBF",
	"VW|MUL",
	/* 48 */
	"VW|DIV",
	"VV|MULF",
	"VV|DIVF",
	"VW|MOD",
	/* 52 */
	"VV|MODF",
	"VW|RANDOM",
	"B|MOVE_DIRN",
	"I|SET_A_PARENT",
	/* 56 */
	"IB|SET_CHILD2_BIT",
	"IB|CLEAR_CHILD2_BIT",
	"II|MAKE_SIBLING",
	"I|INC_STATE",
	/* 60 */
	"I|DEC_STATE",
	"IW|SET_STATE",
	"V|SHOW_INT",
	"T|SHOW_STRING_NL",
	/* 64 */
	"T|SHOW_STRING",
	"WWWWWB|ADD_TEXT_BOX",
	"BT|SET_SHORT_TEXT",
	"BT|SET_LONG_TEXT",
	/* 68 */
	"x|END",
	"x|DONE",
	"V|SHOW_STRING_AR3",
	"W|START_SUB",
	/* 72 */
	NULL,
	"I|PRINT_OBJ",
	"I|PRINT_NAME",
	"I|PRINT_CNAME",
	/* 76 */
	"WW|ADD_TIMEOUT",
	"J|IS_M1_EMPTY",
	"J|IS_M3_EMPTY",
	"ITJ|CHILD_FR2_IS",
	/* 80 */
	"IIJ|IS_ITEM_EQ",
	NULL,
	"B|DEBUG",
	"|RESCAN",
	/* 84 */
	NULL,
	"IBB|WHERE_TO",
	NULL,
	"W|COMMENT",
	/* 88 */
	"|STOP_ANIMATION",
	"T|LOAD_USER_GAME",
	"IB|GET_PARENT",
	"IB|GET_NEXT",
	/* 92 */
	"IB|GET_CHILDREN",
	NULL,
	NULL,
	NULL,
	/* 96 */
	"WB|PICTURE",
	"W|LOAD_ZONE",
	"WBWWW|ANIMATE",
	"W|STOP_ANIMATE",
	/* 100 */
	"|KILL_ANIMATE",
	"BWWWWWW|DEFINE_WINDOW",
	"B|CHANGE_WINDOW",
	"|CLS",
	/* 104 */
	"B|CLOSE_WINDOW",
	"B|MENU",
	"BB|TEXT_MENU",
	"WWWWWIW|ADD_BOX",
	/* 108 */
	"W|DEL_BOX",
	"W|ENABLE_BOX",
	"W|DISABLE_BOX",
	"WWW|MOVE_BOX",
	/* 112 */
	NULL,
	NULL,
	"IB|DO_ICONS",
	"IBJ|IS_CLASS",
	/* 116 */
	"IB|SET_CLASS",
	"IB|UNSET_CLASS",
	NULL,
	"W|WAIT_SYNC",
	/* 120 */
	"W|SYNC",
	"BI|DEF_OBJ",
	NULL,
	"|SET_TIME",
	/* 124 */
	NULL,
	"IJ|IS_SIBLING_WITH_A",
	"IBB|DO_CLASS_ICONS",
	"WW|PLAY_TUNE",
	/* 128 */
	"W|WAIT_END_TUNE",
	"W|IF_END_TUNE",
	"Bww|SET_ADJ_NOUN",
	NULL,
	/* 132 */
	"|SAVE_GAME",
	"|LOAD_GAME",
	"|STOP_TUNE",
	"|PAUSE",
	/* 136 */
	"IV|COPY_SF",
	"B|RESTORE_ICONS",
	"|FREEZE_ZONES",
	"II|SET_PARENT_SPECIAL",
	/* 140 */
	"|CLEAR_TIMERS",
	"BI|SET_M1_OR_M3",
	"WJ|IS_BOX",
	"I|START_ITEM_SUB",
	/* 144 */
	"IB|SET_DOOR_OPEN",
	"IB|SET_DOOR_CLOSED",
	"IB|SET_DOOR_LOCKED",
	"IB|SET_DOOR_OPEN",
	/* 148 */
	"IBJ|IF_DOOR_OPEN",
	"IBJ|IF_DOOR_CLOSED",
	"IBJ|IF_DOOR_LOCKED",
	"BI|STORE_ITEM",
	/* 152 */
	"BB|GET_ITEM",
	"B|SET_BIT",
	"B|CLEAR_BIT",
	"BJ|IS_BIT_CLEAR",
	/* 156 */
	"BJ|IS_BIT_SET",
	"IBB|GET_ITEM_PROP",
	"IBW|SET_ITEM_PROP",
	NULL,
	/* 160 */
	"B|SET_INK",
	"|UNK_161",
	"BT|PRINT_STR",
	"W|PLAY_EFFECT",
	/* 164 */
	"|getDollar2",
	"W|SET_SUPER_ROOM",
	"BV|GET_SUPER_ROOM",
	"B|CLEAR_BIT2",
	/* 168 */
	"BJ|IS_BIT2_CLEAR",
	"BJ|IS_BIT2_SET",
	NULL,
	NULL,
	/* 172 */
	NULL,
	NULL,
	"W|UNK_174",
	"|getDollar2",
	/* 176 */
	"IWBB|UNK_176",
	"B|UNK_177",
	"B|UNK_178",
	"IWWJ|IS_ADJ_NOUN",
	/* 180 */
	"B|SET_BIT2",
	"B|CLEAR_BIT2",
	"BJ|IS_BIT2_CLEAR",
	"BJ|IS_BIT2_SET",
	/* 184 */
	"T|BOX_MESSAGE",
	"T|BOX_MSG",
	"B|BOX_LONG_TEXT",
	"|PRINT_BOX",
	/* 188 */
	"I|BOX_POBJ",
	"|LOCK_ZONES",
	"|UNLOCK_ZONES",
};

static const char *const simon1dos_opcodeNameTable[256] = {
	/* 0 */
	"|NOT",
	"IJ|AT",
	"IJ|NOT_AT",
	NULL,
	/* 4 */
	NULL,
	"IJ|CARRIED",
	"IJ|NOT_CARRIED",
	"IIJ|IS_AT",
	/* 8 */
	NULL,
	NULL,
	NULL,
	"VJ|IS_ZERO",
	/* 12 */
	"VJ|ISNOT_ZERO",
	"VWJ|IS_EQ",
	"VWJ|IS_NEQ",
	"VWJ|IS_LE",
	/* 16 */
	"VWJ|IS_GE",
	"VVJ|IS_EQF",
	"VVJ|IS_NEQF",
	"VVJ|IS_LEF",
	/* 20 */
	"VVJ|IS_GEF",
	NULL,
	NULL,
	"WJ|CHANCE",
	/* 24 */
	NULL,
	"IJ|IS_ROOM",
	"IJ|IS_OBJECT",
	"IWJ|ITEM_STATE_IS",
	/* 28 */
	"IBJ|OBJECT_HAS_FLAG",
	NULL,
	NULL,
	"I|SET_NO_PARENT",
	/* 32 */
	NULL,
	"II|SET_PARENT",
	NULL,
	NULL,
	/* 36 */
	"VV|MOVE",
	NULL,
	NULL,
	NULL,
	/* 40 */
	NULL,
	"V|ZERO",
	"VW|SET",
	"VW|ADD",
	/* 44 */
	"VW|SUB",
	"VV|ADDF",
	"VV|SUBF",
	"VW|MUL",
	/* 48 */
	"VW|DIV",
	"VV|MULF",
	"VV|DIVF",
	"VW|MOD",
	/* 52 */
	"VV|MODF",
	"VW|RANDOM",
	NULL,
	"I|SET_A_PARENT",
	/* 56 */
	"IB|SET_CHILD2_BIT",
	"IB|CLEAR_CHILD2_BIT",
	"II|MAKE_SIBLING",
	"I|INC_STATE",
	/* 60 */
	"I|DEC_STATE",
	"IW|SET_STATE",
	"V|SHOW_INT",
	"T|SHOW_STRING_NL",
	/* 64 */
	"T|SHOW_STRING",
	"WWWWWB|ADD_TEXT_BOX",
	"BT|SET_SHORT_TEXT",
	"BT|SET_LONG_TEXT",
	/* 68 */
	"x|END",
	"x|DONE",
	"V|SHOW_STRING_AR3",
	"W|START_SUB",
	/* 72 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 76 */
	"WW|ADD_TIMEOUT",
	"J|IS_M1_EMPTY",
	"J|IS_M3_EMPTY",
	"ITJ|CHILD_FR2_IS",
	/* 80 */
	"IIJ|IS_ITEM_EQ",
	NULL,
	"B|DEBUG",
	"|RESCAN",
	/* 84 */
	NULL,
	NULL,
	NULL,
	"W|COMMENT",
	/* 88 */
	"|STOP_ANIMATION",
	"|RESTART_ANIMATION",
	"IB|GET_PARENT",
	"IB|GET_NEXT",
	/* 92 */
	"IB|GET_CHILDREN",
	NULL,
	NULL,
	NULL,
	/* 96 */
	"WB|PICTURE",
	"W|LOAD_ZONE",
	"WBWWW|ANIMATE",
	"W|STOP_ANIMATE",
	/* 100 */
	"|KILL_ANIMATE",
	"BWWWWWW|DEFINE_WINDOW",
	"B|CHANGE_WINDOW",
	"|CLS",
	/* 104 */
	"B|CLOSE_WINDOW",
	NULL,
	NULL,
	"WWWWWIW|ADD_BOX",
	/* 108 */
	"W|DEL_BOX",
	"W|ENABLE_BOX",
	"W|DISABLE_BOX",
	"WWW|MOVE_BOX",
	/* 112 */
	NULL,
	NULL,
	"IB|DO_ICONS",
	"IBJ|IS_CLASS",
	/* 116 */
	"IB|SET_CLASS",
	"IB|UNSET_CLASS",
	NULL,
	"W|WAIT_SYNC",
	/* 120 */
	"W|SYNC",
	"BI|DEF_OBJ",
	NULL,
	NULL,
	/* 124 */
	NULL,
	"IJ|IS_SIBLING_WITH_A",
	"IBB|DO_CLASS_ICONS",
	"WW|PLAY_TUNE",
	/* 128 */
	"W|WAIT_END_TUNE",
	"W|IF_END_TUNE",
	"Bww|SET_ADJ_NOUN",
	NULL,
	/* 132 */
	"|SAVE_GAME",
	"|LOAD_GAME",
	"|STOP_TUNE",
	"|PAUSE",
	/* 136 */
	"IV|COPY_SF",
	"B|RESTORE_ICONS",
	"|FREEZE_ZONES",
	"II|SET_PARENT_SPECIAL",
	/* 140 */
	"|CLEAR_TIMERS",
	"BI|SET_M1_OR_M3",
	"WJ|IS_BOX",
	"I|START_ITEM_SUB",
	/* 144 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 148 */
	NULL,
	NULL,
	NULL,
	"BI|STORE_ITEM",
	/* 152 */
	"BB|GET_ITEM",
	"B|SET_BIT",
	"B|CLEAR_BIT",
	"BJ|IS_BIT_CLEAR",
	/* 156 */
	"BJ|IS_BIT_SET",
	"IBB|GET_ITEM_PROP",
	"IBW|SET_ITEM_PROP",
	NULL,
	/* 160 */
	"B|SET_INK",
	"BWBW|SETUP_TEXT",
	"BBT|PRINT_STR",
	"W|PLAY_EFFECT",
	/* 164 */
	"|getDollar2",
	"IWWJ|IS_ADJ_NOUN",
	"B|SET_BIT2",
	"B|CLEAR_BIT2",
	/* 168 */
	"BJ|IS_BIT2_CLEAR",
	"BJ|IS_BIT2_SET",
	NULL,
	NULL,
	/* 172 */
	NULL,
	NULL,
	NULL,
	"|LOCK_ZONES",
	/* 176 */
	"|UNLOCK_ZONES",
	"BBI|SCREEN_TEXT_POBJ",
	"WWBB|GETPATHPOSN",
	"BBB|SCREEN_TEXT_LONG_TEXT",
	/* 180 */
	"|MOUSE_ON",
	"|MOUSE_OFF",
	"|LOAD_BEARD",
	"|UNLOAD_BEARD",
	/* 184 */
	"W|UNLOAD_ZONE",
	"W|LOAD_SOUND_FILES",
	"|UNFREEZE_ZONES",
	"|FADE_TO_BLACK",
};

static const char *const simon1talkie_opcodeNameTable[256] = {
	/* 0 */
	"|NOT",
	"IJ|AT",
	"IJ|NOT_AT",
	NULL,
	/* 4 */
	NULL,
	"IJ|CARRIED",
	"IJ|NOT_CARRIED",
	"IIJ|IS_AT",
	/* 8 */
	NULL,
	NULL,
	NULL,
	"VJ|IS_ZERO",
	/* 12 */
	"VJ|ISNOT_ZERO",
	"VWJ|IS_EQ",
	"VWJ|IS_NEQ",
	"VWJ|IS_LE",
	/* 16 */
	"VWJ|IS_GE",
	"VVJ|IS_EQF",
	"VVJ|IS_NEQF",
	"VVJ|IS_LEF",
	/* 20 */
	"VVJ|IS_GEF",
	NULL,
	NULL,
	"WJ|CHANCE",
	/* 24 */
	NULL,
	"IJ|IS_ROOM",
	"IJ|IS_OBJECT",
	"IWJ|ITEM_STATE_IS",
	/* 28 */
	"IBJ|OBJECT_HAS_FLAG",
	NULL,
	NULL,
	"I|SET_NO_PARENT",
	/* 32 */
	NULL,
	"II|SET_PARENT",
	NULL,
	NULL,
	/* 36 */
	"VV|MOVE",
	NULL,
	NULL,
	NULL,
	/* 40 */
	NULL,
	"V|ZERO",
	"VW|SET",
	"VW|ADD",
	/* 44 */
	"VW|SUB",
	"VV|ADDF",
	"VV|SUBF",
	"VW|MUL",
	/* 48 */
	"VW|DIV",
	"VV|MULF",
	"VV|DIVF",
	"VW|MOD",
	/* 52 */
	"VV|MODF",
	"VW|RANDOM",
	NULL,
	"I|SET_A_PARENT",
	/* 56 */
	"IB|SET_CHILD2_BIT",
	"IB|CLEAR_CHILD2_BIT",
	"II|MAKE_SIBLING",
	"I|INC_STATE",
	/* 60 */
	"I|DEC_STATE",
	"IW|SET_STATE",
	"V|SHOW_INT",
	"T|SHOW_STRING_NL",
	/* 64 */
	"T|SHOW_STRING",
	"WWWWWB|ADD_TEXT_BOX",
	"BT|SET_SHORT_TEXT",
	"BTw|SET_LONG_TEXT",
	/* 68 */
	"x|END",
	"x|DONE",
	"V|SHOW_STRING_AR3",
	"W|START_SUB",
	/* 72 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 76 */
	"WW|ADD_TIMEOUT",
	"J|IS_M1_EMPTY",
	"J|IS_M3_EMPTY",
	"ITJ|CHILD_FR2_IS",
	/* 80 */
	"IIJ|IS_ITEM_EQ",
	NULL,
	"B|DEBUG",
	"|RESCAN",
	/* 84 */
	NULL,
	NULL,
	NULL,
	"W|COMMENT",
	/* 88 */
	"|STOP_ANIMATION",
	"|RESTART_ANIMATION",
	"IB|GET_PARENT",
	"IB|GET_NEXT",
	/* 92 */
	"IB|GET_CHILDREN",
	NULL,
	NULL,
	NULL,
	/* 96 */
	"WB|PICTURE",
	"W|LOAD_ZONE",
	"WBWWW|ANIMATE",
	"W|STOP_ANIMATE",
	/* 100 */
	"|KILL_ANIMATE",
	"BWWWWWW|DEFINE_WINDOW",
	"B|CHANGE_WINDOW",
	"|CLS",
	/* 104 */
	"B|CLOSE_WINDOW",
	NULL,
	NULL,
	"WWWWWIW|ADD_BOX",
	/* 108 */
	"W|DEL_BOX",
	"W|ENABLE_BOX",
	"W|DISABLE_BOX",
	"WWW|MOVE_BOX",
	/* 112 */
	NULL,
	NULL,
	"IB|DO_ICONS",
	"IBJ|IS_CLASS",
	/* 116 */
	"IB|SET_CLASS",
	"IB|UNSET_CLASS",
	NULL,
	"W|WAIT_SYNC",
	/* 120 */
	"W|SYNC",
	"BI|DEF_OBJ",
	NULL,
	NULL,
	/* 124 */
	NULL,
	"IJ|IS_SIBLING_WITH_A",
	"IBB|DO_CLASS_ICONS",
	"WW|PLAY_TUNE",
	/* 128 */
	"W|WAIT_END_TUNE",
	"W|IF_END_TUNE",
	"Bww|SET_ADJ_NOUN",
	NULL,
	/* 132 */
	"|SAVE_GAME",
	"|LOAD_GAME",
	"|STOP_TUNE",
	"|PAUSE",
	/* 136 */
	"IV|COPY_SF",
	"B|RESTORE_ICONS",
	"|FREEZE_ZONES",
	"II|SET_PARENT_SPECIAL",
	/* 140 */
	"|CLEAR_TIMERS",
	"BI|SET_M1_OR_M3",
	"WJ|IS_BOX",
	"I|START_ITEM_SUB",
	/* 144 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 148 */
	NULL,
	NULL,
	NULL,
	"BI|STORE_ITEM",
	/* 152 */
	"BB|GET_ITEM",
	"B|SET_BIT",
	"B|CLEAR_BIT",
	"BJ|IS_BIT_CLEAR",
	/* 156 */
	"BJ|IS_BIT_SET",
	"IBB|GET_ITEM_PROP",
	"IBW|SET_ITEM_PROP",
	NULL,
	/* 160 */
	"B|SET_INK",
	"BWBW|SETUP_TEXT",
	"BBTW|PRINT_STR",
	"W|PLAY_EFFECT",
	/* 164 */
	"|getDollar2",
	"IWWJ|IS_ADJ_NOUN",
	"B|SET_BIT2",
	"B|CLEAR_BIT2",
	/* 168 */
	"BJ|IS_BIT2_CLEAR",
	"BJ|IS_BIT2_SET",
	NULL,
	NULL,
	/* 172 */
	NULL,
	NULL,
	NULL,
	"|LOCK_ZONES",
	/* 176 */
	"|UNLOCK_ZONES",
	"BBI|SCREEN_TEXT_POBJ",
	"WWBB|GETPATHPOSN",
	"BBB|SCREEN_TEXT_LONG_TEXT",
	/* 180 */
	"|MOUSE_ON",
	"|MOUSE_OFF",
	"|LOAD_BEARD",
	"|UNLOAD_BEARD",
	/* 184 */
	"W|UNLOAD_ZONE",
	"W|LOAD_SOUND_FILES",
	"|UNFREEZE_ZONES",
	"|FADE_TO_BLACK",
};

static const char *const simon2dos_opcodeNameTable[256] = {
	/* 0 */
	"|NOT",
	"IJ|AT",
	"IJ|NOT_AT",
	NULL,
	/* 4 */
	NULL,
	"IJ|CARRIED",
	"IJ|NOT_CARRIED",
	"IIJ|IS_AT",
	/* 8 */
	NULL,
	NULL,
	NULL,
	"VJ|IS_ZERO",
	/* 12 */
	"VJ|ISNOT_ZERO",
	"VWJ|IS_EQ",
	"VWJ|IS_NEQ",
	"VWJ|IS_LE",
	/* 16 */
	"VWJ|IS_GE",
	"VVJ|IS_EQF",
	"VVJ|IS_NEQF",
	"VVJ|IS_LEF",
	/* 20 */
	"VVJ|IS_GEF",
	NULL,
	NULL,
	"WJ|CHANCE",
	/* 24 */
	NULL,
	"IJ|IS_ROOM",
	"IJ|IS_OBJECT",
	"IWJ|ITEM_STATE_IS",
	/* 28 */
	"IBJ|OBJECT_HAS_FLAG",
	NULL,
	NULL,
	"I|SET_NO_PARENT",
	/* 32 */
	NULL,
	"II|SET_PARENT",
	NULL,
	NULL,
	/* 36 */
	"VV|MOVE",
	NULL,
	NULL,
	NULL,
	/* 40 */
	NULL,
	"V|ZERO",
	"VW|SET",
	"VW|ADD",
	/* 44 */
	"VW|SUB",
	"VV|ADDF",
	"VV|SUBF",
	"VW|MUL",
	/* 48 */
	"VW|DIV",
	"VV|MULF",
	"VV|DIVF",
	"VW|MOD",
	/* 52 */
	"VV|MODF",
	"VW|RANDOM",
	NULL,
	"I|SET_A_PARENT",
	/* 56 */
	"IB|SET_CHILD2_BIT",
	"IB|CLEAR_CHILD2_BIT",
	"II|MAKE_SIBLING",
	"I|INC_STATE",
	/* 60 */
	"I|DEC_STATE",
	"IW|SET_STATE",
	"V|SHOW_INT",
	"T|SHOW_STRING_NL",
	/* 64 */
	"T|SHOW_STRING",
	"WWWWWB|ADD_TEXT_BOX",
	"BT|SET_SHORT_TEXT",
	"BT|SET_LONG_TEXT",
	/* 68 */
	"x|END",
	"x|DONE",
	"V|SHOW_STRING_AR3",
	"W|START_SUB",
	/* 72 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 76 */
	"WW|ADD_TIMEOUT",
	"J|IS_M1_EMPTY",
	"J|IS_M3_EMPTY",
	"ITJ|CHILD_FR2_IS",
	/* 80 */
	"IIJ|IS_ITEM_EQ",
	NULL,
	"B|DEBUG",
	"|RESCAN",
	/* 84 */
	NULL,
	NULL,
	NULL,
	"W|COMMENT",
	/* 88 */
	"|STOP_ANIMATION",
	"|RESTART_ANIMATION",
	"IB|GET_PARENT",
	"IB|GET_NEXT",
	/* 92 */
	"IB|GET_CHILDREN",
	NULL,
	NULL,
	NULL,
	/* 96 */
	"WB|PICTURE",
	"W|LOAD_ZONE",
	"WWBWWW|ANIMATE",
	"WW|STOP_ANIMATE",
	/* 100 */
	"|KILL_ANIMATE",
	"BWWWWWW|DEFINE_WINDOW",
	"B|CHANGE_WINDOW",
	"|CLS",
	/* 104 */
	"B|CLOSE_WINDOW",
	NULL,
	NULL,
	"WWWWWIW|ADD_BOX",
	/* 108 */
	"W|DEL_BOX",
	"W|ENABLE_BOX",
	"W|DISABLE_BOX",
	"WWW|MOVE_BOX",
	/* 112 */
	NULL,
	NULL,
	"IB|DO_ICONS",
	"IBJ|IS_CLASS",
	/* 116 */
	"IB|SET_CLASS",
	"IB|UNSET_CLASS",
	NULL,
	"W|WAIT_SYNC",
	/* 120 */
	"W|SYNC",
	"BI|DEF_OBJ",
	NULL,
	NULL,
	/* 124 */
	NULL,
	"IJ|IS_SIBLING_WITH_A",
	"IBB|DO_CLASS_ICONS",
	"WWB|PLAY_TUNE",
	/* 128 */
	"W|WAIT_END_TUNE",
	"W|IF_END_TUNE",
	"Bww|SET_ADJ_NOUN",
	NULL,
	/* 132 */
	"|SAVE_GAME",
	"|LOAD_GAME",
	"|STOP_TUNE",
	"|PAUSE",
	/* 136 */
	"IV|COPY_SF",
	"B|RESTORE_ICONS",
	"|FREEZE_ZONES",
	"II|SET_PARENT_SPECIAL",
	/* 140 */
	"|CLEAR_TIMERS",
	"BI|SET_M1_OR_M3",
	"WJ|IS_BOX",
	"I|START_ITEM_SUB",
	/* 144 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 148 */
	NULL,
	NULL,
	NULL,
	"BI|STORE_ITEM",
	/* 152 */
	"BB|GET_ITEM",
	"B|SET_BIT",
	"B|CLEAR_BIT",
	"BJ|IS_BIT_CLEAR",
	/* 156 */
	"BJ|IS_BIT_SET",
	"IBB|GET_ITEM_PROP",
	"IBW|SET_ITEM_PROP",
	NULL,
	/* 160 */
	"B|SET_INK",
	"BWBW|SETUP_TEXT",
	"BBT|PRINT_STR",
	"W|PLAY_EFFECT",
	/* 164 */
	"|getDollar2",
	"IWWJ|IS_ADJ_NOUN",
	"B|SET_BIT2",
	"B|CLEAR_BIT2",
	/* 168 */
	"BJ|IS_BIT2_CLEAR",
	"BJ|IS_BIT2_SET",
	NULL,
	NULL,
	/* 172 */
	NULL,
	NULL,
	NULL,
	"|LOCK_ZONES",
	/* 176 */
	"|UNLOCK_ZONES",
	"BBI|SCREEN_TEXT_POBJ",
	"WWBB|GETPATHPOSN",
	"BBB|SCREEN_TEXT_LONG_TEXT",
	/* 180 */
	"|MOUSE_ON",
	"|MOUSE_OFF",
	NULL,
	NULL,
	/* 184 */
	"W|UNLOAD_ZONE",
	NULL,
	"|UNFREEZE_ZONES",
	NULL,
	/* 188 */
	"BSJ|STRING2_IS",
	"|CLEAR_MARKS",
	"B|WAIT_FOR_MARK",
};

static const char *const simon2talkie_opcodeNameTable[256] = {
	/* 0 */
	"|NOT",
	"IJ|AT",
	"IJ|NOT_AT",
	NULL,
	/* 4 */
	NULL,
	"IJ|CARRIED",
	"IJ|NOT_CARRIED",
	"IIJ|IS_AT",
	/* 8 */
	NULL,
	NULL,
	NULL,
	"VJ|IS_ZERO",
	/* 12 */
	"VJ|ISNOT_ZERO",
	"VWJ|IS_EQ",
	"VWJ|IS_NEQ",
	"VWJ|IS_LE",
	/* 16 */
	"VWJ|IS_GE",
	"VVJ|IS_EQF",
	"VVJ|IS_NEQF",
	"VVJ|IS_LEF",
	/* 20 */
	"VVJ|IS_GEF",
	NULL,
	NULL,
	"WJ|CHANCE",
	/* 24 */
	NULL,
	"IJ|IS_ROOM",
	"IJ|IS_OBJECT",
	"IWJ|ITEM_STATE_IS",
	/* 28 */
	"IBJ|OBJECT_HAS_FLAG",
	NULL,
	NULL,
	"I|SET_NO_PARENT",
	/* 32 */
	NULL,
	"II|SET_PARENT",
	NULL,
	NULL,
	/* 36 */
	"VV|MOVE",
	NULL,
	NULL,
	NULL,
	/* 40 */
	NULL,
	"V|ZERO",
	"VW|SET",
	"VW|ADD",
	/* 44 */
	"VW|SUB",
	"VV|ADDF",
	"VV|SUBF",
	"VW|MUL",
	/* 48 */
	"VW|DIV",
	"VV|MULF",
	"VV|DIVF",
	"VW|MOD",
	/* 52 */
	"VV|MODF",
	"VW|RANDOM",
	NULL,
	"I|SET_A_PARENT",
	/* 56 */
	"IB|SET_CHILD2_BIT",
	"IB|CLEAR_CHILD2_BIT",
	"II|MAKE_SIBLING",
	"I|INC_STATE",
	/* 60 */
	"I|DEC_STATE",
	"IW|SET_STATE",
	"V|SHOW_INT",
	"T|SHOW_STRING_NL",
	/* 64 */
	"T|SHOW_STRING",
	"WWWWWB|ADD_TEXT_BOX",
	"BT|SET_SHORT_TEXT",
	"BTw|SET_LONG_TEXT",
	/* 68 */
	"x|END",
	"x|DONE",
	"V|SHOW_STRING_AR3",
	"W|START_SUB",
	/* 72 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 76 */
	"WW|ADD_TIMEOUT",
	"J|IS_M1_EMPTY",
	"J|IS_M3_EMPTY",
	"ITJ|CHILD_FR2_IS",
	/* 80 */
	"IIJ|IS_ITEM_EQ",
	NULL,
	"B|DEBUG",
	"|RESCAN",
	/* 84 */
	NULL,
	NULL,
	NULL,
	"W|COMMENT",
	/* 88 */
	"|STOP_ANIMATION",
	"|RESTART_ANIMATION",
	"IB|GET_PARENT",
	"IB|GET_NEXT",
	/* 92 */
	"IB|GET_CHILDREN",
	NULL,
	NULL,
	NULL,
	/* 96 */
	"WB|PICTURE",
	"W|LOAD_ZONE",
	"WWBWWW|ANIMATE",
	"WW|STOP_ANIMATE",
	/* 100 */
	"|KILL_ANIMATE",
	"BWWWWWW|DEFINE_WINDOW",
	"B|CHANGE_WINDOW",
	"|CLS",
	/* 104 */
	"B|CLOSE_WINDOW",
	NULL,
	NULL,
	"WWWWWIW|ADD_BOX",
	/* 108 */
	"W|DEL_BOX",
	"W|ENABLE_BOX",
	"W|DISABLE_BOX",
	"WWW|MOVE_BOX",
	/* 112 */
	NULL,
	NULL,
	"IB|DO_ICONS",
	"IBJ|IS_CLASS",
	/* 116 */
	"IB|SET_CLASS",
	"IB|UNSET_CLASS",
	NULL,
	"W|WAIT_SYNC",
	/* 120 */
	"W|SYNC",
	"BI|DEF_OBJ",
	NULL,
	NULL,
	/* 124 */
	NULL,
	"IJ|IS_SIBLING_WITH_A",
	"IBB|DO_CLASS_ICONS",
	"WWB|PLAY_TUNE",
	/* 128 */
	"W|WAIT_END_TUNE",
	"W|IF_END_TUNE",
	"Bww|SET_ADJ_NOUN",
	NULL,
	/* 132 */
	"|SAVE_GAME",
	"|LOAD_GAME",
	"|STOP_TUNE",
	"|PAUSE",
	/* 136 */
	"IV|COPY_SF",
	"B|RESTORE_ICONS",
	"|FREEZE_ZONES",
	"II|SET_PARENT_SPECIAL",
	/* 140 */
	"|CLEAR_TIMERS",
	"BI|SET_M1_OR_M3",
	"WJ|IS_BOX",
	"I|START_ITEM_SUB",
	/* 144 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 148 */
	NULL,
	NULL,
	NULL,
	"BI|STORE_ITEM",
	/* 152 */
	"BB|GET_ITEM",
	"B|SET_BIT",
	"B|CLEAR_BIT",
	"BJ|IS_BIT_CLEAR",
	/* 156 */
	"BJ|IS_BIT_SET",
	"IBB|GET_ITEM_PROP",
	"IBW|SET_ITEM_PROP",
	NULL,
	/* 160 */
	"B|SET_INK",
	"BWBW|SETUP_TEXT",
	"BBTW|PRINT_STR",
	"W|PLAY_EFFECT",
	/* 164 */
	"|getDollar2",
	"IWWJ|IS_ADJ_NOUN",
	"B|SET_BIT2",
	"B|CLEAR_BIT2",
	/* 168 */
	"BJ|IS_BIT2_CLEAR",
	"BJ|IS_BIT2_SET",
	NULL,
	NULL,
	/* 172 */
	NULL,
	NULL,
	NULL,
	"|LOCK_ZONES",
	/* 176 */
	"|UNLOCK_ZONES",
	"BBI|SCREEN_TEXT_POBJ",
	"WWBB|GETPATHPOSN",
	"BBB|SCREEN_TEXT_LONG_TEXT",
	/* 180 */
	"|MOUSE_ON",
	"|MOUSE_OFF",
	NULL,
	NULL,
	/* 184 */
	"W|UNLOAD_ZONE",
	NULL,
	"|UNFREEZE_ZONES",
	NULL,
	/* 188 */
	"BSJ|STRING2_IS",
	"|CLEAR_MARKS",
	"B|WAIT_FOR_MARK",
};

static const char *const feeblefiles_opcodeNameTable[256] = {
	/* 0 */
	"|NOT",
	"IJ|AT",
	"IJ|NOT_AT",
	NULL,
	/* 4 */
	NULL,
	"IJ|CARRIED",
	"IJ|NOT_CARRIED",
	"IIJ|IS_AT",
	/* 8 */
	NULL,
	NULL,
	NULL,
	"VJ|IS_ZERO",
	/* 12 */
	"VJ|ISNOT_ZERO",
	"VWJ|IS_EQ",
	"VWJ|IS_NEQ",
	"VWJ|IS_LE",
	/* 16 */
	"VWJ|IS_GE",
	"VVJ|IS_EQF",
	"VVJ|IS_NEQF",
	"VVJ|IS_LEF",
	/* 20 */
	"VVJ|IS_GEF",
	NULL,
	NULL,
	"WJ|CHANCE",
	/* 24 */
	NULL,
	"IJ|IS_ROOM",
	"IJ|IS_OBJECT",
	"IWJ|ITEM_STATE_IS",
	/* 28 */
	"IBJ|OBJECT_HAS_FLAG",
	NULL,
	NULL,
	"I|SET_NO_PARENT",
	/* 32 */
	NULL,
	"II|SET_PARENT",
	NULL,
	NULL,
	/* 36 */
	"VV|MOVE",
	"|JUMP_OUT",
	NULL,
	NULL,
	/* 40 */
	NULL,
	"V|ZERO",
	"VW|SET",
	"VW|ADD",
	/* 44 */
	"VW|SUB",
	"VV|ADDF",
	"VV|SUBF",
	"VW|MUL",
	/* 48 */
	"VW|DIV",
	"VV|MULF",
	"VV|DIVF",
	"VW|MOD",
	/* 52 */
	"VV|MODF",
	"VW|RANDOM",
	NULL,
	"I|SET_A_PARENT",
	/* 56 */
	"IB|SET_CHILD2_BIT",
	"IB|CLEAR_CHILD2_BIT",
	"II|MAKE_SIBLING",
	"I|INC_STATE",
	/* 60 */
	"I|DEC_STATE",
	"IW|SET_STATE",
	"V|SHOW_INT",
	"T|SHOW_STRING_NL",
	/* 64 */
	"T|SHOW_STRING",
	"WWWWWB|ADD_TEXT_BOX",
	"BT|SET_SHORT_TEXT",
	"BTw|SET_LONG_TEXT",
	/* 68 */
	"x|END",
	"x|DONE",
	"V|SHOW_STRING_AR3",
	"W|START_SUB",
	/* 72 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 76 */
	"WW|ADD_TIMEOUT",
	"J|IS_M1_EMPTY",
	"J|IS_M3_EMPTY",
	"ITJ|CHILD_FR2_IS",
	/* 80 */
	"IIJ|IS_ITEM_EQ",
	NULL,
	"B|DEBUG",
	"|RESCAN",
	/* 84 */
	NULL,
	NULL,
	NULL,
	"W|COMMENT",
	/* 88 */
	"|STOP_ANIMATION",
	"|RESTART_ANIMATION",
	"IB|GET_PARENT",
	"IB|GET_NEXT",
	/* 92 */
	"IB|GET_CHILDREN",
	NULL,
	NULL,
	NULL,
	/* 96 */
	"WB|PICTURE",
	"W|LOAD_ZONE",
	"WWBWWW|ANIMATE",
	"WW|STOP_ANIMATE",
	/* 100 */
	"|KILL_ANIMATE",
	"BWWWWWW|DEFINE_WINDOW",
	"B|CHANGE_WINDOW",
	"|CLS",
	/* 104 */
	"B|CLOSE_WINDOW",
	NULL,
	NULL,
	"WWWWWIW|ADD_BOX",
	/* 108 */
	"W|DEL_BOX",
	"W|ENABLE_BOX",
	"W|DISABLE_BOX",
	"WWW|MOVE_BOX",
	/* 112 */
	NULL,
	NULL,
	"IB|DO_ICONS",
	"IBJ|IS_CLASS",
	/* 116 */
	"IB|SET_CLASS",
	"IB|UNSET_CLASS",
	NULL,
	"W|WAIT_SYNC",
	/* 120 */
	"W|SYNC",
	"BI|DEF_OBJ",
	"|ORACLE_TEXT_DOWN",
	"|ORACLE_TEXT_UP",
	/* 124 */
	"WJ|IF_TIME",
	"IJ|IS_SIBLING_WITH_A",
	"IBB|DO_CLASS_ICONS",
	"WWB|PLAY_TUNE",
	/* 128 */
	"W|WAIT_END_TUNE",
	"W|IF_END_TUNE",
	"Bww|SET_ADJ_NOUN",
	"|SET_TIME",
	/* 132 */
	"|SAVE_GAME",
	"|LOAD_GAME",
	"|LIST_SAVED_GAMES",
	"|SWITCH_CD",
	/* 136 */
	"IV|COPY_SF",
	"B|RESTORE_ICONS",
	"|FREEZE_ZONES",
	"II|SET_PARENT_SPECIAL",
	/* 140 */
	"|CLEAR_TIMERS",
	"BI|SET_M1_OR_M3",
	"WJ|IS_BOX",
	"I|START_ITEM_SUB",
	/* 144 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 148 */
	NULL,
	NULL,
	NULL,
	"BI|STORE_ITEM",
	/* 152 */
	"BB|GET_ITEM",
	"B|SET_BIT",
	"B|CLEAR_BIT",
	"BJ|IS_BIT_CLEAR",
	/* 156 */
	"BJ|IS_BIT_SET",
	"IBB|GET_ITEM_PROP",
	"IBW|SET_ITEM_PROP",
	NULL,
	/* 160 */
	"B|SET_INK",
	"BWWW|SETUP_TEXT",
	"BBTW|PRINT_STR",
	"W|PLAY_EFFECT",
	/* 164 */
	"|getDollar2",
	"IWWJ|IS_ADJ_NOUN",
	"B|SET_BIT2",
	"B|CLEAR_BIT2",
	/* 168 */
	"BJ|IS_BIT2_CLEAR",
	"BJ|IS_BIT2_SET",
	NULL,
	"W|HYPERLINK_ON",
	/* 172 */
	"|HYPERLINK_OFF",
	"|CHECK_PATHS",
	NULL,
	"|LOCK_ZONES",
	/* 176 */
	"|UNLOCK_ZONES",
	"BBI|SCREEN_TEXT_POBJ",
	"WWBB|GETPATHPOSN",
	"BBB|SCREEN_TEXT_LONG_TEXT",
	/* 180 */
	"|MOUSE_ON",
	"|MOUSE_OFF",
	"T|LOAD_VIDEO",
	"|PLAY_VIDEO",
	/* 184 */
	"W|UNLOAD_ZONE",
	NULL,
	"|UNFREEZE_ZONES",
	"|CENTRE_SCROLL",
	/* 188 */
	"BSJ|STRING2_IS",
	"|CLEAR_MARKS",
	"B|WAIT_FOR_MARK",
	"|RESET_PV_COUNT",
	/* 192 */
	"BBBB|SET_PATH_VALUES",
	"|STOP_CLOCK",
	"|RESTART_CLOCK",
	"BBBB|SET_COLOR",
	/* 196 */
	"B|B3_SET",
	"B|B3_CLEAR",
	"B|B3_ZERO",
	"B|B3_NOT_ZERO",
};

static const char *const puzzlepack_opcodeNameTable[256] = {
	/* 0 */
	"|NOT",
	"IJ|AT",
	"IJ|NOT_AT",
	NULL,
	/* 4 */
	NULL,
	"IJ|CARRIED",
	"IJ|NOT_CARRIED",
	"IIJ|IS_AT",
	/* 8 */
	NULL,
	NULL,
	NULL,
	"WJ|IS_ZERO",
	/* 12 */
	"WJ|ISNOT_ZERO",
	"WWJ|IS_EQ",
	"WWJ|IS_NEQ",
	"WWJ|IS_LE",
	/* 16 */
	"WWJ|IS_GE",
	"WWJ|IS_EQF",
	"WWJ|IS_NEQF",
	"WWJ|IS_LEF",
	/* 20 */
	"WWJ|IS_GEF",
	NULL,
	NULL,
	"WJ|CHANCE",
	/* 24 */
	NULL,
	"IJ|IS_ROOM",
	"IJ|IS_OBJECT",
	"IWJ|ITEM_STATE_IS",
	/* 28 */
	"IBJ|OBJECT_HAS_FLAG",
	NULL,
	"I|MINIMIZE_WINDOW",
	"I|SET_NO_PARENT",
	/* 32 */
	"I|RESTORE_OOOPS_POSITION",
	"II|SET_PARENT",
	NULL,
	NULL,
	/* 36 */
	"WW|MOVE",
	"BI|CHECK_TITLES",
	"B|LOAD_MOUSE_IMAGE",
	NULL,
	/* 40 */
	NULL,
	"W|ZERO",
	"WW|SET",
	"WW|ADD",
	/* 44 */
	"WW|SUB",
	"WW|ADDF",
	"WW|SUBF",
	"WW|MUL",
	/* 48 */
	"WW|DIV",
	"WW|MULF",
	"WW|DIVF",
	"WW|MOD",
	/* 52 */
	"WW|MODF",
	"WW|RANDOM",
	NULL,
	"I|SET_A_PARENT",
	/* 56 */
	"IB|SET_CHILD2_BIT",
	"IB|CLEAR_CHILD2_BIT",
	"II|MAKE_SIBLING",
	"I|INC_STATE",
	/* 60 */
	"I|DEC_STATE",
	"IW|SET_STATE",
	"W|SHOW_INT",
	"T|SHOW_STRING_NL",
	/* 64 */
	"T|SHOW_STRING",
	"WWWWWB|ADD_TEXT_BOX",
	"BTWW|SET_SHORT_TEXT",
	"BTw|SET_LONG_TEXT",
	/* 68 */
	"x|END",
	"x|DONE",
	"V|SHOW_STRING_AR3",
	"W|START_SUB",
	/* 72 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 76 */
	"WW|ADD_TIMEOUT",
	"J|IS_M1_EMPTY",
	"J|IS_M3_EMPTY",
	"ITJ|CHILD_FR2_IS",
	/* 80 */
	"IIJ|IS_ITEM_EQ",
	NULL,
	"B|DEBUG",
	"|RESCAN",
	/* 84 */
	NULL,
	NULL,
	NULL,
	"W|COMMENT",
	/* 88 */
	"|STOP_ANIMATION",
	"|RESTART_ANIMATION",
	"IB|GET_PARENT",
	"IB|GET_NEXT",
	/* 92 */
	"IB|GET_CHILDREN",
	NULL,
	NULL,
	NULL,
	/* 96 */
	"WB|PICTURE",
	"W|LOAD_ZONE",
	"WWBWWW|ANIMATE",
	"WW|STOP_ANIMATE",
	/* 100 */
	"|KILL_ANIMATE",
	"BWWWWWW|DEFINE_WINDOW",
	"B|CHANGE_WINDOW",
	"|CLS",
	/* 104 */
	"B|CLOSE_WINDOW",
	"B|LOAD_HIGH_SCORES",
	"BB|CHECK_HIGH_SCORES",
	"WWWWWIW|ADD_BOX",
	/* 108 */
	"W|DEL_BOX",
	"W|ENABLE_BOX",
	"W|DISABLE_BOX",
	"WWW|MOVE_BOX",
	/* 112 */
	NULL,
	NULL,
	"IB|DO_ICONS",
	"IBJ|IS_CLASS",
	/* 116 */
	"IB|SET_CLASS",
	"IB|UNSET_CLASS",
	NULL,
	"W|WAIT_SYNC",
	/* 120 */
	"W|SYNC",
	"BI|DEF_OBJ",
	"|ORACLE_TEXT_DOWN",
	"|ORACLE_TEXT_UP",
	/* 124 */
	"WJ|IF_TIME",
	"IJ|IS_SIBLING_WITH_A",
	"IBB|DO_CLASS_ICONS",
	"WWB|PLAY_TUNE",
	/* 128 */
	"W|WAIT_END_TUNE",
	"W|IF_END_TUNE",
	"Bww|SET_ADJ_NOUN",
	"|SET_TIME",
	/* 132 */
	"|SAVE_GAME",
	"|LOAD_GAME",
	"|LIST_SAVED_GAMES",
	"|SWITCH_CD",
	/* 136 */
	"IV|COPY_SF",
	"B|RESTORE_ICONS",
	"|FREEZE_ZONES",
	"II|SET_PARENT_SPECIAL",
	/* 140 */
	"|CLEAR_TIMERS",
	"BI|SET_M1_OR_M3",
	"WJ|IS_BOX",
	"I|START_ITEM_SUB",
	/* 144 */
	NULL,
	NULL,
	NULL,
	NULL,
	/* 148 */
	NULL,
	NULL,
	NULL,
	"BI|STORE_ITEM",
	/* 152 */
	"BB|GET_ITEM",
	"W|SET_BIT",
	"W|CLEAR_BIT",
	"WJ|IS_BIT_CLEAR",
	/* 156 */
	"WJ|IS_BIT_SET",
	"IBB|GET_ITEM_PROP",
	"IBW|SET_ITEM_PROP",
	NULL,
	/* 160 */
	"B|SET_INK",
	"BWWW|SETUP_TEXT",
	"BBTW|PRINT_STR",
	"W|PLAY_EFFECT",
	/* 164 */
	"|getDollar2",
	"IWWJ|IS_ADJ_NOUN",
	"B|SET_BIT2",
	"B|CLEAR_BIT2",
	/* 168 */
	"BJ|IS_BIT2_CLEAR",
	"BJ|IS_BIT2_SET",
	NULL,
	"W|HYPERLINK_ON",
	/* 172 */
	"|HYPERLINK_OFF",
	"|SAVE_OOPS_POSITION",
	NULL,
	"|LOCK_ZONES",
	/* 176 */
	"|UNLOCK_ZONES",
	"BBI|SCREEN_TEXT_POBJ",
	"WWBB|GETPATHPOSN",
	"BBB|SCREEN_TEXT_LONG_TEXT",
	/* 180 */
	"|MOUSE_ON",
	"|MOUSE_OFF",
	"T|LOAD_VIDEO",
	"|PLAY_VIDEO",
	/* 184 */
	"W|UNLOAD_ZONE",
	NULL,
	"|UNFREEZE_ZONES",
	"|RESET_GAME_TIME",
	/* 188 */
	"BSJ|STRING2_IS",
	"|CLEAR_MARKS",
	"B|WAIT_FOR_MARK",
	"|RESET_PV_COUNT",
	/* 192 */
	"BBBB|SET_PATH_VALUES",
	"|STOP_CLOCK",
	"|RESTART_CLOCK",
	"BBBB|SET_COLOR",
};

const char *const elvira1_videoOpcodeNameTable[] = {
	/* 0 */
	"x|RET",
	"ddd|FADEOUT",
	"d|CALL",
	"ddddd|NEW_SPRITE",
	/* 4 */
	"ddd|FADEIN",
	"vd|SKIP_IF_NEQ",
	"d|SKIP_IFN_SIB_WITH_A",
	"d|SKIP_IF_SIB_WITH_A",
	/* 8 */
	"dd|SKIP_IF_PARENT_IS",
	"dd|SKIP_IF_UNK3_IS",
	"dddd|DRAW",
	"d|VC_11",
	/* 12 */
	"|VC_12",
	"d|DELAY",
	"d|SET_SPRITE_OFFSET_X",
	"d|SET_SPRITE_OFFSET_Y",
	/* 16 */
	"d|SYNC",
	"d|WAIT_SYNC",
	"d|VC_18",
	"i|JUMP_REL",
	/* 20 */
	"|CHAIN_TO",
	"dd|SET_REPEAT",
	"i|END_REPEAT",
	"d|SET_PALETTE",
	/* 24 */
	"d|SET_PRIORITY",
	"diid|SET_SPRITE_XY",
	"x|HALT_SPRITE",
	"ddddd|SET_WINDOW",
	/* 28 */
	"|RESET",
	"dddd|PLAY_SOUND",
	"|STOP_ALL_SOUNDS",
	"d|SET_FRAME_RATE",
	/* 32 */
	"d|SET_WINDOW",
	"|VC_33",
	"|MOUSE_ON",
	"|MOUSE_OFF",
	/* 36 */
	"|VC_36",
	"d|VC_37",
	"dd|CLEAR_WINDOW",
	"d|VC_39",
	/* 40 */
	"dd|SET_WINDOW_IMAGE",
	"dd|VC_41",
	"|VC_42",
	"|VC_43",
	/* 44 */
	"d|VC_44",
	"d|VC_45",
	"d|VC_46",
	"dd|VC_47",
	/* 48 */
	"dd|VC_48",
	"|VC_49",
	"ddddddddd|VC_50",
	"v|SKIP_IF_VAR_ZERO",
	/* 52 */
	"vd|SET_VAR",
	"vd|ADD_VAR",
	"vd|SUB_VAR",
	"|VC_55",
	"dd|VC_56",
};

const char *const ww_videoOpcodeNameTable[] = {
	/* 0 */
	"x|RET",
	"ddd|FADEOUT",
	"d|CALL",
	"ddddd|NEW_SPRITE",
	/* 4 */
	"ddd|FADEIN",
	"vd|SKIP_IF_NEQ",
	"d|SKIP_IFN_SIB_WITH_A",
	"d|SKIP_IF_SIB_WITH_A",
	/* 8 */
	"dd|SKIP_IF_PARENT_IS",
	"dd|SKIP_IF_UNK3_IS",
	"dddd|DRAW",
	"d|VC_11",
	/* 12 */
	"d|DELAY",
	"d|SET_SPRITE_OFFSET_X",
	"d|SET_SPRITE_OFFSET_Y",
	"d|SYNC",
	/* 16 */
	"d|WAIT_SYNC",
	"d|VC_17",
	"i|JUMP_REL",
	"|CHAIN_TO",
	/* 20 */
	"dd|SET_REPEAT",
	"i|END_REPEAT",
	"d|SET_PALETTE",
	"d|SET_PRIORITY",
	/* 24 */
	"diid|SET_SPRITE_XY",
	"x|HALT_SPRITE",
	"ddddd|SET_WINDOW",
	"|RESET",
	/* 28 */
	"dddd|PLAY_SOUND",
	"|STOP_ALL_SOUNDS",
	"d|SET_FRAME_RATE",
	"d|SET_WINDOW",
	/* 32 */
	"|VC_32",
	"|MOUSE_ON",
	"|MOUSE_OFF",
	"dd|CLEAR_WINDOW",
	/* 36 */
	"dd|SET_WINDOW_IMAGE",
	"dd|VC_37",
	"v|SKIP_IF_VAR_ZERO",
	"vd|SET_VAR",
	/* 40 */
	"vd|ADD_VAR",
	"vd|SUB_VAR",
	"vd|DELAY_IF_NOT_EQ",
	"d|SKIP_IF_BIT_CLEAR",
	/* 44 */
	"d|SKIP_IF_BIT_SET",
	"dd|VC_45",
	"d|VC_46",
	"d|VC_47",
	/* 48 */
	"d|VC_48",
	"d|SET_BIT",
	"d|CLEAR_BIT",
	"d|ENABLE_BOX",
	/* 52 */
	"d|PLAY_EFFECT",
	"dd|DUMMY_53",
	"ddd|DUMMY_54",
	"ddd|MOVE_BOX",
	/* 56 */
	"|FULL_SCREEN",
	"|BLACK_PALETTE",
	"|SET_PRIORITIES",
	"|SKIP_IF_NOT_EGA",
	/* 60 */
	"d|STOP_ANIMATE",
	"d|VC_61",
	"|FASTFADEOUT",
	"|FASTFADEIN",
};

const char *const simon1_videoOpcodeNameTable[] = {
	/* 0 */
	"x|RET",
	"ddd|FADEOUT",
	"d|CALL",
	"ddddd|NEW_SPRITE",
	/* 4 */
	"ddd|FADEIN",
	"vd|SKIP_IF_NEQ",
	"d|SKIP_IFN_SIB_WITH_A",
	"d|SKIP_IF_SIB_WITH_A",
	/* 8 */
	"dd|SKIP_IF_PARENT_IS",
	"dd|SKIP_IF_UNK3_IS",
	"ddddd|DRAW",
	"|CLEAR_PATHFIND_ARRAY",
	/* 12 */
	"d|DELAY",
	"d|SET_SPRITE_OFFSET_X",
	"d|SET_SPRITE_OFFSET_Y",
	"d|SYNC",
	/* 16 */
	"d|WAIT_SYNC",
	"dq|SET_PATHFIND_ITEM",
	"i|JUMP_REL",
	"|CHAIN_TO",
	/* 20 */
	"dd|SET_REPEAT",
	"i|END_REPEAT",
	"dd|SET_PALETTE",
	"d|SET_PRIORITY",
	/* 24 */
	"diid|SET_SPRITE_XY",
	"x|HALT_SPRITE",
	"ddddd|SET_WINDOW",
	"|RESET",
	/* 28 */
	"dddd|PLAY_SOUND",
	"|STOP_ALL_SOUNDS",
	"d|SET_FRAME_RATE",
	"d|SET_WINDOW",
	/* 32 */
	"vv|COPY_VAR",
	"|MOUSE_ON",
	"|MOUSE_OFF",
	"dd|CLEAR_WINDOW",
	/* 36 */
	"dd|SET_WINDOW_IMAGE",
	"v|SET_SPRITE_OFFSET_Y",
	"v|SKIP_IF_VAR_ZERO",
	"vd|SET_VAR",
	/* 40 */
	"vd|ADD_VAR",
	"vd|SUB_VAR",
	"vd|DELAY_IF_NOT_EQ",
	"d|SKIP_IF_BIT_CLEAR",
	/* 44 */
	"d|SKIP_IF_BIT_SET",
	"v|SET_SPRITE_X",
	"v|SET_SPRITE_Y",
	"vv|ADD_VAR_F",
	/* 48 */
	"|COMPUTE_YOFS",
	"d|SET_BIT",
	"d|CLEAR_BIT",
	"d|ENABLE_BOX",
	/* 52 */
	"d|PLAY_EFFECT",
	"dd|DUMMY_53",
	"ddd|DUMMY_54",
	"ddd|MOVE_BOX",
	/* 56 */
	"|DUMMY_56",
	"|BLACK_PALETTE",
	"|SET_PRIORITIES",
	"|SKIP_IF_VOICE",
	/* 60 */
	"d|STOP_ANIMATE",
	"ddd|MASK",
	"|FASTFADEOUT",
	"|FASTFADEIN",
};

const char *const simon2_videoOpcodeNameTable[] = {
	/* 0 */
	"x|RET",
	"ddd|FADEOUT",
	"d|CALL",
	"ddddd|NEW_SPRITE",
	/* 4 */
	"ddd|FADEIN",
	"vd|SKIP_IF_NEQ",
	"d|SKIP_IFN_SIB_WITH_A",
	"d|SKIP_IF_SIB_WITH_A",
	/* 8 */
	"dd|SKIP_IF_PARENT_IS",
	"dd|SKIP_IF_UNK3_IS",
	"ddddb|DRAW",
	"|CLEAR_PATHFIND_ARRAY",
	/* 12 */
	"b|DELAY",
	"d|SET_SPRITE_OFFSET_X",
	"d|SET_SPRITE_OFFSET_Y",
	"d|SYNC",
	/* 16 */
	"d|WAIT_SYNC",
	"dq|SET_PATHFIND_ITEM",
	"i|JUMP_REL",
	"|CHAIN_TO",
	/* 20 */
	"dd|SET_REPEAT",
	"i|END_REPEAT",
	"dd|SET_PALETTE",
	"d|SET_PRIORITY",
	/* 24 */
	"diib|SET_SPRITE_XY",
	"x|HALT_SPRITE",
	"ddddd|SET_WINDOW",
	"|RESET",
	/* 28 */
	"dddd|PLAY_SOUND",
	"|STOP_ALL_SOUNDS",
	"d|SET_FRAME_RATE",
	"d|SET_WINDOW",
	/* 32 */
	"vv|COPY_VAR",
	"|MOUSE_ON",
	"|MOUSE_OFF",
	"dd|CLEAR_WINDOW",
	/* 36 */
	"dd|SET_WINDOW_IMAGE",
	"v|SET_SPRITE_OFFSET_Y",
	"v|SKIP_IF_VAR_ZERO",
	"vd|SET_VAR",
	/* 40 */
	"vd|ADD_VAR",
	"vd|SUB_VAR",
	"vd|DELAY_IF_NOT_EQ",
	"d|SKIP_IF_BIT_CLEAR",
	/* 44 */
	"d|SKIP_IF_BIT_SET",
	"v|SET_SPRITE_X",
	"v|SET_SPRITE_Y",
	"vv|ADD_VAR_F",
	/* 48 */
	"|COMPUTE_YOFS",
	"d|SET_BIT",
	"d|CLEAR_BIT",
	"d|ENABLE_BOX",
	/* 52 */
	"d|PLAY_EFFECT",
	"dd|DUMMY_53",
	"ddd|DUMMY_54",
	"ddd|MOVE_BOX",
	/* 56 */
	"i|WAIT_BIG",
	"|BLACK_PALETTE",
	"|SET_PRIORITIES",
	"ddd|STOP_ANIMATIONS",
	/* 60 */
	"dd|STOP_ANIMATE",
	"ddd|MASK",
	"|FASTFADEOUT",
	"|FASTFADEIN",
	/* 64 */
	"|SKIP_IF_VOICE",
	"|SLOW_FADE_IN",
	"|SKIP_IF_NZ",
	"|SKIP_IF_GE",
	/* 68 */
	"|SKIP_IF_LE",
	"dd|PLAY_TRACK",
	"dd|QUEUE_MUSIC",
	"|CHECK_MUSIC_QUEUE",
	/* 72 */
	"dd|PLAY_TRACK_2",
	"bb|SET_MARK",
	"bb|CLEAR_MARK",
};

const char *const feeblefiles_videoOpcodeNameTable[] = {
	/* 0 */
	"x|RET",
	"ddd|FADEOUT",
	"d|CALL",
	"ddddd|NEW_SPRITE",
	/* 4 */
	"ddd|FADEIN",
	"vd|SKIP_IF_NEQ",
	"d|SKIP_IFN_SIB_WITH_A",
	"d|SKIP_IF_SIB_WITH_A",
	/* 8 */
	"dd|SKIP_IF_PARENT_IS",
	"dd|SKIP_IF_UNK3_IS",
	"ddddb|DRAW",
	"|CLEAR_PATHFIND_ARRAY",
	/* 12 */
	"b|DELAY",
	"d|SET_SPRITE_OFFSET_X",
	"d|SET_SPRITE_OFFSET_Y",
	"d|SYNC",
	/* 16 */
	"d|WAIT_SYNC",
	"dq|SET_PATHFIND_ITEM",
	"i|JUMP_REL",
	"|CHAIN_TO",
	/* 20 */
	"dd|SET_REPEAT",
	"i|END_REPEAT",
	"dd|SET_PALETTE",
	"d|SET_PRIORITY",
	/* 24 */
	"diib|SET_SPRITE_XY",
	"x|HALT_SPRITE",
	"ddddd|SET_WINDOW",
	"|RESET",
	/* 28 */
	"dddd|PLAY_SOUND",
	"|STOP_ALL_SOUNDS",
	"d|SET_FRAME_RATE",
	"d|SET_WINDOW",
	/* 32 */
	"vv|COPY_VAR",
	"|MOUSE_ON",
	"|MOUSE_OFF",
	"dd|CLEAR_WINDOW",
	/* 36 */
	"dd|SET_WINDOW_IMAGE",
	"v|SET_SPRITE_OFFSET_Y",
	"v|SKIP_IF_VAR_ZERO",
	"vd|SET_VAR",
	/* 40 */
	"vd|ADD_VAR",
	"vd|SUB_VAR",
	"vd|DELAY_IF_NOT_EQ",
	"d|SKIP_IF_BIT_CLEAR",
	/* 44 */
	"d|SKIP_IF_BIT_SET",
	"v|SET_SPRITE_X",
	"v|SET_SPRITE_Y",
	"vv|ADD_VAR_F",
	/* 48 */
	"|COMPUTE_YOFS",
	"d|SET_BIT",
	"d|CLEAR_BIT",
	"d|ENABLE_BOX",
	/* 52 */
	"ddd|PLAY_EFFECT",
	"ddd|PAN_SFX",
	"ddd|DUMMY_54",
	"ddd|MOVE_BOX",
	/* 56 */
	"i|WAIT_BIG",
	"|BLACK_PALETTE",
	"|SET_PRIORITIES",
	"ddd|STOP_ANIMATIONS",
	/* 60 */
	"dd|STOP_ANIMATE",
	"ddd|MASK",
	"|FASTFADEOUT",
	"|FASTFADEIN",
	/* 64 */
	"|SKIP_IF_VOICE",
	"|SLOW_FADE_IN",
	"|SKIP_IF_NZ",
	"|SKIP_IF_GE",
	/* 68 */
	"|SKIP_IF_LE",
	"dd|PLAY_TRACK",
	"dd|QUEUE_MUSIC",
	"|CHECK_MUSIC_QUEUE",
	/* 72 */
	"dd|PLAY_TRACK_2",
	"bb|SET_MARK",
	"bb|CLEAR_MARK",
	"dd|SETSCALE",
	/* 76 */
	"ddd|SETSCALEXOFFS",
	"ddd|SETSCALEYOFFS",
	"|COMPUTEXY",
	"|COMPUTEPOSNUM",
	/* 80 */
	"ddd|SETOVERLAYIMAGE",
	"dd|SETRANDOM",
	"d|GETPATHVALUE",
	"ddd|PLAYSOUNDLOOP",
	"|STOPSOUNDLOOP",
};

} // End of namespace AGOS

#endif

