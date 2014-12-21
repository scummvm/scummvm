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

#include "lab/vga.h"
#include "lab/stddefines.h"
#include "lab/mouse.h"

#include "graphics/palette.h"

#include "common/events.h"

namespace Lab {

//static uint16 NotInRefresh = 0;

uint32 VGAScreenWidth = 320UL,
              VGAScreenHeight = 200UL,
              VGAPages        = 1UL,
              VGABytesPerPage = 65536UL;

byte *VGABASEADDRESS = 0;

byte *g_DisplayBuffer = 0;
byte *g_Pixels = 0;

int g_ScreenWasLocked = 0;
int g_IgnoreUpdateDisplay = 0;
int g_LastWaitTOFTicks = 0;

int g_MouseX = 0;
int g_MouseY = 0;

uint16 g_NextKeyIn = 0;
uint16 g_KeyBuf[64];
uint16 g_NextKeyOut = 0;
bool g_MouseAtEdge = false;

/*****************************************************************************/
/* Sets the display mode.                                                    */
/*****************************************************************************/
void setMode(char mode) {
	// There are no modes for SDL.
}

/*****************************************************************************/
/* Sets up either a low-res or a high-res 256 color screen.                  */
/*****************************************************************************/
bool createScreen(bool HiRes) {
	//VGABASEADDRESS  = (unsigned long)malloc(640 * 480);
	VGABASEADDRESS  = 0;
	VGAScreenWidth  = 640;
	VGAScreenHeight = 480;
	VGAPages        = 1;
	VGABytesPerPage = 640 * 480;

	g_DisplayBuffer = (byte *)malloc(VGABytesPerPage);
	g_Pixels = (byte *)calloc(VGABytesPerPage, 4);

	return true;
}



/*****************************************************************************/
/* Sets the current page on the VGA card.                                    */
/*****************************************************************************/
void setPage(uint16 PageNum) {
	// PageNum should always calculated out to zero for SDL.
	assert(PageNum == 0);
}



void VGAStorePage(void) {
	// does nothing in SDL
}

void VGARestorePage(void) {
	// does nothing in SDL
}

void changeVolume(int delta) {
	warning("STUB: changeVolume()");
}

uint16 WSDL_GetNextChar() {
	uint16 c = 0;

	WSDL_ProcessInput(0);
	if (g_NextKeyIn != g_NextKeyOut) {
		c = g_KeyBuf[g_NextKeyOut];
		g_NextKeyOut = ((((unsigned int)((g_NextKeyOut + 1) >> 31) >> 26) + (byte)g_NextKeyOut + 1) & 0x3F)
                 - ((unsigned int)((g_NextKeyOut + 1) >> 31) >> 26);
  	}

	return c;
}

bool WSDL_HasNextChar() {
	WSDL_ProcessInput(0);
	return g_NextKeyIn != g_NextKeyOut;
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
				mouse_handler(flags, g_MouseX, g_MouseY);
				break;

			case Common::EVENT_LBUTTONDOWN:
				flags |= 2;
				mouse_handler(flags, g_MouseX, g_MouseY);
				break;

			case Common::EVENT_MOUSEMOVE:
				lastMouseAtEdge = g_MouseAtEdge;
				g_MouseAtEdge = false;
				g_MouseX = event.mouse.x;
				if (event.mouse.x <= 0) {
					g_MouseX = 0;
					g_MouseAtEdge = true;
				}
				if (g_MouseX > 639) {
					g_MouseX = 640;
					g_MouseAtEdge = true;
				}

				g_MouseY = event.mouse.y;
				if (event.mouse.y <= 0) {
					g_MouseY = 0;
					g_MouseAtEdge = true;
				}
				if (g_MouseY > 479) {
					g_MouseY = 480;
					g_MouseAtEdge = true;
				}

				if (!lastMouseAtEdge || !g_MouseAtEdge)
					mouse_handler(1, g_MouseX, g_MouseY);

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
					n = ((((unsigned int)((g_NextKeyIn + 1) >> 31) >> 26) + (byte)g_NextKeyIn + 1) & 0x3F)
					- ((unsigned int)((g_NextKeyIn + 1) >> 31) >> 26);
					if (n != g_NextKeyOut) {
						g_KeyBuf[g_NextKeyIn] = event.kbd.keycode;
						g_NextKeyIn = n;
					}
				}
				break;

