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

#include "common/scummsys.h"
#include "util.h"
#include "tony/window.h"
#include "tony/game.h"
#include "tony/tony.h"

namespace Tony {

#define DDRELEASE(x)		if (x) { (x)->Release(); (x)=NULL; }

/****************************************************************************\
*       RMWindow Methods
\****************************************************************************/

RMWindow::RMWindow() { 

}

RMWindow::~RMWindow() {
	Close();
	RMText::Unload();
}

/**
 * Initialises the graphics window
 */
void RMWindow::Init() {
	initGraphics(RM_SX, RM_SY, false);

	// Inizializza i conteggi degli FPS
	fps = lastfcount = fcount = lastsecond = 0;

	m_bGrabScreenshot = false;
	m_bGrabThumbnail = false;
	m_bGrabMovie = false;
}

/**
 * Close the window
 */
void RMWindow::Close(void) {
}

void RMWindow::GrabThumbnail(uint16 *thumbmem) {
	warning("TODO: RMWindow::GrabThumbnail");
}

/**
 * Repaint the screen
 */
void RMWindow::Repaint(void) {
	g_system->updateScreen();
}

bool RMWindow::Lock() {
	return true;
}

void RMWindow::Unlock() {
}

/**
 * Wipes an area of the screen
 */
void RMWindow::WipeEffect(Common::Rect &rcBoundEllipse) {
	if ((rcBoundEllipse.left == 0) && (rcBoundEllipse.top == 0) &&
		(rcBoundEllipse.right == RM_SX) && (rcBoundEllipse.bottom == RM_SY)) {
		// Full screen clear wanted, so use shortcut method
		g_system->fillScreen(0);
	} else {
		// Clear the designated area a line at a time
		byte line[RM_SX];
		Common::fill(line, line + RM_SX, 0);

		// Loop through each line
		for (int yp = rcBoundEllipse.top; yp < rcBoundEllipse.bottom; ++yp) {
			g_system->copyRectToScreen(line, RM_SX, rcBoundEllipse.left, yp, rcBoundEllipse.width(), 1);
		}
	}
}

void RMWindow::GetNewFrame(byte *lpBuf, Common::Rect *rcBoundEllipse) {
	if (rcBoundEllipse == NULL) {
		// Full screen update
		g_system->copyRectToScreen(lpBuf, RM_SX, 0, 0, RM_SX, RM_SY);
	} else {
		// Update a screen section
		g_system->copyRectToScreen(lpBuf, RM_SX, rcBoundEllipse->left, rcBoundEllipse->top, 
			rcBoundEllipse->width(), rcBoundEllipse->height());
	}
}


/****************************************************************************\
*       RMSnapshot Methods
\****************************************************************************/

char RMSnapshot::bufDrive[MAX_DRIVE];
char RMSnapshot::bufDir[MAX_DIR];
char RMSnapshot::bufName[MAX_FNAME];
char RMSnapshot::bufExt[MAX_EXT];
char RMSnapshot::filename[512];
byte RMSnapshot::rgb[RM_SX * RM_SY * 3];

bool RMSnapshot::GetFreeSnapName(char *fn) {
#ifdef REFACTOR_ME
	int i, j, k;
	HANDLE h;

	theGame.GetDataDirectory(RMGame::DD_SHOTS, fn);
	_splitpath(fn, bufDrive, bufDir, NULL, NULL);

	for (i = 1; i < 10; i++) {
		wsprintf(bufName,"rm%d00",i);
		_makepath(fn,bufDrive,bufDir,bufName,".bmp");
		h = CreateFile(fn,GENERIC_READ,FILE_SHARE_READ, NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, NULL);
		if (h == INVALID_HANDLE_VALUE)
			break;
		CloseHandle(h);
	}

	i--;

	for (j = 1; j < 10; j++) {
		wsprintf(bufName,"rm%d%d0",i,j);
		_makepath(fn,bufDrive,bufDir,bufName,".bmp");
		h=CreateFile(fn,GENERIC_READ,FILE_SHARE_READ, NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, NULL);
		if (h==INVALID_HANDLE_VALUE)
			break;
		CloseHandle(h);
	}

	j--;

	for (k = 0; k < 10; k++) {
		wsprintf(bufName,"rm%d%d%d",i,j,k);
		_makepath(fn,bufDrive,bufDir,bufName,".bmp");
		h = CreateFile(fn,GENERIC_READ,FILE_SHARE_READ, NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, NULL);
		if (h==INVALID_HANDLE_VALUE)
			break;
		CloseHandle(h);
	}

	if (k == 10) {
		k = 0;
		j++;
		if (j == 10) {
			j = 0;
			i++;
			if (i == 10)
				return false;
		}

		wsprintf(bufName,"rm%d%d%d",i,j,k);
		_makepath(fn,bufDrive,bufDir,bufName,".bmp");
	}
#endif
	return true;
}

void RMSnapshot::GrabScreenshot(byte *lpBuf, int dezoom, uint16 *lpDestBuf) {
#ifdef REFACTOR_ME
	uint16 *src = (uint16 *)lpBuf;
	
	int dimx = RM_SX / dezoom;
	int dimy = RM_SY / dezoom;

	int u, v, curv;

	uint16	appo;
	uint32	k = 0;
	int sommar, sommab, sommag;
	uint16 *cursrc;
		
	if (lpDestBuf == NULL)
		src += (RM_SY - 1) * RM_BBX;

	if (dezoom == 1 && 0) { // @@@ NON E' TESTATA MOLTO BENE!!!
		byte *curOut = rgb;
		
		for (int y = 0; y < dimy; y++) {
			for (int x = 0; x < dimx; x++) {
				cursrc = &src[RM_SKIPX + x];

				*curOut++ = ((*cursrc) & 0x1F) << 3;
				*curOut++ = (((*cursrc) >> 5) & 0x1F) << 3;
				*curOut++ = (((*cursrc) >> 10) & 0x1F) << 3;

				if (lpDestBuf)
					*lpDestBuf++ = *cursrc;
			}

			if (lpDestBuf == NULL)
				src -= RM_BBX;
			else
				src += RM_BBX;
		}			
	} else {
		for (int y = 0; y < dimy; y++) {
			for(int x = 0; x < dimx; x++) {
				cursrc = &src[RM_SKIPX + x * dezoom];
				sommar = sommab = sommag = 0;
				
				for (v = 0; v < dezoom; v++)
					for (u = 0; u < dezoom; u++) {
						if (lpDestBuf == NULL)
							curv = -v;
						else
							curv = v;
						
						sommab += cursrc[curv*RM_BBX + u] & 0x1F;
						sommag += (cursrc[curv*RM_BBX + u] >> 5) & 0x1F;
						sommar += (cursrc[curv*RM_BBX + u] >> 10) & 0x1F;
					}

					rgb[k + 0] = (byte) (sommab * 8 / (dezoom * dezoom));
					rgb[k + 1] = (byte) (sommag * 8 / (dezoom * dezoom));
					rgb[k + 2] = (byte) (sommar * 8 / (dezoom * dezoom));

					if (lpDestBuf!=NULL)
						lpDestBuf[k/3] = ((int)rgb[k+0]>>3) | (((int)rgb[k+1]>>3)<<5) | (((int)rgb[k+2]>>3)<<10);

					k+=3;
			}

			if (lpDestBuf == NULL)
				src -= RM_BBX * dezoom;
			else
				src += RM_BBX * dezoom;
		}
	}


	if (lpDestBuf == NULL) {
		if (!GetFreeSnapName(filename))
			return;

		HANDLE	hFile = CreateFile(filename,
									GENERIC_WRITE,
									0,
									NULL,
									CREATE_ALWAYS,
									FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN,
									NULL);

		BITMAPFILEHEADER bmfHeader;
		bmfHeader.bfType = ((uint16) ('M' << 8) | 'B');
		bmfHeader.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dimx * dimy * 3;
		bmfHeader.bfReserved1 = 0;
		bmfHeader.bfReserved2 = 0;
		bmfHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		BITMAPINFOHEADER	bmiHeader;
		bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
		bmiHeader.biWidth = dimx;
		bmiHeader.biHeight = dimy;
		bmiHeader.biPlanes = 1;
		bmiHeader.biBitCount = 24;
		bmiHeader.biCompression = BI_RGB;
		bmiHeader.biSizeImage = dimx * dimy * 3;
		bmiHeader.biXPelsPerMeter = 0xB12;
		bmiHeader.biYPelsPerMeter = 0xB12;
		bmiHeader.biClrUsed = 0;
		bmiHeader.biClrImportant = 0;

		uint32	dwWritten;
		WriteFile(hFile, &bmfHeader, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
		WriteFile(hFile, &bmiHeader, sizeof(BITMAPINFOHEADER), &dwWritten, NULL);

		WriteFile(hFile, rgb, dimx * dimy * 3, &dwWritten, NULL);
		CloseHandle(hFile);
	}
#endif
}

} // End of namespace Tony
