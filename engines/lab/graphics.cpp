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
#include "lab/graphics.h"

namespace Lab {

BitMap bit1, bit2, *DispBitMap = &bit1, *DrawBitMap = &bit1;

DisplayMan::DisplayMan(LabEngine *vm) : _vm(vm) {
	_longWinInFront = false;
	_lastMessageLong = false;
	_doNotDrawMessage = false;

	_screenBytesPerPage = 65536;
	_curapen = 0;
	_curBitmap = nullptr;
	_displayBuffer = nullptr;
	_currentDisplayBuffer = nullptr;
	_tempScrollData = nullptr;

	_screenWidth = 0;
	_screenHeight = 0;

	for (int i = 0; i < 256 * 3; i++)
		_curvgapal[i] = 0;
}

DisplayMan::~DisplayMan() {
	freePict();
}

/*****************************************************************************/
/* Scales the x co-ordinates to that of the new display.  In the room parser */
/* file, co-ordinates are set up on a 360x336 display.                       */
/*****************************************************************************/
uint16 DisplayMan::scaleX(uint16 x) {
	if (_vm->_isHiRes)
		return (uint16)((x * 16) / 9);
	else
		return (uint16)((x * 8) / 9);
}

/*****************************************************************************/
/* Scales the y co-ordinates to that of the new display.  In the room parser */
/* file, co-ordinates are set up on a 368x336 display.                       */
/*****************************************************************************/
uint16 DisplayMan::scaleY(uint16 y) {
	if (_vm->_isHiRes)
		return (y + (y / 14));
	else
		return ((y * 10) / 24);
}

/*****************************************************************************/
/* Scales the VGA cords to SVGA if necessary; otherwise, returns VGA cords.  */
/*****************************************************************************/
int16 DisplayMan::VGAScaleX(int16 x) {
	if (_vm->_isHiRes)
		return (x * 2);
	else
		return x;
}

/*****************************************************************************/
/* Scales the VGA cords to SVGA if necessary; otherwise, returns VGA cords.  */
/*****************************************************************************/
int16 DisplayMan::VGAScaleY(int16 y) {
	if (_vm->_isHiRes)
		return ((y * 12) / 5);
	else
		return y;
}

uint16 DisplayMan::SVGACord(uint16 cord) {
	if (_vm->_isHiRes)
		return cord;
	else
		return 0;
}

/*---------------------------------------------------------------------------*/
/*------ From readPict.c.  Reads in pictures and animations from disk. ------*/
/*---------------------------------------------------------------------------*/

void DisplayMan::loadPict(const char *filename) {
	Common::File *bitmapFile = _vm->_resource->openDataFile(filename);
	freePict();
	_curBitmap = new byte[bitmapFile->size()];
	bitmapFile->read(_curBitmap, bitmapFile->size());
	delete bitmapFile;
}

/*****************************************************************************/
/* Reads in a picture into the dest bitmap.                                  */
/*****************************************************************************/
bool DisplayMan::readPict(const char *filename, bool playOnce) {
	_vm->_anim->stopDiff();

	loadPict(filename);

	_vm->_music->updateMusic();

	if (!_vm->_music->_doNotFilestopSoundEffect)
		_vm->_music->stopSoundEffect();

	DispBitMap->_bytesPerRow = _screenWidth;
	DispBitMap->_rows        = _screenHeight;
	DispBitMap->_flags       = BITMAPF_VIDEO;

	_vm->_anim->readDiff(_curBitmap, playOnce);

	return true;
}

/*****************************************************************************/
/* Reads in a picture into buffer memory.                                    */
/*****************************************************************************/
byte *DisplayMan::readPictToMem(const char *filename, uint16 x, uint16 y) {
	_vm->_anim->stopDiff();

	loadPict(filename);

	_vm->_music->updateMusic();

	if (!_vm->_music->_doNotFilestopSoundEffect)
		_vm->_music->stopSoundEffect();

	DispBitMap->_bytesPerRow = x;
	DispBitMap->_rows = y;
	DispBitMap->_flags = BITMAPF_NONE;
	DispBitMap->_planes[0] = _curBitmap;
	DispBitMap->_planes[1] = DispBitMap->_planes[0] + 0x10000;
	DispBitMap->_planes[2] = DispBitMap->_planes[1] + 0x10000;
	DispBitMap->_planes[3] = DispBitMap->_planes[2] + 0x10000;
	DispBitMap->_planes[4] = DispBitMap->_planes[3] + 0x10000;

	_vm->_anim->readDiff(_curBitmap, true);

	return _curBitmap;
}

void DisplayMan::freePict() {
	delete _curBitmap;
	_curBitmap = NULL;
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

/*---------------------------------------------------------------------------*/
/*------------ Does all the text rendering to the message boxes. ------------*/
/*---------------------------------------------------------------------------*/

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
uint32 DisplayMan::flowText(void *font,      /* the TextAttr pointer */
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
		setAPen(backpen);
		rectFill(x1, y1, x2, y2);
	}

	if (str == NULL)
		return 0L;

	setAPen(pencolor);

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

uint32 DisplayMan::flowTextScaled(void *font,      /* the TextAttr pointer */
	int16 spacing,          /* How much vertical spacing between the lines */
	byte pencolor,         /* pen number to use for text */
	byte backpen,          /* the background color */
	bool fillback,                /* Whether to fill the background */
	bool centerh,                 /* Whether to center the text horizontally */
	bool centerv,                 /* Whether to center the text vertically */
	bool output,                  /* Whether to output any text */
	uint16 x1,               /* Cords */
	uint16 y1, uint16 x2, uint16 y2, const char *str) {
	return flowText(font, spacing, pencolor, backpen, fillback, centerh, centerv, output, VGAScaleX(x1), VGAScaleY(y1), VGAScaleX(x2), VGAScaleY(y2), str);
}

/******************************************************************************/
/* Calls flowText, but flows it to memory.  Same restrictions as flowText.    */
/******************************************************************************/
uint32 DisplayMan::flowTextToMem(Image *destIm, void *font,     /* the TextAttr pointer */
                     int16 spacing,          /* How much vertical spacing between the lines */
                     byte pencolor,         /* pen number to use for text */
                     byte backpen,          /* the background color */
                     bool fillback,                /* Whether to fill the background */
                     bool centerh,                 /* Whether to center the text horizontally */
                     bool centerv,                 /* Whether to center the text vertically */
                     bool output,                  /* Whether to output any text */
                     uint16 x1,               /* Cords */
                     uint16 y1, uint16 x2, uint16 y2, const char *str) { /* The text itself */
	uint32 res, vgabyte = _screenBytesPerPage;
	byte *tmp = _currentDisplayBuffer;

	_currentDisplayBuffer = destIm->_imageData;
	_screenBytesPerPage = (uint32)destIm->_width * (int32)destIm->_height;

	res = flowText(font, spacing, pencolor, backpen, fillback, centerh, centerv, output, x1, y1, x2, y2, str);

	_screenBytesPerPage = vgabyte;
	_currentDisplayBuffer = tmp;

	return res;
}

/*----- The control panel stuff -----*/

void DisplayMan::createBox(uint16 y2) {
	setAPen(7);                 /* Message box area */
	rectFillScaled(4, 154, 315, y2 - 2);

	setAPen(0);                 /* Box around message area */
	drawHLine(VGAScaleX(2), VGAScaleY(152), VGAScaleX(317));
	drawVLine(VGAScaleX(317), VGAScaleY(152), VGAScaleY(y2));
	drawHLine(VGAScaleX(2), VGAScaleY(y2), VGAScaleX(317));
	drawVLine(VGAScaleX(2), VGAScaleY(152), VGAScaleY(y2));
}

int32 DisplayMan::longDrawMessage(const char *str) {
	char newText[512];

	if (str == NULL)
		return 0;

	_vm->_event->attachGadgetList(NULL);
	_vm->_event->mouseHide();
	strcpy(newText, str);

	if (!_longWinInFront) {
		_longWinInFront = true;
		setAPen(3);                 /* Clear Area */
		rectFill(0, VGAScaleY(149) + SVGACord(2), VGAScaleX(319), VGAScaleY(199));
	}

	createBox(198);
	_vm->_event->mouseShow();

	return flowTextScaled(_vm->_msgFont, 0, 1, 7, false, true, true, true, 6, 155, 313, 195, str);
}

/******************************************************************************/
/* Draws a message to the message box.                                        */
/******************************************************************************/
void DisplayMan::drawMessage(const char *str) {
	if (_doNotDrawMessage) {
		_doNotDrawMessage = false;
		return;
	}

	if (str) {
		if ((textLength(_vm->_msgFont, str, strlen(str)) > VGAScaleX(306))) {
			longDrawMessage(str);
			_lastMessageLong = true;
		} else {
			if (_longWinInFront) {
				_longWinInFront = false;
				drawPanel();
			}

			_vm->_event->mouseHide();
			createBox(168);
			text(_vm->_msgFont, VGAScaleX(7), VGAScaleY(155) + SVGACord(2), 1, str, strlen(str));
			_vm->_event->mouseShow();
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
void DisplayMan::doScrollBlack() {
	byte *tempmem;
	Image im;
	uint32 size, copysize;
	byte *baseAddr;
	uint16 width = VGAScaleX(320);
	uint16 height = VGAScaleY(149) + SVGACord(2);
	byte *mem = new byte[width * height];

	_vm->_event->mouseHide();

	im._width = width;
	im._height = height;
	im._imageData = mem;
	_vm->_music->updateMusic();
	im.readScreenImage(0, 0);
	_vm->_music->updateMusic();

	baseAddr = getCurrentDrawingBuffer();

	uint16 by      = VGAScaleX(4);
	uint16 nheight = height;

	while (nheight) {
		_vm->_music->updateMusic();

		if (!_vm->_isHiRes)
			_vm->waitTOF();

		baseAddr = getCurrentDrawingBuffer();

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

		if (!_vm->_isHiRes) {
			if (nheight <= (height / 8))
				by = 1;
			else if (nheight <= (height / 4))
				by = 2;
			else if (nheight <= (height / 2))
				by = 3;
		}
	}

	delete[] mem;
	freePict();
	_vm->_event->mouseShow();
}

void DisplayMan::copyPage(uint16 width, uint16 height, uint16 nheight, uint16 startline, byte *mem) {
	byte *baseAddr = getCurrentDrawingBuffer();

	uint32 size = (int32)(height - nheight) * (int32)width;
	mem += startline * width;
	uint16 curPage = ((int32)nheight * (int32)width) / _screenBytesPerPage;
	uint32 offSet = ((int32)nheight * (int32)width) - (curPage * _screenBytesPerPage);

	while (size) {
		uint32 copysize;
		if (size > (_screenBytesPerPage - offSet))
			copysize = _screenBytesPerPage - offSet;
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
void DisplayMan::doScrollWipe(char *filename) {
	uint16 startline = 0, onrow = 0;

	_vm->_event->mouseHide();
	uint16 width = VGAScaleX(320);
	uint16 height = VGAScaleY(149) + SVGACord(2);

	while (_vm->_music->isSoundEffectActive()) {
		_vm->_music->updateMusic();
		_vm->waitTOF();
	}

	_vm->_anim->_isBM = true;
	readPict(filename, true);
	setPalette(_vm->_anim->_diffPalette, 256);
	_vm->_anim->_isBM = false;
	byte *mem = _vm->_anim->_rawDiffBM._planes[0];

	_vm->_music->updateMusic();
	uint16 by = VGAScaleX(3);
	uint16 nheight = height;

	while (onrow < _vm->_anim->_headerdata._height) {
		_vm->_music->updateMusic();

		if ((by > nheight) && nheight)
			by = nheight;

		if ((startline + by) > (_vm->_anim->_headerdata._height - height - 1))
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

	_vm->_event->mouseShow();
}

/*****************************************************************************/
/* Does the scroll bounce.  Assumes bitmap already in memory.                */
/*****************************************************************************/
void DisplayMan::doScrollBounce() {
	const uint16 *newby, *newby1;

	const uint16 newbyd[5] = {5, 4, 3, 2, 1}, newby1d[8] = {3, 3, 2, 2, 2, 1, 1, 1};
	const uint16 newbyw[5] = {10, 8, 6, 4, 2}, newby1w[8] = {6, 6, 4, 4, 4, 2, 2, 2};

	if (_vm->getPlatform() != Common::kPlatformWindows) {
		newby = newbyd;
		newby1 = newby1d;
	} else {
		newby = newbyw;
		newby1 = newby1w;
	}

	_vm->_event->mouseHide();
	int width = VGAScaleX(320);
	int height = VGAScaleY(149) + SVGACord(2);
	byte *mem = _vm->_anim->_rawDiffBM._planes[0];

	_vm->_music->updateMusic();
	int startline = _vm->_anim->_headerdata._height - height - 1;

	for (int i = 0; i < 5; i++) {
		_vm->_music->updateMusic();
		startline -= newby[i];
		copyPage(width, height, 0, startline, mem);

		screenUpdate();
		_vm->waitTOF();
	}

	for (int i = 8; i > 0; i--) {
		_vm->_music->updateMusic();
		startline += newby1[i - 1];
		copyPage(width, height, 0, startline, mem);

		screenUpdate();
		_vm->waitTOF();
	}

	_vm->_event->mouseShow();
}

/*****************************************************************************/
/* Does the transporter wipe.                                                */
/*****************************************************************************/
void DisplayMan::doTransWipe(CloseDataPtr *cPtr, char *filename) {
	uint16 lastY, curY, linesdone = 0, lineslast;
	Image imSource, imDest;

	if (_vm->_isHiRes) {
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
				_vm->_music->updateMusic();
				_vm->waitTOF();
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
				_vm->_music->updateMusic();
				_vm->waitTOF();
				linesdone = 0;
			}

			rectFill(0, curY, _screenWidth - 1, curY + 1);
			curY += 4;
			linesdone++;
		}
	}

	if (filename == NULL)
		_vm->_curFileName = getPictName(cPtr);
	else if (filename[0] > ' ')
		_vm->_curFileName = filename;
	else
		_vm->_curFileName = getPictName(cPtr);

	byte *BitMapMem = readPictToMem(_vm->_curFileName, _screenWidth, lastY + 5);
	setPalette(_vm->_anim->_diffPalette, 256);

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
					_vm->_music->updateMusic();
					_vm->waitTOF();
					linesdone = 0;
				}

				imDest._imageData = getCurrentDrawingBuffer();

				imSource.blitBitmap(0, curY, &imDest, 0, curY, _screenWidth, 2, false);
				overlayRect(0, 0, curY, _screenWidth - 1, curY + 1);
				curY += 4;
				linesdone++;
			}
		}

		for (uint16 i = 0; i < 2; i++) {
			curY = i * 2;

			while (curY <= lastY) {
				if (linesdone >= lineslast) {
					_vm->_music->updateMusic();
					_vm->waitTOF();
					linesdone = 0;
				}

				imDest._imageData = getCurrentDrawingBuffer();

				if (curY == lastY)
					imSource.blitBitmap(0, curY, &imDest, 0, curY, _screenWidth, 1, false);
				else
					imSource.blitBitmap(0, curY, &imDest, 0, curY, _screenWidth, 2, false);

				curY += 4;
				linesdone++;
			}
		}
	}
}

/*****************************************************************************/
/* Does a certain number of pre-programmed wipes.                            */
/*****************************************************************************/
void DisplayMan::doWipe(uint16 wipeType, CloseDataPtr *cPtr, char *filename) {
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
		_vm->_anim->diffNextFrame();
}

/*****************************************************************************/
/* Changes the front screen to black.                                        */
/*****************************************************************************/
void DisplayMan::blackScreen() {
	byte pal[256 * 3];
	memset(pal, 0, 248 * 3);
	writeColorRegs(pal, 8, 248);

	g_system->delayMillis(32);
}

/*****************************************************************************/
/* Changes the front screen to white.                                        */
/*****************************************************************************/
void DisplayMan::whiteScreen() {
	byte pal[256 * 3];
	memset(pal, 255, 248 * 3);
	writeColorRegs(pal, 8, 248);
}

/*****************************************************************************/
/* Changes the entire screen to black.                                       */
/*****************************************************************************/
void DisplayMan::blackAllScreen() {
	byte pal[256 * 3];
	memset(pal, 0, 256 * 3);
	writeColorRegs(pal, 0, 256);

	g_system->delayMillis(32);
}

/******************************************************************************/
/* Draws the control panel display.                                           */
/******************************************************************************/
void DisplayMan::drawPanel() {
	_vm->_event->mouseHide();

	setAPen(3);                 /* Clear Area */
	rectFill(0, VGAScaleY(149) + SVGACord(2), VGAScaleX(319), VGAScaleY(199));

	setAPen(0);                 /* First Line */
	drawHLine(0, VGAScaleY(149) + SVGACord(2), VGAScaleX(319));
	setAPen(5);                 /* Second Line */
	drawHLine(0, VGAScaleY(149) + 1 + SVGACord(2), VGAScaleX(319));

	/* Gadget Separators */
	setAPen(0);
	drawHLine(0, VGAScaleY(170), VGAScaleX(319));     /* First black line to separate buttons */

	if (!_vm->_alternate) {
		setAPen(4);
		drawHLine(0, VGAScaleY(170) + 1, VGAScaleX(319)); /* The horizontal lines under the black one */
		drawGadgetList(_vm->_moveGadgetList);
	} else {
		if (_vm->getPlatform() != Common::kPlatformWindows) {
			drawVLine(VGAScaleX(124), VGAScaleY(170) + 1, VGAScaleY(199)); /* Vertical Black lines */
			drawVLine(VGAScaleX(194), VGAScaleY(170) + 1, VGAScaleY(199));
		} else {
			drawVLine(VGAScaleX(90), VGAScaleY(170) + 1, VGAScaleY(199));  /* Vertical Black lines */
			drawVLine(VGAScaleX(160), VGAScaleY(170) + 1, VGAScaleY(199));
			drawVLine(VGAScaleX(230), VGAScaleY(170) + 1, VGAScaleY(199));
		}

		setAPen(4);
		drawHLine(0, VGAScaleY(170) + 1, VGAScaleX(122));   /* The horizontal lines under the black one */
		drawHLine(VGAScaleX(126), VGAScaleY(170) + 1, VGAScaleX(192));
		drawHLine(VGAScaleX(196), VGAScaleY(170) + 1, VGAScaleX(319));

		drawVLine(VGAScaleX(1), VGAScaleY(170) + 2, VGAScaleY(198)); /* The vertical high light lines */
		if (_vm->getPlatform() != Common::kPlatformWindows) {
			drawVLine(VGAScaleX(126), VGAScaleY(170) + 2, VGAScaleY(198));
			drawVLine(VGAScaleX(196), VGAScaleY(170) + 2, VGAScaleY(198));
		} else {
			drawVLine(VGAScaleX(92), VGAScaleY(170) + 2, VGAScaleY(198));
			drawVLine(VGAScaleX(162), VGAScaleY(170) + 2, VGAScaleY(198));
			drawVLine(VGAScaleX(232), VGAScaleY(170) + 2, VGAScaleY(198));
		}

		drawGadgetList(_vm->_invGadgetList);
	}

	_vm->_event->mouseShow();
}

/******************************************************************************/
/* Sets up the Labyrinth screens, and opens up the initial windows.           */
/******************************************************************************/
bool DisplayMan::setUpScreens() {
	if (!createScreen(_vm->_isHiRes))
		return false;

	Common::File *controlFile = _vm->_resource->openDataFile("P:Control");
	for (uint16 i = 0; i < 20; i++)
		_vm->_moveImages[i] = new Image(controlFile);
	delete controlFile;

	/* Creates the gadgets for the movement control panel */
	uint16 y = VGAScaleY(173) - SVGACord(2);

	// The key mapping was only set for the Windows version.
	// It's very convenient to have those shortcut, so I added them
	// for all versions. (Strangerke)
	_vm->_moveGadgetList = createButton(1, y, 0, 't', _vm->_moveImages[0], _vm->_moveImages[1]);
	Gadget *curGadget = _vm->_moveGadgetList;
	curGadget->NextGadget = createButton(33, y, 1, 'm', _vm->_moveImages[2], _vm->_moveImages[3]);
	curGadget = curGadget->NextGadget;
	curGadget->NextGadget = createButton(65, y, 2, 'o', _vm->_moveImages[4], _vm->_moveImages[5]);
	curGadget = curGadget->NextGadget;
	curGadget->NextGadget = createButton(97, y, 3, 'c', _vm->_moveImages[6], _vm->_moveImages[7]);
	curGadget = curGadget->NextGadget;
	curGadget->NextGadget = createButton(129, y, 4, 'l', _vm->_moveImages[8], _vm->_moveImages[9]);
	curGadget = curGadget->NextGadget;
	curGadget->NextGadget = createButton(161, y, 5, 'i', _vm->_moveImages[12], _vm->_moveImages[13]);
	curGadget = curGadget->NextGadget;
	curGadget->NextGadget = createButton(193, y, 6, VKEY_LTARROW, _vm->_moveImages[14], _vm->_moveImages[15]);
	curGadget = curGadget->NextGadget;
	curGadget->NextGadget = createButton(225, y, 7, VKEY_UPARROW, _vm->_moveImages[16], _vm->_moveImages[17]);
	curGadget = curGadget->NextGadget;
	curGadget->NextGadget = createButton(257, y, 8, VKEY_RTARROW, _vm->_moveImages[18], _vm->_moveImages[19]);
	curGadget = curGadget->NextGadget;
	curGadget->NextGadget = createButton(289, y, 9, 'p', _vm->_moveImages[10], _vm->_moveImages[11]);

	Common::File *invFile = _vm->_resource->openDataFile("P:Inv");
	if (_vm->getPlatform() == Common::kPlatformWindows) {
		for (uint16 imgIdx = 0; imgIdx < 10; imgIdx++)
			_vm->_invImages[imgIdx] = new Image(invFile);
	} else {
		for (uint16 imgIdx = 0; imgIdx < 6; imgIdx++)
			_vm->_invImages[imgIdx] = new Image(invFile);
	}
	_vm->_invGadgetList = createButton(24, y, 0, 'm', _vm->_invImages[0], _vm->_invImages[1]);
	curGadget = _vm->_invGadgetList;
	curGadget->NextGadget = createButton(56, y, 1, 'g', _vm->_invImages[2], _vm->_invImages[3]);
	curGadget = curGadget->NextGadget;
	curGadget->NextGadget = createButton(94, y, 2, 'u', _vm->_invImages[4], _vm->_invImages[5]);
	curGadget = curGadget->NextGadget;
	curGadget->NextGadget = createButton(126, y, 3, 'l', _vm->_moveImages[8], _vm->_moveImages[9]);
	curGadget = curGadget->NextGadget;
	curGadget->NextGadget = createButton(164, y, 4, VKEY_LTARROW, _vm->_moveImages[14], _vm->_moveImages[15]);
	curGadget = curGadget->NextGadget;
	curGadget->NextGadget = createButton(196, y, 5, VKEY_RTARROW, _vm->_moveImages[18], _vm->_moveImages[19]);

	// The windows version has 2 extra gadgets for breadcrumb trail
	// TODO: the game is really hard to play without those, maybe we could add something to enable that.
	if (_vm->getPlatform() == Common::kPlatformWindows) {
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(234, y, 6, 'b', _vm->_invImages[6], _vm->_invImages[7]);
		curGadget = curGadget->NextGadget;
		curGadget->NextGadget = createButton(266, y, 7, 'f', _vm->_invImages[8], _vm->_invImages[9]);
	}

	delete invFile;

	return true;
}

/*****************************************************************************/
/* Sets the pen number to use on all the drawing operations.                 */
/*****************************************************************************/
void DisplayMan::setAPen(byte pennum) {
	_curapen = pennum;
}

/*****************************************************************************/
/* Fills in a rectangle.                                                     */
/*****************************************************************************/
void DisplayMan::rectFill(uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
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

void DisplayMan::rectFillScaled(uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	rectFill(VGAScaleX(x1), VGAScaleY(y1), VGAScaleX(x2), VGAScaleY(y2));
}

/*****************************************************************************/
/* Draws a horizontal line.                                                  */
/*****************************************************************************/
void DisplayMan::drawVLine(uint16 x, uint16 y1, uint16 y2) {
	rectFill(x, y1, x, y2);
}

/*****************************************************************************/
/* Draws a vertical line.                                                    */
/*****************************************************************************/
void DisplayMan::drawHLine(uint16 x1, uint16 y, uint16 x2) {
	rectFill(x1, y, x2, y);
}

void DisplayMan::screenUpdate() {
	g_system->copyRectToScreen(_displayBuffer, _screenWidth, 0, 0, _screenWidth, _screenHeight);
	g_system->updateScreen();

	_vm->_event->processInput();
}

/*****************************************************************************/
/* Sets up either a low-res or a high-res 256 color screen.                  */
/*****************************************************************************/
bool DisplayMan::createScreen(bool hiRes) {
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
/* Converts an Amiga palette (up to 16 colors) to a VGA palette, then sets   */
/* the VGA palette.                                                          */
/*****************************************************************************/
void DisplayMan::setAmigaPal(uint16 *pal, uint16 numColors) {
	byte vgaPal[16 * 3];
	uint16 vgaIdx = 0;

	if (numColors > 16)
		numColors = 16;

	for (uint16 i = 0; i < numColors; i++) {
		vgaPal[vgaIdx++] = (byte)(((pal[i] & 0xf00) >> 8) << 2);
		vgaPal[vgaIdx++] = (byte)(((pal[i] & 0x0f0) >> 4) << 2);
		vgaPal[vgaIdx++] = (byte)(((pal[i] & 0x00f)) << 2);
	}

	writeColorRegs(vgaPal, 0, 16);
	_vm->waitTOF();
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
void DisplayMan::writeColorRegs(byte *buf, uint16 first, uint16 numreg) {
	byte tmp[256 * 3];

	for (int i = 0; i < 256 * 3; i++) {
		tmp[i] = buf[i] * 4;
	}

	g_system->getPaletteManager()->setPalette(tmp, first, numreg);

	memcpy(&(_curvgapal[first * 3]), buf, numreg * 3);
}

void DisplayMan::setPalette(void *cmap, uint16 numcolors) {
	if (memcmp(cmap, _curvgapal, numcolors * 3) != 0)
		writeColorRegs((byte *)cmap, 0, numcolors);
}

/*****************************************************************************/
/* Returns the base address of the current VGA display.                      */
/*****************************************************************************/
byte *DisplayMan::getCurrentDrawingBuffer() {
	if (_currentDisplayBuffer)
		return _currentDisplayBuffer;

	return _displayBuffer;
}

/*****************************************************************************/
/* Overlays a region on the screen using the desired pen color.              */
/*****************************************************************************/
void DisplayMan::overlayRect(uint16 pencolor, uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
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

/*****************************************************************************/
/* Scrolls the display in the x direction by blitting.                       */
/* The _tempScrollData variable must be initialized to some memory, or this   */
/* function will fail.                                                       */
/*****************************************************************************/
void DisplayMan::scrollDisplayX(int16 dx, uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
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
void DisplayMan::scrollDisplayY(int16 dy, uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
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

} // End of namespace Lab
