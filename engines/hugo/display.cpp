/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
#include "common/events.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "graphics/cursorman.h"
#include "graphics/paletteman.h"

#include "hugo/hugo.h"
#include "hugo/display.h"
#include "hugo/file.h"
#include "hugo/inventory.h"
#include "hugo/util.h"
#include "hugo/object.h"
#include "hugo/parser.h"
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

#ifdef USE_TTS
static const uint8 kHelpFirstNewlineIndex = 20;
#endif


Screen::Screen(HugoEngine *vm) : _vm(vm) {
	_mainPalette = nullptr;
	_curPalette = nullptr;
	_dlAddIndex = 0;
	_dlRestoreIndex = 0;

	for (int i = 0; i < kNumFonts; i++) {
		_arrayFont[i] = nullptr;
		fontLoadedFl[i] = false;
	}
	for (int i = 0; i < kBlitListSize; i++) {
		_dlBlistList[i]._x = 0;
		_dlBlistList[i]._y = 0;
		_dlBlistList[i]._dx = 0;
		_dlBlistList[i]._dy = 0;
	}
	for (int i = 0; i < kRectListSize; i++) {
		_dlAddList[i]._x = 0;
		_dlAddList[i]._y = 0;
		_dlAddList[i]._dx = 0;
		_dlAddList[i]._dy = 0;
		_dlRestoreList[i]._x = 0;
		_dlRestoreList[i]._y = 0;
		_dlRestoreList[i]._dx = 0;
		_dlRestoreList[i]._dy = 0;
	}
	_fnt = 0;
	_paletteSize = 0;

	_frontSurface.init(320, 200, 320, _frontBuffer, Graphics::PixelFormat::createFormatCLUT8());
}

Screen::~Screen() {
}

Icondib &Screen::getIconBuffer() {
	return _iconBuffer;
}

Viewdib &Screen::getBackBuffer() {
	return _backBuffer;
}

Viewdib &Screen::getBackBufferBackup() {
	return _backBufferBackup;
}

Viewdib &Screen::getFrontBuffer() {
	return _frontBuffer;
}

Viewdib &Screen::getGUIBuffer() {
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
void Screen::moveImage(ImagePtr srcImage, const int16 x1, const int16 y1, const int16 dx, int16 dy, const int16 width1, ImagePtr dstImage, const int16 x2, const int16 y2, const int16 width2) {
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
	g_system->copyRectToScreen(&_frontBuffer[xClip + yClip * 320], 320, xClip, yClip, CLIP<int16>(dx, 0, 320 - xClip), CLIP<int16>(dy, 0, 200 - yClip));
}

/**
 * Change a color by remapping supplied palette index with new index in main palette.
 * Also save the new color in the current palette.
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
 * Merge rectangles A,B leaving result in B
 */
void Screen::merge(const Rect *rectA, Rect *rectB) {
	debugC(6, kDebugDisplay, "merge()");

	int16 xa = rectA->_x + rectA->_dx;               // Find x2,y2 for each rectangle
	int16 xb = rectB->_x + rectB->_dx;
	int16 ya = rectA->_y + rectA->_dy;
	int16 yb = rectB->_y + rectB->_dy;

	rectB->_x = MIN(rectA->_x, rectB->_x);           // Minimum x,y
	rectB->_y = MIN(rectA->_y, rectB->_y);
	rectB->_dx = MAX(xa, xb) - rectB->_x;            // Maximum dx,dy
	rectB->_dy = MAX(ya, yb) - rectB->_y;
}

/**
 * Coalesce the rectangles in the restore/add list into one unified
 * blist.  len is the sizes of alist or rlist.  blen is current length
 * of blist.  bmax is the max size of the blist.  Note that blist can
 * have holes, in which case dx = 0.  Returns used length of blist.
 */
int16 Screen::mergeLists(Rect *list, Rect *blist, const int16 len, int16 blen) {
	debugC(4, kDebugDisplay, "mergeLists()");

	int16   coalesce[kBlitListSize];                // List of overlapping rects
	// Process the list
	for (int16 a = 0; a < len; a++, list++) {
		// Compile list of overlapping rectangles in blit list
		int16 c = 0;
		Rect *bp = blist;
		for (int16 b = 0; b < blen; b++, bp++) {
			if (bp->_dx)                            // blist entry used
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
				Rect *cp = &blist[coalesce[c]];
				merge(cp, bp);
				cp->_dx = 0;                         // Delete entry
			}
		}
	}
	return blen;
}

