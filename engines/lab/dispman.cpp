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
#include "lab/music.h"
#include "lab/image.h"
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
	_tempScrollData = nullptr;
	FadePalette = nullptr;

	_screenWidth = 0;
	_screenHeight = 0;

	for (int i = 0; i < 256 * 3; i++)
		_curvgapal[i] = 0;
}

DisplayMan::~DisplayMan() {
	freePict();
}

// From readPict.c.  Reads in pictures and animations from disk.

void DisplayMan::loadPict(const char *filename) {
	Common::File *bitmapFile = _vm->_resource->openDataFile(filename);
	freePict();
	_curBitmap = new byte[bitmapFile->size()];
	bitmapFile->read(_curBitmap, bitmapFile->size());
	delete bitmapFile;
}

/**
 * Reads in a picture into the dest bitmap.
 */
void DisplayMan::readPict(const char *filename, bool playOnce, bool onlyDiffData) {
	_vm->_anim->stopDiff();

	loadPict(filename);

	_vm->_music->updateMusic();

	if (!_vm->_music->_doNotFilestopSoundEffect)
		_vm->_music->stopSoundEffect();

	_dispBitMap._bytesPerRow = _screenWidth;
	_dispBitMap._rows        = _screenHeight;
	_dispBitMap._flags       = BITMAPF_VIDEO;

	_vm->_anim->readDiff(_curBitmap, playOnce, onlyDiffData);
}

/**
 * Reads in a picture into buffer memory.
 */
byte *DisplayMan::readPictToMem(const char *filename, uint16 x, uint16 y) {
	_vm->_anim->stopDiff();

	loadPict(filename);

	_vm->_music->updateMusic();

	if (!_vm->_music->_doNotFilestopSoundEffect)
		_vm->_music->stopSoundEffect();

	_dispBitMap._bytesPerRow = x;
	_dispBitMap._rows = y;
	_dispBitMap._flags = BITMAPF_NONE;
	_dispBitMap._planes[0] = _curBitmap;
	_dispBitMap._planes[1] = _dispBitMap._planes[0] + 0x10000;
	_dispBitMap._planes[2] = _dispBitMap._planes[1] + 0x10000;
	_dispBitMap._planes[3] = _dispBitMap._planes[2] + 0x10000;
	_dispBitMap._planes[4] = _dispBitMap._planes[3] + 0x10000;

	_vm->_anim->readDiff(_curBitmap, true);

	return _curBitmap;
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
static void getWord(char *wordBuffer, const char *mainBuffer, uint16 *wordWidth) {
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

	_vm->_event->attachGadgetList(NULL);
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
	// Gadget Separators
	setAPen(0);
	// First black line to separate buttons
	drawHLine(0, _vm->_utils->vgaScaleY(170), _vm->_utils->vgaScaleX(319));

	if (!_vm->_alternate) {
		setAPen(4);
		// The horizontal lines under the black one
		drawHLine(0, _vm->_utils->vgaScaleY(170) + 1, _vm->_utils->vgaScaleX(319));
		drawGadgetList(&_vm->_moveGadgetList);
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

		drawGadgetList(&_vm->_invGadgetList);
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
		_vm->_moveImages[i] = new Image(controlFile);
	delete controlFile;

	// Creates the gadgets for the movement control panel
	uint16 y = _vm->_utils->vgaScaleY(173) - _vm->_utils->svgaCord(2);

	// The key mapping was only set for the Windows version.
	// It's very convenient to have those shortcut, so I added them
	// for all versions. (Strangerke)
	_vm->_moveGadgetList.push_back(createButton(  1, y, 0,          't', _vm->_moveImages[0],  _vm->_moveImages[1]));
	_vm->_moveGadgetList.push_back(createButton( 33, y, 1,          'm', _vm->_moveImages[2],  _vm->_moveImages[3]));
	_vm->_moveGadgetList.push_back(createButton( 65, y, 2,          'o', _vm->_moveImages[4],  _vm->_moveImages[5]));
	_vm->_moveGadgetList.push_back(createButton( 97, y, 3,          'c', _vm->_moveImages[6],  _vm->_moveImages[7]));
	_vm->_moveGadgetList.push_back(createButton(129, y, 4,          'l', _vm->_moveImages[8],  _vm->_moveImages[9]));
	_vm->_moveGadgetList.push_back(createButton(161, y, 5,          'i', _vm->_moveImages[12], _vm->_moveImages[13]));
	_vm->_moveGadgetList.push_back(createButton(193, y, 6, VKEY_LTARROW, _vm->_moveImages[14], _vm->_moveImages[15]));
	_vm->_moveGadgetList.push_back(createButton(225, y, 7, VKEY_UPARROW, _vm->_moveImages[16], _vm->_moveImages[17]));
	_vm->_moveGadgetList.push_back(createButton(257, y, 8, VKEY_RTARROW, _vm->_moveImages[18], _vm->_moveImages[19]));
	_vm->_moveGadgetList.push_back(createButton(289, y, 9,          'p', _vm->_moveImages[10], _vm->_moveImages[11]));

	Common::File *invFile = _vm->_resource->openDataFile("P:Inv");
	if (_vm->getPlatform() == Common::kPlatformWindows) {
		for (uint16 imgIdx = 0; imgIdx < 10; imgIdx++)
			_vm->_invImages[imgIdx] = new Image(invFile);
	} else {
		for (uint16 imgIdx = 0; imgIdx < 6; imgIdx++)
			_vm->_invImages[imgIdx] = new Image(invFile);
	}
	_vm->_invGadgetList.push_back(createButton( 24, y, 0,          'm', _vm->_invImages[0],   _vm->_invImages[1]));
	_vm->_invGadgetList.push_back(createButton( 56, y, 1,          'g', _vm->_invImages[2],   _vm->_invImages[3]));
	_vm->_invGadgetList.push_back(createButton( 94, y, 2,          'u', _vm->_invImages[4],   _vm->_invImages[5]));
	_vm->_invGadgetList.push_back(createButton(126, y, 3,          'l', _vm->_moveImages[8],  _vm->_moveImages[9]));
	_vm->_invGadgetList.push_back(createButton(164, y, 4, VKEY_LTARROW, _vm->_moveImages[14], _vm->_moveImages[15]));
	_vm->_invGadgetList.push_back(createButton(196, y, 5, VKEY_RTARROW, _vm->_moveImages[18], _vm->_moveImages[19]));

	// The windows version has 2 extra gadgets for breadcrumb trail
	// TODO: the game is really hard to play without those, maybe we could add something to enable that.
	if (_vm->getPlatform() == Common::kPlatformWindows) {
		_vm->_invGadgetList.push_back(createButton(234, y, 6, 'b', _vm->_invImages[6], _vm->_invImages[7]));
		_vm->_invGadgetList.push_back(createButton(266, y, 7, 'f', _vm->_invImages[8], _vm->_invImages[9]));
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
} // End of namespace Lab
