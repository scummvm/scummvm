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

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

// Display.c - DIB related code for HUGOWIN

#include "common/debug.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/cursorman.h"
#include "graphics/palette.h"

#include "hugo/hugo.h"
#include "hugo/display.h"
#include "hugo/inventory.h"
#include "hugo/util.h"
#include "hugo/object.h"
#include "hugo/mouse.h"

namespace Hugo {
/**
 * A black and white Windows-style arrow cursor (12x20).
 * 0 = Black (#000000 in 24-bit RGB).
 * 1 = Transparent.
 * 15 = White (#FFFFFF in 24-bit RGB).
 * This cursor comes from Mohawk engine.
 */
static const byte stdMouseCursor[] = {
	0, 0,  1,  1,  1,  1,  1,  1,  1,  1,  1, 1,
	0, 15, 0,  1,  1,  1,  1,  1,  1,  1,  1, 1,
	0, 15, 15, 0,  1,  1,  1,  1,  1,  1,  1, 1,
	0, 15, 15, 15, 0,  1,  1,  1,  1,  1,  1, 1,
	0, 15, 15, 15, 15, 0,  1,  1,  1,  1,  1, 1,
	0, 15, 15, 15, 15, 15, 0,  1,  1,  1,  1, 1,
	0, 15, 15, 15, 15, 15, 15, 0,  1,  1,  1, 1,
	0, 15, 15, 15, 15, 15, 15, 15, 0,  1,  1, 1,
	0, 15, 15, 15, 15, 15, 15, 15, 15, 0,  1, 1,
	0, 15, 15, 15, 15, 15, 15, 15, 15, 15, 0, 1,
	0, 15, 15, 15, 15, 15, 15, 0,  0,  0,  0, 0,
	0, 15, 15, 15, 0,  15, 15, 0,  1,  1,  1, 1,
	0, 15, 15, 0,  0,  15, 15, 0,  1,  1,  1, 1,
	0, 15, 0,  1,  0,  0,  15, 15, 0,  1,  1, 1,
	0, 0,  1,  1,  1,  0,  15, 15, 0,  1,  1, 1,
	0, 1,  1,  1,  1,  1,  0,  15, 15, 0,  1, 1,
	1, 1,  1,  1,  1,  1,  0,  15, 15, 0,  1, 1,
	1, 1,  1,  1,  1,  1,  1,  0,  15, 15, 0, 1,
	1, 1,  1,  1,  1,  1,  1,  0,  15, 15, 0, 1,
	1, 1,  1,  1,  1,  1,  1,  1,  0,  0,  1, 1
};


Screen::Screen(HugoEngine *vm) : _vm(vm) {
	_mainPalette = 0;
	_curPalette = 0;
	_dlAddIndex = 0;
	_dlRestoreIndex = 0;

	for (int i = 0; i < kNumFonts; i++) {
		_arrayFont[i] = 0;
		fontLoadedFl[i] = false;
	}
	for (int i = 0; i < kBlitListSize; i++) {
		_dlBlistList[i].x = 0;
		_dlBlistList[i].y = 0;
		_dlBlistList[i].dx = 0;
		_dlBlistList[i].dy = 0;
	}
	for (int i = 0; i < kRectListSize; i++) {
		_dlAddList[i].x = 0;
		_dlAddList[i].y = 0;
		_dlAddList[i].dx = 0;
		_dlAddList[i].dy = 0;
		_dlRestoreList[i].x = 0;
		_dlRestoreList[i].y = 0;
		_dlRestoreList[i].dx = 0;
		_dlRestoreList[i].dy = 0;
	}
}

Screen::~Screen() {
}

icondib_t &Screen::getIconBuffer() {
	return _iconBuffer;
}

viewdib_t &Screen::getBackBuffer() {
	return _backBuffer;
}

viewdib_t &Screen::getBackBufferBackup() {
	return _backBufferBackup;
}

viewdib_t &Screen::getFrontBuffer() {
	return _frontBuffer;
}

viewdib_t &Screen::getGUIBuffer() {
	return _GUIBuffer;
}

/**
 * Replace the palette by the main palette
 */
void Screen::createPal() {
	debugC(1, kDebugDisplay, "createPal");
	g_system->getPaletteManager()->setPalette(_mainPalette, 0, _paletteSize / 3);
}

void Screen::setCursorPal() {
	debugC(1, kDebugDisplay, "setCursorPal");
	CursorMan.replaceCursorPalette(_curPalette, 0, _paletteSize / 3);
}

/**
 * Create logical palette
 */
void Screen::initDisplay() {
	debugC(1, kDebugDisplay, "initDisplay");
	createPal();
}

/**
 * Move an image from source to destination
 */
void Screen::moveImage(image_pt srcImage, const int16 x1, const int16 y1, const int16 dx, int16 dy, const int16 width1, image_pt dstImage, const int16 x2, const int16 y2, const int16 width2) {
	debugC(3, kDebugDisplay, "moveImage(srcImage, %d, %d, %d, %d, %d, dstImage, %d, %d, %d)", x1, y1, dx, dy, width1, x2, y2, width2);

	int16 wrap_src = width1 - dx;                   // Wrap to next src row
	int16 wrap_dst = width2 - dx;                   // Wrap to next dst row

	srcImage += y1 * width1 + x1;                   // Offset into src image
	dstImage += y2 * width2 + x2;                   // offset into dst image

	while (dy--) {                                  // For each row
		for (int16 x = dx; x--;)                    // For each column
			*dstImage++ = *srcImage++;
		srcImage += wrap_src;                       // Wrap to next line
		dstImage += wrap_dst;
	}
}

void Screen::displayBackground() {
	debugC(1, kDebugDisplay, "displayBackground()");

	g_system->copyRectToScreen(_frontBuffer, 320, 0, 0, 320, 200);
}

/**
 * Blit the supplied rectangle from _frontBuffer to the screen
 */
void Screen::displayRect(const int16 x, const int16 y, const int16 dx, const int16 dy) {
	debugC(3, kDebugDisplay, "displayRect(%d, %d, %d, %d)", x, y, dx, dy);

	int16 xClip, yClip;
	xClip = CLIP<int16>(x, 0, 319);
	yClip = CLIP<int16>(y, 0, 199);
	g_system->copyRectToScreen(&_frontBuffer[xClip + yClip * 320], 320, xClip, yClip, CLIP<int16>(dx, 0, 319 - xClip), CLIP<int16>(dy, 0, 199 - yClip));
}

/**
 * Change a color by remapping supplied palette index with new index in main palette.
 * Alse save the new color in the current palette.
 */
void Screen::remapPal(const uint16 oldIndex, const uint16 newIndex) {
	debugC(1, kDebugDisplay, "RemapPal(%d, %d)", oldIndex, newIndex);

	_curPalette[3 * oldIndex + 0] = _mainPalette[newIndex * 3 + 0];
	_curPalette[3 * oldIndex + 1] = _mainPalette[newIndex * 3 + 1];
	_curPalette[3 * oldIndex + 2] = _mainPalette[newIndex * 3 + 2];

	g_system->getPaletteManager()->setPalette(_curPalette, 0, _paletteSize / 3);
}

/**
 * Saves the current palette in a savegame
 */
void Screen::savePal(Common::WriteStream *f) const {
	debugC(1, kDebugDisplay, "savePal()");

	for (int i = 0; i < _paletteSize; i++)
		f->writeByte(_curPalette[i]);
}

/**
 * Restore the current palette from a savegame
 */
void Screen::restorePal(Common::ReadStream *f) {
	debugC(1, kDebugDisplay, "restorePal()");

	for (int i = 0; i < _paletteSize; i++)
		_curPalette[i] = f->readByte();

	g_system->getPaletteManager()->setPalette(_curPalette, 0, _paletteSize / 3);
}


/**
 * Set the new background color.
 * This implementation gives the same result than the DOS version.
 * It wasn't implemented in the Win version
 */
void Screen::setBackgroundColor(const uint16 color) {
	debugC(1, kDebugDisplay, "setBackgroundColor(%d)", color);

	remapPal(0, color);
}

/**
 * Merge an object frame into _frontBuffer at sx, sy and update rectangle list.
 * If fore TRUE, force object above any overlay
 */
void Screen::displayFrame(const int sx, const int sy, seq_t *seq, const bool foreFl) {
	debugC(3, kDebugDisplay, "displayFrame(%d, %d, seq, %d)", sx, sy, (foreFl) ? 1 : 0);

	image_pt image = seq->imagePtr;                 // Ptr to object image data
	image_pt subFrontBuffer = &_frontBuffer[sy * kXPix + sx]; // Ptr to offset in _frontBuffer
	int16 frontBufferwrap = kXPix - seq->x2 - 1;     // Wraps dest_p after each line
	int16 imageWrap = seq->bytesPerLine8 - seq->x2 - 1;
	overlayState_t overlayState = (foreFl) ? kOvlForeground : kOvlUndef; // Overlay state of object
	for (uint16 y = 0; y < seq->lines; y++) {       // Each line in object
		for (uint16 x = 0; x <= seq->x2; x++) {
			if (*image) {                           // Non-transparent
				byte ovlBound = _vm->_object->getFirstOverlay((uint16)(subFrontBuffer - _frontBuffer) >> 3); // Ptr into overlay bits
				if (ovlBound & (0x80 >> ((uint16)(subFrontBuffer - _frontBuffer) & 7))) { // Overlay bit is set
					if (overlayState == kOvlUndef)  // Overlay defined yet?
						overlayState = findOvl(seq, subFrontBuffer, y);// No, find it.
					if (overlayState == kOvlForeground) // Object foreground
						*subFrontBuffer = *image;   // Copy pixel
				} else {                            // No overlay
					*subFrontBuffer = *image;       // Copy pixel
				}
			}
			image++;
			subFrontBuffer++;
		}
		image += imageWrap;
		subFrontBuffer += frontBufferwrap;
	}

	// Add this rectangle to the display list
	displayList(kDisplayAdd, sx, sy, seq->x2 + 1, seq->lines);
}

/**
 * Merge rectangles A,B leaving result in B
 */
void Screen::merge(const rect_t *rectA, rect_t *rectB) {
	debugC(6, kDebugDisplay, "merge()");

	int16 xa = rectA->x + rectA->dx;                // Find x2,y2 for each rectangle
	int16 xb = rectB->x + rectB->dx;
	int16 ya = rectA->y + rectA->dy;
	int16 yb = rectB->y + rectB->dy;

	rectB->x = MIN(rectA->x, rectB->x);             // Minimum x,y
	rectB->y = MIN(rectA->y, rectB->y);
	rectB->dx = MAX(xa, xb) - rectB->x;             // Maximum dx,dy
	rectB->dy = MAX(ya, yb) - rectB->y;
}

/**
 * Coalesce the rectangles in the restore/add list into one unified
 * blist.  len is the sizes of alist or rlist.  blen is current length
 * of blist.  bmax is the max size of the blist.  Note that blist can
 * have holes, in which case dx = 0.  Returns used length of blist.
 */
int16 Screen::mergeLists(rect_t *list, rect_t *blist, const int16 len, int16 blen) {
	debugC(4, kDebugDisplay, "mergeLists()");

	int16   coalesce[kBlitListSize];                // List of overlapping rects
	// Process the list
	for (int16 a = 0; a < len; a++, list++) {
		// Compile list of overlapping rectangles in blit list
		int16 c = 0;
		rect_t *bp = blist;
		for (int16 b = 0; b < blen; b++, bp++) {
			if (bp->dx)                             // blist entry used
				if (isOverlapping(list, bp))
					coalesce[c++] = b;
		}

		// Any overlapping blit rects?
		if (c == 0) {                               // None, add a new entry
			blist[blen++] = *list;
		} else {                                    // At least one overlapping
			// Merge add-list entry with first blist entry
			bp = &blist[coalesce[0]];
			merge(list, bp);

			// Merge any more blist entries
			while (--c) {
				rect_t *cp = &blist[coalesce[c]];
				merge(cp, bp);
				cp->dx = 0;                         // Delete entry
			}
		}
	}
	return blen;
}

/**
 * Process the display list
 * Trailing args are int16 x,y,dx,dy for the D_ADD operation
 */
void Screen::displayList(dupdate_t update, ...) {
	debugC(6, kDebugDisplay, "displayList()");

	int16         blitLength = 0;                   // Length of blit list
	va_list       marker;                           // Args used for D_ADD operation
	rect_t       *p;                                // Ptr to dlist entry

	switch (update) {
	case kDisplayInit:                              // Init lists, restore whole screen
		_dlAddIndex = _dlRestoreIndex = 0;
		memcpy(_frontBuffer, _backBuffer, sizeof(_frontBuffer));
		break;
	case kDisplayAdd:                               // Add a rectangle to list
		if (_dlAddIndex >= kRectListSize) {
			warning("Display list exceeded");
			return;
		}
		va_start(marker, update);                   // Initialize variable arguments
		p = &_dlAddList[_dlAddIndex];
		p->x  = va_arg(marker, int);                // x
		p->y  = va_arg(marker, int);                // y
		p->dx = va_arg(marker, int);                // dx
		p->dy = va_arg(marker, int);                // dy
		va_end(marker);                             // Reset variable arguments
		_dlAddIndex++;
		break;
	case kDisplayDisplay:                           // Display whole list
		// Don't blit if newscreen just loaded because _frontBuffer will
		// get blitted via InvalidateRect() at end of this cycle
		// and blitting here causes objects to appear too soon.
		if (_vm->getGameStatus().newScreenFl) {
			_vm->getGameStatus().newScreenFl = false;
			break;
		}

		// Coalesce restore-list, add-list into combined blit-list
		blitLength = mergeLists(_dlRestoreList, _dlBlistList, _dlRestoreIndex, blitLength);
		blitLength = mergeLists(_dlAddList, _dlBlistList, _dlAddIndex, blitLength);

		// Blit the combined blit-list
		for (_dlRestoreIndex = 0, p = _dlBlistList; _dlRestoreIndex < blitLength; _dlRestoreIndex++, p++) {
			if (p->dx)                              // Marks a used entry
				displayRect(p->x, p->y, p->dx, p->dy);
		}
		break;
	case kDisplayRestore:                           // Restore each rectangle
		for (_dlRestoreIndex = 0, p = _dlAddList; _dlRestoreIndex < _dlAddIndex; _dlRestoreIndex++, p++) {
			// Restoring from _backBuffer to _frontBuffer
			_dlRestoreList[_dlRestoreIndex] = *p;   // Copy add-list to restore-list
			moveImage(_backBuffer, p->x, p->y, p->dx, p->dy, kXPix, _frontBuffer, p->x, p->y, kXPix);
		}
		_dlAddIndex = 0;                            // Reset add-list
		break;
	}
}

/**
 * Write supplied character (font data) at sx,sy in supplied color
 * Font data as follows:
 * *(fontdata+1) = Font Height (pixels)
 * *(fontdata+1) = Font Width (pixels)
 * *(fontdata+x) = Font Bitmap (monochrome)
 */
void Screen::writeChr(const int sx, const int sy, const byte color, const char *local_fontdata){
	debugC(2, kDebugDisplay, "writeChr(%d, %d, %d, %d)", sx, sy, color, local_fontdata[0]);

	byte height = local_fontdata[0];
	byte width = 8; //local_fontdata[1];

	// This can probably be optimized quite a bit...
	for (int y = 0; y < height; ++y) {
		for (int x = 0; x < width; ++x) {
			int pixel = y * width + x;
			int bitpos = pixel % 8;
			int offset = pixel / 8;
			byte bitTest = (1 << bitpos);
			if ((local_fontdata[2 + offset] & bitTest) == bitTest)
				_frontBuffer[(sy + y) * 320 + sx + x] = color;
		}
	}
}

/**
 * Returns height of characters in current font
 */
int16 Screen::fontHeight() const {
	debugC(2, kDebugDisplay, "fontHeight()");

	static const int16 height[kNumFonts] = {5, 7, 8};
	return height[_fnt - kFirstFont];
}

/**
 * Returns length of supplied string in pixels
 */
int16 Screen::stringLength(const char *s) const {
	debugC(2, kDebugDisplay, "stringLength(%s)", s);

	byte *const*fontArr = _font[_fnt];
	int16 sum = 0;
	for (; *s; s++)
		sum += *(fontArr[(uint)*s] + 1) + 1;

	return sum;
}

/**
 * Return x which would center supplied string
 */
int16 Screen::center(const char *s) const {
	debugC(1, kDebugDisplay, "center(%s)", s);

	return (int16)((kXPix - stringLength(s)) >> 1);
}

/**
 * Write string at sx,sy in supplied color in current font
 * If sx == CENTER, center it
 */
void Screen::writeStr(int16 sx, const int16 sy, const char *s, const byte color) {
	debugC(2, kDebugDisplay, "writeStr(%d, %d, %s, %d)", sx, sy, s, color);

	if (sx == kCenter)
		sx = center(s);

	byte *const*font = _font[_fnt];
	for (; *s; s++) {
		writeChr(sx, sy, color, (char *)font[(uint)*s]);
		sx += *(font[(uint)*s] + 1) + 1;
	}
}

/**
 * Shadowed version of writestr
 */
void Screen::shadowStr(int16 sx, const int16 sy, const char *s, const byte color) {
	debugC(1, kDebugDisplay, "shadowStr(%d, %d, %s, %d)", sx, sy, s, color);

	if (sx == kCenter)
		sx = center(s);

	writeStr(sx + 1, sy + 1, s, _TBLACK);
	writeStr(sx, sy, s, color);
}

/**
 * Introduce user to the game. In the original games, it was only
 * present in the DOS versions
 */
void Screen::userHelp() const {
	Utils::notifyBox(
	           "F1  - Press F1 again\n"
	           "      for instructions\n"
	           "F2  - Sound on/off\n"
	           "F3  - Recall last line\n"
	           "F4  - Save game\n"
	           "F5  - Restore game\n"
	           "F6  - Inventory\n"
	           "F8  - Turbo button\n"
	           "F9  - Boss button\n\n"
	           "ESC - Return to game");
}

void Screen::drawStatusText() {
	debugC(4, kDebugDisplay, "drawStatusText()");

	loadFont(U_FONT8);
	uint16 sdx = stringLength(_vm->_statusLine);
	uint16 sdy = fontHeight() + 1;                  // + 1 for shadow
	uint16 posX = 0;
	uint16 posY = kYPix - sdy;

	// Display the string and add rect to display list
	writeStr(posX, posY, _vm->_statusLine, _TLIGHTYELLOW);
	displayList(kDisplayAdd, posX, posY, sdx, sdy);

	sdx = stringLength(_vm->_scoreLine);
	posY = 0;

	//Display a black behind the score line
	_vm->_screen->drawRectangle(true, 0, 0, kXPix, 8, _TBLACK);
	writeStr(posX, posY, _vm->_scoreLine, _TCYAN);
	displayList(kDisplayAdd, posX, posY, sdx, sdy);
}

void Screen::drawShape(const int x, const int y, const int color1, const int color2) {
	for (int i = 0; i < kShapeSize; i++) {
		for (int j = 0; j < i; j++) {
			_backBuffer[320 * (y + i) + (x + kShapeSize + j - i)] = color1;
			_frontBuffer[320 * (y + i) + (x + kShapeSize + j - i)] = color1;
			_backBuffer[320 * (y + i) + (x + kShapeSize + j)] = color2;
			_frontBuffer[320 * (y + i) + (x + kShapeSize + j)] = color2;
			_backBuffer[320 * (y + (2 * kShapeSize - 1) - i) + (x + kShapeSize + j - i)] = color1;
			_frontBuffer[320 * (y + (2 * kShapeSize - 1) - i) + (x + kShapeSize + j - i)] = color1;
			_backBuffer[320 * (y + (2 * kShapeSize - 1) - i) + (x + kShapeSize + j)] = color2;
			_frontBuffer[320 * (y + (2 * kShapeSize - 1) - i) + (x + kShapeSize + j)] = color2;
		}
	}
}
/**
 * Display rectangle (filles or empty)
 */
void Screen::drawRectangle(const bool filledFl, const int16 x1, const int16 y1, const int16 x2, const int16 y2, const int color) {
	assert(x1 <= x2);
	assert(y1 <= y2);
	int16 x2Clip = CLIP<int16>(x2, 0, 320);
	int16 y2Clip = CLIP<int16>(y2, 0, 200);

	if (filledFl) {
		for (int i = y1; i < y2Clip; i++) {
			for (int j = x1; j < x2Clip; j++)
				_frontBuffer[320 * i + j] = color;
		}
	} else {
		for (int i = y1; i < y2Clip; i++) {
			_frontBuffer[320 * i + x1] = color;
			_frontBuffer[320 * i + x2] = color;
		}
		for (int i = x1; i < x2Clip; i++) {
			_frontBuffer[320 * y1 + i] = color;
			_frontBuffer[320 * y2 + i] = color;
		}
	}
}

/**
 * Initialize screen components and display results
 */
void Screen::initNewScreenDisplay() {
	displayList(kDisplayInit);
	setBackgroundColor(_TBLACK);
	displayBackground();

	// Stop premature object display in Display_list(D_DISPLAY)
	_vm->getGameStatus().newScreenFl = true;
}

/**
 * Load palette from Hugo.dat
 */
void Screen::loadPalette(Common::ReadStream &in) {
	// Read palette
	_paletteSize = in.readUint16BE();
	_mainPalette = (byte *)malloc(sizeof(byte) * _paletteSize);
	_curPalette = (byte *)malloc(sizeof(byte) * _paletteSize);
	for (int i = 0; i < _paletteSize; i++)
		_curPalette[i] = _mainPalette[i] = in.readByte();
}

/**
 * Free fonts, main and current palettes
 */
void Screen::freeScreen() {
	free(_curPalette);
	free(_mainPalette);

	for (int i = 0; i < kNumFonts; i++) {
		if (_arrayFont[i])
			free(_arrayFont[i]);
	}
}

void Screen::selectInventoryObjId(const int16 objId) {

	_vm->_inventory->setInventoryObjId(objId);      // Select new object

	// Find index of icon
	int16 iconId = _vm->_inventory->findIconId(objId);

	// Compute source coordinates in dib_u
	int16 ux = (iconId + kArrowNumb) * kInvDx % kXPix;
	int16 uy = (iconId + kArrowNumb) * kInvDx / kXPix * kInvDy;

	// Copy the icon and add to display list
	moveImage(getGUIBuffer(), ux, uy, kInvDx, kInvDy, kXPix, _iconImage, 0, 0, 32);

	for (int i = 0; i < stdMouseCursorHeight; i++) {
		for (int j = 0; j < stdMouseCursorWidth; j++) {
			_iconImage[(i * kInvDx) + j] = (stdMouseCursor[(i * stdMouseCursorWidth) + j] == 1) ? _iconImage[(i * kInvDx) + j] : stdMouseCursor[(i * stdMouseCursorWidth) + j];
		}
	}

	CursorMan.replaceCursor(_iconImage, kInvDx, kInvDy, 1, 1, 1);
}

void Screen::resetInventoryObjId() {
	_vm->_inventory->setInventoryObjId(-1);         // Unselect object
	CursorMan.replaceCursor(stdMouseCursor, stdMouseCursorWidth, stdMouseCursorHeight, 1, 1, 1);
}

void Screen::showCursor() {
	CursorMan.showMouse(true);
}

void Screen::hideCursor() {
	CursorMan.showMouse(false);
}

bool Screen::isInX(const int16 x, const rect_t *rect) const {
	return (x >= rect->x) && (x <= rect->x + rect->dx);
}

bool Screen::isInY(const int16 y, const rect_t *rect) const {
	return (y >= rect->y) && (y <= rect->y + rect->dy);
}

/**
 * Check if two rectangles are overlapping
 */
bool Screen::isOverlapping(const rect_t *rectA, const rect_t *rectB) const {
	return (isInX(rectA->x, rectB) || isInX(rectA->x + rectA->dx, rectB) || isInX(rectB->x, rectA) || isInX(rectB->x + rectB->dx, rectA)) &&
		   (isInY(rectA->y, rectB) || isInY(rectA->y + rectA->dy, rectB) || isInY(rectB->y, rectA) || isInY(rectB->y + rectB->dy, rectA));
}

/**
 * Display active boundaries (activated in the console)
 * Light Red   = Exit hotspots
 * Light Green = Visible objects
 * White       = Fix objects, parts of background
 */
void Screen::drawBoundaries() {
	if (!_vm->getGameStatus().showBoundariesFl)
		return;

	_vm->_mouse->drawHotspots();

	for (int i = 0; i < _vm->_object->_numObj; i++) {
		object_t *obj = &_vm->_object->_objects[i]; // Get pointer to object
		if (obj->screenIndex == *_vm->_screen_p) {
			if ((obj->currImagePtr != 0) && (obj->cycling != kCycleInvisible))
				drawRectangle(false, obj->x + obj->currImagePtr->x1, obj->y + obj->currImagePtr->y1,
				                     obj->x + obj->currImagePtr->x2, obj->y + obj->currImagePtr->y2, _TLIGHTGREEN);
			else if ((obj->currImagePtr == 0) && (obj->vxPath != 0) && !obj->carriedFl)
				drawRectangle(false, obj->oldx, obj->oldy, obj->oldx + obj->vxPath, obj->oldy + obj->vyPath, _TBRIGHTWHITE);
		}
	}
	g_system->copyRectToScreen(_frontBuffer, 320, 0, 0, 320, 200);
}

Screen_v1d::Screen_v1d(HugoEngine *vm) : Screen(vm) {
}

Screen_v1d::~Screen_v1d() {
}

/**
 * Load font file, construct font ptrs and reverse data bytes
 * TODO: This uses hardcoded fonts in hugo.dat, it should be replaced
 *       by a proper implementation of .FON files
 */
void Screen_v1d::loadFont(const int16 fontId) {
	debugC(2, kDebugDisplay, "loadFont(%d)", fontId);

	assert(fontId < kNumFonts);

	_fnt = fontId - kFirstFont;                     // Set current font number

	if (fontLoadedFl[_fnt])                         // If already loaded, return
		return;

	fontLoadedFl[_fnt] = true;

	memcpy(_fontdata[_fnt], _arrayFont[_fnt], _arrayFontSize[_fnt]);
	_font[_fnt][0] = _fontdata[_fnt];               // Store height,width of fonts

	int16 offset = 2;                               // Start at fontdata[2] ([0],[1] used for height,width)

	// Setup the font array (127 characters)
	for (int i = 1; i < 128; i++) {
		_font[_fnt][i] = _fontdata[_fnt] + offset;
		byte height = *(_fontdata[_fnt] + offset);
		byte width  = *(_fontdata[_fnt] + offset + 1);

		int16 size = height * ((width + 7) >> 3);
		for (int j = 0; j < size; j++)
			Utils::reverseByte(&_fontdata[_fnt][offset + 2 + j]);

		offset += 2 + size;
	}
}

/**
 * Load fonts from Hugo.dat
 * These fonts are a workaround to avoid handling TTF fonts used by DOS versions
 * TODO: Get rid of this function when the win1 fonts are supported
 */
void Screen_v1d::loadFontArr(Common::ReadStream &in) {
	for (int i = 0; i < kNumFonts; i++) {
		_arrayFontSize[i] = in.readUint16BE();
		_arrayFont[i] = (byte *)malloc(sizeof(byte) * _arrayFontSize[i]);
		for (int j = 0; j < _arrayFontSize[i]; j++) {
			_arrayFont[i][j] = in.readByte();
		}
	}
}

/**
 * Return the overlay state (Foreground/Background) of the currently
 * processed object by looking down the current column for an overlay
 * base byte set (in which case the object is foreground).
 */
overlayState_t Screen_v1d::findOvl(seq_t *seq_p, image_pt dst_p, uint16 y) {
	debugC(4, kDebugDisplay, "findOvl()");

	uint16 index = (uint16)(dst_p - _frontBuffer) >> 3;

	for (int i = 0; i < seq_p->lines-y; i++) {      // Each line in object
		if (_vm->_object->getBaseBoundary(index))   // If any overlay base byte is non-zero then the object is foreground, else back.
			return kOvlForeground;
		index += kCompLineSize;
	}

	return kOvlBackground;                          // No bits set, must be background
}

Screen_v1w::Screen_v1w(HugoEngine *vm) : Screen(vm) {
}

Screen_v1w::~Screen_v1w() {
}

/**
 * Load font file, construct font ptrs and reverse data bytes
 */
void Screen_v1w::loadFont(const int16 fontId) {
	debugC(2, kDebugDisplay, "loadFont(%d)", fontId);

	_fnt = fontId - kFirstFont;                     // Set current font number

	if (fontLoadedFl[_fnt])                         // If already loaded, return
		return;

	fontLoadedFl[_fnt] = true;
	_vm->_file->readUIFItem(fontId, _fontdata[_fnt]);

	// Compile font ptrs.  Note: First ptr points to height,width of font
	_font[_fnt][0] = _fontdata[_fnt];               // Store height,width of fonts

	int16 offset = 2;                               // Start at fontdata[2] ([0],[1] used for height,width)

	// Setup the font array (127 characters)
	for (int i = 1; i < 128; i++) {
		_font[_fnt][i] = _fontdata[_fnt] + offset;
		byte height = *(_fontdata[_fnt] + offset);
		byte width  = *(_fontdata[_fnt] + offset + 1);

		int16 size = height * ((width + 7) >> 3);
		for (int j = 0; j < size; j++)
			Utils::reverseByte(&_fontdata[_fnt][offset + 2 + j]);

		offset += 2 + size;
	}
}

/**
 * Skips the fonts used by the DOS versions
 */
void Screen_v1w::loadFontArr(Common::ReadStream &in) {
	for (int i = 0; i < kNumFonts; i++) {
		uint16 numElem = in.readUint16BE();
		for (int j = 0; j < numElem; j++)
			in.readByte();
	}
}

/**
 * Return the overlay state (Foreground/Background) of the currently
 * processed object by looking down the current column for an overlay
 * base bit set (in which case the object is foreground).
 */
overlayState_t Screen_v1w::findOvl(seq_t *seq_p, image_pt dst_p, uint16 y) {
	debugC(4, kDebugDisplay, "findOvl()");

	for (; y < seq_p->lines; y++) {                 // Each line in object
		byte ovb = _vm->_object->getBaseBoundary((uint16)(dst_p - _frontBuffer) >> 3); // Ptr into overlay bits
		if (ovb & (0x80 >> ((uint16)(dst_p - _frontBuffer) & 7))) // Overlay bit is set
			return kOvlForeground;                  // Found a bit - must be foreground
		dst_p += kXPix;
	}

	return kOvlBackground;                          // No bits set, must be background
}

} // End of namespace Hugo

