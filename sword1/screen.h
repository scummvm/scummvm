/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2005 The ScummVM project
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

#ifndef BSSCREEN_H
#define BSSCREEN_H

#include "sword1/sworddefs.h"

class OSystem;

namespace Sword1 {

#define MAX_FORE 20
#define MAX_BACK 20
#define MAX_SORT 20

struct SortSpr {
	int32 id, y;
};

struct RoomDef {
	int		totalLayers;
	int		sizeX;
	int		sizeY;
	int		gridWidth;	//number of 16*16 grid blocks across - including off screen edges.
	uint32	layers[4];
	uint32	grids[3];
	uint32	palettes[2];
	uint32	parallax[2];
};

#define SCRNGRID_X 16
#define SCRNGRID_Y 8
#define SHRINK_BUFFER_SIZE 50000
#define RLE_BUFFER_SIZE 50000

#define FLASH_RED 0
#define FLASH_BLUE 1
#define BORDER_YELLOW 2
#define BORDER_GREEN 3
#define BORDER_PURPLE 4
#define BORDER_BLACK 5

class ResMan;
class ObjectMan;
class Text; // Text objects use sprites that are created internally at run-time
				 // the buffer belongs to Text, so we need a reference here.

class Screen {
public:
	Screen(OSystem *system, ResMan *pResMan, ObjectMan *pObjMan);
	~Screen(void);
	void useTextManager(Text *pTextMan);
	void draw(void);

	void quitScreen(void);
	void newScreen(uint32 screen);

	void setScrolling(int16 offsetX, int16 offsetY);
	void addToGraphicList(uint8 listId, uint32 objId);

	void fadeDownPalette(void);
	void fadeUpPalette(void);
	void fnSetPalette(uint8 start, uint16 length, uint32 id, bool fadeUp);
	bool stillFading(void);
	void fullRefresh(void);

	bool showScrollFrame(void);
	void updateScreen(void);
	void showFrame(uint16 x, uint16 y, uint32 resId, uint32 frameNo, const byte *fadeMask = NULL, int8 fadeStatus = 0);

	void fnSetParallax(uint32 screen, uint32 resId);
	void fnFlash(uint8 color);
	void fnBorder(uint8 color);

#ifdef BACKEND_8BIT
	void plotYUV(byte *lut, int width, int height, byte *const *dat);
#endif

private:
	// for router debugging
	void drawLine(uint16 x1, uint16 y1, uint16 x2, uint16 y2);
	void vline(uint16 x, uint16 y1, uint16 y2);
	void hline(uint16 x1, uint16 x2, uint16 y);
	void bsubline_1(uint16 x1, uint16 y1, uint16 x2, uint16 y2);
	void bsubline_2(uint16 x1, uint16 y1, uint16 x2, uint16 y2);
	void bsubline_3(uint16 x1, uint16 y1, uint16 x2, uint16 y2);
	void bsubline_4(uint16 x1, uint16 y1, uint16 x2, uint16 y2);

	void verticalMask(uint16 x, uint16 y, uint16 bWidth, uint16 bHeight);
	void blitBlockClear(uint16 x, uint16 y, uint8 *data);
	void renderParallax(uint8 *data);
	void processImage(uint32 id);
	void spriteClipAndSet(uint16 *pSprX, uint16 *pSprY, uint16 *sprWidth, uint16 *sprHeight, uint16 *incr);
	void drawSprite(uint8 *sprData, uint16 sprX, uint16 sprY, uint16 sprWidth, uint16 sprHeight, uint16 sprPitch);
	void decompressRLE7(uint8 *src, uint32 compSize, uint8 *dest);
	void decompressRLE0(uint8 *src, uint32 compSize, uint8 *dest);
	void decompressTony(uint8 *src, uint32 compSize, uint8 *dest);
	void fastShrink(uint8 *src, uint32 width, uint32 height, uint32 scale, uint8 *dest);
	int32 inRange(int32 a, int32 b, int32 c);
	void fadePalette(void);

	OSystem *_system;
	ResMan *_resMan;
	ObjectMan *_objMan;
	Text *_textMan;

	uint16 _currentScreen;
	uint8  *_screenBuf;
	uint8  *_screenGrid;
	uint16 *_layerGrid[4];
	uint8  *_layerBlocks[4];
	uint8  *_parallax[2];
	uint8  _rleBuffer[RLE_BUFFER_SIZE];
	uint8  _shrinkBuffer[SHRINK_BUFFER_SIZE];
	bool   _fullRefresh;
	uint16 _oldScrollX, _oldScrollY; // for drawing additional frames

	uint32  _foreList[MAX_FORE];
	uint32  _backList[MAX_BACK];
	SortSpr _sortList[MAX_SORT];
	uint8   _foreLength, _backLength, _sortLength;
	uint16  _scrnSizeX, _scrnSizeY, _gridSizeX, _gridSizeY;

	static RoomDef _roomDefTable[TOTAL_ROOMS]; // from ROOMS.C (not const, see fnSetParallax)

	uint8 _targetPalette[256 * 4];
	uint8 _currentPalette[256 * 4]; // for fading
	uint8 _fadingStep;
	int8  _fadingDirection; // 1 for fade up, -1 for fade down
	bool _isBlack; // if the logic already faded down the palette, this is set to show the
				   // mainloop that no further fading is necessary.
};

} // End of namespace Sword1

#endif //BSSCREEN_H