/**
 * Process the display list
 * Trailing args are int16 x,y,dx,dy for the D_ADD operation
 */
void Screen::displayList(int update, ...) {
	debugC(6, kDebugDisplay, "displayList()");

	int16         blitLength = 0;                   // Length of blit list
	va_list       marker;                           // Args used for D_ADD operation
	Rect       *p;                                // Ptr to dlist entry

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
		p->_x  = va_arg(marker, int);               // x
		p->_y  = va_arg(marker, int);               // y
		p->_dx = va_arg(marker, int);               // dx
		p->_dy = va_arg(marker, int);               // dy
		va_end(marker);                             // Reset variable arguments
		_dlAddIndex++;
		break;
	case kDisplayDisplay:                           // Display whole list
		// Don't blit if newscreen just loaded because _frontBuffer will
		// get blitted via InvalidateRect() at end of this cycle
		// and blitting here causes objects to appear too soon.
		if (_vm->getGameStatus()._newScreenFl) {
			_vm->getGameStatus()._newScreenFl = false;
			break;
		}

		// Coalesce restore-list, add-list into combined blit-list
		blitLength = mergeLists(_dlRestoreList, _dlBlistList, _dlRestoreIndex, blitLength);
		blitLength = mergeLists(_dlAddList, _dlBlistList, _dlAddIndex, blitLength);

		// Blit the combined blit-list
		for (_dlRestoreIndex = 0, p = _dlBlistList; _dlRestoreIndex < blitLength; _dlRestoreIndex++, p++) {
			if (p->_dx)                              // Marks a used entry
				displayRect(p->_x, p->_y, p->_dx, p->_dy);
		}
		break;
	case kDisplayRestore:                           // Restore each rectangle
		for (_dlRestoreIndex = 0, p = _dlAddList; _dlRestoreIndex < _dlAddIndex; _dlRestoreIndex++, p++) {
			// Restoring from _backBuffer to _frontBuffer
			_dlRestoreList[_dlRestoreIndex] = *p;   // Copy add-list to restore-list
			moveImage(_backBuffer, p->_x, p->_y, p->_dx, p->_dy, kXPix, _frontBuffer, p->_x, p->_y, kXPix);
		}
		_dlAddIndex = 0;                            // Reset add-list
		break;
	default:
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
	Common::String message = "F1  - Press F1 again\n"
	                         "      for instructions\n"
	                         "F2  - Sound on/off\n"
	                         "F3  - Recall last line\n"
	                         "F4  - Save game\n"
	                         "F5  - Restore game\n"
	                         "F6  - Inventory\n"
	                         "F8  - Turbo button\n"
	                         "\n"
	                         "ESC - Return to game";

#ifdef USE_TTS
	// Replace the first newline with a space for smoother voicing
	message[kHelpFirstNewlineIndex] = ' ';
	Utils::sayText(message, Common::TextToSpeechManager::INTERRUPT, false);
	message[kHelpFirstNewlineIndex] = '\n';
#endif

	Common::KeyCode keyCode = _vm->notifyBox(message, false, kTtsNoSpeech);
	
	// DOS: If the help message was dismissed with F1 then show instructions
	if (keyCode == Common::KEYCODE_F1) {
		_vm->_file->instructions();
	}
}

void Screen::updateStatusText() {
	// Format status line
	Common::sprintf_s(_vm->_statusLine, "F1-Help  %s  Score: %3d of %3d  Sound %3s",
		(_vm->_config._turboFl) ? "T" : " ", _vm->getScore(), _vm->getMaxScore(),
		(_vm->_config._soundFl) ? "on" : "off");
}

void Screen::updatePromptText(const char *command, char cursor) {
	// Format prompt line, pad with spaces to fill row
	Common::sprintf_s(_vm->_promptLine, ">%s%c", command, cursor);
	for (int i = strlen(_vm->_promptLine); i < kMaxTextCols; i++) {
		_vm->_promptLine[i] = ' ';
	}
	_vm->_promptLine[kMaxTextCols] = '\0';
}

/**
 * Draw status line at top row and add to blit list
 */
void Screen::drawStatusText() {
	debugC(4, kDebugDisplay, "drawStatusText()");

	Common::Rect r = drawDosText(0, 0, _vm->_statusLine, _TCYAN);
	displayList(kDisplayAdd, r.left, r.top, r.width(), r.height());
}

