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
 */

#ifndef HOPKINS_GRAPHICS_H
#define HOPKINS_GRAPHICS_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/rect.h"
#include "common/str.h"
#include "graphics/surface.h"

namespace Hopkins {

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

struct BlocItem {
	uint16 _activeFl;
	int _x1;
	int _y1;
	int _x2;
	int _y2;
};

class HopkinsEngine;

class GraphicsManager {
private:
	HopkinsEngine *_vm;

	int _lockCounter;
	bool _initGraphicsFl;
	int _screenWidth;
	int _screenHeight;
	Graphics::Surface *_videoPtr;
	int _width;
	int _posXClipped, _posYClipped;
	bool _clipFl;
	int _specialWidth;

	byte SD_PIXELS[PALETTE_SIZE * 2];
	int Agr_x, Agr_y;
	bool Agr_Flag_x, Agr_Flag_y;
	int clip_x1, clip_y1;
	int Red_x, Red_y;
	int Red;

	void loadScreen(const Common::String &file);
	void loadPCX640(byte *surface, const Common::String &file, byte *palette, bool typeFlag);
	void loadPCX320(byte *surface, const Common::String &file, byte *palette);
	void fadeIn(const byte *palette, int step, const byte *surface);
	void fadeOut(const byte *palette, int step, const byte *surface);
	void changePalette(const byte *palette);
	uint16 mapRGB(byte r, byte g, byte b);

	void Trans_bloc(byte *destP, const byte *srcP, int count, int minThreshold, int maxThreshold);
	void Copy_Vga16(const byte *surface, int xp, int yp, int width, int height, int destX, int destY);
	void copy16bFromSurfaceScaleX2(const byte *surface);
public:
	int _lineNbr;
	byte _colorTable[PALETTE_EXT_BLOCK_SIZE];
	byte _palette[PALETTE_EXT_BLOCK_SIZE];
	byte _oldPalette[PALETTE_EXT_BLOCK_SIZE];
	byte *_vesaScreen;
	byte *_vesaBuffer;
	int _scrollOffset;
	int _scrollPosX;
	bool _largeScreenFl;
	int _oldScrollPosX;
	int _scrollSpeed;
	int _lineNbr2;
	int _minX, _minY;
	int _maxX, _maxY;
	bool _noFadingFl;
	Common::Rect dstrect[50];
	int _scrollStatus;
	bool _skipVideoLockFl;
	int _fadeDefaultSpeed;

	int NBBLOC;
	BlocItem BLOC[250];
	int WinScan;
	byte *PAL_PIXELS;
	bool MANU_SCROLL;
	int FADE_LINUX;
public:
	GraphicsManager();
	~GraphicsManager();

	void setParent(HopkinsEngine *vm);
	void lockScreen();
	void unlockScreen();
	void clearPalette();
	void clearScreen();
	void addVesaSegment(int x1, int y1, int x2, int y2);
	void copySurface(const byte *surface, int x1, int y1, int width, int height, byte *destSurface, int destX, int destY);
	void loadImage(const Common::String &file);
	void loadVgaImage(const Common::String &file);
	void fadeInLong();
	void fadeInBreakout();
	void fadeInDefaultLength(const byte *surface);
	void fadeInShort();
	void fadeOutDefaultLength(const byte *surface);
	void fateOutBreakout();
	void fadeOutLong();
	void fadeOutShort();
	void fastDisplay(const byte *spriteData, int xp, int yp, int spriteIndex, bool addSegment = true);
	void displayVesaSegment();
	void resetVesaSegment();
	void copyWinscanVbe3(const byte *srcData, byte *destSurface);
	void copyWinscanVbe(const byte *srcP, byte *destP);
	void copyVideoVbe16(const byte *srcData);
	void copyVideoVbe16a(const byte *srcData);
	void copySurfaceRect(const byte *srcSurface, byte *destSurface, int xs, int ys, int width, int height);
	void restoreSurfaceRect(byte *destSurface, const byte *src, int xp, int yp, int width, int height);
	void displayFont(byte *surface, const byte *spriteData, int xp, int yp, int characterIndex, int colour);
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

	void SETCOLOR3(int palIndex, int r, int g, int b);
	void SETCOLOR4(int palIndex, int r, int g, int b);
	void AFFICHE_SPEEDVGA(const byte *objectData, int xp, int yp, int idx, bool addSegment = true);
	void DD_VBL();
	void Affiche_Perfect(byte *surface, const byte *srcData, int xp300, int yp300, int frameIndex, int zoom1, int zoom2, bool flipFl);
	void Copy_Mem(const byte *srcSurface, int x1, int y1, uint16 width, int height, byte *destSurface, int destX, int destY);
	void SCANLINE(int pitch);
	void Sprite_Vesa(byte *surface, const byte *spriteData, int xp, int yp, int spriteIndex);
	void m_scroll16(const byte *surface, int xs, int ys, int width, int height, int destX, int destY);
	void m_scroll16A(const byte *surface, int xs, int ys, int width, int height, int destX, int destY);
	void Trans_bloc2(byte *surface, byte *col, int size);
	void NB_SCREEN(bool initPalette);
	void Reduc_Ecran(const byte *srcSruface, byte *destSurface, int xp, int yp, int width, int height, int zoom);
};

} // End of namespace Hopkins

#endif /* HOPKINS_GRAPHICS_H */
