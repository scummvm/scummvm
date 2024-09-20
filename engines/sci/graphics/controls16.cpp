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

#include "common/util.h"
#include "common/stack.h"
#include "common/system.h"
#include "common/unicode-bidi.h"
#include "graphics/primitives.h"

#include "sci/sci.h"
#include "sci/event.h"
#include "sci/engine/kernel.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/engine/tts.h"
#include "sci/graphics/compare.h"
#include "sci/graphics/gfxdrivers.h"
#include "sci/graphics/ports.h"
#include "sci/graphics/paint16.h"
#include "sci/graphics/scifont.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/text16.h"
#include "sci/graphics/controls16.h"

namespace Sci {

GfxControls16::GfxControls16(SegManager *segMan, GfxPorts *ports, GfxPaint16 *paint16, GfxText16 *text16, GfxScreen *screen)
	: _segMan(segMan), _ports(ports), _paint16(paint16), _text16(text16), _screen(screen) {
	_texteditBlinkTime = 0;
	_texteditCursorVisible = false;
}

GfxControls16::~GfxControls16() {
}

const char controlListUpArrow[2]	= { 0x18, 0 };
const char controlListDownArrow[2]	= { 0x19, 0 };

void GfxControls16::drawListControl(Common::Rect rect, reg_t obj, int16 maxChars, int16 count, const Common::String *entries, GuiResourceId fontId, int16 upperPos, int16 cursorPos, bool isAlias) {
	Common::Rect workerRect = rect;
	GuiResourceId oldFontId = _text16->GetFontId();
	int16 oldPenColor = _ports->_curPort->penClr;
	uint16 fontSize = 0;
	int16 i;
	int16 lastYpos;

	// draw basic window
	_paint16->eraseRect(workerRect);
	workerRect.grow(1);
	_paint16->frameRect(workerRect);

	// draw UP/DOWN arrows
	//  we draw UP arrow one pixel lower than sierra did, because it looks nicer. Also the DOWN arrow has one pixel
	//  line inbetween as well
	// They "fixed" this in SQ4 by having the arrow character start one pixel line later, we don't adjust there
	if (g_sci->getGameId() != GID_SQ4)
		workerRect.top++;
	_text16->Box(controlListUpArrow, false, workerRect, SCI_TEXT16_ALIGNMENT_CENTER, 0);
	workerRect.top = workerRect.bottom - 10;
	_text16->Box(controlListDownArrow, false, workerRect, SCI_TEXT16_ALIGNMENT_CENTER, 0);

	// Draw inner lines
	workerRect.top = rect.top + 9;
	workerRect.bottom -= 10;
	_paint16->frameRect(workerRect);
	workerRect.grow(-1);

	_text16->SetFont(fontId);
	fontSize = _ports->_curPort->fontHeight;
	_ports->penColor(_ports->_curPort->penClr); _ports->backColor(_ports->_curPort->backClr);
	workerRect.bottom = workerRect.top + fontSize;
	lastYpos = rect.bottom - fontSize;

	// Write actual text
	for (i = upperPos; i < count; i++) {
		_paint16->eraseRect(workerRect);
		const Common::String &listEntry = entries[i];
		if (listEntry[0]) {
			Common::String textString = listEntry;
			if (g_sci->isLanguageRTL())
				textString = Common::convertBiDiString(textString, g_sci->getLanguage());

			if (!g_sci->isLanguageRTL())
				_ports->moveTo(workerRect.left, workerRect.top);
			else {
				// calc width, for right alignment
				const char *textPtr = textString.c_str();
				uint16 textWidth = 0;
				while (*textPtr)
					textWidth += _text16->_font->getCharWidth((byte)*textPtr++);
				_ports->moveTo(workerRect.right - textWidth - 1, workerRect.top);
			}
			_text16->Draw(textString.c_str(), 0, MIN<int16>(maxChars, listEntry.size()), oldFontId, oldPenColor);
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

void GfxControls16::texteditCursorDraw(Common::Rect rect, const char *text, uint16 curPos) {
	if (!_texteditCursorVisible) {
		int16 textWidth = 0;
		for (int16 i = 0; i < curPos; i++) {
			textWidth += _text16->_font->getCharWidth((unsigned char)text[i]);
		}
		if (!g_sci->isLanguageRTL())
			_texteditCursorRect.left = rect.left + textWidth;
		else
			_texteditCursorRect.right = rect.right - textWidth;
		_texteditCursorRect.top = rect.top;
		_texteditCursorRect.bottom = _texteditCursorRect.top + _text16->_font->getHeight();
		if (!g_sci->isLanguageRTL())
			_texteditCursorRect.right = _texteditCursorRect.left + (text[curPos] == 0 ? 1 : _text16->_font->getCharWidth((unsigned char)text[curPos]));
		else
			_texteditCursorRect.left = _texteditCursorRect.right - (text[curPos] == 0 ? 1 : _text16->_font->getCharWidth((unsigned char)text[curPos]));
		_paint16->invertRect(_texteditCursorRect);
		_paint16->bitsShow(_texteditCursorRect);
		_texteditCursorVisible = true;
		texteditSetBlinkTime();
	}
}

void GfxControls16::texteditCursorErase() {
	if (_texteditCursorVisible) {
		_paint16->invertRect(_texteditCursorRect);
		_paint16->bitsShow(_texteditCursorRect);
		_texteditCursorVisible = false;
	}
	texteditSetBlinkTime();
}

void GfxControls16::texteditSetBlinkTime() {
	_texteditBlinkTime = g_system->getMillis() + (30 * 1000 / 60);
}

void GfxControls16::kernelTexteditChange(reg_t controlObject, reg_t eventObject) {
	uint16 cursorPos = readSelectorValue(_segMan, controlObject, SELECTOR(cursor));
	uint16 maxChars = readSelectorValue(_segMan, controlObject, SELECTOR(max));
	reg_t textReference = readSelector(_segMan, controlObject, SELECTOR(text));
	Common::String text;
	uint16 eventKey = 0, modifiers = 0;
	bool textChanged = false;
	bool textAddChar = false;
	Common::Rect rect;

	if (textReference.isNull())
		error("kEditControl called on object that doesn't have a text reference");
	text = _segMan->getString(textReference);

	uint16 oldCursorPos = cursorPos;

	if (!eventObject.isNull()) {
		uint16 textSize = text.size();
		uint16 eventType = readSelectorValue(_segMan, eventObject, SELECTOR(type));

		switch (eventType) {
		case kSciEventMousePress:
			// TODO: Implement mouse support for cursor change
			break;
		case kSciEventKeyDown:
			eventKey = readSelectorValue(_segMan, eventObject, SELECTOR(message));
			modifiers = readSelectorValue(_segMan, eventObject, SELECTOR(modifiers));
			switch (eventKey) {
			case kSciKeyBackspace:
				if (cursorPos > 0) {
					cursorPos--; text.deleteChar(cursorPos);
					textChanged = true;
				}
				break;
			case kSciKeyDelete:
				if (cursorPos < textSize) {
					text.deleteChar(cursorPos);
					textChanged = true;
				}
				break;
			case kSciKeyHome:
				cursorPos = 0; textChanged = true;
				break;
			case kSciKeyEnd:
				cursorPos = textSize; textChanged = true;
				break;
			case kSciKeyLeft:
				if (!g_sci->isLanguageRTL()) {
					if (cursorPos > 0) {
						cursorPos--; textChanged = true;
					}
				} else {
					if (cursorPos + 1 <= textSize) {
						cursorPos++; textChanged = true;
					}
				}
				break;
			case kSciKeyRight:
				if (!g_sci->isLanguageRTL()) {
					if (cursorPos + 1 <= textSize) {
						cursorPos++; textChanged = true;
					}
				} else {
					if (cursorPos > 0) {
						cursorPos--; textChanged = true;
					}
				}
				break;
			case kSciKeyEtx:
				if (modifiers & kSciKeyModCtrl) {
					// Control-C erases the whole line
					cursorPos = 0; text.clear();
					textChanged = true;
				}
				break;
			default:
				if ((modifiers & kSciKeyModCtrl) && eventKey == 99) {
					// Control-C in earlier SCI games (SCI0 - SCI1 middle)
					// Control-C erases the whole line
					cursorPos = 0; text.clear();
					textChanged = true;
				} else if (eventKey > 31 && eventKey < 256 && textSize < maxChars) {
					// insert pressed character
					textAddChar = true;
					textChanged = true;
				}
				break;
			}
			break;
		default:
			break;
		}
	}

	if (g_sci->getVocabulary() && !textChanged && oldCursorPos != cursorPos) {
		assert(!textAddChar);
		textChanged = g_sci->getVocabulary()->checkAltInput(text, cursorPos);
	}

	if (textChanged) {
		GuiResourceId oldFontId = _text16->GetFontId();
		GuiResourceId fontId = readSelectorValue(_segMan, controlObject, SELECTOR(font));
		rect = g_sci->_gfxCompare->getNSRect(controlObject);

		_text16->SetFont(fontId);
		if (textAddChar) {

			const char *textPtr = text.c_str();

			// We check if we are really able to add the new char
			uint16 textWidth = 0;
			while (*textPtr)
				textWidth += _text16->_font->getCharWidth((byte)*textPtr++);
			textWidth += _text16->_font->getCharWidth(eventKey);

			// Does it fit?
			if (textWidth >= rect.width()) {
				_text16->SetFont(oldFontId);
				return;
			}

			text.insertChar(eventKey, cursorPos++);

			// Note: the following checkAltInput call might make the text
			// too wide to fit, but SSCI fails to check that too.
		}
		if (g_sci->getVocabulary())
			g_sci->getVocabulary()->checkAltInput(text, cursorPos);
		texteditCursorErase();
		_paint16->eraseRect(rect);
		_text16->Box(text.c_str(), false, rect, SCI_TEXT16_ALIGNMENT_LEFT, -1);
		_paint16->bitsShow(rect);
		texteditCursorDraw(rect, text.c_str(), cursorPos);
		_text16->SetFont(oldFontId);
		// Write back string
		_segMan->strcpy_(textReference, text.c_str());
	} else {
		if (g_system->getMillis() >= _texteditBlinkTime) {
			_paint16->invertRect(_texteditCursorRect);
			_paint16->bitsShow(_texteditCursorRect);
			_texteditCursorVisible = !_texteditCursorVisible;
			texteditSetBlinkTime();
		}
	}

	writeSelectorValue(_segMan, controlObject, SELECTOR(cursor), cursorPos);
}

int GfxControls16::getPicNotValid() {
	if (getSciVersion() >= SCI_VERSION_1_1)
		return _screen->_picNotValidSci11;
	return _screen->_picNotValid;
}

void GfxControls16::kernelDrawButton(Common::Rect rect, reg_t obj, const char *text, uint16 languageSplitter, int16 fontId, int16 style, bool hilite) {
	g_sci->_tts->button(text);

	if (!hilite) {
		int16 sci0EarlyPen = 0, sci0EarlyBack = 0;
		if (getSciVersion() == SCI_VERSION_0_EARLY) {
			// SCI0early actually used hardcoded green/black buttons instead of using the port colors
			sci0EarlyPen = _ports->_curPort->penClr;
			sci0EarlyBack = _ports->_curPort->backClr;
			_ports->penColor(0);
			_ports->backColor(2);
		}
		rect.grow(1);
		_paint16->eraseRect(rect);
		_paint16->frameRect(rect);

		// Unlike PC-98, the Korean fan translations have CJK text for some button controls. The original PC-98
		// interpreters which were used to make the necessary code changes to kernelDrawText do not have any
		// modifications for button controls, since it is not necessary (due to the English button labels). I
		// have now tried to adapt the code changes from kernelDrawText for the button controls. It does require
		// some extra attention, like drawing the buttons frames first, but seems to work as intended. Also, the
		// different handling also seems to work fine for the English buttons (which both the Korean and the PC-98
		// versions have).
		if (_screen->gfxDriver()->driverBasedTextRendering() && !getPicNotValid()) {
			if (style & SCI_CONTROLS_STYLE_SELECTED) {
				rect.grow(-1);
				_paint16->frameRect(rect);
				rect.grow(1);
			}
			_paint16->bitsShow(rect);
		}

		rect.grow(-2);
		_ports->textGreyedOutput(!(style & SCI_CONTROLS_STYLE_ENABLED));

		if (!g_sci->hasMacFonts()) {
			_text16->Box(text, languageSplitter, _screen->gfxDriver()->driverBasedTextRendering(), rect, SCI_TEXT16_ALIGNMENT_CENTER, fontId);
		} else {
			_text16->macDraw(text, rect, SCI_TEXT16_ALIGNMENT_CENTER, fontId, _text16->GetFontId(), 0);
		}
		_ports->textGreyedOutput(false);

		// Fix for Korean fan translation, see comment above.
		if (!_screen->gfxDriver()->driverBasedTextRendering()) {
			rect.grow(1);
			if (style & SCI_CONTROLS_STYLE_SELECTED)
				_paint16->frameRect(rect);
			if (!getPicNotValid()) {
				rect.grow(1);
				_paint16->bitsShow(rect);
			}
		}

		if (getSciVersion() == SCI_VERSION_0_EARLY) {
			_ports->penColor(sci0EarlyPen);
			_ports->backColor(sci0EarlyBack);
		}
	} else {
		// SCI0early used xor to invert button rectangles resulting in pink/white buttons
		// All PC-98 targets (both SCI_VERSION_01 and SCI_VERSION_1_LATE) also use the
		// xor method, resulting in a grey color.
		if (getSciVersion() == SCI_VERSION_0_EARLY || g_sci->getPlatform() == Common::kPlatformPC98)
			_paint16->invertRectViaXOR(rect);
		else
			_paint16->invertRect(rect);
		if (g_sci->hasMacFonts()) {
			// Mac scripts set a flag to tell the interpreter to draw white text when inverted.
			// Note that KQ6 does not do this because it includes the PC version of the script,
			// causing button text to disappear when clicked in the original.
			uint16 textColor = (style & SCI_CONTROLS_STYLE_MAC_INVERTED) ? 255 : 0;
			rect.grow(-1);
			_text16->macDraw(text, rect, SCI_TEXT16_ALIGNMENT_CENTER, fontId, _text16->GetFontId(), textColor);
			rect.grow(1);
		}
		_paint16->bitsShow(rect);
	}
}

void GfxControls16::kernelDrawText(Common::Rect rect, reg_t obj, const char *text, uint16 languageSplitter, int16 fontId, TextAlignment alignment, int16 style, bool hilite) {
	g_sci->_tts->text(text);

	if (!hilite) {
		rect.grow(1);
		_paint16->eraseRect(rect);
		rect.grow(-1);
		if (!g_sci->hasMacFonts()) {
			// The PC-98 versions set the 'show` argument here (unlike normal DOS versions).
			_text16->Box(text, languageSplitter, _screen->gfxDriver()->driverBasedTextRendering(), rect, alignment, fontId);
		} else {
			_text16->macDraw(text, rect, alignment, fontId, _text16->GetFontId(), 0);
		}
		if (style & SCI_CONTROLS_STYLE_SELECTED) {
			_paint16->frameRect(rect);
		}

		// I have checked the PC-98 versions of QFG1 and KQ5. These set all rect bounds for the
		// screen update rect to 0 after the text drawing. So nothing gets updated on screen.
		// Otherwise, it would just overdraw the hi-res text. I have looked at the DOS version of
		// QFG1 for comparison. There, it copies the text box rect into the screen update rect.
		// So this specific handling for the PC-98 versions is correct.
		bool allowScreenUpdate = _screen->gfxDriver()->driverBasedTextRendering() ? false : true;

		if (allowScreenUpdate && !getPicNotValid())
			_paint16->bitsShow(rect);
	} else {
		// SCI0early used xor to invert button rectangles resulting in pink/white buttons
		// All PC-98 targets (both SCI_VERSION_01 and SCI_VERSION_1_LATE) also use the
		// xor method, resulting in a grey color.
		if (getSciVersion() == SCI_VERSION_0_EARLY || g_sci->getPlatform() == Common::kPlatformPC98)
			_paint16->invertRectViaXOR(rect);
		else
			_paint16->invertRect(rect);
		_paint16->bitsShow(rect);
	}
}

void GfxControls16::kernelDrawTextEdit(Common::Rect rect, reg_t obj, const char *text, uint16 languageSplitter, int16 fontId, int16 mode, int16 style, int16 cursorPos, int16 maxChars, bool hilite) {
	Common::Rect textRect = rect;
	uint16 oldFontId = _text16->GetFontId();

	rect.grow(1);
	_texteditCursorVisible = false;
	texteditCursorErase();
	_paint16->eraseRect(rect);
	_text16->Box(text, languageSplitter, false, textRect, SCI_TEXT16_ALIGNMENT_LEFT, fontId);
	_paint16->frameRect(rect);
	if (style & SCI_CONTROLS_STYLE_SELECTED) {
		_text16->SetFont(fontId);
		rect.grow(-1);
		texteditCursorDraw(rect, text, cursorPos);
		_text16->SetFont(oldFontId);
		rect.grow(1);
	}
	if (!getPicNotValid())
		_paint16->bitsShow(rect);
}

void GfxControls16::kernelDrawIcon(Common::Rect rect, reg_t obj, GuiResourceId viewId, int16 loopNo, int16 celNo, int16 priority, int16 style, bool hilite) {
	if (!hilite) {
		_paint16->drawCelAndShow(viewId, loopNo, celNo, rect.left, rect.top, priority, 0);
		if (style & 0x20) {
			_paint16->frameRect(rect);
		}
		if (!getPicNotValid())
			_paint16->bitsShow(rect);
	} else {
		// SCI0early used xor to invert button rectangles resulting in pink/white buttons
		// All PC-98 targets (both SCI_VERSION_01 and SCI_VERSION_1_LATE) also use the
		// xor method, resulting in a grey color.
		if (getSciVersion() == SCI_VERSION_0_EARLY || g_sci->getPlatform() == Common::kPlatformPC98)
			_paint16->invertRectViaXOR(rect);
		else
			_paint16->invertRect(rect);
		_paint16->bitsShow(rect);
	}
}

void GfxControls16::kernelDrawList(Common::Rect rect, reg_t obj, int16 maxChars, int16 count, const Common::String *entries, GuiResourceId fontId, int16 style, int16 upperPos, int16 cursorPos, bool isAlias, bool hilite) {
	if (!hilite) {
		drawListControl(rect, obj, maxChars, count, entries, fontId, upperPos, cursorPos, isAlias);
		rect.grow(1);
		if (isAlias && (style & SCI_CONTROLS_STYLE_SELECTED)) {
			_paint16->frameRect(rect);
		}
		if (!getPicNotValid())
			_paint16->bitsShow(rect);
	}
}

} // End of namespace Sci
