/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

#ifndef KYRASCREEN_H
#define KYRASCREEN_H

#include "common/util.h"
#include <stdarg.h>

class OSystem;

namespace Kyra {

class KyraEngine;
class Debugger;
struct Rect;

struct ScreenDim {
	uint16 sx;
	uint16 sy;
	uint16 w;
	uint16 h;
	uint16 unk8;
	uint16 unkA;
	uint16 unkC;
	uint16 unkE;
};

struct Font {
	uint8 *fontData;
	uint8 *charWidthTable;
	uint16 charBoxHeight;
	uint16 charBitmapOffset;
	uint16 charWidthTableOffset;
	uint16 charHeightTableOffset;
};

class Screen {
	friend class Debugger;
public:

	enum {
		SCREEN_W = 320,
		SCREEN_H = 200,
		SCREEN_PAGE_SIZE = 320 * 200 + 1024,
		SCREEN_PAGE_NUM  = 16
	};

	enum CopyRegionFlags {
		CR_X_FLIPPED  = 0x01,
		CR_CLIPPED    = 0x02
	};

	enum DrawShapeFlags {
		DSF_X_FLIPPED  = 0x01,
		DSF_Y_FLIPPED  = 0x02,
		DSF_SCALE      = 0x04,
		DSF_WND_COORDS = 0x10,
		DSF_CENTER     = 0x20
	};
	
	enum FontId {
		FID_6_FNT = 0,
		FID_8_FNT,
		FID_CRED6_FNT,
		FID_CRED8_FNT,
		FID_NUM
	};
	
	Screen(KyraEngine *vm, OSystem *system);
	~Screen();

	void updateScreen();
	uint8 *getPagePtr(int pageNum);
	void clearPage(int pageNum);
	int setCurPage(int pageNum);
	void clearCurPage();
	uint8 getPagePixel(int pageNum, int x, int y);
	void setPagePixel(int pageNum, int x, int y, uint8 color);
	void fadeFromBlack();
	void fadeToBlack();
	void fadeSpecialPalette(int palIndex, int startIndex, int size, int fadeTime);
	void fadePalette(const uint8 *palData, int delay);
	void setScreenPalette(const uint8 *palData);
	void copyToPage0(int y, int h, uint8 page, uint8 *seqBuf);
	void copyRegion(int x1, int y1, int x2, int y2, int w, int h, int srcPage, int dstPage, int flags=0);
	void copyBlockToPage(int pageNum, int x, int y, int w, int h, const uint8 *src);
	void copyFromCurPageBlock(int x, int y, int w, int h, const uint8 *src);
	void copyCurPageBlock(int x, int y, int w, int h, uint8 *dst);
	void shuffleScreen(int sx, int sy, int w, int h, int srcPage, int dstPage, int ticks, bool transparent);
	void fillRect(int x1, int y1, int x2, int y2, uint8 color, int pageNum = -1);
	void drawLine(bool horizontal, int x, int y, int length, int color);
	void drawClippedLine(int x1, int y1, int x2, int y2, int color);
	void drawBox(int x1, int y1, int x2, int y2, int color1, int color2);
	void setAnimBlockPtr(int size);
	void setTextColorMap(const uint8 *cmap);
	void setTextColor(const uint8 *cmap, int a, int b);
	void loadFont(FontId fontId, uint8 *fontData);
	FontId setFont(FontId fontId);
	int getCharWidth(uint8 c) const;
	int getTextWidth(const char *str) const;
	void printText(const char *str, int x, int y, uint8 color1, uint8 color2);
	void drawChar(uint8 c, int x, int y);
	void setScreenDim(int dim);
	void drawShapePlotPixelCallback1(uint8 *dst, uint8 color);
	void drawShape(uint8 pageNum, const uint8 *shapeData, int x, int y, int sd, int flags, ...);
	static void decodeFrame3(const uint8 *src, uint8 *dst, uint32 size);
	static void decodeFrame4(const uint8 *src, uint8 *dst, uint32 dstSize);
	static void decodeFrameDelta(uint8 *dst, const uint8 *src);
	static void decodeFrameDeltaPage(uint8 *dst, const uint8 *src, int pitch, int noXor);
	uint8 *encodeShape(int x, int y, int w, int h, int flags);
	void copyRegionToBuffer(int pageNum, int x, int y, int w, int h, uint8 *dest);
		
	int getRectSize(int x, int y);
	void hideMouse();
	void showMouse();
	void setShapePages(int page1, int page2);
	void setMouseCursor(int x, int y, byte *shape);
	uint8 *getPalette(int num);
	
	byte getShapeFlag1(int x, int y);
	byte getShapeFlag2(int x, int y);
	int setNewShapeHeight(uint8 *shape, int height);
	int resetShapeHeight(uint8 *shape);
	
	void addBitBlitRect(int x, int y, int w, int h);
	void bitBlitRects();
	
	void savePageToDisk(const char *file, int page);
	void loadPageFromDisk(const char *file, int page);

	int _charWidth;
	int _charOffset;
	int _curPage;
	uint8 *_currentPalette;
	uint8 *_shapePages[2];

	const ScreenDim *_curDim;
	
	static const ScreenDim _screenDimTable[];
	static const int _screenDimTableCount;
private:
	int16 encodeShapeAndCalculateSize(uint8 *from, uint8 *to, int size);
	void restoreMouseRect();
	void copyMouseToScreen();
	void copyScreenFromRect(int x, int y, int w, int h, uint8 *ptr);
	void copyScreenToRect(int x, int y, int w, int h, uint8 *ptr);

	uint8 *_pagePtrs[16];
	uint8 *_saveLoadPage[8];
	uint8 *_screenPalette;
	uint8 *_palettes[3];
	FontId _currentFont;
	Font _fonts[FID_NUM];
	uint8 _textColorsMap[16];
	uint8 *_decodeShapeBuffer;
	int _decodeShapeBufferSize;
	uint8 *_animBlockPtr;
	int _animBlockSize;
	int _mouseLockCount;
	
	Rect *_bitBlitRects;
	int _bitBlitNum;

	OSystem *_system;
	KyraEngine *_vm;
};

} // End of namespace Kyra

#endif
