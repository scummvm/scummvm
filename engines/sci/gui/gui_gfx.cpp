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
#include "common/stack.h"
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

	_texteditCursorVisible = false;

	_animateListData = NULL;
	_animateListSize = 0;

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

void SciGuiGfx::BackColor(int16 color) {
	_curPort->backClr = color;
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

void SciGuiGfx::OffsetLine(Common::Point &start, Common::Point &end) {
	start.x += _curPort->left;
	start.y += _curPort->top;
	end.x += _curPort->left;
	end.y += _curPort->top;
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
	BitsShow(rect);
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
void SciGuiGfx::BitsShow(const Common::Rect &r) {
	Common::Rect rect(r.left, r.top, r.right, r.bottom);
	rect.clip(_curPort->rect);
	if (rect.isEmpty()) // nothing to show
		return;

	OffsetRect(rect);
	_screen->copyRectToScreen(rect);
}

GuiMemoryHandle SciGuiGfx::BitsSave(const Common::Rect &rect, byte screenMask) {
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

void SciGuiGfx::BitsRestore(GuiMemoryHandle memoryHandle) {
	byte *memoryPtr = NULL;

	if (!memoryHandle.isNull()) {
		memoryPtr = kmem(_s->_segMan, memoryHandle);;

		if (memoryPtr) {
			_screen->restoreBits(memoryPtr);
			kfree(_s->_segMan, memoryHandle);
		}
	}
}

void SciGuiGfx::BitsFree(GuiMemoryHandle memoryHandle) {
	if (!memoryHandle.isNull()) {
		kfree(_s->_segMan, memoryHandle);
	}
}

void SciGuiGfx::Draw_String(const char *text) {
	GuiResourceId orgFontId = GetFontId();
	int16 orgPenColor = _curPort->penClr;

	DrawText(text, 0, strlen(text), orgFontId, orgPenColor);
	SetFont(orgFontId);
	PenColor(orgPenColor);
}

void SciGuiGfx::drawPicture(GuiResourceId pictureId, int16 animationNr, bool mirroredFlag, bool addToFlag, GuiResourceId paletteId) {
	SciGuiPicture *picture;

	picture = new SciGuiPicture(_s->resMan, this, _screen, _palette, pictureId);
	// do we add to a picture? if not -> clear screen
	if (!addToFlag) {
		if (_s->resMan->isVGA())
			ClearScreen(0);
		else
			ClearScreen(15);
	}
	picture->draw(animationNr, mirroredFlag, addToFlag, paletteId);
}

// This one is the only one that updates screen!
void SciGuiGfx::drawCel(GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, uint16 leftPos, uint16 topPos, byte priority, uint16 paletteNo) {
	SciGuiView *view = new SciGuiView(_s->resMan, _screen, _palette, viewId);
	Common::Rect rect;
	Common::Rect clipRect;
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
		if (!_screen->_picNotValid)
			BitsShow(rect);
	}
}

// This version of drawCel is not supposed to call BitsShow()!
void SciGuiGfx::drawCel(GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, Common::Rect celRect, byte priority, uint16 paletteNo) {
	SciGuiView *view = new SciGuiView(_s->resMan, _screen, _palette, viewId);
	Common::Rect clipRect;
	if (view) {
		clipRect = celRect;
		clipRect.clip(_curPort->rect);
		if (clipRect.isEmpty()) // nothing to draw
			return;

		Common::Rect clipRectTranslated = clipRect;
		OffsetRect(clipRectTranslated);
		view->draw(celRect, clipRect, clipRectTranslated, loopNo, celNo, priority, paletteNo);
	}
}

// This version of drawCel is not supposed to call BitsShow()!
void SciGuiGfx::drawCel(SciGuiView *view, GuiViewLoopNo loopNo, GuiViewCelNo celNo, Common::Rect celRect, byte priority, uint16 paletteNo) {
	Common::Rect clipRect;
	clipRect = celRect;
	clipRect.clip(_curPort->rect);
	if (clipRect.isEmpty()) // nothing to draw
		return;

	Common::Rect clipRectTranslated = clipRect;
	OffsetRect(clipRectTranslated);
	view->draw(celRect, clipRect, clipRectTranslated, loopNo, celNo, priority, paletteNo);
}

const char controlListUpArrow[2]	= { 0x18, 0 };
const char controlListDownArrow[2]	= { 0x19, 0 };

void SciGuiGfx::drawListControl(Common::Rect rect, reg_t obj, int16 maxChars, int16 count, const char **entries, GuiResourceId fontId, int16 upperPos, int16 cursorPos, bool isAlias) {
	SegManager *segMan = _s->_segMan;
	Common::Rect workerRect = rect;
	GuiResourceId oldFontId = GetFontId();
	int16 oldPenColor = _curPort->penClr;
	uint16 fontSize = 0;
	int16 i;
	const char *listEntry;
	int16 listEntryLen;

	// draw basic window
	EraseRect(workerRect);
	workerRect.grow(1);
	FrameRect(workerRect);
	PUT_SEL32V(obj, nsLeft, workerRect.left); PUT_SEL32V(obj, nsTop, workerRect.top);
	PUT_SEL32V(obj, nsRight, workerRect.right); PUT_SEL32V(obj, nsBottom, workerRect.bottom);
	// draw UP/DOWN arrows
	workerRect = rect;
	TextBox(controlListUpArrow, 0, workerRect, 1, 0);
	workerRect.top = workerRect.bottom - 10;
	TextBox(controlListDownArrow, 0, workerRect, 1, 0);

	// Draw inner lines
	workerRect = rect;
	workerRect.top = rect.top + 10;
	workerRect.bottom = rect.bottom - 10;
	FrameRect(workerRect);
	workerRect.grow(-1);
	
	SetFont(fontId);
	fontSize = _curPort->fontHeight;
	PenColor(_curPort->penClr); BackColor(_curPort->backClr);
	workerRect.bottom = workerRect.top + fontSize;

	// Write actual text
	for (i = 0; i < count; i++) {
		EraseRect(workerRect);
		listEntry = entries[i];
		if (listEntry[0]) {
			MoveTo(workerRect.left, workerRect.top + 1);
			listEntryLen = strlen(listEntry);
			DrawText(listEntry, 0, MIN(maxChars, listEntryLen), oldFontId, oldPenColor);
			if ((!isAlias) && (i == cursorPos)) {
				InvertRect(workerRect);
			}
		}
		workerRect.translate(0, fontSize);
	}

	SetFont(oldFontId);
}

void SciGuiGfx::TexteditCursorDraw (Common::Rect rect, const char *text, uint16 curPos) {
	int16 textWidth, i;
	if (!_texteditCursorVisible) {
		textWidth = 0;
		for (i = 0; i < curPos; i++) {
			textWidth += _font->getCharWidth(text[i]);
		}
		_texteditCursorRect.left = rect.left + textWidth;
		_texteditCursorRect.top = rect.top;
		_texteditCursorRect.bottom = _texteditCursorRect.top + _font->getHeight();
		_texteditCursorRect.right = _texteditCursorRect.left + (text[curPos] == 0 ? 1 : CharWidth(text[curPos]));
		InvertRect(_texteditCursorRect);
		BitsShow(_texteditCursorRect);
		_texteditCursorVisible = true;
		TexteditSetBlinkTime();
	}
}

void SciGuiGfx::TexteditCursorErase() {
	if (_texteditCursorVisible) {
		InvertRect(_texteditCursorRect);
		BitsShow(_texteditCursorRect);
		_texteditCursorVisible = false;
	}
	TexteditSetBlinkTime();
}

void SciGuiGfx::TexteditSetBlinkTime() {
	_texteditBlinkTime = g_system->getMillis() + (30 * 1000 / 60);
}

void SciGuiGfx::TexteditChange(reg_t controlObject, reg_t eventObject) {
	SegManager *segMan = _s->_segMan;
	uint16 cursorPos = GET_SEL32V(controlObject, cursor);
	uint16 maxChars = GET_SEL32V(controlObject, max);
	reg_t textReference = GET_SEL32(controlObject, text);
	Common::String text;
	uint16 textSize, eventType, eventKey;
	bool textChanged = false;

	if (textReference.isNull())
		error("kEditControl called on object that doesnt have a text reference");
	text = segMan->getString(textReference);

	if (!eventObject.isNull()) {
		textSize = text.size();
		eventType = GET_SEL32V(eventObject, type);

		switch (eventType) {
		case SCI_EVT_MOUSE_PRESS:
			// TODO: Implement mouse support for cursor change
			break;
		case SCI_EVT_KEYBOARD:
			eventKey = GET_SEL32V(eventObject, message);
			switch (eventKey) {
			case SCI_K_BACKSPACE:
				if (cursorPos > 0) {
					cursorPos--; text.deleteChar(cursorPos);
					textChanged = true;
				}
				break;
			case SCI_K_DELETE:
				text.deleteChar(cursorPos);
				textChanged = true;
				break;
			case SCI_K_HOME: // HOME
				cursorPos = 0; textChanged = true;
				break;
			case SCI_K_END: // END
				cursorPos = textSize; textChanged = true;
				break;
			case SCI_K_LEFT: // LEFT
				if (cursorPos > 0) {
					cursorPos--; textChanged = true;
				}
			case SCI_K_RIGHT: // RIGHT
				if (cursorPos + 1 <= textSize) {
					cursorPos++; textChanged = true;
				}
				break;
			default:
				if (eventKey > 31 && eventKey < 256 && textSize < maxChars) {
					// insert pressed character
					text.insertChar(eventKey, cursorPos++);
					textChanged = true;
				}
				break;
			}
			break;
		}
	}

	if (textChanged) {
		GuiResourceId oldFontId = GetFontId();
		GuiResourceId fontId = GET_SEL32V(controlObject, font);
		Common::Rect rect;
		rect = Common::Rect (GET_SEL32V(controlObject, nsLeft), GET_SEL32V(controlObject, nsTop),
							  GET_SEL32V(controlObject, nsRight), GET_SEL32V(controlObject, nsBottom));
		rect.top++;
		TexteditCursorErase();
		EraseRect(rect);
		TextBox(text.c_str(), 0, rect, 0, fontId);
		BitsShow(rect);
		SetFont(fontId);
		rect.top--;
		TexteditCursorDraw(rect, text.c_str(), cursorPos);
		SetFont(oldFontId);
		// Write back string
		segMan->strcpy(textReference, text.c_str());
	} else {
		if (g_system->getMillis() >= _texteditBlinkTime) {
			InvertRect(_texteditCursorRect);
			BitsShow(_texteditCursorRect);
			_texteditCursorVisible = !_texteditCursorVisible;
			TexteditSetBlinkTime();
		}
	}

	PUT_SEL32V(controlObject, cursor, cursorPos);
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

void SciGuiGfx::PriorityBandsInit(int16 bandCount, int16 top, int16 bottom) {
	double bandSize;
	int16 y;

	if (bandCount != -1)
		_priorityBandCount = bandCount;

	_priorityTop = top;
	_priorityBottom = bottom;
	bandSize = (_priorityBottom - _priorityTop) / (_priorityBandCount);

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

void SciGuiGfx::ShowPic() {
	// TODO: Implement animations
	warning("ShowPic animation not implemented");
	_palette->setOnScreen();
	_screen->copyToScreen();
	_screen->_picNotValid = 0;
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
	uint16 signal;

	while (curNode) {
		curObject = curNode->value;
		signal = GET_SEL32V(curObject, signal);
		if (!(signal & SCI_ANIMATE_SIGNAL_FROZEN)) {
			// Call .doit method of that object
			invoke_selector(_s, curObject, _s->_kernel->_selectorCache.doit, kContinueOnInvalidSelector, argv, argc, __FILE__, __LINE__, 0);
			// Lookup node again, since the nodetable it was in may have been reallocated
			curNode = _s->_segMan->lookupNode(curAddress);
		}
		curAddress = curNode->succ;
		curNode = _s->_segMan->lookupNode(curAddress);
	}
}

bool AnimateSortHelper(const GuiAnimateEntry* entry1, const GuiAnimateEntry* entry2) {
	return (entry1->y == entry2->y) ? (entry1->z < entry2->z) : (entry1->y < entry2->y);
}

void SciGuiGfx::AnimateMakeSortedList(List *list) {
	SegManager *segMan = _s->_segMan;
	reg_t curAddress = list->first;
	Node *curNode = _s->_segMan->lookupNode(curAddress);
	reg_t curObject;
	GuiAnimateEntry *listEntry;
	int16 listNr, listCount = 0;

	// Count the list entries
	while (curNode) {
		listCount++;
		curAddress = curNode->succ;
		curNode = _s->_segMan->lookupNode(curAddress);
	}

	_animateList.clear();

	// No entries -> exit immediately
	if (listCount == 0)
		return;

	// Adjust list size, if needed
	if ((_animateListData == NULL) || (_animateListSize < listCount)) {
		if (_animateListData)
			free(_animateListData);
		_animateListData = (GuiAnimateEntry *)malloc(listCount * sizeof(GuiAnimateEntry));
		if (!_animateListData)
			error("Could not allocate memory for _animateList");
		_animateListSize = listCount;
	}

	// Fill the list
	curAddress = list->first;
	curNode = _s->_segMan->lookupNode(curAddress);
	listEntry = _animateListData;
	for (listNr = 0; listNr < listCount; listNr++) {
		curObject = curNode->value;
		listEntry->object = curObject;

		// Get data from current object
		listEntry->viewId = GET_SEL32V(curObject, view);
		listEntry->loopNo = GET_SEL32V(curObject, loop);
		listEntry->celNo = GET_SEL32V(curObject, cel);
		listEntry->paletteNo = GET_SEL32V(curObject, palette);
		listEntry->x = GET_SEL32V(curObject, x);
		listEntry->y = GET_SEL32V(curObject, y);
		listEntry->z = GET_SEL32V(curObject, z);
		listEntry->priority = GET_SEL32V(curObject, priority);
		listEntry->signal = GET_SEL32V(curObject, signal);
		// listEntry->celRect is filled in AnimateFill()
		listEntry->showBitsFlag = false;

		_animateList.push_back(listEntry);

		listEntry++;
		curAddress = curNode->succ;
		curNode = _s->_segMan->lookupNode(curAddress);
	}

	// Now sort the list according y and z (descending)
	GuiAnimateList::iterator listBegin = _animateList.begin();
	GuiAnimateList::iterator listEnd = _animateList.end();

	Common::sort(_animateList.begin(), _animateList.end(), AnimateSortHelper);
}

void SciGuiGfx::AnimateFill(byte &old_picNotValid) {
	SegManager *segMan = _s->_segMan;
	reg_t curObject;
	GuiAnimateEntry *listEntry;
	uint16 signal;
	SciGuiView *view = NULL;
	GuiAnimateList::iterator listIterator;
	GuiAnimateList::iterator listEnd = _animateList.end();

	listIterator = _animateList.begin();
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;

		// Get the corresponding view
		view = new SciGuiView(_s->resMan, _screen, _palette, listEntry->viewId);
		
		// adjust loop and cel, if any of those is invalid
		if (listEntry->loopNo >= view->getLoopCount()) {
			listEntry->loopNo = 0;
			PUT_SEL32V(curObject, loop, listEntry->loopNo);
		}
		if (listEntry->celNo >= view->getCelCount(listEntry->loopNo)) {
			listEntry->celNo = 0;
			PUT_SEL32V(curObject, cel, listEntry->celNo);
		}

		// Create rect according to coordinates and given cel
		view->getCelRect(listEntry->loopNo, listEntry->celNo, listEntry->x, listEntry->y, listEntry->z, &listEntry->celRect);
		PUT_SEL32V(curObject, nsLeft, listEntry->celRect.left);
		PUT_SEL32V(curObject, nsTop, listEntry->celRect.top);
		PUT_SEL32V(curObject, nsRight, listEntry->celRect.right);
		PUT_SEL32V(curObject, nsBottom, listEntry->celRect.bottom);

		signal = listEntry->signal;

		// Calculate current priority according to y-coordinate
		if (!(signal & SCI_ANIMATE_SIGNAL_FIXEDPRIORITY)) {
			listEntry->priority = CoordinateToPriority(listEntry->y);
			PUT_SEL32V(curObject, priority, listEntry->priority);
		}
		
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
		listEntry->signal = signal;

		listIterator++;
	}
}

void SciGuiGfx::AnimateUpdate() {
	SegManager *segMan = _s->_segMan;
	reg_t curObject;
	GuiAnimateEntry *listEntry;
	uint16 signal;
	reg_t bitsHandle;
	Common::Rect rect;
	GuiAnimateList::iterator listIterator;
	GuiAnimateList::iterator listBegin = _animateList.begin();
	GuiAnimateList::iterator listEnd = _animateList.end();

	// Remove all previous cels from screen
	listIterator = _animateList.reverse_begin();
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;
		signal = listEntry->signal;

		if (signal & SCI_ANIMATE_SIGNAL_NOUPDATE) {
			if (!(signal & SCI_ANIMATE_SIGNAL_REMOVEVIEW)) {
				bitsHandle = GET_SEL32(curObject, underBits);
				if (_screen->_picNotValid != 1) {
					BitsRestore(bitsHandle);
					listEntry->showBitsFlag = true;
				} else	{
					BitsFree(bitsHandle);
				}
				PUT_SEL32V(curObject, underBits, 0);
			}
			signal &= 0xFFFF ^ SCI_ANIMATE_SIGNAL_FORCEUPDATE;
			signal &= signal & SCI_ANIMATE_SIGNAL_VIEWUPDATED ? 0xFFFF ^ (SCI_ANIMATE_SIGNAL_VIEWUPDATED | SCI_ANIMATE_SIGNAL_NOUPDATE) : 0xFFFF;
		} else if (signal & SCI_ANIMATE_SIGNAL_STOPUPDATE) {
			signal =  (signal & (0xFFFF ^ SCI_ANIMATE_SIGNAL_STOPUPDATE)) | SCI_ANIMATE_SIGNAL_NOUPDATE;
		}
		listEntry->signal = signal;
		listIterator--;
	}

	listIterator = listBegin;
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;
		signal = listEntry->signal;

		if (signal & SCI_ANIMATE_SIGNAL_ALWAYSUPDATE) {
			// draw corresponding cel
			drawCel(listEntry->viewId, listEntry->loopNo, listEntry->celNo, listEntry->celRect, listEntry->priority, listEntry->paletteNo);
			listEntry->showBitsFlag = true;

			signal &= 0xFFFF ^ (SCI_ANIMATE_SIGNAL_STOPUPDATE | SCI_ANIMATE_SIGNAL_VIEWUPDATED | SCI_ANIMATE_SIGNAL_NOUPDATE | SCI_ANIMATE_SIGNAL_FORCEUPDATE);
			if ((signal & SCI_ANIMATE_SIGNAL_IGNOREACTOR) == 0) {
				rect = listEntry->celRect;
				rect.top = CLIP<int16>(PriorityToCoordinate(listEntry->priority) - 1, rect.top, rect.bottom - 1);  
				FillRect(rect, SCI_SCREEN_MASK_CONTROL, 0, 0, 15);
			}
			listEntry->signal = signal;
		}
		listIterator++;
	}

	listIterator = listBegin;
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;
		signal = listEntry->signal;

		if (signal & SCI_ANIMATE_SIGNAL_NOUPDATE) {
			if (signal & SCI_ANIMATE_SIGNAL_HIDDEN) {
				signal |= SCI_ANIMATE_SIGNAL_REMOVEVIEW;
			} else {
				signal &= 0xFFFF ^ SCI_ANIMATE_SIGNAL_REMOVEVIEW;
				if (signal & SCI_ANIMATE_SIGNAL_IGNOREACTOR)
					bitsHandle = BitsSave(listEntry->celRect, SCI_SCREEN_MASK_PRIORITY|SCI_SCREEN_MASK_PRIORITY);
				else
					bitsHandle = BitsSave(listEntry->celRect, SCI_SCREEN_MASK_ALL);
				PUT_SEL32(curObject, underBits, bitsHandle);
			}
			listEntry->signal = signal;
		}
		listIterator++;
	}

	listIterator = listBegin;
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;
		signal = listEntry->signal;

		if (signal & SCI_ANIMATE_SIGNAL_NOUPDATE && !(signal & SCI_ANIMATE_SIGNAL_HIDDEN)) {
			// draw corresponding cel
			drawCel(listEntry->viewId, listEntry->loopNo, listEntry->celNo, listEntry->celRect, listEntry->priority, listEntry->paletteNo);
			listEntry->showBitsFlag = true;

			if ((signal & SCI_ANIMATE_SIGNAL_IGNOREACTOR) == 0) {
				rect = listEntry->celRect;
				rect.top = CLIP<int16>(PriorityToCoordinate(listEntry->priority) - 1, rect.top, rect.bottom - 1);  
				FillRect(rect, SCI_SCREEN_MASK_CONTROL, 0, 0, 15);
			}
		}
		listIterator++;
	}
}