			case Common::EVENT_QUIT:
			case Common::EVENT_RTL:
			default:
				break;
			}

			g_system->copyRectToScreen(g_DisplayBuffer, VGAScreenWidth, 0, 0, VGAScreenWidth, VGAScreenHeight);
			g_system->updateScreen();
		}
	}

	if (can_delay)
		g_system->delayMillis(10);
}

void WSDL_GetMousePos(int *x, int *y) {
	WSDL_ProcessInput(0);

	*x = g_MouseX;
	*y = g_MouseY;
}


void waitTOF() {
	int untilOutOfRefresh = 1;

  	if (g_ScreenWasLocked || untilOutOfRefresh) {
		g_system->copyRectToScreen(g_DisplayBuffer, VGAScreenWidth, 0, 0, VGAScreenWidth, VGAScreenHeight);
		g_system->updateScreen();
  	}

  	g_ScreenWasLocked = 0;
  	WSDL_ProcessInput(0);

  	uint32 now;

  	for (now = g_system->getMillis(); now - g_LastWaitTOFTicks <= 0xF; now = g_system->getMillis() )
  		g_system->delayMillis(g_LastWaitTOFTicks - now + 17);

  	g_LastWaitTOFTicks = now;
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

static byte curvgapal[256 * 3];

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
	memcpy(&(curvgapal[first * 3]), buf, numreg * 3);
}

void writeColorRegsSmooth(byte *buf, uint16 first, uint16 numreg) {
	WSDL_SetColors(buf, first, numreg, 1);
	memcpy(&(curvgapal[first * 3]), buf, numreg * 3);
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
	if (memcmp(cmap, curvgapal, numcolors * 3) != 0)
		writeColorRegs((byte *)cmap, 0, numcolors);
}

byte *WSDL_LockVideo() {
	g_ScreenWasLocked = 1;

	return g_DisplayBuffer;
}

void WSDL_UnlockVideo() {
}

void WSDL_IgnoreUpdateDisplay(int state) {
	g_IgnoreUpdateDisplay = state;
}

void WSDL_UpdateScreen() {
	WSDL_UnlockVideo();

	if (g_ScreenWasLocked && !g_IgnoreUpdateDisplay) {
		g_system->copyRectToScreen(g_DisplayBuffer, VGAScreenWidth, 0, 0, VGAScreenWidth, VGAScreenHeight);
  		g_system->updateScreen();
  	}

	g_ScreenWasLocked = 0;
	WSDL_ProcessInput(0);
}

/*****************************************************************************/
/* Returns the base address of the current VGA display.                      */
/*****************************************************************************/
byte *getVGABaseAddr(void) {
	if (VGABASEADDRESS)
		return VGABASEADDRESS;

	return WSDL_LockVideo();
}

void ungetVGABaseAddr() {
	if (!VGABASEADDRESS)
		WSDL_UnlockVideo();
}

/*****************************************************************************/
/* Gets information about the current display.                               */
/*****************************************************************************/
void getMode(uint16 *Mode) {
	// Only one mode in SDL.
}

