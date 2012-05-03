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
/**************************************************************************
 *                                     様様様様様様様様様様様様様様様様様 *
 *                                             Nayma Software srl         *
 *                    e                -= We create much MORE than ALL =- *
 *        u-        z$$$c        '.    様様様様様様様様様様様様様様様様様 *
 *      .d"        d$$$$$b        "b.                                     *
 *   .z$*         d$$$$$$$L        ^*$c.                                  *
 *  #$$$.         $$$$$$$$$         .$$$" Project: Roasted Moths........  *
 *    ^*$b       4$$$$$$$$$F      .d$*"                                   *
 *      ^$$.     4$$$$$$$$$F     .$P"     Module:  Window.HPP...........  *
 *        *$.    '$$$$$$$$$     4$P 4                                     *
 *     J   *$     "$$$$$$$"     $P   r    Author:  Giovanni Bajo........  *
 *    z$   '$$$P*4c.*$$$*.z@*R$$$    $.                                   *
 *   z$"    ""       #$F^      ""    '$c  Desc:    Classi per la gestione *
 *  z$$beu     .ue="  $  "=e..    .zed$$c          di una finestra Direct 
 *      "#$e z$*"   .  `.   ^*Nc e$""              Draw.................  *
 *         "$$".  .r"   ^4.  .^$$"                 .....................  *
 *          ^.@*"6L=\ebu^+C$"*b."                                         *
 *        "**$.  "c 4$$$  J"  J$P*"    OS:  [ ] DOS  [X] WIN95  [ ] PORT  *
 *            ^"--.^ 9$"  .--""      COMP:  [ ] WATCOM  [X] VISUAL C++    *
 *                    "                     [ ] EIFFEL  [ ] GCC/GXX/DJGPP *
 *                                                                        *
 * This source code is Copyright (C) Nayma Software.  ALL RIGHTS RESERVED *
 *                                                                        *
 **************************************************************************/

#ifndef TONY_WINDOW_H
#define TONY_WINDOW_H

#include "common/scummsys.h"
#include "common/rect.h"
#include "tony/adv.h"

namespace Tony {

typedef uint32 HWND;
struct DDSURFACEDESC {
};

class RMSnapshot {
private:
	// Buffer per la creazione dei path
	static char bufDrive[_MAX_DRIVE], bufDir[_MAX_DIR], bufName[_MAX_FNAME], bufExt[_MAX_EXT];
	static char filename[512];

	// Buffer per la conversione a RGB
	static byte	rgb[RM_SX * RM_SY * 3];

private:
	bool GetFreeSnapName(char *fn);

public:
	// Prende uno screenshot
	void GrabScreenshot(byte *lpBuf, int dezoom = 1, uint16 *lpDestBuf = NULL);
};


class RMWindow {
private:
	bool Lock(DDSURFACEDESC &ddsd);
	void Unlock(DDSURFACEDESC &ddsd);

	// Inizializza DirectDraw
	void DDInit(void);

	// Chiude DirectDraw
	void DDClose(void);

	// Repaint grafico tramite DirectDraw
	void Repaint(void);

	// Window Proc principale che richiama la WindowProc dentro la classe
//	friend LRESULT CALLBACK GlobalWindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// Procedura di gestione messaggi
//	LRESULT WindowProc(UINT msg, WPARAM wParam, LPARAM lParam);

protected:
	HWND m_hWnd;
	bool m_bFullscreen;

	void * /*LPDIRECTDRAW*/ m_DD;
	void * /*LPDIRECTDRAWSURFACE*/ m_Primary;
	void * /*LPDIRECTDRAWSURFACE*/ m_Back;
	void * /*LPDIRECTDRAWCLIPPER*/ m_MainClipper;
	void * /*LPDIRECTDRAWCLIPPER*/ m_BackClipper;

	int fps, fcount;
	int lastsecond, lastfcount;

	int mskRed, mskGreen, mskBlue;

	bool m_bGrabScreenshot;
	bool m_bGrabThumbnail;
	bool m_bGrabMovie;
	uint16 *m_wThumbBuf;

	void CreateBWPrecalcTable(void);
	void UpdatePixelFormat(void);
	void WipeEffect(Common::Rect &rcBoundEllipse);

	public:
		RMWindow() { m_Primary = NULL; m_Back = NULL; };
		~RMWindow() { Close(); }

		// Inizializzazione
		void Init(/*HINSTANCE hInst*/);
		void InitDirectDraw(void);
		void Close(void);

		// Switch tra windowed e fullscreen
		void SwitchFullscreen(bool bFull);

		// Legge il prossimo frame
		void GetNewFrame(byte *lpBuf, Common::Rect *rcBoundEllipse);

		// Avverte di grabbare un thumbnail per il salvataggio
		void GrabThumbnail(uint16 *buf);

		operator HWND() { return m_hWnd; }

		// Modi pixel format
		// MODE1: 1555
		// MODE2: 5515
		// MODE3: 5551
		bool ISMODE1() { return (mskRed == 0x7C00 && mskGreen == 0x3E0 && mskBlue== 0x1F); }
		bool ISMODE2() { return (mskRed == 0xF800 && mskGreen == 0x7E0 && mskBlue== 0x1F); }
		bool ISMODE3() { return (mskRed == 0xF800 && mskGreen == 0x7C0 && mskBlue== 0x3E); }
		bool ISMODE4() { return (mskBlue == 0xF800 && mskGreen == 0x7E0 && mskRed== 0x1F); }
};

} // End of namespace Tony

#endif /* TONY_WINDOW_H */
