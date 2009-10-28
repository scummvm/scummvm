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
#include "sci/gui/gui_gfx.h"
#include "sci/gui/gui_font.h"
#include "sci/gui/gui_text.h"
#include "sci/gui/gui_controls.h"

namespace Sci {

SciGuiControls::SciGuiControls(SegManager *segMan, SciGuiGfx *gfx, SciGuiText *text)
	: _segMan(segMan), _gfx(gfx), _text(text) {
	init();
}

SciGuiControls::~SciGuiControls() {
}

void SciGuiControls::init() {
	_texteditCursorVisible = false;
}

const char controlListUpArrow[2]	= { 0x18, 0 };
const char controlListDownArrow[2]	= { 0x19, 0 };

void SciGuiControls::drawListControl(Common::Rect rect, reg_t obj, int16 maxChars, int16 count, const char **entries, GuiResourceId fontId, int16 upperPos, int16 cursorPos, bool isAlias) {
	//SegManager *segMan = _s->_segMan;
	Common::Rect workerRect = rect;
	GuiResourceId oldFontId = _text->GetFontId();
	int16 oldPenColor = _gfx->_curPort->penClr;
	uint16 fontSize = 0;
	int16 i;
	const char *listEntry;
	int16 listEntryLen;
	int16 lastYpos;

	// draw basic window
	_gfx->EraseRect(workerRect);
	workerRect.grow(1);
	_gfx->FrameRect(workerRect);

	// draw UP/DOWN arrows
	//  we draw UP arrow one pixel lower than sierra did, because it looks nicer. Also the DOWN arrow has one pixel
	//  line inbetween as well
	workerRect.top++;
	_text->Box(controlListUpArrow, 0, workerRect, SCI_TEXT_ALIGNMENT_CENTER, 0);
	workerRect.top = workerRect.bottom - 10;
	_text->Box(controlListDownArrow, 0, workerRect, SCI_TEXT_ALIGNMENT_CENTER, 0);

	// Draw inner lines
	workerRect.top = rect.top + 9;
	workerRect.bottom -= 10;
	_gfx->FrameRect(workerRect);
	workerRect.grow(-1);

	_text->SetFont(fontId);
	fontSize = _gfx->_curPort->fontHeight;
	_gfx->PenColor(_gfx->_curPort->penClr); _gfx->BackColor(_gfx->_curPort->backClr);
	workerRect.bottom = workerRect.top + 9;
	lastYpos = rect.bottom - fontSize;

	// Write actual text
	for (i = upperPos; i < count; i++) {
		_gfx->EraseRect(workerRect);
		listEntry = entries[i];
		if (listEntry[0]) {
			_gfx->MoveTo(workerRect.left, workerRect.top);
			listEntryLen = strlen(listEntry);
			_text->Draw(listEntry, 0, MIN(maxChars, listEntryLen), oldFontId, oldPenColor);
			if ((!isAlias) && (i == cursorPos)) {
				_gfx->InvertRect(workerRect);
			}
		}
		workerRect.translate(0, fontSize);
		if (workerRect.bottom > lastYpos)
			break;
	}

	_text->SetFont(oldFontId);
}

void SciGuiControls::TexteditCursorDraw (Common::Rect rect, const char *text, uint16 curPos) {
	int16 textWidth, i;
	if (!_texteditCursorVisible) {
		textWidth = 0;
		for (i = 0; i < curPos; i++) {
			textWidth += _text->_font->getCharWidth(text[i]);
		}
		_texteditCursorRect.left = rect.left + textWidth;
		_texteditCursorRect.top = rect.top;
		_texteditCursorRect.bottom = _texteditCursorRect.top + _text->_font->getHeight();
		_texteditCursorRect.right = _texteditCursorRect.left + (text[curPos] == 0 ? 1 : _text->_font->getCharWidth(text[curPos]));
		_gfx->InvertRect(_texteditCursorRect);
		_gfx->BitsShow(_texteditCursorRect);
		_texteditCursorVisible = true;
		TexteditSetBlinkTime();
	}
}

void SciGuiControls::TexteditCursorErase() {
	if (_texteditCursorVisible) {
		_gfx->InvertRect(_texteditCursorRect);
		_gfx->BitsShow(_texteditCursorRect);
		_texteditCursorVisible = false;
	}
	TexteditSetBlinkTime();
}

void SciGuiControls::TexteditSetBlinkTime() {
	_texteditBlinkTime = g_system->getMillis() + (30 * 1000 / 60);
}

void SciGuiControls::TexteditChange(reg_t controlObject, reg_t eventObject) {
	uint16 cursorPos = GET_SEL32V(_segMan, controlObject, cursor);
	uint16 maxChars = GET_SEL32V(_segMan, controlObject, max);
	reg_t textReference = GET_SEL32(_segMan, controlObject, text);
	Common::String text;
	uint16 textSize, eventType, eventKey;
	bool textChanged = false;
	Common::Rect rect;

	if (textReference.isNull())
		error("kEditControl called on object that doesnt have a text reference");
	text = _segMan->getString(textReference);

	if (!eventObject.isNull()) {
		textSize = text.size();
		eventType = GET_SEL32V(_segMan, eventObject, type);

		switch (eventType) {
		case SCI_EVT_MOUSE_PRESS:
			// TODO: Implement mouse support for cursor change
			break;
		case SCI_EVT_KEYBOARD:
			eventKey = GET_SEL32V(_segMan, eventObject, message);
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
				break;
			case SCI_K_RIGHT: // RIGHT
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
		GuiResourceId oldFontId = _text->GetFontId();
		GuiResourceId fontId = GET_SEL32V(_segMan, controlObject, font);
		rect = Common::Rect(GET_SEL32V(_segMan, controlObject, nsLeft), GET_SEL32V(_segMan, controlObject, nsTop),
							  GET_SEL32V(_segMan, controlObject, nsRight), GET_SEL32V(_segMan, controlObject, nsBottom));
		TexteditCursorErase();
		_gfx->EraseRect(rect);
		_text->Box(text.c_str(), 0, rect, SCI_TEXT_ALIGNMENT_LEFT, fontId);
		_gfx->BitsShow(rect);
		_text->SetFont(fontId);
		TexteditCursorDraw(rect, text.c_str(), cursorPos);
		_text->SetFont(oldFontId);
		// Write back string
		_segMan->strcpy(textReference, text.c_str());
	} else {
		if (g_system->getMillis() >= _texteditBlinkTime) {
			_gfx->InvertRect(_texteditCursorRect);
			_gfx->BitsShow(_texteditCursorRect);
			_texteditCursorVisible = !_texteditCursorVisible;
			TexteditSetBlinkTime();
		}
	}

	PUT_SEL32V(_segMan, controlObject, cursor, cursorPos);
}

} // End of namespace Sci