/*****************************************************************************/
/* Draws an image to the screen.                                             */
/*****************************************************************************/
void drawImage(struct Image *Im, uint16 x, uint16 y) {
#if !defined(DOSCODE)
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

	if ((uint)(dx + w) > VGAScreenWidth)
		w = VGAScreenWidth - dx;

	if ((uint)(dy + h) > VGAScreenHeight)
		h = VGAScreenHeight - dy;

	if (w > 0 && h > 0) {
		byte *s = Im->ImageData + sy * Im->Width + sx;
		byte *d = getVGABaseAddr() + dy * VGAScreenWidth + dx;

		while (h-- > 0) {
			memcpy(d, s, w);
			s += Im->Width;
			d += VGAScreenWidth;
		}

		ungetVGABaseAddr();
	}

#else
	uint32  RealOffset,
	         SegmentOffset,
	         LeftInSegment;
	char *video,
	              *curline,
	              *imdata = Im->ImageData;
	uint16 counterx,
	         countery = 0,
	         numwholelines,
	         numpartiallines,
	         curpage;

	while (countery < Im->Height) {
		RealOffset = (VGAScreenWidth * (y + countery)) + x;
		curpage    = RealOffset / VGABytesPerPage;
		SegmentOffset = RealOffset - (curpage * VGABytesPerPage);
		LeftInSegment = VGABytesPerPage - SegmentOffset;
		setPage(curpage);
		video = (char *)(((int32)(VGABASEADDRESS)) + SegmentOffset);

		numwholelines = LeftInSegment / VGAScreenWidth;
		numpartiallines = 0;
		counterx      = LeftInSegment - (numwholelines * VGAScreenWidth);

		if (counterx >= Im->Width)
			numwholelines++;
		else
			numpartiallines = 1;

		while (numwholelines && (countery < Im->Height)) {
			curline = video;

			for (counterx = 0; counterx < Im->Width; counterx++) {
				*video = *imdata;
				video++;
				imdata++;
			}

			video = curline;
			video += VGAScreenWidth;
			countery ++;
			numwholelines --;
			LeftInSegment -= VGAScreenWidth;
		}

		if (numpartiallines && (countery < Im->Height)) {
			countery ++;
			curline = video;

			for (counterx = 0; counterx < Im->Width; counterx++) {
				if (LeftInSegment == 0L) {
					setPage(curpage + 1);
					LeftInSegment = VGABytesPerPage;
					video = (char *)(VGABASEADDRESS);
				}

				*video = *imdata;
				video++;
				imdata++;
				LeftInSegment--;
			}
		}
	}

#endif
}




/*****************************************************************************/
/* Draws an image to the screen.                                             */
/*****************************************************************************/
void drawMaskImage(struct Image *Im, uint16 x, uint16 y) {
#if !defined(DOSCODE)
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

	if ((uint)(dx + w) > VGAScreenWidth)
		w = VGAScreenWidth - dx;

	if ((uint)(dy + h) > VGAScreenHeight)
		h = VGAScreenHeight - dy;

	if (w > 0 && h > 0) {
		byte *s = Im->ImageData + sy * Im->Width + sx;
		byte *d = getVGABaseAddr() + dy * VGAScreenWidth + dx;

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
			d += VGAScreenWidth;
		}

		ungetVGABaseAddr();
	}

#else
	uint32  RealOffset,
	         SegmentOffset,
	         LeftInSegment;
	char *video,
	              *curline,
	              *imdata = Im->ImageData;
	uint16 counterx,
	         countery = 0,
	         numwholelines,
	         numpartiallines,
	         curpage;

	while (countery < Im->Height) {
		RealOffset = (VGAScreenWidth * (y + countery)) + x;
		curpage    = RealOffset / VGABytesPerPage;
		SegmentOffset = RealOffset - (curpage * VGABytesPerPage);
		LeftInSegment = VGABytesPerPage - SegmentOffset;
		setPage(curpage);
		video = (char *)(((int32)(VGABASEADDRESS)) + SegmentOffset);

		numwholelines = LeftInSegment / VGAScreenWidth;
		numpartiallines = 0;
		counterx      = LeftInSegment - (numwholelines * VGAScreenWidth);

		if (counterx >= Im->Width)
			numwholelines++;
		else
			numpartiallines = 1;

		while (numwholelines && (countery < Im->Height)) {
			curline = video;

			for (counterx = 0; counterx < Im->Width; counterx++) {
				if (*imdata)
					*video = *imdata - 1;

				video++;
				imdata++;
			}

			video = curline;
			video += VGAScreenWidth;
			countery ++;
			numwholelines --;
			LeftInSegment -= VGAScreenWidth;
		}

		if (numpartiallines && (countery < Im->Height)) {
			countery ++;
			curline = video;

			for (counterx = 0; counterx < Im->Width; counterx++) {
				if (LeftInSegment == 0L) {
					setPage(curpage + 1);
					LeftInSegment = VGABytesPerPage;
					video = (char *)(VGABASEADDRESS);
				}

				if (*imdata)
					*video = *imdata - 1;

				video++;
				imdata++;
				LeftInSegment--;
			}
		}
	}

