/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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

#ifndef STATICDATA_H
#define STATICDATA_H

const CharInfo charMap[] = {
	//               Letters
	// ---------------------------------------
	{    'A',   6,   0 }, {    'B',  15,   0 },
	{    'C',  24,   0 }, {    'D',  33,   0 },
	{    'E',  42,   0 }, {    'F',  51,   0 },
	{    'G',  60,   0 }, {    'H',  69,   0 },
	{    'I',  78,   0 }, {    'J',  87,   0 },
	{    'K',  96,   0 }, {    'L', 105,   0 },
	{    'M', 114,   0 }, {    'N', 123,   0 },
	{ '\244', 132,   0 }, { '\245', 132,   0 },	// special Spanish char
	{    'O', 141,   0 }, {    'P', 150,   0 },
	{    'Q', 159,   0 }, {    'R', 168,   0 },
	{    'S', 177,   0 }, {    'T', 186,   0 },
	{    'U', 195,   0 }, {    'V', 204,   0 },
	{    'W', 213,   0 }, {    'X', 222,   0 },
	{    'Y', 231,   0 }, {    'Z', 240,   0 },
	// ---------------------------------------
	{ '\247', 250,   0 }, {    ' ', 250,   0 },
	//               Signs
	// ---------------------------------------
	{    '.',   6,   1 }, {    ',',  15,   1 },
	{    '-',  24,   1 }, {    '?',  33,   1 },
	{ '\250',  42,   1 }, {    '"',  51,   1 },
	{    '!',  60,   1 }, { '\255',  69,   1 },
	{    ';',  78,   1 }, {    '>',  87,   1 },
	{    '<',  96,   1 }, {    '$', 105,   1 },
	{    '%', 114,   1 }, {    ':', 123,   1 },
	{    '&', 132,   1 }, {    '/', 141,   1 },
	{    '(', 150,   1 }, {    ')', 159,   1 },
	{    '*', 168,   1 }, {    '+', 177,   1 },
	{    '1', 186,   1 }, {    '2', 195,   1 },
	{    '3', 204,   1 }, {    '4', 213,   1 },
	{    '5', 222,   1 }, {    '6', 231,   1 },
	{    '7', 240,   1 }, {    '8', 249,   1 },
	{    '9', 258,   1 }, {    '0', 267,   1 },
	//               Accented
	// ---------------------------------------
	{ '\240',   6,   2 }, { '\202',  15,   2 },	// A, B
	{ '\241',  24,   2 }, { '\242',  33,   2 },	// C, D
	{ '\243',  42,   2 }, { '\205',  51,   2 },	// E, F
	{ '\212',  60,   2 }, { '\215',  69,   2 },	// G, H
	{ '\225',  78,   2 }, { '\227',  87,   2 },	// I, J
	{ '\203',  96,   2 }, { '\210', 105,   2 },	// K, L
	{ '\214', 114,   2 }, { '\223', 123,   2 },	// M, N
	{ '\226', 132,   2 }, { '\'',   141,   2 }, // special Spanish char, O
	{ '\200', 150,   2 }, { '\207', 150,   2 },	// P, P
	{ '\265',   6,   2 }, { '\220',  15,   2 },	// A, B
	{ '\326',  24,   2 }, { '\340',  33,   2 },	// C, D
	{ '\351',  42,   2 }, { '\267',  51,   2 },	// E, F
	{ '\324',  60,   2 }, { '\336',  69,   2 },	// G, H
	{ '\343',  78,   2 }, { '\353',  87,   2 },	// I, J
	{ '\266',  96,   2 }, { '\322', 105,   2 },	// K, L
	{ '\327', 114,   2 }, { '\342', 123,   2 },	// M, N
	{ '\352', 132,   2 }						// special Spanish char
};

const ItemLocation itemLocations[] = {
	{   0,   0 },							  // empty
	{   5,  10 }, {  50,  10 }, {  95,  10 }, // 1-3
	{ 140,  10 }, { 185,  10 }, { 230,  10 }, // 4-6
	{ 275,  10 }, {   5,  40 }, {  50,  40 }, // 7-9
	{  95,  40 }, { 140,  40 }, { 185,  40 }, // 10-12
	{ 230,  40 }, { 275,  40 }, {   5,  70 }, // 13-15
	{  50,  70 }, {  95,  70 }, { 140,  70 }, // 16-18
	{ 185,  70 }, { 230,  70 }, { 275,  70 }, // 19-21
	{   5, 100 }, {  50, 100 }, {  95, 100 }, // 22-24
	{ 140, 100 }, { 185, 100 }, { 230, 100 }, // 25-27
	{ 275, 100 }, {   5, 130 }, {  50, 130 }, // 28-30
	{  95, 130 }, { 140, 130 }, { 185, 130 }, // 31-33
	{ 230, 130 }, { 275, 130 }, {   5, 160 }, // 34-36
	{  50, 160 }, {  95, 160 }, { 140, 160 }, // 37-39
	{ 185, 160 }, { 230, 160 }, { 275, 160 }, // 40-42
	{ 275, 160 }							  // 43
};

const int x_pol[44]  = {0, 1, 42, 83, 124, 165, 206, 247, 83, 1, 206,
						1, 42, 83, 124, 165, 206, 247, 83, 1, 206,
						247, 83, 165, 1, 206, 42, 124, 83, 1, 247,
						83, 165, 1, 206, 42, 124, 83, 1, 247, 42,
						1, 165, 206};
const int y_pol[44]  = {0, 1, 1, 1, 1, 1, 1, 1, 27, 27, 1,
						27, 27, 27, 27, 27, 27, 27, 1, 1, 27,
						1, 1, 1, 1, 1, 27, 27, 27, 27, 27,
						1, 1, 1, 1, 1, 27, 27, 27, 27, 27,
						27, 1, 1};
const int verbBarX[] = {6, 51, 96, 141, 186, 232, 276, 321};
const int x1d_menu[] = {280, 40, 80, 120, 160, 200, 240, 0, 40, 80, 120,
						160, 200, 240, 0, 40, 80, 120, 160, 200, 240, 0,
						40, 80, 120, 160, 200, 240, 0};
const int y1d_menu[] = {0, 0, 0, 0, 0, 0, 0, 25, 25, 25, 25, 25, 25, 25,
						50, 50, 50, 50, 50, 50, 50, 75, 75, 75, 75, 75, 75, 75, 100};
int frameX[20]		 = {43, 87, 130, 173, 216, 259};
int candleX[]		 = {14, 19, 24};
int candleY[]		 = {158, 172, 186};
int pianistX[]		 = {1, 91, 61, 31, 91, 31, 1, 61, 31};
int drunkX[]		 = {1, 42, 83, 124, 165, 206, 247, 1};

const RoomUpdate roomPreUpdates[] = {
	// room		flag	val		x1		y1		x2		y2		width	height	type
	{  3,		 3,		1,		258,	110,	 85,	 44,	 23,	53,		0	},
	//-------------------------------------------------------------------------------
	{  5,		 8,		0,		256,	152,	208,	 67,	 27,	40,		0	},
	//-------------------------------------------------------------------------------
	{  6,		 0,		0,		  3,	103,	185,	 69,	 23,	76,		0	},
	{  6,		 1,		0,		 97,	117,	 34,	148,	 36,	31,		0	},
	{  6,		 2,		0,		 28,	100,	219,	 72,	 64,	97,		0	},
	//-------------------------------------------------------------------------------
	{  7,		35,		0,		  1,	 72,	158,	162,	 19,	12,		0	},
	//-------------------------------------------------------------------------------
	{ 12,		16,		0,		  1,	131,	106,	117,	 55,	68,		0	},
	//-------------------------------------------------------------------------------
	{ 17,		15,		1,		  1,	135,	108,	 65,	 44,	63,		0	},
	//-------------------------------------------------------------------------------
	{ 21,		 0,		1,		  2,	171,	 84,	126,	 17,	26,		0	},
	{ 21,		10,		1,		 20,	163,	257,	149,	 14,	34,		0	},
	//-------------------------------------------------------------------------------
	{ 22,		24,		1,		  2,	187,	107,	106,	 62,	12,		0	},
	{ 22,		27,		0,		 32,	181,	203,	 88,	 13,	 5,		0	},
	{ 22,		26,		0,		  2,	133,	137,	 83,	 29,	53,		0	},
	{ 22,		26,		1,		 65,	174,	109,	145,	 55,	25,		0	},
	//-------------------------------------------------------------------------------
	{ 24,		 1,		1,		  1,	163,	225,	124,	 12,	36,		0	},
	{ 24,		 2,		1,		 14,	153,	 30,	107,	 23,	46,		0	},
	//-------------------------------------------------------------------------------
	{ 26,		 2,		1,		  1,	130,	 87,	 44,	 50,	69,		0	},
	{ 26,		12,		1,		 52,	177,	272,	103,	 27,	22,		0	},
	{ 26,		18,		0,		 80,	133,	199,	 95,	 50,	66,		0	},
	//-------------------------------------------------------------------------------
	{ 27,		 5,		1,		  1,	175,	 59,	109,	 17,	24,		1	},
	{ 27,		 6,		1,		 19,	177,	161,	103,	 18,	22,		1	},
	//-------------------------------------------------------------------------------
	{ 29,		 4,		1,		 12,	113,	247,	 49,	 41,	84,		0	},
	//-------------------------------------------------------------------------------
	{ 30,		 4,		1,		  1,	148,	148,	 66,	 35,	51,		0	},
	{ 30,		16,		1,		 37,	173,	109,	 84,	 20,	26,		0	},
	//-------------------------------------------------------------------------------
	{ 31,		13,		1,		  1,	163,	116,	 41,	 61,	36,		0	},
	{ 31,		 5,		1,		  1,	 78,	245,	 63,	 30,	84,		0	},
	//-------------------------------------------------------------------------------
	{ 34,		 7,		1,		 99,	127,	 73,	 41,	 79,	72,		0	},
	{ 34,		 8,		1,		 36,	129,	153,	 41,	 62,	65,		0	},
	//-------------------------------------------------------------------------------
	{ 35,		14,		1,		  1,	 86,	246,	 65,	 68,	87,		0	},
	{ 35,		17,		1,		 70,	150,	118,	 52,	 40,	23,		0	},
	//-------------------------------------------------------------------------------
	{ 49,		 6,		0,		  2,	136,	176,	 81,	 49,	62,		0	},
	//-------------------------------------------------------------------------------
	{ 53,		 1,		0,		  2,	113,	205,	 50,	 38,	86,		1	},
	{ 53,		 2,		0,		 41,	159,	 27,	117,	 25,	40,		0	},
	{ 53,		 9,		1,		 67,	184,	 56,	 93,	 32,	15,		0	},
	//-------------------------------------------------------------------------------
	{ 54,		 5,		1,		168,	156,	187,	111,	  7,	11,		0	},
	{ 54,		12,		1,		 16,	156,	190,	 64,	 18,	24,		0	},
	//-------------------------------------------------------------------------------
	{ 56,		10,		0,		  2,	126,	 42,	 67,	 57,	67,		0	},
	{ 56,		11,		1,		 60,	160,	128,	 97,	103,	38,		0	},
	//-------------------------------------------------------------------------------
	{ 58,		 0,		0,		  1,	156,	143,	120,	120,	43,		0	},
	{ 58,		 1,		2,		252,	171,	173,	116,	 25,	28,		1	},
	//-------------------------------------------------------------------------------
	{ 59,		 4,		0,		  1,	146,	 65,	106,	 83,	40,		1	}
};

const RoomUpdate roomUpdates[] = {
	// room		flag	val		x1		y1		x2		y2		width	height	type
	{  5,		 -1,	-1,		114,	130,	211,	 87,	109,	 69,	1	},
	{ 15,		 -1,	-1,		  1,	154,	 83,	122,	131,	 44,	1	},
	{ 17,		 -1,	-1,		 48,	135,	 78,	139,	 80,	 30,	1	},
	{ 18,		 24,	 1,		177,	  1,	 69,	 29,	142,	130,	1	},
	{ 18,		 -1,	-1,		105,	132,	109,	108,	196,	 65,	1	},
	{ 20,		 -1,	-1,		  1,	137,	106,	121,	213,	 61,	1	},
	{ 27,		 -1,	-1,		 38,	177,	103,	171,	 21,	 22,	1	},
	{ 27,		 -1,	-1,		 60,	162,	228,	156,	 18,	 37,	1	},
	{ 29,		 -1,	-1,		  1,	180,	150,	126,	 10,	 17,	1	},
	{ 31,		 -1,	-1,		 63,	190,	223,	157,	 17,	  9,	1	},
	{ 34,		 -1,	-1,		  5,	171,	234,	126,	 29,	 23,	1	},
	{ 35,		 -1,	-1,		  1,	174,	 54,	152,	195,	 25,	1	},
	{ 50,		 -1,	-1,		  4,	153,	118,	 95,	 67,	 44,	1	},
	{ 57,		 -1,	-1,		  7,	113,	166,	 61,	 62,	 82,	1	},
	{ 61,		 -1,	-1,		  1,	154,	 83,	122,	131,	 44,	1	},
	{ 63,		 -1,	-1,		  1,	154,	 83,	122,	131,	 44,	1	},
};

// Note: default action needs to be LAST for each group
// of actions with the same number
RoomTalkAction roomActions[] = {
	//room	num	action			object	speech
	{ 0,	1, 	kVerbLook,		-1,		 54 },
	{ 0,	1,	kVerbMove,		-1,		 19 },
	{ 0,	1,	kVerbPick,		-1,		 11 },
	{ 0,	1,	kVerbOpen,		-1,		  9 },
	{ 0,	1,	kVerbClose,		-1,		  9 },
	{ 0,	1,	kVerbTalk,		-1,		 16 },
	{ 0,	1,	kVerbDefault,	-1,		 11 },
	// ----------------------------------
	{ 0,	2,	kVerbMove,		-1,		 19 },
	{ 0,	2,	kVerbOpen,		-1,		  9 },
	{ 0,	2,	kVerbClose,		-1,		  9 },
	{ 0,	2,	kVerbTalk,		-1,		 16 },
	// ----------------------------------
	{ 0,	3,	kVerbLook,		-1,		316 },
	{ 0,	3,	kVerbMove,		-1,		317 },
	{ 0,	3,	kVerbPick,		-1,		318 },
	{ 0,	3,	kVerbOpen,		-1,		319 },
	{ 0,	3,	kVerbClose,		-1,		319 },
	{ 0,	3,	kVerbTalk,		-1,		320 },
	{ 0,	3,	kVerbDefault,	-1,		318 },
	// ----------------------------------
	{ 0,	4,	kVerbMove,		-1,		 19 },
	{ 0,	4,	kVerbOpen,		-1,		  9 },
	{ 0,	4,	kVerbClose,		-1,		  9 },
	{ 0,	4,	kVerbTalk,		-1,		 16 },
	// ----------------------------------
	{ 0,	5,	kVerbOpen,		-1,		  9 },
	{ 0,	5,	kVerbClose,		-1,		  9 },
	{ 0,	5,	kVerbTalk,		-1,		 16 },
	// ----------------------------------
	{ 0,	6,	kVerbMove,		-1,		 19 },
	{ 0,	6,	kVerbOpen,		-1,		  9 },
	{ 0,	6,	kVerbClose,		-1,		  9 },
	{ 0,	6,	kVerbTalk,		-1,		 16 },
	// ==================================
	{ 1,	-1,	kVerbPick,		118,	  5 },
	{ 1,	-1,	kVerbOpen,		118,	  3 },
	{ 1,	-1,	kVerbClose,		118,	  4 },
	{ 1,	-1,	kVerbTalk,		118,	  6 },
	// ----------------------------------
	{ 1,	-1,	kVerbLook,		119,	  8 },
	{ 1,	-1,	kVerbMove,		119,	 13 },
	{ 1,	-1,	kVerbClose,		119,	 10 },
	{ 1,	-1,	kVerbTalk,		119,	 12 },
	// ----------------------------------
	{ 1,	-1,	kVerbMove,		120,	 13 },
	{ 1,	-1,	kVerbOpen,		120,	 18 },
	{ 1,	-1,	kVerbTalk,		120,	 15 },
	// ==================================
	{ 3,	-1,	kVerbLook,		129,	 21 },
	{ 3,	-1,	kVerbPick,		129,	  5 },
	{ 3,	-1,	kVerbMove,		129,	 24 },
	{ 3,	-1,	kVerbOpen,		129,	 22 },
	{ 3,	-1,	kVerbClose,		129,	 10 },
	// ----------------------------------
	{ 3,	-1,	kVerbLook,		131,	 27 },
	{ 3,	-1,	kVerbPick,		131,	  5 },
	{ 3,	-1,	kVerbMove,		131,	 24 },
	{ 3,	-1,	kVerbOpen,		131,	 22 },
	{ 3,	-1,	kVerbClose,		131,	 10 },
	{ 3,	-1,	kVerbTalk,		131,	 23 },
	// ----------------------------------
	{ 3,	-1,	kVerbLook,		132,	 28 },
	{ 3,	-1,	kVerbPick,		132,	  5 },
	{ 3,	-1,	kVerbMove,		132,	 24 },
	{ 3,	-1,	kVerbOpen,		132,	 22 },
	{ 3,	-1,	kVerbClose,		132,	 10 },
	{ 3,	-1,	kVerbTalk,		132,	 23 },
	// ----------------------------------
	{ 3,	-1,	kVerbLook,		133,	321 },
	{ 3,	-1,	kVerbPick,		133,	 31 },
	{ 3,	-1,	kVerbMove,		133,	 34 },
	{ 3,	-1,	kVerbOpen,		133,	 30 },
	{ 3,	-1,	kVerbClose,		133,	 10 },
	// ----------------------------------
	{ 3,	-1,	kVerbLook,		166,	 55 },
	{ 3,	-1,	kVerbPick,		166,	  7 },
	// ----------------------------------
	{ 3,	-1,	kVerbLook,		211,	184 },
	// ==================================
	{ 4,	-1,	kVerbLook,		189,	182 },
	// ----------------------------------
	{ 4,	-1,	kVerbLook,		207,	175 },
	{ 4,	-1,	kVerbTalk,		207,	176 },
	// ----------------------------------
	{ 4,	-1,	kVerbLook,		208,	177 },
	// ----------------------------------
	{ 4,	-1,	kVerbLook,		209,	179 },
	// ----------------------------------
	{ 4,	-1,	kVerbLook,		210,	180 },
	{ 4,	-1,	kVerbOpen,		210,	181 },
	// ==================================
	{ 5,	-1,	kVerbMove,		136,	 13 },
	{ 5,	-1,	kVerbOpen,		136,	 18 },
	{ 5,	-1,	kVerbTalk,		136,	 15 },
	// ----------------------------------
	{ 5,	-1,	kVerbLook,		212,	187 },
	{ 5,	-1,	kVerbTalk,		212,	188 },
	// ----------------------------------
	{ 5,	-1,	kVerbLook,		213,	189 },
	{ 5,	-1,	kVerbOpen,		213,	190 },
	// ==================================
	{ 6,	-1,	kVerbPick,		144,	 43 },
	// ----------------------------------
	{ 6,	-1,	kVerbLook,		138,	 35 },
	{ 6,	-1,	kVerbTalk,		138,	  6 },
	// ----------------------------------
	{ 6,	-1,	kVerbLook,		143,	 37 },
	{ 6,	-1,	kVerbPick,		143,	  7 },
	{ 6,	-1,	kVerbMove,		143,	  7 },
	{ 6,	-1,	kVerbTalk,		143,	 38 },
	// ----------------------------------
	{ 6,	-1,	kVerbLook,		139,	 36 },
	// ----------------------------------
	{ 6,	-1,	kVerbLook,		140,	147 },
	// ==================================
	{ 7,	-1,	kVerbLook,		164,	 35 },
	// ----------------------------------
	{ 7,	-1,	kVerbLook,		169,	 44 },
	// ==================================
	{ 9,	-1,	kVerbLook,		150,	 35 },
	{ 9,	-1,	kVerbTalk,		150,	  6 },
	// ----------------------------------
	{ 9,	-1,	kVerbLook,		 51,	 60 },
	// ==================================
	{ 12,	-1,	kVerbLook,		154,	329 },
	{ 12,	-1,	kVerbTalk,		154,	330 },
	// ----------------------------------
	{ 12,	-1,	kVerbMove,		155,	 48 },
	{ 12,	-1,	kVerbTalk,		155,	331 },
	// ----------------------------------
	{ 12,	-1,	kVerbLook,		156,	 35 },
	{ 12,	-1,	kVerbMove,		156,	 48 },
	{ 12,	-1,	kVerbTalk,		156,	 50 },
	// ==================================
	{ 14,	-1,	kVerbLook,		200,	165 },
	// ----------------------------------
	{ 14,	-1,	kVerbLook,		201,	166 },
	// ----------------------------------
	{ 14,	-1,	kVerbLook,		202,	167 },
	// ----------------------------------
	{ 14,	-1,	kVerbLook,		203,	168 },
	{ 14,	-1,	kVerbPick,		203,	170 },
	{ 14,	-1,	kVerbMove,		203,	170 },
	{ 14,	-1,	kVerbTalk,		203,	169 },
	// ----------------------------------
	{ 14,	-1,	kVerbLook,		204,	171 },
	// ==================================
	{ 15,	-1,	kVerbTalk,		188,	333 },
	{ 15,	-1,	kVerbLook,		188,	334 },
	// ----------------------------------
	{ 15,	-1,	kVerbLook,		205,	172 },
	// ----------------------------------
	{ 15,	-1,	kVerbLook,		206,	173 },
	{ 15,	-1,	kVerbMove,		206,	174 },
	{ 15,	-1,	kVerbOpen,		206,	174 },
	// ==================================
	{ 16,	-1,	kVerbTalk,		163,	  6 },
	// ----------------------------------
	{ 16,	-1,	kVerbLook,		183,	340 },
	// ----------------------------------
	{ 16,	-1,	kVerbLook,		185,	 37 },
	{ 16,	-1,	kVerbPick,		185,	  7 },
	{ 16,	-1,	kVerbMove,		185,	  7 },
	{ 16,	-1,	kVerbTalk,		185,	 38 },
	// ----------------------------------
	{ 16,	-1,	kVerbTalk,		187,	345 },
	// ==================================
	{ 18,	-1,	kVerbLook,		181,	348 },
	// ----------------------------------
	{ 18,	-1,	kVerbLook,		182,	154 },
	// ==================================
	{ 19,	-1,	kVerbLook,		214,	191 },
	// ==================================
	{ 22,	-1,	kVerbPick,		140,	  7 },
	// ----------------------------------
	{ 22,	-1,	kVerbLook,		 52,	497 },
	{ 22,	-1,	kVerbTalk,		 52,	498 },
	// ==================================
	{ 24,	-1,	kVerbLook,		151,	461 },
	// ==================================
	{ 26,	-1,	kVerbOpen,		167,	467 },
	// ----------------------------------
	{ 26,	-1,	kVerbLook,		164,	470 },
	{ 26,	-1,	kVerbOpen,		164,	471 },
	// ----------------------------------
	{ 26,	-1,	kVerbLook,		163,	472 },
	{ 26,	-1,	kVerbPick,		163,	473 },
	// ----------------------------------
	{ 26,	-1,	kVerbLook,		165,	474 },
	// ----------------------------------
	{ 26,	-1,	kVerbLook,		168,	476 },
	{ 26,	-1,	kVerbPick,		168,	477 },
	// ==================================
	{ 27,	-1,	kVerbLook,		175,	429 },
	// ==================================
	{ 29,	-1,	kVerbLook,		152,	463 },
	{ 29,	-1,	kVerbOpen,		152,	464 },
	// ----------------------------------
	{ 29,	-1,	kVerbLook,		153,	465 },
	// ----------------------------------
	{ 29,	-1,	kVerbPick,		154,	466 },
	// ----------------------------------
	{ 29,	-1,	kVerbOpen,		156,	467 },
	// ==================================
	{ 30,	-1,	kVerbOpen,		157,	468 },
	// ----------------------------------
	{ 30,	-1,	kVerbLook,		158,	469 },
	// ==================================
	{ 31,	-1,	kVerbLook,		161,	470 },
	{ 31,	-1,	kVerbOpen,		161,	471 },
	// ==================================
	{ 34,	-1,	kVerbLook,		146,	458 },
	{ 34,	-1,	kVerbPick,		146,	459 },
	// ==================================
	{ 44,	-1,	kVerbLook,		172,	428 },
	// ==================================
	{ 49,	-1,	kVerbLook,		 51,	132 },
	// ----------------------------------
	{ 49,	-1,	kVerbLook,		200,	133 },
	{ 49,	-1,	kVerbTalk,		200,	134 },
	// ----------------------------------
	{ 49,	-1,	kVerbLook,		201,	135 },
	// ----------------------------------
	{ 49,	-1,	kVerbLook,		203,	137 },
	// ==================================
	{ 53,	-1,	kVerbLook,		121,	128 },
	// ----------------------------------
	{ 53,	-1,	kVerbLook,		209,	129 },
	// ----------------------------------
	{ 53,	-1,	kVerbLook,		 52,	447 },
	{ 53,	-1,	kVerbTalk,		 52,	131 },
	// ==================================
	{ 54,	-1,	kVerbLook,		 53,	127 },
	// ----------------------------------
	{ 54,	-1,	kVerbOpen,		119,	125 },
	{ 54,	-1,	kVerbLook,		119,	126 },
	// ==================================
	{ 55,	-1,	kVerbLook,		122,	138 },
	// ----------------------------------
	{ 55,	-1,	kVerbLook,		204,	139 },
	// ----------------------------------
	{ 55,	-1,	kVerbLook,		205,	140 },
	// ==================================
	{ 56,	-1,	kVerbLook,		124,	450 },
	// ----------------------------------
	{ 56,	-1,	kVerbOpen,		207,	141 },
	// ----------------------------------
	{ 56,	-1,	kVerbLook,		208,	142 },
	// ==================================
	{ 58,	-1,	kVerbLook,		104,	454 },
	// ==================================
	{ 60,	-1,	kVerbLook,		112,	440 },
	// ----------------------------------
	{ 60,	-1,	kVerbTalk,		115,	455 },
	// ----------------------------------
	{ 60,	-1,	kVerbTalk,		 56,	455 },
	// ----------------------------------
	{ 60,	-1,	kVerbLook,		114,	167 },
	// ----------------------------------
	{ 60,	-1,	kVerbLook,		113,	168 },
	{ 60,	-1,	kVerbPick,		113,	170 },
	{ 60,	-1,	kVerbMove,		113,	170 },
	{ 60,	-1,	kVerbTalk,		113,	169 },
	// ==================================
	{ 61,	-1,	kVerbLook,		116,	172 },
	// ----------------------------------
	{ 61,	-1,	kVerbLook,		117,	173 },
	// ----------------------------------
	{ 61,	-1,	kVerbMove,		117,	174 },
	{ 61,	-1,	kVerbOpen,		117,	174 },
	{ 62,	-1,	kVerbLook,		100,	168 },
	{ 62,	-1,	kVerbTalk,		100,	169 },
	{ 62,	-1,	kVerbPick,		100,	170 },
	// ----------------------------------
	{ 62,	-1,	kVerbLook,		101,	171 },
	// ----------------------------------
	{ 62,	-1,	kVerbLook,		102,	167 },
	// ----------------------------------
	{ 62,	-1,	kVerbLook,		103,	166 },
	// ==================================
	{ 63,	-1,	kVerbLook,		110,	172 },
	// ----------------------------------
	{ 63,	-1,	kVerbLook,		109,	173 },
	{ 63,	-1,	kVerbMove,		109,	174 },
	// ----------------------------------
	{ 63,	-1,	kVerbLook,		108,	334 },
	{ 63,	-1,	kVerbTalk,		108,	333 },
	// ==================================
	{ 102,	-1,	kVerbLook,		100,	452 },
	{ 102,	-1,	kVerbLook,		101,	123 },
	// ==================================
	// Pseudoroom. checkAction() menuScreen == 1
	{ 200,	1,	kVerbLook,		28,		328 },
	// ----------------------------------
	{ 200,	2,	kVerbLook,		28,		328 },
	{ 200,	2,	kVerbLook,		7,		143 },
	{ 200,	2,	kVerbLook,		8,		145 },
	{ 200,	2,	kVerbLook,		9,		147 },
	{ 200,	2,	kVerbLook,		10,		151 },
	{ 200,	2,	kVerbLook,		11,		152 },
	{ 200,	2,	kVerbLook,		12,		154 },
	{ 200,	2,	kVerbLook,		13,		155 },
	{ 200,	2,	kVerbLook,		14,		157 },
	{ 200,	2,	kVerbLook,		15,		58 },
	{ 200,	2,	kVerbLook,		16,		158 },
	{ 200,	2,	kVerbLook,		17,		159 },
	{ 200,	2,	kVerbLook,		18,		160 },
	{ 200,	2,	kVerbLook,		19,		161 },
	{ 200,	2,	kVerbLook,		23,		152 },
	{ 200,	2,	kVerbTalk,		7,		144 },
	{ 200,	2,	kVerbTalk,		8,		146 },
	{ 200,	2,	kVerbTalk,		9,		148 },
	{ 200,	2,	kVerbTalk,		11,		153 },
	// ----------------------------------
	{ 200,	3,	kVerbLook,		22,		307 },
	{ 200,	3,	kVerbLook,		28,		328 },
	{ 200,	3,	kVerbLook,		7,		143 },
	{ 200,	3,	kVerbLook,		8,		145 },
	{ 200,	3,	kVerbLook,		9,		147 },
	{ 200,	3,	kVerbLook,		10,		151 },
	{ 200,	3,	kVerbLook,		11,		152 },
	{ 200,	3,	kVerbLook,		12,		154 },
	{ 200,	3,	kVerbLook,		13,		155 },
	{ 200,	3,	kVerbLook,		14,		157 },
	{ 200,	3,	kVerbLook,		15,		58 },
	{ 200,	3,	kVerbLook,		16,		158 },
	{ 200,	3,	kVerbLook,		17,		159 },
	{ 200,	3,	kVerbLook,		18,		160 },
	{ 200,	3,	kVerbLook,		19,		161 },
	{ 200,	3,	kVerbLook,		20,		162 },
	{ 200,	3,	kVerbLook,		23,		152 },
	{ 200,	3,	kVerbTalk,		7,		144 },
	{ 200,	3,	kVerbTalk,		8,		146 },
	{ 200,	3,	kVerbTalk,		9,		148 },
	{ 200,	3,	kVerbTalk,		11,		153 },
	// ----------------------------------
	{ 200,	4,	kVerbLook,		7,		478 },
	{ 200,	4,	kVerbLook,		8,		480 },
	{ 200,	4,	kVerbLook,		10,		485 },
	{ 200,	4,	kVerbLook,		11,		488 },
	{ 200,	4,	kVerbLook,		12,		486 },
	{ 200,	4,	kVerbLook,		13,		490 },
	{ 200,	4,	kVerbLook,		14,		122 },
	{ 200,	4,	kVerbLook,		15,		117 },
	{ 200,	4,	kVerbLook,		16,		491 },
	{ 200,	4,	kVerbLook,		17,		478 },
	{ 200,	4,	kVerbLook,		18,		493 },
	{ 200,	4,	kVerbLook,		20,		162 },
	{ 200,	4,	kVerbLook,		21,		496 },
	{ 200,	4,	kVerbLook,		22,		161 },
	{ 200,	4,	kVerbLook,		28,		328 },
	{ 200,	4,	kVerbTalk,		15,		118 },
	{ 200,	4,	kVerbOpen,		15,		119 },
	{ 200,	4,		   14,		19,		484 },
	{ 200,	4,		   19,		14,		484 },
	// ----------------------------------
	{ 200,	5,	kVerbLook,		7,		478 },
	{ 200,	5,	kVerbLook,		8,		120 },
	{ 200,	5,	kVerbLook,		11,		488 },
	{ 200,	5,	kVerbLook,		13,		490 },
	{ 200,	5,	kVerbLook,		14,		121 },
	{ 200,	5,	kVerbLook,		15,		117 },
	{ 200,	5,	kVerbLook,		17,		478 },
	{ 200,	5,	kVerbLook,		20,		162 },
	{ 200,	5,	kVerbLook,		28,		328 },
	{ 200,	5,	kVerbTalk,		15,		118 },
	{ 200,	5,	kVerbOpen,		15,		119 },
	// ----------------------------------
	{ 200,	6,	kVerbLook,		20,		123 },
	{ 200,	6,	kVerbLook,		21,		441 },
	{ 200,	6,	kVerbLook,		28,		328 },
	// ==================================
	// Pseudoroom. checkAction() menuScreen != 1
	{ 201,	1,	kVerbLook,		50,		308 },
	{ 201,	1,	kVerbOpen,		50,		310 },
	{ 201,	1,	kVerbClose,		50,		311 },
	{ 201,	1,	kVerbMove,		50,		312 },
	{ 201,	1,	kVerbPick,		50,		313 },
	{ 201,	1,	kVerbTalk,		50,		314 },
	// ----------------------------------
	{ 201,	2,	kVerbLook,		50,		308 },
	{ 201,	2,	kVerbOpen,		50,		310 },
	{ 201,	2,	kVerbClose,		50,		311 },
	{ 201,	2,	kVerbMove,		50,		312 },
	{ 201,	2,	kVerbPick,		50,		313 },
	{ 201,	2,	kVerbTalk,		50,		314 },
	{ 201,	2,		   13,		50,		156 },
	{ 201,	2,		   20,		50,		163 },
	// ----------------------------------
	{ 201,	3,	kVerbLook,		50,		309 },
	{ 201,	3,	kVerbOpen,		50,		310 },
	{ 201,	3,	kVerbClose,		50,		311 },
	{ 201,	3,	kVerbMove,		50,		312 },
	{ 201,	3,	kVerbPick,		50,		313 },
	{ 201,	3,	kVerbTalk,		50,		314 },
	// ----------------------------------
	{ 201,	4,	kVerbLook,		50,		309 },
	{ 201,	4,	kVerbOpen,		50,		310 },
	{ 201,	4,	kVerbClose,		50,		311 },
	{ 201,	4,	kVerbMove,		50,		312 },
	{ 201,	4,	kVerbPick,		50,		313 },
	{ 201,	4,	kVerbTalk,		50,		314 },
	{ 201,	4,		    9,		50,		484 },
	{ 201,	4,		   20,		50,		487 },
	// ----------------------------------
	{ 201,	5,	kVerbLook,		50,		309 }, // Originally these are with
	{ 201,	5,	kVerbOpen,		50,		310 }, // completely wrong voices
	{ 201,	5,	kVerbClose,		50,		311 },
	{ 201,	5,	kVerbMove,		50,		312 },
	{ 201,	5,	kVerbPick,		50,		313 },
	{ 201,	5,	kVerbTalk,		50,		314 },
	{ 201,	5,		   20,		50,		487 },
	// ----------------------------------
	{ 201,	6,	kVerbOpen,		50,		310 },
	{ 201,	6,	kVerbClose,		50,		311 },
	{ 201,	6,	kVerbMove,		50,		312 },
	{ 201,	6,	kVerbPick,		50,		313 },
	{ 201,	6,	kVerbTalk,		50,		314 },

};

TalkSequenceCommand talkSequences[] = {
	// Chapter, sequence,		 command type, action
	{		 1,	   2,				kTalkerBJ,	   2 },
	{		 1,	   2,		   kTalkerGeneral,	 215 },
	{		 1,	   2,				kTalkerBJ,	   3 },
	{		 1,	   2,		   kTalkerGeneral,	 216 },
	{		 1,	   2,				kTalkerBJ,	   4 },
	{		 1,	   2,				kTalkerBJ,	   5 },
	{		 1,	   2,				kTalkerBJ,	   6 },
	{		 1,	   2,		   kTalkerGeneral,	 217 },
	{		 1,	   2,				kTalkerBJ,	   7 },
	{		 1,	   2,		   kTalkerGeneral,	 218 },
	{		 1,	   2,				kTalkerBJ,	   8 },
	{		 1,	   2,		   kTalkerGeneral,	 219 },
	{		 1,	   2,				kTalkerBJ,	   9 },
	{		 1,	   2,		   kTalkerGeneral,	 220 },
	{		 1,	   2,		   kTalkerGeneral,	 221 },
	{		 1,	   2,				kTalkerBJ,	  10 },
	{		 1,	   2,		   kTalkerGeneral,	 222 },
	//
	{		 1,	   3,		   kTalkerGeneral,	 192 },
	{		 1,	   3,		kTalkerBartender0,	   1 },
	{		 1,	   3,		   kTalkerGeneral,	 193 },
	{		 1,	   3,		kTalkerBartender0,	   2 },
	{		 1,	   3,		   kTalkerGeneral,	 194 },
	{		 1,	   3,		kTalkerBartender0,	   3 },
	{		 1,	   3,		   kTalkerGeneral,	 195 },
	{		 1,	   3,		kTalkerBartender0,	   4 },
	{		 1,	   3,		   kTalkerGeneral,	 196 },
	{		 1,	   3,		kTalkerBartender0,	   5 },
	{		 1,	   3,		kTalkerBartender0,	   6 },
	{		 1,	   3,		   kTalkerGeneral,	 197 },
	{		 1,	   3,		kTalkerBartender0,	   7 },
	{		 1,	   3,		   kTalkerGeneral,	 198 },
	{		 1,	   3,		kTalkerBartender0,	   8 },
	{		 1,	   3,		   kTalkerGeneral,	 199 },
	{		 1,	   3,		kTalkerBartender0,	   9 },
	{		 1,	   3,		   kTalkerGeneral,	 200 },
	{		 1,	   3,		   kTalkerGeneral,	 201 },
	{		 1,	   3,		   kTalkerGeneral,	 202 },
	{		 1,	   3,				 kSetFlag,	   0 },
	//
	{		 1,		  10,		 kTalkerDrunk,	   1 },
	{		 1,		  11,		 kTalkerDrunk,	   2 },
	{		 1,		  12,		 kTalkerDrunk,	   3 },
	//
	{		 2,		   8,	   kTalkerPianist,	   6 },
	{		 2,		   8,	   kTalkerGeneral,	 358 },
	{		 2,		   8,	   kTalkerPianist,	   7 },
	{		 2,		   8,	   kTalkerPianist,	   8 },
	//
	{		 2,		   9,	   kTalkerPianist,	   9 },
	{		 2,		   9,	   kTalkerPianist,	  10 },
	{		 2,		   9,	   kTalkerPianist,	  11 },
	//
	{		 2,		  10,	   kTalkerPianist,	  12 },
	{		 2,		  10,	   kTalkerGeneral,	 361 },
	{		 2,		  10,			   kPause,    40 },
	{		 2,		  10,	   kTalkerPianist,	  13 },
	{		 2,		  10,	   kTalkerGeneral,	 362 },
	{		 2,		  10,	   kTalkerPianist,	  14 },
	{		 2,		  10,	   kTalkerGeneral,	 363 },
	{		 2,		  10,	   kTalkerPianist,	  15 },
	{		 2,		  10,	   kTalkerGeneral,	 364 },
	{		 2,		  10,	   kTalkerPianist,	  16 },
	{		 2,		  10,	   kTalkerGeneral,	 365 },
	//
	{		 2,		  11,	   kTalkerGeneral,	 352 },
	{		 2,		  11,	kTalkerBartender0,	   1 },
	{		 2,		  11,	   kTalkerGeneral,	 353 },
	{		 2,		  11,	kTalkerBartender0,	  17 },
	{		 2,		  11,	   kTalkerGeneral,	 354 },
	{		 2,		  11,	kTalkerBartender0,	  18 },
	{		 2,		  11,	   kTalkerGeneral,	 355 },
	{		 2,		  11,			   kPause,	  40 },
	{		 2,		  11,	kTalkerBartender0,	  82 },
	//
	{		 2,		  13,	   kTalkerGeneral,	 103 },
	{		 2,		  13,	     kTalkerDrunk,	   4 },
	{		 2,		  13,			 kSetFlag,	  12 },
	{		 2,		  13,	   kTalkerGeneral,	 367 },
	{		 2,		  13,	     kTalkerDrunk,	   5 },
	{		 2,		  13,			 kSetFlag,	  12 },
	{		 2,		  13,	   kTalkerGeneral,	 368 },
	{		 2,		  13,	     kTalkerDrunk,	   6 },
	{		 2,		  13,	     kTalkerDrunk,	   7 },
	{		 2,		  13,			 kSetFlag,	  41 },
	{		 2,		  13,			kConverse,	   2 },
	//
	{		 2,		  15,		 kTalkerDrunk,	   8 },
	{		 2,		  15,			   kPause,     7 },
	{		 2,		  15,		 kTalkerDrunk,	   9 },
	{		 2,		  15,		 kTalkerDrunk,	  10 },
	{		 2,		  15,		 kTalkerDrunk,	  11 },
	//
	{		 2,		  17,		 kTalkerDrunk,	  13 },
	{		 2,		  17,		 kTalkerDrunk,	  14 },
	{		 2,		  17,			 kSetFlag,	  40 },
	{		 2,		  19,	    kTalkerVBDoor,	   5 },
	{		 2,		  21,	    kTalkerVBDoor,	   6 },
	//
	{		 2,		  22,	   kTalkerGeneral,	 374 },
	{		 2,		  22,	kTrackProtagonist,	   2 },
	{		 2,		  22,		  kUpdateRoom,	  -1 },
	{		 2,		  22,		kUpdateScreen,	  -1 },
	{		 2,		  22,		   kPlaySound,	  13 },
	{		 2,		  22,		 kFinishSound,	  -1 },
	{		 2,		  22,	kTrackProtagonist,	   1 },
	{		 2,		  22,		kTalkerVBDoor,	   1 },
	{		 2,		  22,	   kTalkerGeneral,	 375 },
	{		 2,		  22,		kTalkerVBDoor,	   2 },
	{		 2,		  22,	   kTalkerGeneral,	 376 },
	{		 2,		  22,		kTalkerVBDoor,	   3 },
	{		 2,		  22,			 kSetFlag,	  18 },
	//
	{		 2,		  28,	  kTalkerVBNormal,	  27 },
	{		 2,		  28,	  kTalkerVBNormal,	  28 },
	{		 2,		  28,	  kTalkerVBNormal,	  29 },
	{		 2,		  28,	  kTalkerVBNormal,	  30 },
	//
	{		 2,	      29,	  kTalkerVBNormal,	  32 },
	{		 2,	      29,	   kTalkerGeneral,	 398 },
	{		 2,	      29,	  kTalkerVBNormal,	  33 },
	{		 2,	      29,	   kTalkerGeneral,	 399 },
	{		 2,	      29,	  kTalkerVBNormal,	  34 },
	{		 2,	      29,	  kTalkerVBNormal,	  35 },
	{		 2,	      29,	   kTalkerGeneral,	 400 },
	{		 2,	      29,	  kTalkerVBNormal,	  36 },
	{		 2,	      29,	  kTalkerVBNormal,	  37 },
	{		 2,	      29,	   kTalkerGeneral,	 386 },
	{		 2,	      29,	  kTalkerVBNormal,	  38 },
	{		 2,	      29,	  kTalkerVBNormal,	  39 },
	{		 2,	      29,	   kTalkerGeneral,	 401 },
	{		 2,	      29,	  kTalkerVBNormal,	  40 },
	{		 2,	      29,	  kTalkerVBNormal,	  41 },
	{		 2,	      29,			 kSetFlag,	  33 },
	//
	{		 2,	      30,	  kTalkerVBNormal,	  31 },
	{		 2,	      30,	   kTalkerGeneral,	 396 },
	//
	{		 2,	      31,	kTrackProtagonist,	   2 },
	{		 2,	      31,		  kUpdateRoom,	  -1 },
	{		 2,	      31,	    kUpdateScreen,	  -1 },
	{		 2,	      31,			   kPause,	  78 },
	{		 2,	      31,	kTrackProtagonist,	   0 },
	{		 2,	      31,		  kUpdateRoom,	  -1 },
	{		 2,	      31,	    kUpdateScreen,	  -1 },
	{		 2,	      31,			   kPause,	  22 },
	{		 2,	      31,	   kTalkerGeneral,	 406 },
	{		 2,	      31,			 kPlaceVB,	  98 },
	{		 2,	      31,	  kTalkerVBNormal,	  45 },
	{		 2,	      31,	  kTalkerVBNormal,	  46 },
	{		 2,	      31,	  kTalkerVBNormal,	  47 },
	{		 2,	      31,	   kTalkerGeneral,	 407 },
	{		 2,	      31,	  kTalkerVBNormal,	  48 },
	{		 2,	      31,	  kTalkerVBNormal,	  49 },
	{		 2,	      31,	   kTalkerGeneral,	 408 },
	{		 2,	      31,	  kTalkerVBNormal,	  50 },
	{		 2,	      31,	  kTalkerVBNormal,	  51 },
	{		 2,	      31,	   kTalkerGeneral,	 409 },
	{		 2,    	  31,	  kTalkerVBNormal,	  52 },
	{		 2,	      31,	  kTalkerVBNormal,	  53 },
	{		 2,	      31,			   kPause,	  12 },
	{		 2,	      31,	  kTalkerVBNormal,	  54 },
	{		 2,	      31,	  kTalkerVBNormal,	  55 },
	{		 2,	      31,	   kTalkerGeneral,	 410 },
	{		 2,    	  31,	  kTalkerVBNormal,	  56 },
	{		 2,	      31,			kBreakOut,	   1 },
	{		 2,		  31,		   kClearFlag,	  38 },
	{		 2,	      31,		     kSetFlag,	  36 },
	//
	{		 4,		   2,	kTalkerIgorSeated,	  16 },
	{		 4,		   2,	   kTalkerGeneral,	 278 },
	{		 4,		   2,	kTalkerIgorSeated,	  17 },
	{		 4,		   2,	   kTalkerGeneral,	 279 },
	{		 4,		   2,	kTalkerIgorSeated,	  18 },
	{		 4,		   3,	kTalkerIgorSeated,	  19 },
	{		 4,		   3,	kTalkerIgorSeated,	  20 },
	{		 4,		   3,	   kTalkerGeneral,	 281 },
	{		 4,		   4,	   kTalkerGeneral,	 287 },
	{		 4,		   4,	kTalkerIgorSeated,	  21 },
	{		 4,		   4,	   kTalkerGeneral,	 284 },
	{		 4,		   4,	kTalkerIgorSeated,	  22 },
	{		 4,		   4,	   kTalkerGeneral,	 285 },
	{		 4,		   4,	kTalkerIgorSeated,	  23 },
	//
	{		 5,		   2,			kTalkerBJ,	  22 },
	{		 5,		   3,			kTalkerBJ,	  23 },
	{		 5,		   3,		  kPickObject,	  10 },
	{		 5,		   3,			kBreakOut,	   1 },
	//
	{		 5,		   4,		    kSetFlag,	   7 },
	{		 5,		   4,		 kUpdateRoom,	  -1 },
	{		 5,		   4,	   kUpdateScreen,	  -1 },
	{		 5,		   4,	  kTalkerGeneral,	 228 },
	{		 5,		   4,	 kTalkerWerewolf,	   1 },
	{		 5,		   4,	 kTalkerWerewolf,	   2 },
	{		 5,		   4,			  kPause,	  23 },
	{		 5,		   4,	  kTalkerGeneral,	 229 },
	{		 5,		   4,	 kTalkerWerewolf,	   3 },
	{		 5,		   4,	 kTalkerWerewolf,	   4 },
	{		 5,		   4,	  kTalkerGeneral,	 230 },
	{		 5,		   4,	 kTalkerWerewolf,	   5 },
	{		 5,		   4,	  kTalkerGeneral,	 231 },
	{		 5,		   4,	 kTalkerWerewolf,	   6 },
	{		 5,		   4,	 kTalkerWerewolf,	   7 },
	{		 5,		   4,			  kPause,	  33 },
	{		 5,		   4,	  kTalkerGeneral,	 232 },
	{		 5,		   4,	 kTalkerWerewolf,	   8 },
	//
	{		 5,		   6,	  kTalkerWerewolf,	   9 },
	{		 5,		   6,	   kTalkerGeneral,	 234 },
	{		 5,		   7,	  kTalkerWerewolf,	  10 },
	{		 5,		   7,	   kTalkerGeneral,	 236 },
	{		 5,		   7,	  kTalkerWerewolf,	  11 },
	{		 5,		   7,	  kTalkerWerewolf,	  12 },
	{		 5,		   7,	  kTalkerWerewolf,	  13 },
	{		 5,		   7,			   kPause,    34 },
	{		 5,		   7,	  kTalkerWerewolf,	  14 },
	{		 5,		   8,	  kTalkerWerewolf,	  15 },
	{		 5,		   8,	   kTalkerGeneral,	 238 },
	{		 5,		   8,	  kTalkerWerewolf,	  16 },
	{		 5,		  15,		   kTalkerMus,	   4 },
	{		 5,		  15,		   kTalkerMus,	   5 },
	{		 5,		  15,		   kTalkerMus,	   6 },
	{		 5,		  15,	   kTalkerGeneral,	 291 },
	{		 5,		  15,		   kTalkerMus,	   7 },
	{		 5,		  16,		   kTalkerMus,	   8 },
	{		 5,		  17,		   kTalkerMus,	   9 },
	//
	{		 6,		   2,	  kTalkerDrascula,	  24 },
	{		 6,		   3,	  kTalkerDrascula,	  24 },
	{		 6,		   4,	  kTalkerDrascula,	  25 },
	{		 6,		  11,	kTalkerBartender1,	  10 },
	{		 6,		  11,	   kTalkerGeneral,	 268 },
	{		 6,		  11,	kTalkerBartender1,	  11 },
	{		 6,		  12,	kTalkerBartender1,	  12 },
	{		 6,		  12,	   kTalkerGeneral,	 270 },
	{		 6,		  12,	kTalkerBartender1,	  13 },
	{		 6,		  12,	kTalkerBartender1,	  14 },
	{		 6,		  13,	kTalkerBartender1,	  15 },
	{		 6,		  14,	kTalkerBartender1,	  24 },
	{		 6,		  14,		   kAddObject,	  21 },
	{		 6,		  14,			 kSetFlag,	  10 },
	{		 6,		  14,			kBreakOut,	   1 },
	{		 6,		  15,	kTalkerBartender1,	  16 }
};

const char *_text[NUM_LANGS][NUM_TEXT] = {
{
	// 0
	"",
	"THAT'S THE SECOND BIGGEST DOOR I'VE SEEN IN MY LIFE",
	"NOT REALLY",
	"THE CHURCH IS ALL BOARDED UP, IT MUST HAVE BEEN ABANDONED SEVERAL YEARS AGO",
	"I HAVEN'T OPENED IT",
	// 5
	"WHAT SHOULD I DO, SHOULD I PULL IT OFF?",
	"HI THERE DOOR, I'M GOING TO MAKE YOU A DOOR-FRAME",
	"IT'S TOO TOUGH FOR ME",
	"THE WINDOW IS BOARDED UP",
	"I CAN'T",
	// 10
	"YES, THAT'S DONE",
	"WHY?",
	"HI WINDOW, ARE YOU DOING ANYTHING TONIGHT?",
	"NOT WITHOUT PERMISSION FROM THE TOWN HALL",
	"IF ONLY THIS WINDOW WASN'T BOARDED UP...",
	// 15
	"YOO-HOO WINDOW!",
	"HI THERE",
	"LIKE MICROCHOF'S",
	"I CAN'T REACH",
	"IT'S ALRIGHT WHERE IT IS",
	// 20
	"",
	"IT'S A COFFIN IN THE SHAPE OF A CROSS",
	"NO THANKS",
	"HI DEAD MAN. NO, DON'T GET UP FOR MY SAKE",
	"YES, JUST LIKE IN POLTERGEIST",
	// 25
	"",
	"",
	"I'LL BE BACK IN FIFTEEN MINUTES",
	"IT'S FORBIDDEN TO PUT UP POSTERS",
	"IT'S UNCLE EVARISTO'S TOMB",
	// 30
	"IT'S LOCKED",
	"I'VE GOT ONE",
	"YOO HOO, UNCLE EVARISTO!",
	"THERE'S NO POWER",
	"IT'S NOT WELL PARKED",
	// 35
	"IT'S A DOOR",
	"A DRAWER IN THE TABLE",
	"A SUSPICIOUS WARDROBE",
	"HI WARDROBE, HOW ARE YOU?",
	"",
	// 40
	"",
	"IT'S AN ANCIENT CANDELABRUM",
	"IT MUST HAVE BEEN HERE SINCE YULE BRINNER HAD HAIR ON HIS HEAD",
	"NO, IT'S A RELIC",
	"IT'S A NICE ALTARPIECE",
	// 45
	"",
	"HA, HA, HA",
	"",
	"NO",
	"",
	// 50
	"HA, HE, HI, HO, HU, GREAT!",
	"",
	"",
	"",
	"I CAN'T SEE ANYTHING IN PARTICULAR",
	// 55
	"IT'S FERNAN, THE PLANT",
	"IT'S ONE OF THE FENCE SPIKES",
	"HEY! THERE'S A PACKET OF MATCHES UNDER HERE",
	"LOOK! A PACKET OF KLEENEX, AND ONE'S STILL UNUSED",
	"THERE ISN'T ANYTHING ELSE IN THE BUCKET",
	// 60
	"IT'S A BLIND MAN WHO CAN'T SEE",
	"",
	"",
	"",
	"",
	// 65
	"THAT'S A GREAT DEAL OF MONEY",
	"",
	"",
	"",
	"",
	// 70
	"",
	"",
	"",
	"",
	"",
	// 75
	"",
	"",
	"",
	"",
	"",
	// 80
	"",
	"",
	"",
	"",
	"",
	// 85
	"",
	"",
	"",
	"",
	"",
	// 90
	"",
	"",
	"",
	"",
	"",
	// 95
	"",
	"",
	"",
	"",
	"",
	// 100
	"THERE IS NOTHING SPECIAL ABOUT IT",
	"IT'S NOT UNUSUAL",
	"HEY, WHAT'S UP MAN?",
	"HI",
	"NOTHING NEW?",
	// 105
	"HOW IS THE FAMILY?",
	"THAT IS JUST LIKE YOU!",
	"BUT HOW DO I GET THAT?",
	"MY RELIGION DOES NOT ALLOW ME TO",
	"IT'D BE BETTER NOT",
	// 110
	"YEAH, SURE MAN!",
	"NO WAY",
	"IMPOSSIBLE",
	"THIS WILL NOT OPEN",
	"I CAN'T DO IT BY MYSELF",
	// 115
	"I COULD DO IT, BUT I JUST FEEL A LITTLE LAZY",
	"I DO NOT SEE THE REASON",
	"IT'S QUITE A NICE BRAIN",
	"AND SO BRAIN, WHAT ARE YOU UP TONIGHT?",
	"NO, IT MUST BE KEPT SOMEWHERE AWAY FROM THE MUTANT ACTION OF THE ATMOSPHERE",
	// 120
	"IT IS VERY STIFF, JUST LIKE MY BOSS",
	"A VERY SHARP STAKE",
	"YOU FAITHFUL SHARP-POINTED STAKE, FROM THE MOST NOBLE TRANSILVANIAN OAK TREE",
	"DAMN, I HAVE TO CUT MY NAILS!",
	"B.J. IS IN THERE... SHE IS A REALLY HOT CHICK!",
	// 125
	"IT IS FIRMLY LOCKED",
	"\"SAFE AWAY LOCKS LTD.\"",
	"IT IS THE TYPICAL SKELETON YOU FIND IN THE DUNGEONS OF ALL THE GAMES",
	"IT IS COMMONLY USED TO TRANSFER ELECTRICITY TO THE MACHINES CONNECTED TO IT",
	"IT IS HAND MADE BECAUSE THE JAPANESE MAKE THEM POCKET SIZE",
	// 130
	"I HAVE ONLY ONCE SEEN IN MY LIFE ANOTHER THING AS UGLY AS THIS",
	"FORGET IT. I AM NOT GOING TO TELL HIM ANYTHING IN CASE HE GETS MAD",
	"HE SEEMS QUITE RATIONAL",
	"IT IS A PICTURE OF PLATO WRITING HIS LOST DIALOGUE",
	"I AM NOT ONE OF THOSE PEOPLE WHO TALKS TO POSTERS",
	// 135
	"THAT'S A VERY CUTE DESK",
	"IT IS A VAMPIRES HUNTER'S DIPLOMA OFFICIALLY APPROVED BY OXFORD UNIVERSITY",
	"IT'S A DARK NIGHT WITH A FULL MOON",
	"IT SEEMS LIKE THESE SCREWS ARE NOT TIGHTENED PROPERLY",
	"DON'T LOOK NOW, BUT I THINK THAT A HIDDEN CAMERA IS FOCUSING ON ME",
	// 140
	"THAT'S A VERY MODERN STAKE DETECTOR",
	"NO. THE LABORATORY IS ON THE SECOND FLOOR",
	"A NICE BEDSIDE TABLE",
	"IT'S A LOT OF MONEY THAT CAN'T BE MISSING IN ANY VALUABLE ADVENTURE",
	"IF I WERE A RICH MAN, DUBIDUBIDUBIDUBIDUBIDUBIDUBIDU",
	// 145
	"THOSE ARE STRANGE LEAVES. THEY MUST HAVE BROUGHT THEM FROM SOUTH AMERICA OR AROUND THERE",
	"I DON'T THINK THEY WOULD ANSWER ME",
	"THAT'S A BEAUTIFUL WOODEN CRUCIFIX. THE ICON DOESN'T REALLY SHOW THE FULL EXTENT OF IT'S BEAUTY",
	"I ONLY PRAY BEFORE I GO TO BED",
	"HEY, THIS SPIKE SEEMS A LITTLE BIT LOOSE!",
	// 150
	"I HOPE YOU WON'T COMPLAIN ABOUT NOT GETTING ANY CLUES FROM ME",
	"IT'S A QUITE CONVENTIONAL SPIKE",
	"THEY ARE CUTE, THOUGH THEY ARE COVERED WITH A LITTLE BIT OF WAX",
	"NO, THEY WON'T HEAR ME. HA,HA,HA THIS IS GREAT!",
	"\"SLEEPING BEAUTY\" FROM TCHAIKOVSKY, OR CHOIFRUSKY, OR WHATEVER IT IS",
	// 155
	"VERY TEMPTING",
	"NO, I DO NOT PUT USED BUBBLE GUM IN MY MOUTH",
	"THAT'S A VERY NICE SICKLE. I WONDER WHERE THE HAMMER IS",
	"TOBACCO MANUFACTURERS WARN ABOUT HEALTH BEING SERIOUSLY DAMAGED BY SANITARY AUTHORITIES",
	"AN ABSOLUTELY NORMAL CANDLE, INCLUDING WAX AND EVERYTHING",
	// 160
	"THESE TWO SHINY COINS REALLY GLITTER!",
	"THIS SHINY COIN REALLY GLITTERS!",
	"WITH THIS I WILL BE IMMUNE AGAINST VAMPIRE BITES",
	"NO, IT'S IS NOT THE RIGHT MOMENT YET",
	"THERE IS A ONE THOUSAND BILL AND A COUPLE OF COINS",
	// 165
	"IT SAYS \"PLEASE, DO NOT THROW FOOD TO THE PIANIST\"",
	"OMELETTE, 1.00. FRIED FISH, 0.80, MAYONNAISE POTATOES, 1.10",
	"BEST BURGERS ON THIS SIDE OF THE DANUBE, ONLY 325!",
	"THAT'S A NICE SKULL WITH A VERY PENETRATING LOOK, HA, HA, HA, HA, THAT WAS GOOD!",
	"HI SKULL, YOU REMIND ME OF UNCLE HAMLET",
	// 170
	"I HAVE THE HABIT OF NOT TOUCHING THINGS THAT HAVE BEEN ALIVE",
	"IT'S A BIN",
	"IT'S A BET FOR TONIGHT'S GAME",
	"I WONDER WHAT IS BEHIND THAT",
	"HEY, THE CURTAIN WILL NOT MOVE!",
	// 175
	"MAN, THIS CASTLE IS REALLY GLOOMY",
	"I CAN'T, HE IS TOO FAR AWAY TO HEAR ME",
	"IT'S A TYPICAL TRANSILVANIAN FOREST, WITH TREES",
	"MAN YOU SAY REALLY STUPID THINGS, THIS ROOM IS TOO DARK!",
	"GARCIA, CANDY STORE. SWEETS AND BUBBLE GUM",
	// 180
	"A VERY NICE DOOR",
	"IT'S CLOSED",
	"A COMPLETELY SEALED BARREL",
	"",
	"AREN'T THESE BUGS REALLY CUTE?",
	// 185
	"PSSST, PUSSYCAT... LITTLE CAT",
	"THERE IS NO ANSWER",
	"THE MOON IS A SATELLITE THAT TURNS AROUND THE EARTH WITH A REVOLUTION PERIOD OF 28 DAYS",
	"HI, LOONY MOON",
	"IT'S TOTALLY BOARDED UP WITH PLANKS",
	// 190
	"IT'S IMPOSSIBLE. NOT EVEN THAT TOUGH GUY FROM TV COULD OPEN THIS",
	"HEY! THE SHADOW OF THAT CYPRESS LOOKS PROLONGED TO ME!",
	"YOU, BARTENDER...!!",
	"I WOULD LIKE TO HAVE A ROOM PLEASE",
	"DO YOU KNOW WHERE I CAN FIND THE MAN CALLED DRASCULA?",
	// 195
	"YES, SO WHAT?",
	"SO?",
	"IS...THAT RIGHT?",
	"GOOD QUESTION. LET ME TELL YOU MY STORY. LOOK...",
	"IT WILL TAKE JUST FIVE MINUTES",
	// 200
	"I'M JOHN HACKER AND I REPRESENT A BRITISH PROPERTY COMPANY",
	"AS FAR AS I KNOW, COUNT DRASCULA WANTS TO BUY SOME PIECES OF LAND IN GIBRALTAR AND MY COMPANY SENT ME HERE TO NEGOTIATE THE SALE",
	"I THINK I'M GOING BACK TO MY MUM'S TOMORROW, FIRST THING IN THE MORNING",
	"BEAUTIFUL NIGHT, HUH?",
	"NO, NOTHING",
	// 205
	"YOU...PIANIST...!!!!",
	"BEAUTIFUL NIGHT",
	"AND IT'S NOT EVEN COLD OR ANYTHING, BY THE WAY, CANT YOU PLAY A DIFFERENT SONG?",
	"ALL RIGHT, I'LL JUST LET YOU GO ON PLAYING",
	"WELL THEN",
	// 210
	"HI BOSS, HOW ARE YOU?",
	"AND HOW IS THE FAMILY?",
	"THIS IS QUITE GROOVY, HUH?",
	"I'D BETTER NOT SAY ANYTHING",
	"THERE IS NO PLACE LIKE HOME. THERE IS NO...WHAT?, BUT YOU ARE NOT AUNTIE EMMA. AS A MATTER OF FACT, I DON'T HAVE AN AUNTIE EMMA!",
	// 215
	"YOU CAN CALL ME ANYTHING YOU WANT, BUT IF YOU CALL ME JOHNNY, I'LL COME TO YOU LIKE A DOG",
	"AREN'T I JUST A FUNNY GUY, HUH?. BY THE WAY, WHERE AM I?",
	"YES",
	"SHOOT...!",
	"OH, SURE...OF COURSE!",
	// 220
	"WELL, THANKS VERY MUCH FOR YOUR HELP. I WON'T BOTHER YOU ANYMORE IF YOU PLEASE TELL ME WHERE THE DOOR IS...",
	"THE KNOCK MUST HAVE AFFECTED MY BRAIN...I CAN'T SEE A THING...",
	"WELL...THAT DOESN'T MATTER. I ALWAYS CARRY A SPARE PAIR",
	"WOW, WHAT A HOT CHICK!!  I DIDN'T NOTICE!, BUT OF COURSE, I WASN'T WEARING MY GLASSES",
	"HEY...",
	// 225
	"AND ALL THIIIISSS???",
	"DON'T WORRY B.J. HONEY, I'LL SAVE YOU FROM FALLING INTO HIS CLUTCHES...",
	"YOU REALLY GOT ME MAD MAN...",
	"AHHH A WEREWOLF!! DIE YOU DAMNED EVIL!",
	"YES, WELL...",
	// 230
	"YES, WELL... I THINK I'LL JUST GO ON MY WAY. EXCUSE ME",
	"WHAT?",
	"TO TELL YOU THE TRUTH...ON SECOND THOUGHTS...I DON'T REALLY THINK SO",
	"AND SO TELL ME YOU ERUDITE PHILOSOPHER, IS THERE ANY RELATIONSHIP CAUSE-AND-EFFECT BETWEEN SILLY AND BILLY?",
	"OK, OK, FORGET IT. I DON'T EVEN KNOW WHY I SAID ANYTHING ABOUT IT",
	// 235
	"WHY ARE YOU PHILOSOPHIZING INSTEAD OF EATING PEOPLE?",
	"HOW COME?",
	"HEY, COULD YOU SAY ALL THAT STUFF ABOUT PRE-EVOLUTIONARY RELATIONSHIPS AGAIN?",
	"YES, MAN. ALL THAT STUFF YOU TOLD ME ABOUT BEFORE. I DIDN'T UNDERSTAND IT, YOU KNOW",
	"NO, I'D RATHER NOT SAY ANYTHING, IN CASE HE GETS ANGRY OR SOMETHING...",
	// 240
	"HELLO?",
	"YES, WHAT'S UP?",
	"WELL, NOW THAT YOU MENTION IT, I'LL TELL YOU THAT...",
	"",
	"BY THE WAY, THIS IS NOT THE CASE, OF COURSE, BUT WHAT COULD HAPPEN IF A VAMPIRE GOT THE RECIPE BY ANY CHANCE?",
	// 245
	"WELL ANYWAY. LISTEN, DOESN'T THIS LOOK LIKE A LOT OF CRAP TO END THE GAME WITH?. WELL, MAYBE NOT",
	"IT'S EMPTY!",
	"WHY DID YOU TAKE MY ONLY LOVE, B.J., AWAY FROM ME?. LIFE HAS NO MEANING FOR ME WITHOUT HER",
	"HER BRAIN?\?!!",
	"TO TELL YOU THE TRUTH, I THINK I HAVE HAD ENOUGH FUN WITH YOUR LITTLE MONSTER ALREADY",
	// 250
	"OH PLEASE, HOLY VIRGIN, DON'T LET ANYTHING WORSE HAPPEN TO ME!!",
	"YOU ARE NOT GOING TO GET YOUR WAY. I'M SURE SUPERMAN WILL COME AND RESCUE ME!",
	"WHAT SORT OF A SHIT GAME IS THIS IN WHICH THE PROTAGONIST DIES!",
	"HEY, WAIT A SECOND!, WHAT ABOUT MY LAST WISH?",
	"HA. HA, I'M NOW IMMUNIZED AGAINST YOU DAMNED EVIL!. THIS CIGARETTE IS AN ANTI-VAMPIRE BREW THAT VON BRAUN GAVE TO ME",
	// 255
	"YES SURE, BUT YOU'LL NEVER GET ME TO GIVE YOU THE RECIPE",
	"APART FROM CREATING TORTURE, I CAN ALSO WITHSTAND IT.",
	"OH, NO PLEASE! I'LL TALK, BUT PLEASE, DON'T DO THAT TO ME!",
	"ALL RIGHT THEN. I TOLD YOU WHAT YOU WANTED TO KNOW. NOW SET B.J. AND ME FREE THEN LEAVE US ALONE!",
	"WHAT ARE YOU DOING HERE B.J.?. WHERE IS DRASCULA?",
	// 260
	"WHAT A MEAN GUY!. JUST BECAUSE HE BELONGS TO NOBILITY HE THINKS HE IS ENTITLED TO SLEEP WITH ANYBODY HE FEELS LIKE",
	"DOWN WITH ARISTOCRATIC DESPOTISM!!",
	"POOR PEOPLE OF THE WORLD FOR EVER..!!",
	"AND I CAN SEE HE HAS CHAINED YOU UP WITH LOCKS AND ALL THAT STUFF, HUH?",
	"WELL, ALL RIGHT. DO YOU HAVE A HAIR PIN OVER THERE?",
	// 265
	"ALL RIGHT, OK, DON'T GET MAD. I'LL THINK OF SOMETHING",
	"YOU...BARTENDER!!",
	"HOW IS THE GAME GOING?",
	"WHO?",
	"CAN'T YOU SEE DRASCULA IS HERE?",
	// 270
	"THEN, LET'S FINISH HIM OFF, RIGHT?",
	"GIVE ME A SCOTCH ON THE ROCKS",
	"NOTHING, I JUST FORGOT WHAT I WAS GOING TO SAY...",
	"EITHER YOU GET ME A SCOTCH ON THE ROCKS OR I'LL PLAY THE PIANO UNTIL THE GAME IS OVER",
	"WHEN IS THE MATCH GOING TO BE OVER?",
	// 275
	"GOOD EVENING",
	"AND SO IGOR, HOW ARE YOU FEELING...A LITTLE HUMPED...?. HA, HA, HA, THAT WAS FUNNY!",
	"WHAT ARE YOU SUPPOSED TO BE DOING?",
	"WELL, NO, I DONT UNDERSTAND TAXES",
	"THEN WEAR GLASSES",
	// 280
	"WHAT IS ALL THAT ABOUT THE SUPERNATURAL ORGY?",
	"OK, OK, STOP IT. I THINK I GET THE PICTURE",
	"COULDN'T YOU TELL ME WHERE DRASCULA IS?",
	"OH...PLEASE...COME ON...!",
	"WHY NOT?",
	// 285
	"OH...DOES HE SLEEP AT NIGHT?",
	"WELL, I HOPE YOU GET LUCKY",
	"I HAVE TO TALK TO HIM...",
	"YOUUU...SKELETONNNN..!!!",
	"GOOD HEAVENS!, IT'S A DEAD MAN TALKING!",
	// 290
	"HOW DID YOU END UP HERE",
	"AND WHY WOULD DRASCULA WANT TO CREATE A MONSTER?",
	"WHAT'S YOUR NAME, MY SKELETON FRIEND?",
	"HEY, DON'T YOU WANT ANYTHING TO EAT?",
	"I BET YOUR STOMACH IS PRETTY EMPTY...HA,HA,HA!",
	// 295
	"I DON'T FEEL LIKE TALKING RIGHT NOW",
	"I HOPE SOMEONE F...(WHISTLE) YOU...,AND YOUR F...(WHISTLE) SON OF (WHISTLE TWICE)",
	"I REALLY LOVED HER. I KNOW SHE WASN'T MUCH OF A WONDER, BUT NOBODY'S PERFECT, RIGHT?",
	"BESIDES. SHE HAD ONE OF THOSE GREAT BODIES THAT YOU NEVER FORGET...",
	"I'LL NEVER BE THE SAME AGAIN. I WILL SHUT MYSELF AWAY IN A MONASTERY, AND WILL LET MY LIFE JUST FLOW...",
	// 300
	"NOTHING WILL GET ME OUT OF THIS MYSTERY BECAUSE...",
	"WHOSE?. WHOSE?",
	"I WANT TO BECOME A PIRATE",
	"I WANT TO BECOME A PROGRAMMER",
	"TELL ME SOMETHING ABOUT PELAYO",
	// 305
	"I'LL JUST GO ON PLAYING, AND FORGET I SAW YOU",
	"WHOSE STUPID IDEA COULD THIS BE?",
	"IT LOOKS LIKE MY GRANDMOTHER'S HANDBAG ",
	"JESUS, AREN'T I JUST REALLY COOL MAN...!",
	"THE MORE I SEE MYSELF, THE MORE I LOVE MYSELF",
	// 310
	"HOW DO I LOCK MYSELF THEN?",
	"I'LL HAVE TO OPEN ME FIRST, RIGHT?",
	"I'M ALL RIGHT WHERE I AM",
	"I GOT ME",
	"HI, MYSELF!",
	// 315
	"I'LL WEAR THEM WHEN THE RIGHT TIME COMES",
	"I DON'T SEE ANYTHING SPECIAL ABOUT IT",
	"IT'S ALL RIGHT WHERE IT IS",
	"AND WHAT FOR?",
	"I CAN'T",
	// 320
	"HI, YOU!",
	"IT'S UNCLE DESIDERIO'S PANTHEON!",
	"YOUUU...UNCLE DESIDERIOOOO!!",
	"NO, I DON'T WANT TO CUT MYSELF AGAIN",
	"AHHH,,,EXCUS....",
	// 325
	"JAMM. AHH...",
	"YES...WOF, WOF",
	"LOOK, THERE'S A PIECE OF BUBBLE GUM STUCK HERE",
	"THIS IS THE MOBILE PHONE I GOT LAST CHRISTMAS",
	"IT'S VERY HIGH!",
	// 330
	"COME OUT TO THE BALCONY MY JULIET!!",
	"YOU ARE THE LIGHT THAT ILLUMINATES MY WAY!",
	"HEY, DOOR!, WHAT'S THE MATTER?",
	"YOUUU, CIGARETTE VENDING MACHINE!",
	"IT'S A CIGARETTE VENDING MACHINE",
	// 335
	"I HAVE ANOTHER COIN INSIDE",
	"NO, I JUST QUIT SMOKING AND DRINKING ALCOHOL",
	"I WILL DEVOTE MYSELF TO WOMEN FROM NOW ON",
	"THIS IS A TRICK! NOTHING CAME OUT!",
	"AT LAST!",
	// 340
	"IT'S JUST A TRUNK",
	"HELLO TRUNK, YOUR NAME IS JUST LIKE MY COUSIN FRANK...",
	"I'VE FOUND B.J.'S HANDBAG!",
	"OH MY GOD! I HAVE NO REFLECTION! I'M A VAMPIRE!",
	"OH...JESUS, IT'S JUST A DRAWING!",
	// 345
	"LITTLE MIRROR, TELL ME, WHO IS THE MOST BEAUTIFUL IN THE WHOLE KINGDOM?",
	"HE WON'T OPEN",
	"ALL RIGHT. I GOT THE EAR-PLUGS IN",
	"IT'S A VAMPIRE'S HUNTER DIPLOMA, OFFICIALLY APPROVED BY OXFORD UNIVERSITY",
	"NOT YET. THERE ARE STILL SOME INGREDIENTS MISSING. IT'S NOT WORTH WAKING HIM UP",
	// 350
	"BUT I DON'T HAVE MONEY",
	"IT'S A BRITISH LAMP",
	"HELP ME OUT HERE BARTENDER!!",
	"A VAMPIRE CAME IN AND TOOK MY GIRLFRIEND AWAY!!",
	"BUT, AREN'T YOU GOING TO HELP ME!!??",
	// 355
	"DEAD?, WHAT DO YOU MEAN DEAD?",
	"AAHH....",
	"A VAMPIRE HAS KIDNAPPED THE GIRL IN ROOM 501",
	"BUT YOU HAVE TO HELP ME OUT!",
	"CAN'T YOU PLAY ONE FROM BLUR?",
	// 360
	"HOW CAN YOU STAY HERE ALL DAY PLAYING THE SAME SONG ALL THE TIME?",
	"AND HOW CAN YOU HEAR ME?",
	"PLEASE, LEND ME THE EAR-PLUGS",
	"COME ON, I'LL GIVE THEM BACK TO YOU RIGHT AWAY",
	"COME ONNN...",
	// 365
	"WELL GOODBYE, I HAVE TO KILL A VAMPIRE",
	"",
	"WHAT LANGUAGE ARE YOU SPEAKING?, TRANSILVANIAN?",
	"WHAT ARE YOU TALKING ABOUT? WHO IS UNCLE DESIDERIO?",
	"BUT, WHAT'S THE PROBLEM WITH DRASCULA?",
	// 370
	"WHO IS THIS GUY NAMED VON BRAUN?",
	"AND WHY DOESN'T HE DO IT?",
	"WHERE CAN I FIND VON BRAUN?",
	"WELL, THANKS AND GOODBYE. HOPE YOU SLEEP IT OFF JUST FINE",
	"WE HAD BETTER CALL FIRST",
	// 375
	"ARE YOU PROFESSOR BRAUN?",
	"AND COULD YOU TELL ME WHERE I CA...?",
	"I DON'T BELIEVE HE IS GANIMEDES THE DWARF",
	"PROFESSOR!!",
	"PLEASE HELP ME!. MY GIRLFRIEND'S LIFE DEPENDS ON YOU!!",
	// 380
	"WELL, ALL RIGHT. I DON'T NEED YOUR HELP",
	"ALL RIGHT. I'M LEAVING",
	"DON'T DE AFRAID. WE WILL BEAT DRASCULA TOGETHER",
	"THEN WHY DON'T YOU HELP ME?",
	"I GOT THEM",
	// 385
	"YES, I GOT THEM!!",
	"ALL RIGHT",
	"AHH....YES",
	"I HAVE COME TO GET INTO THAT CABIN AGAIN",
	"I AM READY TO FACE YOUR TEST",
	// 390
	"ALL RIGHT OLD MAN. I CAME FOR MY MONEY",
	"NO, NOTHING. I WAS JUST LEAVING",
	"SORRY...",
	"WOULD YOU LIKE THIS BOOK?. IT HAS SOME SCORES FROM TCHAIKOVSKY",
	"HOW CAN I KILL A VAMPIRE?",
	// 395
	"HAS ANYBODY TOLD YOU THAT SLEEPING IN A BAD POSITION IS NOT GOOD FOR YOU?",
	"THAT'S WHAT MY MUM ALWAYS TELL ME",
	"WHY DIDN'T DRASCULA KILL YOU?",
	"AND WHAT WAS IT?",
	"GREAT! YOU HAVE AN IMMUNIZING BREW",
	// 400
	"SO?",
	"ALL RIGHT",
	"CAN YOU REPEAT WHAT I NEED FOR THAT BREW",
	"WELL, I'LL RUSH OUT TO GET IT",
	"HEY, WHAT HAPPENED WITH THE PIANIST?",
	// 405
	"I HAVE ALL THE INGREDIENTS OF THAT BREW",
	"JUST A QUESTION. WHAT IS ALL THAT ABOUT THE ALUCSARD ETEREUM?",
	"YES, YES?...",
	"AND WHERE IS THAT CAVERN?",
	"WHAT HAPPENED? DIDN'T THEY HAVE TO GO TO COURT?",
	// 410
	"...BUT... IF I MEET MORE VAMPIRES?",
	"IT'S A VAMPIRE THAT WON'T LET ME COME THROUGH",
	"HE LOOKS LIKE YODA, BUT A LITTLE TALLER",
	"HEY YODA, IF YOU JUST LET ME GO ON MY WAY, I'LL GIVE YOU A PENNY",
	"OK, OK, YOU GET MAD ABOUT NOTHING MAN",
	// 415
	"HAS ANYBODY TOLD YOU THAT YOU LOOK LIKE YODA?",
	"HI VAMPIRE, IT'S A BEAUTIFUL NIGHT, HUH?",
	"ARE YOU A VAMPIRE OR AN OIL PAINTING?",
	"I'D BETTER NOT SAY ANYTHING, IN CASE YOU GET MAD",
	"IT'S LOCKED",
	// 420
	"THE MAGPIE WOULD PECK OUT MY EYES IF I TRIED!",
	"OH MY GOD. IT'S LOCKED...THAT'S SCARY, HUH?",
	"THE HINGES ARE RUSTY",
	"THERE IS ONLY ONE BAG OF FLOUR IN THERE",
	"THAT TOOK AWAY THE RUST",
	// 425
	"I FOUND A PINE STAKE",
	"I'LL TAKE THIS LARGER ONE",
	"WELL, I THINK I CAN GET RID OF THIS STUPID DISGUISE",
	"\"PASSAGE TO TOWERS CLOSED FOR REPAIRS. PLEASE USE THE MAIN ENTRANCE. SORRY FOR THE INCONVENIENCE\"",
	"...HE IS PALE, HE HAS FANGS AND WEARS A TOUPE\220  HE SURE IS DRASCULA!",
	// 430
	"IT'S B.J.! ARE YOU ALL RIGHT B.J.?",
	"YES, I KNOW SHE IS STUPID, BUT I'M SO LONELY",
	"YOU DON'T HAVE A KEY AROUND THERE, DO YOU?",
	"I BET YOU DON'T HAVE A LOCK PICK AROUND!",
	"GIVE ME A HAIR PIN, I'M GOING TO PLAY MCGYVER HERE!",
	// 435
	"DON'T GO ANYWHERE. I'LL BE RIGHT BACK",
	"SHOOT! IT'S BROKEN!",
	"OLEEEE! I EVEN SHAVED DUDE!",
	"YES, DARLING?",
	"HE HAS NOT ARRIVED YET",
	// 440
	"THE PIANIST IS NOT HERE",
	"A TRANSILVANIAN SCOTCH ON THE ROCKS",
	"I DON'T HAVE A ROOM YET",
	"IT LOOKS LIKE HE GOT STUCK IN THE BATH AND DECIDED TO RUN A BAR",
	"HE WAS AS DRUNK AS A SAILOR",
	// 445
	"THAT HAIR...IT REMINDS ME OF SOMEBODY",
	"IT'S A RAW-BONED SKELETON",
	"LOOK! THERE'S MIGUEL BOSE!",
	"HE'S ASLEEP. IT'D BE A SHAME TO WAKE HIM UP",
	"HE'S UGLIER THAN EMILIO DE PAZ",
	// 450
	"A PINE WOOD COFFIN",
	"HE IS GOING TO CUT ME IN LITTLE SLICES. JUST LIKE A SAUSAGE",
	"I DON'T LIKE PENDULAE. I'D PREFER ARTICHOKES",
	"I CAN'T MAKE IT. I'M HANDCUFFED",
	"IT'S OBVIOUSLY A SECRET DOOR",
	// 455
	"THEY IGNORE ME",
	"COME ON..!",
	"WHEN I READ THE SCRIPT IT WAS SUPPOSED TO MOVE, BUT THE BUDGET GOT CRAZY AND THEY COULDN'T AFFORD TO PAY THE GYM, SO I NEVER GOT STRONGER. END OF STORY",
	"IT SEEMS A LITTLE LOOSE ON THE WALL",
	"I DON'T THINK IT IS GOING TO HELP ME ANYWAY. IT'S TOO WET TO LIGHT",
	// 460
	"TO THE WEST WING? NO WAY! NOBODY KNOWS WHAT YOU WILL FIND THERE!!",
	"SHE'S GOT NICE TRANSILVANIAN FEATURES",
	"",
	"IT'S A SHAME THERE ISN'T A ROASTED LAMB IN THERE",
	"LAST TIME I OPENED AN OVEN I BLEW UP THE HOUSE",
	// 465
	"THAT'S THE TRANSILVANIAN FOOTBALL TEAMS BADGE",
	"WHAT FOR? TO PUT IT ON MY HEAD??",
	"I DON'T THINK THESE TOWERS ARE THE OPENING KIND",
	"I DON'T WANT TO KNOW WHAT KIND OF FOOD IS IN THERE!",
	"IT LOOKS IMPRESSIONIST TO ME...",
	// 470
	"THE NIGHT IS FALLING OVER ALL OF US...THAT'S SCARY, ISN'T IT?",
	"IT'S STUCK!",
	"IT'S ELVIS THE KING. YOU DIDN'T IMAGINE THAT WOULD BE HERE, DID YOU!",
	"NO, I ALREADY HAVE ONE AT HOME TO FEED",
	"A SHELF WITH BOOKS AND SOME OTHER THINGS",
	// 475
	"BUT WHO CAN I CALL AT THIS TIME?",
	"\"HOW TO FILL IN TAX RETURN FORMS\". HOW INTERESTING!",
	"I ALREADY HAVE ONE AT HOME. I THINK IT'S A WORLDWIDE BEST SELLER",
	"A COMPLETELY NORMAL KEY",
	"I THINK SHE IS NOT FROM AROUND HERE",
	// 480
	"HEY, THEY'RE FANG-LIKE FRENCH FRIES! I LOVE IT!",
	"I DON'T THINK THIS IS THE RIGHT TIME TO EAT THAT CRAP KNOWING THAT MY GIRLFRIEND HAS BEEN KIDNAPPED BY THE MOST EVIL PERSON EVER ON EARTH",
	"I'M HAVING A GREAT TIME KILLING VAMPIRES WITH THIS THING!",
	"LET'S SEE IF ANOTHER ONE COMES SOON!",
	"NO, IT HAS TO BE WITH A DIRTY AND STINKY VAMPIRE, JUST LIKE THE ONE I KILLED BEFORE",
	// 485
	"THIS IS THE ONE AND ONLY WIG THAT ELVIS USED WHEN HE WENT BALD",
	"IT'S FLOUR, BUT DON'T ASK ME ANY COMMERCIAL NAMES",
	"MAYBE ANOTHER TIME, OK?",
	"THAT'S  A GREAT AXE, IT'S A SHAME THERE IS NO VAMPIRE'S HEAD AROUND HERE, HUH?",
	"NO. I REALLY AM A GOOD PERSON",
	// 490
	"IT'S MARGARET'S THATCHERS DEODORANT...HA, HA, HA...!!",
	"THAT'S A PRETTY CUTE CLOAK",
	"",
	"JUST LIKE ALL THE BRANCHES FROM ANY TREE IN THE WORLD, THERE IS NOTHING SPECIAL.",
	"HEY, THAT'S AMAZING! A ROPE IN THIS TYPE OF ADVENTURE GAME!",
	// 495
	"I WONDER WHAT WE COULD USE IT FOR...",
	"A ROPE TIED TO A BRANCH OR THE OTHER WAY AROUND, HOWEVER YOU WANT TO PUT IT...",
	"IT LOOKS LIKE THIS MAGPIE IS EVIL-MINDED",
	"FORGET IT, I'M NOT SAYING ANYTHING IN CASE HE GETS MAD",
	"SHE LOOKS DEAD, BUT SHE REALLY ISN'T, HUH?",
	// 500
	"NO ANIMAL WAS HARMED DURING THE PRODUCTION OF THIS GAME",
},
{
	// 0
	"",
	"Es la segunda puerta mas grande que he visto en mi vida",
	"Bueno, no",
	"Esta sellada con tablones. La iglesia debe llevar abandonada varios a\244os",
	"Si no la he abierto",
	// 5
	"\250Que hago? \250La arranco?",
	"Hola puerta. Te voy a hacer un marco.",
	"Demasiado para mi.",
	"una ventana sellada con tablones.",
	"No puedo.",
	// 10
	"Ya lo esta.",
	"\250Y pa que?",
	"Hola ventana. \250Tienes algo que hacer esta noche?",
	"No sin el permiso de obras publicas",
	"\255eh! esa ventana tiene solo un tablon...",
	// 15
	"\255Eoooooo! \255Ventana!",
	"Hola tu.",
	"",
	"No alcanzo.",
	"Esta bien donde esta.",
	// 20
	"",
	"Es una tumba en forma de cruz.",
	"No gracias.",
	"Hola muerto. \250quieres gusanitos?",
	"Si hombre. Como en Poltergueist.",
	// 25
	"",
	"",
	"Vuelvo en quince minutos.",
	"Prohibido  pegar carteles.",
	"",
	// 30
	"Esta cerrado con llave.",
	"Ya tengo uno.",
	"",
	"No contesta.",
	"No, esta bien aparcado.",
	// 35
	"Es una puerta.",
	"Un cajon de la mesa.",
	"Un sospechoso armario.",
	"Hola armario. \250Que tal?",
	"",
	// 40
	"",
	"Es un candelabro muy viejo.",
	"Debe de llevar aqui desde que Mazinguer-z era un tornillo.",
	"No. Es una reliquia.",
	"Es un bonito retablo.",
	// 45
	"",
	"Ji, ji, ji.",
	"",
	"No.",
	"",
	// 50
	"Ja, ja, ja. \255que bueno!",
	"",
	"",
	"",
	"No veo nada en especial.",
	// 55
	"Es Fernan, la planta.",
	"Es una de las picas de la verja.",
	"\255Eh! Aqui debajo hay una caja de cerillas.",
	"\255Mira! un paquete de clinex. \255Y hay uno sin usar!",
	"No hay nada mas en el cubo.",
	// 60
	"Es un ciego que no ve.",
	"",
	"",
	"",
	"",
	// 65
	"Es una cuantiosa cantidad de dinero.",
	"",
	"",
	"",
	"",
	// 70
	"",
	"",
	"",
	"",
	"",
	// 75
	"",
	"",
	"",
	"",
	"",
	// 80
	"",
	"",
	"",
	"",
	"",
	// 85
	"",
	"",
	"",
	"",
	"",
	// 90
	"",
	"",
	"",
	"",
	"",
	// 95
	"",
	"",
	"",
	"",
	"",
	// 100
	"NO TIENE NADA DE ESPECIAL",
	"NO ES NADA FUERA DE LO NORMAL",
	"\250QUE PASA TU?",
	"HOLA",
	"\250NADA NUEVO?",
	// 105
	"\250QUE TAL LA FAMILIA?",
	"\255QUE COSAS TIENES!",
	"\255PERO COMO VOY A COGER ESO!",
	"MI RELIGION ME LO PROHIBE",
	"MEJOR NO",
	// 110
	"\255SI HOMBRE!",
	"NI HABLAR DE ESO",
	"IMPOSIBLE",
	"ESTO NO SE ABRE",
	"YO SOLO NO PUEDO",
	// 115
	"SI QUISIERA PODRIA, PERO ES QUE ME DA PEREZA",
	"NO LE VEO MOTIVO APARENTE",
	"ES UN CEREBRO BASTANTE MAJO",
	"Y BIEN, CEREBRO \250QUE PIENSAS HACER ESTA NOCHE?",
	"NO, DEBE CONSERVARSE EN LUGAR CERRADO A LA ACCION MUTANTE DE LA ATMOSFERA",
	// 120
	"ES UN HUESO, COMO MI JEFE",
	"UNA ESTACA MUY AFILADA",
	"FIEL ESTACA PUNTIAGUDAAA, NOBLE ROBLE TRANSILVAAANOO",
	"\255VAYA, ME TENGO QUE CORTAR LAS U\245AS!",
	"\255AHI DENTRO ESTA B.J., Y HAY QUE VER COMO ESTA LA TIA!",
	// 125
	"ESTA CERRADA A CAL Y CANTO",
	"\"CANDADOS CAL Y CANTO S.A.\"",
	"ES EL TIPICO ESQUELETO QUE ESTA EN TODAS LAS MAZMORRAS DE TODOS LOS JUEGOS",
	"SE SUELE USAR PARA COMUNICAR CORRIENTE ELECTRICA A LOS APARATOS A EL ENCHUFADOS",
	"ES TOTALMENTE ARTESANAL, PORQUE LOS JAPONESES LOS HACEN DE BOLSILLO",
	// 130
	"SOLO HE VISTO EN MI VIDA OTRA COSA TAN FEA",
	"QUITA. NO LE DIGO NADA A VER SI SE VA A ENFADAR",
	"PARECE BASTANTE RACIONAL",
	"ES UNA FOTO DE PLATON ESCRIBIENDO SU DIALOGO PERDIDO",
	"NO SOY DE LOS QUE HABLAN CON POSTERS",
	// 135
	"UN ESCRITORIO MUY MONO",
	"ES UN DIPLOMA DE CAZA-CAZA-VAMPIROS HOMOLOGADO POR LA UNIVERSIDAD DE OXFORD",
	"ES NOCHE CERRADA CON LUNA LLENA",
	"PARECE QUE ESTOS TORNILLOS NO ESTAN MUY ENROSCADOS",
	"NO MIRES, PERO CREO QUE UNA CAMARA OCULTA ME ESTA ENFOCANDO",
	// 140
	"UN DETECTOR DE ESTACAS MUY MODERNO",
	"NO, EL LABORATORIO ESTA EN EL SEGUNDO PISO",
	"UNA BONITA MESILLA DE NOCHE",
	"ES UN MONTON DE DINERO QUE NO PUEDE FALTAR EN UNA AVENTURA QUE SE PRECIE",
	"SI YO FUERA RICO, DUBIDUBIDUBIDUBIDUBIDUBIDUBIDU",
	// 145
	"SON UNAS EXTRA\245AS HOJAS. DEBEN DE HABERLAS TRAIDO DE SUDAMERICA, O POR AHI",
	"NO CREO QUE ME CONTESTARAN",
	"ES UN BONITO CRUCIFIJO DE MADERA. EL ICONO NO CAPTA TODA LA EXTENSION DE SU BELLEZA",
	"YO SOLO REZO ANTES DE ACOSTARME",
	"\255EH, PARECE QUE ESTA PICA ESTA UN POQUITO SUELTA!",
	// 150
	"PARA QUE LUEGO TE QUEJES DE QUE NO DOY PISTAS",
	"ES UNA PICA BASTANTE CONVENCIONAL",
	"SON MONOS, AUNQUE TIENEN UN POCO DE MIERDECILLA",
	"NO, NO ME OIRIAN. JI,JI,JI \255QUE BUENO!",
	"\"LA BELLA DURMIENTE DEL BOSQUE\" DE CHAIKOSKY, O CHOIFRUSKY, O COMO SE DIGA",
	// 155
	"MUY APETITOSO",
	"NO, YO NO SOY DE ESOS QUE SE METEN EN LA BOCA CHICLES USADOS",
	"UNA HOZ MUY MONA. ME PREGUNTO DONDE ESTARA EL MARTILLO",
	"\"LOS FABRICANTES DE TABACO ADVIERTEN QUE LAS AUTORIDADES SANITARIAS PERJUDICAN SERIAMENTE LA SALUD\"",
	"UNA VELA NORMAL Y CORRIENTE, CON CERA Y TODO",
	// 160
	"\255HAY QUE VER COMO BRILLAN ESTAS DOS BRILLANTES MONEDAS!",
	"\255HAY QUE VER COMO BRILLA ESTA BRILLANTE MONEDA!",
	"CON ESTO SERE INMUNE A LAS MORDEDURAS DE LOS VAMPIROS",
	"NO, TODAVIA NO ES EL MOMENTO",
	"HAY UN BILLETE DE MIL Y UN PAR DE MONEDAS",
	// 165
	"DICE \"SE RUEGA NO ECHAR COMIDA AL PIANISTA\"",
	"TORTILLA, 200. PESCAITOS FRITOS, 150, PATATAS ALIOLI, 225",
	"LAS MEJORES HAMBUERGUESAS A ESTE LADO DEL DANUBIO, POR SOLO 325",
	"ES UNA BONITA CALAVERA, CON UNA MIRADA MUY PENETRANTE. \255JI, JI, JI, QUE BUENO!",
	"HOLA CALAVERA, ME RECUERDAS AL TIO DE HAMLET",
	// 170
	"TENGO POR COSTUMBRE NO TOCAR COSAS QUE HAYAN ESTADO VIVAS",
	"ES UNA PAPELERA",
	"ES UNA PORRA PARA EL PARTIDO DE ESTA NOCHE",
	"ME PREGUNTO QUE HABRA DETRAS",
	"\255EH, ESTA CORTINA NO SE MUEVE!",
	// 175
	"HAY QUE VER, QUE TETRICO ES ESE CASTILLO, \250EH? ",
	"NO PUEDO, ESTA DEMASIADO LEJOS PARA OIRME",
	"ES UN TIPICO BOSQUE TRANSILVANO, CON ARBOLES ",
	"\255PERO QUE TONTERIAS DICES, CON LO OSCURO QUE ESTA!",
	"CONFITERIA GARCIA. PASTELES Y CHICLES.",
	// 180
	"UNA PUERTA MUY BONITA",
	"ESTA CERRADA",
	"UN BARRIL COMPLETAMENTE CERRADO",
	"",
	"\255QUE BICHOS MAS MONOS!",
	// 185
	"BSSST, BSSST, GATITO...",
	"NO CONTESTA",
	"LA LUNA ES UN SATELITE QUE GIRA ALREDEDOR DE LA TIERRA CON UN PERIODO DE REVOLUCION DE 28 DIAS",
	"HOLA, LUNA LUNERA CASCABELERA ",
	"ESTA TOTALMENTE TAPIADA CON TABLONES",
	// 190
	"IMPOSIBLE. ESTO NO LO ABRE NI EL MAYORDOMO DE LA TELE",
	"\255EH, PARECE QUE LA SOMBRA DE ESE CIPRES ES ALARGADA!",
	"\255EOOO, TABERNEROOO!",
	"QUISIERA UNA HABITACION ",
	"\250SABE DONDE PUEDO ENCONTRAR A UN TAL CONDE DRASCULA?",
	// 195
	"SI, \250QUE PASA? ",
	"\250Y ESO?",
	"\250DE... DE VERDAD?",
	"BUENA PREGUNTA, LE VOY A CONTAR MI HISTORIA, MIRE...",
	"SON SOLO CINCO MINUTOS",
	// 200
	"ME LLAMO JOHN HACKER, Y SOY REPRESENTANTE DE UNA INMOBILIARIA BRITANICA",
	"AL PARECER, EL CONDE DRASCULA QUIERE COMPRAR UNOS TERRENOS EN GIBRALTAR, Y ME HAN MANDADO A MI PARA NEGOCIAR LA VENTA",
	"PERO ME PARECE A MI QUE MA\245ANA TEMPRANITO ME VUELVO CON MI MAMA",
	"BONITA NOCHE \250VERDAD?",
	"NO, NADA",
	// 205
	"EOOOO, PIANISTA",
	"BONITA NOCHE",
	"Y ADEMAS NO HACE FRIO",
	"PUES NADA, TE DEJO QUE SIGAS TOCANDO",
	"PUES ESO",
	// 210
	"HOLA JEFE, \250QUE TAL?",
	"\250Y LA FAMILIA?",
	"HAY AMBIENTE AQUI \250EH?",
	"MEJOR NO DIGO NADA",
	"SE ESTA MEJOR EN CASA QUE EN NINGUN SITIO... SE ESTA MEJOR EN... \250EH? PERO SI USTED NO ES LA TIA ENMA. ES MAS. \255SI YO NO TENGO NINGUNA TIA ENMA!",
	// 215
	"SI, EL MIO TAMBIEN. USTED PUEDE LLAMARME COMO LE APETEZCA, AUNQUE SI ME LLAMA JOHNY, ACUDO COMO LOS PERROS",
	"SI, QUE GOLPES TENGO \250VERDAD? POR CIERTO, \250DONDE ESTOY?",
	"SI.",
	"CACHIS... ",
	"OH, SI. POR SUPUESTO",
	// 220
	"PUES MUCHAS GRACIAS POR PRESTARME TU AYUDA. NO TE MOLESTO MAS. SI ME DICES DONDE ESTA LA PUERTA, POR FAVOR... ",
	"PORQUE EL GOLPE ME HA DEBIDO AFECTAR AL CEREBRO Y NO VEO TRES EN UN BURRO ",
	"BAH, NO IMPORTA. SIEMPRE LLEVO DE REPUESTO",
	"\255GUAU, QUE TIA MAS BUENA! \255NO ME HABIA DADO CUENTA YO! CLARO, SIN LAS GAFAS",
	"OYE... ",
	// 225
	"\255\250 Y ESTOOOOOO?!",
	"\255NO TE PREOCUPES B.J., AMOR MIO! TE SALVARE DE LAS GARRAS DE ESE ",
	"ME HA TOCADO LAS NARICES, VAMOS.",
	"\255AHHH, UN HOMBRE LOBO! \255MUERE MALDITO!",
	"SI, ESTO...",
	// 230
	"SI, ESTO... CREO QUE SEGUIRE MI CAMINO. CON PERMISO... ",
	"\250QUE?",
	"PUES LA VERDAD, PENSANDOLO BIEN... CREO QUE NO",
	"DIME, OH ERUDITO FILOSOFO, \250EXISTE ALGUNA RELACION CAUSA-EFECTO ENTRE LA VELOCIDAD Y EL TOCINO?",
	"VALE, VALE, DEJALO. DE TODAS FORMAS, NO SE POR QUE HE DICHO ESO.",
	// 235
	"\250QUE HACES AQUI FILOSOFANDO, QUE NO TE ESTAS COMIENDO GENTE?",
	"\250Y ESO?",
	"OYE, PUEDES REPETIR ESO DE \"INCLINACIONES PRE-EVOLUTIVAS\"?",
	"SI HOMBRE. ESE ROLLO QUE ME HAS SOLTADO ANTES. ES QUE NO ME HE ENTERADO MUY BIEN...",
	"NO, MEJOR NO DIGO NADA, QUE COMO LE TOQUE LA VENA...",
	// 240
	"\250SI, DIGA?",
	"SI, \250QUE PASA? ",
	"AH, PUES AHORA QUE SACA EL TEMA LE DIRE QUE...",
	"",
	"POR CIERTO, \250QUE PASARIA SI UN VAMPIRO SE HICIERA CIRCUNSTANCIALMENTE CON LA RECETA... ",
	// 245
	"PUES NADA. OYE, \250A QUE ESTO PARECE UN PEGOTE PUESTO EN EL GUION PARA ACABAR PRONTO EL JUEGO? BUENO, A LO MEJOR NO",
	"\255ESTA VACIO!",
	"PORQUE ME ROBASTE MI AMOR, B.J. SIN ELLA LA VIDA PARA MI NO TIENE SENTIDO",
	"\255\250SU CEREBRO?!",
	"PUES NO ES POR NADA, PERO ME PARECE QUE ME HA CARGADO TU MONSTRUITO",
	// 250
	"\255VIRGENCITA, QUE ME QUEDE COMO ESTOY!",
	"\255NO TE SALDRAS CON LA TUYA. SEGURO QUE APARECE SUPERLOPEZ Y ME RESCATA!",
	"VAYA UNA MIERDA DE JUEGO EN EL QUE MUERE EL PROTAGONISTA ",
	"UN MOMENTO, \250QUE HAY DE MI ULTIMO DESEO?",
	"\255JA, JA! AHORA ESTOY INMUNIZADO CONTRA TI MALDITO DEMONIO. ESTE CIGARRILLO ES UNA POCION ANTIVAMPIRICA QUE ME DIO VON BRAUN ",
	// 255
	"SI, CLARO. PERO NUNCA CONSEGUIRAS QUE TE DIGA LA RECETA",
	"PUEDO SOPORTAR LA TORTURA, ADEMAS DE CREARLA ",
	"\255NO, POR FAVOR! \255HABLARE, PERO NO ME HAGA ESO!",
	"BUENO. YA TE HE DICHO LO QUE QUERIAS SABER. AHORA LIBERANOS A B.J. Y A MI Y DEJANOS EN PAZ",
	"\255B.J.! \250QUE HACES AQUI? \250DONDE ESTA DRASCULA?",
	// 260
	"\255QUE DESALMADO! SOLO PORQUE PERTENECE A LA NOBLEZA SE CREE QUE TIENE EL DERECHO DE PERNADA CON CUALQUIERA QUE SE LE ANTOJE ",
	"\255ABAJO LA ARISTOCRACIA CACIQUIL!",
	"\255ARRIBA LOS POBRES DEL MUUNDOOO....",
	"Y POR LO QUE VEO TE HA ENCADENADO CON CANDADO Y TODO",
	"BUENO, VALE. \250NO TENDRAS UNA HORQUILLA?",
	// 265
	"BUENO, BUENO. NO TE PONGAS ASI, YA SE ME OCURRIRA ALGO.",
	"EH, TABERNERO",
	"\250COMO VA EL PARTIDO?",
	"\250QUIEN?",
	"\250ES QUE NO VES QUE ESTA AQUI DRASCULA?",
	// 270
	"PUES VAMOS A ACABAR CON EL \250NO?",
	"PONME UN CUBATA...",
	"NADA. SE ME HA OLVIDADO LO QUE TE IBA A DECIR",
	"O\247ME\247PONES\247UN\247CUBATA\247O\247ME\247PONGO\247A\247TOCAR\247EL\247PIANO",
	"\250CUANTO QUEDA PARA QUE ACABE EL PARTIDO?",
	// 275
	"BUENAS NOCHES",
	"\250Y QUE TAL ANDAS IGOR? \250JOROBAO? \255JI,JI,JI, QUE BUENO! ",
	"\250QUE SE SUPONE QUE HACES?",
	"PUES NO",
	"PUES PONTE GAFAS",
	// 280
	"\250QUE ES ESO DE LA ORGIA SOBRENATURAL?",
	"VALE, VALE, NO SIGAS. YA ME HAGO UNA IDEA",
	"\250NO PODRIAS DECIRME DONDE ESTA DRASCULA? ",
	"ANDA, PORFAAA",
	"\250POR QUE NO?",
	// 285
	"AH, PERO \250DUERME POR LA NOCHE?",
	"BUENO, PUES QUE SE DE BIEN LA RENTA",
	"ES QUE TENGO QUE HABLAR CON EL",
	"\255EOOO, ESQUELETOOO! ",
	"\255CANASTOS! \255UN MUERTO QUE HABLA!",
	// 290
	"CUENTAME, \250COMO HAS VENIDO A PARAR AQUI?",
	"\250Y PARA QUE VA A QUERER DRASCULA CREAR UN MONSTRUO? ",
	"\250CUAL ES TU NOMBRE, AMIGO ESQUELETO?",
	"OYE, \250NO QUIERES QUE TE TRAIGA NADA DE COMER?",
	"DEBES TENER EL ESTOMAGO VACIO. \255JI, JI, JI!",
	// 295
	"LA VERDAD ES QUE NO ME APETECE HABLAR AHORA",
	"\255LA MADRE QUE...(PITIDO) ESPERO QUE TE...(PITIDO) Y QUE TE...(PITIDO) DOS VECES!",
	"YO LA QUERIA, DE VERDAD. VALE, DE ACUERDO QUE NO ERA UNA LUMBRERA, PERO NADIE ES PERFECTO \250NO? ",
	"ADEMAS, TENIA UN CUERPAZO QUE PARA QUE QUEREMOS MAS",
	"YA NO VOLVERE A SER EL MISMO. ME RECLUIRE EN UN MONASTERIO A DEJAR QUE MI VIDA SE ME ESCAPE POCO A POCO",
	// 300
	"NADA PODRA YA SACARME DE ESTA MISERIA PORQUE...",
	"\250DE QUIEN? \250DE QUIEN?",
	"QUIERO SER PIRATA",
	"QUIERO SER PROGRAMADOR   ",
	"CONTADME ALGO SOBRE PELAYO",
	// 305
	"SEGUIRE JUGANDO Y OLVIDARE QUE OS HE VISTO   ",
	"\255A QUIEN SE LE HABRA OCURRIDO ESTA IDIOTEZ!",
	"ES UN BOLSO COMO EL DE MI ABUELITA ",
	"\255PERO QUE BUENO QUE ESTOY!",
	"CUANTO MAS ME MIRO MAS ME GUSTO",
	// 310
	"\250Y LUEGO COMO ME CIERRO?",
	"TENDRE QUE ABRIRME PRIMERO \250NO?",
	"ESTOY BIEN DONDE ESTOY",
	"YA ME TENGO",
	"HOLA YO",
	// 315
	"ME LOS PONDRE CUANDO LA OCASION SEA OPORTUNA",
	"NO VEO NADA EN ESPECIAL",
	"ESTA BIEN DONDE ESTA",
	"\250Y PA QUE?",
	"NO PUEDO",
	// 320
	"HOLA TU",
	"ES EL PANTEON DEL TIO DESIDERIO",
	"\255EOOOO, TIO DESIDERIOOOO!",
	"NO. NO QUIERO CORTARME OTRA VEZ",
	"\255EJEM,JEM...!",
	// 325
	"\255YAMM, EMMM, JH!",
	"\255SI, COF,COF!",
	"ANDA, SI HAY UN CHICLE AQUI PEGADO",
	"ES EL MOVILANI QUE ME REGALARON EN NAVIDAD",
	"\255QUE ALTO ESTA!",
	// 330
	"\255SAL AL BALCON JULIETA!",
	"\255TU ERES LA LUZ QUE ILUMINA MI CAMINO!",
	"EH,PUERTA \250QUE PASSA?",
	"EOOO, MAQUINA DE TABACO DE TRANSILVANIAAA",
	"ES UNA MAQUINA EXPENDEDORA DE TABACO",
	// 335
	"TENGO OTRA MONEDA DENTRO",
	"NO. HE DECIDIDO DEJAR EL TABACO Y EL ALCOHOL ",
	"A PARTIR DE AHORA ME DEDICARE SOLO A LAS MUJERES  ",
	"\255ESTO ES UN TIMO! NO HA SALIDO NADA",
	"\255POR FIN! ",
	// 340
	"PUES ESO, UN BAUL",
	"HOLA BAUL, TE LLAMAS COMO MI PRIMO QUE SE LLAMA RAUL.",
	"HE ENCONTRADO EL BOLSO DE B.J.",
	"DIOS MIO, NO ME REFLEJO \255SOY UN VAMPIRO!",
	"...AH, NO. ES QUE ES UN DIBUJO",
	// 345
	"ESPEJITO: \250QUIEN ES EL MAS BELLO DEL REINO?",
	"NO ME QUIERE ABRIR",
	"MUY BIEN. ME HE PUESTO LOS TAPONES",
	"ES UN DIPLOMA DE CAZA-VAMPIROS HOMOLOGADO POR LA UNIVERSIDAD DE CAMBRIDGE",
	"NO, AUN ME FALTAN INGREDIENTES, NO VALE LA PENA DESPERTARLE",
	// 350
	"ES QUE NO TENGO DINERO",
	"ES UNA LAMPARA BRITANICA",
	"\255TABERNERO! \255AYUDEME!",
	"HA APARECIDO UN VAMPIRO Y SE HA LLEVADO A MI NOVIA",
	"\255\250PERO NO VA USTED A AYUDARME?!",
	// 355
	"\250MUERTA? \250QUE ES LO QUE QUIERE DECIR?",
	"\255EJEM!",
	"\255UN VAMPIRO HA SECUESTRADO A LA CHICA DE LA 506!",
	"\255TIENES QUE AYUDARME!",
	"\250NO SABES TOCAR NINGUNA DE LOS INHUMANOS?",
	// 360
	"\250COMO TE AGUANTAS, TODO EL DIA TOCANDO LO MISMO?",
	"\250Y ENTONCES COMO ME OYES?",
	"DEJAME LOS TAPONES",
	"ANDA. SI TE LOS DEVUELVO ENSEGUIDA",
	"VENGAAAA...",
	// 365
	"ADIOS. TENGO QUE MATAR A UN VAMPIRO",
	"",
	"\250QUE HABLAS? \250EN TRANSILVANO?",
	"\250QUIEN ES EL TIO DESIDERIO?",
	"\250PERO QUE PASA CON ESE TAL DRASCULA?",
	// 370
	"\250QUIEN ES ESE TAL VON BRAUN?",
	"\250Y POR QUE NO LO HACE?",
	"\250Y DONDE PUEDO ENCONTRAR A VON BRAUN?",
	"PUES GRACIAS Y ADIOS, QUE LA DUERMAS BIEN",
	"SERA MEJOR LLAMAR PRIMERO",
	// 375
	"\250ES USTED EL PROFESOR VON BRAUN?",
	"\250Y NO ME PODRIA DECIR DONDE PUEDO...?",
	"NO ME CREO QUE SEA EL ENANO GANIMEDES",
	"\255PROFESOR!",
	"\255AYUDEME! \255LA VIDA DE MI AMADA DEPENDE DE USTED!",
	// 380
	"ESTA BIEN, NO NECESITO SU AYUDA",
	"DE ACUERDO. ME VOY",
	"NO TENGA MIEDO. JUNTOS VENCEREMOS A DRASCULA",
	"ENTONCES \250POR QUE NO ME AYUDA?",
	"YO LAS TENGO",
	// 385
	"\255SI LAS TENGO!",
	"DE ACUERDO",
	"...ER ...SI",
	"VENGO A METERME EN ESA CABINA DE NUEVO",
	"ESTOY PREPARADO PARA ENFRENTARME A SU PRUEBA",
	// 390
	"ESTA BIEN, VEJETE. HE VENIDO A POR MI DINERO",
	"NO, NADA. YA ME IBA",
	"PERDONA",
	"\250TE INTERESA ESTE LIBRO? TIENE PARTITURAS DE TCHAIKOWSKY",
	"\250COMO PUEDO MATAR A UN VAMPIRO?",
	// 395
	"\250NO TE HAN DICHO QUE ES MALO DORMIR EN MALA POSTURA?",
	"PUES ES LO QUE SIEMPRE ME DICE A MI MI MADRE",
	"\250POR QUE NO TE PUDO MATAR DRASCULA?",
	"\250Y QUE FUE?",
	"\255ESTUPENDO! \255TIENE USTED UNA POCION DE INMUNID...!",
	// 400
	"\250ENTONCES?",
	"MUY BIEN",
	"\250ME PUEDE REPETIR LO QUE NECESITO PARA ESA POCION?",
	"PUES PARTO RAUDO A BUSCARLO",
	"OIGA, \250QUE HA PASADO CON EL PIANISTA?",
	// 405
	"YA\247TENGO\247TODOS\247LOS\247INGREDIENTES\247DE\247ESA\247POCION",
	"UNA PREGUNTA: \250QUE ES ESO DE ALUCSARD ETEREUM?",
	"DIGA, DIGA... ",
	"\250Y DONDE ESTA ESA GRUTA?",
	"\250QUE PASA? \250NO TENIAIS JUZGADO?",
	// 410
	"...PERO ...\250Y SI ME ENCUENTRO A MAS VAMPIROS?",
	"ES UN VAMPIRO QUE NO ME DEJA PASAR",
	"SE PARECE A YODA, PERO MAS ALTO ",
	"EH, YODA. SI ME DEJAS PASAR TE DOY UN DURO",
	"BUENO VALE, QUE NO SE TE PUEDE DECIR NADA",
	// 415
	"HOLA VAMPIRO, BONITA NOCHE \250VERDAD?",
	"\250TE HAN DICHO ALGUNA VEZ QUE TE PARECES A YODA?",
	"\250ERES UN VAMPIRO O UNA PINTURA AL OLEO?",
	"MEJOR NO TE DIGO NADA, NO TE VAYAS A ENFADAR",
	"ESTA CERRADA CON LLAVE",
	// 420
	"\255LA URRACA ME PODRIA SACAR UN OJO SI LO INTENTO!",
	"\255ESTA CERRADA! \255DIOS MIO, QUE MIEDO!",
	"LAS BISAGRAS ESTAN OXIDADAS",
	"AQUI DENTRO SOLO HAY UN BOTE CON HARINA",
	"ESO HA QUITADO EL OXIDO",
	// 425
	"HE ENCONTRADO UNA ESTACA DE PINO",
	"COGERE ESTE QUE ES MAS GORDO",
	"BUENO, CREO QUE YA PUEDO DESHACERME DE ESTE ESTUPIDO DISFRAZ",
	"\"PASADIZO A LOS TORREONES CERRADO POR OBRAS. POR FAVOR, UTILICEN ENTRADA PRINCIPAL. DISCULPEN LAS MOLESTIAS\"",
	"...ES PALIDO, TIENE COLMILLOS, TIENE TUPE Y USA CAPA... \255SEGURO QUE ES DRASCULA!",
	// 430
	"\255ES B.J.! B.J. \250ESTAS BIEN?",
	"SI, YA LO SE QUE ES TONTA, PERO ES QUE ESTOY DE SOLO",
	"NO TENDRAS UNA LLAVE POR AHI, \250VERDAD? ",
	"\250A QUE NO TIENES UNA GANZUA?",
	"DAME UNA HORQUILLA. VOY A HACER COMO MCGYVER",
	// 435
	"NO TE MUEVAS QUE AHORA VUELVO",
	"\255CACHIS! \255SE ME HA ROTO!",
	"\255OLEEEE! \255Y ADEMAS ME HE AFEITADO, COLEGA!",
	"\250SI, CARI\245O?",
	"NO LLEGA",
	// 440
	"EL PIANISTA NO ESTA",
	"UN CUBATA TRANSILVANO",
	"AUN NO TENGO HABITACION",
	"PARECE QUE SE QUEDO ATASCADO EN LA BA\245ERA Y DECIDIO PONER UN BAR",
	"ESTA COMO UNA CUBA DE CUBA",
	// 445
	"ESE PELO... EL CASO ES QUE ME RECUERDA A ALGUIEN",
	"ES UN ESQUELETO HUESUDO",
	"\255MIRA! \255MIGUEL BOSE!",
	"ESTA DURMIENDO. SERIA UNA PENA DESPERTARLE",
	"ES MAS FEO QUE EMILIO DE PAZ ",
	// 450
	"UN ATAUD DE MADERA DE PINO",
	"ME VA A CORTAR EN RODAJITAS, COMO A UN SALCHICHON",
	"NO ME GUSTAN LOS PENDULOS. PREFIERO LAS ALCACHOFAS",
	"MIS MANOS ESTAN ESPOSADAS. NO VOY A PODER",
	"SALTA A LA VISTA QUE ES UNA PUERTA SECRETA",
	// 455
	"ME IGNORAN",
	"\255ANDA YA!",
	"EN EL GUION SE MOVIA, PERO EL JUEGO SE SALIO DE PRESUPUESTO Y NO PUDIERON PAGARME UN GIMNASIO PARA PONERME CACHAS, ASI QUE NADA",
	"PARECE QUE ESTA UN POCO SUELTA DE LA PARED",
	"NO CREO QUE ME VAYA A SERVIR DE NADA. ESTA DEMASIADO HUMEDA PARA ENCENDERLA",
	// 460
	"\250AL ALA OESTE? \255NI LOCO! \255A SABER QUE HABRA ALLI!",
	"TIENE BONITOS MOTIVOS TRANSILVANOS ",
	"",
	"QUE PENA QUE AHI DENTRO NO HAYA UN CORDERITO ASANDOSE ",
	"LA ULTIMA VEZ QUE ABRI UN HORNO LA CASA SALTO POR LOS AIRES",
	// 465
	"ES EL ESCUDO DEL EQUIPO DE FUTBOL DE TRANSILVANIA",
	"\250Y PARA QUE? \250PARA PONERMELA EN LA CABEZA?",
	"NO CREO QUE ESTOS CAJONES SEAN DE LOS QUE SE ABREN",
	"\255NO QUIERO SABER LA COMIDA QUE HABRA AHI DENTRO!",
	"ME DA LA IMPRESION DE QUE ES IMPRESIONISTA",
	// 470
	"LA NOCHE SE APODERA DE TODOS... QUE MIEDO \250NO?",
	"ESTA ATRANCADA",
	"ES EL REY \250ES QUE NO TE LO HABIAS IMAGINADO?",
	"NO, YA TENGO UNO EN MI CASA, AL QUE LE DOY DE COMER Y TODO",
	"UNA ESTANTERIA CON LIBROS Y OTRAS COSAS ",
	// 475
	"\250Y A QUIEN LLAMO A ESTAS HORAS?",
	"\"COMO HACER LA DECLARACION DE LA RENTA\" \255QUE INTERESANTE!",
	"YA TENGO UNO EN MI CASA. CREO QUE ES UN BEST-SELLER MUNDIAL ",
	"UNA LLAVE COMPLETAMENTE NORMAL",
	"ME PARECE A MI QUE ESTA NO ES DE AQUI",
	// 480
	"\255EH, SON PATATAS FRITAS CON FORMA DE COLMILLO! ME ENCANTA",
	"NO CREO QUE SEA EL MEJOR MOMENTO DE PONERSE A COMER CHUCHERIAS, CON MI NOVIA EN MANOS DEL SER MAS MALVADO QUE HA PARIDO MADRE",
	"\255QUE BIEN ME LO ESTOY PASANDO CARGANDOME VAMPIROS CON ESTO!",
	"A VER SI APARECE OTRO PRONTO",
	"NO, TIENE QUE SER CON UN VAMPIRO SUCIO Y MALOLIENTE COMO EL QUE ME CARGUE ANTES",
	// 485
	"ES LA AUTENTICA PELUCA QUE USO ELVIS CUANDO SE QUEDO CALVO",
	"ES HARINA, PERO NO PUEDO DECIR MARCAS",
	"QUIZA EN OTRO MOMENTO \250VALE?",
	"ES UN HACHA MAGNIFICA, QUE PENA QUE NO HAYA POR AQUI CERCA NINGUNA CABEZA DE VAMPIRO",
	"NO. EN EL FONDO SOY BUENA PERSONA",
	// 490
	"ES EL DESODORANTE DE LA TACHER \255JI,JI,JI!",
	"ES UNA CAPA BASTANTE MONA",
	"",
	"COMO TODAS LAS RAMAS DE TODOS LOS ARBOLES DE TODO EL MUNDO, O SEA, SIN NADA DE PARTICULAR",
	"\255OH, INCREIBLE! \255UNA CUERDA EN UNA AVENTURA GRAFICA!",
	// 495
	"ME PREGUNTO PARA QUE SERVIRA...",
	"UNA CUERDA ATADA A UNA RAMA O UNA RAMA ATADA A UNA CUERDA, SEGUN SE MIRE",
	"PARECE QUE ESTA URRACA TIENE MUY MALAS INTENCIONES",
	"QUITA, YO NO LA DIGO NADA, A VER SI SE VA A ENFADAR",
	"PARECE QUE ESTA MUERTA, PERO ES MENTIRA \250EH?",
	// 500
	"NINGUN ANIMAL DA\245ADO EN LA PRODUCCION DE ESTE JUEGO",
},
{
	// 0
	"",
	"Das ist die zweitgroesste Tuer, die ich je gesehen habe!",
	"Ok, stimmt nicht ganz....",
	"Sie haben Bretter davorgenagelt. Die Kirche scheint seit Jahren leer zu stehen",
	"Aber ich habe sie doch nicht aufgemacht!",
	// 5
	"Soll ich sie aufbrechen?",
	"Hallo Tuer! Ich verpasse Dir jetzt mal einen Rahmen!",
	"Zuviel fuer mich!",
	"ein zugenageltes Fenster",
	"Ich schaff\357es nicht!",
	// 10
	"Schon geschafft!",
	"Und wozu?",
	"Hallo Fenster. Hast Du heute abend schon was vor?",
	"Nicht ohne Genehmigung der Staedtischen Baubehoerde",
	"hey! Das Fenster da hat nur EIN Brett...",
	// 15
	"Heyhooo! Fenster!",
	"Hallo Du",
	"",
	"Ich komme nicht dran!",
	"Gut so!",
	// 20
	"",
	"Es ist ein Grabstein in Form eines Kreuzes.",
	"Nein, danke",
	"Hallo Toter. Willst Du ein paar Wuermchen?",
	"Klar Mann! Wie in Poltergeist.",
	// 25
	"",
	"",
	"Bin in einer Viertelstunde zurueck!",
	"Plakate ankleben verboten",
	"",
	// 30
	"Sie ist abgeschlossen.",
	"Ich habe schon eins.",
	"",
	"Keine Reaktion!",
	"Nein, da steht es gut!",
	// 35
	"Es ist eine Tuer",
	"Eine Schublade vom Tisch",
	"Ein verdaechtiger Schrank",
	"Hallo Schrank. Alles klar?",
	"",
	// 40
	"",
	"Ein uralter Kerzenstaender",
	"Der stammt wohl noch aus der Zeit als meine Ururururoma in den Kindergarten ging",
	"Nein, eine Reliquie aus vergangenen Zeiten",
	"Es ist ein huebsches Altarbild",
	// 45
	"",
	"Hi,hi,hi",
	"",
	"Nein.",
	"",
	// 50
	"Ha,ha,ha! klasse!",
	"",
	"",
	"",
	"Ich sehe nichts besonderes",
	// 55
	"Freddy, die Topfblume",
	"Eine Spitze aus dem Jaegerzaun",
	"Ej! Hier unten liegt eine Streichholzschachtel!",
	"Guck mal! Eine Packung Tempos! Und eins sogar ungebraucht!",
	"Sonst ist nichts mehr im Eimer",
	// 60
	"Ein Blinder der nichts sieht",
	"",
	"",
	"",
	"",
	// 65
	"Ein Wahnsinnsbatzen Geld.",
	"",
	"",
	"",
	"",
	// 70
	"",
	"",
	"",
	"",
	"",
	// 75
	"",
	"",
	"",
	"",
	"",
	// 80
	"",
	"",
	"",
	"",
	"",
	// 85
	"",
	"",
	"",
	"",
	"",
	// 90
	"",
	"",
	"",
	"",
	"",
	// 95
	"",
	"",
	"",
	"",
	"",
	// 100
	"SIEHT NICHT UNGEWoeHNLICH AUS",
	"SIEHT NICHT AUSSERGEWoeHNLICH AUS",
	"EJ DU, WAS IST LOS?",
	"HALLO",
	"NICHTS NEUES?",
	// 105
	"WIE GEHTS DER FAMILIE?",
	"WAS DU FueR SACHEN HAST!",
	"ABER WIE SOLLTE ICH DAS AUFHEBEN?",
	"MEINE RELIGION VERBIETET MIR DAS",
	"BESSER NICHT",
	// 110
	"KLAR MANN!",
	"DU SPINNST WOHL!",
	"UNMoeGLICH",
	"GEHT NICHT AUF!",
	"ALLEINE SCHAFFE ICH DAS NICHT",
	// 115
	"WENN ICH WOLLTE, KoeNNTE ICH, ABER ICH HABE KEINE LUST",
	"ICH WueSSTE NICHT WARUM",
	"EIN SCHaeDEL, DER GUT DRAUF IST",
	"UND? SCHON WAS VOR HEUTE ABEND, SCHaeDEL?",
	"NEIN, ER MUSS AN EINEM KueHLEN ORT GELAGERT WERDEN, FERN DER ZERSToeRERISCHEN KRAFT DER ATMOSPHaeRE",
	// 120
	"KNOCHENTROCKEN, WIE MEIN CHEF!",
	"EIN SEHR SPITZER PFLOCK",
	"GUTER SPITZER PFLOHOCK, HOLZ SO EDEL, TRANSSYLVAAANISCH!!!",
	"OH VERDAMMT, ICH MUSS MIR DIE NaeGEL SCHNEIDEN!",
	"DA DRINNEN LIEGT B.J., UND GUCK MAL WIE DIE TANTE AUSSIEHT!",
	// 125
	"NIET UND NAGELFEST",
	" \"NIET & NAGEL GmbH & Co. KG\"",
	"DAS TYPISCHE SKELETT IM TYPISCHEN VERLIESS EINES TYPISCHEN SPIELS",
	"WIRD NORMALERWEISE ZUR STROMVERSORGUNG ALLER EINGESToePSELTEN GERaeTE VERWENDET",
	"TOTALES KUNSTHANDWERK, WEIL SIE DANK DEN JAPANERN KLEINE MINIATUREN SIND",
	// 130
	"ES GIBT NUR EINS, WAS GENAUSO HaeSSLICH IST",
	"HAU AB. ICH SAG IHM GAR NICHTS. MAL GUCKEN OB ER AUSFLIPPT",
	"SCHEINT ZIEMLICH VERNueNFTIG ZU SEIN",
	"EIN FOTO VON PLATON WIE ER SEINEN VERLORENEN DIALOG SCHREIBT",
	"SEH ICH SO AUS ALS OB ICH MIT POSTERN REDE?!",
	// 135
	"EIN HueBSCHER SCHREIBTISCH",
	"EIN VON DER UNIVERSITaeT ZU OXFORD AUSGESTELLTES VAMPIR-JaeGER-JaeGER-DIPLOM",
	"VOLLMOND IN FINSTERSTER NACHT",
	"DIESE SCHRAUBEN SCHEINEN ZIEMLICH FEST ZU SITZEN",
	"GUCK JETZT NICHT HIN, ABER ICH GLAUBE HIER GIBT ES EINE VERSTECKTE KAMERA",
	// 140
	"EIN ULTRAMODERNER PFLOCK-DETEKTOR",
	"NEIN, DAS LABOR IST IM ZWEITEN STOCK",
	"EIN HueBSCHES NACHTTISCHCHEN",
	"EIN BATZEN GELD DER BEI EINEM WIRKLICH GUTEN ABENTEUER NICHT FEHLEN DARF",
	"WENN ICH REICH WaeRE, SCHUBIDUBISCHUBIDUBUDU",
	// 145
	"KOMISCHE BLaeTTER, DIE WERDEN SIE WOHL AUS SueDAMERIKA ODER SO MITGEBRACHT HABEN.",
	"ICH GLAUBE NICHT, DASS SIE ANTWORTEN WERDEN",
	"EIN SCHoeNES HOLZKRUZIFIX. DEM HELDEN SIND DIE AUSMASSE SEINER SCHoeNHEIT NICHT GANZ KLAR.",
	"ICH BETE NUR VOR DEM ZU-BETT-GEHEN",
	"EJ, ICH GLAUBE, DIE SPITZE HIER IST EIN BISSCHEN LOSE!",
	// 150
	"UND NACHHER BESCHWERST DU DICH, ICH HaeTTE DIR KEINEN WINK GEGEBEN!",
	"EINE STINKNORMALE ZAUNSPITZE",
	"NIEDLICH, OBWOHL SIE EIN BISSCHEN VERDRECKT SIND",
	"SIE WERDEN MICH NICHT HoeREN, HI,HI,HI, SUPERGUT!",
	"DIE SCHoeNE SCHLAFENDE AUS DEM WALDE VON TSCHAIKOWSKY ODER TSCHESLOFSKY ODER WIE DER HEISST.",
	// 155
	"SEHR APPETITLICH",
	"NEIN, SEHE ICH SO AUS, ALS WueRDE ICH BENUTZTE KAUGUMMIS KAUEN?",
	"EINE NIEDLICHE SICHEL. ICH WueSSTE NUR GERNE WO DER HAMMER IST.",
	"DER GESUNDHEITSMINISTER WARNT: RAUCHEN GEFaeHRDET IHRE GESUNDHEIT",
	"EINE GANZ NORMALE KERZE, MIT WACHS UND ALLEM DRUM UND DRAN",
	// 160
	"MAN MUSS ECHT MAL GESEHEN HABEN WIE DIESE 2 GLaeNZENDEN MueNZEN GLaeNZEN",
	"MAN MUSS ECHT GESEHEN HABEN WIE DIESE GLaeNZENDE MueNZE GLaeNZT!",
	"DAMIT WERDE ICH GEGEN VAMPIRBISSE IMMUN SEIN.",
	"NEIN,ES IST NOCH NICHT SO WEIT",
	"EIN TAUSENDER UND EIN BISSCHEN KLEINGELD",
	// 165
	"DA STEHT\"DEN KLAVIERSPIELER BITTE NICHT MIT LEBENSMITTELN BEWERFEN",
	"CURRYWURST 3.80. POMMES FRITES 2.30., FRIKADELLEN 1.50",
	"DIE BESTEN HAMBURGER DIESSEITS DER DONAU FueR NUR 3.80 DM",
	"EIN SCHoeNER TOTENKOPF MIT EINEM ZIEMLICH DURCHDRINGENDEN BLICK, HI, HI, SUPERGUT!",
	"HALLO TOTENKOPF, DU ERINNERST MICH AN HAMLETS ONKEL!",
	// 170
	"ICH HABE DIE ANGEWOHNHEIT, NICHTS ANZUFASSEN, WAS MAL LEBENDIG GEWESEN IST",
	"EIN PAPIERKORB",
	"SIE WETTEN, WIE DAS SPIEL HEUTE ABEND AUSGEHT",
	"WAS SICH WOHL DAHINTER VERBERGEN MAG",
	"EJ, DIESER VORHANG BEWEGT SICH NICHT!",
	// 175
	"GANZ SCHoeN GRUSELIG, DAS SCHLOSS DA, WAS?",
	"ICH SCHAFFS NICHT, ER IST ZU WEIT WEG, UM MICH ZU HoeREN",
	"EIN TYPISCH TRANSSYLVANISCHER WALD, MIT BaeUMEN UND SO",
	"RED KEINEN BLoeDSINN, ES IST DOCH STOCKDUNKEL!",
	"KONDITOREI MueLLER. GEBaeCK UND KAUGUMMIS",
	// 180
	"EINE SEHR SCHoeNE TueR",
	"GESCHLOSSEN",
	"EINE TOTAL GESCHLOSSENE TONNE",
	"",
	"WAS FueR SueSSE VIECHER!",
	// 185
	"PSSS,PSSS,PSSS, MIETZCHEN...",
	"KEINE REAKTION",
	"DER MOND IST EIN SATELLIT DER DIE ERDE IN EINER UMLAUFGESCHWINDIGKEIT VON 28 TAGEN UMKREIST.",
	"HALLO MOND DU KREISRUNDES MONDGESICHT",
	"TOTAL MIT BRETTERN ZUGENAGELT",
	// 190
	"NICHTS ZU MACHEN. HIER KOMMT NOCH NICHT MAL ARNOLD S. MIT DER KREISSaeGE DURCH",
	"SCHEINT SO, ALS HaeTTE DIE TANNE DA EINEN ueBERDIMENSIONALEN SCHATTEN!",
	"HEHOO, WIRT!",
	"SIND NOCH ZIMMER FREI?",
	"KoeNNEN SIE MIR SAGEN WO ICH EINEN GEWISSEN GRAF DRASCULA FINDE?",
	// 195
	"JA, WAS GIBTS?",
	"UND WIESO?",
	"WW...WIRKLICH?",
	"GUTE FRAGE, ICH WERDE IHNEN MAL MEINE STORY ERZaeHLEN, ALSO...",
	"NUR FueNF MINUTEN!",
	// 200
	"ICH HEISSE HEINZ HACKER UND BIN VERTRETER EINER NORDDEUTSCHEN IMMOBILIENFIRMA",
	"SO WIES AUSSIEHT WILL GRAF DRASCULA EINIGE HEKTAR LAND AUF HELGOLAND KAUFEN UND ICH BIN HIER UM ueBER DIE PREISE ZU VERHANDELN",
	"MMMMH, ICH GLAUBE DASS ICH MORGEN FRueH MIT MEINER MAMI ZURueCKKOMME",
	"WAS FueR EINE NACHT, NICHT WAHR?",
	"NICHTS, SCHON GUT",
	// 205
	"HEEHOOO, KLAVIERSPIELER",
	"WAS FueR EINE NACHT",
	"UND ES IST AUCH GAR NICHT SO KALT",
	"NUN GUT, ICH LASS DICH JETZT MAL IN RUHE WEITERSPIELEN",
	"GENAU",
	// 210
	"HALLO CHEF, ALLES KLAR?",
	"UND DIE FAMILIE?",
	"HIER IST MaeCHTIG WAS LOS, WAS?",
	"ICH HALTE BESSER MEINEN MUND",
	"ZU HAUSE IST ES IMMER NOCH AM SCHoeNSTEN...ZU HAUSE IST ES IMMER....Hae? ABER SIE SIND DOCH NICHT TANTA EMMA. WO ICH DOCH GAR KEINE TANTE EMMA HABE!",
	// 215
	"JA, MEINER AUCH. SIE KoeNNEN MICH NENNEN WIE SIE WOLLEN, ICH HoeRE SELBST AUF HEINZCHEN WIE AUF KOMMANDO.",
	"JA, ICH HAB GANZ SCHoeNE BEULEN, WAS? ueBRIGENS, WO BIN ICH?",
	"JA",
	"VERDAMMT.....",
	"JA, KLAR, SELBSTVERSTaeNDLICH!",
	// 220
	"DANN ALSO VIELEN DANK FueR DEINE HILFE. ICH WERDE DICH NICHT WEITER BELaeSTIGEN. WENN DU MIR BITTE NOCH SAGEN WueRDEST, WO DIE TueR IST.... ",
	"WEIL DIE BEULE WOHL MEIN GEHIRN ANGEGRIFFEN HAT UND ICH EINFACH ueBERHAUPT NICHTS SEHE",
	"PAH, MACHT NICHTS. ICH HABE IMMER ERSATZ DABEI",
	"WOW, WAS FueR EIN SCHARFES WEIB! OHNE BRILLE HABE ICH DAS GAR NICHT BEMERKT!",
	"EJ, DU....",
	// 225
	"UND WAS IST DAAAAAS?!",
	"MACH DIR KEINE SORGEN, BJ, MEINE GELIEBTE! ICH WERDE DICH AUS SEINEN KLAUEN BEFREIEN",
	"ACH KOMM, ER HAT MICH AUF DIE PALME GEBRACHT",
	"UUAAA, EIN WERWOLF! KREPIER, DU MONSTER!",
	"JA, DAS....",
	// 230
	"JA, DAS.... ICH GLAUB, ICH GEH DANN MAL WEITER, MIT VERLAUB...",
	"WAS?",
	"WENN ICHS MIR RECHT ueBERLEGE, UM EHRLICH ZU SEIN, ICH GLAUBE NEIN",
	"SAG MIR OH GEBILDETER PHILOSOPH, GIBT ES IRGENDEINEN KAUSALZUSAMMENHANG ZWISCHEN DER LICHTGESCHWINDIGKEIT UND SCHINKENSPECK",
	"OKAY, OKAY,VERGISS ES. ICH WEISS JEDENFALLS NICHT, WARUM ICH DAS GESAGT HABE",
	// 235
	"WAS PHILOSOPHIERST DU HIER HERUM, WO DU DOCH MENSCHEN FRESSEN MueSSTEST!",
	"HaeH",
	"EJ, KANNST DU DAS MIT DEN PRaeHISTORISCHEN NEIGUNGEN NOCHMAL ERKLaeREN?",
	"JA, MANN. DER KRAM MIT DEM DU MIR EBEN GEKOMMEN BIST. ICH HAB DAS NaeMLICH NICHT GANZ KAPIERT....",
	"ICH HALTE BESSER MEINEN MUND, WEIL SOBALD ER BLUT RIECHT...",
	// 240
	"JA, BITTE?",
	"JA, WAS GIBTS?",
	"ACH, WO SIES GERADE ERWaeHNEN,....",
	"",
	"ueBRIGENS, WAS WueRDE EIGENTLICH PASSIEREN, WENN DAS REZEPT ZUFaeLLIG IN DIE HaeNDE EINES VAMPIRS GERATEN WueRDE...",
	// 245
	"NA WAS SOLLS. HoeR MAL, KOMMT DIR DAS NICHT AUCH WIE EIN DaeMLICHER KLEINER TRICK VOR, UM DAS SPIEL SCHNELL ZU BEENDEN? NAJA, VIELLEICHT AUCH NICHT",
	"LEER!",
	"WEIL DU MIR B.J., MEINE GELIEBTE, GERAUBT HAST. OHNE SIE HAT MEIN LEBEN KEINEN SINN.",
	"SEIN GEHIRN?",
	"OCH, NUR SO! ICH GLAUBE DASS ICH GENUG VON DEM KLEINEN MONSTERCHEN HABE",
	// 250
	"SueSSE KLEINE JUNGFRAU, ICH BLEIB SO WIE ICH BIN!",
	"DAMIT KOMMST DU NICHT DURCH.. GLEICH KOMMT POPEYE UND HOLT MICH HIER RAUS!",
	"WAS FueR EIN SCHEISS-SPIEL, BEI DEM DEM DER HELD STERBEN MUSS",
	"HALT! WAS IST MIT MEINEM LETZTEN WUNSCH?",
	"HAR, HAR! JETZT BIN ICH IMMUN GEGEN DEIN VERFLUCHTES UNGEHEUER. DAS HIER IST EINE ANTI-VAMPIR-ZIGARETTE,DIE MIR VON BRAUN GEGEBEN HAT.",
	// 255
	"JA, KLAR. ABER ICH WERDE DIR BESTIMMT NICHT DAS REZEPT VERRATEN",
	"MIT FOLTER KOMM ICH GUT KLAR, ICH DENK MIR SOGAR WELCHE AUS",
	"NEIN, BITTE NICHT! ICH SAG ALLES, ABER TUN SIE MIR DAS NICHT AN!",
	"O.K. ICH HAB DIR ALLES ERZaeHLT. JETZT HOL B.J. UND MICH ENDLICH HIER RAUS UND LASS UNS IN RUHE",
	"B.J.! WAS MACHST DU HIER? WO IST DRASCULA?",
	// 260
	"WAS FueR EIN MIESER TYP! NUR WEIL ER BLAUES BLUT HAT, MueSSEN IHM NOCH LANGE NICHT ALLE JUNGFRAUEN DIESER ERDE ZU FueSSEN LIEGEN",
	"NIEDER MIT DEM BONZEN-ADEL!",
	"SOLIDARITaeT MIT ALLEN ARMEN DIESER WELT...!",
	"UND WIES AUSSIEHT HAT ER DICH NACH ALLEN REGELN DER KUNST ANGEKETTET",
	"O.K. DU HAST NICHT ZUFaeLLIG EINE HAARSPANGE?",
	// 265
	"NANANA. STELL DICH NICHT SO AN. MIR FaeLLT SCHON WAS EIN.",
	"HE, WIRT",
	"WER GEWINNT?",
	"WER?",
	"SIEHST DU DENN NICHT, DASS HIER DRASCULA IST?",
	// 270
	"NA, DANN LASS IHN UNS ALLE MACHEN, ODER?",
	"GIB MIR EINEN GIN-TONIC",
	"SCHON GUT. ICH HAB VERGESSEN, WAS ICH DIR SAGEN WOLLTE",
	"GIB\247MIR\247SOFORT\247EINEN\247GIN\247TONIC\247ODER\247ICH\247SPIEL\247KLAVIER",
	"WIELANGE DAUERT DAS SPIEL NOCH?",
	// 275
	"GUTE NACHT",
	"UND WIE KOMMST DU VORAN, IGOR? SCHLEPPEND? HI,HI,HI, KLASSE!",
	"WAS SOLL DAS SEIN, WAS DU DA MACHST?",
	"NEIN",
	"DANN ZIEH DIE BRILLE AUF",
	// 280
	"WAS SOLL DAS MIT DER ueBERNATueRLICHEN ORGIE?",
	"O.K. ES REICHT. ICH KANN ES MIR SCHON VORSTELLEN",
	"KoeNNTEST DU MIR NICHT ZUFaeLLIG SAGEN, WO DRASCULA IST?",
	"ACH KOMM, BITTE, BITTE!",
	"WARUM NICHT?",
	// 285
	"ACH: NACHTS SCHLaeFT ER?",
	"NA, HOFFENTLICH KLAPPT DAS MIT DER EINKOMMENSERKLaeRUNG",
	"ICH MUSS NaeMLICH MIT IHM REDEN",
	"HEYHOO, GERIPPE!",
	"VERDAMMT! EIN SPRECHENDER TOTER!",
	// 290
	"ERaeHL MAL WIESO HAST DU HIER ANGEHALTEN?",
	"UND WOZU SOLLTE DRASCULA EIN MONSTER SCHAFFEN WOLLEN?",
	"WIE HEISST DU, LIEBES GERIPPE?",
	"EJ, SOLL ICH DIR NICHT VIELLEICHT WAS ZU ESSEN BRINGEN?",
	"DU HAST DOCH BESTIMMT EINEN LEEREN MAGEN, HI,HI,HI!",
	// 295
	"ICH HABE GERADE EINFACH KEINEN BOCK, ZU REDEN",
	"FI........(BIEP) DICH DOCH IN DEIN BEFI.......(BIEP) KNIE, DU ALTER WI......(BIEP)!",
	"ICH HABE SIE WIRKLICH GELIEBT, NAJA, GUT, SIe WAR WIRKLICH NICHT DIE HELLSTE VON OSRAM, ABER NOBODY IS PERFECT, STIMMTS?",
	"HEY SIE HATTE EINE WAHNSINNSFIGUR, UND WAS WILLST DU MEHR",
	"NICHTS WIRD MEHR WIE VORHER SEIN. ICH WERDE INS KLOSTER GEHEN UND ELENDIGLICH DAHINSIECHEN.",
	// 300
	"HIER KANN MICH NICHTS UND NIEMAND MEHR RAUSHOLEN, WEIL...",
	"VON WEM? VON WEM?",
	"ICH MoeCHTE PIRAT WERDEN!",
	"ICH MoeCHTE PROGRAMMIERER WERDEN!",
	"ERZaeHL MIR ETWAS ueBER SIEGFRIED UND DIE NIBELUNGEN",
	// 305
	"ICH HAB EUCH NIE GESEHEN UND SPIEL JETZT EINFACH WEITER",
	"WER HAT SICH DENN DIESEN SCHWACHSINN EINFALLEN LASSEN?",
	"EINE HANDTASCHE, WIE SIE MEINE OMI HAT",
	"OH MANN, WIE GUT ICH AUSSEHE!",
	"ICH KoeNNTE MICH GLATT IN MICH VERLIEBEN!",
	// 310
	"UND WIE SOLL ICH MICH ABSCHLIESSEN?",
	"ICH WERDE MICH WOHL ERSTMAL AUFSCHLIESSEN MueSSEN, WAS?",
	"HIER STEH ICH GUT",
	"HAB MICH SCHON",
	"HALLO ICH",
	// 315
	"BEI PASSENDER GELEGENHEIT WERDE ICH SIE MIR MAL REINTUN",
	"ICH SEHE NICHTS BESONDERES",
	"GUT SO",
	"UND WIESOOO?",
	"ICH KANN NICHT",
	// 320
	"HALLO DU DA",
	"DIE GRUFT VON ONKEL DESIDERIUS",
	"HAAALLO, ONKEL DESIDEERIUUS!",
	"NEIN, ICH WILL MICH NICHT SCHON WIEDER SCHNEIDEN",
	"aeHEM,MMM...!",
	// 325
	"MMMM, LECKER,UaeaeHH!",
	"JA, HUST,HUST!",
	"HEY, GUCK MAL, HIER KLEBT EIN KAUGUMMI",
	"DAS SUPERHANDILY DAS ICH ZU WEIHNACHTEN BEKOMMEN HABE",
	"GANZ SCHoeN HOCH",
	// 330
	"TRITT AUF DEN BALKON, JULIA!",
	"DU BIST DER STERN AN MEINEM FIRMAMENT!",
	"EJ, TueR, WAS ISS LOSS?",
	"HEYHOO, TRANSSYLVANISCHES ZIGARRETTENAUTOMaeTCHEN!",
	"EIN ZIGARRETTENSPENDERMASCHINCHEN",
	// 335
	"ICH HAB DA NOCHNE MueNZE DRIN",
	"NEIN, ICH RAUCHE UND TRINKE NICHT MEHR",
	"VON JETZT AN WERDE ICH MICH NUR NOCH DEN FRAUEN WIDMEN",
	"BETRUG! ES IST GAR NICHTS RAUSGEKOMMEN!",
	"ENDLICH!",
	// 340
	"EINE TRUHE, WEITER NICHTS",
	"HALLO TRUHE, DU HEISST WIE MEINE TANTE TRUDE",
	"ICH HABE DIE TASCHE VON B.J. GEFUNDEN",
	"MEIN GOTT, ICH SEH MICH NICHT IM SPIEGEL, ICH BIN EIN VAMPIR!",
	"....ACH, NEE. ES IST NUR EIN GEMaeLDE",
	// 345
	"SPIEGLEIN, SPIEGLEIN AN DER WAND, WER IST DER SCHoeNSTE Im GANZEN LAND?",
	"ER MACHT MIR NICHT AUF",
	"GUT. ICH HABE DIE OHRENSToePSEL DRIN",
	"EIN VON DER UNIVERSITaeT ZU CAMBRIDGE AUSGESTELLTES VAMPIR-JaeGER DIPLOM.",
	"NEIN, MIR FEHLEN NOCH EINIGE ZUTATEN, ES LOHNT SICH NICHT, IHN ZU WECKEN!",
	// 350
	"ICH HABE NaeMLICH KEIN GELD",
	"EINE NORDDEUTSCHE LAMPE",
	"WIRT! HELFEN SIE MIR!",
	"DA STAND PLoeTZLICH EIN VAMPIR UND HAT MEINE FREUNDIN MITGENOMMEN.",
	"ABER WOLLEN SIE MIR DENN NICHT HELFEN?",
	// 355
	"TOT? WAS WOLLEN SIE DAMIT SAGEN?",
	"aeHEM!",
	"EIN VAMPIR HAT DAS MaeDEL VON 506 ENTFueHRT!",
	"DU MUSST MIR HELFEN!",
	"KANNST DU KEIN BEERDIGUNGSLIED SPIELEN?",
	// 360
	"WIE HaeLST DU DAS BLOSS AUS: JEDEN TAG DIE GLEICHEN LIEDER",
	"AHA, UND WIE HoeRST DU MICH DANN?",
	"GIB MIR DIE OHRENSToePSEL",
	"ACH KOMM, ICH GEB SIE DIR AUCH SOFORT WIEDER",
	"ACH KOOOOMMM!",
	// 365
	"AUF WIEDERSEHEN. ICH MUSS EINEN VAMPIR ToeTEN",
	"",
	"REDEST DU AUF TRANSSYLVANISCH ODER WAS?",
	"WER IST ONKEL DESIDERIUS?",
	"WAS SOLL LOS SEIN MIT DIESEM GRAF DRASCULA?",
	// 370
	"WER IST DIESER VON BRAUN?",
	"UND WARUM MACHT ER ES NICHT?",
	"UND WO KANN ICH VON BRAUN FINDEN?",
	"ALSO DANKE UND AUF WIEDERSEHEN, SCHLAF SCHoeN",
	"WIR SOLLTeN BESSER VORHER KLINGELN",
	// 375
	"SIND SIE PROFESSOR VON BRAUN?",
	"UND SIE KoeNNTEN MIR NICHT VERRATEN, WO.....?",
	"ICH NEHM IHM NICHT AB, DASS ER HELGE SCHNEIDER IST",
	"PROFESSOR!",
	"HELFEN SIE MIR! ES GEHT UM DAS LEBEN MEINER GELIEBTEN!",
	// 380
	"SCHON GUT, ICH BRAUCHE IHRE HILFE NICHT",
	"OK. ICH GEHE!",
	"KEINE ANGST. GEMEINSAM BESIEGEN WIR DRASCULA",
	"UND WARUM HELFEN SIE MIR DANN NICHT?",
	"ICH HABE SIE",
	// 385
	"ABER ICH HABE SIE DOCH!",
	"EINVERSTANDEN",
	"...aeHM,...JA",
	"ICH GEHE DIREKT NOCHMAL INS AQUARIUM",
	"ICH BIN SO WEIT, MICH IHREM TEST ZU STELLEN",
	// 390
	"SCHON GUT, ALTERCHEN. ZAHLTAG!",
	"SCHON GUT. ICH WOLLTE SOWIESO GERADE GEHEN",
	"TSCHULDIGE",
	"WIE FINDEST DU DIESES BUCH? KLAVIERSTueCKE VON TSCHAIKOWSKY",
	"WIE BRINGT MAN EINEN VAMPIR UM?",
	// 395
	"HAT MAN DIR NICHT GESAGT WIE MAN SICH BETTET SO LIEGT MAN?",
	"GENAU DAS SAGT MEINE MUTTER IMMER ZU MIR",
	"WARUM KONNTE DICH DRASCULA NICHT ERLEDIGEN?",
	"UND WAS DANN?",
	"SUPER! DA HABEN SIE EIN IMMUNITaeTSMISCH....!",
	// 400
	"ALSO?",
	"SEHR GUT",
	"KoeNNEN SIE NOCHMAL WIEDERHOLEN WAS ICH FueR DIESE MISCHUNG BRAUCHE?",
	"DANN SATTEL ICH MAL DIE HueHNER UND BESORG ES",
	"ENTSCHULDIGUNG, WO IST DER KLAVIERSPIELER?",
	// 405
	"ICH\247HAB\247SCHON\247ALLE\247ZUTATEN\247FueR\247DIE\247MISCHUNG",
	"EINE FRAGE: WAS SOLL DAS HEISSEN: REIPERK ALUCSARD?",
	"SAGEN SIE SCHON...",
	"UND WO SOLL DIESE GROTTE SEIN?",
	"WAS IST? GABS BEI EUCH KEINE HEXENPROZESSE?",
	// 410
	"...ABER WENN ICH NOCH MEHR VAMPIREN BEGEGNE?",
	"EIN VAMPIR, DER MICH NICHT DURCHLaeSST",
	"ER SIEHT WIE AUS WIE JODA, NUR VIEL GRoeSSER",
	"EJ, JODA. WENN DU MICH DURCHLaeSST, GEB ICH DIR NE MARK",
	"NA GUT, DIR KANN MAL WOHL NICHTS ERZaeHLEN",
	// 415
	"HALLO, VAMPIR, WAS FueR EINE NACHT, WAS?",
	"HAT MAN DIR SCHON MAL GESAGT, DASS DU AUSSIEHST WIE JODA?",
	"BIST DU EIN VAMPIR ODER EIN oeLGEMaeLDE?",
	"ICH HALT BESSER MEINEN MUND, SONST REGST DU DICH NOCH AUF",
	"ABGESCHLOSSEN",
	// 420
	"DIE ELSTER KoeNNTE MIR EIN AUGE RAUSREISSEN, WENN ICHS VERSUCHE!",
	"ZU! MEIN GOTT, WIE UNHEIMLICH!",
	"DIE SCHARNIERE SIND ZUGEROSTET",
	"HIER IST NUR EIN PaeCKCHEN MEHL",
	"DAS HAT DEN ROST ENTFERNT",
	// 425
	"ICH HABE EINEN PFLOCK AUS FICHTENHOLZ GEFUNDEN",
	"ICH NEHM DAS HIER, DAS IST SCHoeN DICK!",
	"NA, ICH GLAUBE ICH KANN MICH JETZT MAL AUS DIESER DaeMLICHEN VERKLEIDUNG SCHaeLEN",
	"EINGaeNGE ZU DEN SEITENFLueGELN WEGEN BAUARBEITEN GESPERRT. BENUTZEN SIE BITTE DEN HAUPTEINGANG",
	"...ER IST BLASS, HAT KOMISCHE ZaeHNE, TRaeGT EIN TOUPET UND EINEN UMHANG...DAS MUSS DRASCULA SEIN!",
	// 430
	"ES IST B.J.! B.J. ALLES O.K?",
	"ICH WEISS SCHON DASS SIE DUMM IST, ABER ICH BIN SO EINSAM",
	"DU HAST BESTIMMT KEINEN SCHLueSSEL, STIMMTS?",
	"ICH WETTE, DU HAST KEINEN DIETRICH",
	"GIB MIR DIE HAARSPANGE. ICH MACHS WIE MCGYVER",
	// 435
	"BEWEG DICH NICHT, ICH BIN SOFORT ZURueCK",
	"VERDAMMT! ER IST MIR ABGEBROCHEN!",
	"HEYJEYJEY! UND ICH HAB MICH AUCH RASIERT, ALTER!",
	"JA, SCHATZ?",
	"ES REICHT NICHT",
	// 440
	"DER KLAVIERSPIELER IST NICHT DA",
	"EIN TRANSSYLVANISCHER GIN-TONIC",
	"ICH HABE IMMER NOCH KEIN ZIMMER",
	"ES SCHEINT DASS ER IN DER BADEWANNE HaeNGENGEBLIEBEN IST UND DANN EINE KNEIPE AUFGEMACHT HAT",
	"ER IST VOLL WIE EIN RUSSE",
	// 445
	"DIESES HAAR.... TJA,ES ERINNERT MICH AN JEMANDEN",
	"ES IST EIN KNoeCHRIGES GERIPPE",
	"GUCK MAL! BRAD PITT!",
	"ER SCHLaeFT. ES WaeRE WIRKLICH SCHADE, IHN ZU WECKEN",
	"ER IST HaeSSLICHER ALS DIDI HALLERVORDEN",
	// 450
	"EIN SARG AUS FICHTENHOLZ",
	"ER WIRD MICH IN LECKERE KLEINE SCHEIBCHEN SCHNEIDEN",
	"ICH STEH NICHT SO AUF PENDEL. ICH FIND OHRFEIGEN BESSER",
	"ICH KANN NICHT. ICH HAB HANDSCHELLEN AN",
	"DAS SIEHT DOCH JEDER, DASS DAS EINE GEHEIMTueR IST",
	// 455
	"SIE IGNORIEREN MICH",
	"ACH KOMM!",
	"LAUT DREHBUCH BEWEGE ICH MICH, ABER DAS SPIEL WAR SO TEUER, DASS NICHTS MEHR FueR EIN FITNESS-STUDIO ueBRIG WAR, ALSO HABE ICH AUCH KEINEN STRAMMEN ARSCH.",
	"SCHEINT SO, ALS OB ER EIN BISSCHEN LOSE AN DER WAND HaeNGT",
	"ICH GLAUBE KAUM, DASS SIE MIR NueTZEN WIRD. SIE IST VIEL ZU FEUCHT, UM SIE ANZUZueNDEN",
	// 460
	"ZUM WESTFLueGEL? ICH BIN DOCH NICHT BLoeD! WER WEISS WAS MICH DA ERWARTET!",
	"MIT HueBSCHEN TRANSSYLVANISCHEN MOTIVEN",
	"",
	"WIE SCHADE, DASS DAS DA KEIN LECKERER LAMMBRATEN DRIN IST.",
	"ALS ICH BEIM LETZTEN MAL IN EINEN OFEN GEGUCKT HABE, IST DAS HAUS IN DIE LUFT GEFLOGEN",
	// 465
	"DAS IST DAS WAPPEN DER TRANSSYLVANISCHEN FUSSBALLMANNSCHAFT",
	"UND WOZU? SOLL ICH MIR DAS ANS KNIE BINDEN?",
	"DIESE SORTE SCHUBLADE LaeSST SICH NICHT oeFFNEN",
	"ICH MoeCHTE ECHT NICHT WISSEN, WAS ES DA DRIN ZU ESSEN GIBT!",
	"DAS WIRKT ZIEMLICH IMPRESSIONISTISCH AUF MICH",
	// 470
	"DIE NACHT VERSCHLUCKT ALLES... WIE UNHEIMLICH, WAS?",
	"SIE KLEMMT",
	"DER KoeNIG HAST DU DIR DAS NICHT DENKEN KoeNNEN?",
	"NEIN, ICH HAB SCHON EINEN ZU HAUSE, DEN ICH FueTTERE UND SO",
	"EIN REGAL MIT BueCHERN UND ANDEREN SACHEN",
	// 475
	"UND WEN SOLL ICH UM DIESE UHRZEIT NOCH ANRUFEN?",
	"\"EINKOMMENSERKLaeRUNG LEICHT GEMACHT\" WIE INTERESSANT!",
	"ICH HAB SCHON EINS ZU HAUSE. ICH GLAUBE DAS IST WELTWEIT EIN BESTSELLER",
	"EIN TOTAL NORMALER SCHLueSSEL",
	"ICH GLAUBE, DER GEHoeRT HIER NICHT HIN",
	// 480
	"HEY, DIE FRITTEN SEHEN AUS WIE ECKZaeHNE! ICH BIN HIN UND WEG",
	"DAS IST WOHL ECHT NICHT DER MOMENT, RUMZUNASCHEN, WENN MEINE FREUNDIN IN DEN KLAUEN DER FIESESTEN AUSGEBURT DER WELT IST.",
	"WAS FueR EINE RIESENGAUDI DAS MACHT, DAMIT VAMPIRE FERTIGZUMACHEN!",
	"MAL GUCKEN, OB NOCH EINER AUFTAUCHT",
	"NEIN, ES MUSS MIT NOCH SO EINEM DRECKIGEN, STINKENDEN UND NERVIGEN VAMPIR WIE VORHIN SEIN",
	// 485
	"DAS IST DIE ORIGINALPERueCKE VON ELIVS ALS ER SCHON EINE GLATZE HATTE",
	"MEHL, ABER ICH KANN JETZT KEINE MARKEN NENNEN",
	"VIELLEICHT EIN ANDERES MAL, OK?",
	"EINE WUNDERSCHoeNE AXT. ZU SCHADE, DASS GERADE KEIN VAMPIRKOPF IN DER NaeHE IST",
	"NEIN, IM GRUNDE BIN ICH EIN NETTER MENSCH",
	// 490
	"DAS IST DAS DEO VON MAGGIE THATCHER, HIHIHI!",
	"EIN ZIEMLICH HueBSCHER UMHANG",
	"",
	"WIE JEDER BELIEBIGE AST EINES BELIEBIGEN BAUMES AN JEDEM BELIEBIGEN ORT IN DER WELT, NICHTS BESONDERES ALSO",
	"\"UNGLAUBLICH\" EIN SEIL IN EINEM VIDEOSPIEL!",
	// 495
	"WOZU DAS WOHL NOCH GUT SEIN WIRD...",
	"EIN SEIL AN EINEM AST ODER EIN AST AN EINEM SEIL, JE NACHDEM WIE MAN DAS SO BETRACHTET",
	"ES SCHEINT, DAS DIESE ELSTER WAS ueBLES IM SCHILDE FueHRT",
	"ZISCH AB, ICH SAG IHR NICHTS, MAL SEHEN OB SIE AUSFLIPPT",
	"SIEHT SO AUS ALS WaeRE SIE TOT, ABER STIMMT NICHT, NE?",
	// 500
	"FueR DIESES SPIEL WURDEN KEINE TIERE MISSHANDELT ODER GEToeTET",
},
{
	// 0
	"",
	"C'EST LA DEUXI\212ME PORTE PLUS GRANDE QUE J'AI VUE DANS MA VIE.",
	"ENFIN, PAS AUTANT QUE \207A.",
	"ELLE EST BOUCH\202E AVEC DES GROSSES PLANCHES. L'\202GLISE EST PEUT-\210TRE ABANDONN\202E DEPUIS QUELQUES ANN\202ES.",
	"MAIS JE NE L'AI PAS OUVERTE.",
	// 5
	"QU'EST-CE QUE JE FAIS? JE L'ARRACHE?",
	"BONJOUR, PORTE. JE VAIS T'ENCADRER.",
	"C'EST TROP POUR MOI.",
	"UNE FEN\210TRE BOUCH\202E AUX GROSSES PLANCHES.",
	"JE N'ARRIVE PAS.",
	// 10
	"\200A Y EST.",
	"ET POURQUOI?",
	"SALUT, FEN\210TRE! AS-TU QUELQUE CHOSE \205 FAIRE CE SOIR?",
	"PAS SANS LE PERMIS DE TRAVAUX PUBLIQUES.",
	"H\202! CETTE FEN\210TRE A SEULEMENT UNE GROSSE PLANCHE...",
	// 15
	"OH\202! OH\202!-FEN\210TRE!",
	"BONJOUR, TOI.",
	"",
	"JE N'ARRIVE PAS.",
	"C'EST BIEN O\227 ELLE EST.",
	// 20
	"",
	"C'EST UNE TOMBE EN FORME DE CROIX.",
	"NON, MERCI.",
	"BONJOUR, LE D\202FUNT: VEUX-TU DES VERMISSEAUX?",
	"MAIS OUI. COMME EN POLTERGUEIST.",
	// 25
	"",
	"",
	"JE REVIENS EN QUINZE MINUTES.",
	"D\202FENSE D'AFFICHER.",
	"",
	// 30
	"C'EST FERM\202 \205 CL\202.",
	"J'EN AI D\202J\205 UN.",
	"",
	"IL NE R\202POND PAS.",
	"MAIS NON, C'EST BIEN GAR\202.",
	// 35
	"C'EST UNE PORTE.",
	"UN TIROIR DE LA TABLE.",
	"UNE ARMOIRE SUSPECTE.",
	"BONJOUR, L'ARMOIRE. \200A VA?",
	"",
	// 40
	"",
	"C'EST UN CAND\202LABRE TR\212S VIEUX.",
	"IL DOIT \210TRE L\205 D\212S QUE MAZINGUER-Z \202TAIT UNE VIS.",
	"NON, C'EST UNE RELIQUE.",
	"C'EST UN JOLI R\202TABLE.",
	// 45
	"",
	"HI! HI! HI!",
	"",
	"NON.",
	"",
	// 50
	"HA! HA! HA! -QUE C'EST BON!",
	"",
	"",
	"",
	"JE NE VOIS RIEN DE SP\202CIAL.",
	// 55
	"C'EST FERNAN, LA PLANTE.",
	"C'EST UNE DES PIQUES DE LA GRILLE.",
	"H\202! L\205-DESSOUS IL Y A UNE BO\214TE D'ALLUMETTES.",
	"REGARDE! UN PAQUET DE CLINEX. -ET IL Y A UN TOUT NEUF!",
	"IL N'Y A RIEN DE PLUS DANS LE SEAU.",
	// 60
	"C'EST UN AVEUGLE QUI VE VOIT PAS.",
	"",
	"",
	"",
	"",
	// 65
	"C'EST UNE GROSSE SOMME D'ARGENT.",
	"",
	"",
	"",
	"",
	// 70
	"",
	"",
	"",
	"",
	"",
	// 75
	"",
	"",
	"",
	"",
	"",
	// 80
	"",
	"",
	"",
	"",
	"",
	// 85
	"",
	"",
	"",
	"",
	"",
	// 90
	"",
	"",
	"",
	"",
	"",
	// 95
	"",
	"",
	"",
	"",
	"",
	// 100
	"ELLE N'A RIEN DE SP\220CIAL",
	"CELA N'A RIEN D'EXTRAORDINAIRE",
	"QU'EST-CE QU'IL Y A?",
	"BONJOUR!",
	"RIEN DE NOUVEAU?",
	// 105
	"LA FAMILLE, \200A VA?",
	"-QUELLES CHOSES TU AS!",
	"MAIS, COMME JE VAIS PRENDRE CELA!",
	"MA RELIGION ME L'INTERDIT",
	"CE N'EST PAS MIEUX",
	// 110
	"BIEN S\352R, MON VIEUX!",
	"ON NE PARLE PLUS DE CELA",
	"IMPOSSIBLE",
	"CELA NE S'OUVRE PAS",
	"JE NE PEUX PAS TOUT SEUL",
	// 115
	"SI JE VOULAIS, J' ARRIVERAIS, MAIS \200A ME DONNE DE LA PARESSE",
	"JE N'Y VOIT PAS UNE RAISON APPARENTE",
	"C'EST UN CERVEAU ASSEZ BIEN",
	"ET BIEN CERVEAU, QUE PENSES-TU FAIRE CE SOIR?",
	"NON, ON DOIT LE GARDER DANS UN ENDROIT \267 L'ABRI DES MUTATIONS DE L'ATMOSPH\324RE",
	// 120
	"C'EST UN DUR, COMME MON CHEF",
	"C'EST UN PIEU TR\324S AIGUIS\220",
	"FID\324LE PIEU POINTUUU, NOBLE CH\322NE TRANSYLVAAAN",
	"TIENS! JE DOIS COUPER MES ONGLES!",
	"B.J. EST L\267-DEDANS. ET QUELLE EST MIGNONE CETTE NANA!",
	// 125
	"ELLE EST FERM\220E TR\324S SOLIDEMENT",
	"\"CADENAS SOLIDES S.A.\"",
	"C'EST LE TYPIQUE SQUELETTE QU'ON TROUVE DANS LES GE\342LES DE TOUS LES JEUX",
	"ON L' EMPLOIE NORMALEMENT POUR  INDUIR DU COURANT \220LECTRIQUE AUX APPAREILS QU'Y SONT RACCORD\220S",
	"C'EST ABSOLUMENT ARTISANAL, CAR LES JAPONAIS LES FONT MAINTENANT DE POCHE",
	// 130
	"J'AI SEULEMENT VU DANS MA VIE UNE CHOSE SI MOCHE",
	"LAISSE. JE NE LUI DIS RIEN POUR S'IL SE F\266CHE",
	"IL SEMBLE ASSEZ RATIONNEL",
	"C'EST UNE PHOTO DE PLATON EN TRAIN D'\220CRIRE SON DIALOGUE PERDU",
	"JE NE SUIS PAS DE CEUX QUI PARLENT AUX  POSTERS",
	// 135
	"UN BUREAU ASSEZ MIGNON",
	"C'EST UN DIPL\342ME DE CHASSE-CHASSE-VAMPIRES HOMOLOGU\220 PAR L'UNIVERSIT\220 D'OXFORD",
	"C'EST UNE NUIT NOIRE AU PLEINE LUNE",
	"IL PARA\327T  QUE CES VIS NE SONT PAS TR\324S ENFONC\220ES",
	"N'Y REGARDES PAS, MAIS JE CROIS QU'UNE CAM\220RA  OCCULTE ME VISE",
	// 140
	"UN D\220TECTEUR DE PIEUX ASSEZ MODERNE",
	"NON, LE LABORATOIRE EST AU DEUXI\324ME \220TAGE",
	"UNE JOLIE TABLE DE NUIT",
	"C'EST UN TAS D'ARGENT QUI NE PEUT PAS MANQUER DANS UNE AVENTURE DIGNE DE TEL NOM",
	"SI J'\220TAIS RICHE. DUBIDOUDUBIDOUDUBIDOUDUBIDOU",
	// 145
	"CE SONT DES FEUILLES BIZARRES. ON A D\352 LES AMENER DE L'AM\220RIQUE DU SUD OU PAR L\267",
	"JE NE PENSE PAS QU'ILS VONT ME R\220PONDRE",
	"C'EST UN JOLI CRUCIFIX EN BOIS. L'IC\342NE N'ARRIVE PAS \267 SAISIR TOUT LA SPLENDEUR DE SA BEAUT\220",
	"JE PRIE SEULEMENT AVANT DE ME COUCHER",
	"H\220!, IL PARA\327T QUE CETTE PIQUE S'EST UN PETIT PEU D\220CROCH\220E",
	// 150
	"TU NE POURRAS TE PLAIGNER APR\324S DU PEU DE PISTES QUE JE TE DONNE",
	"C'EST UNE PIQUE ASSEZ CONVENTIONNELLE",
	"ILS SONT MIGNONS, MAIS UN PEU SALES",
	"NON, NON, ILS NE ME \220COUTERAIENT PAS. HI, HI, HI -QUE C'EST BON!",
	"\"LA BELLE DORMANTE DU BOIS\" DE TCHA\330KOVSKI, OU TCHA\330FROSKI, OU N'IMPORTE COMMENT DIT-ON ",
	// 155
	"TR\324S APP\220TISSANT",
	"JE NE SUIS PAS DE CEUX  QUI SUCCENT DES CHEWING-GUMS D\220J\267 M\266CH\220S",
	"UNE FAUCILLE TR\324S MIGNONE. jE ME DEMANDE O\353 SERA LE MARTEAU?",
	"\"LES FABRICANTS DE TABAC AVERTISSENT QUE LES AUTORIT\220S SANITAIRES SONT S\220RIEUSEMENT NUISIBLES POUR LA SANT\220 \"",
	"UNE BOUGIE COURANTE ET NORMALE ET AVEC DE LA CIRE M\322ME",
	// 160
	"IL FAUT VOIR COMME ELLES LUISENT CES DEUX RUTILANTES MONNAIES!",
	"IL FAUT VOIR COMME ELLE LUIT CETTE RUTILANTE MONNAIE!",
	"AVEC \200A JE SERAI IMMUNIS\220 CONTRE LES MORSURES DES VAMPIRES",
	"NON, CE N'EST PAS ENCORE LE MOMENT",
	"IL Y A UN BILLET DE MILLE ET DEUX SOUS",
	// 165
	"ON DIT\"VOUS \322TES PRI\220S DE NE PAS DONNER \267 MANGER AU PIANISTE\"",
	"L'OMELETTE, 200. DES PETITS POISSONS FRITS, 150, DES POMMES A\330OLI, 225",
	"LES MEILLEURES HAMBURGERS DE CE C\342T\220 DU DANUBE, SEULEMENT 325",
	"C'EST UNE JOLIE T\322TE DE MORT AU REGARD TR\324S PER\200ANT -HI, HI, HI, QUE C'EST BON!",
	"BONJOUR T\322TE DE MORT, TU ME RAPPELLES L'ONCLE HAMLET",
	// 170
	"J'AI LA HABITUDE DE NE PAS TOUCHER AUX CHOSES QUI ONT V\220CU AUTREFOIS",
	"C'EST UNE POUBELLE",
	"C'EST UNE MASSUE POUR LE MATCH DE CE SOIR",
	"JE ME DEMANDE CE QU'IL AURA DERRI\324RE",
	"H\220, CE RIDEAU NE BOUGE PAS!",
	// 175
	"TIENS, QUEL CH\266TEAU SI SOMBRE, H\220? ",
	"JE NE PEUX PAS, IL EST TROP LOIN POUR M'ENTENDRE",
	"C'EST UNE TYPIQUE FOR\322T TRANSYLVANE, AVEC DES ARBRES",
	"MAIS TU NE DIS QUE DES B\322TISES, C'EST UN LIEU OBSCUR!",
	"CONFISERIE GARCIA, G\266TEAUX ET CHEWING-GUMS.",
	// 180
	"UNE PORTE TR\324S JOLIE",
	"ELLE EST FERM\220E",
	"UN TONNEAU COMPL\324TEMENT FERM\220",
	"",
	"QUELLES BESTIOLES SI MIGNONES!",
	// 185
	"BSSST, BSSST, PETIT CHAT...",
	"IL NE R\220POND PAS",
	"LA LUNE EST UN SATELLITE TOURNANT AUTOUR DE LA TERRE AVEC UNE P\220RIODE DE ROTATION DE 28 JOURS",
	"SALUT, LUNE!, LUN\220E ET \220TOURDIE ",
	"ELLE EST COMPL\324TEMENT BOUCH\220E AVEC DES GROSSES PLANCHES",
	// 190
	"C'EST IMPOSSIBLE, CECI NE L'OUVRE PAS NI LE MAJORDOME DE LA T\220L\220",
	"H\220, IL PARA\327T QUE L'OMBRE DE CE CYPR\324S-LA EST ALLONG\220E",
	"OH\220!  H\342TELIEEER!",
	"JE VOUDRAIS UNE CHAMBRE",
	"SAVEZ-VOUS O\353 EST QUE JE PEUX TROUVER UN TEL COMTE DRASCULA?",
	// 195
	"OUI, QU'EST-CE QU'IL Y A? ",
	"ET \200A?",
	"EN... EN V\220RIT\220?",
	"UNE BONNE QUESTION, JE VAIS VOUS RACONTER MON HISTOIRE, TIENS...",
	"JE N'AI QUE POUR CINQ MINUTES",
	// 200
	"JE M'APPELLE JOHN HACKER ET SUIS LE REPR\220SENTANT D'UNE INMOBILI\324RE BRITANNIQUE",
	"IL PARA\327T QUE LE COMTE DRASCULA VEUT ACHETER DES TERRAINS \267 GIBRALTAR ET ON M'A ENVOY\220 POUR N\220GOCIER L'AFFAIRE",
	"MAIS JE PENSE QUE DEMAIN DE BONNE HEURE JE RETOURNE AVEC MA MAMAN",
	"UNE BELLE NUIT,  N'EST-CE PAS?",
	"NON, RIEN",
	// 205
	"OH\220! PIANISTE!",
	"UNE BELLE NUIT",
	"ET EN PLUS, IL NE FAIT PAS FROID",
	"EH BIEN, RIEN. CONTINUE \267 JOUER",
	"C'EST \200A",
	// 210
	"BONJOUR CHEF, \200A VA?",
	"ET LA FAMILLE?",
	"IL Y A DE L'AMBIANCE ICI, H\220?",
	"TANT MIEUX SI JE NE DIS RIEN",
	"ON EST MIEUX CHEZ-SOI QU'AILLEURS...  ON EST MIEUX DANS... H\220? MAIS VOUS N'\322TES PAS TANTE EMMA. MIEUX ENCORE. -SI JE N'AI PAS AUCUNE TANTE EMMA!",
	// 215
	"OUI, LE MIEN AUSSI. VOUS POUVEZ M'APPELLER COMME VOUS VOULEZ, MAIS SI VOUS M'APPELLEZ JOHNY, JE VIENS COMME LES CHIENS",
	"OUI, QUELS COUPS QUE J'AI, N'EST-CE PAS? EN FAIT, O\353 SUIS-JE?",
	"OUI",
	"A\330E!, A\330E!...",
	"OH, OUI! BIEN S\352R",
	// 220
	"EH BIEN! MERCI BEAUCOUP POUR TON AIDE. JE NE TE D\220RANGE PLUS SI  TU ME DIS O\353 SE TROUVE LA PORTE, S'IL TE PLA\327T...",
	"CAR LE COUP A D\352 ME TOUCHER LA CERVELLE ET JE N'Y VOIS GOUTTE",
	"BAH!, \200A FAIT RIEN. J'AI TOUJOURS UNE DE R\220CHANGE",
	"OUAH, QUELLE BELLE FEMME! -JE NE M'AVAIS PAS RENDU COMPTE! BIEN S\352R, SANS LES LUNETTES...",
	"\220COUTE...",
	// 225
	"ET \200AAAAAA?!",
	"NE T'EN FAIS PAS B. J., MON AMOUR! JE VAIS TE SAUVER DES GRIFFES DE CELUI-L\267 ",
	"IL ME CASSE LE NEZ, TIENS!",
	"AHHH, UN HOMME-LOUP! MEURS MAUDIT!",
	"OUI, C'EST CELA...",
	// 230
	"OUI, C'EST CELA... JE CROIS QUE JE VAIS SUIVRE MON CHEMIN. PARDON.. ",
	"QUOI?",
	"EH BIEN, EN V\220RIT\220, BIEN PENS\220... JE NE CROIS PAS",
	"DIS -MOI, OH! \220RUDITE PHILOSOPHE! Y A-T-IL UNE R\220LATION CAUSE-EFFET ENTRE LA VITESSE ET LE LARD?",
	"\200A VA, \200A VA, ABANDONNE. EN TOUT CAS, JE NE SAIS PAS POURQUOI JE L'AI DIT.",
	// 235
	"QU'EST-CE QUE TU FAIS ICI EN TRAIN DE PHILOSOPHER AU LIEU DE MANGER DU MONDE?",
	"QU'EST-CE QUE C'EST QUE \200A?",
	"\220COUTE, PEX-TU R\220P\220TER CETTE PHRASE DES \"INCLINATIONS PR\220-\220VOLUTIVES\"?",
	"BIEN S\352R, MON VIEUX. CETTE HISTOITE QUE TU M'AS LACH\220E AVANT. CE QUE JE N'AI PAS BIEN COMPRIS...",
	"NON, LE MIEUX SERA DE NE RIEN DIRE. CAR SI JE LE TOUCHE LA VEINE.....",
	// 240
	"OUI, QUE SE PASSE-T-IL?",
	"OUI, QU'EST-CE QU'IL Y A? ",
	"EH BIEN, MAINTENANT QU'IL ABORDE LE SUJET JE LUI DIRAI QUE...",
	"",
	"EN FAIT, QU'ARRIVERAIT-IL SI UN VAMPIRE SE POURVOYAIT DE LA FORMULE PAR HASARD... ",
	// 245
	"EH BIEN, RIEN. \220COUTE, CECI NE TE SEMBLE PAS UN RAVAUDAGE QU'ON A MIS EN SC\324NE POUR EN FINIR T\342T AVEC LE JEU? BON, PEUT-\322TRE PAS",
	"C'EST VIDE!",
	"POURQUOI TU M'AS VOL\220 MON AMOUR. B.J. SI ELLE N'EST PAS L\267,  LA VIE N'AS PAS DE SENS  POUR MOI",
	"SON CERVEAU?!",
	"CE N'EST POUR RIEN, MAIS JE CROIS QUE TON PETIT MONSTRE M'A F\266CH\220",
	// 250
	"MA VIERGE, QUE JE RESTE TEL QUE JE SUIS!",
	"TU N'AURAS PAS LE DERNIER MOT. C'EST S\352R QUE MAINTENANT APPARA\327T SUPER-LOPEZ ET ME LIB\324RE!",
	"QUELLE MERDE DE JEU DONT MEURT LE PROTAGONISTE!",
	"UN INSTANT, QU'Y A-T-IL DE MON DERNIER D\220SIR?",
	"HA! HA!  MAINTENANT JE SUIS IMMUNIS\220 CONTRE TOI, D\220MON MAUDIT. CETTE CIGARETTE EST UNE POTION ANTI-VAMPIRES QUI M'A DONN\220 VON BRAUN ",
	// 255
	"OUI, C'EST S\352R, MAIS TU N'OBTIENDRAS JAMAIS DE MOI LA FORMULE",
	"JE PEUX SUPPORTER LA TORTURE, ET ENCORE LA CR\220ER ",
	"NON, S'IL VOUS PLA\327T, JE PARLERAI, MAIS NE ME FAITES PAS \200A!",
	"EH BIEN. JE T'AI D\220J\267 DIT CE QUE TU VOULAIS SAVOIR. MAINTENANT D\220LIVRE-NOUS, B.J. ET MOI, ET FICHEZ-NOUS LA PAIX",
	"B.J.! QU'EST-CE QUE TU FAIS L\267? DRASCULA, O\353 EST-IL?",
	// 260
	"QU'IL EST M\220CHANT! C'EST SEULEMENT PAR-CE QU'IL APPARTIENT \267 LA NOBLESSE QU'IL CROIT POUVOIR EXERCER LE DROIT DE GAMBADE AVEC N'IMPORTE QUI",
	"\267 BAS L'ARISTOCRATIE ARBITRAIRE!",
	"DEBOUT LES PAUVRES DU MOOONDE....!",
	"ET D'APR\324S CE QUE JE VOIS ON T'A ENCHA\327N\220 AVEC CADENAS ET TOUT",
	"BON, \200A VA. N'AURAS-TU PAS  UNE \220PINGLE?",
	// 265
	"BON, BON, NE T'EN FAIS PAS COMME \200A. JE PENSERAI \267 QUELQUE CHOSE.",
	"H\220! TAVERNIER!",
	"COMMENT VA LE MATCH?",
	"QUI EST-CE?",
	"NE VOIS-TU PAS QUE DRASCULA EST ICI?",
	// 270
	"ALORS, ON VA FINIR AVEC LUI, NON?",
	"SERS -MOI UN COUP...",
	"RIEN. J'AI OUBLI\220 CE QUE J'ALLAIS TE DIRE",
	"OU\207BIEN\207TU\207ME\207SERS\207UN\207COUP\207OU\207JE\207ME\207METS\207\267\207JOUER\207DU\207PIANO",
	"COMBIEN IL RESTE POUR QUE LE MATCH FINISSE?",
	// 275
	"BON SOIR",
	"COMME VAS-TU, IGOR? BOSSU? -HI! HI! HI! QUE C'EST BON! ",
	"QU'EST-CE QU'ON SUPPOSE QUE TU FAIS?",
	"EH BIEN, NON!",
	"ALORS, METS-TOI DES LUNETTES",
	// 280
	"QU'EST QUE C'EST QU'UNE ORGIE SURNATURELLE?",
	"\200A VA, \200A VA, ARR\322TE-TOI. JE ME FAIS D\220J\267 UNE ID\220E",
	"NE POURRAIS-TU PAS ME DIRE O\353 SE TROUVE DRASCULA? ",
	"ALLONS, S'IL TE PLA\327T",
	"POURQUOI PAS?",
	// 285
	"AH! MAIS IL DORME PENDANT LA NUIT?",
	"EH BIEN! QUE LA RENTE SE DONNE BIEN",
	"CE QUE JE DOIS LUI PARLER",
	"OH\220! SQUELEEETTE! ",
	"SAPRISTI! -UN SQUELETTE QUI PARLE!",
	// 290
	"RACONTE-MOI, COMMENT EST-TU VENU JUSQU'ICI?",
	"ET POUR QUELLE RAISON VOUDRAIT DRASCULA CR\220ER UN MONSTRE? ",
	"COMMENT T'APPELLES-TU, AMI SQUELETTE?",
	"\220COUTE, VEUX-TU QUE JE T'APPORTE QUELQUE CHOSE \267 MANGER?",
	"TU DOIS AVOIR L'ESTOMAC VIDE. -HI! HI! HI!",
	// 295
	"VRAIMENT JE N'AI PAS ENVIE DE PARLER MAINTENANT",
	"MON DIEU! (SIFFLEMENT) J'ESP\324RE QUE...(SIFFLEMENT) ET QUE...(SIFFLEMENT) DEUX FOIS!",
	"J'AI L'AIM\220E VRAIMENT. \200A VA, JE SUIS D'ACCORD, IL N'\220TAIT PAS UN G\220NIE, MAIS PERSONNE EST PARFAIT, N'EST-CE PAS? ",
	"DE PLUS, ELLE AVAIT UNE FIGURE \220POUSTOUFLANTE ",
	"JE NE SERAI PLUS LE M\322ME. JE VAIS M'ENFERMER DANS UN MONAST\324RE POUR VOIR FUIR MA VIE LENTEMENT",
	// 300
	"RIEN NE POURRA M'EN SORTIR D\220J\267 DE CETTE MIS\324RE PARCE QUE...",
	"DE QUI? DE QUI?",
	"JE VEUX \322TRE UN PIRATE",
	"JE VEUX \322TRE PROGRAMMEUR",
	"RACONTEZ-MOI QUELQUE CHOSE SUR PELAYO",
	// 305
	"JE CONTINUERAI \267 JOUER ET J'OUBLIERAI QUE VOUS AI VU ",
	"QUI AURA PENS\220 \267 CETTE B\322TISE?",
	"C'EST UN SAC COMME CELUI DE MA GRANDE-M\324RE",
	"MAIS QUE JE SUIS BEAU!",
	"PLUS JE ME REGARDE PLUS JE ME PLAIS",
	// 310
	"ET APR\324S COMMENT JE ME FERME?",
	"IL FAUDRA QUE M'OUVRE D'ABORD, NON?",
	"JE SUIS BIEN O\353 JE SUIS",
	"JE M'AI D\220J\267",
	"SALUT, MOI!",
	// 315
	"JE VAIS ME LES METTRE \267 TEMPS",
	"JE NE VOIS RIEN DE SP\220CIAL",
	"C'EST BIEN O\353 IL EST",
	"ET POURQUOI FAIRE?",
	"JE NE PEUX PAS",
	// 320
	"SALUT, TOI!",
	"C'EST LE PANTH\220ON DE L'ONCLE D\220SIR\220",
	"OH\220! ONCLE D\220SIR\220\220\220\220!",
	"NON, JE NE VEUX PAS ME COUPER ENCORE UNE FOIS",
	"HEM! HEM!...!",
	// 325
	"YAMM, HEMMM, JH!",
	"OUI, COF,COF!",
	"TIENS, IL Y A ICI UN CHEWING-GUM COLL\220",
	"C'EST LE MOVILANI, LE CADEAU QUI M'ONT DONN\220 POUR NO\323L",
	"QUE C'EST HAUT!",
	// 330
	"SORS DANS LE BALCON JULIETTE!",
	"TU EST LA LUMI\324RE QUI \220CLAIRE MON CHEMIN!",
	"H\220, PORTE! QU'EST-CE QU'IL Y A?",
	"OH\220! MACHINE \267 TABAC DE TRANSYLVANIIIE",
	"C'EST UNE MACHINE \267 D\220BIT DE TABAC",
	// 335
	"J'AI UNE AUTRE MONNAIE L\267 -DEDANS",
	"NON, J'AI D\220CID\220 ABANDONNER LE TABAC ET L'ALCOOL",
	"D\324S MAINTENANT JE VAIS ME CONSACRER SEULEMENT AUX FEMMES",
	"C'EST UN VOL! RIEN EST SORTI!",
	"ENFIN! ",
	// 340
	"C'EST \200A, UN BAHUT",
	"SALUT, BAHUT! TU T'APPELLES COMME MON COUSIN, QUI S'APPELLE RAUL",
	"J'AI TROUV\220 LE SAC DE B.J..",
	"MON DIEU! JE N'Y ME VOIS PAS -SUIS UN VAMPIRE!",
	"...AH, NON! CE N'EST QU'UN DESSIN!",
	// 345
	"PETIT MIROIR: \"C'EST QUI LE PLUS BEAU DU ROYAUME?\"",
	"IL NE VEUT PAS M'OUVRIR",
	"TR\324S BIEN. J'AI MIS LES TAMPONS",
	"C'EST UN DIPL\342ME DE CHASSE-VAMPIRES HOMOLOGU\220 PAR L'UNIVERSIT\220 DE CAMBRIDGE",
	"NON, IL ME FAUT ENCORE D'INGR\220DIENTS, PAS LA PEINE DE LUI REVEILLER",
	// 350
	"C'EST QUE JE SUIS FAUCH\220",
	"C'EST UNE LAMPE BRITANNIQUE",
	"TAVERNIER! -AIDEZ-MOI!",
	"UN VAMPIRE EST APPARU ET IL A ENLEV\220 MA FIANC\220E",
	"MAIS, N'ALLEZ VOUS PAS M'AIDER?!",
	// 355
	"MORTE? QUE VOULEZ-VOUS DIRE?",
	"HEM!",
	"UN VAMPIRE A S\220QUESTR\220E LA FILLE DE LA 506!",
	"IL FAUT QUE TU M'AIDES!",
	"TU NE SAIS PAS JOUER AUCUNE PI\324CE DES INHUMAINS?",
	// 360
	"COMMENT TU TE SUPPORTES TOUT LE JOUR EN JOUANT LA M\322ME CHOSE?",
	"ET ALORS, POURQUOI TU M'\220COUTES?",
	"PR\322TE-MOI LES TAMPONS",
	"ALLONS! JE VAIS TE LES REDONNER TOUT DE SUITE",
	"ALLOOONSSS...",
	// 365
	"AU REVOIR. JE DOIS TUER UN VAMPIRE",
	"",
	"EN QUOI TU PARLES! EN TRANSYLVAN?",
	"C'EST QUI L'ONCLE D\220SIR\220?",
	"MAIS QU'EST-CE QU'IL Y A  AVEC CE  DRASCULA-L\267?",
	// 370
	"QUI EST-CE CE VON BRAUN-L\267?",
	"ET POURQUOI IL NE LE FAIT PAS?",
	"ET O\353 PEUX-JE TROUVER VON BRAUN?",
	"EH BIEN, MERCI ET AU REVOIR. QUE TU LA DORMES BIEN",
	"IL VAUDRA MIEUX SONNER D'ABORD",
	// 375
	"LE PROFESSEUR VON BRAUN, C'EST VOUS?",
	"ET NE POUVEZ-VOUS PAS M'INDIQUER O\353 JE PEUX...?",
	"JE NE CROIS PAS QU'IL SOIT LE NAIN GANYM\324DE",
	"PROFESSEUR!",
	"AIDEZ-MOI! -LA VIE DE MA BIEN AIM\220E DEPENDE DE VOUS!",
	// 380
	"\200A VA, JE N'AI PAS BESOIN DE VOTRE AIDE",
	"D'ACCORD. JE M'EN VAIS",
	"N'AIES PAS PEUR. NOUS ALLONS VAINCRE DRASCULA ENSEMBLE",
	"ALORS, POURQUOI NE M'AIDEZ VOUS PAS?",
	"JE LES AI",
	// 385
	"OUI, JE LES AI!",
	"D'ACCORD",
	"...ER ...OUI",
	"JE VIENS POUR RENTRER DANS CETTE CABINE",
	"SUIS PR\322T \267 CONFRONTER VOTRE \220PREUVE",
	// 390
	"\200A VA, VIEUX RIDICULE. JE SUIS VENU CHERCHER MON ARGENT",
	"NON, RIEN. JE M'EN ALLAIS D\220J\267",
	"PARDONNE-MOI",
	"CE LIVRE T'INT\220RESSE? AVEZ-VOUS DES PARTITIONS DE TCHA\330KOVSKY?",
	"COMMENT PEUX-JE TUER UN VAMPIRE?",
	// 395
	"ON NE T'A JAMAIS DIT QUE C'EST MAUVAIS DORMIR DANS UNE MAUVAISE POSTURE?",
	"EH BIEN, C'EST \200A QUE MA M\324RE ME DIT TOUJOURS",
	"POURQUOI DRASCULA N'A PU JAMAIS TE TUER?",
	"ET QU'EST-CE QUE S'EST PASS\220?",
	"C'EST SUPER! -AVEZ-VOUS UNE POTION D'IMMUNIT...!",
	// 400
	"ALORS?",
	"TR\324S BIEN",
	"POUVEZ-VOUS ME R\220P\220TER  CE DONT J'AI BESOIN POUR CETTE POTION?",
	"EH BIEN! JE PARS RAPIDE LE CHERCHER",
	"\220COUTEZ, QU'EST-CE QUE C'EST PASSE\220 AVEC LE PIANISTE?",
	// 405
	"J'AI\207D\220J\267\207TOUS\207LES\207INGR\220DIENTS\207DE\207CETTE\207POTION",
	"UNE QUESTION: QU'EST.CE QUE C'EST CELA D' ALUCSARD ETEREUM?",
	"PARLEZ, PARLEZ... ",
	"ET C'EST O\353 CETTE GROTTE?",
	"QU'EST-CE QU'IL Y A? N'AVIEZ VOUS PAS UN TRIBUNAL?",
	// 410
	"...MAIS ...ET SI JE TROUVE ENCORE DES VAMPIRES?",
	"C'EST UN VAMPIRE QUI M'EMP\322CHE L'ENTR\220E",
	"IL RESSEMBLE \267 YODA, MAIS C'EST UN PEU PLUS GRAND",
	"H\220, YODA! SI TU ME LAISSES PASSER JE TE DONNE UNE MONNAIE",
	"BON, \200A VA. ON NE PEUT RIEN TE DIRE",
	// 415
	"H\220, VAMPIRE! BELLE NUIT, N'EST-CE PAS?",
	"ON T'A D\220J\267 DIT QUE TU RESSEMBLES \267 YODA?",
	"ES-TU UN VAMPIRE OU UNE PEINTURE \267 L'HUILE?",
	"IL VAUX MIEUX NE RIEN TE DIRE, POUR SI TU TE F\266CHES",
	"C'EST FERM\220E \267 CL\220",
	// 420
	"LA PIE POURRAIT M'ARRACHER UN OEIL SI J'EN ESSAIE!",
	"C'EST FERM\220E! -MON DIEU, QUELLE PEUR!",
	"LES GONDS SONT OXYD\220S",
	"L\267-DEDANS IL Y A SEULEMENT UN POT AVEC DE LA FARINE",
	"CECI A ENLEV\220 L'OXYDE",
	// 425
	"J'AI TROUV\220 UN PIEU DE PIN",
	"JE PRENDRAI CELUI-CI QUI EST PLUS GROS",
	"BON, JE CROIS  QUE JE PEUX ME D\220BARRASSER MAINTENANT DE CE STUPIDE D\220GUISSEMENT",
	"LE PASSAGE AUX DONJONS EST FERM\220 \267 CAUSE DES TRAVAUX. VOUS \322TES PRI\220S D'UTILISER L'ENTR\220E PRINCIPALE. EXCUSEZ LES ENNUIES",
	"...IL EST P\266LE. AVEC DE GROSSES DENTS. IL A UN TOUPET ET UTILISE UNE CAPE... -C'EST S\352REMENT DRASCULA!",
	// 430
	"C'EST B.J.! B.J. TU EST BIEN?",
	"OUI, JE SAIS QU'ELLE EST B\322TE, MAIS JE SUIS SEUL",
	"N'AURAS-TU PAS UNE CL\220 PAR L\267, N'EST-CE PAS?",
	"N'AURAS-TU PAS UN ROSSIGNOL, PAR HASARD?",
	"DONNE-MOI UNE \220PINGLE. JE VAIS FAIRE COMME MCGYVER",
	// 435
	"NE BOUGES PAS, JE REVIENS TOUT DE SUITE",
	"ZUT! -S'EST CASS\220E!",
	"OL\220\220\220! ET EN PLUS JE ME SUIS RAS\220, COLL\324GUE!",
	"OUI, MON AMOUR?",
	"IL N'ARRIVE PAS",
	// 440
	"LE PIANISTE N'EST PAS L\267",
	"UN COKTAIL TRANSYLVAN",
	"JE N'AI PAS UNE CHAMBRE",
	"SELON PARA\327T, IL EST REST\220 COINC\220 DANS LA BAIGNOIRE ET D\220CIDA ALORS D'OUVRIR UN BAR ",
	"IL EST SO\352L COME UNE CUVE DE CUBA",
	// 445
	"CE CHEVEU... LE CAS CE QU'IL ME RAPPELLE QUELQU'UN",
	"C'EST UN SQUELETTE OSSEUX",
	"REGARDE! MIGUEL BOS\220!",
	"IL DORME. CE SERAIT DOMMAGE LE R\220VEILLER",
	"IL EST PLUS MOCHE QU'\220MILE DE PAZ",
	// 450
	"UN CERCUEIL EN BOIS DE PIN",
	"IL VA ME COUPER EN PETITES TRANCHES, COMME UN SAUCISSON",
	"JE N'AIME PAS LES PENDULES. JE PR\220F\324RE LES ARTICHAUTS",
	"MES MAINS SONT EMMENOTT\220ES. JE N'ARRIVERAI PAS",
	"IL SAUTE AUX YEUX  QUE C'EST UNE PORTE SECR\324TE",
	// 455
	"ILS M'IGNORENT",
	"C'EST BIEN!",
	"DANS LE SCRIPT IL BOUGEAIT, MAIS LE JEU A SURPASS\220 LE BUDGET ET ON N'A PAS PU ME PAYER UN GYMNASE POUR ME METTRE EN FORME. DONC, POINT DU TOUT",
	"ELLE PARA\327T UN PEU D\220TACH\220E DU MUR",
	"JE NE CROIS PAS POUVOIR M'EN SERVIR. ELLE TROP HUMIDE POUR L'ALLUMER",
	// 460
	"\267 L'AILE OUEST? -M\322ME PAS EN FOU ACHEV\220! -VA SAVOIR QU'AURAIT-IL L\267-BASI!",
	"IL Y A DE JOLIS MOTIFS TRANSYLVANS",
	"",
	"QUEL DOMMAGE NE PAS AVOIR TROUV\220 L\267-DEDANS UN PETIT AGNEAU EN TRAIN DE SE R\342TIR!",
	"LA DERNI\324RE FOIS QUE J'AI OUVERT UN FOURNEAU  LA MAISON A VOL\220 EN \220CLATS",
	// 465
	"C'EST L'ENSEIGNE DE L'\220QUIPE DE FOOT-BALL DE LA TRANSYLVANIE",
	"ET POURQUOI FAIRE? POUR ME LA METTRE \267 LA T\322TE?",
	"JE NE CROIS PAS QUE CES TIROIRS SOIENT DE CEUX QUI S'OUVRENT",
	"JE NE VEUX PAS SAVOIR LA NOURRITURE QU'IL Y AURA L\267-DEDANS!",
	"J'AI L'IMPRESSION QUE C'EST DE L'IMPRESSIONNISME",
	// 470
	"LA NUIT S'EMPARE DE NOUS TOUS... QUELLE PEUR, N'EST-CE PAS?",
	"ELLE EST BOUCH\220E",
	"C'EST LE ROI. NE L'AVAIT-TU PAS IMAGIN\220?",
	"NON, J'EN AI D\220J\267 UN CHEZ MOI ET JE LUI DONNE \267 MANGER EN PLUS",
	"UN PLACARD AVEC DES LIVRES ET D'AUTRES CHOSES",
	// 475
	"ET QUI J'APPELLE \267 CES HEURES-L\267?",
	"\"COMMENT FAIRE LA D\220CLARATION D'IMP\342TS\" COMME C'EST INT\220RESSANT!",
	"J'AI D\324J\267 UN CHEZ MOI. JE CROIS QUE C'EST UN BEST-SELLER MONDIAL",
	"UNE CL\220 COMPL\324TEMENT NORMALE",
	"IL ME SEMBLE QUE CELLE-CI N'EST PAS D'ICI",
	// 480
	"H\220! CE SONT DES FRITES SOUS FORME DE DENT CANINE! \200A ME PLA\327T",
	"JE NE CROIS PAS QU'IL SOIT LE MEILLEUR MOMENT POUR MANGER DES GOURMANDISES, AVEC MA FIANC\220E AUX MAINS DE L'\322TRE LE PLUS MAUVAIS QU'UNE M\324RE A PU ACCOUCH\220",
	"COMME JE SUIS BIEN EN TUANT DES VAMPIRES AVEC CELA!",
	"VOYONS SI APPARA\327T T\342T UN AUTRE",
	"NON, IL FAUT QU'IL SOIT AVEC UN SALE ET PUANT VAMPIRE, COMME CELUI QUI J'AI TU\220 AVANT",
	// 485
	"C'EST L'AUTHENTIQUE PERRUQUE QU'ELVIS AVAIT UTILIS\220E QUAND IL EST DEVENU CHAUVE",
	"C'EST DE LA FARINE, MAIS JE NE PEUX PAS DIRE DES MARQUES",
	"PEUT-\322TRE DANS UN AUTRE MOMENT, D'ACCORD?",
	"C'EST UNE HACHE MAGNIFIQUE, DOMMAGE DE NE PAS POUVOIR SE PAYER AUCUNE T\322TE DE VAMPIRE PAR L\267",
	"NON. JE SUIS UNE BONNE PERSONNE AU FOND",
	// 490
	"C'EST LE D\220ODORANT DE LA THACHER-HI!HI!HI!",
	"C'EST UNE CAPE ASSEZ MIGNONE",
	"",
	"TOUT COMME LES BRANCHES DE TOUS LES ARBRES DU MONDE, C'EST-\267-DIRE SANS RIEN DE PARTICULIER",
	"OH! C'EST INCROYABLE! -UNE CORDE DANS UNE AVENTURE DESSIN\220E!",
	// 495
	"JE ME DEMANDE \267 QUOI SERVIRA-T-ELLE...?",
	"UNE CORDE ATTACH\220E \267 UNE BRANCE OU UNE BRANCHE ACROCH\220E \267 UNE CORDE, \200A D\220PEND DU POINT DE VUE",
	"IL PARA\327T QUE CETTE PIE \267 DE TR\324S MAUVAISES INTENTIONS",
	"TAIS-TOI! JE NE LA DIS RIEN, POUR SI ELLE SE F\266CHE",
	"ELLE SEMBLE MORTE, MAIS C'EST UNE MENSONGE",
	// 500
	"IL N'Y A AUCUN ANIMAL ABiM\220 DANS LA PRODUCTION DE CE JEU",
},
{
	// 0
	"",
	"\324 LA SECONDA PORTA PI\353 GRANDE CHE IO ABBIA MAI VISTO",
	"BEH, FORSE NO",
	"\324 SIGILLATA CON TAVOLE. LA CHIESA DEV'ESSERE STATA ABBANDONATA PARECCHI ANNI FA.",
	"NON L'HO APERTA",
	// 5
	"CHE FACCIO? LA TOLGO?",
	"CIAO PORTA. STO PER TRASFORMARTI IN UNO STIPITE.",
	"\324 TROPPO PER ME.",
	"UNA FINESTRA SIGILLATA CON TAVOLE.",
	"NON POSSO.",
	// 10
	"GI\267 FATTO.",
	"E PERCH\220?",
	"CIAO FINESTRA. HAI QUALCOSA DA FARE STANOTTE?",
	"NON SENZA IL PERMESSO DEL MINISTERO DELLE OPERE PUBBLICHE",
	"SE SOLO QUESTA FINESTRA NON FOSSE SIGILLATA...",
	// 15
	"YOO-HOO! FINESTRA!",
	"SALVE.",
	"COME QUELLA DELLA MICROCHOF.",
	"NON RIESCO AD ARRIVARCI.",
	"STA BENE DOV'\324.",
	// 20
	"",
	"\324 UNA LAPIDE A FORMA DI CROCE",
	"NO GRAZIE.",
	"CIAO, MORTO. NON TI SCOMODARE AD ALZARTI!",
	"S\326, CERTO. COME IN POLTERGEIST.",
	// 25
	"",
	"",
	"TORNO TRA QUINDICI MINUTI.",
	"VIETATO AFFIGGERE MANIFESTI.",
	"\324 LA TOMBA DELLO ZIO EVARISTO.",
	// 30
	"\324 CHIUSA A CHIAVE",
	"NE HO GI\267 UNO.",
	"YOO HOO, ZIO EVARISTO!",
	"NON RISPONDE.",
	"NO, \324 FISSATO PER BENE.",
	// 35
	"\324 UNA PORTA.",
	"UN CASSETTO DEL TAVOLO.",
	"UN ARMADIO SOSPETTO.",
	"CIAO ARMADIO. COME VA?",
	"",
	// 40
	"",
	"\324 UN CANDELABRO MOLTO ANTICO.",
	"DEV'ESSERE QUI DA QUANDO MAZINGA Z ERA UNA VITE.",
	"NO, \324 UNA RELIQUIA.",
	"\324 UNA GRAZIOSA PALA D'ALTARE.",
	// 45
	"",
	"HI, HI, HI.",
	"",
	"NO.",
	"",
	// 50
	"HA, HA, HA. FANTASTICO!",
	"",
	"",
	"",
	"NON VEDO NIENTE DI SPECIALE.",
	// 55
	"\324 FERNAN, LA PIANTA.",
	"\324 UNO DEI PALETTI DELLA STACCIONATA.",
	"HEY! C'\324 UN PACCHETTO DI FIAMMIFERI QUI SOTTO.",
	"MA GUARDA! UN PACCHETTO DI FAZZOLETTI. CE N'\324 ANCORA UNO NON USATO!",
	"NON C'\324 ALTRO NEL CESTINO.",
	// 60
	"\324 UN CIECO CHE NON VEDE.",
	"",
	"",
	"",
	"",
	// 65
	"\324 UNA BELLA SOMMA DI DENARO.",
	"",
	"",
	"",
	"",
	// 70
	"",
	"",
	"",
	"",
	"",
	// 75
	"",
	"",
	"",
	"",
	"",
	// 80
	"",
	"",
	"",
	"",
	"",
	// 85
	"",
	"",
	"",
	"",
	"",
	// 90
	"",
	"",
	"",
	"",
	"",
	// 95
	"",
	"",
	"",
	"",
	"",
	// 100
	"NON HA NULLA DI SPECIALE.",
	"NON HA NIENTE DI INSOLITO.",
	"COME TE LA PASSI?",
	"CIAO",
	"NIENTE DI NUOVO?",
	// 105
	"COME VA LA FAMIGLIA?",
	"DICI SUL SERIO?",
	"MA COME FACCIO A PRENDERLO?",
	"LA MIA RELIGIONE ME LO PROIBISCE.",
	"MEGLIO DI NO.",
	// 110
	"SICURO!",
	"NEANCHE A PARLARNE.",
	"IMPOSSIBILE",
	"QUESTO NON SI APRE",
	"NON CE LA FACCIO DA SOLO",
	// 115
	"POTREI FARLO, MA MI SENTO UN PO' PIGRO.",
	"NON NE VEDO IL MOTIVO.",
	"\324 UN CERVELLO PIUTTOSTO CARINO.",
	"E ALLORA, CERVELLO, CHE PENSI DI FARE STANOTTE?",
	"NO, DEVE ESSERE CONSERVATO IN UN POSTO LONTANO DALL'AZIONE DEGENERATIVA DELL'ATMOSFERA",
	// 120
	"\324 RIGIDO, COME IL MIO CAPO",
	"UN PICCHETTO MOLTO AFFILATO.",
	"FEDELE PICCHETTO APPUNTITOOO, NOBILE ROVERE TRANSILVANOOO",
	"ACCIDENTI, MI DEVO TAGLIARE LE UNGHIE!",
	"L\326 DENTRO C'\324 B.J., DOVREI VEDERE COME STA LA RAGAZZA!",
	// 125
	"\324 CHIUSA CON LUCCHETTO E CATENACCIO",
	"\"LUCCHETTO E CATENACCIO S.P.A.\"",
	"\324 IL TIPICO SCHELETRO CHE C'\324 IN TUTTE LE CARCERI DI TUTTI I VIDEOGIOCHI",
	"SI USA PER FORNIRE ELETTRICIT\267 AGLI APPARECCHI COLLEGATI",
	"\324 COMPLETAMENTE ARTIGIANALE, VISTO CHE I GIAPPONESI LI FANNO TASCABILI",
	// 130
	"SOLO UNA VOLTA NELLA MIA VITA HO VISTO UNA COSA COS\326 BRUTTA",
	"LASCIA STARE. NON GLI DICO NULLA ALTRIMENTI SI ARRABBIA",
	"SEMBRA ABBASTANZA RAZIONALE",
	"\324 UNA FOTO DI PLATONE MENTRE SCRIVE IL SUO DIALOGO PERDUTO",
	"NON SONO UNO DI QUELLI CHE PARLANO CON I POSTER",
	// 135
	"UNA SCRIVANIA MOLTO CARINA",
	"\324 UN DIPLOMA DI CACCIA-CACCIA-VAMPIRI APPROVATO DALL'UNIVERSIT\267 DI OXFORD",
	"\324 UNA NOTTE BUIA CON LUNA PIENA",
	"SEMBRA CHE QUESTE VITI NON SIANO AVVITATE DEL TUTTO",
	"NON GUARDARE, MA CREDO CHE UNA TELECAMERA NASCOSTA MI STIA REGISTRANDO",
	// 140
	"UN RILEVATORE DI PALETTI MOLTO MODERNO",
	"NO, IL LABORATORIO SI TROVA AL SECONDO PIANO",
	"UN BEL COMODINO",
	"\324 UN MUCCHIO DI DENARO CHE NON PU\343 MANCARE IN NESSUNA AVVENTURA CHE SI RISPETTI",
	"SE FOSSI RICCO, DUBIDUBIDUBIDUBIDUBIDUBIDUBIDU",
	// 145
	"SONO DELLE STRANE FOGLIE. DEVONO AVERLE PORTATE DAL SUDAMERICA O GI\353 DI L\326",
	"NON CREDO CHE MI RISPONDEREBBERO",
	"\324 UN MERAVIGLIOSO CROCIFISSO DI LEGNO. L'ICONA NON RIFLETTE TUTTA LA SUA BELLEZZA",
	"IO PREGO SOLAMENTE PRIMA DI CORICARMI",
	"EH, PARE CHE QUESTA SBARRA SIA UN PO' ALLENTATA!",
	// 150
	"E POI TI LAMENTI PERCH\220 NON TI DO SUGGERIMENTI",
	"\324 UNA SBARRA ABBASTANZA CONVENZIONALE",
	"SONO CARINI, SEBBENE SIANO RICOPERTI DA UN PO' DI SCHIFEZZE",
	"NO, NON MI SENTIRANNO. HI,HI,HI, BUONA QUESTA!",
	"\"LA BELLA ADDORMENTATA NEL BOSCO\" DI CHAIKOSKY, O CHOIFRUSKY, O COME SI CHIAMA",
	// 155
	"MOLTO APPETITOSA",
	"NO, NON SONO UNO DI QUELLI CHE SI METTONO IN BOCCA GOMME USATE",
	"UNA FALCE MOLTO CARINA. MI CHIEDO DOVE SIA IL MARTELLO",
	"I FABBRICANTI DI TABACCO AVVERTONO CHE LE AUTORIT\267 SANITARIE NUOCCIONO GRAVEMENTE ALLA SALUTE",
	"UNA CANDELA ASSOLUTAMENTE NORMALE, CON CERA E TUTTO",
	// 160
	"ACCIDENTI COME SONO LUCENTI QUESTE DUE MONETE!",
	"ACCIDENTI COM'\324 LUCENTE QUESTA MONETA!",
	"CON QUESTO SAR\343 IMMUNE AL MORSO DEI VAMPIRI",
	"NO, ANCORA NON \324 IL MOMENTO",
	"C'E UN BIGLIETTO DA MILLE E UN PAIO DI MONETE",
	// 165
	"DICE \"SI PREGA DI NON TIRARE CIBO AL PIANISTA\"",
	"OMELETTE, 200. PESCE FRITTO, 150, PATATINE CON MAIONESE, 225",
	"I MIGLIORI HAMBURGER DI QUESTA RIVA DEL DANUBIO, SOLTANTO PER 325!",
	"\324 UN BEL TESCHIO, CON UNO SGUARDO MOLTO PENETRANTE. HI, HI, HI, BUONA QUESTA!",
	"CIAO TESCHIO, MI RICORDI LO ZIO DI AMLETO",
	// 170
	"HO L'ABITUDINE DI NON TOCCARE COSE CHE SONO STATE VIVE",
	"\324 UN CESTINO",
	"\324 UNA SCOMMESSA PER LA PARTITA DI STANOTTE",
	"MI DOMANDO CHE CI SAR\267 DIETRO",
	"EHI, QUESTA TENDA NON SI MUOVE!",
	// 175
	"CAVOLI, CHE TETRO QUESTO CASTELLO, EH?",
	"NON POSSO, \324 TROPPO LONTANO PER SENTIRMI",
	"\324 UN TIPICO BOSCO TRANSILVANO, CON ALBERI",
	"CERTO CHE NE SPARI DI SCIOCCHEZZE, CON IL BUIO CHE C'\324!",
	"NEGOZIO DI DOLCI GARCIA. TORTE E GOMME DA MASTICARE",
	// 180
	"UNA PORTA MOLTO BELLA",
	"\324 CHIUSA",
	"\324 UN BARILE COMPLETAMENTE SIGILLATO",
	"",
	"CHE BELLE BESTIOLINE!",
	// 185
	"PSSST, PSSST, GATTINO...",
	"NON C'\324 RISPOSTA",
	"LA LUNA \324 UN SATELLITE CHE GIRA INTORNO ALLA TERRA CON UN PERIODO DI RIVOLUZIONE DI 28 GIORNI",
	"CIAO, LUNA LUNETTA",
	"\324 COMPLETAMENTE SIGILLATA DA DELLE TAVOLE",
	// 190
	"IMPOSSIBILE. QUESTA NON LA APRE NEANCHE BRACCIO DI FERRO",
	"EHI, SEMBRA CHE L'OMBRA DEL CIPRESSO SIA ALLUNGATA!",
	"EHI, BARISTA!",
	"VORREI UNA CAMERA",
	"SA DOVE POSSO TROVARE UN CERTO CONTE DRASCULA?",
	// 195
	"S\326, COSA C'\324?",
	"AH S\326?",
	"DA... DAVVERO?",
	"BELLA DOMANDA. LE RACCONTER\343 LA MIA STORIA. ALLORA...",
	"SONO SOLO CINQUE MINUTI",
	// 200
	"MI CHIAMO JOHN HACKER, E RAPPRESENTO UNA COMPAGNIA IMMOBILIARE BRITANNICA",
	"SEMBRA CHE IL CONTE DRASCULA VOGLIA COMPRARE DEI TERRENI A GIBILTERRA E MI HANNO MANDATO QUI PER NEGOZIARE LA VENDITA",
	"MA CREDO CHE DOMATTINA PRESTO TORNER\343 DA MIA MADRE",
	"BELLA NOTTATA, VERO?",
	"NO, NIENTE",
	// 205
	"EHI, PIANISTA",
	"BELLA NOTTATA",
	"E NON FA NEMMENO FREDDO",
	"VA BENE, TI LASCIO CONTINUARE A SUONARE",
	"BENE ALLORA",
	// 210
	"CIAO CAPO, COME VA?",
	"E LA FAMIGLIA?",
	"CARINO COME POSTO, EH?",
	"MEGLIO CHE NON DICA NULLA",
	"NON C'\324 POSTO PI\353 BELLO DELLA PROPRIA CASA... NON C'\324... EH? MA TU NON SEI LA ZIA EMMA. IN EFFETTI IO NON HO NESSUNA ZIA EMMA!",
	// 215
	"S\326, ANCHE IL MIO. MI PU\343 CHIAMARE COME PI\353 LE PIACE, MA SE MI CHIAMA JOHNNY, CORRER\343 DA LEI COME UN CAGNOLINO",
	"S\326, SONO PROPRIO SPIRITOSO, VERO? COMUNQUE, DOVE MI TROVO?",
	"S\326.",
	"MANNAGGIA...",
	"OH, S\326. IMMAGINO DI S\326",
	// 220
	"BEH, GRAZIE PER IL TUO AIUTO. NON TI DISTURBER\343 PI\353 . POTRESTI DIRMI DOV'\324 LA PORTA, PER FAVORE...",
	"LA BOTTA DEVE AVERMI DANNEGGIATO IL CERVELLO... NON RIESCO A VEDERE UN TUBO...",
	"BAH, NON IMPORTA. NE PORTO SEMPRE UN PAIO DI RISERVA",
	"WOW, CHE BELLA RAGAZZA! NON ME NE ERO ACCORTO PRIMA! CERTO, SENZA GLI OCCHIALI!",
	"SENTI...",
	// 225
	"E QUESTOOO?!",
	"NON TI PREOCCUPARE B.J., AMORE MIO! TI SALVER\343 DALLE SUE GRINFIE",
	"MI HAI FATTO DAVVERO ARRABBIARE...",
	"AHHH, UN LUPO MANNARO! MUORI, MALEDETTO!",
	"S\326, BEH...",
	// 230
	"S\326, BEH... CREDO CHE PROSEGUIR\343 PER LA MIA STRADA. CON PERMESSO...",
	"COSA?",
	"PER LA VERIT\267, PENSANDOCI BENE... CREDO DI NO",
	"DIMMI, O ERUDITO FILOSOFO, ESISTE UNA QUALCHE RELAZIONE CAUSA-EFFETTO TRA LA VELOCIT\267 E LA PANCETTA?",
	"VA BENE, VA BENE, LASCIA PERDERE. NON SO NEANCHE PERCH\220 L'HO DETTO.",
	// 235
	"PERCH\220 STAI QUI A FILOSOFARE, INVECE DI ANDARE A MANGIARE LE PERSONE?",
	"COME MAI?",
	"SENTI, PUOI RIPETERE QUELLA COSA SULLE RELAZIONI PRE-EVOLUTIVE?",
	"S\326, AMICO. QUELLA MENATA CHE MI HAI FATTO SENTIRE PRIMA. \324 CHE NON L'HO CAPITA MOLTO BENE...",
	"NO, MEGLIO NON DIRE NULLA, NON VOGLIO METTERE IL COLTELLO NELLA PIAGA...",
	// 240
	"PRONTO?",
	"S\326, CHE SUCCEDE?",
	"AH, VISTO CHE NE PARLA, LE DIR\343 CHE...",
	"",
	"A PROPOSITO, NON CHE SIA QUESTO IL CASO, CERTO, MA COSA ACCADREBBE SE PER CASO UN VAMPIRO OTTENESSE LA RICETTA?",
	// 245
	"AD OGNI MODO. SENTI, QUESTA NON TI SEMBRA UNA TROVATA MESSA SUL COPIONE PER FINIRE PRESTO IL GIOCO? BEH, FORSE NO",
	"\324 VUOTO!",
	"PERCH\220 MI HAI RUBATO IL MIO UNICO AMORE, B.J.? SENZA DI LEI LA MIA VITA NON HA SENSO",
	"IL SUO CERVELLO?!",
	"NON PER NIENTE, MA CREDO DI AVERNE ABBASTANZA DEL TUO MOSTRICIATTOLO",
	// 250
	"SANTA VERGINE, SALVAMI DA ALTRE SFORTUNE!",
	"NON TE LA CAVERAI. SICURAMENTE APPARIR\267 SUPERMAN E MI SALVER\267!",
	"CHE SCHIFO DI GIOCO \324 QUESTO, UNO IN CUI MUORE IL PROTAGONISTA!",
	"EHI, UN MOMENTO, COSA NE \324 DEL MIO ULTIMO DESIDERIO?",
	"AH, AH! ORA SONO IMMUNIZZATO CONTRO DI TE, MALEDETTO DEMONIO. QUESTA SIGARETTA CONTIENE UNA POZIONE ANTI-VAMPIRO CHE MI HA DATO VON BRAUN",
	// 255
	"S\326, CERTO. MA NON RIUSCIRAI MAI A FARMI DIRE LA RICETTA",
	"POSSO SOPPORTARE QUALUNQUE TORTURA.",
	"NO, TI PREGO! PARLER\343, MA NON FARMI QUESTO!",
	"BENE. TI HO DETTO QUELLO CHE VOLEVI SAPERE. ORA LIBERA B.J. E ME, E LASCIACI IN PACE!",
	"B.J.! COSA CI FAI QUI? DOV'\324 DRASCULA?",
	// 260
	"CHE SPREGEVOLE! SOLTANTO PERCH\220 APPARTIENE ALLA NOBILT\267 PENSA DI AVERE LO \"IUS PRIMAE NOCTIS\" SU QUALUNQUE RAGAZZA LUI VOGLIA",
	"ABBASSO IL DISPOTISMO ARISTOCRATICO!",
	"FORZA I POVERI DEL MONDOOO...!",
	"A QUANTO VEDO TI HA INCATENATO CON LUCCHETTO E TUTTO, EH?",
	"VA BENE. NON HAI UNA FORCINA?",
	// 265
	"VA BENE, VA BENE. NON PRENDERTELA COS\326, MI VERR\267 IN MENTE QUALCOSA.",
	"EHI, BARISTA!",
	"COME VA LA PARTITA?",
	"CHI?",
	"MA NON VEDI CHE DRASCULA \324 QUI?",
	// 270
	"ALLORA LA FINIAMO CON LUI UNA VOLTA PER TUTTE, NO?",
	"SERVIMI UN DRINK",
	"NIENTE. HO DIMENTICATO COSA VOLEVO DIRTI",
	"O MI SERVI UN DRINK O MI METTO A SUONARE IL PIANOFORTE FINO ALLA FINE DELLA PARTITA",
	"QUANTO MANCA ALLA FINE DELLA PARTITA?",
	// 275
	"BUONA SERA",
	"E COME TI SENTI, IGOR? UN PO' INGOBBITO? AH, AH, AH, CHE SPASSO!",
	"COSA STAI FACENDO?",
	"BEH, NO",
	"ALLORA METTITI GLI OCCHIALI",
	// 280
	"COS'\324 QUESTA STORIA DELL'ORGIA SOPRANNATURALE?",
	"OK, OK, NON CONTINUARE, ME NE SONO FATTO UN'IDEA",
	"NON POTRESTI DIRMI DOV'\324 DRASCULA?",
	"DAAAI, PER FAVORE...!",
	"PERCH\220 NO?",
	// 285
	"AH, MA DORME DI NOTTE?",
	"BENE, ALLORA IN BOCCA AL LUPO CON I REDDITI",
	"DEVO PARLARE CON LUI",
	"EHI, SCHELETROOO!",
	"SANTO CIELO! UN MORTO CHE PARLA!",
	// 290
	"RACCONTAMI. COME SEI FINITO QUI?",
	"E PERCH\220 DRASCULA VUOLE CREARE UN MOSTRO?",
	"COME TI CHIAMI, AMICO SCHELETRO?",
	"SENTI, NON VUOI CHE TI PORTI QUALCOSA DA MANGIARE?",
	"DEVI AVERE LO STOMACO VUOTO. AH, AH, AH!",
	// 295
	"ADESSO NON MI VA DI PARLARE",
	"CHE FIGLIA DI ...(BIP). VADA A FARSI F...(BIP) QUELLA STR...(BIP)!",
	"IO LA AMAVO DAVVERO. VA BENE, NON ERA PROPRIO UN'INTELLETTUALE, MA NESSUNO \324 PERFETTO, NO?",
	"E POI, AVEVA UN CORPO MOZZAFIATO",
	"NON SAR\343 MAI PI\353 QUELLO DI PRIMA. MI RINCHIUDER\343 IN UN MONASTERO E LASCER\343 SCORRERE VIA LA MIA VITA A POCO A POCO",
	// 300
	"NIENTE POTR\267 TIRARMI FUORI DA QUESTA MISERIA PERCH\220...",
	"DI CHI? DI CHI?",
	"VOGLIO ESSERE UN PIRATA",
	"VOGLIO ESSERE UN PROGRAMMATORE",
	"RACCONTAMI QUALCOSA SU GARIBALDI",
	// 305
	"CONTINUER\343 A GIOCARE E DIMENTICHER\343 DI AVERVI VISTO",
	"A CHI SAR\267 VENUTA IN MENTE QUESTA IDIOZIA?",
	"\324 UNA BORSETTA COME QUELLA DI MIA NONNA",
	"PER\343, CHE FIGO CHE SONO!",
	"PI\353 MI VEDO, PI\353 MI PIACCIO",
	// 310
	"E POI COME MI CHIUDO?",
	"PRIMA DOVR\343 APRIRMI, NO?",
	"STO BENE DOVE SONO",
	"MI SONO GI\267 PRESO",
	"CIAO ME!",
	// 315
	"LI INDOSSER\343 QUANDO SAR\267 IL MOMENTO GIUSTO",
	"NON CI VEDO NIENTE DI SPECIALE",
	"STA BENE DOV'\324",
	"E PERCH\220?",
	"NON POSSO",
	// 320
	"CIAO A TE",
	"\324 IL SEPOLCRO DELLO ZIO DESIDERIO",
	"EHI, ZIO DESIDERIOOOO!",
	"NO. NON VOGLIO TAGLIARMI UN'ALTRA VOLTA",
	"EHEM, EHM...!",
	// 325
	"GNAMM, AH!",
	"S\326, COF, COF!",
	"GUARDA, C'\324 UNA GOMMA ATTACCATA QUI",
	"\324 IL TELEFONINO CHE MI HANNO REGALATO A NATALE",
	"COM'\324 ALTO!",
	// 330
	"ESCI SUL BALCONE, GIULIETTA!",
	"TU SEI LA LUCE CHE ILLUMINA LA MIA STRADA!",
	"EHI, PORTA, DOVE PORTI?",
	"EHI, DISTRIBUTORE DI SIGARETTE DI TRANSILVANIA!",
	"\324 UN DISTRIBUTORE DI SIGARETTE",
	// 335
	"C'\324 GI\267 UN'ALTRA MONETA DENTRO",
	"NO. HO DECISO DI SMETTERE DI FUMARE E DI BERE",
	"A PARTIRE DA ADESSO MI DEDICHER\343 SOLAMENTE ALLE DONNE",
	"QUESTA \324 UNA TRUFFA! NON \324 USCITO NULLA!",
	"FINALMENTE!",
	// 340
	"CHE TI HO DETTO? UN BAULE",
	"CIAO BAULE, TI CHIAMI QUASI COME MIO CUGINO... RAULE.",
	"HO TROVATO LA BORSA DI B.J.!",
	"MIO DIO, NON HO UN RIFLESSO, SONO UN VAMPIRO!",
	"...AH, NO. \324 UN DISEGNO!",
	// 345
	"SPECCHIO, SPECCHIO DELLE MIE BRAME: CHI \324 IL PI\353 BELLO DEL REAME?",
	"NON VUOLE APRIRMI",
	"MOLTO BENE. HO MESSO I TAPPI",
	"\324 UN DIPLOMA DI CACCIA-VAMPIRI APPROVATO DALL'UNIVERSIT\267 DI CAMBRIDGE",
	"NO, MANCANO ANCORA ALCUNI INGREDIENTI, NON VALE LA PENA SVEGLIARLO",
	// 350
	"MA NON HO SOLDI",
	"\324 UNA LAMPADA BRITANNICA",
	"BARISTA! AIUTAMI!",
	"\324 COMPARSO UN VAMPIRO ED HA PRESO LA MIA FIDANZATA!!",
	"MA NON MI AIUTERAI?!",
	// 355
	"MORTA? COSA INTENDI DIRE?",
	"EHEM!",
	"UN VAMPIRO HA RAPITO LA RAGAZZA DELLA 506!",
	"MA MI DEVI AIUTARE!",
	"NE SAI SUONARE QUALCUNA DI ELIO E LE STORIE TESE?",
	// 360
	"COME FAI A RESISTERE SUONANDO SEMPRE LO STESSO PEZZO TUTTO IL GIORNO?",
	"E ALLORA COME FAI A SENTIRMI?",
	"PRESTAMI I TAPPI PER LE ORECCHIE",
	"DAI, TE LI RESTITUISCO SUBITO",
	"DAIIII...",
	// 365
	"BEH, CIAO. HO UN VAMPIRO DA UCCIDERE",
	"",
	"MA COME PARLI? IN TRANSILVANO?",
	"CHI \324 LO ZIO DESIDERIO?",
	"MA CHE SUCCEDE CON QUESTO DRASCULA?",
	// 370
	"CHI \324 QUESTO VON BRAUN?",
	"E PERCH\220 NON LO FA?",
	"E DOVE POSSO TROVARE VON BRAUN?",
	"GRAZIE E CIAO, DORMI BENE",
	"SAR\267 MEGLIO SUONARE PRIMA",
	// 375
	"\324 LEI IL PROFESSOR VON BRAUN?",
	"E MI POTREBBE DIRE DOVE POSSO...?",
	"NON CREDO SIA IL NANO GANIMEDE",
	"PROFESSORE!",
	"MI AIUTI, LA PREGO! LA VITA DELLA MIA AMATA DIPENDE DA LEI!",
	// 380
	"E VA BENE, NON HO BISOGNO DEL SUO AIUTO",
	"D'ACCORDO. ME NE VADO",
	"NON ABBIA PAURA. INSIEME SCONFIGGEREMO DRASCULA",
	"ALLORA PERCH\220 NON MI AIUTA?",
	"IO CE LE HO",
	// 385
	"S\326 CHE CE LE HO!",
	"D'ACCORDO",
	"...EHH ...S\326",
	"SONO VENUTO PER ENTRARE DI NUOVO IN QUELLA CABINA",
	"SONO PRONTO PER AFFRONTARE LA PROVA",
	// 390
	"E VA BENE, VECCHIETTO. SONO VENUTO PER I MIEI SOLDI",
	"NO, NIENTE. ME NE STAVO ANDANDO",
	"SCUSA",
	"TI INTERESSA QUESTO LIBRO? HA LE PARTITURE DI TCHAIKOWSKY",
	"COME POSSO UCCIDERE UN VAMPIRO?",
	// 395
	"NON TI HANNO DETTO CHE DORMIRE IN UNA BRUTTA POSIZIONE NON \324 SALUTARE?",
	"\324 QUELLO CHE MI DICEVA SEMPRE MIA MADRE",
	"PERCH\220 DRASCULA NON RIUSC\326 AD UCCIDERTI?",
	"E COSA NE FU?",
	"FANTASTICO! LEI HA LA POZIONE DELL'IMMUNIT\267...!",
	// 400
	"E ALLORA?",
	"MOLTO BENE",
	"MI PU\343 RIPETERE DI COSA HO BISOGNO PER QUELLA POZIONE?",
	"OK, CORRO A TROVARLO",
	"SENTA, COSA \324 SUCCESSO CON IL PIANISTA?",
	// 405
	"HO TUTTI GLI INGREDIENTI DI QUESTA POZIONE",
	"SOLO UNA DOMANDA: COS'\324 QUELLA SCRITTA ALUCSARD ETEREUM?",
	"DICA, DICA... ",
	"E DOV'\324 QUELLA GROTTA?",
	"CHE C'\324? NON AVEVATE UN TRIBUNALE?",
	// 410
	"...MA ...E SE TROVO ALTRI VAMPIRI?",
	"\324 UN VAMPIRO CHE NON MI FA PASSARE",
	"ASSOMIGLIA A YODA, MA UN PO' PI\353 ALTO",
	"EHI, YODA. SE MI FAI PASSARE TI DAR\343 UN PENNY",
	"OK, CALMA, CERTO CHE CON TE NON SI PU\343 PROPRIO PARLARE",
	// 415
	"TI HANNO MAI DETTO CHE ASSOMIGLI A YODA?",
	"CIAO VAMPIRO, BELLA NOTTE, VERO?",
	"SEI UN VAMPIRO O UN DIPINTO A OLIO?",
	"MEGLIO NON DIRE NIENTE, ALTRIMENTI POI TI ARRABBI",
	"\324 CHIUSA A CHIAVE",
	// 420
	"SE CI PROVO LA GAZZA POTREBBE CAVARMI UN OCCHIO!",
	"\324 CHIUSA! DIO MIO, CHE PAURA!",
	"I CARDINI SONO ARRUGGINITI",
	"QUI DENTRO C'\324 SOLTANTO UN CESTO DI FARINA",
	"QUESTO HA TOLTO LA RUGGINE",
	// 425
	"HO TROVATO UN PALETTO DI LEGNO DI PINO",
	"PRENDER\343 QUESTO QUI PI\353 GROSSO",
	"BENE, CREDO DI POTERMI TOGLIERE QUESTO STUPIDO COSTUME",
	"\"CORRIDOIO AI TORRIONI CHIUSO PER LAVORI IN CORSO. USATE LA PORTA PRINCIPALE. SCUSATE IL DISTURBO\"",
	"...\324 PALLIDO, HA I CANINI IN FUORI, PORTA IL TOUPET E INDOSSA IL MANTELLO... \324 SICURAMENTE DRASCULA!",
	// 430
	"\324 B.J.! STAI BENE, B.J.?",
	"S\326, LO SO CHE \324 TONTA, MA SONO COS\326 SOLO",
	"NON \324 CHE HAI UNA CHIAVE, EH?",
	"E SCOMMETTO CHE NON HAI NEANCHE UN GRIMALDELLO...",
	"DAMMI UNA FORCINA. GIOCHER\343 A FARE MCGYVER!",
	// 435
	"NON MUOVERTI, TORNO SUBITO",
	"MANNAGGIA! SI \324 ROTTA!",
	"OLEEEE! MI SONO FATTO PERSINO LA BARBA!",
	"S\326, TESORO?",
	"NON \324 ANCORA ARRIVATO",
	// 440
	"IL PIANISTA NON C'\324",
	"UN DRINK TRANSILVANO",
	"NON HO ANCORA UNA CAMERA",
	"SEMBRA CHE SIA RIMASTO INCASTRATO NELLA VASCA DA BAGNO E ABBIA DECISO DI APRIRE UN BAR",
	"\324 UBRIACO FRADICIO",
	// 445
	"QUESTO CAPELLO... MI RICORDA QUALCUNO",
	"\324 UNO SCHELETRO OSSUTO",
	"GUARDA! C'\324 MIGUEL BOSE!",
	"STA DORMENDO. SAREBBE UN PECCATO SVEGLIARLO",
	"\324 PI\353 BRUTTO DI EMILIO FEDE",
	// 450
	"UNA BARA IN LEGNO DI PINO",
	"MI TAGLIER\267 A FETTINE, COME UN SALSICCIOTTO",
	"NON MI PIACCIONO I PENDULI. PREFERISCO GLI ALCACHOFAS",
	"HO LE MANI LEGATE. NON POSSO FARCELA",
	"OVVIAMENTE \324 UNA PORTA SEGRETA",
	// 455
	"MI IGNORANO",
	"DAIII!",
	"SECONDO IL COPIONE SI SAREBBE DOVUTO MUOVERE, MA IL BUDGET ERA RISICATO, NON MI HANNO PAGATO LA PALESTRA E COS\326 NON HO POTUTO FARMI I MUSCOLI. FINE DELLA STORIA",
	"SEMBRA UN PO' STACCATA DALLA PARETE",
	"NON PENSO CHE MI TORNER\267 UTILE. \324 TROPPO UMIDA PER ACCENDERSI",
	// 460
	"ALL'ALA OVEST? \324 DA PAZZI! CHISS\267 COSA POTRESTI TROVARCI!",
	"HA DELLE OTTIME MOTIVAZIONI TRANSILVANE",
	"",
	"PECCATO CHE NON CI SIA UN BELL'AGNELLO ARROSTO, L\326 DENTRO",
	"L'ULTIMA VOLTA CHE HO APERTO UN FORNO HO FATTO SALTARE IN ARIA LA CASA",
	// 465
	"LO STEMMA DELLA SQUADRA DI CALCIO TRANSILVANA",
	"E PERCH\220? PER METTERMELA SULLA TESTA?",
	"NON CREDO CHE QUESTI CASSETTI SIANO DI QUELLI CHE SI APRONO",
	"NON VOGLIO SAPERE CHE RAZZA DI CIBO CI SIA L\267 DENTRO",
	"MI D\267 L'IMPRESSIONE DI ESSERE IMPRESSIONISTA",
	// 470
	"LA NOTTE CALA SU OGNUNO DI NOI... NON \324 TERRIFICANTE?",
	"\324 INCASTRATA",
	"\324 IL RE. LUI NON TE LO SEI IMMAGINATO, VERO?",
	"NO, NE HO GI\267 UNO A CASA DA SFAMARE",
	"UNO SCAFFALE CON LIBRI ED ALTRE COSE",
	// 475
	"MA CHI POTREBBE MAI CHIAMARE A QUEST'ORA?",
	"\"COME COMPILARE LA DICHIARAZIONE DEI REDDITI\". MOLTO INTERESSANTE!",
	"NE HO GI\267 UNO A CASA. CREDO SIA UN BEST SELLER MONDIALE",
	"UNA CHIAVE ASSOLUTAMENTE NORMALE",
	"NON CREDO CHE SIA DI QUESTA ZONA",
	// 480
	"EHI, SONO PATATINE FRITTE A FORMA DI DENTI CANINI! LE ADORO!",
	"NON CREDO SIA IL MOMENTO GIUSTO PER METTERMI A MANGIARE SCHIFEZZE, CONSIDERATO IL FATTO CHE LA MIA FIDANZATA \324 NELLE MANI DELL'UOMO PI\353 CATTIVO DELLA TERRA",
	"ME LA STO DAVVERO SPASSANDO AD UCCIDERE VAMPIRI CON QUESTO!",
	"VEDIAMO SE NE APPARE UN ALTRO",
	"NO, DEV'ESSERE UN VAMPIRO SUDICIO E MALEODORANTE COME QUELLO CHE HO UCCISO PRIMA",
	// 485
	"\324 L'AUTENTICA PARRUCCA CHE UTILIZZ\343 ELVIS QUANDO DIVENT\343 PELATO",
	"\324 FARINA, MA NON POSSO DIRE LA MARCA",
	"FORSE UN'ALTRA VOLTA, VA BENE?",
	"\324 UN'ASCIA MAGNIFICA, PECCATO CHE NON CI SIA NEMMENO UNA TESTA DI VAMPIRO QUI INTORNO",
	"NO. IN FONDO SONO UNA BRAVISSIMA PERSONA",
	// 490
	"\324 IL DEODORANTE DELLA TATCHER ... AH, AH, AH...!!",
	"\324 UN MANTELLO MOLTO CARINO",
	"",
	"COME I RAMI DI TUTTI GLI ALBERI DEL MONDO. NON CI VEDO NIENTE DI SPECIALE",
	"OH, INCREDIBILE! UNA CORDA IN UN'AVVENTURA GRAFICA!",
	// 495
	"MI DOMANDO A COSA POSSA SERVIRE...",
	"UNA CORDA LEGATA AD UN RAMO O UN RAMO LEGATO AD UNA CORDA, DIPENDE DA COME SI GUARDA",
	"PARE CHE QUESTA GAZZA ABBIA CATTIVE INTENZIONI",
	"SCORDATELO, NON DIR\343 NULLA ALTRIMENTI SI ARRABBIA",
	"MI SEMBRA MORTA, MA NON LO \324 VERAMENTE, VERO?",
	// 500
	"NESSUN ANIMALE HA SUBITO MALTRATTAMENTI DURANTE LA PRODUZIONE DI QUESTO VIDEOGIOCO",
},
};

const char *_textd[NUM_LANGS][NUM_TEXTD] = {
{
	// 0
	"",
	"HEY IGOR, HOW IS EVERYTHING GOING?",
	"IT'S ALWAYS THE SAME STORY EVERYTIME THERE'S A GOOD GAME ON THE SATELLITE! ANYWAY, WE'LL GO WATCH IT IN THE BAR, AS USUAL",
	"IGOR LISTEN CAREFULLY MAN, WE ARE GOING TO START WITH PHASE NUMBER ONE OF MY PLAN TO CONQUER THE WORLD",
	"FIRST WE'LL CAPTURE SOME LIGHTNING THEN WE'LL DEMAGNETIZE IT WITH OUR INDIFIBULATOR. THE ELECTRICITY WILL FLOW THROUGH TO MY MONSTER AND HE WILL LIVE!",
	// 5
	"IF EVERYTHING WORKS OUT ALL RIGHT, THIS WILL BE THE BEGINNING OF A GREAT ARMY THAT WILL CONQUER THE WORLD FOR ME. HA, HA",
	"THE MONSTERS WILL DESTROY ALL THE ARMY'S WEAPONS IN THE WORLD, MEANWHILE, WE'LL BE SAFE IN THE PIECES OF LAND I PLAN TO BUY IN GIBRALTAR",
	"WE'LL SET UP A COUP. GOVERNMENTS ALL OVER THE WORLD WILL BE UNCOVERED AND THEIR COUNTRIES WILL SURRENDER TO ME!",
	"I'LL BECOME THE FIRST BAD GUY IN HISTORY TO MAKE IT! HA, HA!",
	"I'M NOT TALKING TO YOU, IDIOT! I'M JUST GIVING YOU THE PLOT. ALL RIGHT, EVERYTHING READY?",
	// 10
	"THE TIME HAS COME! TURN ON THE ALKALINE BATTERIES' SWITCH",
	"DAMN IT! WHAT WENT WRONG?",
	"ARE YOU SURE YOU CHECKED IT ALL AND THERE WAS NOTHING MISSING? LATELY YOU'VE BEEN MESSING AROUND WITH THAT STUFF ABOUT TAXES AND I DON'T KNOW MAN...",
	"YOU STUPID THING! YOU FORGOT TO CONNECT THE INDIFIBULATOR. THE SCREWS HAVE MAGNETIZED AND HIS BRAIN HAS PROBABLY BURNT",
	"YOU ARE DEAD, YOU ARE DEAD...WAIT TILL I GET YOU!",
	// 15
	"SHUT UP! I'LL GET ANOTHER BRAIN TOMORROW AND THEN WE WILL REPEAT THE EXPERIMENT",
	"THIS TIME I'LL GET A WOMAN'S BRAIN. SHINY AND NOT USED YET...HA, HA, HA, GOOD ONE!",
	"SO WHAT? I'M THE BAD GUY, RIGHT? SO I CAN BE AS FULL OF MACHISMO AS I WANT, ALL RIGHT? AND IF YOU SAY ANOTHER THING I'LL TURN YOUR HUMP BACK TO FRONT!",
	"HA, HA, HA. YOU FELL FOR IT!! NOW YOU ARE GOING TO PAY FOR DARING TO FIGHT ME! IGOR, TAKE HIM TO THE PENDULUM OF DEATH!",
	"AND SO, TELL ME, YOU STUPID HUMAN. HOW COME YOU WANT TO DESTROY ME?",
	// 20
	"THAT'S BEAUTIFUL!. IF IT WASN'T BECAUSE IT MAKES ME LAUGH, I WOULD CRY",
	"I PLAN TO USE YOUR GIRLFRIEND'S BRAIN, TO HELP ME CONQUER THE WORLD",
	"YES, SURE! I'LL TAKE IT FROM HER AND GIVE IT TO MY FRUSKYNSTEIN. THE WORLD WILL BE MINE WITH IT, HA, HA",
	"WHAT!? YOU'RE DEAD, MAN! I'M GOING TO...YOU REALLY GOT ME ANGRY MAN...COME ON, PREPARE TO DIE!",
	"HA, HA, HA. NOT EVEN IN YOUR WILDEST DREAMS!",
	// 25
	"YES, ISN'T IT? HA, HA",
	"ALL RIGHT, ALL RIGHT. BUT DO IT QUICKLY, OK?",
	"PUT THAT CIGARETTE OUT NOW! I CAN'T STAND YOU ANYMORE!",
	"SO, DOES THAT BREW HAVE IMMUNISATING EFFECTS FOR VAMPIRES??",
	"WELL, WE'LL SEE ABOUT THAT",
	// 30
	"OK, LET'S SEE. IGOR, BRING ME THE CD \"SCRATCHING YOUR NAILS ALL OVER THE BLACKBOARD\"",
	"NO WAY. THE GIRL STAYS WITH ME. YOU'RE STAYING THERE UNTIL THE PENDULUM CUTS YOU INTO THIN SLICES. HA, HA, HA",
	"MAN I'M SO BAD... COME ON IGOR, LET'S MAKE THE BREW AND CONQUER THE WORLD",
	"WHATS HAPPENED NOW?",
	"YES, WHAT?...OH, DAMNED, THE GAME!",
	// 35
	"I FORGOT ABOUT THAT. GET THE GIRL AND LET'S GO AND WATCH IT. WE CAN CONQUER THE WORLD LATER",
	"THANKS MAN, I WAS THIRSTY",
	"OH, THE CRUCIFIX!!...THE CRUCIFIX...!",
	"I DIDN'T NOTICE THAT BEAUTIFUL CRUCIFIX!",
	"LEAVE ME ALONE!, I'M WATCHING THE GAME",
	// 40
	"",
	"",
	"",
	"",
	"",
	// 45
	"",
	"",
	"",
	"",
	"",
	// 50
	"",
	"",
	"",
	"",
	"",
	// 55
	"",
	"HI BLIND MAN. HOW IT'S GOING?",
	"HOW DO YOU KNOW I'M A FOREIGNER?",
	"YOU LOOK BLIND. BECAUSE YOU'RE WEARING DARK GLASSES LIKE STEVIE WONDER",
	"LOOK, I'M SORRY, I DIDN'T KNOW YOU COULD SEE",
	// 60
	"BUT YOU JUST TOLD ME YOU WEREN'T BLIND",
	"BUT IF YOU CAN'T SEE",
	"OOOOKAY. SORRY. IN THAT CASE, HI THERE SIGHTLESS PERSON",
	"I'M JOHN HACKER. YOU MUST BE ONE OF THOSE CHARACTERS WHO WILL HELP ME OUT IN EXCHANGE FOR AN OBJECT. AREN'T YOU? EH? AREN'T YOU?",
	"UUUUM, EXCUSE ME FOR ASKING BLIN... SIGHTLESS PERSON! BUT WHAT SORT OF JOB IS THAT, TO GIVE SICKLES IN EXCHANGE FOR MONEY WHILE YOU PLAY THE ACCORDION?",
	// 65
	"AH YES, I SUPPOSE THAT'S TRUE. GOODBYE SIGHTLESS PERSON... BLIND MAN",
	"HERE IS THE LARGE AMOUNT OF MONEY YOU ASKED ME FOR",
	"I HOPE SO",
	"HI THERE FOREIGNER",
	"AND HOW DO YOU KNOW I'M BLIND?",
	// 70
	"AND I'M NOT KIDDING YOU BUT YOURS LOOK LIKE WOODY ALLEN'S",
	"NO, I CAN'T SEE",
	"AND I'M NOT",
	"OH OF COURSE. JUST BECAUSE I CAN'T SEE, YOU ACCUSE ME OF BEING BLIND",
	"HI THERE FOREIGNER! WHAT ARE YOU DOING IN TRANSILVANIA?",
	// 75
	"THAT'S RIGHT, FOREIGNER. IN EXCHANGE FOR A LARGE SUM OF MONEY I'LL GIVE YOU A SICKLE YOU NEVER KNOW WHEN YOU MIGHT NEED IT",
	"SHHHH, I'M A SICKLES TRADER, THAT'S WHY I HAVE TO HIDE",
	"BECAUSE YOU TOLD ME BEFORE, DIDN'T YOU?",
	"THANKS FOREIGNER. HERE'S THE SICKLE IN EXCHANGE. YOU'LL FIND IT REALLY USEFUL LATER ON, HONESTLY.",
	"",
	// 80
	"",
	"",
	"NOTHING, NOTHING AT ALL",
	"BLA, BLA, BLA."
},
{
	// 0
	"",
	"\250COMO VA TODO, IGOR?",
	"\255SIEMPRE QUE HAY UN BUEN PARTIDO EN LA PARABOLICA PASA LO MISMO! EN FIN, IREMOS A VERLO AL BAR COMO DE COSTUMBRE",
	"AHORA IGOR, ATIENDE. VAMOS A REALIZAR LA FASE 1 DE MI PLAN PARA CONQUISTAR EL MUNDO",
	"PRIMERO CAPTAREMOS UNO DE LOS RAYOS DE LA TORMENTA Y LO DESMAGNETIZAREMOS CON NUESTRO INDIFIBULADOR. LA ELECTRICIDAD PASARA A MI MONSTRUO \255Y LE DARA VIDA! ",
	// 5
	"SI TODO SALE BIEN ESTE NO SERA MAS QUE EL PRIMERO DE UN INMENSO EJERCITO QUE CONQUISTARA EL MUNDO PARA MI, JA, JA, JA",
	"LOS MONSTRUOS DESTRUIRAN TODAS LAS ARMAS DE TODOS LOS EJERCITOS DEL MUNDO, MIENTRAS NOSOTROS NOS REFUGIAMOS EN UNOS TERRENOS QUE HE COMPRADO EN GIBRALTAR",
	"ENTONCES, DAREMOS UN GOLPE DE ESTADO, LOS GOBIERNOS DEL MUNDO NO TENDRAN CON QUE PROTEGERSE, Y PONDRAN SUS PAISES A MIS PIES",
	"\255SERE EL PRIMER MALO EN LA HISTORIA QUE LO CONSIGA! \255JUA, JUA, JUA!",
	"\255A TI NADA, IDIOTA! ESTOY EXPONIENDO LA TRAMA. BIEN \250ESTA TODO LISTO?",
	// 10
	"\255LLEGO EL MOMENTO, ENTONCES! \255DALE AL INTERRUPTOR DE LAS PILAS ALCALINAS! ",
	"\255MALDITA SEA! \250QUE ES LO QUE HA PODIDO FALLAR?",
	"\250SEGURO QUE LO REVISASTE BIEN Y NO FALTABA NADA? ULTIMAMENTE ESTAS CON ESO DE LA RENTA QUE NO VES UN PIJO A DOS METROS",
	"\255IDIOTA, NO HABIAS CONECTADO EL INDIFIBULADOR! LOS TORNILLOS SE HABRAN MAGNETIZADO Y SE LE HABRA QUEMADO EL CEREBRO",
	"ESTAS MUERTO, ESTAS MUERTO, COMO TE COJA...",
	// 15
	"\255A CALLAR! MA\245ANA IRE A POR OTRO CEREBRO Y REPETIREMOS EL EXPERIMENTO ",
	"NO. ESTA VEZ TRAERE UNO DE MUJER, PARA QUE ESTE NUEVECITO Y SIN ESTRENAR. JA, JA, JA, QUE MALISIMA DE CHISTA",
	"\250Y QUE? YO SOY EL MALO Y SOY TODO LO MACHISTA QUE QUIERA \250ENTENDIDO? Y COMO ME VUELVAS A RECHISTAR TE TRAGAS LA CHEPA ",
	"JA, JA, JA. OTRO QUE HA CAIDO. AHORA VAS A PAGAR TU OSADIA DE QUERER ACABAR CONMIGO. \255IGOR, AL PENDULO DE LA MUERTE!",
	"DIME, HUMANO ESTUPIDO, \250COMO ES QUE TE HA DADO POR QUERER DESTRUIRME? ",
	// 20
	"\255QUE BONITO!, ME PONDRIA A LLORAR SI NO FUERA PORQUE ME HACE GRACIA",
	"A TU NOVIA LA NECESITO PARA QUE ME AYUDE CON SU CEREBRO A CONQUISTAR EL MUNDO",
	"\255SI, JA! SE LO QUITARE Y SE LO PONDRE A MI FRUSKYNSTEIN, Y CON EL DOMINARE EL MUNDO, JA, JA, JA",
	"\255\250QUE?! \255ESTAS MUERTO, ESTAS MUERTO! TE VOY A... ME HAS TOCADO LAS NARICES, VAMOS. \255PREPARATE A SER MATADO!",
	"JA, JA, JA. QUE TE CREES TU ESO",
	// 25
	"SI, \250VERDAD? JA, JA, JA ",
	"AH, ESTA BIEN, PUEDES FUMARTE EL ULTIMO CIGARRILLO, PERO DEPRISITA",
	"APAGA YA ESE CIGARRILLO, ME TIENES HARTO YA",
	"Y DIME, \250ESA POCION TIENE EL EFECTO CONTRARIO?",
	"ESO YA LO VEREMOS...",
	// 30
	"BUENO, A VER SI ES VERDAD. IGOR, TRAE EL COMPACT DISC DE U\245AS ARRASCANDO UNA PIZARRA",
	"ESO NI SO\245ARLO. LA CHICA SE QUEDA CONMIGO, Y TU TE QUEDAS AHI HASTA QUE EL PENDULO TE CORTE EL RODAJITAS. JA, JA, JA",
	"PERO QUE MALO QUE SOY, VAMOS IGOR, VAMOS A PREPARAR LA POCION Y CONQUISTAR EL MUNDO",
	"\250QUE PASA AHORA?",
	"SI, \250QUE PASA?... \255ANDA, EL PARTIDO!",
	// 35
	"SE ME HABIA OLVIDADO. COGE A LA CHICA Y VAMONOS A VERLE. YA CONQUISTARE EL MUNDO DESPUES",
	"GRACIAS MACHO, TENIA SED",
	"\255ArgH! \255ESE CRUCIFIJO! \255ESE CRUCIFIJO!...",
	"QUE BONITO ES ESE CRUCIFIJO, NO ME HABIA YO FIJADO",
	"DEJAME, QUE ESTOY VIENDO EL FUTBOL",
	// 40
	"",
	"",
	"",
	"",
	"",
	// 45
	"",
	"",
	"",
	"",
	"",
	// 50
	"",
	"",
	"",
	"",
	"",
	// 55
	"",
	"Hola ciego. \250Que tal?",
	"\250Como sabes que soy extranjero?",
	"Pareces un ciego. Tienes gafas como serafin zubiri, hablas mirando al frente como Stevie Wonder...",
	"Bueno, perdona. No sabia que vieses.",
	// 60
	"\250Pero no me acabas de decir que no eres ciego?",
	"\255Pero si no ves!",
	"Vaaaale. Perdona. En ese caso: Hola invidente.",
	"Soy John Hacker, y estoy jugando al Drascula. Tu debes ser el tipico personaje que me ayudara a cambio de un objeto. \250A que si? \250Eh? \250A que si?",
	"Em... Perdona que te pregunte cieg..\255Invidente!. Pero... \250Que clase de profesion es la tuya, de dar hoces a cambio de dinero, mientras tocas el acordeon?",
	// 65
	"Ah, si. Es verdad. Adios invidente...(ciego)",
	"Ahi tienes la cuantiosa cantidad de dinero que me pediste.",
	"Mas te vale.",
	"Hola extranjero.",
	"Y tu... \250Como sabes que soy ciego?",
	// 70
	"Y tu hablas como el hijo de Bill Cosby y no me meto contigo.",
	"No, si no veo.",
	"Y no lo soy.",
	"\255Oh, claro! Como no veo se me tacha de ciego \250no?",
	"\255Hola Extranjero! y \250que haces en Transilvania?",
	// 75
	"Correcto extranjero. A cambio de una cuantiosa suma de dinero, te dare una hoz, para cuando te haga falta.",
	"Shhhhhh. Soy traficante de hoces, tengo que disimular.",
	"Porque me lo has dicho antes \250no?",
	"Gracias extranjero. Aqui tienes tu hoz a cambio. Un objeto que te sera muy util algo mas adelante... de verdad.",
	"",
	// 80
	"",
	"",
	"No, nada",
	"\244a, \244a, \244a. que bueno, que bueno."
},
{
	// 0
	"",
	"WIE GEHTS, WIE STEHTS, IGOR?",
	"IMMER WENN AUF KABEL EIN GUTES SPIEL KOMMT, PASSIERT DAS GLEICHE. NAJA, DANN GUCKEN WIR ES UNS EBEN WIE IMMER IN DER KNEIPE AN",
	"JETZT PASS AUF, IGOR. WIR WERDEN JETZT PHASE 1 MEINES WELTEROBERUNGSPLANS STARTEN.",
	"PAH, EINIGE GEWITTERBLITZE, DIE WIR MIT DEM VIBROXATOR ENTMAGNETISIEREN. UND DER STROM WIRD MEINEM MONSTERCHEN LEBEN EINHAUCHEN.",
	// 5
	"WENN ALLES KLAPPT WIRD ER DER ERSTE SOLDAT EINER RIESIGEN ARMEE SEIN, DIE DIE WELT FueR MICH EROBERN WIRD, HARHARHAR",
	"DIE MONSTER WERDEN ALLE WAFFEN IN DER GANZEN WELT VERNICHTEN WaeHREND WIR UNS AUF UNSERE LaeNDEREIEN ZURueCKZIEHEN, DIE ICH UNS AUF HELGOLAND GEKAUFT HABE.",
	"DANN WERDEN WIR EINEN STAATSSTREICH MACHEN UND DIE REGIERUNGEN DIESER WELT WERDEN AUFGEBEN UND SICH MIR ZU FueSSEN WERFEN",
	"ICH WERDE DER ERSTE BoeSE IN DER GESCHICHTE SEIN, DER DAS SCHAFFT, HUAHUAHUA!",
	"GAR NICHTS, IDIOT! ICH ERZaeHLE DIE HANDLUNG. O.K. IST ALLES FERTIG?",
	// 10
	"ES IST ALSO SOWEIT! DRueCK AUF DEN SCHALTER MIT DEN ALKALI-BATTERIEN!",
	"VERFLIXT NOCHMAL! WAS IST DA SCHIEFGELAUFEN?",
	"BIST DU SICHER, DASS DU ALLES GECHECKT HAST? SEIT NEUESTEM DENKST DU NUR NOCH AN DEINE EINKOMMENSERKLaeRUNG UND KRIEGST NICHTS MEHR MIT",
	"DU IDIOT! DU HAST DEN VIBROXITOR NICHT EINGESCHALTET! JETZT SIND DIE SCHRAUBEN MAGNETISIERT UND IHM WIRD WOHL DAS GEHIRN VERBRANNT SEIN",
	"DU BIST EIN TOTER MANN, NA WARTE, WENN ICH DICH KRIEGE...",
	// 15
	"HALT DIE KLAPPE! MORGEN HOLE ICH EIN NEUES HIRN UND WIR WIEDERHOLEN DAS EXPERIMENT",
	"NEIN, DIESMAL BESORG ICH EIN FRAUENHIRN, DASS NOCH GANZ FRISCH UND UNBENUTZT IST, HAR,HAR,HAR, SUPERJOKE!",
	"NA UND? ICH BIN DER BoeSE UND DER SUPER-MACHO ODER WAS? UND WENN DU MIR NOCHMAL WIDERSPRICHST KANNST DU DEINEN BUCKEL SCHLUCKEN",
	"HARHARHAR, SCHON WIEDER EINER. DU WARST SO DREIST, MICH UMBRINGEN ZU WOLLEN. DAS WIRST DU JETZT BueSSEN. IGOR! ZUM TODESPENDEL MIT IHM!",
	"SAG MAL DU DUMMES MENSCHLEIN, WIESO WOLLTEST DU MICH ZERSToeREN?",
	// 20
	"ACH WIE SueSS! ICH KoeNNTE GLATT ANFANGEN ZU HEULEN, WENN ES NICHT SO LUSTIG WaeRE",
	"DEINE FREUNDIN BRAUCHE ICH NOCH, DAMIT SIE MIR MIT IHREM HIRN DABEI HILFT, DIE WELT ZU EROBERN",
	"JA, HA! ICH WERDE ES RAUSREISSEN UND ES FRUSKYNSTEIN GEBEN UND DANN WERDE ICH DIE WELT EROBERN,HARHARHAR",
	"WAS?! DU BIST EIN TOTER MANN! ICH WERDE DICH... DU HAST MICH VERaePPELT, GANZ KLAR. MACH DEIN TESTAMENT!",
	"HARHARHAR, GLAUB DU DAS MAL",
	// 25
	"JA, WIRKLICH? HARHARHAR",
	"JA, SCHON GUT, RAUCH DIR NOCH EINE LETZTE ZIGARETTE, ABER RUCKZUCK",
	"MACH MAL DIE ZIGARETTE AUS. MIR IST SCHON SCHLECHT",
	"UND SAG MAL, DIESE MISCHUNG HAT DIE GEGENWIRKUNG?",
	"DAS WERDEN WIR JA MAL SEHEN...",
	// 30
	"NA, MAL GUCKEN OB DAS STIMMT, IGOR. BRING MAL DIE CD MIT DEN DEM SOUND VON KRATZENDEN FINGERNaeGELN AUF EINER SCHULTAFEL",
	"NICHT IM TRAUM. DAS MaeDCHEN BLEIBT BEI MIR UND DU BLEIBST WO DU BIST, BIS DICH DAS PENDEL IN LECKERE SCHEIBCHEN SCHNEIDET, HARHAR",
	"WIE BoeSE ICH DOCH BIN. KOMM IGOR, WIR BEREITEN DIE MISCHUNG ZU UND EROBERN DANN DIE WELT",
	"WAS IST DENN JETZT WIEDER LOS?",
	"JA, WAS IST LOS?...VERDAMMT, DAS SPIEL!",
	// 35
	"DAS HAB ICH GLATT VERGESSEN. NIMM DAS MaeDCHEN UND LASS UNS DAS SPIEL ANGUCKEN GEHEN. DIE WELT EROBERN WIR DANACH.",
	"DANKE, ALTER, ICH HATTE DURST",
	"ARGHH! DAS KRUZIFIX DA! DAS KRUZIFIX!",
	"WIE SCHoeN DAS KRUZIFIX DA IST, HAB ICH GAR NICHT BEMERKT",
	"LASS MICH, ICH GUCK DOCH GERADE FUSSBALL",
	// 40
	"",
	"",
	"",
	"",
	"",
	// 45
	"",
	"",
	"",
	"",
	"",
	// 50
	"",
	"",
	"",
	"",
	"",
	// 55
	"",
	"Hallo Blinder. Wie gehts?",
	"Woher weisst Du, dass ich nicht von hier bin?",
	"Du siehst wie ein Blinder aus. Du guckst beim Sprechen immer gerade aus, und dann diese Stevie-Wonder-Brille....",
	"O.k, entschuldige. Ich dachte, du kannst nichts sehen",
	// 60
	"Aber hast Du mir nicht gerade gesagt, dass Du NICHT blind bist?",
	"Ich denke, Du siehst nichts?",
	"Okaaay,okaay! Tut mir leid. Also wenn das so ist: Sehkranker ",
	"Ich bin Heinz Hacker und spiele Drascula. Du bist bestimmt derjenige, der mir hilft und dafuer irgendwas haben will. Stimmts? Hae? Hab ich recht!?",
	"aeh....Entschuldige die Frage, Blind-Sehkranker! Aber...Was hast Du fuer einen komischen Beruf, dass Du Sicheln verkaufst und dabei Akkordeon spielst!",
	// 65
	"Ach, klar, stimmt. Auf Wiedersehen Sehkranker...(Blinder)",
	"Hier hast Du den Wahnsinnsbatzen Geld, um den Du mich gebeten hast",
	"Das ist auch besser so!",
	"Hallo Fremder!",
	"Und Du...Woher willst Du wissen, dass ich blind bin?",
	// 70
	"Und Du bist der Sohn von Bill Cosby und ich werde mich nicht mit Dir anlegen",
	"Nein, ich seh doch nichts!",
	"Und ich bin es nicht",
	"Na super! Nur weil ich nichts sehe bin ich direkt ein Blinder, oder was?",
	"Hallo Fremder! Und was machst Du in Transylvanien?",
	// 75
	"Richtig, Fremder. Gegen einen Wahnsinnsbatzen Geld bekommst Du von mir eine Sichel damit Du eine hast, wenn Du eine brauchst.",
	"Psssssst. Ich bin Sichel-Dealer, ich muss unentdeckt bleiben ",
	"Warum hast Du mir das nicht vorher gesagt?",
	"Danke, Fremder. Hier hast du dafuer Deine Sichel. Sie wird Dir nachher noch nuetzlich sein...glaub mir!",
	"",
	// 80
	"",
	"",
	"Nein, nichts",
	"bla, bla, bla."
},
{
	// 0
	"",
	"COMMENT VA TOUT, IGOR?",
	"C'EST TOUJOURS LA M\322ME CHOSE QUAND IL Y A UN BON MATCH \267 LA PARABOLIQUE! ENFIN, ALLONS LE VOIR AU BAR, COMME D'HABITUDE",
	"MAINTENANT, COUTE IGOR. NOUS ALLONS RALISER LA PHASE 1 DE MON PLAN POUR CONQURIR LE MONDE",
	"D'ABORD, ON SAISIRA L'UNE DES FOUDRES DE L'ORAGE ET ON LA DMAGNTISERAIT AVEC l'INDIFUBULATEUR. LA COURANTE PASSERA DANS MON MONSTRE ET LUI DONNERA LA VIE!",
	// 5
	"SI TOUT VA BIEN CELUI-L\267 NE SERA QUE LE PREMIER D'UNE IMMENSE ARME QUE CONQURRA LE MONDE POUR MOI, HA! HA! HA! ",
	"LES MONSTRES VONT ANANTIR TOUTES LES ARMES DE TOUTES LES ARMES DU MONDE, TANDIS QUE NOUS NOUS RFUGIONS DANS LES TERRAINS QUE J'AI ACHET \267 GIBRALTAR",
	"ALORS, ON DONNERA UN COUP D'TAT. LES GOUVERNEMENTS DU MONDE N'AURONT PAS DE PROTECTION ET SES PAYS SERONT \267 MES PIEDS",
	"JE SERAI LE PREMIER MALIN DE L'HISTOIRE \267 Y AVOIR RUSSI! -HOUA! HOUA! HOUA!",
	"POUR TOI RIEN, IDIOT! JE PRSENTE LA TRAME. BIEN, TOUT EST PR\322T? ",
	// 10
	"LE MOMENT EST ARRIV\220, ALORS! -APPUIE DONC SUR L'INTERRUPTEUR DES BATTERIES ALCALINES! ",
	"H\220LAS! QU'EST-CE QUE N'A PAS MARCH\220?",
	"C'EST S\352R QUE TU L'AS BIEN R\220VIS\220 ET IL NE MANQUAIT RIEN? DERNI\324REMENT AVEC CETTE HISTOIRE DE LA RENTE TU N'Y VOIS GOUTTE",
	"IDIOT!  TU N'AVAIS PAS CONNECT\220 L'INFIBULATEUR! LES VIS SE SERONT MAGN\220TIS\220ES ET SA CERVELLE AURA BR\352L\220E",
	"TU ES MORT, TU ES MORT, SI JE T'ATTRAPE...",
	// 15
	"TAIS-TOI! DEMAIN J'AURAI UNE AUTRE CERVELLE ET ON FERA L'ESSAI \267 NOUVEAU",
	"NON, CETTE FOIS J'APPORTERAI UNE DE FEMME POUR QU'ELLE SOIT TOUTE NEUVE ET DE PREMI\324RE MAIN. HA! HA! HA! QUE MALIN DE XISTE",
	"ET QUOI? JE SUIS LE MALIN ET TOUT LE SEXISTE QUE JE D\220SIRE, C'EST ENTENDU? SI TU ME R\220PLIQUES ENCORE JE TE FAIS ENGLOUTIR TA BOSSE",
	"HA! HA! HA!. UN AUTRE QUI S'EST LAISS\220 PRENDRE. MAINTENANT TU PAIERAS CHER TON AUDACE DE VOULOIR EN FINIR AVEC MOI. -IGOR, AU PENDULE DE LA MORT!",
	"DIS-MOI, HUMAIN STUPIDE, POURQUOI TU AS PENS\220 ME D\220TRUIRE? ",
	// 20
	"QUE C'EST BEAU! JE SANGLOTERAIS SI CE N'ETAIT PAS DR\342LE",
	"J'AI BESOIN DE TA FIANC\220E POUR QU'ELLE M'AIDE \267 CONQU\220RIR LE MONDE AVEC SA CERVELLE",
	"OUI, HA! JE LA LUI ARRACHERAI ET L'INSTALLERAI SUR MON FRUSKYNSTEIN, ET AVEC LUI JE VAIS MA\327TRISER LE MONDE, HA! HA! HA!",
	"QUOI?! -TU ES MORT, TU ES MORT! JE VAIS TE... TU M'AS CASS\220 LE NEZ, ALLONS, -APPR\322TE-TOI \267 \322TRE TU\220!",
	"HA! HA! HA! C'EST-CE QUE TU CROIS",
	// 25
	"OUI, N'EST-CE PAS? HA! HA! HA!",
	"AH, C'EST BIEN! TU PEUX FUMER LA DERNI\324RE CIGARETTE, MAIS D\220P\322CHE-TOI!",
	"\220TEINS D\220J\267 CETTE CIGARETTE, J'EN A RAS LE BOL!",
	"ET DIS-MOIS, CETTE POTION, A-T-ELLE L'EFFET CONTRAIRE?",
	"ON VERRA \200A...",
	// 30
	"EH BIEN, ON VERRA SI C'EST VRAI. IGOR, APPORTE LE  COMPACT DISC D'ONGLES GRATTANT UN TABLEAU",
	"N'Y PENSES PAS. LA FILLE RESTE AVEC MOI, ET TOI TU RESTERAS L\267 JUSQU'\267 QUE LA PENDULE TE COUPE EN PETITES TRANCHES. HA! HA! HA!",
	"MAIS QUE JE SUIS MALIN. ON Y VA, IGOR, ALLONS PR\220PARER LA POTION ET CONQU\220RIR LE MONDE",
	"QU'Y A-T-IL MAINTENANT?",
	"OUI, QU'Y A-T-IL?... -TIENS, LE MATCH!",
	// 35
	"JE L'AVAIS OUBLI\220. PRENDS LA FILLE ET ALLONS LE VOIR. J'IRAI CONQU\220RIR LE MONDE PLUS TARD",
	"MERCI MON VIEUX, J'AVAIS SOIF",
	"ArgH! CE CRUCIFIX! -CE CRUCIFIX!...",
	"C'EST BEAU CE CRUCIFIX, JE NE M'AVAIS PAS RENDU COMPTE",
	"FICHE-MOI LA PAIX! JE REGARDE LE FOOT-BALL",
	// 40
	"",
	"",
	"",
	"",
	"",
	// 45
	"",
	"",
	"",
	"",
	"",
	// 50
	"",
	"",
	"",
	"",
	"",
	// 55
	"",
	"BONJOUR, L'AVEUGLE. A VA?",
	"POURQUOI TU SAIS QUE JE SUIS UN TRANGER?",
	"TU PARAT UN AVEUGLE. TU AS DES LUNETTES COMME SERAFIN ZUBIRI ET TU PARLES EN REGARDANT DEVANT TOI, COMME STEVIE WONDER...",
	"BON, EXCUSE-MOI. JE NE SAVAIS PAS QUE TU PUISSES VOIR.",
	// 60
	"MAIS, TU NE VIENS PAS DE ME DIRE QUE TU N'EST PAS AVEUGLE?",
	"MAIS TU NE VOIS PAS!",
	"A VA, A VA.... PARDONNE-MOI. DANS CE CAS-L: BONJOUR AVEUGLE.",
	"JE SUIS JOHN HACQUER ET JE JOUE AU DRASCULA. TU ES SREMENT LE TYPIQUE PERSONNAGE QUI VA M'AIDER EN TROC D'UN OBJET. D'ACCORD? H? D'ACCORD?",
	"EUH... AVEUGLE, UNE QUESTION! MAIS... QUEL GENRE DE MTIER C'EST LE TIEN? CELUI D'CHANGER DE FAUCILLES CONTRE DE L'ARGENT EN JOUANT DE L'ACCORDON?",
	// 65
	"AH, OUI! C'EST VRAI. AU REVOIR AVEUGLE...",
	"VOIL LA GROSSE SOMME D'ARGENT QUE TU M'AS DEMANDE.",
	"IL TE VAUX MIEUX.",
	"BONJOUR, TRANGER!",
	"ET TOI... COMMENT TU SAIS QUE JE SUIS UN AVEUGLE?",
	// 70
	"ET TOI TU PARLES COMME LE FILS DE BILL COSBY ET MOI, JE NE T'EMBTE PAS.",
	"NON, SI JE NE VOIS PAS.",
	"ET MOI, JE NE LE SUIS PAS.",
	"OH, BIEN SR!  COMME JE NE VOIS PAS ON ME TCHE D'AVEUGLE, N'EST-CE PAS",
	"BONJOUR, TRANGER! ET QU'EST-CE QUE TU FAIS EN TRANSYLVANIE?",
	// 75
	"C'EST CORRECT, TRANGER. POUR UNE GROSSE SOMME D'ARGENT, JE TE DONNERAI  UNE FAUCILLE, POUR QUAND TU EN AURAS BESOIN.",
	"CHUT! JE SUIS UN TRAFICANT DE FAUCILLES, JE DOIS ME CACHER.",
	"PARCE QUE TU ME L'AS DIT AVANT, N'EST-CE PAS?",
	"MERCI TRANGER. VOIL TA FAUCILLE EN CHANGE. UNE CHOSE QUI SERA TRS PRATIQUE POUR TOI, PLUS TARD... VRAIMENT.",
	"",
	// 80
	"",
	"",
	"No, nada",
	"bla, bla, bla."
},
{
	// 0
	"",
	"COME STA ANDANDO, IGOR?",
	"\324 SEMPRE LA STESSA STORIA, OGNI VOLTA CHE C'\324 UNA BELLA PARTITA SUL SATELLITE! COMUNQUE ANDREMO A VEDERLA AL BAR, COME AL SOLITO",
	"ADESSO ASCOLTA BENE, IGOR, SIAMO ALLA FASE NUMERO UNO DEL MIO PIANO PER LA CONQUISTA DEL MONDO",
	"ATTIREREMO UN FULMINE E LO DEMAGNETIZZEREMO CON L'INDIFIBULATORE. L'ELETTRICIT\267 VERR\267 TRASFERITA AL MOSTRO E GLI DAR\267 LA VITA!",
	// 5
	"SE TUTTO ANDR\267 PER IL VERSO GIUSTO, QUESTO SAR\267 L'INIZIO DI UN GRANDE ESERCITO CHE CONQUISTER\267 IL MONDO PER ME, AH, AH, AH",
	"I MOSTRI DISTRUGGERANNO TUTTI GLI ESERCITI DEL MONDO, MENTRE NOI CI RIFUGEREMO IN UNO DEI TERRENI CHE HO COMPRATO A GIBILTERRA",
	"POI FAREMO UN COLPO DI STATO, I GOVERNI DI TUTTO IL MONDO NON AVRANNO MODO DI DIFENDERSI E SI PROSTRERANNO AI MIEI PIEDI",
	"SAR\343 IL PRIMO CATTIVO DELLA STORIA A RIUSCIRCI! AH, AH!",
	"NON STO PARLANDO CON TE, IDIOTA! STO ESPONENDO LA TRAMA. BENE, \324 TUTTO PRONTO?",
	// 10
	"\324 IL MOMENTO! PREMI L'INTERRUTTORE DELLE BATTERIE ALCALINE!",
	"PORCA MISERIA! COS'\324 ANDATO STORTO?",
	"SEI SICURO DI AVERLO CONTROLLATO BENE E CHE NON MANCASSE NULLA? ULTIMAMENTE QUESTA STORIA DELLE TASSE TI STA MANDANDO DAVVERO FUORI DI TESTA",
	"IDIOTA! HAI DIMENTICATO DI CONNETTERE L'INDIFIBULATORE. PROBABILMENTE LE VITI SI SARANNO MAGNETIZZATE E IL SUO CERVELLO SI SAR\267 ABBRUSTOLITO",
	"SEI MORTO, SEI MORTO... ASPETTA CHE TI PRENDA!",
	// 15
	"STAI ZITTO! DOMANI CERCHER\343 UN ALTRO CERVELLO E RIPETEREMO L'ESPERIMENTO",
	"NO. QUESTA VOLTA PRENDER\343 UN CERVELLO DI DONNA. PRATICAMENTE NUOVO, MAI UTILIZZATO. AH, AH,AH, BUONA QUESTA. ",
	"E ALLORA? SONO IO IL CATTIVO DELLA STORIA, E POSSO ESSERE MASCHILISTA QUANTO VOGLIO, CAPITO? E SE DICI ANCORA QUALCOSA TI APPICCICO LA GOBBA IN FRONTE!",
	"AH, AH, AH, CI SEI CASCATO ANCHE TU!! ADESSO LA PAGHERAI PER AVER OSATO SFIDARMI! IGOR, PORTALO AL PENDOLO DELLA MORTE!",
	"DIMMI, STUPIDO UMANO, COME MAI VOLEVI DISTRUGGERMI?",
	// 20
	"CHE BELLO! SE NON MI FACESSE RIDERE, MI METTEREI A PIANGERE",
	"HO BISOGNO DELLA TUA RAGAZZA, IL SUO CERVELLO MI AIUTER\267 A CONQUISTARE IL MONDO",
	"S\326, SICURO! LO PRENDER\343 DA LEI E LO DAR\343 AL MIO FRUSKYNSTEIN. IL MONDO SAR\267 NELLE MIE MANI, AH, AH, AH",
	"COSA!? SEI UN UOMO MORTO! TI FAR\343... MI HAI FATTO VERAMENTE ARRABBIARE... PREPARATI A MORIRE!",
	"AH, AH, AH. TI PIACEREBBE!",
	// 25
	"S\326, VERO? AH, AH AH",
	"VA BENE, PUOI FUMARTI L'ULTIMA SIGARETTA. MA FAI PRESTO, OK?",
	"MI HAI STUFATO, BUTTA VIA QUELLA SIGARETTA!",
	"DIMMI UNA COSA, QUELLA POZIONE HA ANCHE L'EFFETTO OPPOSTO?",
	"QUESTO LO VEDREMO...",
	// 30
	"OK, ADESSO VEDREMO. IGOR DAMMI IL CD \"UNGHIE CHE GRAFFIANO LA LAVAGNA\"",
	"NEANCHE PER SOGNO. LA RAGAZZA RIMANE CON ME. E TU ASPETTERAI FINCH\220 IL PENDOLO NON TI AVR\267 FATTO A FETTINE. AH, AH, AH",
	"CAVOLI, SONO DAVVERO CATTIVO... ANDIAMO, IGOR; PREPARIAMO LA POZIONE E CONQUISTIAMO IL MONDO",
	"ADESSO CHE SUCCEDE?",
	"S\326, CHE C'\324?... OH DANNAZIONE, LA PARTITA!",
	// 35
	"ME L'ERO DIMENTICATA. PRENDI LA RAGAZZA E ANDIAMO A GUARDARCELA. CONQUISTEREMO IL MONDO PI\353 TARDI",
	"GRAZIE AMICO, AVEVO SETE",
	"ARGH! QUEL CROCIFISSO! QUEL CROCIFISSO!...",
	"CHE BELLO QUEL CROCIFISSO, NON L'AVEVO NOTATO",
	"LASCIAMI IN PACE, STO GUARDANDO LA PARTITA",
	// 40
	"",
	"",
	"",
	"",
	"",
	// 45
	"",
	"",
	"",
	"",
	"",
	// 50
	"",
	"",
	"",
	"",
	"",
	// 55
	"",
	"CIAO CIECO. COME VA?",
	"COME SAI CHE SONO UNO STRANIERO?",
	"SEMBRI UN CIECO. HAI GLI OCCHIALI COME IL CIECO DI SORRENTO, PARLI GUARDANDO ALL'INFINITO COME STEVIE WONDER..",
	"GUARDA, MI DISPIACE. NON SAPEVO CHE CI VEDESSI",
	// 60
	"MA NON MI HAI APPENA DETTO CHE NON SEI CIECO?",
	"MA SE NON VEDI!",
	"BEEEENE. SCUSA. ALLORA: \"CIAO NON VEDENTE\"",
	"SONO JOHN HACKER, STO GIOCANDO A DRASCULA. TU DEVI PROPRIO ESSERE IL TIPICO PERSONAGGIO CHE MI AIUTER\267 IN CAMBIO DI UN OGGETTO. VERO? EH? VERO?",
	"SCUSA SE TE LO DOMANDO, CIEC... NON VEDENTE! MA... CHE TIPO DI MESTIERE \324 IL TUO, DI DARE FALCI PER SOLDI, MENTRE SUONI LA FISARMONICA?",
	// 65
	"AH, S\326. \324 VERO. CIAO NON VEDENTE...(CIECO)",
	"ECCO LA COSPICUA SOMMA DI DENARO CHE MI HAI CHIESTO",
	"LO SPERO PROPRIO",
	"CIAO STRANIERO.",
	"E TU... COME SAI CHE SONO CIECO?",
	// 70
	"E TU PARLI COME IL FIGLIO DI BILL COSBY E NON SCHERZO.",
	"NO, NON CI VEDO.",
	"E NON LO SONO.",
	"OH, CERTO! SICCOME NON CI VEDO TU MI ACCUSI DI ESSERE CIECO.",
	"CIAO STRANIERO! COSA CI FAI IN TRANSILVANIA?",
	// 75
	"CORRETTO, STRANIERO. PER UN'ABBONDANTE QUANTIT\267 DI DENARO TI DAR\343 UNA FALCE, PER QUANDO NE AVRAI BISOGNO.",
	"SHHHHHH. SONO TRAFFICANTE DI FALCI, PER QUESTO DEVO FINGERE.",
	"PERCH\220 ME LO HAI DETTO PRIMA, NO?",
	"GRAZIE STRANIERO. ECCO LA TUA FALCE. UN OGGETTO CHE TI SAR\267 MOLTO UTILE PI\353 AVANTI...... DAVVERO.",
	"",
	// 80
	"",
	"",
	"NO, NIENTE",
	"BLA, BLA, BLA.",
},
};

const char *_textb[NUM_LANGS][NUM_TEXTB] = {
{
	// 0
	"",
	"I'M HERE, DRINKING",
	"THEY ARE ALL DEAD, THANKS.  HIC...!",
	"YES, SURE...",
	"SHE IS ATTRACTED TO UNCLE DESIDERIO",
	// 5
	"EVEN BETTER, UNCLE DESIDERIO'S DEAD BODY",
	"MY UNCLE. HE WENT TO THE CASTLE AND NEVER CAME BACK",
	"WELL, HE CAME BACK JUST A LITTLE. IF ONLY VON BRAUN HADN'T SCREWED THINGS UP MY UNCLE WOULD BE DRINKING WITH US NOW",
	"NOTHING...",
	"WELL, YES! THAT MEAN MAN HAS TERRIFIED US ALL",
	// 10
	"FROM TIME TO TIME HE COMES DOWN TO THE VILLAGE AND TAKES SOMEONE WITH HIM.",
	"A LITTLE WHILE AFTER WE FOUND A FEW BODY PARTS. I THINK HE IS DEALING IN ORGANS OR SOMETHING LIKE THAT",
	"THE ONLY PERSON IN THE VILLAGE WHO KNOWS HOW TO GET TO DRASCULA IS A CULTIVATED PERSON",
	"HE LIVES IN A LOG-CABIN OUT OF TOWN, EVER SINCE DRASCULA BEAT HIM UP",
	"HE IS THE ONLY ONE WHO COULD HELP US WITH DRASCULA, AND HE DOESN'T WANT TO HEAR ABOUT IT. HOW DO YOU LIKE THAT?",
},
{
	// 0
	"",
	"AQUI, BEBIENDO",
	"MUERTOS TODOS. GRACIAS. BURRP ",
	"SI, NO VEAS... ",
	"ESTA POR EL TIO DESIDERIO",
	// 5
	"Y ESTA OTRA, POR EL CADAVER DEL TIO DESIDERIO",
	"MI TIO. FUE AL CASTILLO Y NO VOLVIO",
	"BUENO, VOLVIO PERO POCO. SI EL VON BRAUN NO HUBIESE METIDO LA PATA MI TIO ESTARIA AQUI BEBIENDO",
	"NADA... ",
	"\255BUENO, SI! ESE DESALMADO NOS TIENE ATEMORIZADOS A TODOS",
	// 10
	"DE VEZ EN CUANDO BAJA AL PUEBLO Y SE LLEVA A ALGUIEN",
	"POCO DESPUES SOLO ENCONTRAMOS UNOS POCOS RESTOS. CREO QUE ESTA TRAFICANDO CON ORGANOS, O ALGO ASI",
	"EL UNICO DEL PUEBLO QUE SSABE COMO ACABAR CON DRASCULA TIENE ESTUDIOS",
	"DESDE QUE LE DERROTO DRASCULA SE HA APARTADO A UNA CABA\245A EN LAS AFUERAS DEL PUEBLO",
	"ES EL UNICO QUE PODRIA AYUDARNOS A ACABAR CON DRASCULA, Y NO QUIERE SABER NADA DE NOSOTROS. \250QUE TE PARECE A TI?",
},
{
	// 0
	"",
	"HIER, BEI EINEM GUTEN TROPFEN",
	"ALLE TOT. VIELEN DANK. RueLPS!",
	"YO, STIMMT...",
	"DIE HIER IST FueR ONKEL DESIDERIUS",
	// 5
	"UND DIESE HIER FueR DIE LEICHE VON ONKEL DESIDERIUS",
	"MEIN ONKEL. ER GING ZUM SCHLOSS UND KAM NICHT ZURueCK",
	"NAJA, ER KAM NUR IN RESTEN ZURueCK. WENN VON BRAUN SICH NICHT VERTAN HaeTTE, WueRDE ER JETZT MIT UNS EINEN HEBEN",
	"NICHTS...",
	"JA STIMMT! DIESER SCHURKE JAGT UNS ALLEN EINE RIESENANGST EIN",
	// 10
	"AB UND ZU GEHT ER RUNTER INS DORF UND HOLT SICH JEMANDEN",
	"KURZ DANACH HABEN WIR NUR NOCH RESTE GEFUNDEN, ICH GLAUBE ER HANDELT MIT ORGANEN ODER SO",
	"DER EINZIGE, DER MIT DRASCULA FERTIGWERDEN KANN, WAR AN DER UNI",
	"SEITDEM DRASCULA IHN BESIEGT HAT, LEBT ER IN EINER EINSAMEN HueTTE AUSSERHALB DES DORFES",
	"ER IST DER EINZIGE, DER DRASCULA ERLEDIGEN KoeNNTE UND WILL NICHTS VON UNS WISSEN. WAS HaeLST DU DAVON?",
},
{
	// 0
	"",
	"ICI, EN BUVANT",
	"TOUS MORTS. MERCI. BOURRP",
	"OUI, VRAIMENT...",
	"CELLE-CI POUR L'ONCLE DSIR",
	// 5
	"ET CELLE-L\267 POUR LE CADAVRE D'ONCLE DSIR",
	"MON ONCLE EST ALL AU CH\266TEAU ET N'EST PAS ENCORE REVENU",
	"BON, IL EST REVENU MAIS POUR PEU DE TEMPS. SI VON BRAUN N'AURAIT FAIT UN IMPAIR, MON ONCLE DSIR SERAIT ICI EN BUVANT",
	"RIEN... ",
	"EH OUI! CE MALIN NOUS A INTIMIDS \267 TOUS",
	// 10
	"DE FOIS IL DESCEND SUR LE VILLAGE ET ENLEVE QUELQU'UN",
	"UN PEU PLUS TARD ON NE TROUVE QUE QUELQUES RESTES. JE PENSE QU'IL FAIT DU TRAFIQUE D'ORGANES, OU QUELQUE CHOSE PAREILLE",
	"LE SEUL DU VILLAGE QUI SAIT COMMENT FINIR AVEC DRASCULA IL A DES \220TUDES",
	"DEPUIS QUE DRASCULA L'A VAINCU, IL S'EST RETIR\220 DANS UNE CABANE, EN DEHORS DU VILLAGE",
	"C'EST LE SEUL QUI POURRAIT NOUS AIDER \267 FINIR AVEC DRASCULA ET LUI NE VEUX RIEN SAVOIR DE NOUS. QU'EN PENSES-TUI?",
},
{
	// 0
	"",
	"SONO QUI, STO BEVENDO",
	"SONO TUTTI MORTI. GRAZIE. BURRP",
	"S\326, CERTO, SICURO...",
	"QUESTA \324 PER LO ZIO DESIDERIO",
	// 5
	"E QUEST'ALTRA \324 PER IL CADAVERE DELLO ZIO DESIDERIO",
	"MIO ZIO. \324 ANDATO AL CASTELLO E NON \324 PI\353 TORNATO",
	"EH, TORN\343, MA NON TUTTO INTERO. SE VON BRAUN NON AVESSE SBAGLIATO, ADESSO MIO ZIO SAREBBE QUI A BERE CON NOI",
	"NIENTE...",
	"EH, S\326! QUEL MALVAGIO CI HA INTIMORITI TUTTI",
	// 10
	"A VOLTE SCENDE IN PAESE E QUANDO SE NE VA SI PORTA VIA QUALCUNO",
	"POCO DOPO TROVIAMO SOLTANTO QUALCHE RESTO. CREDO CHE SIA UN TRAFFICANTE DI ORGANI O QUALCOSA DEL GENERE",
	"\324 L'UNICA PERSONA DEL VILLAGGIO CHE SAPPIA COME FARLA FINITA CON DRASCULA. \324 UNO CHE HA STUDIATO",
	"DA QUANDO FU SCONFITTO DA DRASCULA VIVE IN UNA BARACCA FUORI DAL PAESE",
	"LUI \324 L'UNICO CHE POTREBBE AIUTARCI A SCONFIGGERE DRASCULA, MA NON VUOLE SAPERNE. TU CHE NE PENSI?",
},
};

const char *_textbj[NUM_LANGS][NUM_TEXTBJ] = {
{
	// 0
	"",
	"ARE YOU ALL RIGHT? HEY, COME ON, WAKE UP! CAN YOU HEAR ME? ARE YOU DEAD?",
	"NO, MY NAME IS BILLIE JEAN, BUT YOU CAN CALL ME B.J. IT'S SHORTER",
	"HA, HA...! THAT WAS A GOOD ONE!",
	"WELL, JOHNNY. YOU SEE, I WAS HERE JUST READY TO GO TO BED WHEN I HEARD THIS LOUD NOISE DOWN THE CORRIDOR",
	// 5
	"I DIDN'T PAY ATTENTION AT FIRST, BUT AFTER ABOUT TWO HOURS OR SO I COULDN'T SLEEP AND WENT OUT FOR A WALK",
	"AS I OPENED THE DOOR I WAS SHOCKED TO FIND YOU THERE, LYING ON THE FLOOR. I THOUGHT YOU WERE DEAD, I SWEAR... HA, HA, SILLY BILLY",
	"I WAS GOING TO GIVE YOU THE KISS OF LIFE BUT IT WASN'T NECESSARY BECAUSE YOU STARTED TO TALK",
	"YOU SAID SOMETHING ABOUT A SCARECROW. I WAS VERY SCARED, YOU KNOW. IT'S A REAL SHOCK WHEN A DEAD PERSON STARTS TALKING?",
	"ISN'T THAT RIGHT? WELL, THEN I MANAGED TO BRING YOU TO MY ROOM THE BEST WAY I COULD. I PUT YOU IN MY BED... AND THAT'S ALL... HA, HA, HA...",
	// 10
	"OH, NO...! IT WASN'T THE HIT, HA, HA. I JUST STEPPED ON YOUR GLASSES BY ACCIDENT",
	"YOU REALLY LOOK GOOD WITH THOSE GLASSES. I KNOW HE'S NOT FERNANDO LANCHA, BUT I FIND HIM ATTRACTIVE...",
	"YES, YES, I DO... COME ON, HOLD ME AND KISS ME TIGHT",
	"OH JOHNNY, HONEY, THANK GOD YOU'RE HERE... THAT DAMNED DRASCULA TIED ME UP TO THE BED AND HE'S GONE DOWNSTAIRS TO WATCH THE FOOTBALL GAME",
	"YES, IT'S TRUE. PLEASE, SET ME FREE",
	// 15
	"NO, I'M SORRY. I USED THEM ALL IN THE TOWER WHEN I WAS TRYING TO ESCAPE WHILE YOU LET ME DOWN",
	"JOHNNY, IS THAT YOU? OH, GOD, GREAT! I KNEW YOU'D COME!",
	"YOU DON'T KNOW HOW MUCH THAT EVIL DRASCULA HAS MADE ME SUFFER",
	"FIRSTLY HE BROUGHT ME FLYING OVER HERE AND THEN PUT ME IN THIS DISGUSTING ROOM WITHOUT A MIRROR OR ANYTHING",
	"I'M TELLING YOU! AND THE WORST PART IS THAT HE DIDN'T EVEN APOLOGIZE, NOT EVEN ONCE",
	// 20
	"JOHNNY HONEY, WHERE ARE YOU?",
	"I'M READY TO LEAVE DEAR",
	"WAIT, I'M GOING TO TAKE A LOOK... NO DARLING, I'M SORRY",
	"THERE YOU GO...",
	"\"DEAR JOHNNY\"",
	// 25
	"I'LL NEVER FORGET YOU BUT I'VE REALIZED THAT THIS JUST COULDN'T WORK OUT RIGHT. TO BE HONEST, I'LL TELL YOU THAT THERE IS ANOTHER MAN. HE'S TALLER, STRONGER",
	"AND HE HAS ALSO RESCUED ME FROM DRASCULA. HE HAS ASKED ME TO MARRY HIM, AND I HAVE ACCEPTED",
	"BYE JOHNNY. PLEASE DON'T TRY TO FIND SOME KIND OF EXPLANATION. YOU KNOW LOVE IS BLIND AND HAS IT'S OWN WAYS",
	"I HOPE THERE WON'T BE HARD FEELINGS BETWEEN US. REMEMBER THAT I STILL LOVE YOU, BUT ONLY AS A FRIEND",
},
{
	// 0
	"",
	"\250ESTA USTED BIEN? OIGA, DESPIERTE. \250ME OYE? \250ESTA USTED MUERTO?",
	"NO, MI NOMBRE ES BILLIE JEAN, PERO PUEDES LLAMARME B.J. ES MAS CORTO",
	"\255JI, JI! \255QUE BUENO!",
	"PUES VERAS JOHNY, YO ESTABA AQUI, DISPUESTA A ACOSTARME, CUANDO OI UN FUERTE GOLPE EN EL PASILLO",
	// 5
	"AL PRINCIPIO NO LE DI MUCHA IMPORTANCIA, PERO A LAS DOS HORAS MAS O MENOS DESCUBRI QUE NO PODIA DORMIR Y DECIDI SALIR A DAR UN PASEO",
	"Y CUAL NO SERIA MI SORPRESA CUANDO ABRI LA PUERTA Y TE ENCONTRE AHI, EN EL SUELO TIRADO. TE LO JURO QUE PENSE QUE HABIAS MUERTO, JE, JE, TONTA DE MI",
	"TE FUI A HACER EL BOCA A BOCA, PERO NO FUE NECESARIO PORQUE EMPEZASTE A HABLAR",
	"DECIAS NO SE QUE DE UN ESPANTAPAJAROS. YO ME LLEVE UN SUSTO MUY GORDO, PORQUE CUANDO UN MUERTO SE PONE A HABLAR ES UNA IMPRESION MUY FUERTE, \250NO CREES?",
	"\250VERDAD QUE SI? BUENO, PUES COMO PUDE, CARGUE CONTIGO Y TE TRAJE A MI HABITACION. TE PUSE SOBRE LA CAMA... Y ESO ES TODO. JI, JI,JI",
	// 10
	"OH, NO HA SIDO EL GOLPE, JI, JI. ES QUE SIN QUERER PISE TUS GAFAS",
	"\255HAY QUE VER QUE BIEN LE SIENTAN LAS GAFAS! YA SE QUE NO ES FERNANDO LANCHA, PERO TIENE UN NO SE QUE QUE QUE SE YO",
	"SI, SI, QUIERO... VAMOS, ABRAZAME FUERTE, BESAME MUCHO...",
	"OH JOHNY, CARI\245O, MENOS MAL QUE HAS VENIDO. ESE DESALMADO DE DRASCULA ME HA ATADO A LA CAMA Y LUEGO SE HA IDO ABAJO A VER EL PARTIDO",
	"SI, ES VERDAD. LIBERAME ",
	// 15
	"NO, LO SIENTO. GASTE TODAS EN LA MAZMORRA INTENTANDO LIBERARME MIENTRAS TU ME DEJABAS TIRADA",
	"JOHNY \250ERES TU? \255QUE ALEGRIA! \255SABIA QUE VENDRIAS!",
	"NO SABES CUANTO ME HA HECHO SUFRIR ESE DESALMADO DE DRASCULA",
	"PRIMERO ME TRAJO VOLANDO HASTA AQUI Y LUEGO ME ENCERRO EN ESTE CUARTUCHO, SIN ESPEJO NI NADA",
	"COMO LO OYES. Y LO PEOR ES QUE NO PIDIO PERDON NI UNA SOLA VEZ",
	// 20
	"JOHNY, QUERIDO. \250DONDE ESTAS?",
	"ESTOY LISTA PARA QUE ME SAQUES DE AQUI",
	"ESPERA QUE VOY A MIRAR... NO CARI\245O, LO SIENTO",
	"TOMA...",
	"\"QUERIDO JOHNY:",
	// 25
	"NUNCA PODRE OLVIDARTE, PERO ME HE DADO CUENTA DE QUE LO NUESTRO NO FUNCIONARIA. VOY A SER SINCERA CONTIGO: HAY OTRO HOMBRE; MAS ALTO, MAS FUERTE...",
	"Y ADEMAS ME HA RESCATADO DE LAS MANOS DE DRASCULA. ME HA PEDIDO EN MATRIMONIO, Y YO HE ACEPTADO",
	"ADIOS, JOHNY. NO INTENTES DARLE UNA EXPLICACION, PORQUE EL AMOR ES CIEGO Y NO ATIENDE A RAZONES",
	"ESPERO QUE NO ME GUARDES RENCOR, Y RECUERDA QUE TE SIGO QUERIENDO, AUNQUE SOLO COMO AMIGO\"",
},
{
	// 0
	"",
	"HALLO, ALLES KLAR? HEY, WACHEN SIE AUF. HoeREN SIE MICH? SIND SIE TOT?",
	"NEIN, ICH HEISSE BILLIE JEAN, ABER DU KANNST MICH B.J. NENNEN, DAS IST KueRZER",
	"HI,HI! SUPERGUT!",
	"JA STELL DIR VOR, HEINZ, ICH WOLLTE MICH HIER GERADE HINLEGEN, ALS ICH EINEN LAUTEN KNALL IM FLUR HoeRTE",
	// 5
	"ANFANGS HAB ICH NICHT SO DRAUF GEACHTET, ABER NACHDEM ICH 2 STUNDEN LANG NICHT SCHLAFEN KONNTE, GING ICH RUNTER, UM SPAZIERENZUGEHEN",
	"ICH WAR NATueRLICH BAFF, ALS ICH DICH VOR DER TueR LIEGEN SAH. ICH HABE ECHT GEDACHT, DASS DU TOT BIST, HEHE, WIE BLoeD",
	"ICH WOLLTE GERADE MUND-ZU-MUND BEATMUNG MACHEN, ABER DANN FINGST DU JA AN ZU REDEN",
	"DU FASELTEST WAS VON NER VOGELSCHEUCHE UND ICH HATTE GANZ SCHoeN SCHISS, WEIL SPRECHENDE TOTE SCHON ECHT UNHEIMLICH SIND?",
	"NICHT WAHR? NAJA,ICH HABE DICH DANN AUFGEHOBEN UND IN MEIN ZIMMER GETRAGEN UND AUF MEIN BETT GELEGT.... WEITER NICHTS. HI,HI,HI",
	// 10
	"ACH NEIN, ES WAR NICHT DIE BEULE, HIHI, ICH BIN VERSEHENTLICH AUF DEINE BRILLE GETRETEN",
	"MAN MUSS SICH ECHT MAL ANGUCKEN, WIE GUT IHM DIE BRILLE STEHT. ER IST ZWAR NICHT DER MARTINI-MANN, ABER ER HAT WAS WEISS ICH WAS",
	"JA, JA, ICH WILL. KOMM NIMM MICH UND KueSS MICH...",
	"OH HEINZ, LIEBLING, GUT DASS DU DA BIST. DIESER WueSTLING VON DRASCULA HAT MICH ANS BETT GEFESSELT UND IST DANN RUNTER UM SICH DAS SPIEL ANZUSEHEN",
	"JA, WIRKLICH. BEFREIE MICH",
	// 15
	"NEIN, TUT MIR LEID. ICH HAB ALLE BEIM VERSUCH, AUS DEM VERLIES AUSZUBRECHEN, AUFGEBRAUCHT",
	"HEINZ, BIST DU ES? WIE TOLL! ICH WUSSTE, DASS DU KOMMEN WueRDEST!",
	"DU AHNST JA NICHT, WIE SEHR ICH WEGEN DIESEM UNGEHEUER VON DRASCULA GELITTEN HABE",
	"ZUERST IST ER MIT MIR HIERHER GEFLOGEN UND DANN HAT ER MICH IN DIESES LOCH GESPERRT, OHNE SPIEGEL UND GAR NICHTS",
	"WIE ICHS GESAGT HABE. DAS SCHLIMMSTE IST, DASS ER SICH KEIN EINZIGES MAL ENTSCHULDIGT HAT.",
	// 20
	"HEINZ, GELIEBTER, WO BIST DU?",
	"ICH BIN BEREIT, VON DIR GERETTET ZU WERDEN",
	"WARTE, ICH GUCK MAL...NEIN, SCHATZ, TUT MIR LEID",
	"HIER NIMM...",
	"LIEBER HEINZ!",
	// 25
	"ICH WERDE DICH NIE VERGESSEN, ABER DAS MIT UNS FUNKTIONIERT NICHT. ICH GEBS ZU: ES GIBT EINEN ANDERN. GRoeSSER,STaeRKER...",
	"UND AUSSERDEM HAT ER MICH AUS DRASCULAS KLAUEN BEFREIT UND UM MEINE HAND ANGEHALTEN, UND ICH HABE JA GESAGT",
	"LEB WOHL,HEINZ. VERSUCH NICHT EINE ERKLaeRUNG ZU FINDEN, DENN DIE LIEBE IST BLIND UND KENNT KEINE VERNUNFT",
	"DU BIST MIR HOFFENTLICH NICHT BoeSE. DENK IMMER DARAN DASS ICH DICH IMMER NOCH LIEBE, WENN AUCH NUR ALS FREUND",
},
{
	// 0
	"",
	"VOUS ALLEZ BIEN? ALLEZ, REVEILLEZ-VOUS! VOUS M'ENTENDEZ? VOUS \322TES MORT?",
	"NON, MON NOM EST BILLIE JEAN, MAIS TU PEUX M'APPELLER B.J., C'EST PLUS COURT",
	"HI! HI! -C'EST BON!",
	"EN VRIT JOHNY, J'TAIS L\267, PR\322TE \267 ME COUCHER, ET J'AI ENTENDU UN FORT COUP DANS LE COULOIR",
	// 5
	"AU DBUT JE N'Y AI PAS DONN D'IMPORTANCE, MAIS APR\324S JE ME SUIS RENDUE COMPTE QUE NE POUVAIS PAS M'ENDORMIR ET SUIS SORTIE FAIRE UNE PROMENADE",
	"ET \267 MON GRAN TONNEMENT, QUAND J'AI OUVERT LA PORTE, JE T'AI TROUV L\267, PAR TERRE. J'TAIS SUR LE POINT DE PENSER QUE TU AVAIS MORT, HE!, QUE JE SUIS B\322TE!",
	"J'ALLAIS TE FAIRE LE BOUCHE-\267-BOUCHE MAIS IL N'A T PAS NCESSAIRE PUISQUE TU AS COMMENC \267 PARLER",
	"TU DISAIS \267 SAVOIR QUOI D'UN POUVANTAIL. CELA M'A FAIT UNE GRANDE PEUR, PARCE QUE QUAND UN MORT SE MET \267 PARLER L'IMPRESSION EST TR\324S FORTE, NE CROIS-TU PAS?",
	"C'EST VRAI, NON? ENFIN. JE ME SUIS DBROUILLE POUR TE PRENDRE SUR MOI, T'EMMENER DANS MA CHAMBRE ET T'INSTALLER SUR LE LIT... ET VOIL\267 TOUT. HI! HI! HI!",
	// 10
	"HO! CE N'EST PAS \267 CAUSE DU COUP, HI! HI! CE QUE J'AI MARCHE SUR TES LUNETTES PAR ACCIDENT",
	"\220VIDEMMENT LES LUNETTES LUI FONT DU BIEN! JE SAIS BIEN QUIL N'EST PAS FERNANDO LANCHA, MAIS IL A  UN AIR QU'ALLEZ DONC SAVOIR",
	"OUI, OUI, JE VEUX... VAS-Y, EMBRASSE-MOI FORT, EMBRASSE-MOI BEAUCOUP...",
	"OH,JOHNY!, MON AMOUR! HEUREUSEMENT QUE TU ES VENU. CE M\220CHANT DRASCULA M'A ATTACH\220 AU LIT ET APR\324S IL EST PARTI EN BAS VOIR LE MATCH",
	"OUI, C'EST VRAI, LIB\324RE-MOI",
	// 15
	"NON, SUIS D\220SOL\220E. JE LES AI TOUTES UTILIS\220ES, DANS LE CACHOT, EN T\266CHANT DE ME LIB\220RER ALORS QUE TOI TU M'ABANDONNAIS",
	"JOHNY, C'EST TOI? -QUELLE JOIE! -JE SAVAIS QUE TU VIENDRAIS!",
	"TU NE PEUX PAS SAVOIR COMBIEN M'A FAIT SOUFFRIR CE M\220CHANT DRASCULA",
	"D'ABORD IL M'EMMEN\220 EN VOLANT JUSQU'ICI ET APR\324S IL M'ENFERM\220E DANS CE TAUDIS, SANS MIROIR NI RIEN D'AUTRE",
	"COMME TU L'ENTENDS. ET LE PIRE C'EST QUIL NE M'A PAS DEMAND\220 PARDON N'UNE SEULE FOIS ",
	// 20
	"JOHNY, MON CH\220RI, O\353 EST-CE QUE TU ES?",
	"JE SUIS PR\322TE POUR QUE TU M'EN SORTES D'ICI",
	"ATTEND, JE VAIS REGARDER... NON CH\220RI, JE REGRETTE",
	"TIENS...",
	"\"CHER JOHNY",
	// 25
	"JAMAIS JE T'OUBLIERAI, MAIS JE ME SUIS APER\200UE QUE NOTRE AFFAIRE NE POUVAIT PAS MARCHER. JE VAIS \322TRE SINC\324RE AVEC TOI: IL Y A UN AUTRE HOMME, PLUS GRAND...",
	"ET ENCORE IL M'A LIB\220R\220 DES MAINS DE DRASCULA. IL M'A DEMAND\220 EN MARIAGE, ET MOI J'AI ACCEPT\220",
	"AU REVOIR, JOHNY. N'Y CHERCHES PAS UNE EXPLICATION, L'AMOUR EST AVEUGLE ET N'\220COUTE PAS DES RAISONS",
	"J'ESP\324RE QUE TU SERAS SANS RANCUNE, ET N'OUBLIES PAS QUE JE T'AIME ENCORE, MAIS EN AMI SEULEMENT\"",
},
{
	// 0
	"",
	"TI SENTI BENE? ANDIAMO, SVEGLIATI! RIESCI A SENTIRMI? MA SEI MORTO?",
	"NO, MI CHIAMO BILLIE JEAN, MA PUOI CHIAMARMI B.J., \324 PI\353 CORTO.",
	"HI, HI! QUESTA ERA BUONA!",
	"BEH, JOHNNY. VEDI, ME NE STAVO QUA, GI\267 PRONTA PER ANDARE A LETTO, QUANDO HO SENTITO UN FORTE RUMORE IN CORRIDOIO",
	// 5
	"ALL'INIZIO NON CI HO FATTO CASO, MA DOPO CIRCA DUE ORE NON RIUSCIVO ANCORA A PRENDERE SONNO E ME NE SONO ANDATA A FARE DUE PASSI",
	"E IMMAGINA LA MIA SORPRESA QUANDO HO APERTO LA PORTA E TI HO VISTO DISTESO A TERRA. TI GIURO, HO PENSATO CHE FOSSI MORTO... AH, AH, CHE SCIOCCA",
	"VOLEVO FARTI LA RESPIRAZIONE BOCCA A BOCCA MA NON \324 SERVITO PERCH\220 HAI INIZIATO A PARLARE",
	"HAI DETTO QUALCOSA A PROPOSITO DI UNO SPAVENTAPASSERI. MI HAI FATTO VENIRE UN COLPO. SAI, \324 ABBASTANZA SCIOCCANTE VEDERE UN MORTO CHE PARLA",
	"VERO? NON SO COME, MA SONO RIUSCITA A PORTARTI IN CAMERA MIA, TI HO MESSO SUL LETTO E... QUESTO \324 TUTTO... AH, AH, AH.",
	// 10
	"NO, NON \324 STATA LA BOTTA, HI, HI. \324 CHE PER SBAGLIO HO PESTATO I TUOI OCCHIALI",
	"MAMMA MIA COM'\324 BELLO CON GLI OCCHIALI! SO CHE NON \324 ANTONIO BANDERAS, MA HA QUALCOSA CHE MI PIACE MOLTISSIMO",
	"S\326, S\326, LO VOGLIO... ABBRACCIAMI FORTE, BACIAMI...",
	"OH JOHNNY, CARO, MENO MALE CHE SEI QUI. DRASCULA, QUEL MALEDETTO, MI HA LEGATA AL LETTO E POI SE NE \324 ANDATO A GUARDARE LA PARTITA",
	"S\326, \324 VERO, LIBERAMI",
	// 15
	"NO, MI DISPIACE. LE HO USATE TUTTE NELLA CELLA CERCANDO DI LIBERARMI QUANDO MI HAI ABBANDONATA",
	"JOHNNY, SEI TU? OH, GRAZIE AL CIELO, SAPEVO CHE SARESTI VENUTO!",
	"NON IMMAGINI NEMMENO QUANTO QUEL MALVAGIO DI DRASCULA MI ABBIA FATTO SOFFRIRE",
	"PRIMA MI HA PORTATO FIN QUA VOLANDO E POI MI HA RINCHIUSA IN QUESTA CAMERACCIA CHE NON HA NEANCHE UNO SPECCHIO",
	"\324 QUELLO CHE TI STO DICENDO! E IL PEGGIO \324 CHE NON SI \324 MAI SCUSATO, NEMMENO UNA VOLTA",
	// 20
	"JOHNNY, CARO. DOVE SEI?",
	"SONO PRONTA, FAMMI USCIRE DA QUI",
	"ASPETTA CHE CONTROLLO... NO, CARO, MI DISPIACE",
	"PRENDI...",
	"\"CARO JOHNNY",
	// 25
	"NON POTR\343 MAI DIMENTICARTI, MA NON SAREMO MAI FELICI INSIEME. SAR\343 SINCERA CON TE: C'\324 UN ALTRO; PI\353 ALTO, PI\353 FORTE...",
	"MI HA LIBERATO DA DRASCULA. MI HA CHIESTO LA MANO E HO DETTO DI S\326",
	"ADDIO JOHNNY. NON CERCARE UNA SPIEGAZIONE, PERCH\220 L'AMORE \324 CIECO.",
	"SPERO CHE NON MI ODIERAI, E RICORDA CHE TI VOGLIO BENE, SEBBENE TU PER ME SIA SOLTANTO UN AMICO\"",
},
};


const char *_texte[NUM_LANGS][NUM_TEXTE] = {
{
	// 0
	"",
	"YOU... HEY YOU!",
	"DON'T GIVE ME THAT CRAP ABOUT A DEAD BODY OK?",
	"I'M ALIVE. IT'S JUST THAT I'M STARVING",
	"WELL, YOU SEE. I WAS THE DRUNKARD OF THE VILLAGE, JUST KEEPING UP WITH THE FAMILY TRADITION, YOU KNOW? ONE NIGHT DRASCULA KIDNAPPED ME TO TAKE MY ORGANS AWAY",
	// 5
	"SINCE ALCOHOL STILL KEEPS ME QUITE YOUNG, I'M HERE LIKE A SCRAP YARD. EVERYTIME HE NEEDS SOMETHING FOR THE MONSTER HE IS MAKING, HE COMES AND TAKES IT FROM ME",
	"IT HURT AT FIRST, BUT I DON'T CARE ANYMORE",
	"I DON'T KNOW. I GUESS IT'S HIS GRADUATE PROJECT",
	"I'M DESIDERIO, AND I CAN HELP YOU WITH ANYTHING YOU NEED",
	"THE TRUTH IS THAT I DON'T REALLY FEEL LIKE IT, BUT THANKS VERY MUCH ANYWAY SIR",
	// 10
	"TAKE IT, IT'S YOURS",
	"WHY DO ALL ADVENTURE GAMES END WITH A SUNRISE OR A SUNSET?",
	"DO ALL THESE NAMES BELONG TO THE CREATORS OF THE GAME?",
	"AREN'T THEY ASHAMED TO BE SEEN BY EVERYBODY?",
	"JESUS, THAT EMILIO DE PAZ IS EVERYWHERE!!",
	// 15
	"REALLY?",
	"YES",
	"WELL, DON'T MAKE A FUSS ABOUT IT",
	"HEY WEREWOLF, BY THE WAY...",
	"DIDN'T YOU FALL OFF A WINDOW AND GET BADLY HURT",
	// 20
	"AT LEAST IT WASN'T ALWAYS THE SAME ONES...",
	"HE'S BEEN OUT FOUR TIMES ALREADY",
	"I'D LIKE TO BE A MODEL",
	"ALL RIGHT, AND WHAT ARE YOU GOING TO DO?",
},
{
	// 0
	"",
	"EO, USTED",
	"\255QUE MUERTO NI OCHO CUARTOS!",
	"ESTOY VIVO, LO QUE PASA ES QUE YO PASO HAMBRE, \250SABE USTED? ",
	"PUES VERA, SE\245OR. YO ERA EL BORRACHO DEL PUEBLO, DIGNO SUCESOR DE UNA FAMILIA DE ILUSTRES BORRACHUZOS, Y UNA NOCHE DRASCULA ME SECUESTRO PARA ROBARME LOS ORGANOS",
	// 5
	"RESULTA QUE COMO ME CONSERVO EN ALCOHOL, ME TIENE AQUI EN PLAN DESSGUACE. CADA VEZ QUE NECESITA ALGO PARA EL MONSTRUO QUE ESTA CREANDO VIENE Y ME LO COGE",
	"AL PRINCIPIO ME DOLIA, NO CREA, PERO YA ME DA IGUAL",
	"NO SE, SERA SU PROYECTO DE FIN DE CARRERA",
	"MI NOMBRE ES DESIDERIO, PARA SERVIRLE A USTED",
	"PUES LA VERDAD ES QUE NO TENGO MUCHA GANA, PERO MUCHAS GRACIAS DE TODAS MANERAS, SE\245OR",
	// 10
	"SI, TU MISMO",
	"\250POR QUE TODOS LOS JUEGOS DE AVENTURAS TERMINAN CON UN AMANECER O UNA PUESTA DE SOL? ",
	"\250Y TODOS ESTOS NOMBRES SON DE LOS QUE HAN HECHO EL JUEGO?",
	"\250Y NO LES DA VERG\232ENZA SALIR Y QUE LOS VEA TODO EL MUNDO?",
	"JOLIN, NO HACE MAS QUE SALIR \"EMILIO DE PAZ\" ",
	// 15
	"\250DE VERDAD?",
	"SI",
	"PUES NO ES PARA TANTO",
	"POR CIERTO HOMBRE LOBO...",
	"... \250NO CAISTE POR UNA VENTANA Y TE HICISTE POLVO?",
	// 20
	"SI AL MENOS NO FUESEN SIEMPRE LOS MISMOS",
	"ESE YA HA SALIDO CUATRO VECES",
	"ME GUSTARIA SER MODELO",
	"YA, \250Y TU QUE VAS A HACER?",
},
{
	// 0
	"",
	"EJ, SIE DAA",
	"DU BIST WOHL BESOFFEN ODER WAS?!",
	"ICH BIN LEBENDIG, ABER ICH HABE HUNGER, WISSEN SIE?",
	"TJA, ICH WAR DER DORFSaeUFER, STOLZER ABKoeMMLING EINER DORFSaeUFERSIPPE UND EINES NACHTS HOLTE MICH DRASCULA, UM MIR DIE ORGANE ZU KLAUEN",
	// 5
	"WEIL ICH MICH IN ALKOHOL gut HALTE BIN ICH SEIN ERSATZTEILLAGER. IMMER WENN ER WAS FueR SEIN MONSTERCHEN BRAUCHT, BEDIENT ER SICH BEI MIR",
	"AM ANFANG TAT DAS HoeLLISCH WEH, ABER JETZT MERK ICH SCHON NICHTS MEHR",
	"KEINE AHNUNG, ES WIRD WOHL SEINE DIPLOMARBEIT SEIN",
	"ICH HEISSE DESIDERIUS UND STEHE ZU IHREN DIENSTEN",
	"NAJA, EIGENTLICH HABE ICH KEINE GROSSE LUST ABER VIELEN DANK JEDENFALLS, MEIN HERR",
	// 10
	"JA, NUR ZU",
	"WARUM HoeREN ALLE SPIELE MIT SONNENAUFGANG ODER SONNENUNTERGANG AUF?",
	"UND SIE HABEN DIE NAMEN VON DEN MACHERN DIESES SPIELS BENUTZT?",
	"UND SIE SCHaeMEN SICH NICHT SICH ALLER WELT ZU OFFENBAREN?",
	"SCHEIBENKLEISTER. SCHON WIEDER DER PROGRAMMIERER",
	// 15
	"WIRKLICH?",
	"JA",
	"SO SCHLIMM IST ES AUCH WIEDER NICHT",
	"APROPOS WERWOLF....",
	"....BIST DU NICHT AUS EINEM FENSTER GEFALLEN UND ZU STAUB GEWORDEN?",
	// 20
	"WENN ES WENIGSTENS NICHT IMMER DIESELBEN WaeREN",
	"DER IST SCHON 4 MAL AUFGETAUCHT",
	"ICH WaeRE GERN FOTOMODELL",
	"KLAR, UND WAS WILLST DU SPaeTER MAL MACHEN?",
},
{
	// 0
	"",
	"OH\220! VOUS!",
	"QUEL MORT NI QUELLE HISTOIRE!",
	"JE SUIS VIVANT. C'EST QUE J'AI FAIM, VOUS SAVEZ? ",
	"J'TAIS L'IVROGNE DU VILLAGE, LE DIGNE REPRSENTANT D'UN FAMILLE D'ILLUSTRES SO\352LARDS, ET DRASCULA M'A SQUESTR UNE NUIT POUR ME VOLER LES ORGANES",
	// 5
	"COMME JE ME CONSERVE EN ALCOOL, IL ME TIENS ICI EN PLAN DBALLAGE. \267 CHAQUE FOIS QU'IL A BESOIN DE QUELQUE CHOSE POUR SON MONSTRE, IL LE PRENDRE",
	"AU DBUT A ME FAISAIT MAL, MAIS JE M'EN FICHE DJ\267",
	"JE NE SAIS PAS, MAIS CE SERA SON PROJET FIN D'TUDES",
	"MON NOM EST DSIR, POUR VOUS SERVIR",
	"VRAIMENT JE N'AI PAS UNE ENVIE FOLLE, MAIS MERCI QUAND M\322ME, MONSIEUR",
	// 10
	"OUI, TOI M\322ME",
	"POURQUOI TOUS LES JEUX D'AVENTURES FINISSENT AVEC UNE AUBE OU UN COUCHER DU SOLEIL? ",
	"ET VOIL\267 LES NOMS DE TOUS QUI ON FAIT LE JEU?",
	"ET N'ONT-ILS PAS LA HONTE DE SE MONTRER ET QUE TOUT LE MONDE LES VOIT?",
	"AH ZUT! IL NE FAIT QUE SORTIR \"EMILIO DE PAZ\" ",
	// 15
	"C'EST VRAI",
	"OUI",
	"EH BIEN, IL NE FAUT PAS EXAG\220RER",
	"EN FAIT HOMME-LOUP...",
	"...N'EST-TU PAS TOMB\220 D'UNE FEN\322TRE EN TE BR\220SILLANT?",
	// 20
	"SI DU MOINS N'\220TAIENT PAS TOUJOURS LES M\322MES...",
	"CELUI-L\267 EST D\220J\267 SORTI QUATRE FOIS",
	"J'AIMERAIS \322TRE MANNEQUIN",
	"PARFAITEMENT. ET TOI, QUE VAS-TU FAIRE?",
},
{
	// 0
	"",
	"EHI, LEI!",
	"MA CHE MORTO E MORTO!",
	"SONO VIVO. STO SOLO MORENDO DI FAME",
	"IO ERO L'UBRIACONE DEL PAESE, DEGNO SUCCESSORE DI UNA FAMIGLIA DI ILLUSTRI UBRIACONI, E UNA NOTTE DRASCULA MI RAP\326 PER RUBARMI GLI ORGANI",
	// 5
	"SICCOME L'ALCOL MI MANTIENE IN VITA, MI TIENE QUI COME UNA SCORTA. OGNI VOLTA CHE HA BISOGNO DI QUALCOSA PER IL MOSTRO CHE STA CREANDO, VIENE DA ME E SE LO PRENDE",
	"ALL'INIZIO FACEVA MALE, MA ORA NON SENTO PI\353 NULLA",
	"NON SO, MAGARI SAR\267 LA SUA TESI DI LAUREA",
	"IL MIO NOME \324 DESIDERIO, PER SERVIRVI",
	"LA VERIT\267 \324 CHE NON HO MOLTA VOGLIA, COMUNQUE GRAZIE MILLE, SIGNORE",
	// 10
	"PROPRIO TU!",
	"PERCH\220 TUTTI I GIOCHI D'AVVENTURA FINISCONO SEMPRE CON UN'ALBA O UN TRAMONTO?",
	"E TUTTI QUESTI NOMI SONO DI CHI HA FATTO IL GIOCO?",
	"E NON SI VERGOGNANO A FARSI VEDERE DA TUTTI?",
	"CAVOLI, QUELL'EMILIO DE PAZ \324 DAPPERTUTTO!!",
	// 15
	"DAVVERO?",
	"S\326",
	"BEH, NON NE FARE UNA QUESTIONE",
	"CERTO, LUPO MANNARO...",
	"... MA NON TI SEI FATTO MALE CADENDO DA UNA FINESTRA?",
	// 20
	"SE ALMENO NON FOSSERO SEMPRE GLI STESSI",
	"QUELLO \324 GIA USCITO QUATTRO VOLTE",
	"MI PIACEREBBE ESSERE UN MODELLO",
	"S\326, E TU COSA FARAI?",
},
};


const char *_texti[NUM_LANGS][NUM_TEXTI] = {
{
	// 0
	"",
	"MASTER, THIS IS NOT WORKING",
	"I AM POSITIVE, MASTER",
	"I'M SORRY MASTER",
	"ARE YOU GOING TO BRING ANOTHER CRAZY SCIENTIST HERE? THE LABORATORY IS ALREADY FULL UP, AND BESIDES, THEY'RE ALL OUT OF DATE",
	// 5
	"HUSH MASTER, THE FEMINISTS MIGHT HEAR YOU",
	"DAMN IT!",
	"I DIDN'T EXPECT YOU SO SOON, MASTER",
	"QUITE BAD MASTER. THERE ARE SOME PROBLEMS WITH THE SATELLITE, I JUST CAN'T RECEIVE ANYTHING. THERE MUST BE SOME INTERFERENCE FROM THE STORM",
	"WHAT DO I KNOW, MASTER?",
	// 10
	"YES, MY MASTER",
	"MASTER",
	"DO YOU KNOW WHAT TIME IS IT?",
	"WHAT? OH, THAT SCARED ME!. YOU ARE THE \"NIGHT-CLEANING GUY\", RIGHT?",
	"I'M IGOR, THE VALET. YOU CAN START WITH THE BALL ROOM. THERE'S BEEN A SUPER NATURAL ORGY IN THERE YESTERDAY AND IT LOOKS LIKE SHIT",
	// 15
	"IF YOU NEED ANYTHING, JUST BUY IT",
	"IT'S THE TAX RETURN APPLICATION FORM, CAN'T YOU SEE?",
	"NEITHER DO I. THE NUMBERS ARE VERY SMALL AND I CAN'T SEE MUCH AT THIS DISTANCE.",
	"NO WAY! THEY MAKE ME LOOK UGLY",
	"OH, WELL. IT'S JUST LIKE A CRAZY PARTY THAT THE MASTER ORGANIZES WITH HIS FRIENDS EACH TIME SOME IDIOT COMES ALONG TRYING TO KILL HIM",
	// 20
	"THEY TAKE HIS EYES OUT. THEN, POUR IN SOME LEMON JUICE SO THAT IT ITCHES TO DEATH, AND THEN...",
	"NO",
	"WHAT DO YOU MEAN WHY NOT? DO YOU KNOW WHAT TIME IT IS?",
	"YES, IT'S WINTER",
	"SEE YOU LATER",
	// 25
	"DON'T EVEN THINK ABOUT IT!",
	"WELL, THAT'S ENOUGH FOR TODAY. I'M GOING TO HAVE SUPPER",
	"MAN, I ALWAYS FORGET TO LOCK IT, RIGHT?",
	"THE HELL WITH IT!",
	"WHAT? OH, YOU SCARED ME MASTER, I THOUGHT YOU WERE ASLEEP",
	// 30
	"OH, BY THE WAY, I TOOK THE LIVING-ROOM KEYS SO THAT YOU CAN WATCH THE EARLY MORNING CARTOONS WITHOUT WAKING ME UP",
	"YOU'VE GOT ANOTHER COLD MASTER? DAMN IT! I TOLD YOU TO GET SOME HEATING IN HERE...",
	"ALL RIGHT, JUST TAKE YOUR ASPIRIN AND GO TO BED TO SWEAT FOR A WHILE. GOOD NIGHT",
},
{
	// 0
	"",
	"MAESTRO, CREO QUE ESTO NO RULA",
	"ESTOY SEGURISIMO, MAESTRO...",
	"LO SIENTO, MAESTRO",
	"\250VA A TRAER OTRO DE CIENTIFICO LOCO? LE ADVIERTO QUE TENEMOS EL LABORATORIO LLENO Y ESTAN TODOS CADUCADOS",
	// 5
	"CALLE, MAESTRO, QUE COMO LE OIGAN LAS FEMINISTAS",
	"HAY QUE JOROBARSE",
	"\255MAESTRO! \255NO LE ESPERABA TAN PRONTO!",
	"MAL MAESTRO, DEBE HABER PROBLEMAS CON EL SATELITE Y NO CONSIGO SINTONIZAR LA IMAGEN. ADEMAS LA TORMENTA PRODUCE INTERFERENCIAS",
	"\250Y A MI QUE ME CUENTA, MAESTRO?",
	// 10
	"SI, MAESTRO",
	"MAESTRO",
	"\250SABE LA HORA QUE ES?",
	"\250EH? \255AH, QUE SUSTO ME HAS DADO! TU ERES EL DE \"LIMPIEZA NOCTURNA\"\250NO?",
	"YO SOY IGOR, EL MAYORDOMO. PUEDES EMPEZAR POR EL SALON DE BAILE. AYER HUBO ORGIA SOBRENATURAL Y ESTA HECHO UNA MIERDA",
	// 15
	"SI NECESITAS ALGO, LO COMPRAS",
	"LA DECLARACION DE LA RENTA \250ES QUE NO LO VES?",
	"PUES YO TAMPOCO, PORQUE ENTRE QUE LOS NUMEROS SON MUY CHICOS, Y YO QUE NO VEO BIEN DE LEJOS...",
	"\255NI HABLAR! ME HACEN PARECER FEO",
	"BAH, ES UN FESTORRO QUE SE MONTA EL MAESTRO CON SUS COLEGAS CADA VEZ QUE LLEGA ALGUN IMBECIL QUERIENDO ACABAR CON EL",
	// 20
	"PRIMERO LE SACAN LOS OJOS; LUEGO LE ECHAN ZUMO DE LIMON PARA QUE LE ESCUEZA; DESPUES...",
	"NO",
	"\250COMO QUE POR QUE NO? \250TU HAS VISTO LA HORA QUE ES?",
	"EN INVIERNO, SI",
	"HASTA LUEGO",
	// 25
	"\255NI SE TE OCURRA!",
	"BUENO, POR HOY YA VALE. ME VOY A CENAR",
	"\255Y QUE SIEMPRE SE ME OLVIDA CERRAR CON LLAVE!",
	"\255HAY QUE JOROBARSE!",
	"\250EH? \255AH! QUE SUSTO ME HA DADO, MAESTRO. CREIA QUE ESTABA DURMIENDO",
	// 30
	"AH, POR CIERTO, TOME LAS LLAVES DE LA SALA DE ESTAR Y ASI NO ME MOLESTA MA\245ANA TEMPRANO SI QUIERE VER LOS DIBUJOS",
	"\250QUE, YA SE HA RESFRIADO OTRA VEZ, MAESTRO? HAY QUE JOROBARSE. SI SE LO TENGO DICHO, QUE PONGA CALEFACCION... ",
	"BUENO, TOMESE UNA PASTILLA DE ACIDOACETIL SALICILICO Y HALA, A SUDAR. BUENAS NOCHES",
},
{
	// 0
	"",
	"MEISTER, ICH GLAUBE, DAS KLAPPT NICHT",
	"ICH BIN ABSOLUT SICHER, MEISTER....",
	"TUT MIR LEID, MEISTER",
	"NOCH SO EINEN VERRueCKTEN WISSENSCHAFTLER? ABER DAS LABOR IST DOCH VOLL MIT IHNEN UND ALLE SIND ueBER DEM VERFALLSDATUM",
	// 5
	"SEIEN SIE STILL MEISTER. WENN DAS DIE FEMINISTINNEN HoeREN",
	"GRueNSPAN UND ENTENDRECK",
	"MEISTER! ICH HABE SIE NOCH GAR NICHT ERWARTET!",
	"SCHLECHT, MEISTER. ES GIBT WOHL PROBLEME MIT DER SCHueSSEL UND ICH BEKOMME DAS BILD EINFACH NICHT KLAR. UND DANN DAS GEWITTER!",
	"UND WIESO ERZaeHLEN SIE MIR DAS, MEISTER?",
	// 10
	"JA, MEISTER",
	"MEISTER",
	"WISSEN SIE, WIEVIEL UHR ES IST?",
	"Hae? OH, DU HAST MICH GANZ SCHoeN ERSCHRECKT. BIST DU VON \"BLITZ UND BLANK BEI MONDESSCHEIN\"",
	"ICH BIN IGOR, DER HAUSVERWALTER. DU KANNST MIT DEM WOHNZIMMER ANFANGEN. GESTERN HATTEN WIR EINE ueBERNATueRLICHE ORGIE UND ES SIEHT AUS WIE IM SAUSTALL",
	// 15
	"KAUF ALLES WAS DU BRAUCHST!",
	"DIE STEUERERKLaeRUNG. SIEHST DU DAS NICHT?",
	"ICH AUCH NICHT, WEIL DIE ZAHLEN SCHON MAL SEHR KLEIN SIND UND ICH DOCH KURZSICHTIG BIN....",
	"VON WEGEN! SIE MACHEN MICH HaeSSLICH",
	"PAH, DAS IST EINE RIESENFETE DIE DER MEISTER JEDESMAL DANN ABZIEHT, WENN IRGENDSOEIN IDIOT KOMMT, DER IHN ERLEDIGEN WILL",
	// 20
	"ZUERST REISSEN SIE IHM DIE AUGEN AUS DANACH GIESSEN SIE ZITRONENSAFT DRueBER DAMIT ES SCHoeN BRENNT UND DANN...",
	"NEIN",
	"WIE WARUM NICHT? WEISST DU WIEVIEL UHR ES IST?",
	"IM WINTER JA",
	"TSCHueSS",
	// 25
	"WAG ES BLOOOSS NICHT!",
	"GUT, FueR HEUTE REICHTS, DAS ABENDESSEN RUFT",
	"IMMER VERGESSE ICH ZUZUSCHLIESSEN!",
	"VERDAMMTER MIST NOCHMAL!",
	"HaeH? AH! SIE HABEN MICH GANZ SCHoeN ERSCHRECKT, MEISTER. ICH DACHTE SIE SCHLAFEN",
	// 30
	"ACH,ueBRIGENS, ICH HABE DIE WOHNZIMMERSCHLueSSEL GENOMMEN. SO NERVEN SIE MICH MORGEN FRueH NICHT MIT IHREN ZEICHENTRICKSERIEN",
	"ACH, SIE HABEN SICH SCHON WIEDER ERKaeLTET, MEISTER? ICH HAB JA IMMER SCHON GESAGT, SIE SOLLEN DIE HEIZUNG ANMACHEN...",
	"NAJA, SCHLUCKEN SIE EINE SALICILACETYLSaeURE-TABLETTE UND AB INS BETT ZUM SCHWITZEN. GUTE NACHT",
},
{
	// 0
	"",
	"MA\327TRE, JE CROIS QUE \200A NE ROULE PAS",
	"J'EN SUIS TR\324S S\352R, MA\327TRE...",
	"JE LE REGRETTE, MA\327TRE",
	"IL APPORTERA UN AUTRE DE SCIENTIFIQUE FOU? JE VOUS PRVIENS QUE LE LABORATOIRE EN EST PLEIN ET TOUS SONT PRIMS",
	// 5
	"TAISEZ-VOUS, MA\327TRE, SI LES FMINISTES VOUS COUTENT...",
	"CE QU'IL FAUT SUPPORTER!",
	"MA\327TRE! -JE NE VOUS ATTENDAIT PAS SI T\342T!",
	"A VA MAL MA\327TRE. IL DOIT AVOIR DES PROBL\324MES AVEC LE SATELLITE ET JE NE RUSSIT PAS \267 SINTONISER L'IMAGE. ET ENCORE L'ORAGE PRODUIT DES INTERFRENCES.",
	"CANCANS QUE TOUT CELA, MA\327TRE!",
	// 10
	"OUI, MA\327TRE",
	"MA\327TRE",
	"QUELLE HEURE IL EST?",
	"H\220? -AH! TU M'AS FAIT PEUR! TU EST CELUI DU \"NETTOYAGE DE NUIT\", NON?",
	"JE SUIS IGOR, LE MAJORDOME. TU PEUX COMMENCER PAR LE SALON DE BAL. HIER IL Y A EU UNE ORGIE SURNATURELLE ET C'EST UNE SALOPERIE",
	// 15
	"SI TU AS BESOIN DE QUELQUE CHOSE, ACHETE-EN!",
	"LA D\220CLARATION D'IMP\342TS, NE LE VOIS-TU PAS?",
	"EH BIEN, MOI NON PLUS, CAR \267 CAUSE DE SI PETITS NUM\220ROS ET MA DIFFICULT\220 POUR BIEN VOIR DE LOIN...",
	"ON N'EN PARLE PLUS! ILS ME FONT PARA\327TRE LAID",
	"BAH! C'EST UNE BELLE F\322TE QUE LE MA\327TRE ORGANISE AVEC SES COLL\324GUES \267 CHAQUE FOIS QU'UN IMB\220CILE ARRIVE ET VEUT FINIR AVEC LUI",
	// 20
	"D'ABORD ON LUI ARRACHE LES YEUX; APR\324S, ON LUI VERSE DE JUS DE CITRON POUR QUE \200A LUI CUISE; APR\324S...",
	"NON",
	"POURQUOI PAS? TU AS VU L'HEURE QU'IL EST?",
	"EN HIVER,  OUI",
	"AU REVOIR",
	// 25
	"N'Y PENSES M\322ME PAS!",
	"BON, \200A VA POUR AUJOURD'HUI. JE VAIS D\327NER",
	"J'OUBLIE TOUJOURS FERMER \267 CL\220, H\220LAS!",
	"QUEL ENNUI!",
	"H\220? -VOUS M'AVEZ FAIT PEUR, MA\327TRE. JE PENSAIS QUE VOUS DORMIEZ",
	// 30
	"H\220, MA\327TRE! PRENEZ LES CL\220S DE LA SALLE DE S\220JOUR, AINSI DONC SI VOUS VOULEZ VOIR  LES DESSINS ANIM\220S DEMAIN DE BONNE HEURE NE ME D\220RANGEREZ PAS",
	"QUOI? VOUS VOUS \322TES ENRHUM\220 ENCORE UNE FOIS, MA\327TRE? QUELLE CONTRARI\220T\220! JE VOUS AI D\220J\267 DIT D'Y METTRE LE CHAUFFAGE... ",
	"BON, AVALEZ UN COMPRIM\220 D'ACIDEAC\220TYL SALICYLIQUE ET ALLEZ TRANSPIRER! BONNE NUIT!",
},
{
	// 0
	"",
	"PADRONE, CREDO NON STIA FUNZIONANDO",
	"SICURISSIMO, PADRONE",
	"MI DISPIACE, PADRONE",
	"HA INTENZIONE DI ATTIRARE ALTRI SCIENZIATI PAZZI? IL LABORATORIO \324 GI\267 PIENO, E TRA L'ALTRO SONO TUTTI SCADUTI.",
	// 5
	"ZITTO, PADRONE, LE FEMMINISTE POTREBBERO SENTIRLA",
	"DANNAZIONE!",
	"PADRONE! NON LA ASPETTAVO COS\326 PRESTO!",
	"MALE, PADRONE; DEV'ESSERCI UN PROBLEMA CON IL SATELLITE E NON RIESCO A SINTONIZZARE L'IMMAGINE. SEMBRA CHE LA TEMPESTA CAUSI INTERFERENZE",
	"E A ME COSA IMPORTA, PADRONE?",
	// 10
	"S\326, MIO PADRONE",
	"PADRONE",
	"SA CHE ORE SONO?",
	"COSA? AH, MI HAI SPAVENTATO! TU SEI IL \"RAGAZZO DELLE PULIZIE NOTTURNE\" GIUSTO?",
	"SONO IGOR, IL MAGGIORDOMO. PUOI COMINCIARE DALLA SALA DA BALLO. IERI \324 STATA TEATRO DI UN'ORGIA SOPRANNATURALE ED OGGI \324 RIDOTTA AD UNO SCHIFO.",
	// 15
	"SE HAI BISOGNO DI QUALCOSA, COMPRALA",
	"\324 LA DICHIARAZIONE DEI REDDITI, NON VEDI?",
	"NEANCH'IO. I NUMERI SONO PICCOLISSIMI E NON CI VEDO MOLTO BENE DA LONTANO",
	"NEANCHE PER SOGNO! MI FANNO SEMBRARE BRUTTO",
	"\324 UNA FESTA CHE ORGANIZZA IL PADRONE CON I SUOI AMICI OGNI VOLTA CHE ARRIVA QUALCHE IMBECILLE CHE VUOL CERCARE DI UCCIDERLO",
	// 20
	"PER PRIMA COSA GLI CAVANO GLI OCCHI. POI GLI VERSANO DEL SUCCO DI LIMONE IN MODO CHE BRUCI, POI...",
	"NO",
	"COME PERCH\220 NO? HAI VISTO CHE ORE SONO?",
	"IN INVERNO, S\326",
	"ARRIVEDERCI",
	// 25
	"NON PENSARCI NEMMENO!",
	"BENE, BASTA PER OGGI. VADO A CENA",
	"DIMENTICO SEMPRE DI CHIUDERE A CHIAVE",
	"ACCIDENTI!",
	"COSA? PADRONE! MI HA SPAVENTATO! PENSAVO STESSE DORMENDO",
	// 30
	"OH, A PROPOSITO, HO PRESO LE CHIAVI DEL SOGGIORNO, COS\326 POTR\267 VEDERE I CARTONI ANIMATI DEL MATTINO SENZA SVEGLIARMI",
	"HA DI NUOVO PRESO IL RAFFREDDORE, PADRONE? DANNAZIONE. SONO ANNI CHE LE DICO DI METTERE IL RISCALDAMENTO",
	"BENE, PRENDA UN'ASPIRINA E VADA A LETTO A SUDARE. BUONA NOTTE",
},
};


const char *_textl[NUM_LANGS][NUM_TEXTL] = {
{
	// 0
	"",
	"ARE YOU GOING TO LET YOURSELF BE GUIDED BY PRIMITIVE INSTINCTS JUST BECAUSE WE BELONG TO DIFFERENT RACES AND THE SOCIAL SITUATION IS TELLING US TO DO SO?",
	"AREN'T WE TIED BY SENSE? WHICH IS OUR MOST POWERFUL WEAPON AS WELL AS OUR MOST PRECIOUS GIFT?",
	"OH, IF WE ALL LET OUR THOUGHTS GUIDE OUR WAY IN LIFE WITHOUT LEAVING SOME ROOM FOR FEELINGS WHICH LET OUR PRE-EVOLUTIVE INSTINCTS COME OUT!",
	"ANSWER ME, EPHEMERAL CREATURE. WOULDN'T WE ALL BE HAPPIER WITHOUT THOSE EMOTIONAL BINDINGS?",
	// 5
	"YOU ARE NOT GETTING THROUGH",
	"THIS IS A VERY CLEAR EXAMPLE, YOU SEE?: YOU WANT TO GET THROUGH AND GO AHEAD WITH YOUR ADVENTURE, AND I WON'T LET YOU DO THAT",
	"WILL THIS BE A CONTROVERSIAL POINT BETWEEN US AFTER WE HAVE ONLY JUST MET?",
	"WELL THEN",
	"WELL, THAT DEPENDS ON WHAT WE HAVE A RELATIONSHIP FOR. SOME AUTHORS DEFEND...",
	// 10
	"YUCK! HUNTING AS A WAY TO SURVIVE IS AN INCOMPREHENSIBLE ARCHAIC THING FOR A SUPERIOR BEING LIKE ME. BESIDES, I'VE BECOME A VEGETARIAN",
	"IT JUST SO HAPPENS THAT I WAS ACTUALLY EATING A GUY AND I STARTED TO THINK ABOUT THE ABOVE MENTIONED THOUGHT",
	"IT TOOK ME A LONG TIME TO QUIT OLD HABITS BUT AT LEAST MY IRASCIBLE SOUL SWALLOWED UP THE CONCUPISCIBLE ONE, AND EVER SINCE, I'VE NEVER EATEN MEAT AGAIN",
	"NOT EVEN THE PLEASURE OF SUCKING ON THE BONE, THE TASTE OF THE SKIN AND THAT SWEET TASTE OF MARROW...THAT JUST TAKES YOU TO HEAVENLY PLACES",
	"IT DOESN'T REALLY GET TO ME AT ALL",
	// 15
	"WHAT?",
	"I DON'T KNOW WHAT YOU'RE TALKING ABOUT, YOU EPHEMERAL CREATURE",
	"I'M NOT INTERESTED",
	"I DON'T KNOW ABOUT THE OTHER GAMES, BUT WE COULD USE THIS BEAUTIFUL SCREEN",
	"",
	// 20
	"I'D CARE...",
	"NO. IT'S JUST THE SON, THE FATHER, THE GRANDFATHER AND A FRIEND, WHO ARE CALLED LIKE THAT",
	"BUT, IT IS GOING TO LOOK LIKE THE GAME WAS MADE BY FIVE PEOPLE",
	"THESE ARE PROMISING GUYS",
	"THAT'S A GOOD ONE! A GOOD ONE!",
	// 25
	"PLEASE, CALL ME CONSTANTINO",
	"IT WASN'T ME MAN. IT WAS \"EL COYOTE\", MY TWIN",
	"JESUS, THESE ARE REALLY LONG CREDIT TITLES",
	"I STOPPED COUNTING A LONG TIME AGO",
	"WHAT WILL BECOME OF YOU NOW, DESIDERIO?",
	// 30
	"BUT, YOU SHOULD LOSE SOME WEIGHT",
	"I'LL JUST RETIRE TO THE TIBET AND THINK ABOUT THE MEANING OF LIFE",
},
{
	// 0
	"",
	"UN MOMENTO. \250SOLO PORQUE PERTENECEMOS A RAZAS DISTINTAS Y LA SOCIEDAD DICE QUE SEREMOS ENEMIGOS VAMOS A DEJARNOS LLEVAR POR NUESTROS MAS PRIMITIVOS INSTINTOS?",
	"\250ACASO NO NOS UNE LA RAZON, ELARMA MAS PODEROSA Y, A LA VEZ, EL DON MAS PRECIADO QUE TENEMOS?",
	"\255AH, SI EL ENTENDIMIENTO GUIARA NUESTROS PASOS POR LA VIDA SIN DEJAR SITIO A LOS SENTIMIENTOS, QUE AFLORAN NUESTRAS INCLINACIONES PRE-EVOLUTIVAS!",
	"\250NO CREES QUE SERIAMOS MAS DICHOSOS SIN ESAS ATADURAS EMOCIONALES? CONTESTA EFIMERA CRIATURA",
	// 5
	"NO PASAS",
	"\250VES? ESTE ES UN CLARO EJEMPLO: TU QUIERES PASAR Y CONTINUAR TU AVENTURA Y YO NO PUEDO PERMITIRLO",
	"\250HA DE SER ESE MOTIVO SUFICIENTE DE CONFLICTO ENTRE NOSOTROS, QUE NO NOS CONOCEMOS DE NADA?",
	"PUES ESO",
	"BUENO, ESO DEPENDE DE LO QUE ENTENDAMOS POR RELACION. ALGUNOS AUTORES DEFIENDEN...",
	// 10
	"AGGG. LA CAZA COMO MEDIO DE SUBSISTENCIA ES UNA ACTIVIDAD ARCAICA INCOMPATIBLE CON UNA NATURALEZA SUPERIOR COMO LA MIA. Y ADEMAS ME HE VUELTO VEGETARIANO",
	"RESULTA QUE ESTABA COMIENDOME A UN TIO Y ME PUSE A REFLEXIONAR. LLEGUE A LA CONCLUSION ANTES MECIONADA",
	"ME COSTO MUCHO DEJAR LOS HABITOS DESDE TANTO TIEMPO ADQUIRIDOS, PERO POR FIN MI ALMA IRASCIBLE VENCIO A LA CONCUPISCIBLE Y NO HE VUELTO A PROBAR LA CARNE",
	"NI SIQUIERA EL PLACER QUE SUPONE UN HUESO, CON EL JUGO DE LA PIEL ENTRE SUS POROS Y ESE SABOR QUE TE TRANSPORTA A LUGARES REMOTOS PARADISIACOS...",
	"NI SIQUIERA ME AFECTA. PARA NADA, DE VERDAD",
	// 15
	"\250EL QUE?",
	"NO SE DE QUE ME HABLAS, EFIMERA CRIATURA",
	"NO ME INTERESA",
	"LOS DEMAS JUEGOS, NO SE, PERO ESTE PARA APROVECHAR ESTA PANTALLA TAN BONITA",
	"",
	// 20
	"A MI SI ME DARIA",
	"NO, ES QUE SON EL ABUELO, EL PADRE, EL HIJO, Y UN AMIGO QUE SE LLAMA ASI",
	"NO, PERO ES QUE SI NO VA A PARECER QUE HAN HECHO EL JUEGO ENTRE CINCO",
	"ESTOS CHICOS PROMETEN",
	"\255ESE ES BUENO, ESE ES BUENO!",
	// 25
	"LLAMAME CONSTANTINO",
	"NO ERA YO, HOMBRE. ERA MI DOBLE, EL COYOTE",
	"VAYA, QUE CREDITOS MAS LARGOS",
	"YO YA PERDI LA CUENTA",
	"BUENO DESIDERIO, \250Y QUE VA A SER DE TI AHORA?",
	// 30
	"PERO TENDRIAS QUE ADELGAZAR",
	"ME RETIRARE AL TIBEL A REFLEXIONAR SOBRE EL SENTIDO DE LA VIDA",
},
{
	// 0
	"",
	"MOMENT MAL. NUR WEIL WIR UNTERSCHIEDLICHER HERKUNFT SIND UND EIGENTLICH FEINDE SEIN MueSSTEN, MueSSEN WIR UNS DOCH NICHT WIE DIE PRIMATEN AUFFueHREN!",
	"VERBINDET UNS ETWA NICHT DIE VERNUNFT, GEFaeHRLICHSTE WAFFE UND ZUGLEICH GRoeSSTE GABE, DIE WIR HABEN?",
	"AH, WENN NUR DAS GEGENSEITIGE VERSTaeNDNIS DIE GEFueHLE BESIEGEN KoeNNTE, DIE DIE WURZEL UNSERER PRaeHISTORISCHEN NEIGUNGEN SIND!",
	"GLAUBST DU NICHT, DASS WIR OHNE DIESE GEFueHLSFESSELN GLueCKLICHER WaeREN? ANTWORTE, DU EINTAGSFLIEGE",
	// 5
	"DU KOMMST NICHT DURCH",
	"SIEHST DU? DU WILLST HIER DURCH UND MIT DEM ABENTEUER WEITERMACHEN UND ICH KANN DAS NICHT ZULASSEN",
	"MUSS DENN DIE TATSACHE, DASS WIR UNS NICHT KENNEN, ANLASS ZUM STREIT SEIN?",
	"GENAU",
	"NAJA. KOMMT DARAUF AN, WAS WIR UNTER BEZIEHUNG VERSTEHEN. EINIGE VERTRETEN JA DEN STANDPUNKT, DASS....",
	// 10
	"ARRGH. NUR ZU JAGEN, UM ZU ueBERLEBEN IST MIR ZU PRIMITIV. UND AUSSERDEM BIN ICH SEIT KURZEM VEGETARIER",
	"ES GING LOS, ALS ICH MIR GERADE EINEN TYPEN EINVERLEIBTE UND ANFING NACHZUDENKEN UND ZUM EBENERWaeHNTEN SCHLUSS KAM",
	"ES war hart, DIE ALTEN LASTER ABZULEGEN, ABER seit MEINE FLEISCHESLUST MEINEn JaeHZORN BESIEGT HAT, HABE ICH KEIN FLEISCH MEHR GEGESSEN.",
	"NICHT MAL DIE GAUMENFREUDE EINES KNOCHENS, MIT DEM SAFT DER HAUT IN DEN POREN UND DIESEM GESCHMACK DER DICH INS REICH DER SINNE ENTFueHRT...",
	"ES MACHT MIR ueBERHAUPT NICHTS AUS, WIRKLICH",
	// 15
	"WAS JETZT?",
	"ICH WEISS NICHT, WOVON DU REDEST, DU EINTAGSFLIEGE",
	"DAS INTERESSIERT MICH NICHT",
	"ANDERE SPIELE, ACH ICH WEISS NICHT, ABER DAS HIER HAT SO EINE NETTE GRAFIK",
	"",
	// 20
	"ICH HaeTTE SCHON LUST..",
	"NEIN, SO HEISSEN DER OPA, DER VATER, DER SOHN UND EIN FREUND.",
	"NEIN, ABER WENN NICHT, SIEHT ES SO AUS ALS WaeRE DAS SPIEL VON FueNF LEUTEN GEMACHT WORDEN",
	"DIESE JUNGS SIND VIELVERSPRECHEND!",
	"DER IST GUT, SUPERKLASSE!",
	// 25
	"NENN MICH KONSTANTIN",
	"MANN, DAS WAR NICHT ICH, SONDERN MEIN DOUBLE, DER KOYOTE",
	"WOW, WAS FueR EINE LANGE DANKESLISTE",
	"ICH HAB SCHON DEN ueBERBLICK VERLOREN",
	"O.K. DESIDERIO, UND WAS WIRD JETZT AUS DIR WERDEN?",
	// 30
	"ABER DANN MueSSTEST DU ABNEHMEN",
	"ICH WERDE NACH TIBET AUSWANDERN UND ueBER DEN SINN DES LEBENS NACHDENKEN",
},
{
	// 0
	"",
	"c'est parce qu'on APPARTIENT \267 DES RACES DIFF\220RENTES ET QUE LA SOCI\220T\220 NOUS CONFRONTE, QUE NOUS ALLONS AGIR PAR NOS PLUS M\220PRISABLES INSTINCTS?",
	"NE SOMMES-NOUS PAR HASARD ACCROCHS PAR LA RAISON, L'ARME LA PLUS PUISSANTE, ET AUSSI LE DON LE PLUS PRCIEUX QUE NOUS AVONS?",
	"AH!, SI LA RAISON GUIDAIT NOS PAS DANS LA VIE SANS Y ENTRA\327NER LES SENTIMENTS, QUI FONT JAILLIR NOS INCLINAISONS PR-VOLUTIVES!",
	"NE CROIS-TU PAS QUE NOUS SERIONS PLUS HEREUX SANS CES ATTACHEMENTS-L\267? RPONDS-MOI, CRATURE PHM\324RE ",
	// 5
	"TU NE PASSES PAS",
	"TU VOIS? C'EST UN EXEMPLE CLAIR: TOI, TU VEUX PASSER ET POURSUIVRE TON AVENTURE ET MOI, JE NE PEUX PAS LE TOLRER",
	"CELA DOIT \322TRE UNE RAISON SUFFISANTE DE CONFLIT ENTRE NOUS DEUX, QUI NE NOUS CONNAISSONS DE RIEN?",
	"C'EST CELA",
	"EH BIEN, CELA DPEND DE CE QU'ON ENTEND PAR RLATION. D'APR\324S QUELQUES AUTEURS...",
	// 10
	"LA CHASSE COMME MOYEN DE SUBSISTANCE EST UNE ACTIVIT\220 ARCHA\330QUE, INCOMPATIBLE AVEC ma NATURE SUP\220RIEURE . ET DE PLUS JE SUIS DEVENU V\220G\220TARIEN",
	"IL S'EN SUIT QU'EN TRAIN DE D\220VORER UN TYPE, JE ME SUIS MIS \267 R\220FL\220CHIR. ALORS, SUIS ARRIV\220 \267 LA CONCLUSION MENTIONN\220 CI-DESSUS",
	"ABANDONNER CES HABITUDES M'EST REVENU CHER, MAIS \267 LA FIN MON \266ME IRASCIBLE a vancue MON \266ME CONCUPISCIBLE, ET D\324S LORS JE N'AI GO\352T\220 \267 LA VIANDE",
	"M\322ME PAS LE PLAISIR DE CROQUER UN OS, AVEC LE SUC DE LA PEAU ENTRE SES PORES ET sa SAVEUR QUI TE TRANSPORTE VERS DES LIEUX TR\324S LONTAINS, PARADISIAQUES...",
	"CECI NE M'AFFECTE PAS M\322ME, ABSOLUMENT PAS, C'EST VRAI",
	// 15
	"QUOI?",
	"JE NE SAIS PAS DE QUOI TU M'EN PARLES, CR\220ATURE \220PH\220M\324RE",
	"CELA NE ME REGARDE PAS",
	"LES AUTRES JEUX, JE NE SAIS PAS; MAIS CELUI-CI OUI, POUR EN PROFITER DE CET \220CRAN SI JOLI",
	"",
	// 20
	"MOI, JE NE M'EN FICHERAI PAS",
	"NON, C'EST QU'ILS SONT LE GRAND-P\324RE, LE P\324RE, LE FILS, ET UN AMI QUI S'APPELLE COMME \200A",
	"NON, MAIS SI NON, IL VA PARA\327TRE QU'ON A FAIT LE JEU ENTRE CINQ",
	"CES GAR\200ONS ONT DU FUTUR",
	"CELUI-L\267 EST BON! CELUI-L\267 EST BON!",
	// 25
	"APPELLE-MOI CONSTANTIN",
	"CE N'\220TAIT PAS MOI, MON VIEUX. C'\220TAIT MON DOUBLE, LE COYOTE",
	"TIENS! QUELS CR\220DITS SI LONGS",
	"J'AI D\220J\267 PERDU LES COMPTES",
	"EH BIEN, D\220SID\324RE, QUE T'ARRIVERA-T-IL MAINTENANT?",
	// 30
	"MAIS TU DEVRAIS MAIGRIR",
	"JE VAIS ME RETIRER AU TIBEL POUR Y R\220FL\220CHIR SUR LE SENS DE LA VIE",
},
{
	// 0
	"",
	"UN ATTIMO. SOLO PERCH\220 APPARTENIAMO A RAZZE DIVERSE E LA SOCIET\267 DICE CHE SIAMO NEMICI, VOGLIAMO LASCIARE CHE SIANO I NOSTRI ISTINTI PI\353 PRIMITIVI A GUIDARCI?",
	"NON SIAMO FORSE UNITI DALLA RAGIONE? CHE \324 SIA L'ARMA PI\353 PODEROSA SIA IL DONO PI\353 PREZIOSO CHE ABBIAMO?",
	"AH, SE IL GIUDIZIO GUIDASSE I NOSTRI PASSI NELLA VITA SENZA LASCIARE IL POSTO AI SENTIMENTI CHE MOSTRANO LE NOSTRE INCLINAZIONI PRE-EVOLUTIVE!",
	"RISPONDI, EFFIMERA CREATURA. NON CREDI CHE SAREMMO PI\353 FELICI SENZA QUESTO LEGAME EMOZIONALE?",
	// 5
	"NON PASSERAI",
	"VEDI? QUESTO \324 UN CHIARO ESEMPIO: TU VUOI PASSARE E PROSEGUIRE LA TUA AVVENTURA ED IO NON POSSO PERMETTERLO",
	"DEVE ESSERE DUNQUE MOTIVO DI CONFRONTO TRA NOI, CHE NON CI CONOSCIAMO PER NULLA?",
	"BENE QUINDI",
	"BEH, DIPENDE DA COSA INTENDIAMO PER RELAZIONE. ALCUNI AUTORI DIFENDONO...",
	// 10
	"LA CACCIA COME FORMA DI SUSSISTENZA \324 UNA ATTIVIT\267 ARCAICA, INCOMPATIBILE CON LA MIA ATTUALE NATURA SUPERIORE: SONO DIVENTATO VEGETARIANO",
	"SONO ARRIVATO A QUESTA CONCLUSIONE TEMPO FA. STAVO MANGIANDO UN TIZIO QUANDO MI SONO MESSO A RIFLETTERE.",
	"FU DIFFICILE LASCIARE LE MIE VECCHIE ABITUDINI, MA LA MIA ANIMA IRASCIBILE HA AVUTO LA MEGLIO SU QUELLA CONCUPISCENTE E DA ALLORA NON MANGIO PI\353 CARNE",
	"NEPPURE IL PIACERE DI SUCCHIARE UN OSSO, SENTIRE IL GUSTO DELLA PELLE E IL SAPORE DOLCE DEL MIDOLLO CHE TI PORTA IN POSTI LONTANI E PARADISIACI...",
	"NEPPURE QUESTO MI TOCCA, DAVVERO",
	// 15
	"CHE COSA?",
	"NON CAPISCO DI CHE PARLI, EFFIMERA CREATURA",
	"NON MI INTERESSA",
	"GLI ALTRI VIDEOGIOCHI, NON SAPREI, MA QUESTO \324 PER SFRUTTARE AL MEGLIO QUESTO BELLO SCHERMO",
	"",
	// 20
	"IO ME LA PRENDEREI",
	"NO, SONO IL NONNO, IL PADRE, IL FIGLIO, E UN AMICO CHE SI CHIAMA COS\326",
	"NO, MA SE NON \324 COS\326, SEMBRER\267 CHE IL VIDEOGIOCO L'ABBIANO FATTO IN CINQUE",
	"SONO RAGAZZI PROMETTENTI",
	"\324 BRAVO, \324 BRAVO!",
	// 25
	"CHIAMAMI COSTANTINO",
	"NON ERO IO, DAI. ERA IL MIO GEMELLO, IL COYOTE",
	"ALLA FACCIA, CHE TITOLI DI CODA LUNGHI",
	"HO PERSO IL CONTO",
	"ALLORA DESIDERIO, CHE FARAI ORA?",
	// 30
	"MA DOVRESTI DIMAGRIRE",
	"MI RITIRER\343 IN TIBET A RIFLETTERE SUL SENSO DELLA VITA",
},
};

const char *_textp[NUM_LANGS][NUM_TEXTP] = {
{
	// 0
	"",
	"HI",
	"YES SIR. IT'S BEAUTIFUL",
	"NO, NO. HE WON'T DO IT",
	"ALL RIGHT THEN",
	// 5
	"REALLY?",
	"SO?",
	"I'M SORRY. THE PIANIST TRADE UNION DOESN'T ALLOW ME TO SAVE GIRLS FROM VAMPIRES' CLUTCHES",
	"IF SHE HAD BEEN KIDNAPPED BY THE WEREWOLF...",
	"I CAN ONLY PLAY THIS SONG",
	// 10
	"I'M A CONSERVATOIRE PIANIST AND THE BARTENDER WON'T BUY MORE SCORES FOR ME",
	"OH GOD, I REALLY LOVE CLASSICAL MUSIC!",
	"IT'S BECAUSE I'M WEARING EAR-PLUGS",
	"IT'S BECAUSE I CAN LIP-READ",
	"NOOO",
	// 15
	"NO! I'M NOT TAKING THIS ANY LONGER!",
	"NO WAYYYYY!",
	"WHAT? OF COURSE I'M INTERESTED",
	"THANK GOD! I CAN PLAY A DIFFERENT SONG NOW!",
	"I GUESS YOU CAN KEEP MY EAR-PLUGS",
},
{
	// 0
	"",
	"HOLA",
	"BONITA. SI SE\245OR",
	"NO QUE NO LO HACE",
	"PUES VALE",
	// 5
	"\250SI?",
	"\250Y?",
	"LO SIENTO. EL SINDICATO DE PIANISTAS NO ME PERMITE RESCATAR CHICAS DE LAS GARRAS DE LOS VAMPIROS",
	"SI LA HUBIERA RAPTADO UN HOMBRE LOBO...",
	"NO PUEDO TOCAR MAS QUE ESTA CANCION",
	// 10
	"ES QUE SOY PIANISTA DE CONSERVATORIO Y EL TABERNERO NO COMPRA MAS PARTITURAS",
	"\255CON LO QUE A MI ME GUSTA LA MUSICA CLASICA!",
	"PORQUE LLEVO TAPONES EN LOS OIDOS",
	"PORQUE LEO LOS LABIOS",
	"NOOO",
	// 15
	"\255QUE NO! \255QUE NO ME AGUANTO!",
	"\255QUE NOOOOOO!",
	"\250QUE? CLARO QUE ME INTERESA",
	"AHORA PODRE TOCAR OTRA CANCION \255QUE ALIVIO!",
	"SUPONGO QUE TE PUEDES QUEDAR CON MIS TAPONES",
},
{
	// 0
	"",
	"HALLO",
	"JAWOLL, SEHR SCHoeN",
	"NEIN, ER TUT ES EINFACH NICHT",
	"NA GUT, OK.",
	// 5
	"JA?",
	"UND?",
	"TUT MIR LEID. DIE KLAVIERSPIELERGEWERKSCHAFT ERLAUBT ES MIR NICHT, MaeDCHEN AUS DEN KLAUEN VON VAMPIREN ZU BEFREIEN",
	"WENN SIE WERWOLF ENTFueHRT HaeTTE.....",
	"ICH KANN NUR DIESES EINE LIED",
	// 10
	"ICH HABE AM KONSERVATORIUM KLAVIER GESPIELT UND DER WIRT WILL MEINE PARTITueDEN NICHT KAUFEN",
	"WO MIR DOCH KLASSISCHE MUSIK SO GUT GEFaeLLT!",
	"WEIL ICH OHRENSToePSEL IN DEN OHREN HABE",
	"WEIL ICH VON DEN LIPPEN LESEN KANN",
	"NEEEIN",
	// 15
	"NEEIEN! ICH KANN MICH NICHT BEHERRSCHEN!",
	"NEIN,NEIN, UND NOCHMAL NEIN!",
	"WIE BITTE? KLAR BIN ICH INTERESSIERT",
	"ENDLICH KANN ICH WAS ANDERES SPIELEN, WELCH ERLEICHTERUNG!",
	"ICH GLAUBE, DU KANNST MEINE OHRENSToePSEL BEHALTEN",
},
{
	// 0
	"",
	"BONJOUR!",
	"JOLIE, OUI M'SIEUR",
	"NON, QU'ELLE NE LE FAIT PAS",
	"D'ACCORD, A VA",
	// 5
	"OUI?",
	"ET QUOI?",
	"SUIS DSOL. LE SYNDICAT DE PIANISTES NE ME PERMET PAS DE LIBRER LES FILLES DES GRIFFES DE VAMPIRES",
	"SI ELLE AVAIT T RAPTE PAR UN HOMME-LOUP...",
	"JE NE PEUX PAS JOUER DAVANTAGE CETTE CHANSON",
	// 10
	"JE SUIS LE PIANISTE DU CONSERVATOIRE ET LE TAVERNIER N'ACH\324TE PAS MES PARTITIONS",
	"ET MOI QUI AIME BEAUCOUP LA MUSIQUE CLASSIQUE!",
	"PARCE QUE J'AI DES TAMPONS \267 L'OU\330E",
	"PARCE QUE JE LIS LES LEVRES",
	"NOOON!",
	// 15
	"QUE NON! QUE JE NE PEUX PAS ME RETENIR DAVANTAGE!",
	"QUE NOOOOOON!",
	"QUOI? BIEN S\352R QUE \200A M'INT\220RESSE",
	"MAINTENANT JE POURRAI JOUER UNE AUTRE CHANSON -QUEL SOULAGEMENT!",
	"TU PEUX GARDER MES TAMPONS, JE SUPPOSE",
},
{
	// 0
	"",
	"CIAO",
	"SISSIGNORE, MOLTO BELLA",
	"NO CHE NON LO FA",
	"VA BENE",
	// 5
	"S\326?",
	"E?",
	"MI DISPIACE. IL SINDACATO DEI PIANISTI NON MI PERMETTE DI SALVARE RAGAZZE DALLE GRINFIE DEI VAMPIRI",
	"SE FOSSE STATA RAPITA DAL LUPO MANNARO...",
	"NON POSSO SUONARE ALTRO CHE QUESTA CANZONE",
	// 10
	"\324 PERCH\220 SONO PIANISTA DI CONSERVATORIO E IL BARISTA NON COMPRA PI\353 SPARTITI",
	"E DIRE CHE MI PIACE MOLTISSIMO LA MUSICA CLASSICA!",
	"PERCH\220 HO I TAPPI NELLE ORECCHIE",
	"PERCH\220 SO LEGGERE LE LABBRA",
	"NOOO",
	// 15
	"NO! NON POSSO ANDARE AVANTI COS\326!",
	"HO DETTO DI NOOO!",
	"COSA? CERTO CHE MI INTERESSA",
	"DEO GRATIAS! ADESSO POTR\343 SUONARE UN'ALTRA CANZONE!",
	"SUPPONGO CHE ORA POSSA DARTI I MIEI TAPPI",
},
};


const char *_textt[NUM_LANGS][NUM_TEXTT] = {
{
	// 0
	"",
	"WHAT HAPPENED? WHAT'S THE MATTER?",
	"OK. ROOM 512. UPSTAIRS. THE KEY IS UNDER THE DOOR",
	"COUNT DRASCULA!!?",
	"NO, NOTHING. THAT GUY JUST HAS A BAD REPUTATION AROUND HERE",
	// 5
	"WELL, THERE ARE ALL KINDS OF STORIES GOING AROUND ABOUT HIM, SOME SAY HE IS A VAMPIRE WHO KIDNAPS PEOPLE TO SUCK THEIR BLOOD",
	"HOWEVER, SOME OTHERS SAY THAT HE IS JUST AN ORGAN-DEALER AND THAT IS THE REASON WHY THERE ARE BODY PARTS ALL OVER THE PLACE",
	"BUT OF COURSE, THOSE ARE JUST RUMORS. HE'S PROBABLY BOTH THINGS. BY THE WAY, WHY DO YOU WANT TO MEET HIM?",
	"FORGET IT. I'M REALLY BUSY...",
	"WELL, OK. BUT JUST BECAUSE I WANT TO DO IT, NOT BECAUSE YOU TOLD ME TO",
	// 10
	"THEY'RE WINNING",
	"LEAVE ME ALONE, ALL RIGHT?",
	"OF COURSE. I'M NOT BLIND",
	"THE TRADITION IN THIS VILLAGE IS TO FORGET ALL HARD FEELINGS WHENEVER THERE IS A GAME, TO CHEER UP THE LOCAL TEAM",
	"AND PLEASE, SHUT UP FOR GOD'S SAKE. I CAN'T HEAR ANYTHING!",
	// 15
	"COME ON, LEAVE ME ALONE AND DON'T BOTHER ME ANYMORE",
	"IT HAS JUST STARTED! SHUT UP!",
	"OK, OK, I THOUGHT SOMETHING WAS GOING ON",
	"IT DOESN'T MATTER, ANYWAY. SHE IS PROBABLY DEAD BY NOW",
	"HE JUST STARTED PLAYING CLASSICAL MUSIC, AND I COULDN'T STAND IT",
	// 20
	"SINCE I'M PAYING HIM FOR PLAYING WHATEVER I WISH, I JUST FIRED HIM",
	"AND THEN, HE GOT FRESH WITH ME. JESUS!, HE LOOKED SO NICE AND INNOCENT...WHAT A HYPOCRITE!",
	"BY THE WAY, BE CAREFUL BECAUSE I JUST WAXED THE FLOOR",
	"SHUT UP! WE'RE WATCHING THE GAME!",
	"OH, COME ON! TAKE IT!",
},
{
	// 0
	"",
	"\250QUE PASA, QUE PASA?",
	"DE ACUERDO. HABITACION 512. SUBIENDO POR LAS ESCALERAS. LA LLAVE ESTA EN LA PUERTA",
	"\255\250 EL CONDE DRASCULA?!",
	"NO, NADA. ESE TIPO TIENE MALA FAMA POR AQUI",
	// 5
	"BUENO, CORREN VARIOS RUMORES SOBRE EL. ALGUNOS DICEN QUE ES UN VAMPIRO Y VA SECUESTRANDO GENTE PARA CHUPARLES LA SANGRE",
	"CLARO, QUE OTROS OPINAN QUE SOLO ES UN TRAFICANTE DE ORGANOS, Y POR ESO APARECE GENTE DESCUARTIZADA POR LOS ALREDEDORES",
	"POR SUPUESTO SON SOLO RUMORES. LO MAS PROBABLE ES QUE SEA LAS DOS COSAS. POR CIERTO, \250PARA QUE QUIERE USTED VER A ESE TIPO?",
	"NO DEJELO, QUE ES QUE TENGO MUCHO QUE HACER...",
	"BUENO VALE, PERO PORQUE QUIERO YO, NO PORQUE LO DIGAS TU",
	// 10
	"VAN GANANDO",
	"DEJAME EN PAZ, \250VALE?",
	"PUES CLARO, NO SOY CIEGO",
	"ES TRADICION EN EL PUEBLO QUE CUANDO HAY PARTIDO SE OLVIDAN RENCORES PARA ANIMAR A LA SELECCION",
	"Y CALLATE YA DE UNA VEZ, QUE NO ME DEJAS OIR",
	// 15
	"ANDA, DEJAME EN PAZ Y NO MOLESTES",
	"\255ACABA DE EMPEZAR! \255Y CALLATE!",
	"AH, BUENO. CREIA QUE PASABA ALGO",
	"NO, SI ES IGUAL. A ESTAS HORAS YA ESTARA MUERTA",
	"ES QUE SE PUSO A TOCAR MUSICA CLASICA Y YO NO LA AGUANTO",
	// 20
	"Y COMO YO LE PAGO PARA QUE TOQUE LO QUE YO QUIERO, PUES LE HE ECHADO",
	"Y ENCIMA SE ME PUSO CHULO... \255Y PARECIA UNA MOSQUITA MUERTA!",
	"...POR CIERTO, TENGA CUIDADO. EL SUELO ESTA RECIEN ENCERADO",
	"\255SILENCIO! \255ESTAMOS VIENDO EL PARTIDO!",
	"\255VENGA, ANDA! TOMA.",
},
{
	// 0
	"",
	"WAS ZUM TEUFEL IST LOS?",
	"O.K. ZIMMER 512. DIE TREPPE HOCH. DIE SCHLueSSEL STECKEN SCHON",
	"GRAF DRASCULA?!",
	"NEIN, DER TYP HAT HIER EINEN SCHLECHTEN RUF",
	// 5
	"NAJA, ES GIBT MEHRERE GERueCHTE ueBER IHN. EINIGE BEHAUPTEN, DASS ER LEUTE ENTFueHRT UND IHNEN DAS BLUT AUSSAUGT",
	"KLAR, ANDERE GLAUBEN, DASS ER EIN ORGANHaeNDLER IST UND DESHALB TAUCHEN ueBERALL AM WALDRAND ZERSTueCKELTE LEICHEN AUF",
	"NATueRLICH SIND DAS ALLES NUR GERueCHTE. HoeCHSTWAHRSCHEINLICH STIMMEN SIE ABER. ACH, UND WAS WOLLEN SIE VON IHM?",
	"NEIN, VERGESSEN SIES. ICH HABE NaeMLICH ZU TUN...",
	"O.K. ABER WEIL ICH DAS WILL UND NICHT WEIL DU DAS SAGST",
	// 10
	"SIE GEWINNEN",
	"LASS MICH IN RUHE, OK?",
	"KLAR, ICH BIN DOCH NICHT BLIND",
	"ES IST EIN ALTER BRAUCH IM DORF, DASS MAN BEI EINEM SPIEL ALTE ZWISTE VERGISST, UM DIE MANNSCHAFT ANZUFEUERN",
	"UND HALT ENDLICH DEINEN MUND, ICH WILL ZUHoeREN",
	// 15
	"MANN, LASS MICH IN RUHE UND NERV MICH NICHT",
	"ES FaeNGT GERADE AN! STILL JETZT!",
	"ACH SO, ICH DACHTE ES IST IRGENDWAS LOS",
	"NEIN, IST DOCH EGAL. JETZT WIRD SIE EH SCHON TOT SEIN",
	"ER HAT KLASSIK GESPIELT UND ICH HALTE DAS NICHT AUS",
	// 20
	"UND WEIL ICH ES BIN, DER IHN DAFueR BEZAHLT, DASS ER SPIELT, WAS ICH WILL, HABE ICH IHN ENTLASSEN",
	"UND DANN KAM ER MIR AUCH NOCH SCHRaeG... UND WAS FueR EINE PAPPNASE ER DOCH IST!",
	"...ACH JA, UND PASSEN SIE AUF. DER BODEN IST FRISCH GEBOHNERT",
	"RUHE! WIR GUCKEN GERADE DAS SPIEL!",
	"LOS, HAU REIN, GIBS IHM",
},
{
	// 0
	"",
	"QU'EST-CE QU'IL Y A, QU'Y A-T-IL?",
	"D'ACCORD. CHAMBRE 512. PAR LES ESCALIERS. LA CL EST SUR LA PORTE",
	"LE COMTE DRASCULA?!",
	"NON, RIEN, CE TYPE A UNE MAUVAISE R\220PUTATION PAR ICI",
	// 5
	"EH BIEN, IL Y A DES RUMEURS QUI COURENT SUR LUI. CERTAINS DISENT QUE C'EST UN VAMPIRE ET QU'IL ENLEVE DU MONDE POUR SUCER LEUR SANG ",
	"MAIS D'AUTRES PENSENT QU'IL EST SEULEMENT UN TRAFICANT D'ORGANES, ET C'EST POUR CELA QUE DES PERSONNES DPECES SONT APPARUES DANS LES ALENTOURS",
	"CERTAINEMENT IL NE S'AGIT QUE DES BRUITS QUI COURENT. S\352REMENT IL AURA LES DEUX MTIERS. MAIS, POURQUOI VOULEZ-VOUS VOIR CE TYPE?",
	"OH, NON! OUBLIEZ CELA, J'AI BEAUCOUP \267 FAIRE.",
	"BON, A VA. MAIS PARCE QUE JE VEUX ET NON PARCE QUE TU LE DIS",
	// 10
	"ILS GAGNENT",
	"FICHE-MOI LA PAIX, D'ACCORD?",
	"C'EST S\352R, JE NE SUIS PAS UN AVUGLE",
	"SELON LA TRADITION DU VILLAGE, QUAND IL Y A UN MATCH ON OUBLIE LES RANCUNES, POUR ALLER ENCOURAGER LA S\220LECTION",
	"ET FERME-LA D'UNE FOIS!, JE NE PEUX PAS ENTENDRE",
	// 15
	"ALLEZ, VA-T'-EN ET NE D\220RANGES PAS!",
	"\200A VIENT DE COMMENCER! -ET TAIS-TOI!",
	"AH, BON! JE PENSAIS QU'IL SE PASSAIT QUELQUE CHOSE",
	"NON, C'EST \220GAL. \267 CES HEURES-CI ELLE SERA D\220J\267 MORTE",
	"LE FAIT EST QU'ELLE A COMMENC\220 JOUER DE LA MUSIQUE CLASSIQUE ET QUE JE NE SUPPORTE PAS CELA",
	// 20
	"ET MAINTENANT QUE JE LUI AI MIS DEHORS, COMMENT JE LUI PAIE POUR QU'IL JOUE CE QUE JE VEUX",
	"ET ENCORE IL S'EST MONTR\220 ARROGANT... -ET DIRE QU'IL PARAISSAIT UNE SAINTE-NITOUCHE!",
	"...FAITES ATTENTION, ON VIENT DE CIRER LE PARQUET",
	"SILENCE! ON VOIT LE MATCH!",
	"OH L\267 L\267! TIENS!",
},
{
	// 0
	"",
	"CHE C'\324, CHE C'\324?",
	"D'ACCORDO. CAMERA 512. DEVE SALIRE LE SCALE. LA CHIAVE \324 NELLA PORTA",
	"IL CONTE DRASCULA?!",
	"NO, NIENTE. \324 SOLO CHE QUEL TIPO HA UNA CATTIVA REPUTAZIONE QUI",
	// 5
	"BEH, CORRONO MOLTE VOCI SUL SUO CONTO. ALCUNI DICONO CHE \324 UN VAMPIRO E RAPISCE LE PERSONE PER SUCCHIAR LORO IL SANGUE",
	"COMUNQUE, ALTRI DICONO CHE \324 SOLO UN TRAFFICANTE DI ORGANI, ED \324 PER QUESTO CHE SI TROVANO CORPI SQUARTATI NELLE VICINANZE",
	"CHIARAMENTE SONO SOLTANTO VOCI. \324 PI\353 PROBABILE CHE SIA ENTRAMBE LE COSE. A PROPOSITO, PERCH\220 VUOLE INCONTRARE QUEL TIPO?",
	"NO, LASCIA PERDERE. HO MOLTO DA FARE...",
	"VA BENE, OK. MA PERCH\220 LO VOGLIO IO, NON PERCH\220 L'HAI DETTO TU",
	// 10
	"STANNO VINCENDO",
	"LASCIAMI IN PACE, OK?",
	"CERTO, NON SONO CIECO",
	"IN PAESE C'\324 L'USANZA DI DIMENTICARE I RANCORI QUANDO C'\324 UNA PARTITA, PER SOSTENERE LA SQUADRA LOCALE",
	"E STAI ZITTO UNA BUONA VOLTA. NON RIESCO A SENTIRE",
	// 15
	"INSOMMA, LASCIAMI IN PACE E NON MI DISTURBARE PI\353",
	"\324 APPENA INIZIATA! STAI ZITTO!",
	"AH, OK. CREDEVO CHE FOSSE SUCCESSO QUALCOSA",
	"NO, \324 INUTILE. PROBABILMENTE A QUEST'ORA SAR\267 GI\267 MORTA",
	"\324 CHE SI \324 MESSO A SUONARE MUSICA CLASSICA. E IO NON LA SOPPORTO",
	// 20
	"E SICCOME LO PAGO PERCH\220 SUONI QUELLO CHE VOGLIO IO, L'HO LICENZIATO",
	"E POI SI \324 MESSO A FARE IL BULLO... SEMBRAVA COS\326 INNOCUO PRIMA, CHE IPOCRITA!",
	"...A PROPOSITO, FACCIA ATTENZIONE. HO APPENA PASSATO LA CERA",
	"SILENZIO! STIAMO GUARDANDO LA PARTITA!",
	"DAI, FORZA! PRENDI.",
},
};


const char *_textvb[NUM_LANGS][NUM_TEXTVB] = {
{
	// 0
	"",
	"WHO THE HELL IS CALLING AT THIS TIME?",
	"OH, ..OH, NO, NO....I'M...GANIMEDES THE DWARF. PROFESSOR VON BRAUN DOESN'T LIVE HERE ANYMORE",
	"NO, I DON'T KNOW WHERE HE LIVES!!",
	"GET OUT!!",
	// 5
	"IT'S TOO LATE NOW, YOU IDIOT!! IT ALWAYS IS",
	"I COULDN'T AGREE MORE",
	"ME, SCARED?",
	"LISTEN HERE, DUDE. YOU'RE TALKING TO THE ONLY PERSON WHO KNOWS THE SECRET OF HOW TO FIGHT AGAINST THE VAMPIRES",
	"YOU NEED TO HAVE SPECIAL SKILLS TO FIGHT AGAINST A VAMPIRE. NOT EVERYBODY CAN DO IT",
	// 10
	"YOU DON'T HAVE THEM",
	"I'M SURE YOU WOULDN'T BET ALL YOUR MONEY ON IT, HUH?",
	"WELL, ALL RIGHT, COME ON IN",
	"IF YOU REALLY MEAN TO FACE DRASCULA, YOU'VE GOT TO BE ABLE TO STAND ALL TYPES OF CREAKING AND VAMPIRE-LIKE NOISES",
	"IS THAT CLEAR?",
	// 15
	"OK, WAIT A MINUTE",
	"STAND IN THE CENTER OF THE ROOM, PLEASE",
	"NOW WHERE DID I PUT THE RECORD CALLED \"NAILS SCRATCHING THE BLACKBOARD\"?",
	"ALL RIGHT. LET'S GET TO IT",
	"YOU ARE USELESS. YOU SEE...?, JUST LIKE THE REST!!",
	// 20
	"JUST GIVE ME THE MONEY, YOU LOST SO GET OUT OF HERE",
	"AND DON'T COME BACK UNTIL YOU ARE ABSOLUTELY READY",
	"WHAT DO YOU WANT NOW?",
	"I HAVE TO ADMIT IT. YOU REALLY HAVE WHAT IT TAKES TO FIGHT AGAINST THE VAMPIRES",
	"HEY, TAKE YOUR MONEY. I'LL ADMIT IT. I MAKE A MISTAKE...",
	// 25
	"LEAVE ME ALONE NOW, I WANT TO GET SOME SLEEP",
	"WHENEVER YOU ARE READY TO FIGHT AGAINST THE VAMPIRES, JUST COME BACK AND I'LL HELP YOU OUT",
	"OH, THAT'S EASY. TAKE THIS CRUCIFIX JUST USING IT'S LIGHT IS ENOUGH TO DESTROY A VAMPIRE",
	"YOU HAVE TO BE EXTRA CAREFUL WITH DRASCULA, HIS FRISISNOTIC POWERS HAVE MADE OF HIM THE MOST POWERFUL VAMPIRE",
	"YOU'D BE LOST IF IT WASN'T FOR THIS...",
	// 30
	"...BREW!",
	"YEAH, YOU'RE RIGHT! I MIGHT HAVE SOME PROBLEMS WITH MY BACK IN THE FUTURE IF I KEEP ON SLEEPING THIS WAY",
	"I UNDERSTAND HE WAS A BETTER OPPONENT THAN ME, BUT YOU HAVE TO ADMIT THAT THE DISCOVERY I MADE ABOUT ANTI-VAMPIRE TECHNIQUES WAS WHAT ACTUALLY PROTECTED ME",
	"I'VE FOUND THIS IMMUNIZING BREW THAT KEEPS YOU SAFE FROM ANY VAMPIRES' BITE OR AGAINST HIS FRISISNOTIC POWERS",
	"NO, NO, EXCUSE ME. I HAD IT ONCE BUT IT'S VERY DANGEROUS TO HAVE A BREW OF THAT TYPE. CAN YOU IMAGINE WHAT COULD HAPPEN IF A VAMPIRE GOT IT?",
	// 35
	"HE'D BE IMMUNIZED AGAINST GARLIC, AND THE SUNSLIGHT... SO I HAD TO GET RID OF WHAT I DIDN'T USE BY THE SCIENTIFIC METHOD OF THROWING IT IN THE LAVATORY",
	"DON'T WORRY, I REMEMBER EXACTLY HOW TO MAKE THAT BREW",
	"I NEED GARLIC, BUT I ALREADY HAVE THAT. HOWEVER YOU'LL HAVE TO GET ME SOME WAX, BUBBLE GUM AND CIGARETTE PAPER OR PERHAPS A NAPKIN OR SOMETHING ALIKE",
	"OH...AND OF COURSE THE MOST IMPORTANT INGREDIENT. LEAVES FROM A VERY STRANGE PLANT CALLED FERNAN",
	"IT'S A CLIMBING PLANT WHOSE LEAVES HAVE MAGIC POWERS IF THEY'RE CUT WITH A GOLDEN SICKLE",
	// 40
	"SO AS SOON AS YOU HAVE THESE FIVE THINGS, JUST COME HERE AND I'LL MAKE THE BREW",
	"YOU'LL BE READY THEN TO FIGHT AGAINST DRASCULA",
	"REMEMBER: WAX, NICOTINE, A PIECE OF BUBBLE GUM, PAPER AND SOME FERNAN'S LEAVES CUT WITH A GOLDEN SICKLE",
	"I TOLD YOU! IT WAS BECAUSE OF THE BREW!",
	"OH, ALL RIGHT. I'M GOING TO MAKE MYSELF ...THE BREW. JUST A MOMENT, OK?",
	// 45
	"IT'S A PROTECTING SPELL AGAINST VAMPIRES",
	"I PUT IT THERE IN ORDER TO PRETEND THE ARTIST DIDN'T FORGET TO DRAW THE WINDOW YOU CAN SEE FROM OUTSIDE",
	"ALL RIGHT, THE FIRST THING YOU MUST KNOW IS THE WAY TO DRASCULA'S CASTLE",
	"THERE IS A CAVERN THAT GETS YOU STRAIGHT FROM THE CASTLE. IGOR, THE CRAZY SERVANT', TAKES IT TO GET TO THE VILLAGE EACH MORNING",
	"BE CAREFUL THOUGH, THERE IS A VAMPIRE ALWAYS WATCHING OVER IT. YOU'LL HAVE TO GET RID OF HIM",
	// 50
	"THERE IS AN OLD WELL RIGHT BY THE CEMETERY CHURCH",
	"IT WAS USED A LONG TIME AGO FOR WITCHCRAFT TRIALS",
	"THEY THREW THE WITCHES IN THE WELL. IF THEY DROWNED THEY WERE REAL WITCHES. IF THEY DIDN'T, THEY WEREN'T",
	"WE THREW ONE ONCE AND SHE DIDN'T DROWN, I GUESS SHE WASN'T A WITCH",
	"ANYWAY. THERE IS YOUR BREW. HOWEVER, I ONLY GOT TO MAKE ENOUGH FOR ONE PERSON",
	// 55
	"YOU'D BETTER SMOKE IT RIGHT BEFORE YOU FIGHT AGAINST DRASCULA",
	"COME ON, RUN!",
	"OH, JUST EXCUSES...!",
	"ARE YOU JOHN HACKER? I'M DOCTOR VON BRAUN",
	"LISTEN TO ME, THIS IS VERY IMPORTANT. IT'S ABOUT THE BREW",
	// 60
	"SHUT UP AND LET ME TALK. I JUST FOUND THIS BOOK ABOUT ANTI-VAMPIRE BREWS WARNING AGAINST MIXING THE CIGARETTE WITH ANY ALCOHOLIC DRINK",
	"ALCOHOL REACTS WHEN MIXED WITH BREW, SO THAT IT CANCELS OUT IT'S EFFECTS IN A FEW SECONDS",
	"I'M SORRY, BUT I HAVE TO HANG UP RIGHT NOW. THE POLICE ARE LOOKING FOR ME, THEY THINK I'M A DRUG PUSHER. IDIOTS!. ANYWAY, BYE AND GOOD LUCK SAVING THE WORLD!",
},
{
	// 0
	"",
	"\250QUIEN DIABLOS LLAMA A ESTAS HORAS?",
	"EH... NO, NO. YO SOY EL ENANO GANIMEDES... EL PROFESOR VON BRAUN YA... YA NO VIVE AQUI",
	"\255NO, NO SE DONDE ESTA!",
	"\255QUE TE VAYAS! ",
	// 5
	"IMBECIL. YA ES DEMASIADO TARDE, SIEMPRE LO ES",
	"ESTOY TOTALMENTE DE ACUERDO",
	"\250YO MIEDO?",
	"ENTERATE CHAVAL: ESTAS HABLANDO CON EL UNICO QUE CONOCE EL SECRETO PARA ENFRENTARSE A LOS VAMPIROS",
	"NO TODO EL MUNDO ES CAPAZ DE LUCHAR CON UN VAMPIRO. HAY QUE TENER UNAS CUALIDADES ESPECIALES",
	// 10
	"NO LAS TIENES",
	"\250TE APUESTAS TODO TU DINERO A QUE NO?",
	"ESTA BIEN. PASA",
	"SI DE VERDAD ERES CAPAZ DE ENFRENTARTE A DRASCULA, DEBER PODER SOPORTAR TODOS LOS RUIDOS CHIRRIANTES Y VAMPIRICOS",
	"\250QUEDA CLARO?",
	// 15
	"DE ACUERDO. ESPERA UN MOMENTO",
	"POR FAVOR, PONTE EN EL CENTRO DE LA HABITACION",
	"VEAMOS. \250DONDE HE PUESTO EL DISCO ESTE DE \"U\245AS ARRASCANDO UNA PIZARRA\"?",
	"MUY BIEN. VAMOS ALLA",
	"\250VES? ERES UN INUTIL, COMO TODOS LOS DEMAS",
	// 20
	"AHORA DAME EL DINERO QUE HAS PERDIDO Y VETE DE AQUI",
	"Y NO VUELVAS HASTA QUE NO ESTES PREPARADO DEL TODO",
	"\250Y QUE ES LO QUE QUIERES TU AHORA?",
	"HE DE RECONOCERLO... TIENES APTITUDES PARA LUCHAR CON LOS VAMPIROS",
	"POR CIERTO, TOMA TU DINERO. SE CUANDO ME HE EQUIVOCADO",
	// 25
	"AHORA VETE, QUE QUIERO DORMIR UN POCO",
	"CUANDO ESTES DISPUESTO A ENFRENTARTE A ALGUN VAMPIRO, VUELVE Y TE AYUDARE EN LO QUE PUEDA",
	"OH, ESO ES FACIL. LA LUZ DEL SOL O UN CRUCIFIJO Y LE HACES POLVO",
	"CON EL QUE DEBES PONER ESPECIAL CUIDADO ES CON DRASCULA. SUS PODERES FRISISHNOSTICOS LE HACEN EL MAS PODEROSO DE LOS VAMPIROS",
	"ESTARIAS PERDIDO A NO SER POR LA...",
	// 30
	"...\255POCION!",
	"OH, CLARO. TIENES RAZON, SI SIGO DURMIENDO ASI A LO MEJOR TENGO PROBLEMAS DE COLUMNA CUANDO SEA VIEJO",
	"BUENO, ADMITO QUE RESULTO MEJOR ADVERSARIO QUE YO, PERO MI PRINCIPAL HALLAZGO EN EL ESTUDIO DE TECNICAS ANTI-VAMPIROS FUE LO QUE ME CUBRIO LAS ESPALDAS",
	"DESCUBRI UNA POCION DE INMUNIDAD. TE HACE INVULNERABLE ANTE CUALQUIER MORDEDURA DE VAMPIRO, O A SUS PODERES FRSISSHNOTICOS",
	"NO, PERDONA LA TUVE EN SU DIA, PERO UNA POCION DE ESAS CARACTERISTICAS ES PELIGROSA. IMAGINATE SI CAYERA EN MANOS DE UN VAMPIRO",
	// 35
	"LE HARIA INMUNE A LOS AJOS, A LA LUZ DEL SOL... ASI QUE TUVE QUE DESHACERME DE LO QUE NO USE POR EL CIENTIFICO METODO DE TIRARLO A LA TAZA DEL WATER",
	"TRANQUILO, ME ACUERDO PERFECTAMENTE DE COMO PREPARAR ESA POCION ",
	"NECESITO AJOS, QUE DE ESO YA TENGO, PERO ME TENDRAS QUE CONSEGUIR ALGO DE CERA, NICOTINA, UN CHICLE, Y UN PAPEL DE FUMAR, O UNA SERVILLETA, O ALGO PARECIDO",
	"\255AH! Y POR SUPUESTO, EL INGREDIENTE PRINCIPAL: UNAS HOJAS DE UNA EXTRA\245A PLANTA LLAMADA FERNAN",
	"SE TRATA DE UNA PLANTA ENREDADERA CUYAS HOJAS PROPORCIONAN PODERES MAGICOS SI SE CORTAN CON UNA HOZ DE ORO",
	// 40
	"PUES YA SABES, EN CUANTO TENGAS ESAS CINCO COSAS ME LAS TRAES Y TE PREPARARE LA POCION",
	"DESPUES ESTARAS LISTO PARA LUCHAR CON DRASCULA",
	"RECUERDA: CERA, NICOTINA, UN CHICLE, UN PAPEL Y UNAS HOJAS DE FERNAN, LA PLANTA, CORTADAS CON UNA HOZ DE ORO",
	"\255YA TE LO HE DICHO! FUE TODO GRACIAS A LA POCION",
	"AH, MUY BIEN. VOY ENTONCES A PREPARARME EL PO... LA POCION. SOLO TARDO UN MOMENTO",
	// 45
	"ES UN SORTILEGIO DE PROTECCION CONTRA VAMPIROS",
	"LO PUSE PARA DISIMULAR QUE EL DIBUJANTE SE OLVIDO DE PONER LA VENTANA QUE SE VE DESDE FUERA",
	"BUENO, LO PRIMERO QUE DEBES SABER ES COMO SE VA AL CASTILLO DRASCULA",
	"HAY UNA GRUTA QUE LLEVA DIRECTAMENTE AL CASTILLO Y QUE ESE LOCO FAN DE ELVIS, IGOR, USA PARA BAJAR AL PUEBLO POR LAS MA\245ANAS",
	"PERO TEN CUIDADO, SIEMPRE ESTA PROTEGIDA POR UN VAMPIRO. TENDRAS QUE LIBRARTE DE EL",
	// 50
	"HAY UN VIEJO POZO AL LADO DE LA CAPILLA DEL CEMENTERIO",
	"SE USABA ANTIGUAMENTE PARA JUZGAR CASOS DE BRUJERIA",
	"SE TIRABA A LAS BRUJAS AL POZO. SI SE HUNDIAN ERAN BRUJAS. SI NO, NO",
	"UNA VEZ TIRAMOS A UNA Y NO SE HUNDIO, ASI QUE NO SERIA BRUJA",
	"A LO QUE VAMOS: TOMA TU POCION, PERO SOLO ME HA DADO PARA UNO",
	// 55
	"LO MEJOR ES QUE TE LO FUMES JUSTO ANTES DE ENFRENTARTE A DRASCULA",
	"CORRE",
	"\255EXCUSAS!",
	"\250JOHN HACKER? SOY EL DOCTOR VON BRAUN",
	"ESCUCHEME, ES MUY IMPORTANTE. ES SOBRE LA POCION",
	// 60
	"ME HE ENCONTRADO UN LIBRO SOBRE POCIONES Y DICE QUE NO DEBE USTED BEBER ALCOHOL UNA VEZ SE HAYA FUMADO LA POCION",
	"EL ALCOHOL INGERIDO REACCIONA CON LAS SUSTANCIAS DE LA POCION Y ANULA SUS EFECTOS EN DECIMAS DE SEGUNDO",
	"TENGO QUE COLGAR. LA POLICIA ME BUSCA. DICEN QUE TRAFICO CON DROGAS \255IGNORANTES! BUENO, ADIOS Y SUERTE SALVANDO AL MUNDO",
},
{
	// 0
	"",
	"WER ZUM TEUFEL KLINGELT UM DIESE UHRZEIT?",
	"aeH......NEIN, NEIN. ICH BIN HELGE SCHNEIDER...HERR PROFESSOR VON BRAUN ...aeH...WOHNT HIER NICHT MEHR",
	"NEIN, ICH WEISS NICHT WO ER IST!",
	"NUN GEH SCHON!",
	// 5
	"TROTTEL. ES IST SCHON ZU SPaeT, WIE IMMER",
	"ICH BIN ABSOLUT EINVERSTANDEN",
	"ICH UND DIE HOSEN VOLL?",
	"DAMIT DAS KLAR IST, KURZER: DU SPRICHST GERADE MIT DEM EINZIGEN DER DAS GEHEIMREZEPT GEGEN VAMPIRE HAT",
	"NICHT JEDER KANN GEGEN EINEN VAMPIR ANTRETEN. MAN MUSS EIN PAAR GANZ BESONDERE EIGENSCHAFTEN BESITZEN",
	// 10
	"DU HAST SIE NICHT",
	"WETTEN NICHT? 100.000 MARK, DASS NICHT!",
	"SCHON GUT, KOMM REIN",
	"WENN DU DICH DRACULA WIRKLICH STELLEN WILLST, WIRST DU ALLE DIESE VAMPIRTYPISCHEN SAUG-UND QUIETSCHGERaeUSCHE ERTRAGEN MueSSEN",
	"IST DAS KLAR?",
	// 15
	"O.K. WARTE MAL EINEN MOMENT",
	"STELL DICH BITTE MAL IN DIE MITTE DES ZIMMERS",
	"GUCKEN WIR MAL. WO IST DIESE PLATTE MIT\"FINGERNaeGEL KRATZEN AN DER TAFEL ENTLANG\"?",
	"SEHR GUT, GEHEN WIR DA HIN",
	"SIEHST DU, DU BIST GENAUSO UNFaeHIG WIE ALLE ANDEREN!",
	// 20
	"JETZT GIB MIR DAS GELD, UM DAS WIR GEWETTET HABEN UND KOMM HER",
	"UND KOMM ERST DANN WIEDER, WENN DU 100%IG VORBEREITET BIST",
	"UND WAS WILLST DU JETZT?",
	"ICH MUSS ZUGEBEN, DASS DU DAS NoeTIGE TALENT ZUR VAMPIRJAGD HAST",
	"ueBRIGENS, HIER IST DEIN GELD. ICH HABE MICH GEIRRT",
	// 25
	"UND JETZT GEH, ICH MoeCHTE MICH HINLEGEN",
	"KOMM ZURueCK, WENN DU SO WEIT BIST, DICH EINEM VAMPIR GEGENueBERZUSTELLEN UND ICH WERDE DIR HELFEN",
	"ACH, DAS IST GANZ EINFACH. EIN SONNENSTRAHL ODER EIN KRUZIFIX UND ER WIRD ZU STAUB",
	"AM MEISTEN AUFPASSEN MUSST DU BEI DRASCULA..WEGEN SEINEN TRANSHYPNITISCHEN FaeHIGKEITEN IST ER DER GEFaeHRLICHSTE UNTER DEN VAMPIREN",
	"DU WaeRST VERLOREN, ES SEI DENN DU HaeTTEST...",
	// 30
	"....DIE MISCHUNG!",
	"JA, STIMMT, DU HAST RECHT. WENN ICH WEITERHIN SO LIEGE WERDE ICH IM HOHEN ALTER WOHL RueCKENPROBLEME HABEN",
	"NAJA, ICH GEBE ZU DASS ER DER BESSERE VON UNS WAR, ABER DIE WICHTIGSTE ERKENNTNIS IN MEINEM ANTI-VAMPIR-STUDIUM WAR, MIR EINEN BODYGUARD ZUZULEGEN",
	"ICH HABE EINE IMMUNITaeTSMISCHUNG GEFUNDEN. MIT DER KANN DIR KEIN EINZIGER VAMPIRBISS ETWAS ANHABEN, UND AUCH NICHT SEINE TRANSHYPNISIE.",
	"NEIN, TUT MIR LEID, ICH HATTE SIE DAMALS, ABER EINE MISCHUNG DIESER KONSISTENZ IST SEHR GEFaeHRLICH, UND STELL DIR MAL VOR, ES GERaeT IN VAMPIRHaeNDE",
	// 35
	"DAS WueRDE IHN IMMUN GEGEN KNOBLAUCH UND SONNENLICHT GEMACHT HABEN, DIE RESTE, DIE ICH NICHT MEHR FueR WISSENSCHAFTLICHE ZWECKE BRAUCHTE, LANDETEN IM KLO",
	"NUR DIE RUHE, ICH HABE DAS REZEPT KOMPLETT IM KOPF",
	"ICH BRAUCHE KNOBLAUCH, AH, HAB ICH SCHON, ABER DANN MUSST DU NOCH WACHS BESORGEN, NIKOTIN, KAUGUMMI UND EIN BLaeTTCHEN ODER EINE SERVIETTE ODER SO WAS",
	"ACH KLAR! UND DIE WICHTIGSTE ZUTAT: EIN PAAR BLaeTTER EINER SELTSAMEN PFLANZE DIE FREDDY HEISST",
	"ES HANDELT SICH UM EINE TOPFBLUME DEREN BLaeTTER ZAUBERKRaeFTE HABEN, WENN MAN SIE MIT EINER GOLDSICHEL ABSCHNEIDET",
	// 40
	"ALSO, DU WEISST SCHON, SOBALD DU MIR DIESE FueNF ZUTATEN GEBRACHT HAST, BEREITE ICH DIR DIE MISCHUNG ZU",
	"DANACH BIST DU FueR DEN KAMPF GEGEN DRASCULA GERueSTET",
	"DENK DRAN: WACHS, NIKOTIN, EIN KAUGUMMI, EIN BLaeTTCHEN UND EIN PAAR BLaeTTER VON FREDDY, MIT EINER GOLDSICHEL ABGESCHNITTEN",
	"HAB ICH DIR DOCH SCHON GESAGT! NUR DANK DER MISCHUNG",
	"OK. ALLES KLAR. ICH FANG DANN MAL AN EINEN ZU BAUEN...aeH VORZUBEREITEN. DAUERT AUCH NICHT LANGE",
	// 45
	"DAS IST EINE ZAUBERFORMEL, DIE VOR VAMPIREN SCHueTZT",
	"DAS WAR ICH, DAMIT MAN NICHT MERKT, DASS DER ZEICHNER VERGESSEN HAT DAS FENSTER VON VORHIN ZU ZEICHNEN",
	"ALSO, ZUERST MUSST DU WISSEN WIE MAN ZUM SCHLOSS VON DRASCULA KOMMT.",
	"ES GIBT EINE GROTTE, DIE DIREKT ZUM SCHLOSS FueHRT, UND DURCH DIE IGOR, DIESER BEKLOPPTE ELVIS-FAN MORGENS IMMER ZUM DORF RUNTER GEHT",
	"ABER PASS AUF, SIE WIRD PERMANENT VON EINEM VAMPIR BEWACHT, DEN DU ERSTMAL LOSWERDEN MUSST",
	// 50
	"ES GIBT DA EINEN ALTEN BRUNNEN BEI DER FRIEDHOFSKAPELLE",
	"FRueHER WURDE ER FueR HEXENPROZESSE BENUTZT",
	"MAN WARF DIE HEXEN IN DEN BRUNNEN. WENN SIE UNTERGINGEN WAREN SIE HEXEN, WENN NICHT, DANN NICHT",
	"EINMAL WARFEN WIR EINE RUNTER, DIE NICHT UNTERGING, ALSO WAR SIE KEINE HEXE",
	"WENN WIR LOSGEHEN, NIMMST DU DEINE MISCHUNG, ABER WIR HABEN NUR FueR EINE PERSON",
	// 55
	"AM BESTEN RAUCHST DU IHN DIR KURZ BEVOR DU DRACULA GEGENueBERSTEHST",
	"LAUF!",
	"BILLIGE AUSREDEN!",
	"HEINZ HACKER? ICH BIN DOKTOR VON BRAUN",
	"HoeREN SIE ZU, ES IST SEHR WICHTIG. ES GEHT UM DIE MISCHUNG",
	// 60
	"ICH HABE EIN BUCH ueBER MISCHUNGEN GEFUNDEN UND DA STEHT DASS MAN KEINEN ALKOHOL TRINKEN DARF, SOBALD MAN DIE MISCHUNG GERAUCHT HAT",
	"DER ALKOHOL REAGIERT IM MAGEN MIT DEN SUBSTANZEN DER MISCHUNG UND VERNICHTET IHRE WIRKUNG BINNEN WENIGER MILLISEKUNDEN",
	"ICH MUSS AUFLEGEN. DIE POLIZEI IST HINTER MIR HER. DIE IDIOTEN HALTEN MICH FueR EINEN DROGEN-DEALER! ALSO, TSCHueSS UND VIEL GLueCK!",
},
{
	// 0
	"",
	"QUI DIABLE APPELLE \267 CETTE HEURE-CI?",
	"H... NON, NON. JE SUIS LE NAIN GANYM\324DE... LE PROFESSEUR VON BRAUN... N'HABITE DJ\267 L\267",
	"NON, JE NE SAIS PAS O\353 IL EST!",
	"VA-T'-EN!",
	// 5
	"IMBCIL! C'EST DJ\267 TROP TARD! C'EST TOUJOURS TROP TARD!",
	"JE SUIS TOUT-\267-FAIT CONFORME",
	"MOI PEUR?",
	"RENSEIGNE-TOI, MON GARS: TU PARLES AVEC LE SEUL QUI CONNA\327T LE SECRET POUR CONFRONTER LES VAMPIRES",
	"TOUT LE MONDE N'EST PAS CAPABLE DE LUTTER AVEC UN VAMPIRE. IL FAUT DES QUALITS SPCIALES",
	// 10
	"TU NE LES A PAS",
	"EST-CE QUE TU PARIE TOUT L'ARGENT QUE TU AS \267 QUE C'EST NON?",
	"\200A VA, ENTRE",
	"SI EN V\220RIT\220 TU TE SENS CAPABLE DE CONFRONTER DRASCULA, IL FAUDRA QUE TU SUPPORTES TOUS LES BRUITS GRIN\200ANTS ET SU\200ANTS",
	"C'EST CLAIR?",
	// 15
	"D'ACCORD. ATTENDS UN INSTANT",
	"METS-TOI AU MILIEU DE LA CHAMBRE, S'IL TE PLA\327T",
	"VOYONS! O\353 EST-CE QUE J'AI MIS LE DISQUE \"ONGLES GRATTANT UN TABLEAU\"?",
	"TR\324S BIEN. ON Y VA",
	"TU VOIS? TU ES UN INCAPABLE, COMME TOUS LES AUTRES",
	// 20
	"MAINTENANT DONNE-MOI L'ARGENT QUE TU AS PERDU ET VA-T'-EN",
	"ET N'Y REVIENS \267 MOINS QUE TU SOIS COMPL\324TEMENT EN FORME",
	"ET QUE VEUX-TU MAINTENANT?",
	"JE DOIS LE RECONNA\327TRE... TU AS DES APTITUDES POUR LUTTER CONTRE LES VAMPIRES",
	"EN FAIT, TIENS TON ARGENT. JE SAIS RECONNA\327TRE MES ERREURS",
	// 25
	"MAINTENANT VA-T'-EN, JE VEUX DORMIR UN PEU ",
	"QUAND TU SOIS PR\322T \267 TE CONFRONTER AVEC UN VAMPIRE, REVIENS ET TU POURRAS COMPTER SUR MOI",
	"OH! C'EST FACIL. AVEC LA LUMI\324RE DU SOLEIL OU UN CRUCIFIX TU LE R\220DUIT EN CENDRES",
	"MAIS TU DOIS FAIRE SP\220CIALE ATTENTION AVEC DRASCULA. GR\266CE \267 SES POUVOIRS FRISYSHNOSTIQUES C'EST LE PLUS PUISSANT DES VAMPIRES",
	"JE SERAIS PERDU SI CE N'EST PAS POUR LA...",
	// 30
	"...POTION!",
	"OH, BIEN S\352R. TU AS RAISON, SI JE CONTINUE \267 DORMIR COMME \200\265, J'AURAI DES PROBL\324MES DE COLONNE DANS MA VIEILLESSE",
	"bon, J'ACCEPTE QU'IL A \220T\220 MEILLEURE ADVERSAIRE QUE MOI, MAIS JE ME SUIS GARD\220 LE DOS GR\266CE \267 MA BONNE TROUVAILLE DANS L'\220TUDE DE TECHNIQUES ANTI-VAMPIRES",
	"J'AI D\220COUVERT UNE POTION IMMUNOLOGIQUE QUI TE FAIT INVULN\220RABLE CONTRE N'IMPORTE QUELLE MORSURE DE VAMPIRE OU SES POUVOIRS FRSYSSHNOTIQUES",
	"NON, EXCUSES-MOI. JE L'AI EUE, MAIS UNE POTION DE CES CARACT\220RISTIQUES EST DANGEREUSE. IMAGINE TOI SI ELLE TOMBAIT DANS LES MAINS D'UN VAMPIRE",
	// 35
	"IL DEVIENDRAIT IMMUNIS\220 AUX AILS, \267 LA LUMI\324RE DU SOLEIL. DONC, J'AI D\352 M'EN D\220BARRASSER DES EXC\220DENTS PAR LA SCIENTIFIQUE M\220THODE DE LES JETER AUX \220GOUTS",
	"RESTE TRANQUILLE, JE ME SOUVIENS PARFAITEMENT DE LA PR\220PARATION DE CETTE POTION",
	"il me faut D'AIL, MAIS J'EN AI D\220J\267. IL FAUDRA UN PEU DE CIRE, DE LA NICOTINE, UN CHEWING-GUM ET UN PAPIER \267 CIGARETTES, OU QUELQUE CHOSE PAREILLE",
	"AH! ET CERTAINEMENT, L'INGR\220DIANT PRINCIPAL: LES FEUILLES D'UNE \220TRANGE PLANTE APPEL\220E FERNAN",
	"IL S'AGIT D'UNE PLANTE GRIMPANTE DONT LES FEUILLES FOURNISSENT DES POUVOIRS MAGIQUES SI ON LES COUPE AVEC UNE FAUCILLE EN OR",
	// 40
	"TU LE SAIS D\220J\267. D\324S QUE TU AURAS CES CINQ TRUCS APPORTE-LES-MOI ET JE TE PR\220PARERAI LA POTION",
	"APR\324S TU SERAS PR\322T POUR LUTTER CONTRE DRASCULA",
	"RAPPELLE-TOI: DE LA CIRE, DE LA NICOTINE, UN CHEWING-GUM, UN PAPIER ET LES FEUILLES DE FERNAN, LA PLANTE, COUP\220ES AVEC UNE FAUCILLE EN OR",
	"JE TE L'AI D\220J\267 DIT! TOUT A \220T\220 GR\266CE \267 LA POTION",
	"AH! TR\324S BIEN! ALORS, JE VAIS ME PR\220PARER LE PO... LA POTION. JE N'AI QUE POUR UN MOMENT",
	// 45
	"C'EST UN SORTIL\324GE DE PROTECTION CONTRE-VAMPIRES",
	"JE L'AI MIS POUR DISSIMULER, LE DESSINATEUR AYANT OUBLI\220 D'Y METTRE LA FEN\322TRE QU'ON VOIT DU DEHORS",
	"EH BIEN, CE QUE TU DOIS SAVOIR D'ABORD C'EST LA FA\200ON D'ALLER AU CH\266TEAU DE DRASCULA",
	"IL Y A UNE GROTTE QUI M\324NE DIRECTEMENT AU CH\266TEAU ET QU'IGOR, CE FOU FAN D'ELVIS, EN UTILISE POUR SE RENDRE AU VILLAGE LE MATIN",
	"MAIS FAIT ATTENTION, ELLE TOUJOURS GARD\220E PAR UN VAMPIRE. TU DEVRAS T'EN D\220BARRASSER",
	// 50
	"IL Y A UN VIEUX PUITS \267 C\342T\220 DE LA CHAPELLE DU CIMETI\324RE",
	"ON L'UTILISAIT JADIS POUR JUGER DES AFFAIRES DE SORCELLERIE",
	"ON JETAIT LES SORCI\324RES AU PUITS. SI ELLES COULAIENT, ON \220TAIT S\352R; AUTREMENT, PAS",
	"UNE FOIS ON A JET\220 UNE ET ELLE N'AS PAS COUL\220, DONC ELLE NE SERAIT PAS UNE SORCI\324RE",
	"MAINTENANT GARDE \267 LA QUESTION QUI NOUS INT\220RESSE: TIENS TA POTION, MAIS ON NE M'A PAS DONN\220 QUE POUR UN SEULEMENT.",
	// 55
	"IL VAUDRA MIEUX QUE TU LA FUMES JUSTE AVANT DE CONFRONTER DRASCULA",
	"COURS!",
	"DES EXCUSES!",
	"JHON HACKER? SUIS LE DOCTEUR VON BRAUN",
	"\220COUTEZ-MOI. C'EST TR\324S IMPORTANT. C'EST SUR LA POTION",
	// 60
	"DANS UN LIVRE QUE J'AI TROUV\220 SUR LES POTIONS, IL DIT QUE VOUS NE DEVEZ PAS BOIR DE L'ALCOOL APR\324S AVOIR FUM\220 LA POTION ",
	"L'ALCOOL AVAL\220 R\220AGIT AVEC LES INGR\220DIENTS DE LA POTION ET ANNULE SES EFFETS EN DIXI\324MES DE SECONDE",
	"JE DOIS RACCROCHER. SUIS CHERCH\220 PAR LA POLICE. ILS DISENT QUE JE TRAFIQUE EN DROGUES -IGNORANTS! AU REVOIR ET BONNE CHANCE EN SAUVANT LE MONDE",
},
{
	// 0
	"",
	"CHI DIAVOLO \324 A QUEST'ORA?",
	"OH... NO, NO. IO SONO IL NANO GANIMEDE... IL PROFESSOR VON BRAUN NON VIVE PI\353 QUI",
	"NO, NON SO DOV'\324!",
	"VATTENE!",
	// 5
	"IMBECILLE! ORMAI \324 TROPPO TARDI, LO \324 SEMPRE",
	"SONO TOTALMENTE D'ACCORDO",
	"IO, PAURA?",
	"ASCOLTA BENE, RAGAZZO: STAI PARLANDO CON L'UNICA PERSONA CHE CONOSCE IL SEGRETO PER AFFRONTARE I VAMPIRI",
	"NON TUTTI POSSONO COMBATTERE CONTRO UN VAMPIRO. BISOGNA AVERE DELLE QUALIT\267 SPECIALI",
	// 10
	"NO, NON CE LE HAI",
	"SCOMMETTERESTI TUTTI I TUOI SOLDI?",
	"VA BENE. ENTRA",
	"SE DAVVERO INTENDI AFFRONTARE DRASCULA, DEVI ESSERE IN GRADO DI SOPPORTARE TUTTI I RUMORI PI\353 STRIDENTI E VAMPIRICI",
	"\324 CHIARO?",
	// 15
	"D'ACCORDO. ASPETTA UN MOMENTO",
	"PER FAVORE, METTITI AL CENTRO DELLA STANZA",
	"DOV'\324 CHE HO MESSO IL CD DI \"UNGHIE CHE GRAFFIANO LA LAVAGNA\"?",
	"MOLTO BENE. SI COMINCIA",
	"VEDI? SEI UN INCAPACE, COME TUTTI GLI ALTRI",
	// 20
	"ORA DAMMI I SOLDI CHE HAI PERSO E VATTENE",
	"E NON TORNARE FINCH\220 NON SARAI COMPLETAMENTE PRONTO",
	"CHE COSA VUOI ADESSO?",
	"DEVO RICONOSCERLO... HAI LA STOFFA PER COMBATTERE I VAMPIRI",
	"A PROPOSITO, PRENDI I TUOI SOLDI. SO RICONOSCERE I MIEI ERRORI",
	// 25
	"ADESSO VATTENE, VOGLIO DORMIRE UN PO'",
	"QUANDO SARAI PRONTO AD AFFRONTARE UN VAMPIRO, TORNA E TI AIUTER\343 COME POSSO",
	"OH, \324 FACILE. BASTA LA LUCE DEL SOLE O UN CROCIFISSO PER INCENERIRLO",
	"DEVI INVECE STARE MOLTO ATTENTO A DRASCULA. I SUOI POTERI FRISISNOTICI LO RENDONO IL PI\353 POTENTE DEI VAMPIRI",
	"SARESTI SPACCIATO, SE NON FOSSE PER LA...",
	// 30
	"...POZIONE!",
	"OH, CERTO, HAI RAGIONE! SE CONTINUO A DORMIRE COS\326, FORSE AVR\343 PROBLEMI ALLA SCHIENA QUANDO SAR\343 VECCHIO",
	"BEH, AMMETTO CHE SI \324 RIVELATO PI\353 FORTE DI ME, MA LA MIA PRINCIPALE SCOPERTA SULLE TECNICHE ANTI-VAMPIRO MI HA COPERTO LE SPALLE",
	"HO SCOPERTO UNA POZIONE DI IMMUNIT\267 CHE TI RENDE INVULNERABILE A QUALUNQUE MORSO DI VAMPIRO, O AI SUOI POTERI FRISISNOTICI",
	"NO, MI DISPIACE. CE L'AVEVO UNA VOLTA, MA UNA POZIONE CON QUELLE CARATTERISTICHE \324 PERICOLOSA. IMMAGINA SE CADESSE NELLE MANI DI UN VAMPIRO",
	// 35
	"LO RENDEREBBE IMMUNE ALL'AGLIO E ALLA LUCE DEL SOLE... COS\326 MI SONO LIBERATO DI QUELLA CHE NON HO USATO CON UN METODO ALTAMENTE SCIENTIFICO: GETTANDOLA NELLA TAZZA DEL WATER",
	"TRANQUILLO, MI RICORDO PERFETTAMENTE COME PREPARARE QUELLA POZIONE",
	"HO BISOGNO DI AGLIO, CHE HO GI\267 QUI. PER\343 DOVRAI PORTARMI UN PO' DI CERA, DELLA NICOTINA, UNA GOMMA E UNA CARTINA DI SIGARETTA, O UN TOVAGLIOLO O QUALCOSA DEL GENERE",
	"AH! E, OVVIAMENTE, L'INGREDIENTE PRINCIPALE: LE FOGLIE DI UNA STRANA PIANTA CHIAMATA FERNAN",
	"SI TRATTA DI UNA PIANTA RAMPICANTE LE CUI FOGLIE ACQUISTANO POTERI MAGICI SE VENGONO TAGLIATE CON UNA FALCE D'ORO",
	// 40
	"ALLORA, QUANDO AVRAI QUESTE CINQUE COSE, PORTAMELE E TI PREPARER\343 LA POZIONE",
	"DOPO SARAI PRONTO PER AFFRONTARE DRASCULA",
	"RICORDA: CERA, NICOTINA, UNA GOMMA, UNA CARTINA E DELLE FOGLIE DI FERNAN, LA PIANTA, TAGLIATE CON UNA FALCE D'ORO",
	"TE L'HO GI\267 DETTO! FU MERITO DELLA POZIONE",
	"AH, MOLTO BENE. ALLORA VADO A FARMI LA CAN... LA POZIONE. DAMMI UN MINUTO, OK?",
	// 45
	"\324 UN INCANTESIMO DI PROTEZIONE CONTRO I VAMPIRI",
	"L'HO MESSA L\326 PER NASCONDERE IL FATTO CHE IL DISEGNATORE HA DIMENTICATO DI METTERE LA FINESTRA CHE SI VEDE DA FUORI",
	"BENE, LA PRIMA COSA CHE DEVI SAPERE \324 COME ARRIVARE AL CASTELLO DI DRASCULA",
	"C'\324 UNA GROTTA CHE PORTA DIRETTAMENTE AL CASTELLO. QUEL MATTO FAN DI ELVIS, IGOR, LA USA PER SCENDERE AL PAESE TUTTE LA MATTINE",
	"MA FA' ATTENZIONE, \324 SEMPRE PROTETTA DA UN VAMPIRO. DOVRAI LIBERARTI DI LUI",
	// 50
	"C'\324 UN VECCHIO POZZO ACCANTO ALLA CAPPELLA DEL CIMITERO",
	"SI USAVA ANTICAMENTE PER GIUDICARE I CASI DI STREGONERIA",
	"SI BUTTAVANO LE STREGHE NEL POZZO. SE AFFONDAVANO ERANO STREGHE. SE NO, NO",
	"UNA VOLTA NE BUTTAMMO GI\353 UNA E NON AFFOND\343. SUPPONGO NON FOSSE UNA STREGA.",
	"AD OGNI MODO. PRENDI LA POZIONE. BASTA PER UNA VOLTA SOLA",
	// 55
	"\324 MEGLIO CHE LA FUMI APPENA PRIMA DI AFFRONTARE DRASCULA",
	"CORRI!",
	"SCUSI!",
	"JOHN HACKER? SONO IL DOTTOR VON BRAUN",
	"ASCOLTA, \324 MOLTO IMPORTANTE. RIGUARDA LA POZIONE",
	// 60
	"STAI ZITTO E LASCIAMI PARLARE. HO TROVATO UN LIBRO SULLE POZIONI E DICE CHE NON SI DEVE BERE ALCOL DOPO AVERE FUMATO LA POZIONE",
	"L'ALCOL INGERITO REAGISCE CON LE SOSTANZE DELLA POZIONE E ANNULLA I SUOI EFFETTI IN POCHI SECONDI",
	"MI DISPIACE, MA DEVO RIATTACCARE. LA POLIZIA MI CERCA. DICONO CHE SONO UNO SPACCIATORE. IGNORANTI! BEH, ADDIO E BUONA FORTUNA NEL SALVARE IL MONDO",
},
};


const char *_textsys[NUM_LANGS][NUM_TEXTSYS] = {
{
	"PRESS 'DEL' AGAIN TO RESTART",
	"PRESS 'ESC' AGAIN TO EXIT",
	"SPEECH ONLY",
	"TEXT AND SPEECH",
},
{
	"PULSA DE NUEVO SUPR PARA EMPEZAR",
	"PULSA DE NUEVO ESC PARA SALIR",
	"SOLO VOCES",
	"VOZ Y TEXTO",
},
{
	"BETaeTIGEN SIE ERNEUT entf UM ANZUFANGEN",
	"BETaeTIGEN SIE ERNEUT ESC UM ZU BEENDEN",
	"NUR STIMMEN",
	"STIMME UND TEXT",
},
{
	"APPUYEZ \267 NOUVEAU SUR SUPR POUR COMMENCER",
	"APPUYEZ \267 NOUVEAU SUR ESC POUR SORTIR",
	"SEULEMENT DES VOIX",
	"VOIX ET TEXT",
},
{
	"PREMI DI NUOVO CANC PER RICOMINCIARE",
	"PREMI DI NUOVO ESC PER USCIRE",
	"SOLO VOCI",
	"VOCI E TESTO",
},
};


const char *_texthis[NUM_LANGS][NUM_TEXTHIS] = {
{
	"",
	"A long time ago, it seems that Drascula killed Von Braun's wife, and then, as he intended to face the count, Von Braun started to research all he could find on vampires.",
	"When he thought he was ready, he went up to the castle and had a very violent encounter with Drascula.",
	"Nobody knows exactly what happened there. Although Von Braun lost, Drascula could not kill him.",
	"Von Braun felt humiliated by his defeat, he run away from the castle and has never dared to face Drascula again.",
},
{
	"",
	"",
	"",
	"",
	""
},
{
	"",
	"",
	"",
	"",
	""
},
{
	"",
	"",
	"",
	"",
	""
},
{
	"",
	"RACCONTANO CHE, MOLTI ANNI FA, DRASCULA UCCISE LA MOGLIE DI VON BRAUN. DA ALLORA, CON L'INTENZIONE DI AFFRONTARE IL CONTE, VON BRAUN COMINCI\343 A STUDIARE TUTTO QUELLO CHE TROVAVA SUI VAMPIRI.",
	"QUANDO PENS\343 DI ESSERE PRONTO, AND\343 AL CASTELLO ED EBBE UN VIOLENTO SCONTRO CON DRASCULA.",
	"NESSUNO SA COSA ACCADDE LASS\353. MA SEBBENE VON BRAUN FU SCONFITTO, DRASCULA NON RIUSC\326 A UCCIDERLO.",
	"UMILIATO DALLA SCONFITTA, VON BRAUN SCAPP\343 DAL CASTELLO E DA ALLORA NON HA MAI PI\353 OSATO AFFRONTARE DRASCULA.",
},
};


const char *_textd1[NUM_LANGS][NUM_TEXTD1] = {
{
	// 68
	"44472225500022227555544444664447222550002222755554444466",
	"444722255000222275555444446655033336666664464402256555005504450005446",
	// 70
	"4447222550002222755554444466550333226220044644550044755665500440006655556666655044744656550446470046",
	"550330227556444744446660004446655544444722255000222275555444446644444",
	"55033022755644455550444744400044504447222550002222755554444466000",
	"55033022755644444447227444466644444722255000222275555444446664404446554440055655022227500544446044444446",
	"55033022755644444472244472225500022227555544444662550002222755444446666055522275550005550022200222000222666",
	// 75
	"44447774444555500222205550444444466666225266444755444722255000222275555444446633223220044022203336227555770550444660557220553302224477777550550550222635533000662220002275572227025555",
	"5555500004445550330244472225500022227555544444662755644446666005204402266222003332222774440446665555005550335544444",
	"56665004444447222550002222755554444466555033022755555000444444444444444444444444444444",
	"004447222550002222755554444466222000220555002220550444446666662220000557550033344477222522665444466663337446055504446550550550222633003330000666622044422755722270255566667555655007222777"
},
{
	// 68
	"4447222550002222755554444466",
	"55033336666664464402256555005504450005446",
	// 70
	"550333226220044644550044755665500440006655556666655044744656550446470046",
	"74444666000444665554444444",
	"5555044474440004450000",
	"4444722744446664464404446554440055655022227500544446044444446",
	"4447222550002222755444446666055522275550005550022200222000222666",
	// 75
	"4444777444455550022220555044444446666622526644475533223220044022203336227555770550444660557220553302224477777550550550222635533000662220002275572227025555",
	"5555000444555033022755644446666005204402266222003332222774440446665555005550335544444",
	"5666500444555033022755555000444444444444444444444444444444",
	"00222000220555002220550444446666662220000557550033344477222522665444466663337446055504446550550550222633003330000666622044422755722270255566667555655007222777"
},
{
	// 68
	"44472225500022227555544444664447222550002222755554444466",
	"444722255000222275555444446655033336666664464402256555005504450005446",
	// 70
	"4447222550002222755554444466550333226220044644550044755665500440006655556666655044744656550446470046",
	"550330227556444744446660004446655544444722255000222275555444446644444",
	"55033022755644455550444744400044504447222550002222755554444466000",
	"55033022755644444447227444466644444722255000222275555444446664404446554440055655022227500544446044444446",
	"55033022755644444472244472225500022227555544444662550002222755444446666055522275550005550022200222000222666",
	// 75
	"44447774444555500222205550444444466666225266444755444722255000222275555444446633223220044022203336227555770550444660557220553302224477777550550550222635533000662220002275572227025555",
	"5555500004445550330244472225500022227555544444662755644446666005204402266222003332222774440446665555005550335544444",
	"56665004444447222550002222755554444466555033022755555000444444444444444444444444444444",
	"004447222550002222755554444466222000220555002220550444446666662220000557550033344477222522665444466663337446055504446550550550222633003330000666622044422755722270255566667555655007222777"
},
{
	// 68
	"44472225500022227555544444664447222550002222755554444466",
	"444722255000222275555444446655033336666664464402256555005504450005446",
	// 70
	"4447222550002222755554444466550333226220044644550044755665500440006655556666655044744656550446470046",
	"550330227556444744446660004446655544444722255000222275555444446644444",
	"55033022755644455550444744400044504447222550002222755554444466000",
	"55033022755644444447227444466644444722255000222275555444446664404446554440055655022227500544446044444446",
	"55033022755644444472244472225500022227555544444662550002222755444446666055522275550005550022200222000222666",
	// 75
	"44447774444555500222205550444444466666225266444755444722255000222275555444446633223220044022203336227555770550444660557220553302224477777550550550222635533000662220002275572227025555",
	"5555500004445550330244472225500022227555544444662755644446666005204402266222003332222774440446665555005550335544444",
	"56665004444447222550002222755554444466555033022755555000444444444444444444444444444444",
	"004447222550002222755554444466222000220555002220550444446666662220000557550033344477222522665444466663337446055504446550550550222633003330000666622044422755722270255566667555655007222777"
},
{
	// 68
	"44472225500022227555544444664447222550002222755554444466",
	"444722255000222275555444446655033336666664464402256555005504450005446",
	// 70
	"4447222550002222755554444466550333226220044644550044755665500440006655556666655044744656550446470046",
	"550330227556444744446660004446655544444722255000222275555444446644444",
	"55033022755644455550444744400044504447222550002222755554444466000",
	"55033022755644444447227444466644444722255000222275555444446664404446554440055655022227500544446044444446",
	"55033022755644444472244472225500022227555544444662550002222755444446666055522275550005550022200222000222666",
	// 75
	"44447774444555500222205550444444466666225266444755444722255000222275555444446633223220044022203336227555770550444660557220553302224477777550550550222635533000662220002275572227025555",
	"5555500004445550330244472225500022227555544444662755644446666005204402266222003332222774440446665555005550335544444",
	"56665004444447222550002222755554444466555033022755555000444444444444444444444444444444",
	"004447222550002222755554444466222000220555002220550444446666662220000557550033344477222522665444466663337446055504446550550550222633003330000666622044422755722270255566667555655007222777"
},

};


const char *_textverbs[NUM_LANGS][NUM_TEXTVERBS] = {
{
	"look",
	"take",
	"open",
	"close",
	"talk",
	"push",
},
{
	"mirar",
	"coger",
	"abrir",
	"cerrar",
	"hablar",
	"mover",
},
{
	"Schau",
	"Nimm",
	"oeffne",
	"Schliesse",
	"Rede",
	"Druecke",
},
{
	"regardez",
	"ramassez",
	"ouvrez",
	"fermez",
	"parlez",
	"poussez",
},
{
	"guarda",
	"prendi",
	"apri",
	"chiudi",
	"parla",
	"premi",
},
};


const char *_textmisc[NUM_LANGS][NUM_TEXTMISC] = {
{
	"HUNCHBACKED",
	"Transilvania, 1993 d.c.",
	"GOOOOOOOAAAAAAAL!",
},
{
	"jorobado",
	"Transilvania, 1993 d.c. (despues de cenar)",
	"GOOOOOOOOOOOOOOOL",
},
{
	"HUNCHBACKED",
	"Transilvania, 1993 d.c.",
	"GOOOOOOOAAAAAAAL!",
},
{
	"HUNCHBACKED",
	"Transilvania, 1993 d.c.",
	"GOOOOOOOAAAAAAAL!",
},
{
	"GOBBO",
	"Transilvania, 1993 d.c. (dopo cena)",
	"GOOOOOOOOOOOOOOOL!",
},
};

#endif /* STATICDATA_H */
