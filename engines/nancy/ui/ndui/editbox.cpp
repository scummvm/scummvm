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

#include "graphics/font.h"

#include "engines/nancy/font.h"
#include "engines/nancy/graphics.h"

#include "engines/nancy/ui/ndui/editbox.h"
#include "engines/nancy/ui/ndui/panel.h"

namespace Nancy {
namespace UI {
namespace NDUI {

// The caret blinks this often, in seconds
static const float kCaretBlinkTime = 0.53f;

void EditBox::readExtra(Common::SeekableReadStream &stream) {
	_selectedTextColor = stream.readUint32BE();
	_selectionColor = stream.readUint32BE();
	_caretColor = stream.readUint32BE();
	_border = stream.readSint32LE();
}

Common::Rect EditBox::getTextBounds() const {
	Common::Rect bounds = _bounds;
	bounds.grow(-_border);
	return bounds;
}

// Relative to the start of the text
int EditBox::getCharX(int index) const {
	const Graphics::Font *font = _elements[0].getSystemFont();
	return font ? font->getStringWidth(_text.substr(0, index)) : 0;
}

// The caret goes to the nearest gap between two letters
int EditBox::getCharAt(int x) const {
	int textX = x - getTextBounds().left + _scroll;
	for (uint i = 0; i < _text.size(); ++i) {
		if (textX < (getCharX(i) + getCharX(i + 1)) / 2) {
			return i;
		}
	}

	return _text.size();
}

void EditBox::scrollToCaret() {
	int width = getTextBounds().width();
	int caretX = getCharX(_caret);
	if (caretX < _scroll) {
		_scroll = caretX;
	} else if (caretX > _scroll + width) {
		_scroll = caretX - width;
	}
}

void EditBox::moveCaret(int caret, bool select) {
	_caret = CLIP<int>(caret, 0, _text.size());
	if (!select) {
		_selectionStart = _caret;
	}

	scrollToCaret();
	_isCaretShown = true;
	_blinkTime = 0;
	_panel->setDirty();
}

void EditBox::deleteSelection() {
	int start = MIN(_caret, _selectionStart);
	int end = MAX(_caret, _selectionStart);
	_text.erase(start, end - start);
	_caret = _selectionStart = start;
}

void EditBox::handleKey(const Common::KeyState &key) {
	bool select = key.hasFlags(Common::KBD_SHIFT);

	switch (key.keycode) {
	case Common::KEYCODE_LEFT:
		moveCaret(_caret - 1, select);
		return;
	case Common::KEYCODE_RIGHT:
		moveCaret(_caret + 1, select);
		return;
	case Common::KEYCODE_HOME:
		moveCaret(0, select);
		return;
	case Common::KEYCODE_END:
		moveCaret(_text.size(), select);
		return;
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER:
		sendEvent(kEventTextEntered);
		return;
	case Common::KEYCODE_BACKSPACE:
		if (_caret != _selectionStart) {
			deleteSelection();
		} else if (_caret > 0) {
			_text.deleteChar(--_caret);
			_selectionStart = _caret;
		} else {
			return;
		}

		break;
	case Common::KEYCODE_DELETE:
		if (_caret != _selectionStart) {
			deleteSelection();
		} else if (_caret < (int)_text.size()) {
			_text.deleteChar(_caret);
		} else {
			return;
		}

		break;
	default:
		if (key.ascii < 32 || key.ascii > 126) {
			return;
		}

		// Typing replaces the selection
		if (_caret != _selectionStart) {
			deleteSelection();
		}

		_text.insertChar((char)key.ascii, _caret);
		++_caret;
		_selectionStart = _caret;
		break;
	}

	moveCaret(_caret, false);
	sendEvent(kEventTextChanged);
}

bool EditBox::doAction(const Action &action) {
	switch (action.type) {
	case kActionAddText:
		_text += action.text;
		moveCaret(_text.size(), false);
		return true;
	case kActionSetText:
		// A value selects all of the text, so that typing replaces it
		_text = action.text;
		_scroll = 0;
		moveCaret(_text.size(), false);
		if (action.value != 0) {
			_selectionStart = 0;
		}

		return true;
	default:
		return Control::doAction(action);
	}
}

bool EditBox::getValue(int32 eventType, double &value, Common::String &text) const {
	if (eventType != kEventTextEntered && eventType != kEventTextChanged) {
		return false;
	}

	value = 0;
	text = _text;
	return true;
}

bool EditBox::update(float seconds) {
	bool changed = false;
	_blinkTime += seconds;
	if (_blinkTime >= kCaretBlinkTime) {
		_blinkTime = 0;
		_isCaretShown = !_isCaretShown;
		changed = true;
	}

	changed |= _elements[0].blend(_visible ? getState() : kStateHidden, seconds, kControlBlendRate);
	return changed;
}

void EditBox::draw(Graphics::ManagedSurface &surface, const Common::Point &offset) const {
	const Graphics::Font *font = _elements[0].getSystemFont();
	if (!_visible || !font) {
		return;
	}

	Common::Rect bounds = _bounds;
	bounds.translate(offset.x, offset.y);
	drawSprite(surface, 0, bounds);

	Common::Rect textBounds = getTextBounds();
	textBounds.translate(offset.x, offset.y);
	textBounds.clip(Common::Rect(surface.w, surface.h));
	if (textBounds.isEmpty()) {
		return;
	}

	// Everything gets drawn inside the text area, which the text scrolls through
	Graphics::ManagedSurface area(surface, textBounds);
	int x = -_scroll;
	int y = (area.h - font->getFontHeight()) / 2;

	uint32 textColor = _elements[0].getFontColor(getState());
	font->drawString(&area, _text, x, y, getCharX(_text.size()) + 1, argbToColor(area.format, textColor | 0xFF000000));

	if (_caret != _selectionStart) {
		int start = MIN(_caret, _selectionStart);
		int end = MAX(_caret, _selectionStart);
		Common::Rect selection(x + getCharX(start), 0, x + getCharX(end), area.h);
		selection.clip(Common::Rect(area.w, area.h));
		area.fillRect(selection, argbToColor(area.format, _selectionColor));
		font->drawString(&area, _text.substr(start, end - start), x + getCharX(start), y, getCharX(end) - getCharX(start) + 1,
			argbToColor(area.format, _selectedTextColor | 0xFF000000));
	}

	if (_isCaretShown) {
		int caretX = x + getCharX(_caret);
		Common::Rect caret(caretX - 1, 0, caretX + 1, area.h);
		caret.clip(Common::Rect(area.w, area.h));
		area.fillRect(caret, argbToColor(area.format, _caretColor));
	}
}

bool EditBox::onMouseDown(const Common::Point &pos) {
	moveCaret(getCharAt(pos.x), false);
	return true;
}

void EditBox::onMouseMove(const Common::Point &pos) {
	moveCaret(getCharAt(pos.x), true);
}

} // End of namespace NDUI
} // End of namespace UI
} // End of namespace Nancy