/**
 * Draw status line at top row and display on the screen
 */
void Screen::displayStatusText() {
	debugC(4, kDebugDisplay, "displayStatusText()");

	Common::Rect r = drawDosText(0, 0, _vm->_statusLine, _TCYAN);
	_vm->_system->copyRectToScreen(_frontBuffer, 320, r.left, r.top, r.width(), r.height());
}

/**
 * Draw prompt line at bottom row and add to blit list
 */
void Screen::drawPromptText() {
	debugC(4, kDebugDisplay, "drawPromptText()");

	Common::Rect r = drawDosText(0, kMaxTextRows - 1, _vm->_promptLine, _TLIGHTYELLOW);
	displayList(kDisplayAdd, r.left, r.top, r.width(), r.height());
}

/**
 * Draw prompt line at bottom row and display on the screen
 */
void Screen::displayPromptText() {
	debugC(4, kDebugDisplay, "displayPromptText()");

	Common::Rect r = drawDosText(0, kMaxTextRows - 1, _vm->_promptLine, _TLIGHTYELLOW);
	_vm->_system->copyRectToScreen(&_frontBuffer[r.top * 320], 320, r.left, r.top, r.width(), r.height());
}

/**
 * Display diamond in Hugo1 DOS introduction
 *
 * x,y: upper left of diamond.
 * color1: left color.
 * color2: right color.
 */
void Screen::drawShape(const int x, const int y, const int color1, const int color2) {
	for (int i = 0; i < kShapeSize; i++) {
		const int top = y + i;
		const int bottom = y + (kShapeSize * 2) - 2 - i;
		for (int j = 0; j <= i; j++) {
			const int left  = x + kShapeSize - 1 - j;
			const int right = x + kShapeSize + j;
			_frontBuffer[320 * top + left] = color1;
			_frontBuffer[320 * top + right] = color2;
			_frontBuffer[320 * bottom + left] = color1;
			_frontBuffer[320 * bottom + right] = color2;
		}
	}
}

/**
 * Display rectangle (filled or empty)
 *
 * x1,y1: upper left of rectangle.
 * x2,y2: lower right of rectangle.
 *
 * This is used by DOS code as a replacement for _rectangle() from QuickC
 */
void Screen::drawRectangle(const bool filledFl, const int16 x1, const int16 y1, const int16 x2, const int16 y2, const int color) {
	assert(x1 <= x2);
	assert(y1 <= y2);
	int16 x2Clip = CLIP<int16>(x2, 0, 319);
	int16 y2Clip = CLIP<int16>(y2, 0, 199);

	if (filledFl) {
		for (int i = y1; i <= y2Clip; i++) {
			for (int j = x1; j <= x2Clip; j++)
				_frontBuffer[320 * i + j] = color;
		}
	} else {
		for (int i = y1; i <= y2Clip; i++) {
			_frontBuffer[320 * i + x1] = color;
			_frontBuffer[320 * i + x2Clip] = color;
		}
		for (int i = x1; i < x2Clip; i++) {
			_frontBuffer[320 * y1 + i] = color;
			_frontBuffer[320 * y2Clip + i] = color;
		}
	}
}

/**
 * Draws text to the screen using DOS font with a black background
 *
 * Coordinates are in text: x = 0-39, y = 0-24.
 * Returns screen rectangle of drawn text so that it can be invalidated.
 */
Common::Rect Screen::drawDosText(byte x, byte y, const char *text, byte color) {
	// Calculate text length
	assert(x < kMaxTextCols);
	assert(y < kMaxTextRows);
	int textLength = strlen(text);
	if (x + textLength > kMaxTextCols) {
		textLength = kMaxTextCols - x;
	}

	// Draw black background
	int sx = x * 8;
	int sy = y * 8;
	Common::Rect rect(sx, sy, sx + (textLength * 8), sy + 8);
	_frontSurface.fillRect(rect, _TBLACK);

	// Draw text
	for (int i = 0; i < textLength; i++) {
		_dosFont.drawChar(&_frontSurface, (byte)text[i], sx, sy, color);
		sx += 8;
	}
	return rect;
}

/**
 * Returns the DOS message box border color for the current game
 */