void SciGuiGfx::AnimateDrawCels() {
	SegManager *segMan = _s->_segMan;
	reg_t curObject;
	GuiAnimateEntry *listEntry;
	uint16 signal;
	reg_t bitsHandle;
	GuiAnimateList::iterator listIterator;
	GuiAnimateList::iterator listEnd = _animateList.end();

	listIterator = _animateList.begin();
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;
		signal = listEntry->signal;

		if (!(signal & (SCI_ANIMATE_SIGNAL_NOUPDATE | SCI_ANIMATE_SIGNAL_HIDDEN | SCI_ANIMATE_SIGNAL_ALWAYSUPDATE))) {
			// Save background
			bitsHandle = BitsSave(listEntry->celRect, SCI_SCREEN_MASK_ALL);
			PUT_SEL32(curObject, underBits, bitsHandle);

			// draw corresponding cel
			drawCel(listEntry->viewId, listEntry->loopNo, listEntry->celNo, listEntry->celRect, listEntry->priority, listEntry->paletteNo);
			listEntry->showBitsFlag = true;

			if (signal & SCI_ANIMATE_SIGNAL_REMOVEVIEW) {
				signal &= 0xFFFF ^ SCI_ANIMATE_SIGNAL_REMOVEVIEW;
			}
			listEntry->signal = signal;
			
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
		listIterator++;
	}
}

