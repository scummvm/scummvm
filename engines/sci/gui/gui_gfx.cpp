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
 * $URL$
 * $Id$
 *
 */

#include "common/util.h"
#include "graphics/primitives.h"

#include "sci/sci.h"
#include "sci/engine/state.h"
#include "sci/tools.h"
#include "sci/gui/gui_font.h"
#include "sci/gui/gui_picture.h"
#include "sci/gui/gui_view.h"
#include "sci/gui/gui_screen.h"
#include "sci/gui/gui_palette.h"
#include "sci/gui/gui_gfx.h"

namespace Sci {

SciGuiGfx::SciGuiGfx(EngineState *state, SciGuiScreen *screen, SciGuiPalette *palette)
	: _s(state), _screen(screen), _palette(palette) {
	init();
}

SciGuiGfx::~SciGuiGfx() {
	delete _mainPort;
	delete _menuPort;
}

void SciGuiGfx::init() {
	_font = NULL;
	_textFonts = NULL; _textFontsCount = 0;
	_textColors = NULL; _textColorsCount = 0;

	// _mainPort is not known to windowmanager, that's okay according to sierra sci
	//  its not even used currently in our engine
	_mainPort = new GuiPort(0);
	SetPort(_mainPort);
	OpenPort(_mainPort);

	// _menuPort has actually hardcoded id 0xFFFF. Its not meant to be known to windowmanager according to sierra sci
	_menuPort = new GuiPort(0);
	OpenPort(_menuPort);
	SetFont(0);
	_menuPort->rect = Common::Rect(0, 0, _screen->_width, _screen->_height);
	_menuRect = Common::Rect(0, 0, _screen->_width, 9);

	// Initialize priority bands
	if (_s->usesOldGfxFunctions()) {
		_priorityBandCount = 15;
		PriorityBandsInit(42, 200);
	} else {
		_priorityBandCount = 14;
		PriorityBandsInit(42, 190);
	}
}

GuiPort *SciGuiGfx::SetPort(GuiPort *newPort) {
	GuiPort *oldPort = _curPort;
	_curPort = newPort;
	return oldPort;
}

GuiPort *SciGuiGfx::GetPort(void) {
	return _curPort;
}

void SciGuiGfx::SetOrigin(int16 left, int16 top) {
	_curPort->left = left;
	_curPort->top = top;
}

void SciGuiGfx::MoveTo(int16 left, int16 top) {
	_curPort->curTop = top;
	_curPort->curLeft = left;
}

void SciGuiGfx::Move(int16 left, int16 top) {
	_curPort->curTop += top;
	_curPort->curLeft += left;
}

GuiResourceId SciGuiGfx::GetFontId() {
	return _curPort->fontId;
}

SciGuiFont *SciGuiGfx::GetFont() {
	if ((_font == NULL) || (_font->getResourceId() != _curPort->fontId))
		_font = new SciGuiFont(_s->resMan, _curPort->fontId);

	return _font;
}

void SciGuiGfx::SetFont(GuiResourceId fontId) {
	if ((_font == NULL) || (_font->getResourceId() != fontId))
		_font = new SciGuiFont(_s->resMan, fontId);

	_curPort->fontId = _font->getResourceId();
	_curPort->fontHeight = _font->getHeight();
}

void SciGuiGfx::OpenPort(GuiPort *port) {
	port->fontId = 0;
	port->fontHeight = 8;

	GuiPort *tmp = _curPort;
	_curPort = port;
	SetFont(port->fontId);
	_curPort = tmp;

	port->top = 0;
	port->left = 0;
	port->textFace = 0;
	port->penClr = 0;
	port->backClr = 255;
	port->penMode = 0;
	port->rect = _bounds;
}

void SciGuiGfx::PenColor(int16 color) {
	_curPort->penClr = color;
}

void SciGuiGfx::PenMode(int16 mode) {
	_curPort->penMode = mode;
}

void SciGuiGfx::TextFace(int16 textFace) {
	_curPort->textFace = textFace;
}

int16 SciGuiGfx::GetPointSize(void) {
	return _curPort->fontHeight;
}

void SciGuiGfx::ClearScreen(byte color) {
	FillRect(_curPort->rect, SCI_SCREEN_MASK_ALL, color, 0, 0);
}

void SciGuiGfx::InvertRect(const Common::Rect &rect) {
	int16 oldpenmode = _curPort->penMode;
	_curPort->penMode = 2;
	FillRect(rect, 1, _curPort->penClr, _curPort->backClr);
	_curPort->penMode = oldpenmode;
}
//-----------------------------
void SciGuiGfx::EraseRect(const Common::Rect &rect) {
	FillRect(rect, 1, _curPort->backClr);
}
//-----------------------------
void SciGuiGfx::PaintRect(const Common::Rect &rect) {
	FillRect(rect, 1, _curPort->penClr);
}

void SciGuiGfx::FillRect(const Common::Rect &rect, int16 drawFlags, byte clrPen, byte clrBack, byte bControl) {
	Common::Rect r = rect;
	r.clip(_curPort->rect);
	if (r.isEmpty()) // nothing to fill
		return;

	int16 oldPenMode = _curPort->penMode;
	OffsetRect(r);
	int16 x, y;
	byte curVisual;

	// Doing visual first
	if (drawFlags & SCI_SCREEN_MASK_VISUAL) {
		if (oldPenMode == 2) { // invert mode
			for (y = r.top; y < r.bottom; y++) {
				for (x = r.left; x < r.right; x++) {
					curVisual = _screen->getVisual(x, y);
					if (curVisual == clrPen) {
						_screen->putPixel(x, y, 1, clrBack, 0, 0);
					} else if (curVisual == clrBack) {
						_screen->putPixel(x, y, 1, clrPen, 0, 0);
					}
				}
			}
		} else { // just fill rect with ClrPen
			for (y = r.top; y < r.bottom; y++) {
				for (x = r.left; x < r.right; x++) {
					_screen->putPixel(x, y, 1, clrPen, 0, 0);
				}
			}
		}
	}

	if (drawFlags < 2)
		return;
	drawFlags &= SCI_SCREEN_MASK_PRIORITY|SCI_SCREEN_MASK_CONTROL;

	if (oldPenMode != 2) {
		for (y = r.top; y < r.bottom; y++) {
			for (x = r.left; x < r.right; x++) {
				_screen->putPixel(x, y, drawFlags, 0, clrBack, bControl);
			}
		}
	} else {
		for (y = r.top; y < r.bottom; y++) {
			for (x = r.left; x < r.right; x++) {
				_screen->putPixel(x, y, drawFlags, 0, !_screen->getPriority(x, y), !_screen->getControl(x, y));
			}
		}
	}
}

void SciGuiGfx::FrameRect(const Common::Rect &rect) {
	Common::Rect r;
	// left
	r = rect;
	r.right = rect.left + 1;
	PaintRect(r);
	// right
	r.right = rect.right;
	r.left = rect.right - 1;
	PaintRect(r);
	//top
	r.left = rect.left;
	r.bottom = rect.top + 1;
	PaintRect(r);
	//bottom
	r.bottom = rect.bottom;
	r.top = rect.bottom - 1;
	PaintRect(r);
}

void SciGuiGfx::OffsetRect(Common::Rect &r) {
	r.top += _curPort->top;
	r.bottom += _curPort->top;
	r.left += _curPort->left;
	r.right += _curPort->left;
}

byte SciGuiGfx::CharHeight(int16 ch) {
#if 0
	CResFont *res = getResFont();
	return res ? res->getCharH(ch) : 0;
#endif
	return 0;
}
//-----------------------------
byte SciGuiGfx::CharWidth(int16 ch) {
	SciGuiFont *font = GetFont();
	return font ? font->getCharWidth(ch) : 0;
}

void SciGuiGfx::ClearChar(int16 chr) {
	if (_curPort->penMode != 1)
		return;
	Common::Rect rect;
	rect.top = _curPort->curTop;
	rect.bottom = rect.top + _curPort->fontHeight;
	rect.left = _curPort->curLeft;
	rect.right = rect.left + CharWidth(chr);
	EraseRect(rect);
}

void SciGuiGfx::DrawChar(int16 chr) {
	chr = chr & 0xFF;
	ClearChar(chr);
	StdChar(chr);
	_curPort->curLeft += CharWidth(chr);
}

void SciGuiGfx::StdChar(int16 chr) {
#if 0
	CResFont*res = getResFont();
	if (res)
		res->Draw(chr, _curPort->top + _curPort->curTop, _curPort->left
				+ _curPort->curLeft, _vSeg, 320, _curPort->penClr,
				_curPort->textFace);
#endif
}

void SciGuiGfx::SetTextFonts(int argc, reg_t *argv) {
	int i;

	if (_textFonts) {
		delete _textFonts;
	}
	_textFontsCount = argc;
	_textFonts = new GuiResourceId[argc];
	for (i = 0; i < argc; i++) {
		_textFonts[i] = (GuiResourceId)argv[i].toUint16();
	}
}

void SciGuiGfx::SetTextColors(int argc, reg_t *argv) {
	int i;

	if (_textColors) {
		delete _textColors;
	}
	_textColorsCount = argc;
	_textColors = new uint16[argc];
	for (i = 0; i < argc; i++) {
		_textColors[i] = argv[i].toUint16();
	}
}

// This internal function gets called as soon as a '|' is found in a text
//  It will process the encountered code and set new font/set color
//  We only support one-digit codes currently, don't know if multi-digit codes are possible
//  Returns textcode character count
int16 SciGuiGfx::TextCodeProcessing(const char *&text, GuiResourceId orgFontId, int16 orgPenColor) {
	const char *textCode = text;
	int16 textCodeSize = 0;
	char curCode;
	unsigned char curCodeParm;

	// Find the end of the textcode
	while ((++textCodeSize) && (*text++ != 0x7C)) { }

	// possible TextCodes:
	//  c -> sets textColor to current port pen color
	//  cX -> sets textColor to _textColors[X-1]
	curCode = textCode[0];
	curCodeParm = textCode[1];
	if (isdigit(curCodeParm)) {
		curCodeParm -= '0';
	} else {
		curCodeParm = 0;
	}
	switch (curCode) {
	case 'c': // set text color
		if (curCodeParm == 0) {
			_curPort->penClr = orgPenColor;
		} else {
			if (curCodeParm < _textColorsCount) {
				_curPort->penClr = _textColors[curCodeParm];
			}
		}
		break;
	case 'f':
		if (curCodeParm == 0) {
			SetFont(orgFontId);
		} else {
			if (curCodeParm < _textFontsCount) {
				SetFont(_textFonts[curCodeParm]);
			}
		}
		break;
	}
	return textCodeSize;
}

// return max # of chars to fit maxwidth with full words
int16 SciGuiGfx::GetLongest(const char *text, int16 maxWidth, GuiResourceId orgFontId) {
	char curChar;
	int16 maxChars = 0, curCharCount = 0;
	uint16 width = 0;
	GuiResourceId oldFontId = GetFontId();
	int16 oldPenColor = _curPort->penClr;

	GetFont();
	if (!_font)
		return 0;

	while (width <= maxWidth) {
		curChar = *text++;
		switch (curChar) {
		case 0x7C:
			curCharCount++;
			curCharCount += TextCodeProcessing(text, orgFontId, oldPenColor);
			continue;

		case 0xD:
			curCharCount++;
			continue;

		case 0xA:
			curCharCount++;
		case 0:
			SetFont(oldFontId);
			PenColor(oldPenColor);
			return curCharCount;

		case ' ':
			maxChars = curCharCount + 1;
			break;
		}
		width += _font->getCharWidth(curChar);
		curCharCount++;
	}
	SetFont(oldFontId);
	PenColor(oldPenColor);
	return maxChars;
}

void SciGuiGfx::TextWidth(const char *text, int16 from, int16 len, GuiResourceId orgFontId, int16 &textWidth, int16 &textHeight) {
	unsigned char curChar;
	GuiResourceId oldFontId = GetFontId();
	int16 oldPenColor = _curPort->penClr;

	textWidth = 0; textHeight = 0;

	GetFont();
	if (_font) {
		text += from;
		while (len--) {
			curChar = *text++;
			switch (curChar) {
			case 0x7C:
				len -= TextCodeProcessing(text, orgFontId, 0);
				break;
			case 0x0A:
			case 0x0D:
				textHeight = MAX<int16> (textHeight, _curPort->fontHeight);
				break;
			default:
				textHeight = MAX<int16> (textHeight, _curPort->fontHeight);
				textWidth += _font->getCharWidth(curChar);
			}
		}
	}
	SetFont(oldFontId);
	PenColor(oldPenColor);
	return;
}

void SciGuiGfx::StringWidth(const char *str, GuiResourceId orgFontId, int16 &textWidth, int16 &textHeight) {
	TextWidth(str, 0, (int16)strlen(str), orgFontId, textWidth, textHeight);
}

int16 SciGuiGfx::TextSize(Common::Rect &rect, const char *str, GuiResourceId fontId, int16 maxWidth) {
	GuiResourceId oldFontId = GetFontId();
	int16 oldPenColor = _curPort->penClr;
	int16 charCount;
	int16 maxTextWidth = 0, textWidth;
	int16 totalHeight = 0, textHeight;

	if (fontId != -1)
		SetFont(fontId);
	rect.top = rect.left = 0;

	if (maxWidth < 0) { // force output as single line
		StringWidth(str, oldFontId, textWidth, textHeight);
		rect.bottom = textHeight;
		rect.right = textWidth;
	} else {
		// rect.right=found widest line with RTextWidth and GetLongest
		// rect.bottom=num. lines * GetPointSize
		rect.right = (maxWidth ? maxWidth : 192);
		const char*p = str;
		while (*p) {
			//if (*p == 0xD || *p == 0xA) {
			//	p++;
			//	continue;
			//}
			charCount = GetLongest(p, rect.right, oldFontId);
			if (charCount == 0)
				break;
			TextWidth(p, 0, charCount, oldFontId, textWidth, textHeight);
			maxTextWidth = MAX(textWidth, maxTextWidth);
			totalHeight += textHeight;
			p += charCount;
		}
		rect.bottom = totalHeight;
		rect.right = maxWidth ? maxWidth : MIN(rect.right, maxTextWidth);
	}
	SetFont(oldFontId);
	PenColor(oldPenColor);
	return rect.right;
}

// returns maximum font height used
void SciGuiGfx::DrawText(const char *text, int16 from, int16 len, GuiResourceId orgFontId, int16 orgPenColor) {
	int16 curChar, charWidth;
	Common::Rect rect;

	GetFont();
	if (!_font)
		return;

	rect.top = _curPort->curTop;
	rect.bottom = rect.top + _curPort->fontHeight;
	text += from;
	while (len--) {
		curChar = (*text++);
		switch (curChar) {
		case 0x7C:
			len -= TextCodeProcessing(text, orgFontId, orgPenColor);
			break;

		case 0x0A:
		case 0x0D:
		case 0:
			break;

		default:
			charWidth = _font->getCharWidth(curChar);
			// clear char
			if (_curPort->penMode == 1) {
				rect.left = _curPort->curLeft;
				rect.right = rect.left + charWidth;
				EraseRect(rect);
			}
			// CharStd
			_font->draw(_screen, curChar, _curPort->top + _curPort->curTop, _curPort->left + _curPort->curLeft, _curPort->penClr, _curPort->textFace);
			_curPort->curLeft += charWidth;
		}
	}
}

// returns maximum font height used
void SciGuiGfx::ShowText(const char *text, int16 from, int16 len, GuiResourceId orgFontId, int16 orgPenColor) {
	Common::Rect rect;

	rect.top = _curPort->curTop;
	rect.bottom = rect.top + GetPointSize();
	rect.left = _curPort->curLeft;
	DrawText(text, from, len, orgFontId, orgPenColor);
	rect.right = _curPort->curLeft;
	ShowBits(rect, 1);
}

// Draws a text in rect.
void SciGuiGfx::TextBox(const char *text, int16 bshow, const Common::Rect &rect, int16 align, GuiResourceId fontId) {
	int16 textWidth, textHeight, charCount, offset;
	int16 hline = 0;
	GuiResourceId orgFontId = GetFontId();
	int16 orgPenColor = _curPort->penClr;

	if (fontId != -1)
		SetFont(fontId);

	while (*text) {
//		if (*text == 0xD || *text == 0xA) {
//			text++;
//			continue;
//		}
		charCount = GetLongest(text, rect.width(), orgFontId);
		if (charCount == 0)
			break;
		TextWidth(text, 0, charCount, orgFontId, textWidth, textHeight);
		switch (align) {
		case -1: // right-aligned
			offset = rect.width() - textWidth;
			break;
		case 1: // center text
			offset = (rect.width() - textWidth) / 2;
			break;
		default: // left-aligned
			offset = 0;
		}
		MoveTo(rect.left + offset, rect.top + hline);

		if (bshow) {
			ShowText(text, 0, charCount, orgFontId, orgPenColor);
		} else {
			DrawText(text, 0, charCount, orgFontId, orgPenColor);
		}

		hline += textHeight;
		text += charCount;
	}
	SetFont(orgFontId);
	PenColor(orgPenColor);
}

// Update (part of) screen
void SciGuiGfx::ShowBits(const Common::Rect &r, uint16 screenMask) {
	Common::Rect rect(r.left, r.top, r.right, r.bottom);
	rect.clip(_curPort->rect);
	if (rect.isEmpty()) // nothing to show
		return;

	OffsetRect(rect);
	assert((screenMask & 0x8000) == 0);
	_screen->copyToScreen();
//	_system->copyRectToScreen(GetSegment(flags) + _baseTable[rect.top] + rect.left, 320, rect.left, rect.top, rect.width(), rect.height());
//	_system->updateScreen();
}

GuiMemoryHandle SciGuiGfx::SaveBits(const Common::Rect &rect, byte screenMask) {
	GuiMemoryHandle memoryId;
	byte *memoryPtr;
	int size;

	Common::Rect r(rect.left, rect.top, rect.right, rect.bottom);
	r.clip(_curPort->rect);
	if (r.isEmpty()) // nothing to save
		return NULL_REG;

	OffsetRect(r); //local port coords to screen coords

	// now actually ask _screen how much space it will need for saving
	size = _screen->getBitsDataSize(r, screenMask);

	memoryId = kalloc(_s->_segMan, "SaveBits()", size);
	memoryPtr = kmem(_s->_segMan, memoryId);
	_screen->saveBits(r, screenMask, memoryPtr);
	return memoryId;
}

void SciGuiGfx::RestoreBits(GuiMemoryHandle memoryHandle) {
	byte *memoryPtr = NULL;

	if (!memoryHandle.isNull()) {
		memoryPtr = kmem(_s->_segMan, memoryHandle);;

		if (memoryPtr) {
			_screen->restoreBits(memoryPtr);
			kfree(_s->_segMan, memoryHandle);
		}
	}
}

// Sierra's Bresenham line drawing
// WARNING: Do not just blindly replace this with Graphics::drawLine(), as it seems to create issues with flood fill
void SciGuiGfx::drawLine(int16 left, int16 top, int16 right, int16 bottom, byte color, byte priority, byte control) {
	//set_drawing_flag
	byte drawMask = _screen->getDrawingMask(color, priority, control);

	// offseting the line
	left += _curPort->left;
	right += _curPort->left;
	top += _curPort->top;
	bottom += _curPort->top;

	// horizontal line
	if (top == bottom) {
		if (right < left)
			SWAP(right, left);
		for (int i = left; i <= right; i++)
			_screen->putPixel(i, top, drawMask, color, priority, control);
		return;
	}
	// vertical line
	if (left == right) {
		if (top > bottom)
			SWAP(top, bottom);
		for (int i = top; i <= bottom; i++)
			_screen->putPixel(left, i, drawMask, color, priority, control);
		return;
	}
	// sloped line - draw with Bresenham algorithm
	int dy = bottom - top;
	int dx = right - left;
	int stepy = dy < 0 ? -1 : 1;
	int stepx = dx < 0 ? -1 : 1;
	dy = ABS(dy) << 1;
	dx = ABS(dx) << 1;

	// setting the 1st and last pixel
	_screen->putPixel(left, top, drawMask, color, priority, control);
	_screen->putPixel(right, bottom, drawMask, color, priority, control);
	// drawing the line
	if (dx > dy) { // going horizontal
		int fraction = dy - (dx >> 1);
		while (left != right) {
			if (fraction >= 0) {
				top += stepy;
				fraction -= dx;
			}
			left += stepx;
			fraction += dy;
			_screen->putPixel(left, top, drawMask, color, priority, control);
		}
	} else { // going vertical
		int fraction = dx - (dy >> 1);
		while (top != bottom) {
			if (fraction >= 0) {
				left += stepx;
				fraction -= dy;
			}
			top += stepy;
			fraction += dx;
			_screen->putPixel(left, top, drawMask, color, priority, control);
		}
	}
	//g_sci->eventMgr->waitUntil(5);
	//ShowBits(&_rThePort->rect,6);
}

// Bitmap for drawing sierra circles
static const byte s_patternCircles[8][30] = {
	{ 0x01 },
	{ 0x4C, 0x02 },
	{ 0xCE, 0xF7, 0x7D, 0x0E },
	{ 0x1C, 0x3E, 0x7F, 0x7F, 0x7F, 0x3E, 0x1C, 0x00 },
	{ 0x38, 0xF8, 0xF3, 0xDF, 0x7F, 0xFF, 0xFD, 0xF7, 0x9F, 0x3F, 0x38 },
	{ 0x70, 0xC0, 0x1F, 0xFE, 0xE3, 0x3F, 0xFF, 0xF7, 0x7F, 0xFF, 0xE7, 0x3F, 0xFE, 0xC3, 0x1F, 0xF8, 0x00 },
	{ 0xF0, 0x01, 0xFF, 0xE1, 0xFF, 0xF8, 0x3F, 0xFF, 0xDF, 0xFF, 0xF7, 0xFF, 0xFD, 0x7F, 0xFF, 0x9F, 0xFF,
		0xE3, 0xFF, 0xF0, 0x1F, 0xF0, 0x01 },
	{ 0xE0, 0x03, 0xF8, 0x0F, 0xFC, 0x1F, 0xFE, 0x3F, 0xFE, 0x3F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF, 0x7F, 0xFF,
		0x7F, 0xFF, 0x7F, 0xFE, 0x3F, 0xFE, 0x3F, 0xFC, 0x1F, 0xF8, 0x0F, 0xE0, 0x03 }
//  { 0x01 };
//	{ 0x03, 0x03, 0x03 },
//	{ 0x02, 0x07, 0x07, 0x07, 0x02 },
//	{ 0x06, 0x06, 0x0F, 0x0F, 0x0F, 0x06, 0x06 },
//	{ 0x04, 0x0E, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x0E, 0x04 },
//	{ 0x0C, 0x1E, 0x1E, 0x1E, 0x3F, 0x3F, 0x3F, 0x1E, 0x1E, 0x1E, 0x0C },
//	{ 0x1C, 0x3E, 0x3E, 0x3E, 0x7F, 0x7F, 0x7F, 0x7F, 0x7F, 0x3E, 0x3E, 0x3E, 0x1C },
//	{ 0x18, 0x3C, 0x7E, 0x7E, 0x7E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7E, 0x7E, 0x7E, 0x3C, 0x18 }
};

// TODO: perhaps this is a better way to set the s_patternTextures array below?
//  in that case one would need to adjust bits of secondary table. Bit 256 is ignored by original interpreter
#if 0
static const byte patternTextures[32 * 2] = {
	0x04, 0x29, 0x40, 0x24, 0x09, 0x41, 0x25, 0x45,
	0x41, 0x90, 0x50, 0x44, 0x48, 0x08, 0x42, 0x28,
	0x89, 0x52, 0x89, 0x88, 0x10, 0x48, 0xA4, 0x08,
	0x44, 0x15, 0x28, 0x24, 0x00, 0x0A, 0x24, 0x20,
	// Now the table is actually duplicated, so we won't need to wrap around
	0x04, 0x29, 0x40, 0x24, 0x09, 0x41, 0x25, 0x45,
	0x41, 0x90, 0x50, 0x44, 0x48, 0x08, 0x42, 0x28,
	0x89, 0x52, 0x89, 0x88, 0x10, 0x48, 0xA4, 0x08,
	0x44, 0x15, 0x28, 0x24, 0x00, 0x0A, 0x24, 0x20,
};
#endif

// This table is bitwise upwards (from bit0 to bit7), sierras original table went down the bits (bit7 to bit0)
//  this was done to simplify things, so we can just run through the table w/o worrying too much about clipping
static const bool s_patternTextures[32 * 8 * 2] = {
	false, false,  true, false, false, false, false, false, // 0x04
	 true, false, false,  true, false,  true, false, false, // 0x29
	false, false, false, false, false, false,  true, false, // 0x40
	false, false,  true, false, false,  true, false, false, // 0x24
	 true, false, false,  true, false, false, false, false, // 0x09
	 true, false, false, false, false, false,  true, false, // 0x41
	 true, false,  true, false, false,  true, false, false, // 0x25
	 true, false,  true, false, false, false,  true, false, // 0x45
	 true, false, false, false, false, false,  true, false, // 0x41
	false, false, false, false,  true, false, false,  true, // 0x90
	false, false, false, false,  true, false,  true, false, // 0x50
	false, false,  true, false, false, false,  true, false, // 0x44
	false, false, false,  true, false, false,  true, false, // 0x48
	false, false, false,  true, false, false, false, false, // 0x08
	false,  true, false, false, false, false,  true, false, // 0x42
	false, false, false,  true, false,  true, false, false, // 0x28
	 true, false, false,  true, false, false, false,  true, // 0x89
	false,  true, false, false,  true, false,  true, false, // 0x52
	 true, false, false,  true, false, false, false,  true, // 0x89
	false, false, false,  true, false, false, false,  true, // 0x88
	false, false, false, false,  true, false, false, false, // 0x10
	false, false, false,  true, false, false,  true, false, // 0x48
	false, false,  true, false, false,  true, false,  true, // 0xA4
	false, false, false,  true, false, false, false, false, // 0x08
	false, false,  true, false, false, false,  true, false, // 0x44
	 true, false,  true, false,  true, false, false, false, // 0x15
	false, false, false,  true, false,  true, false, false, // 0x28
	false, false,  true, false, false,  true, false, false, // 0x24
	false, false, false, false, false, false, false, false, // 0x00
	false,  true, false,  true, false, false, false, false, // 0x0A
	false, false,  true, false, false,  true, false, false, // 0x24
	false, false, false, false, false,  true, false,        // 0x20 (last bit is not mentioned cause original interpreter also ignores that bit)
	// Now the table is actually duplicated, so we won't need to wrap around
	false, false,  true, false, false, false, false, false, // 0x04
	 true, false, false,  true, false,  true, false, false, // 0x29
	false, false, false, false, false, false,  true, false, // 0x40
	false, false,  true, false, false,  true, false, false, // 0x24
	 true, false, false,  true, false, false, false, false, // 0x09
	 true, false, false, false, false, false,  true, false, // 0x41
	 true, false,  true, false, false,  true, false, false, // 0x25
	 true, false,  true, false, false, false,  true, false, // 0x45
	 true, false, false, false, false, false,  true, false, // 0x41
	false, false, false, false,  true, false, false,  true, // 0x90
	false, false, false, false,  true, false,  true, false, // 0x50
	false, false,  true, false, false, false,  true, false, // 0x44
	false, false, false,  true, false, false,  true, false, // 0x48
	false, false, false,  true, false, false, false, false, // 0x08
	false,  true, false, false, false, false,  true, false, // 0x42
	false, false, false,  true, false,  true, false, false, // 0x28
	 true, false, false,  true, false, false, false,  true, // 0x89
	false,  true, false, false,  true, false,  true, false, // 0x52
	 true, false, false,  true, false, false, false,  true, // 0x89
	false, false, false,  true, false, false, false,  true, // 0x88
	false, false, false, false,  true, false, false, false, // 0x10
	false, false, false,  true, false, false,  true, false, // 0x48
	false, false,  true, false, false,  true, false,  true, // 0xA4
	false, false, false,  true, false, false, false, false, // 0x08
	false, false,  true, false, false, false,  true, false, // 0x44
	 true, false,  true, false,  true, false, false, false, // 0x15
	false, false, false,  true, false,  true, false, false, // 0x28
	false, false,  true, false, false,  true, false, false, // 0x24
	false, false, false, false, false, false, false, false, // 0x00
	false,  true, false,  true, false, false, false, false, // 0x0A
	false, false,  true, false, false,  true, false, false, // 0x24
	false, false, false, false, false,  true, false,        // 0x20 (last bit is not mentioned cause original interpreter also ignores that bit)
};

// Bit offsets into pattern_textures
static const byte s_patternTextureOffset[128] = {
	0x00, 0x18, 0x30, 0xc4, 0xdc, 0x65, 0xeb, 0x48,
	0x60, 0xbd, 0x89, 0x05, 0x0a, 0xf4, 0x7d, 0x7d,
	0x85, 0xb0, 0x8e, 0x95, 0x1f, 0x22, 0x0d, 0xdf,
	0x2a, 0x78, 0xd5, 0x73, 0x1c, 0xb4, 0x40, 0xa1,
	0xb9, 0x3c, 0xca, 0x58, 0x92, 0x34, 0xcc, 0xce,
	0xd7, 0x42, 0x90, 0x0f, 0x8b, 0x7f, 0x32, 0xed,
	0x5c, 0x9d, 0xc8, 0x99, 0xad, 0x4e, 0x56, 0xa6,
	0xf7, 0x68, 0xb7, 0x25, 0x82, 0x37, 0x3a, 0x51,
	0x69, 0x26, 0x38, 0x52, 0x9e, 0x9a, 0x4f, 0xa7,
	0x43, 0x10, 0x80, 0xee, 0x3d, 0x59, 0x35, 0xcf,
	0x79, 0x74, 0xb5, 0xa2, 0xb1, 0x96, 0x23, 0xe0,
	0xbe, 0x05, 0xf5, 0x6e, 0x19, 0xc5, 0x66, 0x49,
	0xf0, 0xd1, 0x54, 0xa9, 0x70, 0x4b, 0xa4, 0xe2,
	0xe6, 0xe5, 0xab, 0xe4, 0xd2, 0xaa, 0x4c, 0xe3,
	0x06, 0x6f, 0xc6, 0x4a, 0xa4, 0x75, 0x97, 0xe1
};

void SciGuiGfx::Draw_Box(Common::Rect box, byte color, byte prio, byte control) {
	byte flag = _screen->getDrawingMask(color, prio, control);
	int y, x;

	for (y = box.top; y < box.bottom; y++) {
		for (x = box.left; x < box.right; x++) {
			_screen->putPixel(x, y, flag, color, prio, control);
		}
	}
}

void SciGuiGfx::Draw_TexturedBox(Common::Rect box, byte color, byte prio, byte control, byte texture) {
	byte flag = _screen->getDrawingMask(color, prio, control);
	const bool *textureData = &s_patternTextures[s_patternTextureOffset[texture]];
	int y, x;

	for (y = box.top; y < box.bottom; y++) {
		for (x = box.left; x < box.right; x++) {
			if (*textureData) {
				_screen->putPixel(x, y, flag, color, prio, control);
			}
			textureData++;
		}
	}
}

void SciGuiGfx::Draw_Circle(Common::Rect box, byte size, byte color, byte prio, byte control) {
	byte flag = _screen->getDrawingMask(color, prio, control);
	byte *circleData = (byte *)&s_patternCircles[size];
	byte bitmap = *circleData;
	byte bitNo = 0;
	int y, x;

	for (y = box.top; y < box.bottom; y++) {
		for (x = box.left; x < box.right; x++) {
			if (bitmap & 1) {
				_screen->putPixel(x, y, flag, color, prio, control);
			}
			bitNo++;
			if (bitNo == 8) {
				circleData++; bitmap = *circleData; bitNo = 0;
			} else {
				bitmap = bitmap >> 1;
			}
		}
	}
}

void SciGuiGfx::Draw_TexturedCircle(Common::Rect box, byte size, byte color, byte prio, byte control, byte texture) {
	byte flag = _screen->getDrawingMask(color, prio, control);
	byte *circleData = (byte *)&s_patternCircles[size];
	byte bitmap = *circleData;
	byte bitNo = 0;
	const bool *textureData = &s_patternTextures[s_patternTextureOffset[texture]];
	int y, x;

	for (y = box.top; y < box.bottom; y++) {
		for (x = box.left; x < box.right; x++) {
			if (bitmap & 1) {
				if (*textureData) {
					_screen->putPixel(x, y, flag, color, prio, control);
				}
				textureData++;
			}
			bitNo++;
			if (bitNo == 8) {
				circleData++; bitmap = *circleData; bitNo = 0;
			} else {
				bitmap = bitmap >> 1;
			}
		}
	}
}

void SciGuiGfx::Draw_Pattern(int16 x, int16 y, byte color, byte priority, byte control, byte code, byte texture) {
	byte size = code & SCI_PATTERN_CODE_PENSIZE;
	Common::Rect rect;

	// We need to adjust the given coordinates, because the ones given us do not define upper left but somewhat middle
	y -= size; if (y < 0) y = 0;
	x -= size; if (x < 0) x = 0;

	rect.top = y; rect.left = x;
	rect.setHeight((size*2)+1); rect.setWidth((size*2)+2);
	OffsetRect(rect);
	rect.clip(_screen->_width, _screen->_height);

	if (code & SCI_PATTERN_CODE_RECTANGLE) {
		// Rectangle
		if (code & SCI_PATTERN_CODE_USE_TEXTURE) {
			Draw_TexturedBox(rect, color, priority, control, texture);
		} else {
			Draw_Box(rect, color, priority, control);
		}

	} else {
		// Circle
		if (code & SCI_PATTERN_CODE_USE_TEXTURE) {
			Draw_TexturedCircle(rect, size, color, priority, control, texture);
		} else {
			Draw_Circle(rect, size, color, priority, control);
		}
	}
}

void SciGuiGfx::Draw_String(const char *text) {
	GuiResourceId orgFontId = GetFontId();
	int16 orgPenColor = _curPort->penClr;

	DrawText(text, 0, strlen(text), orgFontId, orgPenColor);
	SetFont(orgFontId);
	PenColor(orgPenColor);
}

// Do not replace w/ some generic code. This algo really needs to behave exactly as the one from sierra
void SciGuiGfx::FloodFill(int16 x, int16 y, byte color, byte priority, byte control) {
	Common::Stack<Common::Point> stack;
	Common::Point p, p1;

	byte screenMask = _screen->getDrawingMask(color, priority, control), matchMask;
	p.x = x + _curPort->left;
	p.y = y + _curPort->top;
	stack.push(p);

	byte searchColor = _screen->getVisual(p.x, p.y);
	byte searchPriority = _screen->getPriority(p.x, p.y);
	byte searchControl = _screen->getControl(p.x, p.y);
	int16 w, e, a_set, b_set;
	// if in 1st point priority,control or color is already set to target, clear the flag
	if (screenMask & SCI_SCREEN_MASK_VISUAL && searchColor == color)
		screenMask ^= SCI_SCREEN_MASK_VISUAL;
	if (screenMask & SCI_SCREEN_MASK_PRIORITY && searchPriority == priority)
		screenMask ^= SCI_SCREEN_MASK_PRIORITY;
	if (screenMask & SCI_SCREEN_MASK_CONTROL && searchControl == control)
		screenMask ^= SCI_SCREEN_MASK_CONTROL;
	if (screenMask == 0)// nothing to fill
		return;

	// hard borders for filling
	int l = _curPort->rect.left + _curPort->left;
	int t = _curPort->rect.top + _curPort->top;
	int r = _curPort->rect.right + _curPort->left - 1;
	int b = _curPort->rect.bottom + _curPort->top - 1;
	while (stack.size()) {
		p = stack.pop();
		if ((matchMask = _screen->isFillMatch(p.x, p.y, screenMask, searchColor, searchPriority, searchControl)) == 0) // already filled
			continue;
		_screen->putPixel(p.x, p.y, screenMask, color, priority, control);
		w = p.x;
		e = p.x;
		// moving west and east pointers as long as there is a matching color to fill
		while (w > l && (matchMask == _screen->isFillMatch(w - 1, p.y, screenMask, searchColor, searchPriority, searchControl)))
			_screen->putPixel(--w, p.y, matchMask, color, priority, control);
		while (e < r && (matchMask == _screen->isFillMatch(e + 1, p.y, screenMask, searchColor, searchPriority, searchControl)))
			_screen->putPixel(++e, p.y, matchMask, color, priority, control);
		// checking lines above and below for possible flood targets
		a_set = b_set = 0;
		while (w <= e) {
			if (p.y > t && (matchMask == _screen->isFillMatch(w, p.y - 1, screenMask, searchColor, searchPriority, searchControl))) { // one line above
				if (a_set == 0) {
					p1.x = w;
					p1.y = p.y - 1;
					stack.push(p1);
					a_set = 1;
				}
			} else
				a_set = 0;

			if (p.y < b && (matchMask == _screen->isFillMatch(w, p.y + 1, screenMask, searchColor, searchPriority, searchControl))) { // one line below
				if (b_set == 0) {
					p1.x = w;
					p1.y = p.y + 1;
					stack.push(p1);
					b_set = 1;
				}
			} else
				b_set = 0;
			w++;
		}
	}
}

void SciGuiGfx::drawPicture(GuiResourceId pictureId, int16 animationNr, bool mirroredFlag, bool addToFlag, GuiResourceId paletteId) {
	SciGuiPicture *picture;

	picture = new SciGuiPicture(_s, this, _screen, _palette, pictureId);
	// do we add to a picture? if not -> clear screen
	if (!addToFlag) {
		if (_s->resMan->isVGA())
			ClearScreen(0);
		else
			ClearScreen(15);
	}
	picture->draw(animationNr, mirroredFlag, addToFlag, paletteId);
}

void SciGuiGfx::drawCel(GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, uint16 leftPos, uint16 topPos, byte priority, uint16 paletteNo) {
	SciGuiView *view = new SciGuiView(_s->resMan, _screen, _palette, viewId);
	Common::Rect rect(0, 0);
	Common::Rect clipRect(0, 0);
	if (view) {
		rect.left = leftPos;
		rect.top = topPos;
		rect.right = rect.left + view->getWidth(loopNo, celNo);
		rect.bottom = rect.top + view->getHeight(loopNo, celNo);
		clipRect = rect;
		clipRect.clip(_curPort->rect);
		if (clipRect.isEmpty()) // nothing to draw
			return;

		Common::Rect clipRectTranslated = clipRect;
		OffsetRect(clipRectTranslated);
		view->draw(rect, clipRect, clipRectTranslated, loopNo, celNo, priority, paletteNo);

		//if (_picNotValid == 0)
		//	_gfx->ShowBits(rect, 1);
	}
}

uint16 SciGuiGfx::onControl(uint16 screenMask, Common::Rect rect) {
	Common::Rect outRect(rect.left, rect.top, rect.right, rect.bottom);
	int16 x, y;
	uint16 result = 0;

	outRect.clip(_curPort->rect);
	if (outRect.isEmpty()) // nothing to control
		return 0;
	OffsetRect(outRect);

	if (screenMask & SCI_SCREEN_MASK_PRIORITY) {
		for (y = outRect.top; y < outRect.bottom; y++) {
			for (x = outRect.left; x < outRect.right; x++) {
				result |= 1 << _screen->getPriority(x, y);
			}
		}
	} else {
		for (y = outRect.top; y < outRect.bottom; y++) {
			for (x = outRect.left; x < outRect.right; x++) {
				result |= 1 << _screen->getControl(x, y);
			}
		}
	}
	return result;
}

static inline int sign_extend_byte(int value) {
	if (value & 0x80)
		return value - 256;
	else
		return value;
}

void SciGuiGfx::PriorityBandsInit(int16 top, int16 bottom) {
	double bandSize;
	int16 y;

	_priorityTop = top;
	_priorityBottom = bottom;
	bandSize = (_priorityBottom - _priorityTop) / (_priorityBandCount - 1);

	memset(_priorityBands, 0, _priorityTop);
	for (y = _priorityTop; y < _priorityBottom; y++)
		_priorityBands[y] = (byte)(1 + (y - _priorityTop) / bandSize);
	for (y = _priorityBottom; y < _screen->_height; y++)
		_priorityBands[y] = _priorityBandCount;
}

void SciGuiGfx::PriorityBandsInit(byte *data) {
	int i = 0, inx;
	byte priority = 0;

	for (inx = 0; inx < 14; inx++) {
		priority = *data++;
		while (i < priority)
			_priorityBands[i++] = inx;
	}
	while (i < 200)
		_priorityBands[i++] = inx;
}

byte SciGuiGfx::CoordinateToPriority(int16 y) {
	if (y < _priorityTop) 
		return _priorityBands[_priorityTop];
	if (y > _priorityBottom)
		return _priorityBands[_priorityBottom];
	return _priorityBands[y];
}

int16 SciGuiGfx::PriorityToCoordinate(byte priority) {
	int16 y;
	if (priority <= _priorityBandCount) {
		for (y = 0; y <= _priorityBottom; y++)
			if (_priorityBands[y] == priority)
				return y;
	}
	return _priorityBottom;
}

void SciGuiGfx::AnimateDisposeLastCast() {
	// FIXME
	//if (!_lastCast->first.isNull())
		//_lastCast->DeleteList();
}

void SciGuiGfx::AnimateInvoke(List *list, int argc, reg_t *argv) {
	SegManager *segMan = _s->_segMan;
	reg_t curAddress = list->first;
	Node *curNode = _s->_segMan->lookupNode(curAddress);
	reg_t curObject;
	uint16 mask;

	while (curNode) {
		curObject = curNode->value;
		mask = GET_SEL32V(curObject, signal);
		if (!(mask & SCI_ANIMATE_SIGNAL_FROZEN)) {
			// Call .doit method of that object
			invoke_selector(_s, curObject, _s->_kernel->_selectorCache.doit, kContinueOnInvalidSelector, argv, argc, __FILE__, __LINE__, 0);
			// Lookup node again, since the nodetable it was in may have been reallocated
			curNode = _s->_segMan->lookupNode(curAddress);
		}
		curAddress = curNode->succ;
		curNode = _s->_segMan->lookupNode(curAddress);
	}
}

void SciGuiGfx::AnimateFill(List *list, byte &old_picNotValid) {
	SegManager *segMan = _s->_segMan;
	reg_t curAddress = list->first;
	Node *curNode = _s->_segMan->lookupNode(curAddress);
	reg_t curObject;
	SciGuiView *view = NULL;
	GuiResourceId viewId;
	GuiViewLoopNo loopNo;
	GuiViewCelNo celNo;
	int16 x, y, z;
	Common::Rect celRect;
	uint16 signal;

	while (curNode) {
		curObject = curNode->value;

		// Get animation data...
		viewId = GET_SEL32V(curObject, view);
		loopNo = GET_SEL32V(curObject, loop);
		celNo = GET_SEL32V(curObject, cel);
		x = GET_SEL32V(curObject, x);
		y = GET_SEL32V(curObject, y);
		z = GET_SEL32V(curObject, z);
		signal = GET_SEL32V(curObject, signal);

		// Get the corresponding view
		view = new SciGuiView(_s->resMan, _screen, _palette, viewId);
		
		// adjust loop and cel, if any of those is invalid
		if (loopNo >= view->getLoopCount()) {
			loopNo = 0;
			PUT_SEL32V(curObject, loop, loopNo);
		}
		if (celNo >= view->getCelCount(loopNo)) {
			celNo = 0;
			PUT_SEL32V(curObject, cel, celNo);
		}

		// Create rect according to coordinates and given cel
		view->getCelRect(loopNo, celNo, x, y, z, &celRect);
		PUT_SEL32V(curObject, nsLeft, celRect.left);
		PUT_SEL32V(curObject, nsTop, celRect.top);
		PUT_SEL32V(curObject, nsRight, celRect.right);
		PUT_SEL32V(curObject, nsBottom, celRect.bottom);

		if (!(signal & SCI_ANIMATE_SIGNAL_FIXEDPRIORITY))
			PUT_SEL32V(curObject, priority, CoordinateToPriority(y));
		
		if (signal & SCI_ANIMATE_SIGNAL_NOUPDATE) {
			if (signal & (SCI_ANIMATE_SIGNAL_FORCEUPDATE | SCI_ANIMATE_SIGNAL_VIEWUPDATED)
				|| (signal & SCI_ANIMATE_SIGNAL_HIDDEN && !(signal & SCI_ANIMATE_SIGNAL_REMOVEVIEW))
				|| (!(signal & SCI_ANIMATE_SIGNAL_HIDDEN) && signal & SCI_ANIMATE_SIGNAL_REMOVEVIEW)
				|| (signal & SCI_ANIMATE_SIGNAL_ALWAYSUPDATE))
				old_picNotValid++;
			signal &= 0xFFFF ^ SCI_ANIMATE_SIGNAL_STOPUPDATE;
		} else {
			if (signal & SCI_ANIMATE_SIGNAL_STOPUPDATE || signal & SCI_ANIMATE_SIGNAL_ALWAYSUPDATE)
				old_picNotValid++;
			signal &= 0xFFFF ^ SCI_ANIMATE_SIGNAL_FORCEUPDATE;
		}
		PUT_SEL32V(curObject, signal, signal);

		curAddress = curNode->succ;
		curNode = _s->_segMan->lookupNode(curAddress);
	}
}

Common::List<GuiAnimateList> *SciGuiGfx::AnimateMakeSortedList(List *list) {
	SegManager *segMan = _s->_segMan;
	Common::List<GuiAnimateList> *sortedList = new Common::List<GuiAnimateList>;
	GuiAnimateList listHelper;
	reg_t curAddress = list->first;
	Node *curNode = _s->_segMan->lookupNode(curAddress);
	reg_t curObject;

	return sortedList;

	// First convert the given List to Common::List
	while (curNode) {
		curObject = curNode->value;
		listHelper.address = curAddress;
		listHelper.y = (int16)GET_SEL32V(curObject, y);
		listHelper.z = (int16)GET_SEL32V(curObject, z);
		sortedList->push_back(listHelper);

		curAddress = curNode->succ;
		curNode = _s->_segMan->lookupNode(curAddress);
	}

	// Now do a bubble sort on this Common::List
	if (sortedList->size() < 2)
		return sortedList;

	sortedList->begin();
//	Common::List<ExecStack>::iterator iter;
//	for (iter = s->_executionStack.begin();
//	     iter != s->_executionStack.end(); ++iter) {
//		ExecStack &es = *iter;

	

	return sortedList;
}

#define SCI_ANIMATE_MAXLIST 50

void SciGuiGfx::AnimateUpdate(List *list) {
	SegManager *segMan = _s->_segMan;
	reg_t curAddress = list->first;
	Node *curNode = _s->_segMan->lookupNode(curAddress);
	reg_t curObject;
	int16 listNr = 0, listCount = 0;
	reg_t object[SCI_ANIMATE_MAXLIST];
	GuiResourceId viewId[SCI_ANIMATE_MAXLIST];
	GuiViewLoopNo loopNo[SCI_ANIMATE_MAXLIST];
	GuiViewCelNo celNo[SCI_ANIMATE_MAXLIST];
	int16 z[SCI_ANIMATE_MAXLIST];
	Common::Rect celRect[SCI_ANIMATE_MAXLIST];
	uint16 paletteNo[SCI_ANIMATE_MAXLIST], signal[SCI_ANIMATE_MAXLIST];
	reg_t bitsHandle;
	Common::Rect rect;

	// first cache information about the list
	// FIXME: perhaps make a list that is persistent in memory and resize it on demand...
	while (curNode) {
		curObject = curNode->value;

		object[listNr] = curObject;
		viewId[listNr] = GET_SEL32V(curObject, view);
		loopNo[listNr] = GET_SEL32V(curObject, loop);
		celNo[listNr] = GET_SEL32V(curObject, cel);
		celRect[listNr].left = GET_SEL32V(curObject, nsLeft);
		celRect[listNr].top = GET_SEL32V(curObject, nsTop);
		celRect[listNr].right = GET_SEL32V(curObject, nsRight);
		celRect[listNr].bottom = GET_SEL32V(curObject, nsBottom);
		z[listNr] = GET_SEL32V(curObject, z);
		paletteNo[listNr] = GET_SEL32V(curObject, palette);
		signal[listNr] = GET_SEL32V(curObject, signal);
		listNr++;

		curAddress = curNode->succ;
		curNode = _s->_segMan->lookupNode(curAddress);
	}
	listCount = listNr;

	// Remove all previous cels from screen
	for (listNr = listCount - 1; listNr >= 0; listNr--) {
		curObject = object[listNr];
		if (signal[listNr] & SCI_ANIMATE_SIGNAL_NOUPDATE) {
			if (!(signal[listNr] & SCI_ANIMATE_SIGNAL_REMOVEVIEW)) {
				bitsHandle = GET_SEL32(curObject, underBits);
				if (_screen->_picNotValid != 1) {
					RestoreBits(bitsHandle);
					//arr1[i] = 1;
				} else	{
					//FreeBits(bits_gfx->UnloadBits(hBits);
				}
				PUT_SEL32V(curObject, underBits, 0);
			}
			signal[listNr] &= 0xFFFF ^ SCI_ANIMATE_SIGNAL_FORCEUPDATE;
			signal[listNr] &= signal[listNr] & SCI_ANIMATE_SIGNAL_VIEWUPDATED ? 0xFFFF ^ (SCI_ANIMATE_SIGNAL_VIEWUPDATED | SCI_ANIMATE_SIGNAL_NOUPDATE) : 0xFFFF;
		} else if (signal[listNr] & SCI_ANIMATE_SIGNAL_STOPUPDATE) {
			signal[listNr] =  (signal[listNr] & (0xFFFF ^ SCI_ANIMATE_SIGNAL_STOPUPDATE)) | SCI_ANIMATE_SIGNAL_NOUPDATE;
		}
	}

	for (listNr = 0; listNr < listCount; listNr++) {
		if (signal[listNr] & SCI_ANIMATE_SIGNAL_ALWAYSUPDATE) {
			curObject = object[listNr];

			// draw corresponding cel
			drawCel(viewId[listNr], loopNo[listNr], celNo[listNr], celRect[listNr].left, celRect[listNr].top, z[listNr], paletteNo[listNr]);
//			arr1[i] = 1;
			signal[listNr] &= 0xFFFF ^ (SCI_ANIMATE_SIGNAL_STOPUPDATE | SCI_ANIMATE_SIGNAL_VIEWUPDATED | SCI_ANIMATE_SIGNAL_NOUPDATE | SCI_ANIMATE_SIGNAL_FORCEUPDATE);
			if ((signal[listNr] & SCI_ANIMATE_SIGNAL_IGNOREACTOR) == 0) {
				rect = celRect[listNr];
				rect.top = CLIP<int16>(PriorityToCoordinate(z[listNr]) - 1, rect.top, rect.bottom - 1);  
				FillRect(rect, SCI_SCREEN_MASK_CONTROL, 0, 0, 15);
			}
		}
	}

	for (listNr = 0; listNr < listCount; listNr++) {
		if (signal[listNr] & SCI_ANIMATE_SIGNAL_NOUPDATE) {
			if (signal[listNr] & SCI_ANIMATE_SIGNAL_HIDDEN) {
				signal[listNr] |= SCI_ANIMATE_SIGNAL_REMOVEVIEW;
			} else {
				signal[listNr] &= 0xFFFF ^ SCI_ANIMATE_SIGNAL_REMOVEVIEW;
				curObject = object[listNr];
				if (signal[listNr] & SCI_ANIMATE_SIGNAL_IGNOREACTOR)
					bitsHandle = SaveBits(celRect[listNr], SCI_SCREEN_MASK_PRIORITY|SCI_SCREEN_MASK_PRIORITY);
				else
					bitsHandle = SaveBits(celRect[listNr], SCI_SCREEN_MASK_ALL);
				PUT_SEL32(curObject, underBits, bitsHandle);
			}
		}
	}

	for (listNr = 0; listNr < listCount; listNr++) {
		curObject = object[listNr];
		if (signal[listNr] & SCI_ANIMATE_SIGNAL_NOUPDATE && !(signal[listNr] & SCI_ANIMATE_SIGNAL_HIDDEN)) {
			// draw corresponding cel
			drawCel(viewId[listNr], loopNo[listNr], celNo[listNr], celRect[listNr].left, celRect[listNr].top, z[listNr], paletteNo[listNr]);
			// arr1[i] = 1;
			if ((signal[listNr] & SCI_ANIMATE_SIGNAL_IGNOREACTOR) == 0) {
				rect = celRect[listNr];
				rect.top = CLIP<int16>(PriorityToCoordinate(z[listNr]) - 1, rect.top, rect.bottom - 1);  
				FillRect(rect, SCI_SCREEN_MASK_CONTROL, 0, 0, 15);
			}
		}
		PUT_SEL32V(curObject, signal, signal[listNr]);
	}
}

void SciGuiGfx::AnimateDrawCels(List *list) {
	SegManager *segMan = _s->_segMan;
	reg_t curAddress = list->first;
	Node *curNode = _s->_segMan->lookupNode(curAddress);
	reg_t curObject;
	GuiResourceId viewId;
	GuiViewLoopNo loopNo;
	GuiViewCelNo celNo;
	int16 x, y, z;
	Common::Rect celRect;
	uint16 signal, paletteNo;
	reg_t bitsHandle;

	while (curNode) {
		curObject = curNode->value;

		signal = GET_SEL32V(curObject, signal);
		if (!(signal & (SCI_ANIMATE_SIGNAL_NOUPDATE | SCI_ANIMATE_SIGNAL_HIDDEN | SCI_ANIMATE_SIGNAL_ALWAYSUPDATE))) {
			// Get animation data...
			viewId = GET_SEL32V(curObject, view);
			loopNo = GET_SEL32V(curObject, loop);
			celNo = GET_SEL32V(curObject, cel);
			x = GET_SEL32V(curObject, x);
			y = GET_SEL32V(curObject, y);
			z = GET_SEL32V(curObject, z);
			paletteNo = GET_SEL32V(curObject, palette);

			celRect.left = GET_SEL32V(curObject, nsLeft);
			celRect.top = GET_SEL32V(curObject, nsTop);
			celRect.right = GET_SEL32V(curObject, nsRight);
			celRect.bottom = GET_SEL32V(curObject, nsBottom);

			// Save background
			bitsHandle = SaveBits(celRect, SCI_SCREEN_MASK_ALL);
			PUT_SEL32(curObject, underBits, bitsHandle);

			// draw corresponding cel
			drawCel(viewId, loopNo, celNo, celRect.left, celRect.top, z, paletteNo);

			// arr1[inx] = 1;
			if (signal & SCI_ANIMATE_SIGNAL_REMOVEVIEW) {
				signal &= 0xFFFF ^ SCI_ANIMATE_SIGNAL_REMOVEVIEW;
				PUT_SEL32V(curObject, signal, signal);
			}
			
//			HEAPHANDLE hNewCast = heapNewPtr(sizeof(sciCast), kDataCast);
//			sciCast *pNewCast = (sciCast *)heap2Ptr(hNewCast);
//			pNewCast->view = view;
//			pNewCast->loop = loop;
//			pNewCast->cel = cel;
//			pNewCast->z = z;
//			pNewCast->pal = pal;
//			pNewCast->hSaved = 0;
//			pNewCast->rect = *rect;
//			_lastCast->AddToEnd(hNewCast);
		}

		curAddress = curNode->succ;
		curNode = _s->_segMan->lookupNode(curAddress);
	}
}

void SciGuiGfx::AnimateRestoreAndDelete(List *list, int argc, reg_t *argv) {
	SegManager *segMan = _s->_segMan;
	reg_t curAddress = list->first;
	Node *curNode = _s->_segMan->lookupNode(curAddress);
	reg_t curObject;
	uint16 signal;

	// FIXME: we are supposed to go through this table backwards
	while (curNode) {
		curObject = curNode->value;
		signal = GET_SEL32V(curObject, signal);

		// FIXME: this is supposed to go into the loop above (same method)
		if (signal & SCI_ANIMATE_SIGNAL_HIDDEN) {
			signal |= SCI_ANIMATE_SIGNAL_REMOVEVIEW;
			PUT_SEL32V(curObject, signal, signal);
		}

		if ((signal & (SCI_ANIMATE_SIGNAL_NOUPDATE | SCI_ANIMATE_SIGNAL_REMOVEVIEW)) == 0) {
			RestoreBits(GET_SEL32(curObject, underBits));
			PUT_SEL32V(curObject, underBits, 0);
		}

		if (signal & SCI_ANIMATE_SIGNAL_DISPOSEME) {
			// Call .delete_ method of that object
			invoke_selector(_s, curObject, _s->_kernel->_selectorCache.delete_, kContinueOnInvalidSelector, argv, argc, __FILE__, __LINE__, 0);
			// Lookup node again, since the nodetable it was in may have been reallocated
			curNode = _s->_segMan->lookupNode(curAddress);
		}
		curAddress = curNode->succ;
		curNode = _s->_segMan->lookupNode(curAddress);
	}
}

void SciGuiGfx::AddToPicDrawCels(List *list) {
	SegManager *segMan = _s->_segMan;
	reg_t curAddress = list->first;
	Node *curNode = _s->_segMan->lookupNode(curAddress);
	reg_t curObject;
	SciGuiView *view = NULL;
	GuiResourceId viewId;
	GuiViewLoopNo loopNo;
	GuiViewCelNo celNo;
	int16 x, y, z, priority;
	uint16 paletteNo, signal;
	Common::Rect celRect;

	while (curNode) {
		curObject = curNode->value;

		// Get cel data...
		viewId = GET_SEL32V(curObject, view);
		loopNo = GET_SEL32V(curObject, loop);
		celNo = GET_SEL32V(curObject, cel);
		x = GET_SEL32V(curObject, x);
		y = GET_SEL32V(curObject, y);
		z = GET_SEL32V(curObject, z);
		priority = GET_SEL32V(curObject, priority);
		if (priority == -1)
			priority = CoordinateToPriority(y);
		paletteNo = GET_SEL32V(curObject, palette);
		signal = GET_SEL32V(curObject, signal);

		// Get the corresponding view
		view = new SciGuiView(_s->resMan, _screen, _palette, viewId);

		// Create rect according to coordinates and given cel
		view->getCelRect(loopNo, celNo, x, y, priority, &celRect);

		// draw corresponding cel
		drawCel(viewId, loopNo, celNo, celRect.left, celRect.top, z, paletteNo);
		if ((signal & SCI_ANIMATE_SIGNAL_IGNOREACTOR) == 0) {
			celRect.top = CLIP<int16>(PriorityToCoordinate(priority) - 1, celRect.top, celRect.bottom - 1);
			FillRect(celRect, SCI_SCREEN_MASK_CONTROL, 0, 0, 15);
		}

		curAddress = curNode->succ;
		curNode = _s->_segMan->lookupNode(curAddress);
	}
}

void SciGuiGfx::SetNowSeen(reg_t objectReference) {
	SegManager *segMan = _s->_segMan;
	SciGuiView *view = NULL;
	Common::Rect celRect(0, 0);
	GuiResourceId viewId = (GuiResourceId)GET_SEL32V(objectReference, view);
	GuiViewLoopNo loopNo = sign_extend_byte((GuiViewLoopNo)GET_SEL32V(objectReference, loop));
	GuiViewCelNo celNo = sign_extend_byte((GuiViewCelNo)GET_SEL32V(objectReference, cel));
	int16 x = (int16)GET_SEL32V(objectReference, x);
	int16 y = (int16)GET_SEL32V(objectReference, y);
	int16 z = 0;
	if (_s->_kernel->_selectorCache.z > -1) {
		z = (int16)GET_SEL32V(objectReference, z);
	}

	// now get cel rectangle
	view = new SciGuiView(_s->resMan, _screen, _palette, viewId);
	view->getCelRect(loopNo, celNo, x, y, z, &celRect);

	// TODO: sometimes loop is negative. Check what it means
	if (lookup_selector(_s->_segMan, objectReference, _s->_kernel->_selectorCache.nsTop, NULL, NULL) == kSelectorVariable) {
		PUT_SEL32V(objectReference, nsLeft, celRect.left);
		PUT_SEL32V(objectReference, nsRight, celRect.right);
		PUT_SEL32V(objectReference, nsTop, celRect.top);
		PUT_SEL32V(objectReference, nsBottom, celRect.bottom);
	}
}

} // End of namespace Sci
