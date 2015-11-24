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

#include "lab/lab.h"
#include "lab/vga.h"
#include "lab/stddefines.h"
#include "lab/mouse.h"

#include "graphics/palette.h"

#include "common/events.h"

namespace Lab {

static byte _curvgapal[256 * 3];
static unsigned char _curapen = 0;

byte *_currentDsplayBuffer = 0;
byte *_displayBuffer = 0;

int _lastWaitTOFTicks = 0;

uint32 _mouseX = 0;
uint32 _mouseY = 0;

uint16 _nextKeyIn = 0;
uint16 _keyBuf[64];
uint16 _nextKeyOut = 0;
bool _mouseAtEdge = false;
byte *_tempScrollData;

/*****************************************************************************/
/* Sets up either a low-res or a high-res 256 color screen.                  */
/*****************************************************************************/
bool createScreen(bool HiRes) {
	if (HiRes) {
		g_lab->_screenWidth  = 640;
		g_lab->_screenHeight = 480;
	} else {
		g_lab->_screenWidth  = 320;
		g_lab->_screenHeight = 200;
	}
	g_lab->_screenBytesPerPage = g_lab->_screenWidth * g_lab->_screenHeight;

	_displayBuffer = (byte *)malloc(g_lab->_screenBytesPerPage);

	return true;
}

/*****************************************************************************/
/* Sets the current page on the VGA card.                                    */
/*****************************************************************************/
void changeVolume(int delta) {
	warning("STUB: changeVolume()");
}

uint16 WSDL_GetNextChar() {
	uint16 c = 0;

	WSDL_ProcessInput(0);
	if (_nextKeyIn != _nextKeyOut) {
		c = _keyBuf[_nextKeyOut];
		_nextKeyOut = ((((unsigned int)((_nextKeyOut + 1) >> 31) >> 26) + (byte)_nextKeyOut + 1) & 0x3F)
                 - ((unsigned int)((_nextKeyOut + 1) >> 31) >> 26);
  	}

	return c;
}

bool WSDL_HasNextChar() {
	WSDL_ProcessInput(0);
	return _nextKeyIn != _nextKeyOut;
}

void WSDL_ProcessInput(bool can_delay) {
	int n;
	int lastMouseAtEdge;
	int flags = 0;

	Common::Event event;

	if (1 /*!g_IgnoreProcessInput*/) {
		while (g_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_RBUTTONDOWN:
				flags |= 8;
				mouseHandler(flags, _mouseX, _mouseY);
				break;

			case Common::EVENT_LBUTTONDOWN:
				flags |= 2;
				mouseHandler(flags, _mouseX, _mouseY);
				break;

			case Common::EVENT_MOUSEMOVE:
				lastMouseAtEdge = _mouseAtEdge;
				_mouseAtEdge = false;
				_mouseX = event.mouse.x;
				if (event.mouse.x <= 0) {
					_mouseX = 0;
					_mouseAtEdge = true;
				}
				if (_mouseX > g_lab->_screenWidth - 1) {
					_mouseX = g_lab->_screenWidth;
					_mouseAtEdge = true;
				}

				_mouseY = event.mouse.y;
				if (event.mouse.y <= 0) {
					_mouseY = 0;
					_mouseAtEdge = true;
				}
				if (_mouseY > g_lab->_screenHeight - 1) {
					_mouseY = g_lab->_screenHeight;
					_mouseAtEdge = true;
				}

				if (!lastMouseAtEdge || !_mouseAtEdge)
					mouseHandler(1, _mouseX, _mouseY);

				break;

			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_LEFTBRACKET:
					changeVolume(-1);
					break;

				case Common::KEYCODE_RIGHTBRACKET:
					changeVolume(1);
					break;

				case Common::KEYCODE_z:
					//saveSettings();
					break;

				default:
					n = ((((unsigned int)((_nextKeyIn + 1) >> 31) >> 26) + (byte)_nextKeyIn + 1) & 0x3F)
					- ((unsigned int)((_nextKeyIn + 1) >> 31) >> 26);
					if (n != _nextKeyOut) {
						_keyBuf[_nextKeyIn] = event.kbd.keycode;
						_nextKeyIn = n;
					}
				}
				break;

			case Common::EVENT_QUIT:
			case Common::EVENT_RTL:
			default:
				break;
			}

			g_system->copyRectToScreen(_displayBuffer, g_lab->_screenWidth, 0, 0, g_lab->_screenWidth, g_lab->_screenHeight);
			g_system->updateScreen();
		}
	}

	if (can_delay)
		g_system->delayMillis(10);
}