void SciGuiGfx::AnimateUpdateScreen(byte oldPicNotValid) {
	SegManager *segMan = _s->_segMan;
	GuiAnimateEntry *listEntry;
	uint16 signal;
	GuiAnimateList::iterator listIterator;
	GuiAnimateList::iterator listEnd = _animateList.end();

	listIterator = _animateList.begin();
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		signal = listEntry->signal;

		if (listEntry->showBitsFlag || !(signal & (SCI_ANIMATE_SIGNAL_REMOVEVIEW | SCI_ANIMATE_SIGNAL_NOUPDATE) ||
										!(signal & SCI_ANIMATE_SIGNAL_REMOVEVIEW) && signal & SCI_ANIMATE_SIGNAL_NOUPDATE && oldPicNotValid)) {
// TODO: code finish
//			rect = (Common::Rect *)&cobj[_objOfs[7]];
//			rect1 = (Common::Rect *)&cobj[_objOfs[8]];
//
//			Common::Rect ro(rect->left, rect->top, rect->right, rect->bottom);
//			ro.clip(*rect1);
//
//			if (!ro.isEmpty()) {
//				ro = *rect; 
//				ro.extend(*rect1);
//			} else {
//				_gfx->ShowBits(*rect, _showMap);
//			//	ro = *rect1;
//			//}
//			//*rect  = *rect1;
//			_gfx->ShowBits(ro, _showMap);
			if (signal & SCI_ANIMATE_SIGNAL_HIDDEN) {
				listEntry->signal |= SCI_ANIMATE_SIGNAL_REMOVEVIEW;
			}
		}

		listIterator++;
	}
	_screen->copyToScreen();
}

