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
#include "lab/processroom.h"
#include "lab/graphics.h"

namespace Lab {

/*****************************************************************************/
/* Scrolls the display to black.                                             */
/*****************************************************************************/
void DisplayMan::doScrollBlack() {
	byte *tempmem;
	Image im;
	uint32 size, copysize;
	byte *baseAddr;
	uint16 width = _vm->_utils->vgaScaleX(320);
	uint16 height = _vm->_utils->vgaScaleY(149) + _vm->_utils->svgaCord(2);
	byte *mem = new byte[width * height];

	_vm->_event->mouseHide();

	im._width = width;
	im._height = height;
	im._imageData = mem;
	_vm->_music->updateMusic();
	im.readScreenImage(0, 0);
	_vm->_music->updateMusic();

	baseAddr = getCurrentDrawingBuffer();

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
	uint16 width = _vm->_utils->vgaScaleX(320);
	uint16 height = _vm->_utils->vgaScaleY(149) + _vm->_utils->svgaCord(2);

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
	uint16 by = _vm->_utils->vgaScaleX(3);
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
			by = _vm->_utils->vgaScaleX(5);
		else if (nheight <= (height / 3))
			by = _vm->_utils->vgaScaleX(4);
		else if (nheight <= (height / 2))
			by = _vm->_utils->vgaScaleX(3);
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
	int width = _vm->_utils->vgaScaleX(320);
	int height = _vm->_utils->vgaScaleY(149) + _vm->_utils->svgaCord(2);
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

	for (uint16 j = 0; j < 2; j++) {
		for (uint16 i = 0; i < 2; i++) {
			curY = i * 2;

			while (curY < lastY) {
				if (linesdone >= lineslast) {
					_vm->_music->updateMusic();
					_vm->waitTOF();
					linesdone = 0;
				}

				if (j == 9)
					overlayRect(0, 0, curY, _screenWidth - 1, curY + 1);
				else
					rectFill(0, curY, _screenWidth - 1, curY + 1);
				curY += 4;
				linesdone++;
			}	// while
		}	// for i

		setAPen(0);
	}	// for j

	if (filename == NULL)
		_vm->_curFileName = g_lab->getPictName(cPtr);
	else if (filename[0] > ' ')
		_vm->_curFileName = filename;
	else
		_vm->_curFileName = g_lab->getPictName(cPtr);

	byte *BitMapMem = readPictToMem(_vm->_curFileName, _screenWidth, lastY + 5);
	setPalette(_vm->_anim->_diffPalette, 256);

	imSource._width = _screenWidth;
	imSource._height = lastY;
	imSource._imageData = BitMapMem;

	imDest._width = _screenWidth;
	imDest._height = _screenHeight;
	imDest._imageData = getCurrentDrawingBuffer();

	for (uint16 j = 0; j < 2; j++) {
		for (uint16 i = 0; i < 2; i++) {
			curY = i * 2;

			while (curY < lastY) {
				if (linesdone >= lineslast) {
					_vm->_music->updateMusic();
					_vm->waitTOF();
					linesdone = 0;
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
				linesdone++;
			}	// while
		}	// for i
	}	// for j
}

/*****************************************************************************/
/* Does a certain number of pre-programmed wipes.                            */
/*****************************************************************************/
void DisplayMan::doTransition(TransitionType transitionType, CloseDataPtr *cPtr, char *filename) {
	switch (transitionType) {
	case kTransitionWipe:
	case kTransitionTransporter:
		doTransWipe(cPtr, filename);
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

/*****************************************************************************/
/* Scrolls the display in the x direction by blitting.                       */
/* The _tempScrollData variable must be initialized to some memory, or this   */
/* function will fail.                                                       */
/*****************************************************************************/
void DisplayMan::scrollDisplayX(int16 dx, uint16 x1, uint16 y1, uint16 x2, uint16 y2) {
	Image im;

	im._imageData = _tempScrollData;

	if (x1 > x2)
		SWAP<uint16>(x1, x2);

	if (y1 > y2)
		SWAP<uint16>(y1, y2);

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

	im._imageData = _tempScrollData;

	if (x1 > x2)
		SWAP<uint16>(x1, x2);

	if (y1 > y2)
		SWAP<uint16>(y1, y2);

	im._width = x2 - x1 + 1;
	im._height = y2 - y1 + 1 - dy;

	im.readScreenImage(x1, y1);
	im.drawImage(x1, y1 + dy);

	setAPen(0);
	rectFill(x1, y1, x2, y1 + dy - 1);
}

/*****************************************************************************/
/* Does the fading of the Palette on the screen.                             */
/*****************************************************************************/
uint16 DisplayMan::fadeNumIn(uint16 num, uint16 res, uint16 counter) {
	return (num - ((((int32)(15 - counter)) * ((int32)(num - res))) / 15));
}


uint16 DisplayMan::fadeNumOut(uint16 num, uint16 res, uint16 counter) {
	return (num - ((((int32) counter) * ((int32)(num - res))) / 15));
}

void DisplayMan::fade(bool fadein, uint16 res) {
	uint16 newpal[16];

	for (uint16 i = 0; i < 16; i++) {
		for (uint16 palIdx = 0; palIdx < 16; palIdx++) {
			if (fadein)
				newpal[palIdx] = (0x00F & fadeNumIn(0x00F & FadePalette[palIdx], 0x00F & res, i)) +
				(0x0F0 & fadeNumIn(0x0F0 & FadePalette[palIdx], 0x0F0 & res, i)) +
				(0xF00 & fadeNumIn(0xF00 & FadePalette[palIdx], 0xF00 & res, i));
			else
				newpal[palIdx] = (0x00F & fadeNumOut(0x00F & FadePalette[palIdx], 0x00F & res, i)) +
				(0x0F0 & fadeNumOut(0x0F0 & FadePalette[palIdx], 0x0F0 & res, i)) +
				(0xF00 & fadeNumOut(0xF00 & FadePalette[palIdx], 0xF00 & res, i));
		}

		setAmigaPal(newpal, 16);
		_vm->waitTOF();
		_vm->_music->updateMusic();
	}
}

} // End of namespace Lab
