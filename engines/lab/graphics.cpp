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
#include "lab/anim.h"
#include "lab/parsetypes.h"
#include "lab/image.h"
#include "lab/labfun.h"
#include "lab/parsefun.h"
#include "lab/text.h"
#include "lab/resource.h"

namespace Lab {

BitMap bit1, bit2, *DispBitMap = &bit1, *DrawBitMap = &bit1;

extern bool stopsound;

/*****************************************************************************/
/* Scales the x co-ordinates to that of the new display.  In the room parser */
/* file, co-ordinates are set up on a 360x336 display.                       */
/*****************************************************************************/
uint16 scaleX(uint16 x) {
	if (g_lab->_isHiRes)
		return (uint16)((x * 16) / 9);
	else
		return (uint16)((x * 8) / 9);
}

/*****************************************************************************/
/* Scales the y co-ordinates to that of the new display.  In the room parser */
/* file, co-ordinates are set up on a 368x336 display.                       */
/*****************************************************************************/
uint16 scaleY(uint16 y) {
	if (g_lab->_isHiRes)
		return (y + (y / 14));
	else
		return ((y * 10) / 24);
}

/*****************************************************************************/
/* Scales the VGA cords to SVGA if necessary; otherwise, returns VGA cords.  */
/*****************************************************************************/
int16 VGAScaleX(int16 x) {
	if (g_lab->_isHiRes)
		return (x * 2);
	else
		return x;
}

/*****************************************************************************/
/* Scales the VGA cords to SVGA if necessary; otherwise, returns VGA cords.  */
/*****************************************************************************/
int16 VGAScaleY(int16 y) {
	if (g_lab->_isHiRes)
		return ((y * 12) / 5);
	else
		return y;
}

uint16 SVGACord(uint16 cord) {
	if (g_lab->_isHiRes)
		return cord;
	else
		return 0;
}

/*****************************************************************************/
/* Converts SVGA cords to VGA if necessary, otherwise returns VGA cords.     */
/*****************************************************************************/
int VGAUnScaleX(int x) {
	if (g_lab->_isHiRes)
		return (x / 2);
	else
		return x;
}

/*****************************************************************************/
/* Converts SVGA cords to VGA if necessary, otherwise returns VGA cords.     */
/*****************************************************************************/
int VGAUnScaleY(int y) {
	if (g_lab->_isHiRes)
		return ((y * 5) / 12);
	else
		return y;
}

/*---------------------------------------------------------------------------*/
/*------ From readPict.c.  Reads in pictures and animations from disk. ------*/
/*---------------------------------------------------------------------------*/

/*****************************************************************************/
/* Reads in a picture into the dest bitmap.                                  */
/*****************************************************************************/
bool readPict(const char *filename, bool playOnce) {
	g_lab->_anim->stopDiff();

	byte **file = g_lab->_music->newOpen(filename);

	if (file == NULL) {
		if ((filename[0] == 'p') || (filename[0] == 'P'))
			blackScreen();

		return false;
	}

	DispBitMap->_bytesPerRow = g_lab->_screenWidth;
	DispBitMap->_rows        = g_lab->_screenHeight;
	DispBitMap->_flags       = BITMAPF_VIDEO;

	g_lab->_anim->readDiff(playOnce);

	return true;
}

/*****************************************************************************/
/* Reads in a music file.  Ignores any graphics.                             */
/*****************************************************************************/
bool readMusic(const char *filename, bool waitTillFinished) {
	Common::File *file = g_lab->_resource->openDataFile(filename, MKTAG('D', 'I', 'F', 'F'));
	g_lab->_music->updateMusic();
	if (!g_lab->_music->_doNotFilestopSoundEffect)
		g_lab->_music->stopSoundEffect();
	if (!file)
		return false;

	g_lab->_anim->_doBlack = false;
	g_lab->_anim->readSound(waitTillFinished, file);

	return true;
}

/*****************************************************************************/
/* Reads in a picture into buffer memory.                                    */
/*****************************************************************************/
byte *readPictToMem(const char *filename, uint16 x, uint16 y) {
	byte *mem;

	g_lab->_anim->stopDiff();

	allocFile((void **)&mem, (int32)x * (int32)y, "Bitmap");
	byte *curMem = mem;

	byte **file = g_lab->_music->newOpen(filename);

	if (file == NULL)
		return NULL;

	DispBitMap->_bytesPerRow = x;
	DispBitMap->_rows        = y;
	DispBitMap->_flags       = BITMAPF_NONE;
	DispBitMap->_planes[0] = curMem;
	DispBitMap->_planes[1] = DispBitMap->_planes[0] + 0x10000;
	DispBitMap->_planes[2] = DispBitMap->_planes[1] + 0x10000;
	DispBitMap->_planes[3] = DispBitMap->_planes[2] + 0x10000;
	DispBitMap->_planes[4] = DispBitMap->_planes[3] + 0x10000;

	g_lab->_anim->readDiff(true);

	return mem;
}

/*---------------------------------------------------------------------------*/
/*------------ Does all the text rendering to the message boxes. ------------*/
/*---------------------------------------------------------------------------*/
bool DoNotDrawMessage = false;

/*----- The flowText routines -----*/

/******************************************************************************/
/* Extracts the first word from a string.                                     */
/******************************************************************************/
static void getWord(char *wordBuffer, const char *mainBuffer, uint16 *wordWidth) {
	uint16 width = 0;

	while ((mainBuffer[width] != ' ') && mainBuffer[width] &&
	        (mainBuffer[width] != '\n')) {
		wordBuffer[width] = mainBuffer[width];
		width++;
	}

	wordBuffer[width] = 0;

	*wordWidth = width;
}

/******************************************************************************/
/* Gets a line of text for flowText; makes sure that its length is less than  */
/* or equal to the maximum width.                                             */
/******************************************************************************/
static void getLine(TextFont *tf, char *lineBuffer, const char **mainBuffer, uint16 lineWidth) {
	uint16 curWidth = 0, wordWidth;
	char wordBuffer[100];
	bool doit = true;

	lineWidth += textLength(tf, " ", 1);

	lineBuffer[0] = 0;

	while ((*mainBuffer)[0] && doit) {
		getWord(wordBuffer, *mainBuffer, &wordWidth);
		strcat(wordBuffer, " ");

		if ((curWidth + textLength(tf, wordBuffer, wordWidth + 1)) <= lineWidth) {
			strcat(lineBuffer, wordBuffer);
			(*mainBuffer) += wordWidth;

			if ((*mainBuffer)[0] == '\n')
				doit = false;

			if ((*mainBuffer)[0])
				(*mainBuffer)++;

			curWidth = textLength(tf, lineBuffer, strlen(lineBuffer));
		} else
			doit = false;
	}
}

/******************************************************************************/
/* Dumps a chunk of text to an arbitrary box; flows it within that box and    */
/* optionally centers it. Returns the number of characters that were          */
/* processed.                                                                 */
/*                                                                            */
/* Note: Every individual word MUST be int16 enough to fit on a line, and     */
/* each line less than 255 characters.                                        */
/******************************************************************************/
uint32 flowText(void *font,      /* the TextAttr pointer */
                int16 spacing,          /* How much vertical spacing between the lines */
                byte pencolor,         /* pen number to use for text */
                byte backpen,          /* the background color */
                bool fillback,                /* Whether to fill the background */
                bool centerh,                 /* Whether to center the text horizontally */
                bool centerv,                 /* Whether to center the text vertically */
                bool output,                  /* Whether to output any text */
                uint16 x1,               /* Cords */
                uint16 y1, uint16 x2, uint16 y2, const char *str) { /* The text itself */
	TextFont *_msgFont = (TextFont *)font;
	char linebuffer[256];
	const char *temp;
	uint16 numlines, actlines, fontheight, width;
	uint16 x, y;

	if (fillback) {
		g_lab->setAPen(backpen);
		g_lab->rectFill(x1, y1, x2, y2);
	}

	if (str == NULL)
		return 0L;

	g_lab->setAPen(pencolor);

	fontheight = textHeight(_msgFont) + spacing;
	numlines   = (y2 - y1 + 1) / fontheight;
	width      = x2 - x1 + 1;
	y          = y1;

	if (centerv && output) {
		temp = str;
		actlines = 0;

		while (temp[0]) {
			getLine(_msgFont, linebuffer, &temp, width);
			actlines++;
		}

		if (actlines <= numlines)
			y += ((y2 - y1 + 1) - (actlines * fontheight)) / 2;
	}

	temp = str;

	while (numlines && str[0]) {
		getLine(_msgFont, linebuffer, &str, width);

		x = x1;

		if (centerh)
			x += (width - textLength(_msgFont, linebuffer, strlen(linebuffer))) / 2;

		if (output)
			text(_msgFont, x, y, pencolor, linebuffer, strlen(linebuffer));

		numlines--;
		y += fontheight;
	}

	return (str - temp);
}

/******************************************************************************/
/* Calls flowText, but flows it to memory.  Same restrictions as flowText.    */
/******************************************************************************/
uint32 flowTextToMem(Image *destIm, void *font,     /* the TextAttr pointer */
                     int16 spacing,          /* How much vertical spacing between the lines */
                     byte pencolor,         /* pen number to use for text */
                     byte backpen,          /* the background color */
                     bool fillback,                /* Whether to fill the background */
                     bool centerh,                 /* Whether to center the text horizontally */
                     bool centerv,                 /* Whether to center the text vertically */
                     bool output,                  /* Whether to output any text */
                     uint16 x1,               /* Cords */
                     uint16 y1, uint16 x2, uint16 y2, const char *str) { /* The text itself */
	uint32 res, vgabyte = g_lab->_screenBytesPerPage;
	byte *tmp = g_lab->_currentDisplayBuffer;

	g_lab->_currentDisplayBuffer = destIm->_imageData;
	g_lab->_screenBytesPerPage = (uint32)destIm->_width * (int32)destIm->_height;

	res = flowText(font, spacing, pencolor, backpen, fillback, centerh, centerv, output, x1, y1, x2, y2, str);

	g_lab->_screenBytesPerPage = vgabyte;
	g_lab->_currentDisplayBuffer = tmp;

	return res;
}

/*----- The control panel stuff -----*/

void createBox(uint16 y2) {
	g_lab->setAPen(7);                 /* Message box area */
	g_lab->rectFill(VGAScaleX(4), VGAScaleY(154), VGAScaleX(315), VGAScaleY(y2 - 2));

	g_lab->setAPen(0);                 /* Box around message area */
	g_lab->drawHLine(VGAScaleX(2), VGAScaleY(152), VGAScaleX(317));
	g_lab->drawVLine(VGAScaleX(317), VGAScaleY(152), VGAScaleY(y2));
	g_lab->drawHLine(VGAScaleX(2), VGAScaleY(y2), VGAScaleX(317));
	g_lab->drawVLine(VGAScaleX(2), VGAScaleY(152), VGAScaleY(y2));
}

int32 LabEngine::longDrawMessage(const char *str) {
	char newText[512];

	if (str == NULL)
		return 0;

	_event->attachGadgetList(NULL);
	_event->mouseHide();
	strcpy(newText, str);

	if (!_longWinInFront) {
		_longWinInFront = true;
		setAPen(3);                 /* Clear Area */
		rectFill(0, VGAScaleY(149) + SVGACord(2), VGAScaleX(319), VGAScaleY(199));
	}

	createBox(198);
	_event->mouseShow();

	return flowText(_msgFont, 0, 1, 7, false, true, true, true, VGAScaleX(6), VGAScaleY(155), VGAScaleX(313), VGAScaleY(195), str);
}

void LabEngine::drawStaticMessage(byte index) {
	drawMessage(_resource->getStaticText((StaticText)index).c_str());
}

/******************************************************************************/
/* Draws a message to the message box.                                        */
/******************************************************************************/
void LabEngine::drawMessage(const char *str) {
	if (DoNotDrawMessage) {
		DoNotDrawMessage = false;
		return;
	}

	if (str) {
		if ((textLength(_msgFont, str, strlen(str)) > VGAScaleX(306))) {
			longDrawMessage(str);
			_lastMessageLong = true;
		} else {
			if (_longWinInFront) {
				_longWinInFront = false;
				drawPanel();
			}

			_event->mouseHide();
			createBox(168);
			text(_msgFont, VGAScaleX(7), VGAScaleY(155) + SVGACord(2), 1, str, strlen(str));
			_event->mouseShow();
			_lastMessageLong = false;
		}
	}
}

/*---------------------------------------------------------------------------*/
/*--------------------------- All the wipe stuff. ---------------------------*/
/*---------------------------------------------------------------------------*/

#define TRANSWIPE      1
#define SCROLLWIPE     2
#define SCROLLBLACK    3
#define SCROLLBOUNCE   4
#define TRANSPORTER    5
#define READFIRSTFRAME 6
#define READNEXTFRAME  7

/*****************************************************************************/
/* Scrolls the display to black.                                             */
/*****************************************************************************/
void LabEngine::doScrollBlack() {
	byte *mem, *tempmem;
	Image im;
	uint32 size, copysize;
	uint32 *baseAddr;

	_event->mouseHide();
	uint16 width = VGAScaleX(320);
	uint16 height = VGAScaleY(149) + SVGACord(2);

	allocFile((void **)&mem, (int32)width * (int32)height, "Temp Mem");

	im._width = width;
	im._height = height;
	im._imageData = mem;
	_music->updateMusic();
	im.readScreenImage(0, 0);
	_music->updateMusic();

	baseAddr = (uint32 *)getCurrentDrawingBuffer();

	uint16 by      = VGAScaleX(4);
	uint16 nheight = height;

	while (nheight) {
		_music->updateMusic();

		if (!_isHiRes)
			waitTOF();

		baseAddr = (uint32 *)getCurrentDrawingBuffer();

		if (by > nheight)
			by = nheight;

		mem += by * width;
		nheight -= by;
		size = (int32)nheight * (int32)width;
		tempmem = mem;

		while (size) {
			if (size > _screenBytesPerPage)
				copysize = _screenBytesPerPage;
			else
				copysize = size;

			size -= copysize;

			memcpy(baseAddr, tempmem, copysize);
			tempmem += copysize;
		}

		setAPen(0);
		rectFill(0, nheight, width - 1, nheight + by - 1);

		screenUpdate();

		if (!_isHiRes) {
			if (nheight <= (height / 8))
				by = 1;
			else if (nheight <= (height / 4))
				by = 2;
			else if (nheight <= (height / 2))
				by = 3;
		}
	}

	freeAllStolenMem();
	_event->mouseShow();
}

static void copyPage(uint16 width, uint16 height, uint16 nheight, uint16 startline, byte *mem) {
	uint32 size, offSet, copysize;
	uint16 curPage;
	uint32 *baseAddr;

	baseAddr = (uint32 *)g_lab->getCurrentDrawingBuffer();

	size = (int32)(height - nheight) * (int32)width;
	mem += startline * width;
	curPage = ((int32)nheight * (int32)width) / g_lab->_screenBytesPerPage;
	offSet = ((int32)nheight * (int32)width) - (curPage * g_lab->_screenBytesPerPage);

	while (size) {
		if (size > (g_lab->_screenBytesPerPage - offSet))
			copysize = g_lab->_screenBytesPerPage - offSet;
		else
			copysize = size;

		size -= copysize;

		memcpy(baseAddr + (offSet >> 2), mem, copysize);
		mem += copysize;
		curPage++;
		offSet = 0;
	}
}

/*****************************************************************************/
/* Scrolls the display to a new picture from a black screen.                 */
/*****************************************************************************/
void LabEngine::doScrollWipe(char *filename) {
	uint16 startline = 0, onrow = 0;

	_event->mouseHide();
	uint16 width = VGAScaleX(320);
	uint16 height = VGAScaleY(149) + SVGACord(2);

	while (_music->isSoundEffectActive()) {
		_music->updateMusic();
		waitTOF();
	}

	_anim->_isBM = true;
	readPict(filename, true);
	setPalette(_anim->_diffPalette, 256);
	_anim->_isBM = false;
	byte *mem = _anim->_rawDiffBM._planes[0];

	_music->updateMusic();
	uint16 by = VGAScaleX(3);
	uint16 nheight = height;

	while (onrow < _anim->_headerdata._height) {
		_music->updateMusic();

		if ((by > nheight) && nheight)
			by = nheight;

		if ((startline + by) > (_anim->_headerdata._height - height - 1))
			break;

		if (nheight)
			nheight -= by;

		copyPage(width, height, nheight, startline, mem);

		screenUpdate();

		if (!nheight)
			startline += by;

		onrow += by;

		if (nheight <= (height / 4))
			by = VGAScaleX(5);
		else if (nheight <= (height / 3))
			by = VGAScaleX(4);
		else if (nheight <= (height / 2))
			by = VGAScaleX(3);
	}

	_event->mouseShow();
}

/*****************************************************************************/
/* Does the scroll bounce.  Assumes bitmap already in memory.                */
/*****************************************************************************/
void LabEngine::doScrollBounce() {
	const uint16 *newby, *newby1;

	const uint16 newbyd[5] = {5, 4, 3, 2, 1}, newby1d[8] = {3, 3, 2, 2, 2, 1, 1, 1};
	const uint16 newbyw[5] = {10, 8, 6, 4, 2}, newby1w[8] = {6, 6, 4, 4, 4, 2, 2, 2};

	if (getPlatform() != Common::kPlatformWindows) {
		newby = newbyd;
		newby1 = newby1d;
	} else {
		newby = newbyw;
		newby1 = newby1w;
	}

	_event->mouseHide();
	int width = VGAScaleX(320);
	int height = VGAScaleY(149) + SVGACord(2);
	byte *mem = _anim->_rawDiffBM._planes[0];

	_music->updateMusic();
	int startline = _anim->_headerdata._height - height - 1;

	for (int i = 0; i < 5; i++) {
		_music->updateMusic();
		startline -= newby[i];
		copyPage(width, height, 0, startline, mem);

		screenUpdate();
		waitTOF();
	}

	for (int i = 8; i > 0; i--) {
		_music->updateMusic();
		startline += newby1[i - 1];
		copyPage(width, height, 0, startline, mem);

		screenUpdate();
		waitTOF();

	}

	_event->mouseShow();
}

/*****************************************************************************/
/* Does the transporter wipe.                                                */
/*****************************************************************************/
void LabEngine::doTransWipe(CloseDataPtr *cPtr, char *filename) {
	uint16 lastY, curY, linesdone = 0, lineslast;
	Image imSource, imDest;

	if (_isHiRes) {
		lineslast = 3;
		lastY = 358;
	} else {
		lineslast = 1;
		lastY = 148;
	}

	for (uint16 i = 0; i < 2; i++) {
		curY = i * 2;

		while (curY < lastY) {
			if (linesdone >= lineslast) {
				_music->updateMusic();
				waitTOF();
				linesdone = 0;
			}

			overlayRect(0, 0, curY, _screenWidth - 1, curY + 1);
			curY += 4;
			linesdone++;
		}
	}

	setAPen(0);

	for (uint16 i = 0; i < 2; i++) {
		curY = i * 2;

		while (curY <= lastY) {
			if (linesdone >= lineslast) {
				_music->updateMusic();
				waitTOF();
				linesdone = 0;
			}

			rectFill(0, curY, _screenWidth - 1, curY + 1);
			curY += 4;
			linesdone++;
		}
	}

	if (filename == NULL)
		g_lab->_curFileName = getPictName(cPtr);
	else if (filename[0] > ' ')
		g_lab->_curFileName = filename;
	else
		g_lab->_curFileName = getPictName(cPtr);

	byte *BitMapMem = readPictToMem(g_lab->_curFileName, _screenWidth, lastY + 5);
	setPalette(_anim->_diffPalette, 256);

	if (BitMapMem) {
		imSource._width = _screenWidth;
		imSource._height = lastY;
		imSource._imageData = BitMapMem;

		imDest._width = _screenWidth;
		imDest._height = _screenHeight;
		imDest._imageData = getCurrentDrawingBuffer();

		for (uint16 i = 0; i < 2; i++) {
			curY = i * 2;

			while (curY < lastY) {
				if (linesdone >= lineslast) {
					_music->updateMusic();
					waitTOF();
					linesdone = 0;
				}

				imDest._imageData = getCurrentDrawingBuffer();

				imSource.bltBitMap(0, curY, &imDest, 0, curY, _screenWidth, 2);
				overlayRect(0, 0, curY, _screenWidth - 1, curY + 1);
				curY += 4;
				linesdone++;
			}
		}

		for (uint16 i = 0; i < 2; i++) {
			curY = i * 2;

			while (curY <= lastY) {
				if (linesdone >= lineslast) {
					_music->updateMusic();
					waitTOF();
					linesdone = 0;
				}

				imDest._imageData = getCurrentDrawingBuffer();

				if (curY == lastY)
					imSource.bltBitMap(0, curY, &imDest, 0, curY, _screenWidth, 1);
				else
					imSource.bltBitMap(0, curY, &imDest, 0, curY, _screenWidth, 2);

				curY += 4;
				linesdone++;
			}
		}
	}
}

/*****************************************************************************/
/* Does a certain number of pre-programmed wipes.                            */
/*****************************************************************************/
void LabEngine::doWipe(uint16 wipeType, CloseDataPtr *cPtr, char *filename) {
	if ((wipeType == TRANSWIPE) || (wipeType == TRANSPORTER))
		doTransWipe(cPtr, filename);
	else if (wipeType == SCROLLWIPE)
		doScrollWipe(filename);
	else if (wipeType == SCROLLBLACK)
		doScrollBlack();
	else if (wipeType == SCROLLBOUNCE)
		doScrollBounce();
	else if (wipeType == READFIRSTFRAME)
		readPict(filename, false);
	else if (wipeType == READNEXTFRAME)
		_anim->diffNextFrame();
}

/*****************************************************************************/
/* Changes the front screen to black.                                        */
/*****************************************************************************/
void blackScreen() {
	byte pal[256 * 3];
	memset(pal, 0, 248 * 3);
	g_lab->writeColorRegs(pal, 8, 248);

	g_system->delayMillis(32);
}

/*****************************************************************************/
/* Changes the front screen to white.                                        */
/*****************************************************************************/
void whiteScreen() {
	byte pal[256 * 3];
	memset(pal, 255, 248 * 3);
	g_lab->writeColorRegs(pal, 8, 248);
}

/*****************************************************************************/
/* Changes the entire screen to black.                                       */
/*****************************************************************************/
void blackAllScreen() {
	byte pal[256 * 3];
	memset(pal, 0, 256 * 3);
	g_lab->writeColorRegs(pal, 0, 256);

	g_system->delayMillis(32);
}

} // End of namespace Lab