void SciGuiGfx::AnimateRestoreAndDelete(int argc, reg_t *argv) {
	SegManager *segMan = _s->_segMan;
	reg_t curObject;
	GuiAnimateEntry *listEntry;
	uint16 signal;
	GuiAnimateList::iterator listIterator;
	GuiAnimateList::iterator listEnd = _animateList.end();

	listIterator = _animateList.reverse_begin();
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;
		signal = listEntry->signal;

		// FIXME: this is supposed to go into the loop above (same method)
		if (signal & SCI_ANIMATE_SIGNAL_HIDDEN) {
			signal |= SCI_ANIMATE_SIGNAL_REMOVEVIEW;
		}

		if ((signal & (SCI_ANIMATE_SIGNAL_NOUPDATE | SCI_ANIMATE_SIGNAL_REMOVEVIEW)) == 0) {
			BitsRestore(GET_SEL32(curObject, underBits));
			PUT_SEL32V(curObject, underBits, 0);
		}

		// Finally update signal
		PUT_SEL32V(curObject, signal, signal);

		if (signal & SCI_ANIMATE_SIGNAL_DISPOSEME) {
			// Call .delete_ method of that object
			invoke_selector(_s, curObject, _s->_kernel->_selectorCache.delete_, kContinueOnInvalidSelector, argv, argc, __FILE__, __LINE__, 0);
		}
		listIterator--;
	}
}

