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

#ifndef SKYOBJECTS_H
#define SKYOBJECTS_H




namespace Sky {

namespace SkyCompact {

Compact blank5 = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	50*64,	// frame
	0,	// cursorText
	TOUCH_ARROW,	// mouseOn
	UNTOUCH_ARROW,	// mouseOff
	0,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact tape_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	265*64,	// frame
	173,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	MENU_SELECT,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact right_arrow = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	447-27,	// xcood
	112,	// ycood
	49*64+1,	// frame
	0,	// cursorText
	TOUCH_ARROW,	// mouseOn
	UNTOUCH_ARROW,	// mouseOff
	CLICK_RIGHT_ARROW,	// mouseClick
	(int16) 65534,	// mouseRel_x
	0,	// mouseRel_y
	29,	// mouseSize_x
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

uint16 move_2[] = {
	20,
	0
};

Compact glass_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	36*64,	// frame
	200,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	MENU_SELECT,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact lightbulb_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	73*64+2,	// frame
	66,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	MENU_SELECT,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact blank3 = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	50*64,	// frame
	0,	// cursorText
	TOUCH_ARROW,	// mouseOn
	UNTOUCH_ARROW,	// mouseOff
	0,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact wd40_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	87*64,	// frame
	8373,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	MENU_SELECT,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact plaster_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	0+268*64,	// frame
	20506,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	MENU_SELECT,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact new_cable_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	267*64,	// frame
	320,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	MENU_SELECT,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact blank8 = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	50*64,	// frame
	0,	// cursorText
	TOUCH_ARROW,	// mouseOn
	UNTOUCH_ARROW,	// mouseOff
	0,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact joeyb_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	50*64+3,	// frame
	6,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	MENU_SELECT,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

uint16 move_menu[] = {
	46,
	18
};

uint16 *move_list[] = {
	move_menu,
	move_2
};

Compact blank6 = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	50*64,	// frame
	0,	// cursorText
	TOUCH_ARROW,	// mouseOn
	UNTOUCH_ARROW,	// mouseOff
	0,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact shades_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	50*64+9,	// frame
	10,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	MENU_SELECT,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact citycard_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	50*64+11,	// frame
	9,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	MENU_SELECT,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact blank9 = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	50*64,	// frame
	0,	// cursorText
	TOUCH_ARROW,	// mouseOn
	UNTOUCH_ARROW,	// mouseOff
	0,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact rope_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	266*64,	// frame
	49,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	MENU_SELECT,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact left_arrow = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	128,	// xcood
	112,	// ycood
	49*64,	// frame
	0,	// cursorText
	TOUCH_ARROW,	// mouseOn
	UNTOUCH_ARROW,	// mouseOff
	CLICK_LEFT_ARROW,	// mouseClick
	(int16) 65534,	// mouseRel_x
	0,	// mouseRel_y
	28,	// mouseSize_x
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact magazine_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	264*64,	// frame
	163,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	MENU_SELECT,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact gallcard_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	12+268*64,	// frame
	68,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	MENU_SELECT,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact dog_food_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	271*64,	// frame
	198,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	MENU_SELECT,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact blank7 = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	50*64,	// frame
	0,	// cursorText
	TOUCH_ARROW,	// mouseOn
	UNTOUCH_ARROW,	// mouseOff
	0,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact blank2 = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	50*64,	// frame
	0,	// cursorText
	TOUCH_ARROW,	// mouseOn
	UNTOUCH_ARROW,	// mouseOff
	0,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact blank11 = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	50*64,	// frame
	0,	// cursorText
	TOUCH_ARROW,	// mouseOn
	UNTOUCH_ARROW,	// mouseOff
	0,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact anchor_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	263*64,	// frame
	191,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	MENU_SELECT,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact anita_card_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	262*64,	// frame
	68,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	MENU_SELECT,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact tongs_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	4+268*64,	// frame
	20639,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	MENU_SELECT,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact blank4 = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	50*64,	// frame
	0,	// cursorText
	TOUCH_ARROW,	// mouseOn
	UNTOUCH_ARROW,	// mouseOff
	0,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact crow_bar_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	50*64+1,	// frame
	5,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	MENU_SELECT,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact secateurs_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	272*64,	// frame
	180,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	MENU_SELECT,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact sarnie_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	50*64+5,	// frame
	4+T7,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	MENU_SELECT,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact spanner_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	50*64+7,	// frame
	5+T7,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	MENU_SELECT,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact blank10 = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	50*64,	// frame
	0,	// cursorText
	TOUCH_ARROW,	// mouseOn
	UNTOUCH_ARROW,	// mouseOff
	0,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact skey_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	87*64+2,	// frame
	8380,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	MENU_SELECT,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact blank1 = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	50*64,	// frame
	0,	// cursorText
	TOUCH_ARROW,	// mouseOn
	UNTOUCH_ARROW,	// mouseOff
	0,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact ticket_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	269*64,	// frame
	189,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	MENU_SELECT,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact brick_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	2+268*64,	// frame
	20509,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	MENU_SELECT,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact putty_menu = {
	L_SCRIPT,	// logic
	0,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	112,	// ycood
	73*64,	// frame
	137,	// cursorText
	TOUCH_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	MENU_SELECT,	// mouseClick
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
	MENU_SCRIPT,	// baseSub
	0,	// baseSub_off
	0
};

Compact menu_bar = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	0,	// screen
	0,	// place
	0,	// getToTable
	128,	// xcood
	136,	// ycood
	0,	// frame
	1,	// cursorText
	START_MENU,	// mouseOn
	UNTOUCH_MENU,	// mouseOff
	0,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	320,	// mouseSize_x
	1,	// mouseSize_y
	0,
	0,
	0,
	0,
	0,
	0,
	{ 0, 0, 0 },
	0,
	0,
	0,
	0,
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
