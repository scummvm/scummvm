/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003 The ScummVM project
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef SKYSCREEN_H
#define SKYSCREEN_H

#include "stdafx.h"
#include <string.h>
#include "common/engine.h"
#include "common/scummsys.h"
#include "sky/disk.h"
#include "sky/skydefs.h"
#include "sky/sky.h"
#include "sky/logic.h"

#define SCROLL_JUMP		16
#define VGA_COLOURS		256
#define GAME_COLOURS		240
#define SEQ_DELAY 3

#define FORE 1
#define BACK 0

typedef struct {
	uint16 yCood;
	Compact *compact;
	dataFileHeader *sprite;
} StSortList;

class SkyState;

class SkyScreen {
public:
	SkyScreen(OSystem *pSystem, SkyDisk *pDisk);
	~SkyScreen(void);
	void setPalette(uint8 *pal);
	void setPalette(uint16 fileNum);
	void paletteFadeUp(uint8 *pal);
	void paletteFadeUp(uint16 fileNr);

	void showScreen(uint16 fileNum);
	void showScreen(uint8 *pScreen);
	
	void handleTimer(void);
	void startSequence(uint16 fileNum);
	void stopSequence(void);
	bool sequenceRunning(void);
	uint32 seqFramesLeft(void) { return _seqInfo.framesLeft; };
	uint8 *giveCurrent(void) { return _currentScreen; };
	void halvePalette(void);

	//- regular screen.asm routines
	void forceRefresh(void) { memset(_gameGrid, 0x80, GRID_X * GRID_Y); };
	void fnFadeUp(uint32 palNum, uint32 scroll);
	void fnFadeDown(uint32 scroll);
	void fnDrawScreen(uint32 palette, uint32 scroll);
	void clearScreen(void) { memset(_currentScreen, 0, FULL_SCREEN_WIDTH * FULL_SCREEN_HEIGHT); };

	void recreate(void);
	void flip(void);

	void spriteEngine(void);

private:
	OSystem *_system;
	SkyDisk *_skyDisk;
	static uint8 _top16Colours[16*3];
	uint8 _palette[1024];
	uint32 _currentPalette;

	bool volatile _gotTick;
	void waitForTimer(void);
	void processSequence(void);

	uint8 *_gameGrid;
	uint8 *_currentScreen;
	uint8 *_scrollScreen;
	uint8 *_backScreen;
	//uint32 *_scriptVariables;
	struct {
		uint32 framesLeft;
		uint32 delay;
		uint8 *seqData;
		uint8 *seqDataPos;
		bool running;
	} _seqInfo;
	//byte *_workScreen;
	//byte *_tempPal;
	//byte *_workPalette;
	//byte *_halfPalette;
	//byte *_scrollAddr;
	//byte *_lScreenBuf, *_lPaletteBuf;

    //- more regular screen.asm + layer.asm routines
	void convertPalette(uint8 *inPal, uint8* outPal);
	void palette_fadedown_helper(uint32 *pal, uint num);
	
	//- sprite.asm routines
	// fixme: get rid of these globals
	uint32 _sprWidth, _sprHeight, _sprX, _sprY, _maskX1, _maskX2;
	void doSprites(uint8 layer);
	void sortSprites(void);
	void drawSprite(uint8 *spriteData, Compact *sprCompact);
	void verticalMask(void);
	void vertMaskSub(uint16 *grid, uint32 gridOfs, uint8 *screenPtr, uint32 layerId);
	void vectorToGame(uint8 gridVal);
};

#endif //SKYSCREEN_H
