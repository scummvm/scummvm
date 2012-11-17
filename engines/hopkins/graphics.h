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

	void CHARGE_ECRAN(const Common::String &file);
	int Asm_Zoom(int v, int percentage);
	int Asm_Reduc(int v, int percentage);
public:
	int _lockCtr;
	bool SDL_MODEYES;
	int SDL_ECHELLE;
	int XSCREEN;
	int YSCREEN;
	int WinScan;
	int Winbpp;
	byte SD_PIXELS[PALETTE_SIZE * 2];
	byte *PAL_PIXELS;
	int nbrligne;
	byte TABLE_COUL[PALETTE_EXT_BLOCK_SIZE];
	byte cmap[PALETTE_BLOCK_SIZE];
	byte Palette[PALETTE_EXT_BLOCK_SIZE];
	byte OLD_PAL[PALETTE_EXT_BLOCK_SIZE];
	bool Linear;
	Graphics::Surface *VideoPtr;
	byte *VESA_SCREEN;
	byte *VESA_BUFFER;
	int ofscroll;
	int SCROLL;
	byte HEADER_PCX[128];
	int PCX_L, PCX_H;
	bool DOUBLE_ECRAN;
	int OLD_SCROLL;
	int MANU_SCROLL;
	int SPEED_SCROLL;
	int nbrligne2;
	int Agr_x, Agr_y;
	int Agr_Flag_x, Agr_Flag_y;
	int FADESPD;
	byte PALPCX[PALETTE_EXT_BLOCK_SIZE];
	int FADE_LINUX;
	bool NOLOCK;
	int no_scroll;
	Common::Rect dstrect[50];
	int REDRAW;
	int min_x, min_y;
	int max_x, max_y;
	int clip_x, clip_y;
	int clip_x1, clip_y1;
	bool clip_flag;
	int SDL_NBLOCS;
	int Red_x, Red_y;
	int Red;
	int Largeur;
	int Compteur_y;
	int spec_largeur;
	bool NOFADE;
	int Reduc_Ecran_L, Reduc_Ecran_H;
public:
	GraphicsManager();
	~GraphicsManager();
	void setParent(HopkinsEngine *vm);

	void SET_MODE(int width, int height);
	void DD_Lock();
	void DD_Unlock();
	void DD_LOCK() { DD_Lock(); }
	void DD_UNLOCK() { DD_Unlock(); }
	void Cls_Video();
	void LOAD_IMAGE(const Common::String &file);
	void LOAD_IMAGEVGA(const Common::String &file);
	void INIT_TABLE(int minIndex, int maxIndex, byte *palette);
	int SCROLL_ECRAN(int amount);
	void Trans_bloc(byte *destP, const byte *srcP, int count, int minThreshold, int maxThreshold);
	void Trans_bloc2(byte *surface, byte *col, int size);
	void A_PCX640_480(byte *surface, const Common::String &file, byte *palette, bool typeFlag);
	void A_PCX320(byte *surface, const Common::String &file, byte *palette);
	void Cls_Pal();
	void SCANLINE(int pitch);
	void m_scroll(const byte *surface, int xs, int ys, int width, int height, int destX, int destY);
	void m_scroll2(const byte *surface, int xs, int ys, int width, int height, int destX, int destY);
	void m_scroll2A(const byte *surface, int xs, int ys, int width, int height, int destX, int destY);
	void m_scroll16(const byte *surface, int xs, int ys, int width, int height, int destX, int destY);
	void m_scroll16A(const byte *surface, int xs, int ys, int width, int height, int destX, int destY);
	void Copy_Vga(const byte *surface, int xp, int yp, int width, int height, int destX, int destY);
	void Copy_Vga16(const byte *surface, int xp, int yp, int width, int height, int destX, int destY);
	void fade_in(const byte *palette, int step, const byte *surface);
	void fade_out(const byte *palette, int step, const byte *surface);
	void FADE_INS();
	void FADE_OUTS();
	void FADE_INW();
	void FADE_OUTW();
	void FADE_OUTW_LINUX(const byte *surface);
	void FADE_INW_LINUX(const byte *surface);
	void FADE_IN_CASSE();
	void FADE_OUT_CASSE();
	void setpal_vga256(const byte *palette);
	void setpal_vga256_linux(const byte *palette, const byte *surface);
	void SETCOLOR(int palIndex, int r, int g, int b);
	void SETCOLOR2(int palIndex, int r, int g, int b);
	void SETCOLOR3(int palIndex, int r, int g, int b);
	void SETCOLOR4(int palIndex, int r, int g, int b);
	void CHANGE_PALETTE(const byte *palette);
	uint16 MapRGB(byte r, byte g, byte b);
	void DD_VBL();
	void Copy_WinScan_Vbe3(const byte *srcData, byte *destSurface);
	void Copy_Video_Vbe3(const byte *srcData);
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
	void Ajoute_Segment_Vesa(int x1, int y1, int x2, int y2);
	int Magic_Number(int v);
	void Affiche_Segment_Vesa();
	void AFFICHE_SPEEDVGA(const byte *objectData, int xp, int yp, int idx);
	void CopyAsm(const byte *surface);
	void CopyAsm16(const byte *surface);
	void Restore_Mem(byte *destSurface, const byte *src, int xp, int yp, int width, int height);
	int Reel_Zoom(int v, int percentage);
	int Reel_Reduc(int v, int percentage);
	void Affiche_Perfect(byte *surface, const byte *srcData, int xp300, int yp300, int frameIndex, int zoom1, int zoom2, int modeFlag);
	void AFFICHE_SPEED(const byte *spriteData, int xp, int yp, int spriteIndex);
	void SCOPY(const byte *surface, int x1, int y1, int width, int height, byte *destSurface, int destX, int destY);
	void Copy_Mem(const byte *srcSurface, int x1, int y1, unsigned int width, int height, byte *destSurface, int destX, int destY);
	void Affiche_Fonte(byte *surface, const byte *spriteData, int xp, int yp, int characterIndex, int colour);
	void INI_ECRAN(const Common::String &file);
	void INI_ECRAN2(const Common::String &file);
	void OPTI_INI(const Common::String &file, int mode);
	void NB_SCREEN();
	void SHOW_PALETTE();
	void videkey();
	void Copy_WinScan_Vbe(const byte *srcP, byte *destP);
	void Copy_Video_Vbe(const byte *src);
	void Reduc_Ecran(const byte *srcSruface, byte *destSurface, int xp, int yp, int width, int height, int zoom);
	void Plot_Hline(byte *surface, int xp, int yp, unsigned int width, byte col);
	void Plot_Vline(byte *surface, int xp, int yp, int height, byte col);
};

} // End of namespace Hopkins

#endif /* HOPKINS_GRAPHICS_H */
