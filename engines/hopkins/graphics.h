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
	byte TABLE_COUL[PALETTE_SIZE];
	byte cmap[PALETTE_BLOCK_SIZE];
	byte Palette[PALETTE_EXT_BLOCK_SIZE];
	bool Linear;
	Graphics::Surface *VideoPtr;
	byte *VESA_SCREEN;
	byte *VESA_BUFFER;
	int start_x;
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
	void INIT_TABLE(int a1, int a2, byte *a3);
	int SCROLL_ECRAN(int amount);
	void Trans_bloc(byte *destP, byte *srcP, int count, int param1, int param2);
	void A_PCX640_480(byte *surface, const Common::String &file, byte *palette, bool typeFlag);
	void Cls_Pal();
	void souris_max();
	void SCANLINE(int pitch);
	void m_scroll(const byte *surface, int xs, int ys, int width, int height, int destX, int destY);
	void m_scroll2(const byte *surface, int xs, int ys, int width, int height, int destX, int destY);
	void m_scroll2A(const byte *surface, int xs, int ys, int width, int height, int destX, int destY);
	void m_scroll16(const byte *surface, int xs, int ys, int width, int height, int destX, int destY);
	void m_scroll16A(const byte *surface, int xs, int ys, int width, int height, int destX, int destY);
	void fade_in(const byte *palette, int step, const byte *surface);
	void fade_out(const byte *palette, int step, const byte *surface);
	void FADE_INS();
	void FADE_OUTS();
	void FADE_INW();
	void FADE_OUTW();
	void setpal_vga256(const byte *palette);
	void setpal_vga256_linux(const byte *palette, const byte *surface);
	void SETCOLOR3(int palIndex, int r, int g, int b);
	void CHANGE_PALETTE(const byte *palette);
	uint16 MapRGB(byte r, byte g, byte b);
	void DD_VBL();
	void FADE_OUTW_LINUX(const byte *surface);
	void Copy_WinScan_Vbe3(const byte *sourceSurface, byte *destSurface);
	void Copy_Video_Vbe3(const byte *surface);
	void Copy_Video_Vbe16(const byte *surface);
	void Capture_Mem(byte *a1, void *a2, int a3, int a4, unsigned int a5, int a6);
	void Sprite_Vesa(byte *a1, byte *a2, int a3, int a4, int a5);
	void FIN_VISU();
	void VISU_ALL();
	void RESET_SEGMENT_VESA();
	void Ajoute_Segment_Vesa(int a1, int a2, int a3, int a4);
	int Magic_Number(int v);
	void Affiche_Segment_Vesa();
	void CopyAsm(const byte *surface);
	void Restore_Mem(const byte *a1, const byte *a2, int a3, int a4, unsigned int a5, int a6);
	int Reel_Zoom(int a1, int a2);
	void AFF_SPRITES();
	void Affiche_Perfect(const byte *a1, const byte *a2, int a3, int a4, int a5, int a6, int a7, int a8);
};

} // End of namespace Hopkins

#endif /* HOPKINS_GRAPHICS_H */
