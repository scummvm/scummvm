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

#ifndef SKY21COMP_H
#define SKY21COMP_H


#include "22comp.h"


namespace Sky {

namespace SkyCompact {

uint16 lamb_sit_up[] = {
	218*64,
	248,
	246,
	9,
	248,
	246,
	8,
	248,
	246,
	7,
	248,
	246,
	6,
	249,
	246,
	5,
	249,
	245,
	4,
	249,
	246,
	3,
	253,
	247,
	2,
	264,
	241,
	1,
	268,
	241,
	0,
	0
};

uint16 s21_logic[] = {
	ID_FOSTER,
	ID_JOEY,
	ID_LAMB,
	ID_CATFOOD,
	ID_VIDEO,
	ID_CASSETTE,
	ID_VIDEO_SCREEN,
	ID_CAT,
	ID_INNER_LAMB_DOOR,
	ID_LAMB_DOOR_20,
	ID_GALLAGER_BEL,
	ID_LIFT_29,
	0XFFFF,
	ID_MENU_LOGIC
};

uint16 cat1_seq[] = {
	103*64,
	347,
	214,
	0,
	347,
	214,
	1,
	347,
	214,
	2,
	347,
	214,
	3,
	347,
	214,
	3,
	347,
	214,
	4,
	347,
	214,
	5,
	347,
	214,
	6,
	347,
	214,
	7,
	347,
	214,
	8,
	347,
	214,
	9,
	347,
	214,
	10,
	347,
	214,
	11,
	347,
	214,
	12,
	347,
	214,
	13,
	347,
	214,
	14,
	347,
	214,
	15,
	347,
	214,
	16,
	347,
	214,
	17,
	347,
	214,
	18,
	347,
	214,
	19,
	347,
	214,
	20,
	347,
	214,
	21,
	347,
	214,
	22,
	347,
	214,
	23,
	347,
	214,
	24,
	347,
	214,
	25,
	347,
	214,
	26,
	347,
	214,
	27,
	347,
	214,
	28,
	347,
	214,
	29,
	347,
	214,
	30,
	347,
	214,
	31,
	347,
	214,
	32,
	347,
	214,
	33,
	347,
	214,
	34,
	347,
	214,
	35,
	347,
	214,
	36,
	347,
	214,
	37,
	347,
	214,
	27,
	347,
	214,
	28,
	347,
	214,
	29,
	347,
	214,
	30,
	347,
	214,
	31,
	347,
	214,
	32,
	347,
	214,
	33,
	347,
	214,
	34,
	347,
	214,
	35,
	347,
	214,
	36,
	347,
	214,
	37,
	347,
	214,
	38,
	347,
	214,
	39,
	347,
	214,
	40,
	347,
	214,
	41,
	347,
	214,
	42,
	347,
	214,
	43,
	347,
	214,
	44,
	0
};

uint16 rs_foster_21_20[] = {
	C_XCOOD,
	328,
	C_YCOOD,
	280,
	C_SCREEN,
	20,
	C_PLACE,
	ID_S20_FLOOR,
	65535
};

uint16 inner_lamb_close[] = {
	89*64,
	1,
	1,
	10,
	1,
	1,
	9,
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
	2,
	1,
	1,
	1,
	1,
	1,
	0,
	0
};

uint16 push_dispensor[] = {
	201*64,
	352,
	224,
	0,
	352,
	224,
	1,
	352,
	224,
	2,
	352,
	224,
	3,
	352,
	224,
	4,
	352,
	224,
	4,
	352,
	224,
	4,
	352,
	224,
	4,
	352,
	224,
	2,
	352,
	224,
	5,
	352,
	224,
	0,
	0
};

uint16 s21_floor_table[] = {
	ID_S21_FLOOR,
	RET_OK,
	ID_JOEY_PARK,
	GET_TO_JP2,
	0,
	S21_START_ON,
	ID_LEFT_EXIT_21,
	GT_LEFT_EXIT_21,
	ID_LAMBS_BOOKS,
	GT_LAMBS_BOOKS,
	ID_LAMBS_CHAIR,
	GT_LAMBS_CHAIR,
	ID_DISPENSOR,
	GT_DISPENSOR,
	ID_CATFOOD,
	GT_CAT_FOOD,
	ID_VIDEO,
	GT_VIDEO,
	ID_CASSETTE,
	GT_CASSETTE,
	ID_BIG_PICT1,
	GT_BIG_PICT1,
	ID_BIG_PICT2,
	GT_BIG_PICT2,
	ID_BIG_PICT3,
	GT_BIG_PICT3,
	ID_VIDEO_SCREEN,
	GT_VIDEO_SCREEN,
	ID_CAT,
	GT_CAT,
	65535
};

Compact s21_floor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	21,	// screen
	0,	// place
	s21_floor_table,	// getToTable
	128,	// xcood
	248,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	320,	// mouseSize_x
	4242,	// mouseSize_y
	FLOOR_ACTION,	// actionScript
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

uint16 s21_chip_list[] = {
	201+DISK_9,
	202+DISK_9,
	110+DISK_9,
	109+DISK_9,
	108+DISK_9,
	107+DISK_9,
	101+DISK_9,
	102+DISK_9,
	89+DISK_9,
	131+DISK_2,
	132+DISK_2,
	218+DISK_9,
	0
};

uint16 lamb_sit_seq[] = {
	218*64,
	268,
	241,
	0,
	264,
	241,
	1,
	253,
	247,
	2,
	249,
	246,
	3,
	249,
	245,
	4,
	249,
	246,
	5,
	248,
	246,
	6,
	248,
	246,
	7,
	248,
	246,
	8,
	248,
	246,
	9,
	0
};

Compact cat_food = {
	L_SCRIPT,	// logic
	ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	21,	// screen
	0,	// place
	0,	// getToTable
	0,	// xcood
	0,	// ycood
	0,	// frame
	193,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	2,	// mouseRel_x
	18,	// mouseRel_y
	6,	// mouseSize_x
	1,	// mouseSize_y
	CAT_FOOD_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	CAT_FOOD_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact video = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	21,	// screen
	0,	// place
	0,	// getToTable
	345,	// xcood
	259,	// ycood
	0,	// frame
	12677,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	(int16) 65529,	// mouseRel_y
	25,	// mouseSize_x
	19,	// mouseSize_y
	VIDEO_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	VIDEO_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact video_screen = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_RECREATE,	// status
	0,	// sync
	21,	// screen
	0,	// place
	0,	// getToTable
	348,	// xcood
	216,	// ycood
	0,	// frame
	57,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	378-349,	// mouseSize_x
	253-216,	// mouseSize_y
	VIDEO_SCREEN_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	VIDEO_SCREEN_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

uint16 rs_foster_20_21[] = {
	C_XCOOD,
	136,
	C_YCOOD,
	296,
	C_SCREEN,
	21,
	C_PLACE,
	ID_S21_FLOOR,
	65535
};

uint16 cat4_seq[] = {
	106*64,
	347,
	214,
	0,
	347,
	214,
	1,
	347,
	214,
	2,
	347,
	214,
	3,
	347,
	214,
	4,
	347,
	214,
	5,
	347,
	214,
	6,
	347,
	214,
	7,
	347,
	214,
	8,
	347,
	214,
	9,
	347,
	214,
	10,
	347,
	214,
	11,
	347,
	214,
	12,
	347,
	214,
	13,
	347,
	214,
	14,
	347,
	214,
	15,
	347,
	214,
	16,
	347,
	214,
	17,
	347,
	214,
	18,
	347,
	214,
	19,
	347,
	214,
	0,
	347,
	214,
	1,
	347,
	214,
	2,
	347,
	214,
	3,
	347,
	214,
	4,
	347,
	214,
	5,
	347,
	214,
	6,
	347,
	214,
	7,
	347,
	214,
	8,
	347,
	214,
	9,
	347,
	214,
	10,
	347,
	214,
	11,
	347,
	214,
	12,
	347,
	214,
	13,
	347,
	214,
	14,
	347,
	214,
	15,
	347,
	214,
	16,
	347,
	214,
	20,
	347,
	214,
	21,
	347,
	214,
	22,
	347,
	214,
	23,
	0
};

uint16 video_out[] = {
	109*64,
	345,
	259,
	5,
	345,
	259,
	4,
	345,
	259,
	3,
	345,
	259,
	2,
	345,
	259,
	1,
	345,
	259,
	0,
	345,
	259,
	0,
	345,
	259,
	0,
	0
};

uint16 inner_lamb_open[] = {
	89*64,
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
	4,
	1,
	1,
	5,
	1,
	1,
	6,
	1,
	1,
	7,
	1,
	1,
	8,
	1,
	1,
	9,
	1,
	1,
	10,
	1,
	1,
	11,
	0
};

uint16 s21_mouse[] = {
	ID_JOEY,
	ID_LAMB,
	ID_INNER_LAMB_DOOR,
	ID_LEFT_EXIT_21,
	ID_LAMBS_BOOKS,
	ID_LAMBS_CHAIR,
	ID_DISPENSOR,
	ID_CATFOOD,
	ID_VIDEO,
	ID_CASSETTE,
	ID_BIG_PICT1,
	ID_BIG_PICT2,
	ID_BIG_PICT3,
	ID_VIDEO_SCREEN,
	ID_CAT,
	ID_S21_FLOOR,
	ID_FAKE_FLOOR_21,
	0XFFFF,
	ID_TEXT_MOUSE
};

uint16 video_in[] = {
	109*64,
	345,
	259,
	0,
	345,
	259,
	0,
	345,
	259,
	0,
	345,
	259,
	0,
	345,
	259,
	1,
	345,
	259,
	2,
	345,
	259,
	3,
	345,
	259,
	4,
	345,
	259,
	5,
	0
};

Compact lambs_books = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	21,	// screen
	0,	// place
	0,	// getToTable
	202,	// xcood
	240,	// ycood
	0,	// frame
	12669,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	227-202,	// mouseSize_x
	270-240,	// mouseSize_y
	LAMBS_BOOKS_ACTION,	// actionScript
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

uint16 cat_walk_seq[] = {
	101*64,
	250,
	228,
	0,
	250,
	228,
	1,
	250,
	228,
	2,
	251,
	226,
	3,
	254,
	224,
	4,
	254,
	221,
	5,
	254,
	221,
	6,
	254,
	222,
	7,
	254,
	220,
	8,
	253,
	218,
	9,
	253,
	218,
	9,
	253,
	218,
	9,
	253,
	218,
	10,
	253,
	218,
	10,
	253,
	220,
	11,
	253,
	219,
	12,
	254,
	220,
	13,
	254,
	219,
	14,
	254,
	220,
	15,
	254,
	221,
	16,
	254,
	222,
	17,
	254,
	222,
	18,
	254,
	224,
	19,
	254,
	226,
	20,
	255,
	226,
	21,
	255,
	226,
	22,
	255,
	226,
	23,
	254,
	227,
	24,
	256,
	228,
	25,
	256,
	230,
	26,
	264,
	232,
	27,
	278,
	238,
	28,
	289,
	250,
	29,
	292,
	256,
	30,
	293,
	260,
	31,
	296,
	261,
	32,
	297,
	262,
	33,
	299,
	263,
	34,
	300,
	264,
	35,
	302,
	265,
	36,
	305,
	265,
	37,
	307,
	266,
	38,
	309,
	266,
	39,
	314,
	267,
	40,
	316,
	268,
	41,
	317,
	268,
	42,
	320,
	268,
	43,
	322,
	268,
	44,
	324,
	268,
	45,
	327,
	269,
	46,
	329,
	269,
	47,
	331,
	269,
	48,
	334,
	269,
	49,
	338,
	269,
	50,
	340,
	269,
	51,
	344,
	269,
	52,
	347,
	269,
	53,
	0
};

Compact big_pict1 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	21,	// screen
	0,	// place
	0,	// getToTable
	195,	// xcood
	191,	// ycood
	0,	// frame
	12680,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	211-195,	// mouseSize_x
	228-191,	// mouseSize_y
	BIG_PICT1_ACTION,	// actionScript
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

Compact lambs_chair = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	21,	// screen
	0,	// place
	0,	// getToTable
	250,	// xcood
	262,	// ycood
	0,	// frame
	12665,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	271-250,	// mouseSize_x
	293-262,	// mouseSize_y
	LAMBS_CHAIR_ACTION,	// actionScript
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

uint16 put_video_in[] = {
	108*64,
	322,
	218,
	0,
	322,
	218,
	1,
	322,
	218,
	2,
	322,
	218,
	3,
	SEND_SYNC,
	ID_VIDEO,
	1,
	322,
	218,
	4,
	0
};

uint16 watch_film[] = {
	108*64,
	322,
	218,
	4,
	322,
	218,
	5,
	322,
	218,
	6,
	322,
	218,
	7,
	322,
	218,
	0,
	0
};

Compact cat = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_LOGIC+ST_RECREATE+ST_BACKGROUND,	// status
	0,	// sync
	21,	// screen
	0,	// place
	0,	// getToTable
	250,	// xcood
	228,	// ycood
	101*64,	// frame
	195,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	274-250,	// mouseSize_x
	235-228,	// mouseSize_y
	CAT_ACTION,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	0,	// mode
	CAT_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact big_pict3 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	21,	// screen
	0,	// place
	0,	// getToTable
	264,	// xcood
	190,	// ycood
	0,	// frame
	12680,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	295-264,	// mouseSize_x
	213-190,	// mouseSize_y
	BIG_PICT3_ACTION,	// actionScript
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

uint16 cat3_seq[] = {
	105*64,
	347,
	214,
	0,
	347,
	214,
	1,
	347,
	214,
	2,
	347,
	214,
	3,
	347,
	214,
	4,
	347,
	214,
	5,
	347,
	214,
	6,
	347,
	214,
	7,
	347,
	214,
	8,
	347,
	214,
	9,
	347,
	214,
	10,
	347,
	214,
	11,
	347,
	214,
	12,
	347,
	214,
	13,
	347,
	214,
	14,
	347,
	214,
	15,
	347,
	214,
	16,
	347,
	214,
	17,
	347,
	214,
	18,
	347,
	214,
	19,
	347,
	214,
	20,
	347,
	214,
	21,
	347,
	214,
	22,
	347,
	214,
	23,
	347,
	214,
	24,
	347,
	214,
	25,
	347,
	214,
	26,
	347,
	214,
	27,
	347,
	214,
	28,
	0
};

uint16 s21_fast_list[] = {
	12,
	51,
	52,
	53,
	54,
	135,
	193+DISK_9,
	194+DISK_9,
	195+DISK_9,
	219+DISK_9,
	220+DISK_9,
	0
};

uint16 bend_down[] = {
	108*64,
	322,
	218,
	0,
	322,
	218,
	7,
	322,
	218,
	6,
	322,
	218,
	5,
	322,
	218,
	4,
	0
};

uint16 take_video_out[] = {
	108*64,
	322,
	218,
	3,
	SEND_SYNC,
	ID_VIDEO,
	1,
	322,
	218,
	2,
	322,
	218,
	1,
	322,
	218,
	0,
	0
};

Compact big_pict2 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	21,	// screen
	0,	// place
	0,	// getToTable
	227,	// xcood
	188,	// ycood
	0,	// frame
	12680,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	236-227,	// mouseSize_x
	212-188,	// mouseSize_y
	BIG_PICT2_ACTION,	// actionScript
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

uint16 cat_food_seq[] = {
	202*64,
	372,
	263,
	0,
	372,
	263,
	1,
	372,
	263,
	2,
	372,
	263,
	3,
	372,
	263,
	4,
	372,
	263,
	5,
	372,
	263,
	2,
	372,
	263,
	3,
	372,
	263,
	4,
	372,
	263,
	5,
	372,
	263,
	6,
	372,
	263,
	7,
	372,
	263,
	8,
	372,
	263,
	9,
	372,
	263,
	10,
	372,
	263,
	11,
	372,
	263,
	12,
	372,
	263,
	13,
	372,
	263,
	14,
	372,
	263,
	15,
	372,
	263,
	16,
	372,
	263,
	17,
	372,
	263,
	18,
	372,
	263,
	19,
	372,
	263,
	20,
	0
};

Compact left_exit_21 = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	21,	// screen
	0,	// place
	0,	// getToTable
	173,	// xcood
	240,	// ycood
	0,	// frame
	1+T7,	// cursorText
	STD_EXIT_LEFT_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	181-173,	// mouseSize_x
	301-240,	// mouseSize_y
	EL21_ACTION,	// actionScript
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

uint32 *grid21 = 0;

Compact inner_lamb_door = {
	L_SCRIPT,	// logic
	ST_BACKGROUND+ST_LOGIC+ST_RECREATE+ST_MOUSE,	// status
	0,	// sync
	21,	// screen
	0,	// place
	0,	// getToTable
	173,	// xcood
	242,	// ycood
	89*64,	// frame
	181,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	(int16) 65534,	// mouseRel_y
	181-173,	// mouseSize_x
	308-242,	// mouseSize_y
	0,	// actionScript
	0,	// upFlag
	0,	// downFlag
	0,	// getToFlag
	0,	// flag
	0,	// mood
	{ 0, 0, 0 },
	0,	// offset
	C_BASE_MODE,	// mode
	INNER_LAMB_DOOR_LOGIC,	// baseSub
	0,	// baseSub_off
	0
};

Compact fake_floor_21 = {
	0,	// logic
	0,	// status
	0,	// sync
	21,	// screen
	0,	// place
	s22_floor_table,	// getToTable
	128,	// xcood
	136,	// ycood
	0,	// frame
	0,	// cursorText
	0,	// mouseOn
	0,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	320,	// mouseSize_x
	4242,	// mouseSize_y
	FLOOR_ACTION,	// actionScript
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

uint16 cat_eat_seq[] = {
	102*64,
	348,
	269,
	0,
	348,
	269,
	1,
	348,
	269,
	2,
	348,
	269,
	3,
	348,
	269,
	1,
	348,
	269,
	4,
	0
};

uint16 more_cassette_seq[] = {
	107*64,
	217,
	216,
	2,
	217,
	216,
	1,
	217,
	216,
	0,
	0
};

Compact cassette = {
	L_SCRIPT,	// logic
	ST_MOUSE+ST_BACKGROUND+ST_RECREATE,	// status
	0,	// sync
	21,	// screen
	0,	// place
	0,	// getToTable
	216,	// xcood
	230,	// ycood
	110*64,	// frame
	173,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	5,	// mouseSize_x
	8,	// mouseSize_y
	CASSETTE_ACTION,	// actionScript
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

uint16 s21_pal[] = {
	0,
	1280,
	257,
	515,
	2308,
	257,
	775,
	1281,
	1795,
	1031,
	3332,
	258,
	779,
	2308,
	2052,
	1545,
	1541,
	1032,
	785,
	3073,
	1542,
	1801,
	2057,
	3079,
	2061,
	5123,
	773,
	1552,
	4359,
	519,
	2059,
	2317,
	3849,
	3082,
	6149,
	516,
	2062,
	5133,
	1798,
	1809,
	7179,
	516,
	2577,
	3078,
	2061,
	2571,
	6162,
	1542,
	2831,
	5387,
	1546,
	2579,
	3594,
	4364,
	3084,
	6165,
	1035,
	1822,
	4355,
	4875,
	2076,
	8966,
	773,
	3094,
	5385,
	3339,
	3342,
	4632,
	4109,
	3599,
	8212,
	1544,
	2843,
	6155,
	3085,
	1323,
	7169,
	782,
	3604,
	3859,
	6927,
	3857,
	9751,
	1287,
	2339,
	7176,
	3597,
	2089,
	6147,
	2833,
	4370,
	5658,
	4369,
	3360,
	7434,
	1042,
	4636,
	6664,
	3857,
	2348,
	7685,
	3088,
	4129,
	4614,
	7698,
	4887,
	5140,
	3096,
	4633,
	7447,
	4115,
	3623,
	8460,
	3346,
	4386,
	9233,
	2578,
	4639,
	6931,
	5141,
	4133,
	11023,
	3088,
	5406,
	6167,
	4124,
	5157,
	9999,
	4626,
	6174,
	7186,
	6168,
	5665,
	7445,
	7447,
	4906,
	10000,
	3607,
	4910,
	10254,
	4630,
	5670,
	11286,
	4885,
	6434,
	10780,
	5654,
	7965,
	8725,
	6172,
	5679,
	10513,
	5145,
	6948,
	12063,
	5400,
	7210,
	12824,
	4889,
	8993,
	9753,
	8222,
	7214,
	10006,
	6944,
	6706,
	11287,
	7197,
	7216,
	10266,
	8991,
	9763,
	13340,
	6684,
	8491,
	12834,
	6943,
	10022,
	14110,
	6430,
	8501,
	12061,
	8229,
	9518,
	10789,
	8234,
	9016,
	11038,
	9258,
	10290,
	12070,
	10029,
	3616,
	9231,
	2327,
	5154,
	7431,
	4367,
	1319,
	16130,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	16191,
	0,
	5632,
	2316,
	4376,
	6667,
	3857,
	4126,
	5132,
	3096,
	4893,
	8720,
	4369,
	7192,
	9488,
	3860,
	7965,
	8725,
	6172,
	8993,
	9753,
	7719,
	10794,
	12064,
	10029,
	0,
	1280,
	257,
	515,
	2308,
	257,
	775,
	1281,
	1795,
	1031,
	3332,
	258,
	779,
	5124,
	773,
	1552,
	6151,
	1542,
	2076,
	8198,
	1544,
	2843,
	8971,
	2057,
	259,
	1795,
	1795,
	1547,
	3851,
	3593,
	3092,
	6162,
	5647,
	4892,
	8474,
	7703,
	7205,
	10529,
	9504,
	9517,
	12841,
	11818,
	12342,
	14899,
	14134,
	15423,
	61,
	0,
	0,
	14336,
	13364,
	10283,
	7720,
	7196,
	5401,
	5396,
	3855,
	2576,
	1802,
	1285,
	0,
	15616,
	9522,
	10041,
	12059,
	5152,
	5928,
	7948,
	1806,
	2836,
	11267,
	2056,
	8210,
	7441,
	6954,
	9494,
	4119,
	3866,
	4864,
	12548,
	13361,
	10281,
	8236,
	8734,
	10284,
	9264,
	10271,
	5917,
	5664,
	6159,
	2319,
	2064,
	2052,
	13631,
	14376,
	6694,
	6189,
	8975,
	1805,
	1048,
	3586,
	0,
	9533,
	13100,
	9244,
	5162,
	8476,
	5390,
	2072,
	3855,
	2308,
	2865,
	9995,
	1285,
	285,
	4865,
	0,
	13631,
	14376,
	6694,
	0,
	16128,
	0,
	16191,
	16191,
	63,
	16191,
	16128,
	63,
	16191,
	13824,
	13878,
	12077,
	8241,
	10527,
	5917,
	5925,
	7698,
	2865,
	9995,
	1285,
	285,
	16129,
	16191
};

Compact dispensor = {
	0,	// logic
	ST_MOUSE,	// status
	0,	// sync
	21,	// screen
	0,	// place
	0,	// getToTable
	387,	// xcood
	228,	// ycood
	0,	// frame
	12668,	// cursorText
	STD_ON,	// mouseOn
	STD_OFF,	// mouseOff
	ADVISOR_188,	// mouseClick
	0,	// mouseRel_x
	0,	// mouseRel_y
	398-387,	// mouseSize_x
	280-228,	// mouseSize_y
	DISPENSOR_ACTION,	// actionScript
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

uint16 cat2_seq[] = {
	104*64,
	347,
	214,
	0,
	347,
	214,
	1,
	347,
	214,
	2,
	347,
	214,
	3,
	347,
	214,
	4,
	347,
	214,
	5,
	347,
	214,
	6,
	347,
	214,
	7,
	347,
	214,
	8,
	347,
	214,
	9,
	347,
	214,
	10,
	347,
	214,
	11,
	347,
	214,
	12,
	347,
	214,
	13,
	347,
	214,
	14,
	347,
	214,
	15,
	347,
	214,
	16,
	347,
	214,
	17,
	347,
	214,
	18,
	347,
	214,
	19,
	347,
	214,
	20,
	347,
	214,
	21,
	347,
	214,
	22,
	0
};

uint16 take_cassette_seq[] = {
	107*64,
	217,
	216,
	0,
	217,
	216,
	1,
	217,
	216,
	2,
	217,
	216,
	3,
	217,
	216,
	3,
	217,
	216,
	3,
	217,
	216,
	3,
	0
};

} // namespace SkyCompact

} // namespace Sky

#endif