byte Screen::getDosMessageBoxBorder() const {
	switch (_vm->getGameType()) {
	case kGameTypeHugo1: return _TLIGHTRED;
	case kGameTypeHugo2: return _TBLUE;
	default:             return _TGREEN;
	}
}

/**
 * Display DOS message box
 *
 * Returns the KeyState of the keydown event that dismissed the message box.
 * Callers can use this to implement original behavior such as applying that
 * key to an input prompt, such as dosPromptBox(), or detecting an F1 press
 * on the help screen so that the instructions 
 */
Common::KeyState Screen::dosMessageBox(const Common::String &text, bool protect, TtsOptions ttsOptions) {
	if (text.empty()) {
		return Common::KeyState();
	}

	// Handle TTS the same as Utils::notifyBox()
#ifdef USE_TTS
	if (ttsOptions & kTtsSpeech) {
		bool replaceNewlines = ((ttsOptions & kTtsReplaceNewlines) == kTtsReplaceNewlines);
		Utils::sayText(text, Common::TextToSpeechManager::QUEUE, replaceNewlines);
	}
#endif

	// Draw the status bar, as the score may have just been changed.
	// The DOS original updated the status text on screen immediately, but we
	// add it to the list of rectangles to eventually blit to the screen,
	// similar to how the Windows version draws. In order to replicate the
	// DOS behavior of displaying the new score with the message, we draw
	// the status bar directly to the screen when showing a message box.
	if (_vm->getGameStatus()._viewState == kViewPlay) {
		displayStatusText();
	}

	// Compute size of formatted text for box size
	int16 width = 0;
	int16 x = 0;
	int16 y = 1;
	for (uint i = 0; i < text.size(); i++) {
		if (text[i] != '\n') {
			x++;
		} else {
			y++;
			if (x > width)
				width = x;
			x = 0;
		}
	}

	// Get width, height of text
	int16 height = y;
	if (x > width)
		width = x;
	width += 2; // Border text by +- one char
	height += 2;
	if (width > kMaxTextCols || height > kMaxTextRows) {
		warning("text too long: %s", text.c_str());
		return Common::KeyState();
	}

	// Get x,y text start coords
	int16 xOffset = (_vm->getGameType() == kGameTypeHugo1) ? 0 : 1;
	x = (kMaxTextCols - width + xOffset) / 2;
	y = (kMaxTextRows - height) / 2;
	// Adjust one based coordinates from original code to zero based
	x--;
	y--;
	x = MAX<int>(x, 0);
	y = MAX<int>(y, 0);
	// Get screen start coordinates and dimensions
	int16 sx1 = x * 8;
	int16 sy1 = y * 8;
	int16 sx2 = (x + width) * 8;
	int16 sy2 = (y + height) * 8;
	// If box reaches the right edge of screen, reduce by one pixel
	// for the right border. The original games did not have messages
	// this wide, but we display a large advertisement when exiting.
	if (sx2 >= 320) {
		sx2 = kXPix - 1;
	}

	// Save screen
	int16 boxWidth = sx2 - sx1 + 1;
	int16 boxHeight = sy2 - sy1 + 1;
	if (sx1 + boxWidth > 320) {
		boxWidth = 320 - sx1;
	}
	if (sy1 + boxHeight > 200) {
		boxHeight = 200 - sy1;
	}
	moveImage(_frontBuffer, sx1, sy1, boxWidth, boxHeight, 320, _frontBufferBoxBackup, sx1, sy1, 320);

	// Draw box
	drawRectangle(true,  sx1, sy1, sx2, sy2, _TBLACK);
	drawRectangle(false, sx1, sy1, sx2, sy2, getDosMessageBoxBorder());
	char c[] = "X"; // A dummy string with 2nd byte null
	int16 xTextPos = x + 1;
	int16 yTextPos = y + 1;
	for (uint i = 0; i < text.size(); i++) {
		if (text[i] == '\n') {
			xTextPos = x + 1;
			yTextPos++;
		} else {
			c[0] = text[i];
			drawDosText(xTextPos, yTextPos, c, _TLIGHTCYAN);
			xTextPos++;
		}
	}
	_vm->_system->copyRectToScreen(&_frontBuffer[320 * sy1 + sx1], 320, sx1, sy1, boxWidth, boxHeight);

	// Wait for key
	const uint32 startTime = _vm->_system->getMillis();
	Common::KeyState keyState = getKey();

	// Protect mode: there is a short duration in which keys are ignored
	// unless they are Enter or Escape. This is used with messages that
	// occur on timers so that the user doesn't accidentally dismiss the
	// message while in the middle of typing a command.
	if (protect) {
		const uint32 delay = 3 * (1000 / _vm->getTPS());
		while (keyState.keycode != Common::KEYCODE_RETURN &&
			keyState.keycode != Common::KEYCODE_ESCAPE &&
			_vm->_system->getMillis() - startTime < delay) {
			keyState = getKey();
		}
	}

	// Restore screen
	moveImage(_frontBufferBoxBackup, sx1, sy1, boxWidth, boxHeight, 320, _frontBuffer, sx1, sy1, 320);
	_vm->_system->copyRectToScreen(&_frontBuffer[320 * sy1 + sx1], 320, sx1, sy1, boxWidth, boxHeight);

	// Handle TTS the same as Utils::notifyBox()
#ifdef USE_TTS
	Utils::stopTextToSpeech();
#endif

	return keyState;
}