#endif
}




/*****************************************************************************/
/* Reads an image from the screen.                                           */
/*****************************************************************************/
void readScreenImage(struct Image *Im, uint16 x, uint16 y) {
#if !defined(DOSCODE)
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

	if ((uint)(dx + w) > VGAScreenWidth)
		w = VGAScreenWidth - dx;

	if ((uint)(dy + h) > VGAScreenHeight)
		h = VGAScreenHeight - dy;

	if (w > 0 && h > 0) {
		byte *s = Im->ImageData + sy * Im->Width + sx;
		byte *d = getVGABaseAddr() + dy * VGAScreenWidth + dx;

		while (h-- > 0) {
			memcpy(s, d, w);
			s += Im->Width;
			d += VGAScreenWidth;
		}

		ungetVGABaseAddr();
	}

#else
	uint32  RealOffset,
	         SegmentOffset,
	         LeftInSegment;
	char *video,
	              *curline,
	              *imdata = Im->ImageData;
	uint16 counterx,
	         countery = 0,
	         numwholelines,
	         numpartiallines,
	         curpage;

	while (countery < Im->Height) {
		RealOffset = (VGAScreenWidth * (y + countery)) + x;
		curpage    = RealOffset / VGABytesPerPage;
		SegmentOffset = RealOffset - (curpage * VGABytesPerPage);
		LeftInSegment = VGABytesPerPage - SegmentOffset;
		setPage(curpage);
		video = (char *)(((int32)(VGABASEADDRESS)) + SegmentOffset);

		numwholelines = LeftInSegment / VGAScreenWidth;
		numpartiallines = 0;
		counterx      = LeftInSegment - (numwholelines * VGAScreenWidth);

		if (counterx >= Im->Width)
			numwholelines++;
		else
			numpartiallines = 1;

		while (numwholelines && (countery < Im->Height)) {
			curline = video;

			for (counterx = 0; counterx < Im->Width; counterx++) {
				*imdata = *video;
				video++;
				imdata++;
			}

			video = curline;
			video += VGAScreenWidth;
			countery ++;
			numwholelines --;
			LeftInSegment -= VGAScreenWidth;
		}

		if (numpartiallines && (countery < Im->Height)) {
			countery ++;
			curline = video;

			for (counterx = 0; counterx < Im->Width; counterx++) {
				if (LeftInSegment == 0L) {
					setPage(curpage + 1);
					LeftInSegment = VGABytesPerPage;
					video = (char *)(VGABASEADDRESS);
				}

				*imdata = *video;
				video++;
				imdata++;
				LeftInSegment--;
			}
		}
	}

#endif
}




