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

#ifndef SKYLINC_GEN_H
#define SKYLINC_GEN_H




namespace Sky {

namespace SkyCompact {

uint16 window_mouse[] = {
	ID_INFO_BUTTON,
	0
};

uint16 logon[] = {
	117*64,
	1,
	1,
	8,
	1,
	1,
	7,
	1,
	1,
	6,
	1,
	1,
	5,
	1,
	1,
	4,
	1,
	1,
	3,
	1,
	1,
	1,
	1,
	1,
	2,
	1,
	1,
	1,
	1,
	1,
	0,
	0
};

uint32 *grid96 = 0;

uint16 note_module[] = {
	11,
	176,
	0,
	199,
	226
};

uint16 window_logic[] = {
	ID_WINDOW_1,
	ID_WINDOW_2,
	ID_WINDOW_3,
	ID_WINDOW_4,
	ID_INFO_BUTTON,
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

uint16 crouch_right_b[] = {
	17*64,
	1,
	1,
	3,
	1,
	1,
	3,
	1,
	1,
	2,
	1,
	1,
	1,
	1,
	1,
	0,
	0
};

uint16 crouch_down[] = {
	20*64,
	201,
	272,
	0,
	201,
	272,
	1,
	201,
	272,
	2,
	201,
	272,
	3,
	201,
	272,
	3,
	201,
	272,
	3,
	201,
	272,
	3,
	201,
	272,
	2,
	201,
	272,
	1,
	201,
	272,
	0,
	0
};

uint16 auth_module[] = {
	11,
	176,
	0,
	199,
	202
};

Compact window_3 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_FOREGROUND+ST_RECREATE,	// status
	0,	// sync
	90,	// screen
	0,	// place
	0,	// getToTable
	195,	// xcood
	160+72,	// ycood
	2+26*64,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	0,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	0,	// mouseSize_x
	0,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	INFO_WINDOW_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint32 *grid91 = 0;

Compact info_button = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_MOUSE+ST_FOREGROUND+ST_RECREATE,	// status
	0,	// sync
	90,	// screen
	0,	// place
	0,	// getToTable
	334,	// xcood
	285,	// ycood
	137*64,	// frame
	0,	// cursorText
	BUTTON_MOUSE,	// mouseOn
	NORMAL_MOUSE,	// mouseOff
	CLOSE_WINDOW,	// mouseClick
	2,	// mouseRel_x
	1,	// mouseRel_y
	37,	// mouseSize_x
	9,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	INFO_BUTTON_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 size_module[] = {
	11,
	176,
	0,
	199,
	190
};

Compact window_2 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_FOREGROUND+ST_RECREATE,	// status
	0,	// sync
	90,	// screen
	0,	// place
	0,	// getToTable
	195+76,	// xcood
	160,	// ycood
	1+26*64,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	0,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	0,	// mouseSize_x
	0,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	INFO_WINDOW_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 logoff[] = {
	117*64,
	261,
	181,
	0,
	261,
	181,
	1,
	261,
	181,
	2,
	261,
	181,
	1,
	261,
	181,
	3,
	261,
	181,
	4,
	261,
	181,
	5,
	261,
	181,
	6,
	261,
	181,
	7,
	261,
	181,
	8,
	0
};

uint32 *grid94 = 0;

uint16 enter_top[] = {
	135*64,
	200,
	215,
	0,
	200,
	215,
	1,
	200,
	215,
	2,
	200,
	215,
	3,
	200,
	215,
	4,
	200,
	215,
	5,
	200,
	215,
	6,
	0
};

uint16 crouch_right[] = {
	17*64,
	260,
	215,
	0,
	260,
	215,
	1,
	260,
	215,
	2,
	260,
	215,
	3,
	260,
	215,
	3,
	260,
	215,
	3,
	260,
	215,
	3,
	260,
	215,
	2,
	260,
	215,
	1,
	260,
	215,
	0,
	0
};

uint32 *grid95 = 0;

Compact window_1 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_FOREGROUND+ST_RECREATE,	// status
	0,	// sync
	90,	// screen
	0,	// place
	0,	// getToTable
	195,	// xcood
	160,	// ycood
	26*64,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	0,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	0,	// mouseSize_x
	0,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	INFO_WINDOW_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint32 *grid90 = 0;

uint16 exit_top[] = {
	136*64,
	200,
	215,
	0,
	200,
	215,
	1,
	200,
	215,
	2,
	200,
	215,
	3,
	200,
	215,
	4,
	200,
	215,
	5,
	200,
	215,
	6,
	0
};

uint16 shrug[] = {
	182*64,
	1,
	1,
	48,
	1,
	1,
	49,
	1,
	1,
	49,
	1,
	1,
	49,
	1,
	1,
	49,
	1,
	1,
	48,
	0
};

uint16 head_module[] = {
	11,
	176,
	0,
	199,
	161
};

uint16 crouch_right_a[] = {
	17*64,
	1,
	1,
	0,
	1,
	1,
	1,
	1,
	1,
	2,
	1,
	1,
	3,
	1,
	1,
	3,
	0
};

uint16 file_module[] = {
	11,
	176,
	0,
	199,
	178
};

uint16 crouch_left[] = {
	16*64,
	320,
	215,
	0,
	320,
	215,
	1,
	320,
	215,
	2,
	320,
	215,
	3,
	320,
	215,
	3,
	320,
	215,
	3,
	320,
	215,
	3,
	320,
	215,
	2,
	320,
	215,
	1,
	320,
	215,
	0,
	0
};

uint32 *grid92 = 0;

Compact window_4 = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_FOREGROUND+ST_RECREATE,	// status
	0,	// sync
	90,	// screen
	0,	// place
	0,	// getToTable
	195+76,	// xcood
	160+72,	// ycood
	3+26*64,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	0,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	0,	// mouseSize_x
	0,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	INFO_WINDOW_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 rs_foster_s90[] = {
	C_SCREEN,
	90,
	C_PLACE,
	ID_SC90_FLOOR,
	C_XCOOD,
	288,
	C_YCOOD,
	280,
	C_DIR,
	DOWN,
	C_FRAME,
	117*64,
	65535
};

uint32 *grid93 = 0;

} // namespace SkyCompact

} // namespace Sky

#endif