/**
 * Display DOS message box and prompt for a response
 *
 * Returns the text entered by the user.
 */
Common::String Screen::dosPromptBox(const Common::String &text) {
	if (text.empty()) {
		return Common::String();
	}

	// Show the message box until it is dismissed with a printable key
	Common::KeyState keyState;
	while (!Common::isPrint(keyState.ascii)) {
		keyState = dosMessageBox(text);
		if (_vm->shouldQuit()) {
			return Common::String();
		}
	}

	// Initialize command with the key used to dismiss the message box
	char command[kMaxLineSize + 1];
	int index = 0;
	command[index++] = keyState.ascii;
	command[index] = '\0';
	updatePromptText(command, ' ');
	displayPromptText();

	// Keyboard input loop
	while (!_vm->shouldQuit()) {
		keyState = getKey();

		if (keyState.keycode == Common::KEYCODE_RETURN) {
			break;
		} else if (keyState.keycode == Common::KEYCODE_BACKSPACE) {
			if (index > 0) {
				command[--index] = '\0';
				updatePromptText(command, ' ');
				displayPromptText();
			}
		} else if (Common::isPrint(keyState.ascii)) {
			if (index < ARRAYSIZE(command) - 1) {
				command[index++] = keyState.ascii;
				command[index] = '\0';
				updatePromptText(command, ' ');
				displayPromptText();
			}
		}
	}

	// Clear the prompt text
	Common::String response = command;
	command[0] = '\0';
	updatePromptText(command, ' ');
	displayPromptText();
	_vm->_parser->resetCommandLine();

	// Handle TTS the same as Utils::promptBox()
#ifdef USE_TTS
	Utils::sayText(response);
#endif

	return response;
}

/**
 * Get key from keyboard
 *
 * This function roughly mimics getch(). It returns the keystate of the first
 * keydown message for a printable key or one of the specific keys that callers
 * expect: enter, backspace, escape, and F1.
 */
Common::KeyState Screen::getKey() {
	while (!_vm->shouldQuit()) {
		Common::Event event;
		while (_vm->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				// Ignore keydown if modifier other than shift is pressed
				if (event.kbd.flags & (Common::KBD_NON_STICKY & ~Common::KBD_SHIFT)) {
					continue;
				}
				// Enter, backspace, and printable keys are acceptable
				if (event.kbd.keycode == Common::KEYCODE_RETURN ||
					event.kbd.keycode == Common::KEYCODE_BACKSPACE ||
					Common::isPrint(event.kbd.ascii)) {
					return event.kbd;
				}
				break;

			// Several keys that we wish to return have been turned into
			// actions for the keymapper. Translate them back into their
			// original keycodes for the caller to test like any other key.
			case Common::EVENT_CUSTOM_ENGINE_ACTION_START:
				switch (event.customType) {
				case kActionEscape:
					return Common::KeyState(Common::KEYCODE_ESCAPE);
				case kActionUserHelp:
					return Common::KeyState(Common::KEYCODE_F1);
				default:
					break;
				}
				break;

			default:
				break;
			}
		}
		_vm->_system->updateScreen();
		_vm->_system->delayMillis(10);
	}
	return Common::KeyState();
}

/**
 * Initialize screen components and display results
 */