/*****************************************************************************/
/* Blits a piece of one image to another.                                    */
/* NOTE: for our purposes, assumes that ImDest is to be in VGA memory.       */
/*****************************************************************************/
void bltBitMap(Image *ImSource, uint16 xs, uint16 ys, Image *ImDest,
					uint16 xd, uint16 yd, uint16 width, uint16 height) {
#if !defined(DOSCODE)
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

#else
	uint32  RealOffset,
	         SegmentOffset,
	         LeftInSegment;
	char *video,
	              *curdestline,
	              *cursourceline = ImSource->ImageData,
	               *imdata;
	uint16 counterx,
	         countery = 0,
	         numwholelines,
	         numpartiallines,
	         curpage;

	cursourceline += (((int32) ys) * ImSource->Width) + xs;
	imdata        = cursourceline;

	while (countery < height) {
		RealOffset = (ImDest->Width * (yd + countery)) + xd;
		curpage    = RealOffset / VGABytesPerPage;
		SegmentOffset = RealOffset - (curpage * VGABytesPerPage);
		LeftInSegment = VGABytesPerPage - SegmentOffset;
		setPage(curpage);

		video = (char *)(((int32)(ImDest->ImageData)) + SegmentOffset);

		numwholelines = LeftInSegment / ImDest->Width;
		numpartiallines = 0;
		counterx      = LeftInSegment - (numwholelines * ImDest->Width);

		if (counterx >= width)
			numwholelines++;
		else
			numpartiallines = 1;

		while (numwholelines && (countery < height)) {
			curdestline = video;
			cursourceline = imdata;

			for (counterx = 0; counterx < width; counterx++) {
				*video = *imdata;
				video++;
				imdata++;
			}

			video = curdestline;
			video += ImDest->Width;
			imdata = cursourceline;
			imdata += ImSource->Width;

			countery ++;
			numwholelines --;
			LeftInSegment -= ImDest->Width;
		}

		if (numpartiallines && (countery < height)) {
			countery ++;
			curdestline = video;
			cursourceline = imdata;

			for (counterx = 0; counterx < width; counterx++) {
				if (LeftInSegment == 0L) {
					setPage(curpage + 1);
					LeftInSegment = VGABytesPerPage;
					video = ImDest->ImageData;
				}

				*video = *imdata;
				video++;
				imdata++;
				LeftInSegment--;
			}

			video = curdestline;
			video += ImDest->Width;
			imdata = cursourceline;
			imdata += ImSource->Width;
		}
	}

#endif
}


byte *TempScrollData;


