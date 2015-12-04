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

/*
 * This code is based on Labyrinth of Time code with assistance of
 *
 * Copyright (c) 1993 Terra Nova Development
 * Copyright (c) 2004 The Wyrmkeep Entertainment Co.
 *
 */

#include "graphics/palette.h"
#include "common/events.h"

#include "lab/lab.h"
#include "lab/image.h"

namespace Lab {

/*****************************************************************************/
/* Sets up either a low-res or a high-res 256 color screen.                  */
/*****************************************************************************/
bool LabEngine::createScreen(bool hiRes) {
	if (hiRes) {
		_screenWidth  = 640;
		_screenHeight = 480;
	} else {
		_screenWidth  = 320;
		_screenHeight = 200;
	}
	_screenBytesPerPage = _screenWidth * _screenHeight;

	_displayBuffer = new byte[_screenBytesPerPage];	// FIXME: Memory leak!

	return true;
}

/*****************************************************************************/
/* Sets the current page on the VGA card.                                    */
/*****************************************************************************/
void LabEngine::changeVolume(int delta) {
	warning("STUB: changeVolume()");
}


void LabEngine::waitTOF() {
	g_system->copyRectToScreen(_displayBuffer, _screenWidth, 0, 0, _screenWidth, _screenHeight);
	g_system->updateScreen();

  	_event->processInput();

  	uint32 now;

	for (now = g_system->getMillis(); now - _lastWaitTOFTicks <= 0xF; now = g_system->getMillis() )
		g_system->delayMillis(_lastWaitTOFTicks - now + 17);

	_lastWaitTOFTicks = now;
}

/*****************************************************************************/
/* Writes any number of the 256 color registers.                             */
/* first:    the number of the first color register to write.                */
/* numreg:   the number of registers to write                                */
/* buf:      a char pointer which contains the selected color registers.     */
/*           Each value representing a color register occupies 3 bytes in    */
/*           the array.  The order is red, green then blue.  The first byte  */
/*           in the array is the red component of the first element selected.*/
/*           The length of the buffer is 3 times the number of registers     */
/*           selected.                                                       */
/*****************************************************************************/
void LabEngine::writeColorRegs(byte *buf, uint16 first, uint16 numreg) {
	byte tmp[256 * 3];

	for (int i = 0; i < 256 * 3; i++) {
		tmp[i] = buf[i] * 4;
	}

	g_system->getPaletteManager()->setPalette(tmp, first, numreg);

	memcpy(&(_curvgapal[first * 3]), buf, numreg * 3);
}

void LabEngine::setPalette(void *cmap, uint16 numcolors) {
	if (memcmp(cmap, _curvgapal, numcolors * 3) != 0)
		writeColorRegs((byte *)cmap, 0, numcolors);
}

void LabEngine::screenUpdate() {
	g_system->copyRectToScreen(_displayBuffer, _screenWidth, 0, 0, _screenWidth, _screenHeight);
	g_system->updateScreen();

	_event->processInput();
}

/*****************************************************************************/
/* Returns the base address of the current VGA display.                      */
/*****************************************************************************/
byte *LabEngine::getCurrentDrawingBuffer() {
	if (_currentDisplayBuffer)
		return _currentDisplayBuffer;

	return _displayBuffer;
}

/*****************************************************************************/
/* Scrolls the display in the x direction by blitting.                       */
/* The _tempScrollData variable must be initialized to some memory, or this   */
/* function will fail.                                                       */
/*****************************************************************************/
void LabEngine::scrollDisplayX(int16 dx, uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	Image im;
	uint16 temp;

	im._imageData = _tempScrollData;

	if (x1 > x2) {
		temp = x2;
		x2 = x1;
		x1 = temp;
	}

	if (y1 > y2) {
		temp = y2;
		y2 = y1;
		y1 = temp;
	}

	im._width = x2 - x1 + 1 - dx;
	im._height = y2 - y1 + 1;

	im.readScreenImage(x1, y1);
	im.drawImage(x1 + dx, y1);

	setAPen(0);
	rectFill(x1, y1, x1 + dx - 1, y2);
}

/*****************************************************************************/
/* Scrolls the display in the y direction by blitting.                       */
/*****************************************************************************/
void LabEngine::scrollDisplayY(int16 dy, uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	Image im;
	uint16 temp;

	im._imageData = _tempScrollData;

	if (x1 > x2) {
		temp = x2;
		x2 = x1;
		x1 = temp;
	}

	if (y1 > y2) {
		temp = y2;
		y2 = y1;
		y1 = temp;
	}

	im._width = x2 - x1 + 1;
	im._height = y2 - y1 + 1 - dy;

	im.readScreenImage(x1, y1);
	im.drawImage(x1, y1 + dy);

	setAPen(0);
	rectFill(x1, y1, x2, y1 + dy - 1);
}

/*****************************************************************************/
/* Sets the pen number to use on all the drawing operations.                 */
/*****************************************************************************/
void LabEngine::setAPen(byte pennum) {
	_curapen = pennum;
}

/*****************************************************************************/
/* Fills in a rectangle.                                                     */
/*****************************************************************************/
void LabEngine::rectFill(uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	int w = x2 - x1 + 1;
	int h = y2 - y1 + 1;

	if (x1 + w > _screenWidth)
		w = _screenWidth - x1;

	if (y1 + h > _screenHeight)
		h = _screenHeight - y1;

	if ((w > 0) && (h > 0)) {
		char *d = (char *)getCurrentDrawingBuffer() + y1 * _screenWidth + x1;

		while (h-- > 0) {
			char *dd = d;
			int ww = w;

			while (ww-- > 0) {
				*dd++ = _curapen;
			}

			d += _screenWidth;
		}
	}
}

/*****************************************************************************/
/* Draws a horizontal line.                                                  */
/*****************************************************************************/
void LabEngine::drawVLine(uint16 x, uint16 y1, uint16 y2) {
	rectFill(x, y1, x, y2);
}

/*****************************************************************************/
/* Draws a vertical line.                                                    */
/*****************************************************************************/
void LabEngine::drawHLine(uint16 x1, uint16 y, uint16 x2) {
	rectFill(x1, y, x2, y);
}

/*****************************************************************************/
/* Overlays a region on the screen using the desired pen color.              */
/*****************************************************************************/
void LabEngine::overlayRect(uint16 pencolor, uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	int w = x2 - x1 + 1;
	int h = y2 - y1 + 1;

	if (x1 + w > _screenWidth)
		w = _screenWidth - x1;

	if (y1 + h > _screenHeight)
		h = _screenHeight - y1;

	if ((w > 0) && (h > 0)) {
		char *d = (char *)getCurrentDrawingBuffer() + y1 * _screenWidth + x1;

		while (h-- > 0) {
			char *dd = d;
			int ww = w;

			if (y1 & 1) {
				dd++;
				ww--;
			}

			while (ww > 0) {
				*dd = pencolor;
				dd += 2;
				ww -= 2;
			}

			d += _screenWidth;
			y1++;
		}
	}
}

} // End of namespace Lab