void Screen::initNewScreenDisplay() {
	displayList(kDisplayInit);
	setBackgroundColor(_TBLACK);
	displayBackground();

	// Stop premature object display in Display_list(D_DISPLAY)
	_vm->getGameStatus()._newScreenFl = true;
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

bool Screen::isInX(const int16 x, const Rect *rect) const {
	return (x >= rect->_x) && (x <= rect->_x + rect->_dx);
}

bool Screen::isInY(const int16 y, const Rect *rect) const {
	return (y >= rect->_y) && (y <= rect->_y + rect->_dy);
}

/**
 * Check if two rectangles are overlapping
 */
bool Screen::isOverlapping(const Rect *rectA, const Rect *rectB) const {
	return (isInX(rectA->_x, rectB) || isInX(rectA->_x + rectA->_dx, rectB) || isInX(rectB->_x, rectA) || isInX(rectB->_x + rectB->_dx, rectA)) &&
		   (isInY(rectA->_y, rectB) || isInY(rectA->_y + rectA->_dy, rectB) || isInY(rectB->_y, rectA) || isInY(rectB->_y + rectB->_dy, rectA));
}

/**
 * Display active boundaries (activated in the console)
 * Light Red   = Exit hotspots
 * Light Green = Visible objects
 * White       = Fix objects, parts of background
 */
void Screen::drawBoundaries() {
	if (!_vm->getGameStatus()._showBoundariesFl)
		return;

	_vm->_mouse->drawHotspots();

	for (int i = 0; i < _vm->_object->_numObj; i++) {
		Object *obj = &_vm->_object->_objects[i]; // Get pointer to object
		if (obj->_screenIndex == *_vm->_screenPtr) {
			if ((obj->_currImagePtr != nullptr) && (obj->_cycling != kCycleInvisible))
				drawRectangle(false, obj->_x + obj->_currImagePtr->_x1, obj->_y + obj->_currImagePtr->_y1,
				                     obj->_x + obj->_currImagePtr->_x2, obj->_y + obj->_currImagePtr->_y2, _TLIGHTGREEN);
			else if ((obj->_currImagePtr == nullptr) && (obj->_vxPath != 0) && !obj->_carriedFl)
				drawRectangle(false, obj->_oldx, obj->_oldy, obj->_oldx + obj->_vxPath, obj->_oldy + obj->_vyPath, _TBRIGHTWHITE);
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
 * Merge an object frame into _frontBuffer at sx, sy and update rectangle list.
 * If fore TRUE, force object above any overlay
 *
 * Note: The DOS version has subtlety different overlay logic than the Windows version.
 * The overlay base comparison occurs in a different order, and this alters the outcome.
 * This may have been unintentional when the original DOS code was rewritten from assembly
 * in C for Windows, as the Windows version introduces a priority bug when standing in
 * front of the bed in the second room of Hugo2.
 */
void Screen_v1d::displayFrame(const int sx, const int sy, Seq *seq, const bool foreFl) {
	debugC(3, kDebugDisplay, "displayFrame(%d, %d, seq, %d)", sx, sy, (foreFl) ? 1 : 0);

	ImagePtr image = seq->_imagePtr;                 // Ptr to object image data
	ImagePtr subFrontBuffer = &_frontBuffer[sy * kXPix + sx]; // Ptr to offset in _frontBuffer
	int16 frontBufferwrap = kXPix - seq->_x2 - 1;     // Wraps dest_p after each line
	int16 imageWrap = seq->_bytesPerLine8 - seq->_x2 - 1;
	OverlayState overlayState = (foreFl) ? kOvlForeground : kOvlUndef; // Overlay state of object
	for (uint16 y = 0; y < seq->_lines; y++) {       // Each line in object
		for (uint16 x = 0; x <= seq->_x2; x++) {
			byte ovlBound = _vm->_object->getFirstOverlay((uint16)(subFrontBuffer - _frontBuffer) >> 3); // Ptr into overlay bits
			if (ovlBound != 0) {
				if (overlayState == kOvlUndef)  // Overlay defined yet?
					overlayState = findOvl(seq, subFrontBuffer, y);// No, find it.
			}

			if (*image) {                           // Non-transparent
				if (ovlBound & (0x80 >> ((uint16)(subFrontBuffer - _frontBuffer) & 7))) { // Overlay bit is set
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
	displayList(kDisplayAdd, sx, sy, seq->_x2 + 1, seq->_lines);
}

/**
 * Return the overlay state (Foreground/Background) of the currently
 * processed object by looking down the current column for an overlay
 * base byte set (in which case the object is foreground).
 *
 * Note: This DOS function was originally in assembly. We have structured
 * it to resemble the Windows C code, but the original DOS assembly takes
 * "y" as the number of lines left to scan, not the number of lines that
 * have been scanned, so we invert it here. For example, if an object is
 * 45 lines and and processFrame() locates an overlay byte on the second
 * line, it will pass 1 for y, so we must scan 44 rows for a base byte
 * starting from dstPtr (the second row from the top).
 */
OverlayState Screen_v1d::findOvl(Seq *seqPtr, ImagePtr dstPtr, uint16 y) {
	debugC(4, kDebugDisplay, "findOvl()");

	uint16 index = (uint16)(dstPtr - _frontBuffer) >> 3;

	int linesToScan = seqPtr->_lines - y;
	for (int i = 0; i < linesToScan; i++) {         // Each line in object
		if (_vm->_object->getBaseBoundary(index))   // If any overlay base byte is non-zero then the object is foreground, else back.
			return kOvlForeground;
		index += kCompLineSize;
	}

	return kOvlBackground;                          // No bits set, must be background
}

/**
 * Load default EGA palette. Skip Windows palette from Hugo.dat
 */
void Screen_v1d::loadPalette(Common::SeekableReadStream &in) {
	// Skip Windows palette
	uint16 winPaletteSize = in.readUint16BE();
	in.skip(winPaletteSize);

	// Load default EGA palette
	Graphics::Palette egaPalette = Graphics::Palette::createEGAPalette();
	_paletteSize = egaPalette.size() * 3;
	_mainPalette = (byte *)malloc(_paletteSize);
	_curPalette = (byte *)malloc(_paletteSize);
	memcpy(_mainPalette, egaPalette.data(), _paletteSize);
	memcpy(_curPalette, egaPalette.data(), _paletteSize);
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
 * Merge an object frame into _frontBuffer at sx, sy and update rectangle list.
 * If fore TRUE, force object above any overlay
 */
void Screen_v1w::displayFrame(const int sx, const int sy, Seq *seq, const bool foreFl) {
	debugC(3, kDebugDisplay, "displayFrame(%d, %d, seq, %d)", sx, sy, (foreFl) ? 1 : 0);

	ImagePtr image = seq->_imagePtr;                 // Ptr to object image data
	ImagePtr subFrontBuffer = &_frontBuffer[sy * kXPix + sx]; // Ptr to offset in _frontBuffer
	int16 frontBufferwrap = kXPix - seq->_x2 - 1;     // Wraps dest_p after each line
	int16 imageWrap = seq->_bytesPerLine8 - seq->_x2 - 1;
	OverlayState overlayState = (foreFl) ? kOvlForeground : kOvlUndef; // Overlay state of object
	for (uint16 y = 0; y < seq->_lines; y++) {       // Each line in object
		for (uint16 x = 0; x <= seq->_x2; x++) {
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
	displayList(kDisplayAdd, sx, sy, seq->_x2 + 1, seq->_lines);
}

/**
 * Return the overlay state (Foreground/Background) of the currently
 * processed object by looking down the current column for an overlay
 * base bit set (in which case the object is foreground).
 */
OverlayState Screen_v1w::findOvl(Seq *seqPtr, ImagePtr dstPtr, uint16 y) {
	debugC(4, kDebugDisplay, "findOvl()");

	for (; y < seqPtr->_lines; y++) {                 // Each line in object
		byte ovb = _vm->_object->getBaseBoundary((uint16)(dstPtr - _frontBuffer) >> 3); // Ptr into overlay bits
		if (ovb & (0x80 >> ((uint16)(dstPtr - _frontBuffer) & 7))) // Overlay bit is set
			return kOvlForeground;                  // Found a bit - must be foreground
		dstPtr += kXPix;
	}

	return kOvlBackground;                          // No bits set, must be background
}

/**
 * Load Windows palette from Hugo.dat
 */
void Screen_v1w::loadPalette(Common::SeekableReadStream &in) {
	// Read palette
	_paletteSize = in.readUint16BE();
	_mainPalette = (byte *)malloc(sizeof(byte) * _paletteSize);
	_curPalette = (byte *)malloc(sizeof(byte) * _paletteSize);
	for (int i = 0; i < _paletteSize; i++)
		_curPalette[i] = _mainPalette[i] = in.readByte();
}

} // End of namespace Hugo
