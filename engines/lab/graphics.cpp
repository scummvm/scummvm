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
#include "lab/diff.h"
#include "lab/parsetypes.h"
#include "lab/labfun.h"
#include "lab/parsefun.h"
#include "lab/mouse.h"
#include "lab/text.h"
#include "lab/resource.h"

namespace Lab {

BitMap bit1, bit2, *DispBitMap = &bit1, *DrawBitMap = &bit1;


extern BitMap RawDiffBM;
extern char diffcmap[256 * 3];
extern bool IsBM, nopalchange;

extern bool DoBlack, stopsound;
extern TextFont *MsgFont;
extern const char *CurFileName;

/*---------------------------------------------------------------------------*/
/*------ From readPict.c.  Reads in pictures and animations from disk. ------*/
/*---------------------------------------------------------------------------*/


/*****************************************************************************/
/* Reads in a picture into the dest bitmap.                                  */
/*****************************************************************************/
bool readPict(const char *filename, bool playOnce) {
	stopDiff();

	byte **file = g_lab->_music->newOpen(filename);

	if (file == NULL) {
		if ((filename[0] == 'p') || (filename[0] == 'P'))
			blackScreen();

		return false;
	}

	DispBitMap->BytesPerRow = g_lab->_screenWidth;
	DispBitMap->Rows        = g_lab->_screenHeight;
	DispBitMap->Flags       = BITMAPF_VIDEO;

	readDiff(playOnce);

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

	DoBlack = false;
	readSound(waitTillFinished, file);

	return true;
}

/*****************************************************************************/
/* Reads in a picture into buffer memory.                                    */
/*****************************************************************************/
byte *readPictToMem(const char *filename, uint16 x, uint16 y) {
	byte *mem;

	stopDiff();

	allocFile((void **)&mem, (int32)x * (int32)y, "Bitmap");
	byte *curMem = mem;

	byte **file = g_lab->_music->newOpen(filename);

	if (file == NULL)
		return NULL;

	DispBitMap->BytesPerRow = x;
	DispBitMap->Rows        = y;
	DispBitMap->Flags       = 0;
	DispBitMap->Planes[0] = curMem;
	DispBitMap->Planes[1] = DispBitMap->Planes[0] + 0x10000;
	DispBitMap->Planes[2] = DispBitMap->Planes[1] + 0x10000;
	DispBitMap->Planes[3] = DispBitMap->Planes[2] + 0x10000;
	DispBitMap->Planes[4] = DispBitMap->Planes[3] + 0x10000;

	readDiff(true);

	return mem;
}

/*---------------------------------------------------------------------------*/
/*------------ Does all the text rendering to the message boxes. ------------*/
/*---------------------------------------------------------------------------*/
bool DoNotDrawMessage = false;

extern bool LongWinInFront, Alternate;

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
	TextFont *msgfont = (TextFont *)font;
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

	fontheight = textHeight(msgfont) + spacing;
	numlines   = (y2 - y1 + 1) / fontheight;
	width      = x2 - x1 + 1;
	y          = y1;

	if (centerv && output) {
		temp = str;
		actlines = 0;

		while (temp[0]) {
			getLine(msgfont, linebuffer, &temp, width);
			actlines++;
		}

		if (actlines <= numlines)
			y += ((y2 - y1 + 1) - (actlines * fontheight)) / 2;
	}

	temp = str;

	while (numlines && str[0]) {
		getLine(msgfont, linebuffer, &str, width);

		x = x1;

		if (centerh)
			x += (width - textLength(msgfont, linebuffer, strlen(linebuffer))) / 2;

		if (output)
			text(msgfont, x, y, pencolor, linebuffer, strlen(linebuffer));

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

	g_lab->_currentDisplayBuffer = destIm->ImageData;
	g_lab->_screenBytesPerPage = (uint32)destIm->Width * (int32)destIm->Height;

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

	if (!LongWinInFront) {
		LongWinInFront = true;
		setAPen(3);                 /* Clear Area */
		rectFill(0, VGAScaleY(149) + SVGACord(2), VGAScaleX(319), VGAScaleY(199));
	}

	createBox(198);
	_event->mouseShow();

	return flowText(MsgFont, 0, 1, 7, false, true, true, true, VGAScaleX(6), VGAScaleY(155), VGAScaleX(313), VGAScaleY(195), str);
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
		if ((textLength(MsgFont, str, strlen(str)) > VGAScaleX(306))) {
			longDrawMessage(str);
			_lastMessageLong = true;
		} else {
			if (LongWinInFront) {
				LongWinInFront = false;
				drawPanel();
			}

			_event->mouseHide();
			createBox(168);
			text(MsgFont, VGAScaleX(7), VGAScaleY(155) + SVGACord(2), 1, str, strlen(str));
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

	im.Width = width;
	im.Height = height;
	im.ImageData = mem;
	_music->updateMusic();
	readScreenImage(&im, 0, 0);
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

extern BitMap RawDiffBM;
extern DIFFHeader headerdata;

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

	IsBM = true;
	readPict(filename, true);
	setPalette(diffcmap, 256);
	IsBM = false;
	byte *mem = RawDiffBM.Planes[0];

	_music->updateMusic();
	uint16 by = VGAScaleX(3);
	uint16 nheight = height;

	while (onrow < headerdata.y) {
		_music->updateMusic();

		if ((by > nheight) && nheight)
			by = nheight;

		if ((startline + by) > (headerdata.y - height - 1))
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
	byte *mem = RawDiffBM.Planes[0];

	_music->updateMusic();
	int startline = headerdata.y - height - 1;

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

			ghoastRect(0, 0, curY, _screenWidth - 1, curY + 1);
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
		CurFileName = getPictName(cPtr);
	else if (filename[0] > ' ')
		CurFileName = filename;
	else
		CurFileName = getPictName(cPtr);

	byte *BitMapMem = readPictToMem(CurFileName, _screenWidth, lastY + 5);
	setPalette(diffcmap, 256);

	if (BitMapMem) {
		imSource.Width = _screenWidth;
		imSource.Height = lastY;
		imSource.ImageData = BitMapMem;

		imDest.Width = _screenWidth;
		imDest.Height = _screenHeight;
		imDest.ImageData = getCurrentDrawingBuffer();

		for (uint16 i = 0; i < 2; i++) {
			curY = i * 2;

			while (curY < lastY) {
				if (linesdone >= lineslast) {
					_music->updateMusic();
					waitTOF();
					linesdone = 0;
				}

				imDest.ImageData = getCurrentDrawingBuffer();

				bltBitMap(&imSource, 0, curY, &imDest, 0, curY, _screenWidth, 2);
				ghoastRect(0, 0, curY, _screenWidth - 1, curY + 1);
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

				imDest.ImageData = getCurrentDrawingBuffer();

				if (curY == lastY)
					bltBitMap(&imSource, 0, curY, &imDest, 0, curY, _screenWidth, 1);
				else
					bltBitMap(&imSource, 0, curY, &imDest, 0, curY, _screenWidth, 2);

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
		diffNextFrame();
}

} // End of namespace Lab
