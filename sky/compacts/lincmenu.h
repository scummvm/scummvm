/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2004 The ScummVM project
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

#ifndef SKYLINCMENU_H
#define SKYLINCMENU_H




namespace Sky {

namespace SkyCompact {

Compact playbak_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	29+191*64,	// frame
	24602,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	LINC_MENU_SELECT,	// mouseClick
	(int16) 65534,	// mouseRel_x
	0,	// mouseRel_y
	24,	// mouseSize_x
	24,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	LINC_MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact persona_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	23+191*64,	// frame
	24583,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	LINC_MENU_SELECT,	// mouseClick
	(int16) 65534,	// mouseRel_x
	0,	// mouseRel_y
	24,	// mouseSize_x
	24,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	LINC_MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact info_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	1+191*64,	// frame
	24577,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	INFO_MENU_SELECT,	// mouseClick
	(int16) 65534,	// mouseRel_x
	0,	// mouseRel_y
	24,	// mouseSize_x
	24,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	LINC_MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact decrypt_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	21+191*64,	// frame
	24581,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	DECRYPT_MENU_SELECT,	// mouseClick
	(int16) 65534,	// mouseRel_x
	0,	// mouseRel_y
	24,	// mouseSize_x
	24,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	LINC_MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

uint16 linc_menu_mouse[] = {
	18,
	23,
	24,
	25,
	26,
	27,
	28,
	29,
	30,
	31,
	32,
	47,
	48,
	51,
	52,
	53,
	54,
	55,
	56,
	57,
	58,
	59,
	60,
	61,
	ID_INFO_MENU,
	ID_READ_MENU,
	ID_OPEN_MENU,
	ID_CHARON_MENU,
	ID_ORDERS_MENU,
	ID_ORDERS2_MENU,
	ID_JOIN_MENU,
	ID_GREEN_MENU,
	ID_RED_MENU,
	ID_REPORT_MENU,
	ID_REPORT2_MENU,
	ID_DECOMP_MENU,
	ID_DECRYPT_MENU,
	ID_PERSONA_MENU,
	ID_ADJUST_MENU,
	ID_ADJUST2_MENU,
	ID_PLAYBAK_MENU,
	ID_BLIND_MENU,
	ID_OSCILL_MENU,
	ID_KILL_MENU,
	ID_VIRUS_MENU,
	46,
	0
};

Compact kill_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	33+191*64,	// frame
	24689,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	LINC_MENU_SELECT,	// mouseClick
	(int16) 65534,	// mouseRel_x
	0,	// mouseRel_y
	24,	// mouseSize_x
	24,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	LINC_MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact read_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	3+191*64,	// frame
	24578,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	DIS_MENU_SELECT,	// mouseClick
	(int16) 65534,	// mouseRel_x
	0,	// mouseRel_y
	24,	// mouseSize_x
	24,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	LINC_MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact orders_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	31+191*64,	// frame
	24629,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	DOC_MENU_SELECT,	// mouseClick
	(int16) 65534,	// mouseRel_x
	0,	// mouseRel_y
	24,	// mouseSize_x
	24,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	LINC_MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact report_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	31+191*64,	// frame
	24629,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	DOC_MENU_SELECT,	// mouseClick
	(int16) 65534,	// mouseRel_x
	0,	// mouseRel_y
	24,	// mouseSize_x
	24,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	LINC_MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact virus_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	25+191*64,	// frame
	24591,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	LINC_MENU_SELECT,	// mouseClick
	(int16) 65534,	// mouseRel_x
	0,	// mouseRel_y
	24,	// mouseSize_x
	24,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	LINC_MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact adjust_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	31+191*64,	// frame
	24629,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	DOC_MENU_SELECT,	// mouseClick
	(int16) 65534,	// mouseRel_x
	0,	// mouseRel_y
	24,	// mouseSize_x
	24,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	LINC_MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact charon_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	15+191*64,	// frame
	24599,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	LINC_MENU_SELECT,	// mouseClick
	(int16) 65534,	// mouseRel_x
	0,	// mouseRel_y
	24,	// mouseSize_x
	24,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	LINC_MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact orders2_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	19+191*64,	// frame
	24629,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	LINC_MENU_SELECT,	// mouseClick
	(int16) 65534,	// mouseRel_x
	0,	// mouseRel_y
	24,	// mouseSize_x
	24,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	LINC_MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact report2_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	19+191*64,	// frame
	24629,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	LINC_MENU_SELECT,	// mouseClick
	(int16) 65534,	// mouseRel_x
	0,	// mouseRel_y
	24,	// mouseSize_x
	24,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	LINC_MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact oscill_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	27+191*64,	// frame
	24589,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	LINC_MENU_SELECT,	// mouseClick
	(int16) 65534,	// mouseRel_x
	0,	// mouseRel_y
	24,	// mouseSize_x
	24,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	LINC_MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact join_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	7+191*64,	// frame
	24593,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	JOIN_MENU_SELECT,	// mouseClick
	(int16) 65534,	// mouseRel_x
	0,	// mouseRel_y
	24,	// mouseSize_x
	24,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	LINC_MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact green_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	11+191*64,	// frame
	24580,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	LINC_MENU_SELECT,	// mouseClick
	(int16) 65534,	// mouseRel_x
	0,	// mouseRel_y
	24,	// mouseSize_x
	24,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	LINC_MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

uint16 linc_menu_logic[] = {
	18,
	47,
	48,
	ID_INFO_MENU,
	ID_READ_MENU,
	ID_OPEN_MENU,
	ID_CHARON_MENU,
	ID_ORDERS_MENU,
	ID_ORDERS2_MENU,
	ID_JOIN_MENU,
	ID_GREEN_MENU,
	ID_RED_MENU,
	ID_REPORT_MENU,
	ID_REPORT2_MENU,
	ID_DECOMP_MENU,
	ID_DECRYPT_MENU,
	ID_PERSONA_MENU,
	ID_ADJUST_MENU,
	ID_ADJUST2_MENU,
	ID_PLAYBAK_MENU,
	ID_BLIND_MENU,
	ID_OSCILL_MENU,
	ID_KILL_MENU,
	ID_VIRUS_MENU,
	51,
	52,
	53,
	54,
	55,
	56,
	57,
	58,
	59,
	60,
	61,
	23,
	24,
	25,
	26,
	27,
	28,
	29,
	30,
	31,
	32,
	0
};

Compact adjust2_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	19+191*64,	// frame
	24629,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	LINC_MENU_SELECT,	// mouseClick
	(int16) 65534,	// mouseRel_x
	0,	// mouseRel_y
	24,	// mouseSize_x
	24,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	LINC_MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact red_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	13+191*64,	// frame
	24580,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	LINC_MENU_SELECT,	// mouseClick
	(int16) 65534,	// mouseRel_x
	0,	// mouseRel_y
	24,	// mouseSize_x
	24,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	LINC_MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact open_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	5+191*64,	// frame
	24579,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	LINC_MENU_SELECT,	// mouseClick
	(int16) 65534,	// mouseRel_x
	0,	// mouseRel_y
	24,	// mouseSize_x
	24,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	LINC_MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact decomp_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	17+191*64,	// frame
	24582,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	DECOMP_MENU_SELECT,	// mouseClick
	(int16) 65534,	// mouseRel_x
	0,	// mouseRel_y
	24,	// mouseSize_x
	24,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	LINC_MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact blind_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	9+191*64,	// frame
	24600,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	LINC_MENU_SELECT,	// mouseClick
	(int16) 65534,	// mouseRel_x
	0,	// mouseRel_y
	24,	// mouseSize_x
	24,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	LINC_MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