void WSDL_GetMousePos(int *x, int *y) {
	WSDL_ProcessInput(0);

	*x = _mouseX;
	*y = _mouseY;
}

void waitTOF() {
	g_system->copyRectToScreen(_displayBuffer, g_lab->_screenWidth, 0, 0, g_lab->_screenWidth, g_lab->_screenHeight);
	g_system->updateScreen();

  	WSDL_ProcessInput(0);

  	uint32 now;

	for (now = g_system->getMillis(); now - _lastWaitTOFTicks <= 0xF; now = g_system->getMillis() )
		g_system->delayMillis(_lastWaitTOFTicks - now + 17);

	_lastWaitTOFTicks = now;
}

void WSDL_SetColors(byte *buf, uint16 first, uint16 numreg, uint16 slow) {
	byte tmp[256 * 3];

	for (int i = 0; i < 256 * 3; i++) {
		tmp[i] = buf[i] * 4;
	}

	g_system->getPaletteManager()->setPalette(tmp, first, numreg);

	if (slow)
    	waitTOF();
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
void writeColorRegs(byte *buf, uint16 first, uint16 numreg) {
	WSDL_SetColors(buf, first, numreg, 0);
	memcpy(&(_curvgapal[first * 3]), buf, numreg * 3);
}

void writeColorRegsSmooth(byte *buf, uint16 first, uint16 numreg) {
	WSDL_SetColors(buf, first, numreg, 1);
	memcpy(&(_curvgapal[first * 3]), buf, numreg * 3);
}

/*****************************************************************************/
/* Sets one of the 256 (0..255) color registers.  buf is a char pointer,     */
/* the first character in the string is the red value, then green, then      */
/* blue.  Each color value is a 6 bit value.                                 */
/*****************************************************************************/
void writeColorReg(byte *buf, uint16 regnum) {
	writeColorRegs(buf, regnum, 1);
}

void VGASetPal(void *cmap, uint16 numcolors) {
	if (memcmp(cmap, _curvgapal, numcolors * 3) != 0)
		writeColorRegs((byte *)cmap, 0, numcolors);
}

void WSDL_UpdateScreen() {
	g_system->copyRectToScreen(_displayBuffer, g_lab->_screenWidth, 0, 0, g_lab->_screenWidth, g_lab->_screenHeight);
	g_system->updateScreen();

	WSDL_ProcessInput(0);
}

/*****************************************************************************/
/* Returns the base address of the current VGA display.                      */
/*****************************************************************************/
byte *getVGABaseAddr() {
	if (_currentDsplayBuffer)
		return _currentDsplayBuffer;

	return _displayBuffer;
}

/*****************************************************************************/
/* Draws an image to the screen.                                             */
/*****************************************************************************/
void drawImage(Image *Im, uint16 x, uint16 y) {
	int sx, sy, dx, dy, w, h;

	sx = 0;
	sy = 0;
	dx = x;
	dy = y;
	w = Im->Width;
	h = Im->Height;

	if (dx < 0) {
		sx -= dx;
		w += dx;
		dx = 0;
	}

	if (dy < 0) {
		sy -= dy;
		w += dy;
		dy = 0;
	}

	if ((uint)(dx + w) > g_lab->_screenWidth)
		w = g_lab->_screenWidth - dx;

	if ((uint)(dy + h) > g_lab->_screenHeight)
		h = g_lab->_screenHeight - dy;

	if (w > 0 && h > 0) {
		byte *s = Im->ImageData + sy * Im->Width + sx;
		byte *d = getVGABaseAddr() + dy * g_lab->_screenWidth + dx;

		while (h-- > 0) {
			memcpy(d, s, w);
			s += Im->Width;
			d += g_lab->_screenWidth;
		}
	}
}

/*****************************************************************************/
/* Draws an image to the screen.                                             */
/*****************************************************************************/
void drawMaskImage(Image *Im, uint16 x, uint16 y) {
	int sx, sy, dx, dy, w, h;

	sx = 0;
	sy = 0;
	dx = x;
	dy = y;
	w = Im->Width;
	h = Im->Height;

	if (dx < 0) {
		sx -= dx;
		w += dx;
		dx = 0;
	}

	if (dy < 0) {
		sy -= dy;
		w += dy;
		dy = 0;
	}

	if ((uint)(dx + w) > g_lab->_screenWidth)
		w = g_lab->_screenWidth - dx;

	if ((uint)(dy + h) > g_lab->_screenHeight)
		h = g_lab->_screenHeight - dy;

	if (w > 0 && h > 0) {
		byte *s = Im->ImageData + sy * Im->Width + sx;
		byte *d = getVGABaseAddr() + dy * g_lab->_screenWidth + dx;

		while (h-- > 0) {
			byte *ss = s;
			byte *dd = d;
			int ww = w;

			while (ww-- > 0) {
				byte c = *ss++;

				if (c) *dd++ = c - 1;
				else dd++;
			}

			s += Im->Width;
			d += g_lab->_screenWidth;
		}
	}
}

/*****************************************************************************/
/* Reads an image from the screen.                                           */
/*****************************************************************************/
void readScreenImage(Image *Im, uint16 x, uint16 y) {
	int sx, sy, dx, dy, w, h;

	sx = 0;
	sy = 0;
	dx = x;
	dy = y;
	w = Im->Width;
	h = Im->Height;

	if (dx < 0) {
		sx -= dx;
		w += dx;
		dx = 0;
	}

	if (dy < 0) {
		sy -= dy;
		w += dy;
		dy = 0;
	}

	if ((uint)(dx + w) > g_lab->_screenWidth)
		w = g_lab->_screenWidth - dx;

	if ((uint)(dy + h) > g_lab->_screenHeight)
		h = g_lab->_screenHeight - dy;

	if (w > 0 && h > 0) {
		byte *s = Im->ImageData + sy * Im->Width + sx;
		byte *d = getVGABaseAddr() + dy * g_lab->_screenWidth + dx;

		while (h-- > 0) {
			memcpy(s, d, w);
			s += Im->Width;
			d += g_lab->_screenWidth;
		}
	}
}

/*****************************************************************************/
/* Blits a piece of one image to another.                                    */
/* NOTE: for our purposes, assumes that ImDest is to be in VGA memory.       */
/*****************************************************************************/
void bltBitMap(Image *ImSource, uint16 xs, uint16 ys, Image *ImDest,
					uint16 xd, uint16 yd, uint16 width, uint16 height) {
	// I think the old code assumed that the source image data was valid for the given box.
	// I will proceed on that assumption.
	int sx, sy, dx, dy, w, h;

	sx = xs;
	sy = ys;
	dx = xd;
	dy = yd;
	w = width;
	h = height;

	if (dx < 0) {
		sx -= dx;
		w += dx;
		dx = 0;
	}

	if (dy < 0) {
		sy -= dy;
		w += dy;
		dy = 0;
	}

	if (dx + w > ImDest->Width) w = ImDest->Width - dx;

	if (dy + h > ImDest->Height) h = ImDest->Height - dy;

	if (w > 0 && h > 0) {
		byte *s = ImSource->ImageData + sy * ImSource->Width + sx;
		byte *d = ImDest->ImageData + dy * ImDest->Width + dx;

		while (h-- > 0) {
			memcpy(d, s, w);
			s += ImSource->Width;
			d += ImDest->Width;
		}
	}
}

/*****************************************************************************/
/* Scrolls the display in the x direction by blitting.                       */
/* The _tempScrollData variable must be initialized to some memory, or this   */
/* function will fail.                                                       */
/*****************************************************************************/
void scrollDisplayX(int16 dx, uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	Image Im;
	uint16 temp;

	Im.ImageData = _tempScrollData;

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

	if (dx > 0) {
		Im.Width = x2 - x1 + 1 - dx;
		Im.Height = y2 - y1 + 1;

		readScreenImage(&Im, x1, y1);
		drawImage(&Im, x1 + dx, y1);

		setAPen(0);
		rectFill(x1, y1, x1 + dx - 1, y2);
	} else if (dx < 0) {
		Im.Width = x2 - x1 + 1 + dx;
		Im.Height = y2 - y1 + 1;

		readScreenImage(&Im, x1 - dx, y1);
		drawImage(&Im, x1, y1);

		setAPen(0);
		rectFill(x2 + dx + 1, y1, x2, y2);
	}
}

/*****************************************************************************/
/* Scrolls the display in the y direction by blitting.                       */
/*****************************************************************************/
void scrollDisplayY(int16 dy, uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	Image Im;
	uint16 temp;

	Im.ImageData = _tempScrollData;

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

	if (dy > 0) {
		Im.Width = x2 - x1 + 1;
		Im.Height = y2 - y1 + 1 - dy;

		readScreenImage(&Im, x1, y1);
		drawImage(&Im, x1, y1 + dy);

		setAPen(0);
		rectFill(x1, y1, x2, y1 + dy - 1);
	} else if (dy < 0) {
		Im.Width = x2 - x1 + 1;
		Im.Height = y2 - y1 + 1 + dy;

		readScreenImage(&Im, x1, y1 - dy);
		drawImage(&Im, x1, y1);

		setAPen(0);
		rectFill(x1, y2 + dy + 1, x2, y2);
	}
}

/*****************************************************************************/
/* Sets the pen number to use on all the drawing operations.                 */
/*****************************************************************************/
void setAPen(uint16 pennum) {
	_curapen = (unsigned char)pennum;
}

/*****************************************************************************/
/* Fills in a rectangle.                                                     */
/*****************************************************************************/
void rectFill(uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	int dx, dy, w, h;

	dx = x1;
	dy = y1;
	w = x2 - x1 + 1;
	h = y2 - y1 + 1;

	if (dx < 0) {
		w += dx;
		dx = 0;
	}

	if (dy < 0) {
		w += dy;
		dy = 0;
	}

	if ((uint)(dx + w) > g_lab->_screenWidth)
		w = g_lab->_screenWidth - dx;

	if ((uint)(dy + h) > g_lab->_screenHeight)
		h = g_lab->_screenHeight - dy;

	if (w > 0 && h > 0) {
		char *d = (char *)getVGABaseAddr() + dy * g_lab->_screenWidth + dx;

		while (h-- > 0) {
			char *dd = d;
			int ww = w;

			while (ww-- > 0) {
				*dd++ = _curapen;
			}

			d += g_lab->_screenWidth;
		}
	}
}

/*****************************************************************************/
/* Draws a horizontal line.                                                  */
/*****************************************************************************/
void drawVLine(uint16 x, uint16 y1, uint16 y2) {
	rectFill(x, y1, x, y2);
}

/*****************************************************************************/
/* Draws a vertical line.                                                    */
/*****************************************************************************/
void drawHLine(uint16 x1, uint16 y, uint16 x2) {
	rectFill(x1, y, x2, y);
}

/*****************************************************************************/
/* Ghoasts a region on the screen using the desired pen color.               */
/*****************************************************************************/
void ghoastRect(uint16 pencolor, uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	int dx, dy, w, h;

	dx = x1;
	dy = y1;
	w = x2 - x1 + 1;
	h = y2 - y1 + 1;

	if (dx < 0) {
		w += dx;
		dx = 0;
	}

	if (dy < 0) {
		w += dy;
		dy = 0;
	}

	if ((uint)(dx + w) > g_lab->_screenWidth)
		w = g_lab->_screenWidth - dx;

	if ((uint)(dy + h) > g_lab->_screenHeight)
		h = g_lab->_screenHeight - dy;

	if (w > 0 && h > 0) {
		char *d = (char *)getVGABaseAddr() + dy * g_lab->_screenWidth + dx;

		while (h-- > 0) {
			char *dd = d;
			int ww = w;

			if (dy & 1) {
				dd++;
				ww--;
			}

			while (ww > 0) {
				*dd = pencolor;
				dd += 2;
				ww -= 2;
			}

			d += g_lab->_screenWidth;
			dy++;
		}
	}
}

} // End of namespace Lab
