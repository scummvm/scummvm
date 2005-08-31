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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#ifndef KYRASCREEN_H
#define KYRASCREEN_H

#include "common/util.h"

class OSystem;

namespace Kyra {

class CPSImage;
class KyraEngine;

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

class Screen {
public:

	Screen(KyraEngine *vm, OSystem *system);
	~Screen();

	void updateScreen();
	uint8 *getPagePtr(int pageNum);
	void clearPage(int pageNum);
	int setCurPage(int pageNum);
	void clearCurPage();
	void fadeFromBlack();
	void fadeToBlack();
	void setScreenPalette(const uint8 *palData);
	void copyToPage0(int y, int h, uint8 page, uint8 *seqBuf);
	void copyRegion(int x1, int y1, int x2, int y2, int w, int h, int srcPage, int dstPage);
	void copyBlockToPage(int pageNum, int x, int y, int w, int h, const uint8 *src);
	void fillRect(int x1, int y1, int x2, int y2, uint8 color, int pageNum = -1);
	void setAnimBlockPtr(uint8 *p, int size);
	void setTextColorMap(const uint8 *cmap);
	void setTextColor(const uint8 *cmap, int a, int b);
	int getCharWidth(uint8 c) const;
	int getTextWidth(const char *str) const;
	void printText(const char *str, int x, int y, uint8 color1, uint8 color2);
	void setScreenDim(int dim);
	void drawShapePlotPixelCallback1(uint8 *dst, uint8 color);
	void drawShape(uint8 pageNum, const uint8 *shapeData, int x, int y, int sd, int flags, int *flagsTable);
	static void decodeFrame3(const uint8 *src, uint8 *dst, uint32 size);
	static void decodeFrame4(const uint8 *src, uint8 *dst, uint32 dstSize);
	static void decodeFrameDelta(uint8 *dst, const uint8 *src);
	static void decodeFrameDeltaPage(uint8 *dst, const uint8 *src, int pitch);
	
	int _charOffset;
	int _charWidth;
	int _curPage;
	uint8 *_palette1;

	enum {
		SCREEN_W = 320,
		SCREEN_H = 200,
		SCREEN_PAGE_SIZE = 320 * 200 + 1024,
		SCREEN_PAGE_NUM  = 16
	};

	enum DrawShapeFlags {
		DSF_X_FLIPPED  = 0x01,
		DSF_Y_FLIPPED  = 0x02,
		DSF_SCALE      = 0x04,
		DSF_WND_COORDS = 0x10,
		DSF_CENTER     = 0x20
	};
	
	typedef void (Screen::*DrawShapePlotPixelCallback)(uint8 *dst, uint8 c);

private:

	uint8 _textColorsMap[16];
	uint8 *_animBlockPtr;
	int _animBlockSize;
	uint8 *_pagePtrs[16];
	uint8 *_palette3;
	uint8 *_fadePalette;
	const ScreenDim *_curDim;
	uint8 *_decodeShapeBuffer;
	int _decodeShapeBufferSize;
	
	OSystem *_system;
	KyraEngine *_vm;
	
	static const ScreenDim _screenDimTable[];
	static const DrawShapePlotPixelCallback _drawShapePlotPixelTable[];
	static const int _drawShapePlotPixelCount;
};

} // End of namespace Kyra

#endif
