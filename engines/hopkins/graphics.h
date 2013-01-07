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

class HopkinsEngine;

class GraphicsManager {
private:
	HopkinsEngine *_vm;

	void loadScreen(const Common::String &file);
public:
	int _lockCounter;
	bool SDL_MODEYES;
	int XSCREEN;
	int YSCREEN;
	int WinScan;
	byte SD_PIXELS[PALETTE_SIZE * 2];
	byte *PAL_PIXELS;
	int _lineNbr;
	byte _colorTable[PALETTE_EXT_BLOCK_SIZE];
	byte _palette[PALETTE_EXT_BLOCK_SIZE];
	byte _oldPalette[PALETTE_EXT_BLOCK_SIZE];
	Graphics::Surface *_videoPtr;
	byte *_vesaScreen;
	byte *_vesaBuffer;
	int _scrollOffset;
	int SCROLL;
	byte HEADER_PCX[128];
	bool _largeScreenFl;
	int OLD_SCROLL;
	bool MANU_SCROLL;
	int _scrollSpeed;
	int _lineNbr2;
	int Agr_x, Agr_y;
	int Agr_Flag_x, Agr_Flag_y;
	int _fadeDefaultSpeed;
	int FADE_LINUX;
	bool _skipVideoLockFl;
	int no_scroll;
	Common::Rect dstrect[50];
	int min_x, min_y;
	int max_x, max_y;
	int clip_x, clip_y;
	int clip_x1, clip_y1;
	bool clip_flag;
	int Red_x, Red_y;
	int Red;
	int _width;
	int Compteur_y;
	int spec_largeur;
	bool _noFadingFl;
public:
	GraphicsManager();
	~GraphicsManager();
	void setParent(HopkinsEngine *vm);

	void setGraphicalMode(int width, int height);
	void lockScreen();
	void unlockScreen();
	void clearScreen();
	void loadImage(const Common::String &file);
	void loadVgaImage(const Common::String &file);
	void initColorTable(int minIndex, int maxIndex, byte *palette);
	void scrollScreen(int amount);
	void Trans_bloc(byte *destP, const byte *srcP, int count, int minThreshold, int maxThreshold);
	void Trans_bloc2(byte *surface, byte *col, int size);
	void A_PCX640_480(byte *surface, const Common::String &file, byte *palette, bool typeFlag);
	void A_PCX320(byte *surface, const Common::String &file, byte *palette);
	void clearPalette();
	void SCANLINE(int pitch);
	void m_scroll16(const byte *surface, int xs, int ys, int width, int height, int destX, int destY);
	void m_scroll16A(const byte *surface, int xs, int ys, int width, int height, int destX, int destY);
	void Copy_Vga16(const byte *surface, int xp, int yp, int width, int height, int destX, int destY);
	void fadeIn(const byte *palette, int step, const byte *surface);
	void fadeOut(const byte *palette, int step, const byte *surface);
	void fadeInShort();
	void fadeOutShort();
	void fadeInLong();
	void fadeOutLong();
	void fadeOutDefaultLength(const byte *surface);
	void fadeInDefaultLength(const byte *surface);
	void fadeInBreakout();
	void fateOutBreakout();
	void setPaletteVGA256(const byte *palette);
	void setPaletteVGA256WithRefresh(const byte *palette, const byte *surface);
	void SETCOLOR3(int palIndex, int r, int g, int b);
	void SETCOLOR4(int palIndex, int r, int g, int b);
	void changePalette(const byte *palette);
	uint16 mapRGB(byte r, byte g, byte b);
	void DD_VBL();
	void Copy_WinScan_Vbe3(const byte *srcData, byte *destSurface);
	void Copy_Video_Vbe16(const byte *srcData);
	void Copy_Video_Vbe16a(const byte *srcData);
	void Capture_Mem(const byte *srcSurface, byte *destSurface, int xs, int ys, int width, int height);

	/**
	 * Draws a sprite onto the screen
	 * @param surface		Destination surface
	 * @param spriteData	The raw data for a sprite set
	 * @param xp			X co-ordinate. For some reason, starts from 300 = first column
	 * @param yp			Y co-ordinate. FOr some reason, starts from 300 = top row
	 * @param spriteIndex	Index of the sprite to draw
	 */
	void Sprite_Vesa(byte *surface, const byte *spriteData, int xp, int yp, int spriteIndex);

	void FIN_VISU();
	void VISU_ALL();
	void RESET_SEGMENT_VESA();
	void addVesaSegment(int x1, int y1, int x2, int y2);
	void displayVesaSegment();
	void AFFICHE_SPEEDVGA(const byte *objectData, int xp, int yp, int idx);
	void CopyAsm(const byte *surface);
	void copy16bFromSurfaceScaleX2(const byte *surface);
	void Restore_Mem(byte *destSurface, const byte *src, int xp, int yp, int width, int height);
	int zoomIn(int v, int percentage);
	int zoomOut(int v, int percentage);
	void Affiche_Perfect(byte *surface, const byte *srcData, int xp300, int yp300, int frameIndex, int zoom1, int zoom2, int modeFlag);
	void fastDisplay(const byte *spriteData, int xp, int yp, int spriteIndex);
	void SCOPY(const byte *surface, int x1, int y1, int width, int height, byte *destSurface, int destX, int destY);
	void Copy_Mem(const byte *srcSurface, int x1, int y1, unsigned int width, int height, byte *destSurface, int destX, int destY);
	void displayFont(byte *surface, const byte *spriteData, int xp, int yp, int characterIndex, int colour);
	void INI_ECRAN(const Common::String &file);
	void INI_ECRAN2(const Common::String &file);
	void OPTI_INI(const Common::String &file, int mode);
	void NB_SCREEN(bool initPalette);
	void SHOW_PALETTE();
	void Copy_WinScan_Vbe(const byte *srcP, byte *destP);
	void Copy_Video_Vbe(const byte *src);
	void Reduc_Ecran(const byte *srcSruface, byte *destSurface, int xp, int yp, int width, int height, int zoom);
	void drawHorizontalLine(byte *surface, int xp, int yp, unsigned int width, byte col);
	void drawVerticalLine(byte *surface, int xp, int yp, int height, byte col);
};

} // End of namespace Hopkins

#endif /* HOPKINS_GRAPHICS_H */
