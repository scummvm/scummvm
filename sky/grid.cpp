/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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

#include "stdafx.h"
#include "sky/sky.h"
#include "sky/skydefs.h"

#define	GRID_FILE_START	60000

int8 gridConvertTable[] = {

	1,	//1
	2,	//2
	3,	//3
	4,	//4
	5,	//5
	6,	//6
	7,	//7
	8,	//8
	9,	//9
	10,	//10
	11,	//11
	12,	//12
	13,	//13
	14,	//14
	15,	//15
	16,	//16
	17,	//17
	18,	//18
	19,	//19
	20,	//20
	21,	//21
	22,	//22
	23,	//23
	24,	//24
	25,	//25
	26,	//26
	27,	//27
	28,	//28
	29,	//29
	30,	//30
	31,	//31
	32,	//32
	33,	//33
	34,	//34
	-1,	//35
	35,	//36
	36,	//37
	37,	//38
	38,	//39
	39,	//40
	40,	//41
	41,	//42
	-1,	//43
	42,	//44
	43,	//45
	44,	//46
	45,	//47
	46,	//48
	-1,	//49
	-1,	//50
	-1,	//51
	-1,	//52
	-1,	//53
	-1,	//54
	-1,	//55
	-1,	//56
	-1,	//57
	-1,	//58
	-1,	//59
	-1,	//60
	-1,	//61
	-1,	//62
	-1,	//63
	-1,	//64
	47,	//65
	TOT_NO_GRIDS,	//66
	48,	//67
	49,	//68
	50,	//69
	51,	//70
	52,	//71
	53,	//72
	54,	//73
	55,	//74
	56,	//75
	57,	//76
	58,	//77
	59,	//78
	60,	//79
	-1,	//80
	61,	//81
	62,	//82
	-1,	//83
	-1,	//84
	-1,	//85
	-1,	//86
	-1,	//87
	-1,	//88
	TOT_NO_GRIDS,	//89
	63,	//90
	64,	//91
	65,	//92
	66,	//93
	67,	//94
	68,	//95
	69,	//96
};

void SkyState::initialiseGrids() {

	_gameGrids = (byte *)malloc(TOT_NO_GRIDS * GRID_SIZE);
}