void SciGuiGfx::ReAnimate(Common::Rect rect) {
	// TODO: implement ReAnimate
	BitsShow(rect);
}

void SciGuiGfx::AddToPicDrawCels(List *list) {
	reg_t curObject;
	GuiAnimateEntry *listEntry;
	SciGuiView *view = NULL;
	GuiAnimateList::iterator listIterator;
	GuiAnimateList::iterator listEnd = _animateList.end();

	listIterator = _animateList.begin();
	while (listIterator != listEnd) {
		listEntry = *listIterator;
		curObject = listEntry->object;

		if (listEntry->priority == -1)
			listEntry->priority = CoordinateToPriority(listEntry->y);

		// Get the corresponding view
		view = new SciGuiView(_s->resMan, _screen, _palette, listEntry->viewId);

		// Create rect according to coordinates and given cel
		view->getCelRect(listEntry->loopNo, listEntry->celNo, listEntry->x, listEntry->y, listEntry->priority, &listEntry->celRect);

		// draw corresponding cel
		drawCel(listEntry->viewId, listEntry->loopNo, listEntry->celNo, listEntry->celRect, listEntry->priority, listEntry->paletteNo);
		if ((listEntry->signal & SCI_ANIMATE_SIGNAL_IGNOREACTOR) == 0) {
			listEntry->celRect.top = CLIP<int16>(PriorityToCoordinate(listEntry->priority) - 1, listEntry->celRect.top, listEntry->celRect.bottom - 1);
			FillRect(listEntry->celRect, SCI_SCREEN_MASK_CONTROL, 0, 0, 15);
		}

		listIterator++;
	}
}

