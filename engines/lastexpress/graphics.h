/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef LASTEXPRESS_GRAPHICS_H
#define LASTEXPRESS_GRAPHICS_H

#include "lastexpress/lastexpress.h"

#include "graphics/surface.h"

namespace LastExpress {

typedef struct TBM {
	int32 x;
	int32 y;
	int32 width;
	int32 height;

	TBM() {
		x = 0;
		y = 0;
		width = 0;
		height = 0;
	}

	TBM(int32 newX, int32 newY, int32 newWidth, int32 newHeight) {
		x = newX;
		y = newY;
		width = newWidth;
		height = newHeight;
	}

} TBM;

#include "common/pack-start.h"
typedef struct CursorHeader {
	int16 hotspotX;
	int16 hotspotY;
} CursorHeader;
#include "common/pack-end.h"

typedef uint16 PixMap;

struct Sprite;

class GraphicsManager {
	friend class CFight;
	friend class MemoryManager;
	friend class LastExpressEngine;
	friend class LogicManager;
	friend class SubtitleManager;
	friend class ArchiveManager;
	friend class GoldArchiveManager;
	friend class NISManager;
	friend class Menu;
	friend class SpriteManager;

public:
	GraphicsManager(LastExpressEngine *engine);
	~GraphicsManager();

	bool acquireSurface();
	void unlockSurface();
	void burstAll();

	void stepBG(int sceneIndex);
	void goStepBG(int sceneIndex);
	void stepDissolve(TBM *tbm);
	void clear(Graphics::Surface &surface, int32 x, int32 y, int32 width, int32 height);
	void clear(PixMap *pixels, int32 x, int32 y, int32 width, int32 height);
	void copy(PixMap *src, PixMap *dst, int32 x, int32 y, int32 width, int32 height);
	void goDissolve(int32 location, int32 width, int32 height, PixMap *buffer);
	void bitBltSprite255(Sprite *sprite, PixMap *pixels);
	void bitBltSprite128(Sprite *sprite, PixMap *pixels);
	void bitBltSprite32(Sprite *sprite, PixMap *pixels);
	void bitBltSprite16(Sprite *sprite, PixMap *pixels);
	void bitBltSprite8(Sprite *sprite, PixMap *pixels);
	void bitBltWane128(Sprite *sprite, PixMap *pixels);
	void bitBltWane32(Sprite *sprite, PixMap *pixels);
	void bitBltWane16(Sprite *sprite, PixMap *pixels);
	void bitBltWane8(Sprite *sprite, PixMap *pixels);
	void bitBltWax128(Sprite *sprite, PixMap *pixels);
	void bitBltWax32(Sprite *sprite, PixMap *pixels);
	void bitBltWax16(Sprite *sprite, PixMap *pixels);
	void bitBltWax8(Sprite *sprite, PixMap *pixels);
	void eraseSprite(byte *data);
	void dissolve(int32 location, int32 width, int32 height, PixMap *buf);
	void doErase(byte *data);

	// PALETTE
	void modifyPalette(uint16 *data, uint32 size);
	int getGammaLevel();
	void setGammaLevel(int level);
	void initLuminosityValues(int16 rMask, int16 gMask, int16 bMask);
	void initDecomp(PixMap *data, TBM *tbm);
	void decompR(byte *data, int32 size);
	void decompG(byte *data, int32 size);
	void decompB(byte *data, int32 size);
	bool decomp16(byte *data, int32 size);

	// CURSOR HANDLING
	void drawItem(int cursor, int32 x, int32 y);
	void drawItemDim(int cursor, int32 x, int32 y, int brightness);
	void drawMouse();
	void storeMouse();
	void restoreMouse();
	void newMouseLoc();
	void burstMouseArea(bool updateScreen = true);
	void burstBox(int32 x, int32 y, int32 width, int32 height, bool updateScreen = true);
	bool canDrawMouse();
	void setMouseDrawable(bool drawable);

protected:
	LastExpressEngine *_engine = nullptr;
	PixMap *_frontBuffer = nullptr;
	PixMap *_backBuffer = nullptr;
	PixMap _mouseBackBuffer[32 * 32];

	byte *_backgroundCompBuffer = nullptr;
	PixMap _subtitlesBackBuffer[36480];
	Graphics::Surface _screenSurface;
	bool _mainSurfaceIsInit = false;

	TBM _renderBox1;
	TBM _renderBox2;
	int _gammaLevel = 3;
	int32 _dissolveSteps = 3;

	bool _luminosityIsInit = false;

	uint16 _brightnessData[6] = {
		0x0000, 0x7BDE, 0x739C,
		0x6318, 0x4210, 0x0000
	};

	int16 _brightnessMaskedBits = 0;
	int16 _brightnessUnmaskedBits = 0;

	TBM *_bgDecompTargetRect = nullptr;
	byte *_bgDecompOutBuffer = nullptr;
	byte *_bgDecompOutBufferTemp = nullptr;
	int32 _bgDecompFlags = 0;
	byte _bgLastCompItem = 0;
	int32 _bgOutChannelDataSizes[3] = { 0, 0, 0 };

	// For the Wax&Wane compression
	PixMap *_frameData = nullptr;
	PixMap *_decompressedOffset = nullptr;
	int _numBlanks = 0;

	bool _disableCharacterDrawing = false;
	bool _stepBGRecursionFlag = false;
	bool _shouldDrawEggOrHourglass = false;

	int32 _mouseScreenBufStart = 0;
	TBM _mouseAreaRect = TBM(320, 240, 32, 32);
	bool _canDrawMouse = false;

	CursorHeader *_cursorsDataHeader = nullptr;
	PixMap *_iconsBitmapData = nullptr;
	uint8 _luminosityData[32 * 7] = {
		0,  1,  1,  2,  3,  4,  4,  5,  6,  7,  8,  8,  9, 10, 11, 12, 13, 13, 14, 15, 16, 17, 18, 19, 20, 21, 23, 24, 25, 27, 28, 31,
		0,  1,  2,  2,  3,  4,  5,  6,  6,  7,  8,  9, 10, 11, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 25, 26, 27, 29, 31,
		0,  1,  2,  3,  3,  4,  5,  6,  7,  8,  9,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 27, 28, 29, 31,
		0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
		0,  2,  3,  4,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 22, 23, 24, 25, 26, 27, 28, 28, 29, 30, 31,
		0,  2,  4,  5,  6,  8,  9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 20, 21, 22, 23, 24, 25, 25, 26, 27, 28, 29, 29, 30, 31,
		0,  3,  4,  6,  7,  8, 10, 11, 12, 13, 14, 15, 16, 17, 18, 18, 19, 20, 21, 22, 23, 23, 24, 25, 26, 27, 27, 28, 29, 30, 30, 31
	};
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_GRAPHICS_H
