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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef HOPKINS_GRAPHICS_H
#define HOPKINS_GRAPHICS_H

#include "common/scummsys.h"
#include "common/array.h"
#include "common/endian.h"
#include "common/rect.h"
#include "common/str.h"
#include "graphics/surface.h"

namespace Hopkins {

#define DIRTY_RECTS_SIZE 250
#define PALETTE_SIZE 256
#define PALETTE_BLOCK_SIZE (PALETTE_SIZE * 3)
#define PALETTE_EXT_BLOCK_SIZE 800
static const byte kSetOffset = 251;
static const byte kByteStop = 252;
static const byte k8bVal = 253;
static const byte k16bVal = 254;

struct RGB8 {
	byte r;
	byte g;
	byte b;
};

class HopkinsEngine;

class GraphicsManager {
private:
	HopkinsEngine *_vm;

	int _lockCounter;
	bool _initGraphicsFl;
	int _screenWidth;
	int _screenHeight;
	byte *_videoPtr;
	int _width;
	int _posXClipped, _posYClipped;
	bool _clipFl;
	int _specialWidth;

	int _enlargedX, _enlargedY;
	bool _enlargedXFl, _enlargedYFl;
	int _clipX1, _clipY1;
	int _reduceX, _reducedY;
	int _zoomOutFactor;

	bool _manualScroll;

	void loadScreen(const Common::String &file);
	void loadPCX640(byte *surface, const Common::String &file, byte *palette, bool typeFlag);
	void loadPCX320(byte *surface, const Common::String &file, byte *palette);
	void fadeIn(const byte *palette, int step, const byte *surface);
	void fadeOut(const byte *palette, int step, const byte *surface);
	void changePalette(const byte *palette);
	uint16 mapRGB(byte r, byte g, byte b);
	void copy16bFromSurfaceScaleX2(const byte *surface);

	void translateSurface(byte *destP, const byte *srcP, int count, int minThreshold, int maxThreshold);
	void displayScaled8BitRect(const byte *surface, int xp, int yp, int width, int height, int destX, int destY);

	void lockScreen();
	void unlockScreen();
public:
	byte _paletteBuffer[PALETTE_SIZE * 2];
	byte _colorTable[PALETTE_EXT_BLOCK_SIZE];
	byte _palette[PALETTE_EXT_BLOCK_SIZE];
	byte _oldPalette[PALETTE_EXT_BLOCK_SIZE];
	byte *_backBuffer;
	byte *_frontBuffer;
	byte *_screenBuffer;
	byte *_backupScreen;
	bool _largeScreenFl;
	bool _noFadingFl;
	bool _fadingFl;
	bool _skipVideoLockFl;
	int _scrollOffset;
	int _scrollPosX;
	int _oldScrollPosX;
	int _scrollSpeed;
	int _lineNbr;
	int _lineNbr2;
	int _minX, _minY;
	int _maxX, _maxY;
	int _scrollStatus;
	int _fadeDefaultSpeed;
	int _screenLineSize;

	/**
	 * The _dirtyRects list contains paletted game areas that need to be redrawn.
	 * The _dstrect array is the list of areas of the screen that ScummVM needs to be redrawn.
	 * Some areas, such as the animation managers, skip the _dirtyRects and use _dstrec directly.
	 */
	Common::Array<Common::Rect> _dirtyRects;
	Common::Array<Common::Rect> _refreshRects;
	bool _showDirtyRects;
	bool _showZones;
	bool _showLines;

	byte *_palettePixels;
public:
	GraphicsManager(HopkinsEngine *vm);
	~GraphicsManager();

	void clearPalette();
	void clearScreen();
	void clearVesaScreen();
	void resetDirtyRects();
	void resetRefreshRects();
	void addDirtyRect(int x1, int y1, int x2, int y2);
	void addDirtyRect(const Common::Rect &r) { addDirtyRect(r.left, r.top, r.right, r.bottom); }
	void addRefreshRect(int x1, int y1, int x2, int y2);
	void addRectToArray(Common::Array<Common::Rect> &rects, const Common::Rect &newRect);
	void displayDirtyRects();
	void displayRefreshRects();
	void displayZones();
	void displayLines();
	void displayDebugRect(Graphics::Surface *surface, const Common::Rect &srcRect, uint32 color = 0xffffff);
	void copySurface(const byte *surface, int x1, int y1, int width, int height, byte *destSurface, int destX, int destY);
	void loadImage(const Common::String &file);
	void loadVgaImage(const Common::String &file);
	void fadeInLong();
	void fadeInBreakout();
	void fadeInDefaultLength(const byte *surface);
	void fadeInShort();
	void fadeOutDefaultLength(const byte *surface);
	void fadeOutBreakout();
	void fadeOutLong();
	void fadeOutShort();
	void copyWinscanVbe3(const byte *srcData, byte *destSurface);
	void copyWinscanVbe(const byte *srcP, byte *destP);
	void copyVideoVbe16(const byte *srcData);
	void copyVideoVbe16a(const byte *srcData);
	void copySurfaceRect(const byte *srcSurface, byte *destSurface, int xs, int ys, int width, int height);
	void restoreSurfaceRect(byte *destSurface, const byte *src, int xp, int yp, int width, int height);
	void displayFont(byte *surface, const byte *spriteData, int xp, int yp, int characterIndex, int color);
	void drawHorizontalLine(byte *surface, int xp, int yp, uint16 width, byte col);
	void drawVerticalLine(byte *surface, int xp, int yp, int height, byte col);
	void initColorTable(int minIndex, int maxIndex, byte *palette);
	void setGraphicalMode(int width, int height);
	void setPaletteVGA256(const byte *palette);
	void setPaletteVGA256WithRefresh(const byte *palette, const byte *surface);
	void scrollScreen(int amount);
	int zoomIn(int v, int percentage);
	int zoomOut(int v, int percentage);
	void initScreen(const Common::String &file, int mode, bool initializeScreen);
	void displayAllBob();
	void endDisplayBob();
	void updateScreen();
	void reduceScreenPart(const byte *srcSruface, byte *destSurface, int xp, int yp, int width, int height, int zoom);
	void setScreenWidth(int pitch);

	void setColorPercentage(int palIndex, int r, int g, int b);
	void setColorPercentage2(int palIndex, int r, int g, int b);
	void fastDisplay(const byte *spriteData, int xp, int yp, int spriteIndex, bool addSegment = true);
	void fastDisplay2(const byte *objectData, int xp, int yp, int idx, bool addSegment = true);
	void drawCompressedSprite(byte *surface, const byte *srcData, int xp300, int yp300, int frameIndex, int zoom1, int zoom2, bool flipFl);
	void copyRect(const byte *srcSurface, int x1, int y1, uint16 width, int height, byte *destSurface, int destX, int destY);
	void drawVesaSprite(byte *surface, const byte *spriteData, int xp, int yp, int spriteIndex);
	void display8BitRect(const byte *surface, int xs, int ys, int width, int height, int destX, int destY);
	void fillSurface(byte *surface, byte *col, int size);
	void displayScreen(bool initPalette);
	void backupScreen();
	void restoreScreen();
};

} // End of namespace Hopkins

#endif /* HOPKINS_GRAPHICS_H */
