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
	Graphics::PixelFormat pixelFormat(2, 5, 5, 5, 0, 10, 5, 0, 0);
	initGraphics(RM_SX, RM_SY, true, &pixelFormat);

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
	m_bGrabThumbnail = true;
	m_wThumbBuf = thumbmem;
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
		uint16 line[RM_SX];
		Common::fill(line, line + RM_SX, 0);

		// Loop through each line
		for (int yp = rcBoundEllipse.top; yp < rcBoundEllipse.bottom; ++yp) {
			g_system->copyRectToScreen((const byte *)&line[0], RM_SX * 2, rcBoundEllipse.left, yp, rcBoundEllipse.width(), 1);
		}
	}
}

void RMWindow::GetNewFrame(byte *lpBuf, Common::Rect *rcBoundEllipse) {
	if (rcBoundEllipse != NULL) {
		// Circular wipe effect
		GetNewFrameWipe(lpBuf, *rcBoundEllipse);
	} else {
		// Standard screen copy
		g_system->copyRectToScreen(lpBuf, RM_SX * 2, 0, 0, RM_SX, RM_SY);
	}

	if (m_bGrabThumbnail) {
		// Need to generate a thumbnail
		RMSnapshot s;

		s.GrabScreenshot(lpBuf, 4, m_wThumbBuf);
		m_bGrabThumbnail = false;
	}
}

/**
 * Copies a section of the game frame in a circle bounded by the specified rectangle
 */
void RMWindow::GetNewFrameWipe(byte *lpBuf, Common::Rect &rcBoundEllipse) {
	// Clear the screen
	g_system->fillScreen(0);

	if (!rcBoundEllipse.isValidRect())
		return;

	Common::Point center(rcBoundEllipse.left + rcBoundEllipse.width() / 2, 
		rcBoundEllipse.top + rcBoundEllipse.height() / 2);
	int radius = rcBoundEllipse.width() / 2;

	int error = -radius;
	int x = radius;
	int y = 0;

	while (x >= y) {
		plotSplices(lpBuf, center, x, y);
 
		error += y;
		++y;
		error += y;
 
		if (error >= 0) {
			error -= x;
			--x;
			error -= x;
		}
	}
}

/**
 * Handles drawing the line splices for the circle of viewable area
 */
void RMWindow::plotSplices(const byte *lpBuf, const Common::Point &center, int x, int y) {
	plotLines(lpBuf, center, x, y);
	if (x != y) 
		plotLines(lpBuf, center, y, x);
}

/**
 * Handles drawing the line splices for the circle of viewable area
 */
void RMWindow::plotLines(const byte *lpBuf, const Common::Point &center, int x, int y) {
	// Skips lines that have no width (i.e. at the top of the circle)
	if ((x == 0) || (y > center.y))
		return;

	const byte *pSrc;

	if ((center.y - y) >= 0) {
		// Draw line in top half of circle
		pSrc = lpBuf + ((center.y - y) * RM_SX * 2) + (center.x - x) * 2;
		g_system->copyRectToScreen(pSrc, RM_SX * 2, center.x - x, center.y - y, x * 2, 1);
	}

	if ((center.y + y) < RM_SY) {
		// Draw line in bottom half of circle
		pSrc = lpBuf + ((center.y + y) * RM_SX * 2) + (center.x - x) * 2;
		g_system->copyRectToScreen(pSrc, RM_SX * 2, center.x - x, center.y + y, x * 2, 1);
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
	uint32 h;

	theGame.GetDataDirectory(RMGame::DD_SHOTS, fn);
	_splitpath(fn, bufDrive, bufDir, NULL, NULL);

	for (i = 1; i < 10; i++) {
		wsprintf(bufName,"rm%d00",i);
		_makepath(fn,bufDrive,bufDir,bufName,".bmp");
		h = CreateFile(fn,GENERIC_READ,FILE_SHARE_READ, NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, NULL);
		if (h == CORO_INVALID_PID_VALUE)
			break;
		CloseHandle(h);
	}

	i--;

	for (j = 1; j < 10; j++) {
		wsprintf(bufName,"rm%d%d0",i,j);
		_makepath(fn,bufDrive,bufDir,bufName,".bmp");
		h=CreateFile(fn,GENERIC_READ,FILE_SHARE_READ, NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, NULL);
		if (h==CORO_INVALID_PID_VALUE)
			break;
		CloseHandle(h);
	}

	j--;

	for (k = 0; k < 10; k++) {
		wsprintf(bufName,"rm%d%d%d",i,j,k);
		_makepath(fn,bufDrive,bufDir,bufName,".bmp");
		h = CreateFile(fn,GENERIC_READ,FILE_SHARE_READ, NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL, NULL);
		if (h==CORO_INVALID_PID_VALUE)
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
	uint16 *src = (uint16 *)lpBuf;
	
	int dimx = RM_SX / dezoom;
	int dimy = RM_SY / dezoom;

	int u, v, curv;

	uint32 k = 0;
	int sommar, sommab, sommag;
	uint16 *cursrc;
		
	if (lpDestBuf == NULL)
		src += (RM_SY - 1) * RM_BBX;

	if (dezoom == 1 && 0) {
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
						lpDestBuf[k / 3] = ((int)rgb[k + 0] >> 3) | (((int)rgb[k + 1] >> 3) << 5) | 
							(((int)rgb[k + 2] >> 3) << 10);

					k += 3;
			}

			if (lpDestBuf == NULL)
				src -= RM_BBX * dezoom;
			else
				src += RM_BBX * dezoom;
		}
	}
}

} // End of namespace Tony
