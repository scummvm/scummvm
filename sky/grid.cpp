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

#include "sky/compact.h"
#include "sky/disk.h"
#include "sky/grid.h"
#include "sky/logic.h"

#define	GRID_FILE_START	60000

int8 SkyGrid::_gridConvertTable[] = {

	0,	//0
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

SkyGrid::SkyGrid(SkyDisk *pDisk) {

	_gameGrids = (uint8 *)malloc(TOT_NO_GRIDS * GRID_SIZE);
	_skyDisk = pDisk;
}

SkyGrid::~SkyGrid(void) {

	free(_gameGrids);
}

void SkyGrid::loadGrids(void) {

	// no endian conversion necessary as I'm using uint8* instead of uint32*
	for (uint8 cnt = 0; cnt < TOT_NO_GRIDS; cnt++)
		_skyDisk->loadFile(GRID_FILE_START + cnt, _gameGrids + (cnt * GRID_SIZE));
	if (!SkyEngine::isDemo()) { // single disk demos never get that far
		// Reloading the grids can sometimes cause problems eg when reichs door is
		// open the door grid bit gets replaced so you can't get back in (or out)
		if (SkyLogic::_scriptVariables[REICH_DOOR_FLAG])
			removeGrid(256, 280, 1, &SkyCompact::reich_door_20);
	}
}

bool SkyGrid::getGridValues(Compact *cpt, uint32 *resBitNum, uint32 *resWidth) {

	uint16 width = SkyCompact::getMegaSet(cpt, cpt->extCompact->megaSet)->gridWidth;
	return getGridValues(cpt->xcood, cpt->ycood, width, cpt, resBitNum, resWidth);
}

bool SkyGrid::getGridValues(uint32 x, uint32 y, uint32 width, Compact *cpt, uint32 *resBitNum, uint32 *resWidth) {

	uint32 bitPos;
	if (y < TOP_LEFT_Y) return false; // off screen
	y -= TOP_LEFT_Y;
	y >>= 3; // convert to blocks
	if (y >= GAME_SCREEN_HEIGHT >> 3) return false; // off screen
	bitPos = y * 40;
	width++;
	x >>= 3; // convert to blocks
	
	if (x < (TOP_LEFT_X >> 3)) { // at least partially off screen
		if (x + width < (TOP_LEFT_X >> 3)) return false; // completely off screen
		else {
			width -= (TOP_LEFT_X >> 3) - x;
			x = 0;
		}
	} else
		x -= TOP_LEFT_X >> 3;

	if ((GAME_SCREEN_WIDTH >> 3) <= x) return false; // off screen
	if ((GAME_SCREEN_WIDTH >> 3) < x + width) // partially off screen
		width = (GAME_SCREEN_WIDTH >> 3) - x;

	bitPos += x;
	int32 screenGridOfs = _gridConvertTable[cpt->screen] * GRID_SIZE;
	bitPos += (screenGridOfs << 3); // convert to bits
	uint32 tmpBits = 0x1F - (bitPos&0x1F);
	bitPos &= ~0x1F; // divide into dword address and bit number
	bitPos += tmpBits;
	*resBitNum = bitPos;
	*resWidth = width;
	return true;
}

void SkyGrid::removeObjectFromWalk(Compact *cpt) {

	uint32 bitNum, width;
	if (getGridValues(cpt, &bitNum, &width))
		removeObjectFromWalk(bitNum, width);
}

void SkyGrid::removeObjectFromWalk(uint32 bitNum, uint32 width) {

	for (uint32 cnt = 0; cnt < width; cnt++) {
		_gameGrids[bitNum >> 3] &= ~(1 << (bitNum & 0x7));
		if ((bitNum & 0x1F) == 0)
			bitNum += 0x3F;
		else bitNum--;
	}
}

void SkyGrid::objectToWalk(Compact *cpt) {

	uint32 bitNum, width;
	if (getGridValues(cpt, &bitNum, &width))
		objectToWalk(bitNum, width);
}

void SkyGrid::objectToWalk(uint32 bitNum, uint32 width) {

	for (uint32 cnt = 0; cnt < width; cnt++) {
		_gameGrids[bitNum >> 3] |= (1 << (bitNum & 0x7));
		if ((bitNum & 0x1F) == 0)
			bitNum += 0x3F;
		else bitNum--;
	}
}

void SkyGrid::plotGrid(uint32 x, uint32 y, uint32 width, Compact *cpt) {

	uint32 resBitPos, resWidth;
	if (getGridValues(x, y, width-1, cpt, &resBitPos, &resWidth))
		objectToWalk(resBitPos, resWidth);
}

void SkyGrid::removeGrid(uint32 x, uint32 y, uint32 width, Compact *cpt) {

	uint32 resBitPos, resWidth;
	if (getGridValues(x, y, width, cpt, &resBitPos, &resWidth))
		removeObjectFromWalk(resBitPos, resWidth);
}

uint8 *SkyGrid::giveGrid(uint32 pScreen)
{
	return _gameGrids + GRID_SIZE * _gridConvertTable[pScreen];
}