void SciGuiGfx::AddToPicDrawView(GuiResourceId viewId, GuiViewLoopNo loopNo, GuiViewCelNo celNo, int16 leftPos, int16 topPos, int16 priority, int16 control) {
	SciGuiView *view = NULL;
	Common::Rect celRect;

	view = new SciGuiView(_s->resMan, _screen, _palette, viewId);

	// Create rect according to coordinates and given cel
	view->getCelRect(loopNo, celNo, leftPos, topPos, priority, &celRect);
	drawCel(view, loopNo, celNo, celRect, priority, 0);
}

bool SciGuiGfx::CanBeHereCheckRectList(reg_t checkObject, Common::Rect checkRect, List *list) {
	SegManager *segMan = _s->_segMan;
	reg_t curAddress = list->first;
	Node *curNode = _s->_segMan->lookupNode(curAddress);
	reg_t curObject;
	uint16 signal;
	Common::Rect curRect;

	while (curNode) {
		curObject = curNode->value;
		if (curObject != checkObject) {
			signal = GET_SEL32V(curObject, signal);
			if ((signal & (SCI_ANIMATE_SIGNAL_IGNOREACTOR | SCI_ANIMATE_SIGNAL_REMOVEVIEW | SCI_ANIMATE_SIGNAL_NOUPDATE)) == 0) {
				curRect.left = GET_SEL32V(curObject, brLeft);
				curRect.top = GET_SEL32V(curObject, brTop);
				curRect.right = GET_SEL32V(curObject, brRight);
				curRect.bottom = GET_SEL32V(curObject, brBottom);
				// Check if curRect is within checkRect
				if (curRect.right > checkRect.left && curRect.left < checkRect.right && curRect.bottom > checkRect.top && curRect.top < checkRect.bottom) {
					return false;
				}
			}
		}
		curAddress = curNode->succ;
		curNode = _s->_segMan->lookupNode(curAddress);
	}
	return true;
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