/*****************************************************************************/
/* Scrolls the display in the x direction by blitting.                       */
/* The TempScrollData variable must be initialized to some memory, or this   */
/* function will fail.                                                       */
/*****************************************************************************/
void scrollDisplayX(int16 dx, uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	struct Image Im;
	uint16 temp;

	Im.ImageData = TempScrollData;

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
	struct Image Im;
	uint16 temp;

	Im.ImageData = TempScrollData;

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



static unsigned char curapen = 0;

/*****************************************************************************/
/* Sets the pen number to use on all the drawing operations.                 */
/*****************************************************************************/
void setAPen(uint16 pennum) {
	curapen = (unsigned char)pennum;
}




/*****************************************************************************/
/* Fills in a rectangle.                                                     */
/*****************************************************************************/
void rectFill(uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
#if !defined(DOSCODE)
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

	if ((uint)(dx + w) > VGAScreenWidth)
		w = VGAScreenWidth - dx;

	if ((uint)(dy + h) > VGAScreenHeight)
		h = VGAScreenHeight - dy;

	if (w > 0 && h > 0) {
		char *d = (char *)getVGABaseAddr() + dy * VGAScreenWidth + dx;

		while (h-- > 0) {
			char *dd = d;
			int ww = w;

			while (ww-- > 0) {
				*dd++ = curapen;
			}

			d += VGAScreenWidth;
		}

		ungetVGABaseAddr();
	}

#else
	uint32  RealOffset,
	         SegmentOffset,
	         LeftInSegment;
	char *video,
	              *curline;
	uint16 counterx,
	         countery = y1,
	         numwholelines,
	         numpartiallines,
	         curpage;

	while (countery <= y2) {
		RealOffset = (VGAScreenWidth * countery) + x1;
		curpage    = RealOffset / VGABytesPerPage;
		SegmentOffset = RealOffset - (curpage * VGABytesPerPage);
		LeftInSegment = VGABytesPerPage - SegmentOffset;
		setPage(curpage);
		video = (char *)(((int32)(VGABASEADDRESS)) + SegmentOffset);

		numwholelines = LeftInSegment / VGAScreenWidth;
		numpartiallines = 0;
		counterx      = LeftInSegment - (numwholelines * VGAScreenWidth);

		if (counterx >= (x2 - x1 + 1))
			numwholelines++;
		else
			numpartiallines = 1;

		while (numwholelines && (countery <= y2)) {
			curline = video;

			for (counterx = x1; counterx <= x2; counterx++) {
				*video = curapen;
				video++;
			}

			video = curline;
			video += VGAScreenWidth;
			countery ++;
			numwholelines --;
			LeftInSegment -= VGAScreenWidth;
		}

		if (numpartiallines && (countery <= y2)) {
			countery ++;
			curline = video;

			for (counterx = x1; counterx <= x2; counterx++) {
				if (LeftInSegment == 0L) {
					setPage(curpage + 1);
					LeftInSegment = VGABytesPerPage;
					video = (char *)(VGABASEADDRESS);
				}

				*video = curapen;
				video++;
				LeftInSegment--;
			}
		}
	}

#endif
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
#if !defined(DOSCODE)
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

	if ((uint)(dx + w) > VGAScreenWidth)
		w = VGAScreenWidth - dx;

	if ((uint)(dy + h) > VGAScreenHeight)
		h = VGAScreenHeight - dy;

	if (w > 0 && h > 0) {
		char *d = (char *)getVGABaseAddr() + dy * VGAScreenWidth + dx;

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

			d += VGAScreenWidth;
			dy++;
		}

		ungetVGABaseAddr();
	}

#else
	uint32  RealOffset,
	         SegmentOffset;
	int32           LeftInSegment;
	char *video,
	              *curline;
	uint16 counterx,
	         countery = y1,
	         numwholelines,
	         numpartiallines,
	         curpage;

	while (countery <= y2) {
		RealOffset = (VGAScreenWidth * countery) + x1;
		curpage    = RealOffset / VGABytesPerPage;
		SegmentOffset = RealOffset - (curpage * VGABytesPerPage);
		LeftInSegment = VGABytesPerPage - SegmentOffset;
		setPage(curpage);
		video = (char *)(((int32)(VGABASEADDRESS)) + SegmentOffset);

		numwholelines = LeftInSegment / VGAScreenWidth;
		numpartiallines = 0;
		counterx      = LeftInSegment - (numwholelines * VGAScreenWidth);

		if (counterx >= (x2 - x1 + 1))
			numwholelines++;
		else
			numpartiallines = 1;

		while (numwholelines && (countery <= y2)) {
			curline = video;
			counterx = x1;

			if (1 & countery) {
				video++;
				counterx ++;
			}

			while (counterx <= x2) {
				*video = pencolor;
				video += 2;
				counterx += 2;
			}

			video = curline;
			video += VGAScreenWidth;
			countery ++;
			numwholelines --;
			LeftInSegment -= VGAScreenWidth;
		}

		if (numpartiallines && (countery <= y2)) {
			countery ++;
			curline = video;
			counterx = x1;

			if (1 & countery) {
				video++;
				counterx ++;
				LeftInSegment --;
			}

			while (counterx < x2) {
				if (LeftInSegment <= 0L) {
					setPage(curpage + 1);
					video = (char *)(((int32)(VGABASEADDRESS)) - LeftInSegment);
					LeftInSegment = VGABytesPerPage + LeftInSegment;
				}

				*video = pencolor;
				video += 2;
				counterx += 2;
				LeftInSegment -= 2;
			}
		}
	}

#endif
}

} // End of namespace Lab
