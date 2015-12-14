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

#include "lab/lab.h"

#include "lab/anim.h"
#include "lab/dispman.h"
#include "lab/eventman.h"
#include "lab/music.h"
#include "lab/image.h"
#include "lab/resource.h"
#include "lab/utils.h"

namespace Lab {

DisplayMan::DisplayMan(LabEngine *vm) : _vm(vm) {
	_longWinInFront = false;
	_lastMessageLong = false;
	_doNotDrawMessage = false;

	_screenBytesPerPage = 65536;
	_curapen = 0;
	_curBitmap = nullptr;
	_displayBuffer = nullptr;
	_currentDisplayBuffer = nullptr;
	_fadePalette = nullptr;

	_screenWidth = 0;
	_screenHeight = 0;

	for (int i = 0; i < 256 * 3; i++)
		_curvgapal[i] = 0;

	_dispBitMap = new BitMap;
}

DisplayMan::~DisplayMan() {
	freePict();
	delete _dispBitMap;
}

// From readPict.c.  Reads in pictures and animations from disk.

void DisplayMan::loadPict(const char *filename) {
	Common::File *bitmapFile = _vm->_resource->openDataFile(filename);
	freePict();
	_curBitmap = new byte[bitmapFile->size()];
	bitmapFile->read(_curBitmap, bitmapFile->size());
	delete bitmapFile;
}

void DisplayMan::loadBackPict(const char *fileName, uint16 *highPal) {
	_fadePalette = highPal;
	_vm->_anim->_noPalChange = true;
	readPict(fileName, true);

	for (uint16 i = 0; i < 16; i++) {
		highPal[i] = ((_vm->_anim->_diffPalette[i * 3] >> 2) << 8) +
			((_vm->_anim->_diffPalette[i * 3 + 1] >> 2) << 4) +
			((_vm->_anim->_diffPalette[i * 3 + 2] >> 2));
	}

	_vm->_anim->_noPalChange = false;
}

/**
 * Reads in a picture into the display bitmap.
 */
void DisplayMan::readPict(const char *filename, bool playOnce, bool onlyDiffData, byte *memoryBuffer, uint16 maxHeight) {
	_vm->_anim->stopDiff();

	loadPict(filename);

	_vm->_music->updateMusic();

	if (!_vm->_music->_loopSoundEffect)
		_vm->_music->stopSoundEffect();

	_dispBitMap->_bytesPerRow  = _screenWidth;
	_dispBitMap->_rows         = (maxHeight > 0) ? maxHeight : _screenHeight;
	_dispBitMap->_drawOnScreen = (memoryBuffer == nullptr);
	if (memoryBuffer)
		_dispBitMap->_planes[0] = memoryBuffer;

	_vm->_anim->readDiff(_curBitmap, playOnce, onlyDiffData);
}

void DisplayMan::freePict() {
	delete _curBitmap;
	_curBitmap = NULL;
}

//---------------------------------------------------------------------------
//------------ Does all the text rendering to the message boxes. ------------
//---------------------------------------------------------------------------

/**
 * Extracts the first word from a string.
 */
void DisplayMan::getWord(char *wordBuffer, const char *mainBuffer, uint16 *wordWidth) {
	uint16 width = 0;

	while ((mainBuffer[width] != ' ') && mainBuffer[width] && (mainBuffer[width] != '\n')) {
		wordBuffer[width] = mainBuffer[width];
		width++;
	}

	wordBuffer[width] = 0;

	*wordWidth = width;
}

/**
 * Gets a line of text for flowText; makes sure that its length is less than
 * or equal to the maximum width.
 */
void DisplayMan::getLine(TextFont *tf, char *lineBuffer, const char **mainBuffer, uint16 lineWidth) {
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

/**
 * Dumps a chunk of text to an arbitrary box; flows it within that box and
 * optionally centers it. Returns the number of characters that were
 * processed.
 * Note: Every individual word MUST be int16 enough to fit on a line, and
 * each line less than 255 characters.
 */
uint32 DisplayMan::flowText(
			void *font,            // the TextAttr pointer
			int16 spacing,         // How much vertical spacing between the lines
			byte pencolor,         // pen number to use for text
			byte backpen,          // the background color
			bool fillback,         // Whether to fill the background
			bool centerh,          // Whether to center the text horizontally
			bool centerv,          // Whether to center the text vertically
			bool output,           // Whether to output any text
			uint16 x1, uint16 y1,  // Cords
			uint16 x2, uint16 y2,
			const char *str) {     // The text itself
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

uint32 DisplayMan::flowTextScaled(
	void *font,                // the TextAttr pointer
	int16 spacing,             // How much vertical spacing between the lines
	byte penColor,             // pen number to use for text
	byte backPen,              // the background color
	bool fillBack,             // Whether to fill the background
	bool centerX,              // Whether to center the text horizontally
	bool centerY,              // Whether to center the text vertically
	bool output,               // Whether to output any text
	uint16 x1, uint16 y1,      // Cords
	uint16 x2, uint16 y2,
	const char *str) {
	return flowText(font, spacing, penColor, backPen, fillBack, centerX, centerY, output,
					_vm->_utils->vgaScaleX(x1), _vm->_utils->vgaScaleY(y1),
					_vm->_utils->vgaScaleX(x2), _vm->_utils->vgaScaleY(y2), str);
}

/**
 * Calls flowText, but flows it to memory.  Same restrictions as flowText.
 */
uint32 DisplayMan::flowTextToMem(Image *destIm,
			void *font,            // the TextAttr pointer
			int16 spacing,         // How much vertical spacing between the lines
			byte pencolor,         // pen number to use for text
			byte backpen,          // the background color
			bool fillback,         // Whether to fill the background
			bool centerh,          // Whether to center the text horizontally
			bool centerv,          // Whether to center the text vertically
			bool output,           // Whether to output any text
			uint16 x1, uint16 y1,  // Cords
			uint16 x2, uint16 y2,
			const char *str) {     // The text itself
	uint32 res, vgabyte = _screenBytesPerPage;
	byte *tmp = _currentDisplayBuffer;

	_currentDisplayBuffer = destIm->_imageData;
	_screenBytesPerPage = (uint32)destIm->_width * (int32)destIm->_height;

	res = flowText(font, spacing, pencolor, backpen, fillback, centerh, centerv, output, x1, y1, x2, y2, str);

	_screenBytesPerPage = vgabyte;
	_currentDisplayBuffer = tmp;

	return res;
}

//----- The control panel stuff -----

void DisplayMan::createBox(uint16 y2) {
	// Message box area
	setAPen(7);
	rectFillScaled(4, 154, 315, y2 - 2);

	// Box around message area
	setAPen(0);
	drawHLine(_vm->_utils->vgaScaleX(2), _vm->_utils->vgaScaleY(152), _vm->_utils->vgaScaleX(317));
	drawVLine(_vm->_utils->vgaScaleX(317), _vm->_utils->vgaScaleY(152), _vm->_utils->vgaScaleY(y2));
	drawHLine(_vm->_utils->vgaScaleX(2), _vm->_utils->vgaScaleY(y2), _vm->_utils->vgaScaleX(317));
	drawVLine(_vm->_utils->vgaScaleX(2), _vm->_utils->vgaScaleY(152), _vm->_utils->vgaScaleY(y2));
}

int32 DisplayMan::longDrawMessage(const char *str) {
	char newText[512];

	if (str == NULL)
		return 0;

	_vm->_event->attachButtonList(NULL);
	_vm->_event->mouseHide();
	strcpy(newText, str);

	if (!_longWinInFront) {
		_longWinInFront = true;
		// Clear Area
		setAPen(3);
		rectFill(0, _vm->_utils->vgaScaleY(149) + _vm->_utils->svgaCord(2), _vm->_utils->vgaScaleX(319), _vm->_utils->vgaScaleY(199));
	}

	createBox(198);
	_vm->_event->mouseShow();

	return flowTextScaled(_vm->_msgFont, 0, 1, 7, false, true, true, true, 6, 155, 313, 195, str);
}

/**
 * Draws a message to the message box.
 */
void DisplayMan::drawMessage(const char *str) {
	if (_doNotDrawMessage) {
		_doNotDrawMessage = false;
		return;
	}

	if (str) {
		if ((textLength(_vm->_msgFont, str, strlen(str)) > _vm->_utils->vgaScaleX(306))) {
			longDrawMessage(str);
			_lastMessageLong = true;
		} else {
			if (_longWinInFront) {
				_longWinInFront = false;
				drawPanel();
			}

			_vm->_event->mouseHide();
			createBox(168);
			text(_vm->_msgFont, _vm->_utils->vgaScaleX(7), _vm->_utils->vgaScaleY(155) + _vm->_utils->svgaCord(2), 1, str, strlen(str));
			_vm->_event->mouseShow();
			_lastMessageLong = false;
		}
	}
}

/**
 * Draws the control panel display.
 */
void DisplayMan::drawPanel() {
	_vm->_event->mouseHide();

	// Clear Area
	setAPen(3);
	rectFill(0, _vm->_utils->vgaScaleY(149) + _vm->_utils->svgaCord(2), _vm->_utils->vgaScaleX(319), _vm->_utils->vgaScaleY(199));

	// First Line
	setAPen(0);
	drawHLine(0, _vm->_utils->vgaScaleY(149) + _vm->_utils->svgaCord(2), _vm->_utils->vgaScaleX(319));
	// Second Line
	setAPen(5);
	drawHLine(0, _vm->_utils->vgaScaleY(149) + 1 + _vm->_utils->svgaCord(2), _vm->_utils->vgaScaleX(319));
	// Button Separators
	setAPen(0);
	// First black line to separate buttons
	drawHLine(0, _vm->_utils->vgaScaleY(170), _vm->_utils->vgaScaleX(319));

	if (!_vm->_alternate) {
		setAPen(4);
		// The horizontal lines under the black one
		drawHLine(0, _vm->_utils->vgaScaleY(170) + 1, _vm->_utils->vgaScaleX(319));
		_vm->_event->drawButtonList(&_vm->_moveButtonList);
	} else {
		if (_vm->getPlatform() != Common::kPlatformWindows) {
			// Vertical Black lines
			drawVLine(_vm->_utils->vgaScaleX(124), _vm->_utils->vgaScaleY(170) + 1, _vm->_utils->vgaScaleY(199));
			drawVLine(_vm->_utils->vgaScaleX(194), _vm->_utils->vgaScaleY(170) + 1, _vm->_utils->vgaScaleY(199));
		} else {
			// Vertical Black lines
			drawVLine(_vm->_utils->vgaScaleX(90), _vm->_utils->vgaScaleY(170) + 1, _vm->_utils->vgaScaleY(199));
			drawVLine(_vm->_utils->vgaScaleX(160), _vm->_utils->vgaScaleY(170) + 1, _vm->_utils->vgaScaleY(199));
			drawVLine(_vm->_utils->vgaScaleX(230), _vm->_utils->vgaScaleY(170) + 1, _vm->_utils->vgaScaleY(199));
		}

		setAPen(4);
		// The horizontal lines under the black one
		drawHLine(0, _vm->_utils->vgaScaleY(170) + 1, _vm->_utils->vgaScaleX(122));
		drawHLine(_vm->_utils->vgaScaleX(126), _vm->_utils->vgaScaleY(170) + 1, _vm->_utils->vgaScaleX(192));
		drawHLine(_vm->_utils->vgaScaleX(196), _vm->_utils->vgaScaleY(170) + 1, _vm->_utils->vgaScaleX(319));
		// The vertical high light lines
		drawVLine(_vm->_utils->vgaScaleX(1), _vm->_utils->vgaScaleY(170) + 2, _vm->_utils->vgaScaleY(198));

		if (_vm->getPlatform() != Common::kPlatformWindows) {
			drawVLine(_vm->_utils->vgaScaleX(126), _vm->_utils->vgaScaleY(170) + 2, _vm->_utils->vgaScaleY(198));
			drawVLine(_vm->_utils->vgaScaleX(196), _vm->_utils->vgaScaleY(170) + 2, _vm->_utils->vgaScaleY(198));
		} else {
			drawVLine(_vm->_utils->vgaScaleX(92), _vm->_utils->vgaScaleY(170) + 2, _vm->_utils->vgaScaleY(198));
			drawVLine(_vm->_utils->vgaScaleX(162), _vm->_utils->vgaScaleY(170) + 2, _vm->_utils->vgaScaleY(198));
			drawVLine(_vm->_utils->vgaScaleX(232), _vm->_utils->vgaScaleY(170) + 2, _vm->_utils->vgaScaleY(198));
		}

		_vm->_event->drawButtonList(&_vm->_invButtonList);
	}

	_vm->_event->mouseShow();
}

/**
 * Sets up the Labyrinth screens, and opens up the initial windows.
 */
void DisplayMan::setUpScreens() {
	createScreen(_vm->_isHiRes);

	Common::File *controlFile = _vm->_resource->openDataFile("P:Control");
	for (uint16 i = 0; i < 20; i++)
		_vm->_moveImages[i] = new Image(controlFile, _vm);
	delete controlFile;

	// Creates the buttons for the movement control panel
	uint16 y = _vm->_utils->vgaScaleY(173) - _vm->_utils->svgaCord(2);

	// The key mapping was only set for the Windows version.
	// It's very convenient to have those shortcut, so I added them
	// for all versions. (Strangerke)
	_vm->_moveButtonList.push_back(_vm->_event->createButton(  1, y, 0,          't', _vm->_moveImages[0],  _vm->_moveImages[1]));
	_vm->_moveButtonList.push_back(_vm->_event->createButton( 33, y, 1,          'm', _vm->_moveImages[2],  _vm->_moveImages[3]));
	_vm->_moveButtonList.push_back(_vm->_event->createButton( 65, y, 2,          'o', _vm->_moveImages[4],  _vm->_moveImages[5]));
	_vm->_moveButtonList.push_back(_vm->_event->createButton( 97, y, 3,          'c', _vm->_moveImages[6],  _vm->_moveImages[7]));
	_vm->_moveButtonList.push_back(_vm->_event->createButton(129, y, 4,          'l', _vm->_moveImages[8],  _vm->_moveImages[9]));
	_vm->_moveButtonList.push_back(_vm->_event->createButton(161, y, 5,          'i', _vm->_moveImages[12], _vm->_moveImages[13]));
	_vm->_moveButtonList.push_back(_vm->_event->createButton(193, y, 6, VKEY_LTARROW, _vm->_moveImages[14], _vm->_moveImages[15]));
	_vm->_moveButtonList.push_back(_vm->_event->createButton(225, y, 7, VKEY_UPARROW, _vm->_moveImages[16], _vm->_moveImages[17]));
	_vm->_moveButtonList.push_back(_vm->_event->createButton(257, y, 8, VKEY_RTARROW, _vm->_moveImages[18], _vm->_moveImages[19]));
	_vm->_moveButtonList.push_back(_vm->_event->createButton(289, y, 9,          'p', _vm->_moveImages[10], _vm->_moveImages[11]));

	Common::File *invFile = _vm->_resource->openDataFile("P:Inv");
	if (_vm->getPlatform() == Common::kPlatformWindows) {
		for (uint16 imgIdx = 0; imgIdx < 10; imgIdx++)
			_vm->_invImages[imgIdx] = new Image(invFile, _vm);
	} else {
		for (uint16 imgIdx = 0; imgIdx < 6; imgIdx++)
			_vm->_invImages[imgIdx] = new Image(invFile, _vm);
	}
	_vm->_invButtonList.push_back(_vm->_event->createButton( 24, y, 0,          'm', _vm->_invImages[0],   _vm->_invImages[1]));
	_vm->_invButtonList.push_back(_vm->_event->createButton( 56, y, 1,          'g', _vm->_invImages[2],   _vm->_invImages[3]));
	_vm->_invButtonList.push_back(_vm->_event->createButton( 94, y, 2,          'u', _vm->_invImages[4],   _vm->_invImages[5]));
	_vm->_invButtonList.push_back(_vm->_event->createButton(126, y, 3,          'l', _vm->_moveImages[8],  _vm->_moveImages[9]));
	_vm->_invButtonList.push_back(_vm->_event->createButton(164, y, 4, VKEY_LTARROW, _vm->_moveImages[14], _vm->_moveImages[15]));
	_vm->_invButtonList.push_back(_vm->_event->createButton(196, y, 5, VKEY_RTARROW, _vm->_moveImages[18], _vm->_moveImages[19]));

	// The windows version has 2 extra buttons for breadcrumb trail
	// TODO: the game is really hard to play without those, maybe we could add something to enable that.
	if (_vm->getPlatform() == Common::kPlatformWindows) {
		_vm->_invButtonList.push_back(_vm->_event->createButton(234, y, 6, 'b', _vm->_invImages[6], _vm->_invImages[7]));
		_vm->_invButtonList.push_back(_vm->_event->createButton(266, y, 7, 'f', _vm->_invImages[8], _vm->_invImages[9]));
	}

	delete invFile;
}

/**
 * Sets the pen number to use on all the drawing operations.
 */
void DisplayMan::setAPen(byte pennum) {
	_curapen = pennum;
}

/**
 * Fills in a rectangle.
 */
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
	rectFill(_vm->_utils->vgaScaleX(x1), _vm->_utils->vgaScaleY(y1), _vm->_utils->vgaScaleX(x2), _vm->_utils->vgaScaleY(y2));
}

/**
 * Draws a horizontal line.
 */
void DisplayMan::drawVLine(uint16 x, uint16 y1, uint16 y2) {
	rectFill(x, y1, x, y2);
}

/**
 * Draws a vertical line.
 */
void DisplayMan::drawHLine(uint16 x1, uint16 y, uint16 x2) {
	rectFill(x1, y, x2, y);
}

void DisplayMan::screenUpdate() {
	g_system->copyRectToScreen(_displayBuffer, _screenWidth, 0, 0, _screenWidth, _screenHeight);
	g_system->updateScreen();

	_vm->_event->processInput();
}

/**
 * Sets up either a low-res or a high-res 256 color screen.
 */
void DisplayMan::createScreen(bool hiRes) {
	if (hiRes) {
		_screenWidth  = 640;
		_screenHeight = 480;
	} else {
		_screenWidth  = 320;
		_screenHeight = 200;
	}
	_screenBytesPerPage = _screenWidth * _screenHeight;
	_displayBuffer = new byte[_screenBytesPerPage];	// FIXME: Memory leak!
}

/**
 * Converts an Amiga palette (up to 16 colors) to a VGA palette, then sets
 * the VGA palette.
 */
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

/**
 * Writes any number of the 256 color registers.
 * first:    the number of the first color register to write.
 * numreg:   the number of registers to write
 * buf:      a char pointer which contains the selected color registers.
 *           Each value representing a color register occupies 3 bytes in
 *           the array.  The order is red, green then blue.  The first byte
 *           in the array is the red component of the first element selected.
 *           The length of the buffer is 3 times the number of registers
 *           selected.
 */
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

/**
 * Returns the base address of the current VGA display.
 */
byte *DisplayMan::getCurrentDrawingBuffer() {
	if (_currentDisplayBuffer)
		return _currentDisplayBuffer;

	return _displayBuffer;
}

/**
 * Overlays a region on the screen using the desired pen color.
 */
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

/**
 * Closes a font and frees all memory associated with it.
 */
void DisplayMan::closeFont(TextFont *tf) {
	if (tf) {
		if (tf->_data && tf->_dataLength)
			delete[] tf->_data;

		delete tf;
	}
}

/**
 * Returns the length of a text in the specified font.
 */
uint16 DisplayMan::textLength(TextFont *tf, const char *text, uint16 numchars) {
	uint16 length = 0;

	if (tf) {
		for (uint16 i = 0; i < numchars; i++) {
			length += tf->_widths[(uint)*text];
			text++;
		}
	}

	return length;
}

/**
 * Returns the height of a specified font.
 */
uint16 DisplayMan::textHeight(TextFont *tf) {
	return (tf) ? tf->_height : 0;
}

/**
 * Draws the text to the screen.
 */
void DisplayMan::text(TextFont *tf, uint16 x, uint16 y, uint16 color, const char *text, uint16 numchars) {
	byte *VGATop, *VGACur, *VGATemp, *VGATempLine, *cdata;
	uint32 RealOffset, SegmentOffset;
	int32 templeft, LeftInSegment;
	uint16 bwidth, mask, curpage, data;

	VGATop = getCurrentDrawingBuffer();

	for (uint16 i = 0; i < numchars; i++) {
		RealOffset = (_screenWidth * y) + x;
		curpage    = RealOffset / _screenBytesPerPage;
		SegmentOffset = RealOffset - (curpage * _screenBytesPerPage);
		LeftInSegment = _screenBytesPerPage - SegmentOffset;
		VGACur = VGATop + SegmentOffset;

		if (tf->_widths[(uint)*text]) {
			cdata = tf->_data + tf->_offsets[(uint)*text];
			bwidth = *cdata++;
			VGATemp = VGACur;
			VGATempLine = VGACur;

			for (uint16 rows = 0; rows < tf->_height; rows++) {
				VGATemp = VGATempLine;
				templeft = LeftInSegment;

				for (uint16 cols = 0; cols < bwidth; cols++) {
					data = *cdata++;

					if (data && (templeft >= 8)) {
						for (int j = 7; j >= 0; j--) {
							if ((1 << j) & data)
								*VGATemp = color;
							VGATemp++;
						}

						templeft -= 8;
					} else if (data) {
						mask = 0x80;
						templeft = LeftInSegment;

						for (uint16 counterb = 0; counterb < 8; counterb++) {
							if (templeft <= 0) {
								curpage++;
								VGATemp = (byte *)(VGATop - templeft);
								// Set up VGATempLine for next line
								VGATempLine -= _screenBytesPerPage;
								// Set up LeftInSegment for next line
								LeftInSegment += _screenBytesPerPage + templeft;
								templeft += _screenBytesPerPage;
							}

							if (mask & data)
								*VGATemp = color;

							VGATemp++;

							mask = mask >> 1;
							templeft--;
						}
					} else {
						templeft -= 8;
						VGATemp += 8;
					}
				}

				VGATempLine += _screenWidth;
				LeftInSegment -= _screenWidth;

				if (LeftInSegment <= 0) {
					curpage++;
					VGATempLine -= _screenBytesPerPage;
					LeftInSegment += _screenBytesPerPage;
				}
			}
		}

		x += tf->_widths[(int)*text];
		text++;
	}
}

/**
 * Scrolls the display to black.
 */
void DisplayMan::doScrollBlack() {
	uint16 width = _vm->_utils->vgaScaleX(320);
	uint16 height = _vm->_utils->vgaScaleY(149) + _vm->_utils->svgaCord(2);
	byte *mem = new byte[width * height];

	_vm->_event->mouseHide();

	Image img(_vm);
	img._width = width;
	img._height = height;
	img._imageData = mem;
	_vm->_music->updateMusic();
	img.readScreenImage(0, 0);
	_vm->_music->updateMusic();

	byte *baseAddr = getCurrentDrawingBuffer();
	uint16 by = _vm->_utils->vgaScaleX(4);
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
		uint32 copysize;
		uint32 size = (int32)nheight * (int32)width;
		byte *tempmem = mem;

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

void DisplayMan::copyPage(uint16 width, uint16 height, uint16 nheight, uint16 startLine, byte *mem) {
	byte *baseAddr = getCurrentDrawingBuffer();

	uint32 size = (int32)(height - nheight) * (int32)width;
	mem += startLine * width;
	uint16 curPage = ((int32)nheight * (int32)width) / _screenBytesPerPage;
	uint32 offSet = ((int32)nheight * (int32)width) - (curPage * _screenBytesPerPage);

	while (size) {
		uint32 copySize;
		if (size > (_screenBytesPerPage - offSet))
			copySize = _screenBytesPerPage - offSet;
		else
			copySize = size;

		size -= copySize;

		memcpy(baseAddr + (offSet >> 2), mem, copySize);
		mem += copySize;
		curPage++;
		offSet = 0;
	}
}

/**
 * Scrolls the display to a new picture from a black screen.
 */
void DisplayMan::doScrollWipe(char *filename) {
	uint16 startLine = 0, onRow = 0;

	_vm->_event->mouseHide();
	uint16 width = _vm->_utils->vgaScaleX(320);
	uint16 height = _vm->_utils->vgaScaleY(149) + _vm->_utils->svgaCord(2);

	while (_vm->_music->isSoundEffectActive()) {
		_vm->_music->updateMusic();
		_vm->waitTOF();
	}

	readPict(filename, true, true);
	setPalette(_vm->_anim->_diffPalette, 256);
	byte *mem = _vm->_anim->_rawDiffBM._planes[0];

	_vm->_music->updateMusic();
	uint16 by = _vm->_utils->vgaScaleX(3);
	uint16 nheight = height;

	while (onRow < _vm->_anim->_headerdata._height) {
		_vm->_music->updateMusic();

		if ((by > nheight) && nheight)
			by = nheight;

		if ((startLine + by) > (_vm->_anim->_headerdata._height - height - 1))
			break;

		if (nheight)
			nheight -= by;

		copyPage(width, height, nheight, startLine, mem);

		screenUpdate();

		if (!nheight)
			startLine += by;

		onRow += by;

		if (nheight <= (height / 4))
			by = _vm->_utils->vgaScaleX(5);
		else if (nheight <= (height / 3))
			by = _vm->_utils->vgaScaleX(4);
		else if (nheight <= (height / 2))
			by = _vm->_utils->vgaScaleX(3);
	}

	_vm->_event->mouseShow();
}

/**
 * Does the scroll bounce.  Assumes bitmap already in memory.
 */
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
	int width = _vm->_utils->vgaScaleX(320);
	int height = _vm->_utils->vgaScaleY(149) + _vm->_utils->svgaCord(2);
	byte *mem = _vm->_anim->_rawDiffBM._planes[0];

	_vm->_music->updateMusic();
	int startLine = _vm->_anim->_headerdata._height - height - 1;

	for (int i = 0; i < 5; i++) {
		_vm->_music->updateMusic();
		startLine -= newby[i];
		copyPage(width, height, 0, startLine, mem);

		screenUpdate();
		_vm->waitTOF();
	}

	for (int i = 8; i > 0; i--) {
		_vm->_music->updateMusic();
		startLine += newby1[i - 1];
		copyPage(width, height, 0, startLine, mem);

		screenUpdate();
		_vm->waitTOF();
	}

	_vm->_event->mouseShow();
}

/**
 * Does the transporter wipe.
 */
void DisplayMan::doTransWipe(CloseDataPtr *closePtrList, char *filename) {
	uint16 lastY, curY, linesDone = 0, linesLast;

	if (_vm->_isHiRes) {
		linesLast = 3;
		lastY = 358;
	} else {
		linesLast = 1;
		lastY = 148;
	}

	for (uint16 j = 0; j < 2; j++) {
		for (uint16 i = 0; i < 2; i++) {
			curY = i * 2;

			while (curY < lastY) {
				if (linesDone >= linesLast) {
					_vm->_music->updateMusic();
					_vm->waitTOF();
					linesDone = 0;
				}

				if (j == 9)
					overlayRect(0, 0, curY, _screenWidth - 1, curY + 1);
				else
					rectFill(0, curY, _screenWidth - 1, curY + 1);
				curY += 4;
				linesDone++;
			}	// while
		}	// for i

		setAPen(0);
	}	// for j

	if (filename == NULL)
		_vm->_curFileName = _vm->getPictName(closePtrList);
	else if (filename[0] > ' ')
		_vm->_curFileName = filename;
	else
		_vm->_curFileName = _vm->getPictName(closePtrList);

	byte *bitMapBuffer = new byte[_screenWidth * (lastY + 5)];
	readPict(_vm->_curFileName, true, false, bitMapBuffer, lastY + 5);

	setPalette(_vm->_anim->_diffPalette, 256);

	Image imSource(_vm);
	imSource._width = _screenWidth;
	imSource._height = lastY;
	imSource._imageData = bitMapBuffer;

	Image imDest(_vm);
	imDest._width = _screenWidth;
	imDest._height = _screenHeight;
	imDest._imageData = getCurrentDrawingBuffer();

	for (uint16 j = 0; j < 2; j++) {
		for (uint16 i = 0; i < 2; i++) {
			curY = i * 2;

			while (curY < lastY) {
				if (linesDone >= linesLast) {
					_vm->_music->updateMusic();
					_vm->waitTOF();
					linesDone = 0;
				}

				imDest._imageData = getCurrentDrawingBuffer();

				if (j == 0) {
					imSource.blitBitmap(0, curY, &imDest, 0, curY, _screenWidth, 2, false);
					overlayRect(0, 0, curY, _screenWidth - 1, curY + 1);
				} else {
					uint16 bitmapHeight = (curY == lastY) ? 1 : 2;
					imSource.blitBitmap(0, curY, &imDest, 0, curY, _screenWidth, bitmapHeight, false);
				}
				curY += 4;
				linesDone++;
			}	// while
		}	// for i
	}	// for j

	delete[] bitMapBuffer;
}

/**
 * Does a certain number of pre-programmed wipes.
 */
void DisplayMan::doTransition(TransitionType transitionType, CloseDataPtr *closePtrList, char *filename) {
	switch (transitionType) {
	case kTransitionWipe:
	case kTransitionTransporter:
		doTransWipe(closePtrList, filename);
		break;
	case kTransitionScrollWipe:
		doScrollWipe(filename);
		break;
	case kTransitionScrollBlack:
		doScrollBlack();
		break;
	case kTransitionScrollBounce:
		doScrollBounce();
		break;
	case kTransitionReadFirstFrame:
		readPict(filename, false);
		break;
	case kTransitionReadNextFrame:
		_vm->_anim->diffNextFrame();
		break;
	case kTransitionNone:
	default:
		break;
	}
}

/**
 * Changes the front screen to black.
 */
void DisplayMan::blackScreen() {
	byte pal[256 * 3];
	memset(pal, 0, 248 * 3);
	writeColorRegs(pal, 8, 248);

	g_system->delayMillis(32);
}

/**
 * Changes the front screen to white.
 */
void DisplayMan::whiteScreen() {
	byte pal[256 * 3];
	memset(pal, 255, 248 * 3);
	writeColorRegs(pal, 8, 248);
}

/**
 * Changes the entire screen to black.
*/
void DisplayMan::blackAllScreen() {
	byte pal[256 * 3];
	memset(pal, 0, 256 * 3);
	writeColorRegs(pal, 0, 256);

	g_system->delayMillis(32);
}

/**
 * Scrolls the display in the x direction by blitting.
 * The _tempScrollData variable must be initialized to some memory, or this
 * function will fail.
 */
void DisplayMan::scrollDisplayX(int16 dx, uint16 x1, uint16 y1, uint16 x2, uint16 y2, byte *buffer) {
	Image im(_vm);
	im._imageData = buffer;

	if (x1 > x2)
		SWAP<uint16>(x1, x2);

	if (y1 > y2)
		SWAP<uint16>(y1, y2);

	if (dx > 0) {
		im._width = x2 - x1 + 1 - dx;
		im._height = y2 - y1 + 1;

		im.readScreenImage(x1, y1);
		im.drawImage(x1 + dx, y1);

		setAPen(0);
		rectFill(x1, y1, x1 + dx - 1, y2);
	} else if (dx < 0) {
		im._width = x2 - x1 + 1 + dx;
		im._height = y2 - y1 + 1;

		im.readScreenImage(x1 - dx, y1);
		im.drawImage(x1, y1);

		setAPen(0);
		rectFill(x2 + dx + 1, y1, x2, y2);
	}
}

/**
 * Scrolls the display in the y direction by blitting.
 */
void DisplayMan::scrollDisplayY(int16 dy, uint16 x1, uint16 y1, uint16 x2, uint16 y2, byte *buffer) {
	Image im(_vm);
	im._imageData = buffer;

	if (x1 > x2)
		SWAP<uint16>(x1, x2);

	if (y1 > y2)
		SWAP<uint16>(y1, y2);

	if (dy > 0) {
		im._width = x2 - x1 + 1;
		im._height = y2 - y1 + 1 - dy;

		im.readScreenImage(x1, y1);
		im.drawImage(x1, y1 + dy);

		setAPen(0);
		rectFill(x1, y1, x2, y1 + dy - 1);
	} else if (dy < 0) {
		im._width = x2 - x1 + 1;
		im._height = y2 - y1 + 1 + dy;

		im.readScreenImage(x1, y1 - dy);
		im.drawImage(x1, y1);

		setAPen(0);
		rectFill(x1, y2 + dy + 1, x2, y2);
	}
}

/**
 * Does the fading of the Palette on the screen.
 */
uint16 DisplayMan::fadeNumIn(uint16 num, uint16 res, uint16 counter) {
	return (num - ((((int32)(15 - counter)) * ((int32)(num - res))) / 15));
}

uint16 DisplayMan::fadeNumOut(uint16 num, uint16 res, uint16 counter) {
	return (num - ((((int32) counter) * ((int32)(num - res))) / 15));
}

void DisplayMan::fade(bool fadeIn, uint16 res) {
	uint16 newPal[16];

	for (uint16 i = 0; i < 16; i++) {
		for (uint16 palIdx = 0; palIdx < 16; palIdx++) {
			if (fadeIn)
				newPal[palIdx] = (0x00F & fadeNumIn(0x00F & _fadePalette[palIdx], 0x00F & res, i)) +
				(0x0F0 & fadeNumIn(0x0F0 & _fadePalette[palIdx], 0x0F0 & res, i)) +
				(0xF00 & fadeNumIn(0xF00 & _fadePalette[palIdx], 0xF00 & res, i));
			else
				newPal[palIdx] = (0x00F & fadeNumOut(0x00F & _fadePalette[palIdx], 0x00F & res, i)) +
				(0x0F0 & fadeNumOut(0x0F0 & _fadePalette[palIdx], 0x0F0 & res, i)) +
				(0xF00 & fadeNumOut(0xF00 & _fadePalette[palIdx], 0xF00 & res, i));
		}

		setAmigaPal(newPal, 16);
		_vm->waitTOF();
		_vm->_music->updateMusic();
	}
}

} // End of namespace Lab
