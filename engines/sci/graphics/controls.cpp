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
#include "common/system.h"
#include "graphics/primitives.h"

#include "sci/sci.h"
#include "sci/event.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/graphics/ports.h"
#include "sci/graphics/paint16.h"
#include "sci/graphics/font.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/text16.h"
#include "sci/graphics/controls.h"

namespace Sci {

GfxControls::GfxControls(SegManager *segMan, GfxPorts *ports, GfxPaint16 *paint16, GfxText16 *text16, GfxScreen *screen)
	: _segMan(segMan), _ports(ports), _paint16(paint16), _text16(text16), _screen(screen) {
	init();
}

GfxControls::~GfxControls() {
}

void GfxControls::init() {
	_texteditCursorVisible = false;
}

const char controlListUpArrow[2]	= { 0x18, 0 };
const char controlListDownArrow[2]	= { 0x19, 0 };

void GfxControls::drawListControl(Common::Rect rect, reg_t obj, int16 maxChars, int16 count, const char **entries, GuiResourceId fontId, int16 upperPos, int16 cursorPos, bool isAlias) {
	Common::Rect workerRect = rect;
	GuiResourceId oldFontId = _text16->GetFontId();
	int16 oldPenColor = _ports->_curPort->penClr;
	uint16 fontSize = 0;
	int16 i;
	const char *listEntry;
	int16 listEntryLen;
	int16 lastYpos;

	// draw basic window
	_paint16->eraseRect(workerRect);
	workerRect.grow(1);
	_paint16->frameRect(workerRect);

	// draw UP/DOWN arrows
	//  we draw UP arrow one pixel lower than sierra did, because it looks nicer. Also the DOWN arrow has one pixel
	//  line inbetween as well
	workerRect.top++;
	_text16->Box(controlListUpArrow, 0, workerRect, SCI_TEXT16_ALIGNMENT_CENTER, 0);
	workerRect.top = workerRect.bottom - 10;
	_text16->Box(controlListDownArrow, 0, workerRect, SCI_TEXT16_ALIGNMENT_CENTER, 0);

	// Draw inner lines
	workerRect.top = rect.top + 9;
	workerRect.bottom -= 10;
	_paint16->frameRect(workerRect);
	workerRect.grow(-1);

	_text16->SetFont(fontId);
	fontSize = _ports->_curPort->fontHeight;
	_ports->penColor(_ports->_curPort->penClr); _ports->backColor(_ports->_curPort->backClr);
	workerRect.bottom = workerRect.top + 9;
	lastYpos = rect.bottom - fontSize;

	// Write actual text
	for (i = upperPos; i < count; i++) {
		_paint16->eraseRect(workerRect);
		listEntry = entries[i];
		if (listEntry[0]) {
			_ports->moveTo(workerRect.left, workerRect.top);
			listEntryLen = strlen(listEntry);
			_text16->Draw(listEntry, 0, MIN(maxChars, listEntryLen), oldFontId, oldPenColor);
			if ((!isAlias) && (i == cursorPos)) {
				_paint16->invertRect(workerRect);
			}
		}
		workerRect.translate(0, fontSize);
		if (workerRect.bottom > lastYpos)
			break;
	}

	_text16->SetFont(oldFontId);
}

void GfxControls::texteditCursorDraw(Common::Rect rect, const char *text, uint16 curPos) {
	int16 textWidth, i;
	if (!_texteditCursorVisible) {
		textWidth = 0;
		for (i = 0; i < curPos; i++) {
			textWidth += _text16->_font->getCharWidth((unsigned char)text[i]);
		}
		_texteditCursorRect.left = rect.left + textWidth;
		_texteditCursorRect.top = rect.top;
		_texteditCursorRect.bottom = _texteditCursorRect.top + _text16->_font->getHeight();
		_texteditCursorRect.right = _texteditCursorRect.left + (text[curPos] == 0 ? 1 : _text16->_font->getCharWidth((unsigned char)text[curPos]));
		_paint16->invertRect(_texteditCursorRect);
		_paint16->bitsShow(_texteditCursorRect);
		_texteditCursorVisible = true;
		texteditSetBlinkTime();
	}
}

void GfxControls::texteditCursorErase() {
	if (_texteditCursorVisible) {
		_paint16->invertRect(_texteditCursorRect);
		_paint16->bitsShow(_texteditCursorRect);
		_texteditCursorVisible = false;
	}
	texteditSetBlinkTime();
}

void GfxControls::texteditSetBlinkTime() {
	_texteditBlinkTime = g_system->getMillis() + (30 * 1000 / 60);
}

void GfxControls::kernelTexteditChange(reg_t controlObject, reg_t eventObject) {
	uint16 cursorPos = GET_SEL32V(_segMan, controlObject, SELECTOR(cursor));
	uint16 maxChars = GET_SEL32V(_segMan, controlObject, SELECTOR(max));
	reg_t textReference = GET_SEL32(_segMan, controlObject, SELECTOR(text));
	Common::String text;
	uint16 textSize, eventType, eventKey;
	bool textChanged = false;
	Common::Rect rect;

	if (textReference.isNull())
		error("kEditControl called on object that doesnt have a text reference");
	text = _segMan->getString(textReference);

	if (!eventObject.isNull()) {
		textSize = text.size();
		eventType = GET_SEL32V(_segMan, eventObject, SELECTOR(type));

		switch (eventType) {
		case SCI_EVENT_MOUSE_PRESS:
			// TODO: Implement mouse support for cursor change
			break;
		case SCI_EVENT_KEYBOARD:
			eventKey = GET_SEL32V(_segMan, eventObject, SELECTOR(message));
			switch (eventKey) {
			case SCI_KEY_BACKSPACE:
				if (cursorPos > 0) {
					cursorPos--; text.deleteChar(cursorPos);
					textChanged = true;
				}
				break;
			case SCI_KEY_DELETE:
				text.deleteChar(cursorPos);
				textChanged = true;
				break;
			case SCI_KEY_HOME: // HOME
				cursorPos = 0; textChanged = true;
				break;
			case SCI_KEY_END: // END
				cursorPos = textSize; textChanged = true;
				break;
			case SCI_KEY_LEFT: // LEFT
				if (cursorPos > 0) {
					cursorPos--; textChanged = true;
				}
				break;
			case SCI_KEY_RIGHT: // RIGHT
				if (cursorPos + 1 <= textSize) {
					cursorPos++; textChanged = true;
				}
				break;
			default:
				if (eventKey > 31 && eventKey < 256 && textSize < maxChars) {
					// insert pressed character
					// we check, if there is space left for this character

					text.insertChar(eventKey, cursorPos++);
					textChanged = true;
				}
				break;
			}
			break;
		}
	}

	if (textChanged) {
		GuiResourceId oldFontId = _text16->GetFontId();
		GuiResourceId fontId = GET_SEL32V(_segMan, controlObject, SELECTOR(font));
		rect = Common::Rect(GET_SEL32V(_segMan, controlObject, SELECTOR(nsLeft)), GET_SEL32V(_segMan, controlObject, SELECTOR(nsTop)),
							  GET_SEL32V(_segMan, controlObject, SELECTOR(nsRight)), GET_SEL32V(_segMan, controlObject, SELECTOR(nsBottom)));
		texteditCursorErase();
		_paint16->eraseRect(rect);
		_text16->Box(text.c_str(), 0, rect, SCI_TEXT16_ALIGNMENT_LEFT, fontId);
		_paint16->bitsShow(rect);
		_text16->SetFont(fontId);
		texteditCursorDraw(rect, text.c_str(), cursorPos);
		_text16->SetFont(oldFontId);
		// Write back string
		_segMan->strcpy(textReference, text.c_str());
	} else {
		if (g_system->getMillis() >= _texteditBlinkTime) {
			_paint16->invertRect(_texteditCursorRect);
			_paint16->bitsShow(_texteditCursorRect);
			_texteditCursorVisible = !_texteditCursorVisible;
			texteditSetBlinkTime();
		}
	}

	PUT_SEL32V(_segMan, controlObject, SELECTOR(cursor), cursorPos);
}

int GfxControls::getPicNotValid() {
	if (getSciVersion() >= SCI_VERSION_1_1)
		return _screen->_picNotValidSci11;
	return _screen->_picNotValid;
}

void GfxControls::kernelDrawButton(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 style, bool hilite) {
	if (!hilite) {
		rect.grow(1);
		_paint16->eraseRect(rect);
		_paint16->frameRect(rect);
		rect.grow(-2);
		_ports->textGreyedOutput(style & 1 ? false : true);
		_text16->Box(text, 0, rect, SCI_TEXT16_ALIGNMENT_CENTER, fontId);
		_ports->textGreyedOutput(false);
		rect.grow(1);
		if (style & 8) // selected
			_paint16->frameRect(rect);
		if (!getPicNotValid()) {
			rect.grow(1);
			_paint16->bitsShow(rect);
		}
	} else {
		_paint16->invertRect(rect);
		_paint16->bitsShow(rect);
	}
}

void GfxControls::kernelDrawText(Common::Rect rect, reg_t obj, const char *text, int16 fontId, TextAlignment alignment, int16 style, bool hilite) {
	if (!hilite) {
		rect.grow(1);
		_paint16->eraseRect(rect);
		rect.grow(-1);
		_text16->Box(text, 0, rect, alignment, fontId);
		if (style & 8) { // selected
			_paint16->frameRect(rect);
		}
		rect.grow(1);
		if (!getPicNotValid())
			_paint16->bitsShow(rect);
	} else {
		_paint16->invertRect(rect);
		_paint16->bitsShow(rect);
	}
}

void GfxControls::kernelDrawTextEdit(Common::Rect rect, reg_t obj, const char *text, int16 fontId, int16 mode, int16 style, int16 cursorPos, int16 maxChars, bool hilite) {
	Common::Rect textRect = rect;
	uint16 oldFontId = _text16->GetFontId();

	rect.grow(1);
	_texteditCursorVisible = false;
	texteditCursorErase();
	_paint16->eraseRect(rect);
	_text16->Box(text, 0, textRect, SCI_TEXT16_ALIGNMENT_LEFT, fontId);
	_paint16->frameRect(rect);
	if (style & 8) {
		_text16->SetFont(fontId);
		rect.grow(-1);
		texteditCursorDraw(rect, text, cursorPos);
		_text16->SetFont(oldFontId);
		rect.grow(1);
	}
	if (!getPicNotValid())
		_paint16->bitsShow(rect);
}

void GfxControls::kernelDrawIcon(Common::Rect rect, reg_t obj, GuiResourceId viewId, int16 loopNo, int16 celNo, int16 priority, int16 style, bool hilite) {
	if (!hilite) {
		_paint16->drawCelAndShow(viewId, loopNo, celNo, rect.left, rect.top, priority, 0);
		if (style & 0x20) {
			_paint16->frameRect(rect);
		}
		if (!getPicNotValid())
			_paint16->bitsShow(rect);
	} else {
		_paint16->invertRect(rect);
		_paint16->bitsShow(rect);
	}
}

void GfxControls::kernelDrawList(Common::Rect rect, reg_t obj, int16 maxChars, int16 count, const char **entries, GuiResourceId fontId, int16 style, int16 upperPos, int16 cursorPos, bool isAlias, bool hilite) {
	if (!hilite) {
		drawListControl(rect, obj, maxChars, count, entries, fontId, upperPos, cursorPos, isAlias);
		rect.grow(1);
		if (isAlias && (style & 8)) {
			_paint16->frameRect(rect);
		}
		if (!getPicNotValid())
			_paint16->bitsShow(rect);
	}
}

} // End of namespace Sci
