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

#include "common/system.h"
#include "common/events.h"

#include "engines/engine.h"

#include "graphics/surface.h"

#include "scumm/macgui/macgui_impl.h"

namespace Scumm {

// ---------------------------------------------------------------------------
// Widgets imitating the look and feel of an 80s Macintosh
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Base widget
// ---------------------------------------------------------------------------

MacGuiImpl::MacWidget::MacWidget(MacGuiImpl::MacDialogWindow *window, Common::Rect bounds, Common::String text, bool enabled) : MacGuiObject(bounds, enabled), _window(window), _text(text) {
	// Widgets are clipped to the inner surface of the dialog. If a widget
	// is clipped out of existence, make it invisible to avoid crashes.

	Graphics::Surface *s = _window->innerSurface();

	_bounds.clip(Common::Rect(s->w, s->h));

	if (_bounds.width() <= 0 || _bounds.height() <= 0)
		_visible = false;
}

Common::String MacGuiImpl::MacWidget::getText() const {
	Common::String temp = Common::U32String(_text, Common::kMacRoman).encode(Common::kUtf8);
	return temp;
}

bool MacGuiImpl::MacWidget::findWidget(int x, int y) const {
	return _enabled && _bounds.contains(x, y);
}

void MacGuiImpl::MacWidget::setRedraw(bool fullRedraw) {
	if (fullRedraw)
		_fullRedraw = true;
	else
		_redraw = true;
}

void MacGuiImpl::MacWidget::setEnabled(bool enabled) {
	_enabled = enabled;
	setRedraw(true);
}

void MacGuiImpl::MacWidget::setValue(int value) {
	_value = value;
	setRedraw();
}

void MacGuiImpl::MacWidget::drawBitmap(Common::Rect r, const uint16 *bitmap, Color color) const {
	_window->_gui->drawBitmap(_window->innerSurface(), r, bitmap, color);
}

int MacGuiImpl::MacWidget::drawText(Common::String text, int x, int y, int w, Color fg, Color bg, Graphics::TextAlign align, bool wordWrap, int deltax) const {
	if (text.empty())
		return 0;

	const Graphics::Font *font = _window->_gui->getFont(kSystemFont);

	// Apply text substitutions

	for (uint i = 0; i < text.size() - 1; i++) {
		if (text[i] == '^') {
			uint nr = text[i + 1] - '0';
			if (_window->hasSubstitution(nr)) {
				Common::String &subst = _window->getSubstitution(nr);
				text.replace(i, 2, subst);
			}
		}
	}

	// Word-wrap text

	Common::StringArray lines;
	int maxLineWidth = 0;

	if (wordWrap) {
		maxLineWidth = font->wordWrapText(text, w, lines);
	} else {
		lines.push_back(text);
		maxLineWidth = font->getStringWidth(text);
	}

	// Draw the text. Disabled text is implemented as a filter on top of
	// the already drawn text.

	int y0 = y;

	for (uint i = 0; i < lines.size(); i++) {
		font->drawString(_window->innerSurface(), lines[i], x, y0, w, fg, align, deltax);

		if (!_enabled) {
			Common::Rect textBox = font->getBoundingBox(lines[i], x, y0, w, align);

			for (int y1 = textBox.top; y1 < textBox.bottom; y1++) {
				for (int x1 = textBox.left; x1 < textBox.right; x1++) {
					if (((x1 + y1) % 2) == 0)
						_window->innerSurface()->setPixel(x1, y1, bg);
				}
			}
		}

		y0 += font->getFontHeight();
	}

	return maxLineWidth;
}

// ---------------------------------------------------------------------------
// Button widget
// ---------------------------------------------------------------------------

void MacGuiImpl::MacButton::draw(bool drawFocused) {
	if (!_redraw && !_fullRedraw)
		return;

	debug(1, "MacGuiImpl::MacButton: Drawing button %d (_fullRedraw = %d, drawFocused = %d, _value = %d)", _id, _fullRedraw, drawFocused, _value);

	// ScummVM's rounded rectangles aren't a complete match for QuickDraw's
	// rounded rectangles, and for arcs this small they don't look very
	// good. So we draw the corners manually.

	CornerLine buttonCorner[] = { { 0, 0 }, { 1, 2 }, { 1, 1 }, { 0, -1 } };
	CornerLine smallButtonCorner[] = { { 0, 0 }, { 1, 2 }, { 1, 1 }, { 0, -1 } };
	CornerLine frameCorner[] = { { 5, 1 }, { 3, 3 }, { 2, 4 }, { 1, 5 }, { 1, 3 }, { 0, 4 }, { 0, -1 } };

	Graphics::Surface *s = _window->innerSurface();
	Color fg, bg;

	if (drawFocused || (_window->getFocusedWidget() == this && _bounds.contains(_window->getMousePos()))) {
		fg = kWhite;
		bg = kBlack;
	} else {
		fg = kBlack;
		bg = kWhite;
	}

	int x0 = _bounds.left;
	int x1 = _bounds.right - 1;

	int y0 = _bounds.top;
	int y1 = _bounds.bottom - 1;

	CornerLine *corner;
	int cornerSize;

	if (_bounds.height() >= 20) {
		corner = buttonCorner;
		cornerSize = 3;
	} else {
		corner = smallButtonCorner;
		cornerSize = 2;
	}

	s->hLine(x0 + cornerSize, y0, x1 - cornerSize, kBlack);
	s->hLine(x0 + cornerSize, y1, x1 - cornerSize, kBlack);
	s->vLine(x0, y0 + cornerSize, y1 - cornerSize, kBlack);
	s->vLine(x1, y0 + cornerSize, y1 - cornerSize, kBlack);

	// The way the corners are rounded, we can fill this entire rectangle
	// in one go.

	Common::Rect inside = _bounds;
	inside.grow(-1);
	s->fillRect(inside, bg);

	drawCorners(_bounds, corner, true);

	const Graphics::Font *font = _window->_gui->getFont(kSystemFont);

	drawText(_text, _bounds.left, _bounds.top + (_bounds.height() - font->getFontHeight()) / 2, _bounds.width(), fg, bg, Graphics::kTextAlignCenter);

	Common::Rect bounds = _bounds;

	if (_window->getDefaultWidget() == this && _fullRedraw) {
		bounds.grow(4);

		x0 = bounds.left;
		x1 = bounds.right - 1;

		y0 = bounds.top;
		y1 = bounds.bottom - 1;

		for (int i = 0; i < 3; i++) {
			hLine(x0 + 6, y0 + i, x1 - 6, _enabled);
			hLine(x0 + 6, y1 - i, x1 - 6, _enabled);
			vLine(x0 + i, y0 + 6, y1 - 6, _enabled);
			vLine(x1 - i, y0 + 6, y1 - 6, _enabled);
		}

		drawCorners(bounds, frameCorner, _enabled);
	}

	_redraw = false;
	_fullRedraw = false;

	_window->markRectAsDirty(bounds);
}

void MacGuiImpl::MacButton::hLine(int x0, int y0, int x1, bool enabled) {
	Graphics::Surface *s = _window->innerSurface();

	if (!enabled) {
		if (x0 > x1)
			SWAP(x0, x1);

		for (int x = x0; x <= x1; x++) {
			if (((x + y0) % 2) == 0)
				s->setPixel(x, y0, kBlack);
			else
				s->setPixel(x, y0, kWhite);
		}
	} else
		s->hLine(x0, y0, x1, kBlack);
}

void MacGuiImpl::MacButton::vLine(int x0, int y0, int y1, bool enabled) {
	Graphics::Surface *s = _window->innerSurface();

	if (!enabled) {
		if (y0 > y1)
			SWAP(y0, y1);

		for (int y = y0; y <= y1; y++) {
			if (((x0 + y) % 2) == 0)
				s->setPixel(x0, y, kBlack);
			else
				s->setPixel(x0, y, kWhite);
		}
	} else
		s->vLine(x0, y0, y1, kBlack);
}

void MacGuiImpl::MacButton::drawCorners(Common::Rect r, CornerLine *corner, bool enabled) {
	for (int i = 0; corner[i].length >= 0; i++) {
		if (corner[i].length == 0)
			continue;

		int x0 = r.left + corner[i].start;
		int x1 = r.left + corner[i].start + corner[i].length - 1;
		int x2 = r.right - 1 - corner[i].start;
		int x3 = r.right - 1 - corner[i].start - corner[i].length + 1;

		int y0 = r.top + i;
		int y1 = r.bottom - i - 1;

		hLine(x0, y0, x1, enabled);
		hLine(x2, y0, x3, enabled);
		hLine(x0, y1, x1, enabled);
		hLine(x2, y1, x3, enabled);
	}
}

// ---------------------------------------------------------------------------
// Checkbox widget
// ---------------------------------------------------------------------------

MacGuiImpl::MacCheckbox::MacCheckbox(MacGuiImpl::MacDialogWindow *window, Common::Rect bounds, Common::String text, bool enabled) : MacWidget(window, bounds, text, enabled) {
	// The DITL may define a larger than necessary area for the checkbox,
	// so we need to calculate the hit bounds.

	const Graphics::Font *font = _window->_gui->getFont(kSystemFont);

	_hitBounds.left = _bounds.left;
	_hitBounds.top = _bounds.bottom - _bounds.height() / 2 - 8;
	_hitBounds.bottom = _hitBounds.top + 16;
	_hitBounds.right = _bounds.left + 18 + font->getStringWidth(_text) + 2;
}

bool MacGuiImpl::MacCheckbox::findWidget(int x, int y) const {
	return _hitBounds.contains(x, y);
}

void MacGuiImpl::MacCheckbox::draw(bool drawFocused) {
	if (!_redraw && !_fullRedraw)
		return;

	debug(1, "MacGuiImpl::MacCheckbox: Drawing checkbox %d (_fullRedraw = %d, drawFocused = %d, _value = %d)", _id, _fullRedraw, drawFocused, _value);

	Graphics::Surface *s = _window->innerSurface();
	Common::Rect box(_hitBounds.left + 2, _hitBounds.top + 2, _hitBounds.left + 14, _hitBounds.top + 14);

	if (_fullRedraw) {
		_window->innerSurface()->fillRect(_bounds, kWhite);

		int x = _hitBounds.left + 18;
		int y = _hitBounds.top;

		drawText(_text, x, y, _hitBounds.right - x);
		_window->markRectAsDirty(_bounds);
	} else
		_window->markRectAsDirty(box);

	s->fillRect(box, kBlack);
	if (drawFocused || (_window->getFocusedWidget() == this && _hitBounds.contains(_window->getMousePos()))) {
		box.grow(-2);
	} else {
		box.grow(-1);
	}
	s->fillRect(box, kWhite);

	if (_value && _enabled) {
		s->drawLine(box.left, box.top, box.right - 1, box.bottom - 1, kBlack);
		s->drawLine(box.left, box.bottom - 1, box.right - 1, box.top, kBlack);
	}

	_redraw = false;
	_fullRedraw = false;
}

bool MacGuiImpl::MacCheckbox::handleMouseUp(Common::Event &event) {
	_value = _value ? 0 : 1;
	setRedraw();
	return true;
}

// ---------------------------------------------------------------------------
// Static text widget. Text is encoded as MacRoman, so any outside strings
// (e.g.save file names or hard-coded texts) have to be re-encoded.
// ---------------------------------------------------------------------------

void MacGuiImpl::MacStaticText::draw(bool drawFocused) {
	if (!_redraw && !_fullRedraw)
		return;

	debug(1, "MacGuiImpl::MacStaticText: Drawing text %d (_fullRedraw = %d, drawFocused = %d, _value = %d)", _id, _fullRedraw, drawFocused, _value);

	_window->innerSurface()->fillRect(_bounds, _bg);
	drawText(_text, _bounds.left, _bounds.top, _bounds.width(), _fg, _bg, Graphics::kTextAlignLeft, _wordWrap, 1);
	_window->markRectAsDirty(_bounds);

	_redraw = false;
	_fullRedraw = false;
}

// ---------------------------------------------------------------------------
// Editable text widget
//
// Text is encoded as MacRoman, and has to be re-encoded before handed over
// to the outside world.
//
// The current edit position is stored in _caretPos. This holds the character
// the caret is placed right after, so 0 is the first character.
//
// The length of the current selection is stored in _selectionLen. Selections
// can extend left or right of the _caretPos. This makes it slightly more
// tricky to handle selections after they've been made, but simplifies the
// actual selection by mouse enormously.
// ---------------------------------------------------------------------------

MacGuiImpl::MacEditText::MacEditText(MacGuiImpl::MacDialogWindow *window, Common::Rect bounds, Common::String text, bool enabled) : MacWidget(window, bounds, text, enabled) {
	_font = _window->_gui->getFont(kSystemFont);

	// This widget gets its own text surface, to ensure that the text is
	// properly clipped to the widget's bounds. Technically, the other
	// widgets that draw text could use this too, but there we assume that
	// the texts are chosen to fit without clipping.

	Common::Rect textBounds = _bounds;
	textBounds.left++;

	_textSurface = _window->innerSurface()->getSubArea(textBounds);
}

bool MacGuiImpl::MacEditText::findWidget(int x, int y) const {
	// Once we start dragging the handle, any mouse position is considered
	// within the widget.

	if (_window->getFocusedWidget() == this)
		return true;

	return _bounds.contains(x, y);
}

int MacGuiImpl::MacEditText::getTextPosFromMouse(int x, int y) {
	if (_text.empty())
		return 0;

	if (y < _bounds.top)
		return 0;

	if (y >= _bounds.bottom)
		return _text.size();

	x -= _bounds.left;

	int textX = _textPos;
	uint i;

	for (i = 0; i < _text.size() && textX <= _bounds.width(); i++) {
		int charWidth = _font->getCharWidth(_text[i]);

		if (x >= textX && x < textX + charWidth) {
			if (x > textX + charWidth / 2)
				return i + 1;
			return i;
		}

		textX += charWidth;
	}

	if (x <= _bounds.left)
		return 0;

	return i;
}

void MacGuiImpl::MacEditText::updateSelection(int x, int y) {
	int oldSelectLen = _selectLen;

	int pos = getTextPosFromMouse(x, y);

	_selectLen = pos - _caretPos;

	if (_selectLen != oldSelectLen)
		setRedraw();
}

void MacGuiImpl::MacEditText::deleteSelection() {
	int startPos;
	int len;

	if (_selectLen < 0) {
		startPos = _caretPos + _selectLen;
		len = -_selectLen;
	} else {
		startPos = _caretPos;
		len = _selectLen;
	}

	_text.erase(startPos, len);
	_caretPos = startPos;
	_selectLen = 0;
	setRedraw();
}

void MacGuiImpl::MacEditText::selectAll() {
	_caretPos = 0;
	_selectLen = _text.size();
	setRedraw();
}

void MacGuiImpl::MacEditText::draw(bool drawFocused) {
	int caretX = 0;

	// Calculate the caret position, and make sure that it will be placed
	// inside the visible area of the widget. This may require scrolling
	// the text, which will trigger a redraw.

	if (_selectLen == 0) {
		caretX = _textPos - 1;

		for (int i = 0; i < _caretPos; i++)
			caretX += _font->getCharWidth((byte)_text[i]);

		int delta = 0;

		if (caretX < 0)
			delta = caretX;
		else if (caretX >= _textSurface.w)
			delta = caretX - _textSurface.w + 1;

		if (delta) {
			_textPos -= delta;
			_caretX -= delta;
			caretX -= delta;
			setRedraw();
		}
	}

	// Redraw the contents of the widget. This redraws the entire text,
	// which is a bit wasteful.

	if (_redraw || _fullRedraw) {
		Graphics::Surface *s = _window->innerSurface();

		s->fillRect(_bounds, kWhite);

		int selectStart = -1;
		int selectEnd = -1;

		if (_selectLen != 0) {
			if (_selectLen < 0) {
				selectStart = _caretPos + _selectLen;
				selectEnd = _caretPos - 1;
			} else {
				selectStart = _caretPos;
				selectEnd = _caretPos + _selectLen - 1;
			}
		}

		int x = _textPos;
		int y = 0;

		bool firstChar = true;
		bool firstCharSelected = false;
		bool lastCharSelected = false;

		for (int i = 0; i < (int)_text.size() && x < _textSurface.w; i++) {
			Color color = kBlack;
			int charWidth = _font->getCharWidth((byte)_text[i]);

			if (x + charWidth >= 0) {
				if (_selectLen != 0 && i >= selectStart && i <= selectEnd) {
					if (firstChar)
						firstCharSelected = true;
					lastCharSelected = true;

					_textSurface.fillRect(Common::Rect(x, 0, x + charWidth, _textSurface.h), kBlack);
					color = kWhite;
				} else
					lastCharSelected = false;

				_font->drawChar(&_textSurface, (byte)_text[i], x, y, color);
				firstChar = false;
			}

			x += charWidth;
		}

		if (firstCharSelected)
			_window->innerSurface()->fillRect(Common::Rect(_bounds.left + 1, _bounds.top, _bounds.left + 2, _bounds.bottom), kBlack);

		if (lastCharSelected && _bounds.left + x + 1 < _bounds.right)
			_window->innerSurface()->fillRect(Common::Rect(_bounds.left + x + 1, _bounds.top, _bounds.right, _bounds.bottom), kBlack);

		_window->markRectAsDirty(_bounds);
	}

	// Redraw the caret, if it has changed since the last time.

	if (_selectLen == 0) {
		uint32 now = _window->_system->getMillis();
		bool caretVisible = _caretVisible;

		if (now >= _nextCaretBlink) {
			_caretVisible = !_caretVisible;
			_nextCaretBlink = now + 500;
		}

		if (caretX != _caretX || caretVisible != _caretVisible) {
			if (caretX != _caretX && !_redraw && !_fullRedraw) {
				// Erase the old caret. Not needed if the whole
				// widget was just redrawn.

				_textSurface.vLine(_caretX, 0, _bounds.bottom - 1, kWhite);
				if (!_redraw && !_fullRedraw)
					_window->markRectAsDirty(Common::Rect(_bounds.left + _caretX + 1, _bounds.top, _bounds.left + _caretX + 2, _bounds.bottom));
			}

			// Draw the new caret

			_textSurface.vLine(caretX, 0, _bounds.bottom - 1, _caretVisible ? kBlack : kWhite);

			if (!_redraw && !_fullRedraw)
				_window->markRectAsDirty(Common::Rect(_bounds.left + caretX + 1, _bounds.top, _bounds.left + caretX + 2, _bounds.bottom));

			_caretX = caretX;
		}
	}

	_redraw = false;
	_fullRedraw = false;
}

void MacGuiImpl::MacEditText::handleMouseDown(Common::Event &event) {
	int oldSelectLen = _selectLen;
	int oldCaretPos = _caretPos;

	_caretPos = getTextPosFromMouse(event.mouse.x, event.mouse.y);
	_selectLen = 0;

	if (_selectLen != oldSelectLen || _caretPos != oldCaretPos)
		setRedraw();
}

bool MacGuiImpl::MacEditText::handleDoubleClick(Common::Event &event) {
	if (_text.empty())
		return false;

	_selectLen = 0;

	int startPos = _caretPos;
	int endPos = _caretPos;

	// Check if used clicked past the end of the text
	if (_caretPos >= (int)_text.size())
		startPos = endPos = _text.size() - 1;

	if (_text[startPos] == ' ') {
		while (startPos >= 0) {
			if (_text[startPos] != ' ') {
				startPos++;
				break;
			}
			startPos--;
		}

		while (endPos < (int)_text.size()) {
			if (_text[endPos] != ' ') {
				endPos--;
				break;
			}
			endPos++;
		}
	} else {
		while (startPos >= 0) {
			if (_text[startPos] == ' ') {
				startPos++;
				break;
			}
			startPos--;
		}

		while (endPos < (int)_text.size()) {
			if (_text[endPos] == ' ') {
				endPos--;
				break;
			}
			endPos++;
		}
	}

	if (startPos < 0)
		startPos = 0;

	if (endPos >= (int)_text.size())
		endPos = _text.size() - 1;

	_caretPos = startPos;
	_selectLen = endPos - startPos + 1;
	setRedraw();

	return false;
}

bool MacGuiImpl::MacEditText::handleKeyDown(Common::Event &event) {
	if (event.kbd.flags & (Common::KBD_CTRL | Common::KBD_ALT | Common::KBD_META))
		return false;

	// Stop caret blinking while typing. We do this by making the caret
	// visible, but force the next blink to happen immediately. Otherwise,
	// it may not register that the blink state has changed.

	_caretVisible = false;
	_nextCaretBlink = _window->_system->getMillis();

	switch (event.kbd.keycode) {
	case Common::KEYCODE_LEFT:
		if (_selectLen < 0)
			_caretPos = _caretPos + _selectLen;

		_caretPos--;

		if (_caretPos < 0)
			_caretPos = 0;

		if (_selectLen != 0) {
			_selectLen = 0;
			setRedraw();
		}
		return true;

	case Common::KEYCODE_RIGHT:
		if (_selectLen > 0)
			_caretPos += _selectLen;

		_caretPos++;

		if (_caretPos > (int)_text.size())
			_caretPos = _text.size();

		if (_selectLen != 0) {
			_selectLen = 0;
			setRedraw();
		}
		return true;

	case Common::KEYCODE_HOME:
	case Common::KEYCODE_UP:
		_caretPos = 0;

		if (_selectLen != 0) {
			_selectLen = 0;
			setRedraw();
		}
		return true;

	case Common::KEYCODE_END:
	case Common::KEYCODE_DOWN:
		_caretPos = _text.size();

		if (_selectLen != 0) {
			_selectLen = 0;
			setRedraw();
		}
		return true;

	case Common::KEYCODE_BACKSPACE:
		if (_selectLen != 0) {
			deleteSelection();
		} else if (_caretPos > 0) {
			_caretPos--;
			_text.deleteChar(_caretPos);
			setRedraw();
		}
		return true;

	case Common::KEYCODE_DELETE:
		if (_selectLen != 0) {
			deleteSelection();
		} else if (_caretPos < (int)_text.size()) {
			_text.deleteChar(_caretPos);
			setRedraw();
		}
		return true;

	default:
		break;
	}

	int c = _window->_gui->toMacRoman(event.kbd.ascii);

	if (c > 0) {
		if (_selectLen != 0)
			deleteSelection();
		if (_text.size() < _maxLength) {
			_text.insertChar(c, _caretPos);
			_caretPos++;
			setRedraw();
		}
		return true;
	}

	return false;
}

void MacGuiImpl::MacEditText::handleMouseHeld() {
	if (_text.empty())
		return;

	Common::Point mousePos = _window->getMousePos();

	int oldTextPos = _textPos;

	int minTextPos = MIN(_bounds.width() - _font->getStringWidth(_text) - 1, 1);

	if (mousePos.x < _bounds.left + 1 && mousePos.y < _bounds.bottom && _textPos < 1) {
		_textPos += 8;
		if (_textPos > 1)
			_textPos = 1;
	} else if (mousePos.x >= _bounds.right) {
		_textPos -= 8;
		if (_textPos < minTextPos)
			_textPos = minTextPos;
	}

	if (_textPos != oldTextPos) {
		updateSelection(mousePos.x, mousePos.y);
		setRedraw();
	}
}

void MacGuiImpl::MacEditText::handleMouseMove(Common::Event &event) {
	updateSelection(event.mouse.x, event.mouse.y);
}

// ---------------------------------------------------------------------------
// Picture widget
// ---------------------------------------------------------------------------

MacGuiImpl::MacPicture::MacPicture(MacGuiImpl::MacDialogWindow *window, Common::Rect bounds, int id, bool enabled) : MacWidget(window, bounds, "Picture", enabled) {
	_picture = _window->_gui->loadPict(id);
}

MacGuiImpl::MacPicture::~MacPicture() {
	if (_picture) {
		_picture->free();
		delete _picture;
	}
}

void MacGuiImpl::MacPicture::draw(bool drawFocused) {
	if (!_redraw && !_fullRedraw)
		return;

	debug(1, "MacGuiImpl::MacPicture: Drawing picture %d (_fullRedraw = %d, drawFocused = %d, _value = %d)", _id, _fullRedraw, drawFocused, _value);

	_window->drawSprite(_picture, _bounds.left, _bounds.top);

	_redraw = false;
	_fullRedraw = false;
}

// ---------------------------------------------------------------------------
// Slider base class
// ---------------------------------------------------------------------------

void MacGuiImpl::MacSliderBase::setValue(int value) {
	_value = CLIP(value, _minValue, _maxValue);
	_handlePos = calculatePosFromValue();
}

int MacGuiImpl::MacSliderBase::calculateValueFromPos() const {
	int posRange = _maxPos - _minPos;
	int posOffset = _handlePos - _minPos;

	int valueRange = _maxValue - _minValue;
	int valueOffset = (posRange / 2 + valueRange * posOffset) / posRange;

	return _minValue + valueOffset;
}

int MacGuiImpl::MacSliderBase::calculatePosFromValue() const {
	int valueRange = _maxValue - _minValue;
	int valueOffset = _value - _minValue;

	int posRange = _maxPos - _minPos;
	int posOffset = (valueRange / 2 + posRange * valueOffset) / valueRange;

	return _minPos + posOffset;
}

// ---------------------------------------------------------------------------
// Standard slider widget
// ---------------------------------------------------------------------------

MacGuiImpl::MacSlider::MacSlider(MacGuiImpl::MacDialogWindow *window, Common::Rect bounds, int minValue, int maxValue, int pageSize, bool enabled)
	: MacSliderBase(window, bounds, minValue, maxValue, 0, 0, enabled),
	_pageSize(pageSize) {
	_boundsButtonUp = Common::Rect(_bounds.left, _bounds.top, _bounds.right, _bounds.top + 16);
	_boundsButtonDown = Common::Rect(_bounds.left, _bounds.bottom - 16, _bounds.right, _bounds.bottom);
	_boundsBody = Common::Rect(_bounds.left, _bounds.top + 16, _bounds.right, _bounds.bottom - 16);

	_minPos = _boundsBody.top;
	_maxPos = _boundsBody.bottom - 16;

	_clickPos.x = -1;
	_clickPos.y = -1;
}

bool MacGuiImpl::MacSlider::findWidget(int x, int y) const {
	if (!isScrollable())
		return false;

	Common::Rect bounds = _bounds;

	// While dragging the handle, you're allowed to go outside the slider.
	// I don't know by how much, though.

	if (_grabOffset >= 0) {
		bounds.left -= 25;
		bounds.right += 25;
		bounds.top -= 50;
		bounds.bottom += 50;
	}

	return bounds.contains(x, y);
}

Common::Rect MacGuiImpl::MacSlider::getHandleRect(int value) {
	int handlePos = value * (_boundsBody.bottom - _boundsBody.top - 16) / (_maxValue - _minValue);

	Common::Rect handleRect;

	handleRect.left = _boundsBody.left + 1;
	handleRect.top = _boundsBody.top + handlePos;
	handleRect.right = _boundsBody.right - 1;
	handleRect.bottom = handleRect.top + 16;

	return handleRect;
}

void MacGuiImpl::MacSlider::fill(Common::Rect r, bool inverted) {
	Color pattern[2][4] = {
		{ kBlack, kWhite, kWhite, kWhite },
		{ kWhite, kWhite, kBlack, kWhite }
	};

	Graphics::Surface *s = _window->innerSurface();

	for (int y = r.top; y < r.bottom; y++) {
		for (int x = r.left; x < r.right; x++) {
			if (inverted) {
				// The inverted style is used for drawing the "ghost" of the
				// slider handle while dragging. I think this matches the
				// original behavior, though I'm not quite sure.

				bool srcPixel = s->getPixel(x, y) == kBlack;
				bool dstPixel = pattern[y % 2][x % 4] == kWhite;

				Color color = (srcPixel ^ dstPixel) ? kBlack : kWhite;

				s->setPixel(x, y, color);
			} else
				s->setPixel(x, y, pattern[y % 2][x % 4]);
		}
	}
}

void MacGuiImpl::MacSlider::draw(bool drawFocused) {
	if (!_redraw && !_fullRedraw)
		return;

	// There are several things that will trigger a redraw, but unlike
	// other widgets this one only handles full redraws. Everything else
	// is handled outside of draw().

	if (_fullRedraw) {
		debug(1, "MacGuiImpl::MacSlider: Drawing slider (_fullRedraw = %d, drawFocused = %d, _value = %d)", _fullRedraw, drawFocused, _value);

		Graphics::Surface *s = _window->innerSurface();

		s->frameRect(_bounds, kBlack);
		s->hLine(_bounds.left + 1, _bounds.top + 15, _bounds.right - 2, kBlack);
		s->hLine(_bounds.left + 1, _bounds.bottom - 16, _bounds.right - 2, kBlack);

		drawUpArrow(false);
		drawDownArrow(false);

		Common::Rect fillRect(_boundsBody.left + 1, _boundsBody.top, _boundsBody.right - 1, _boundsBody.bottom);

		if (isScrollable()) {
			fill(fillRect);

			Common::Rect handleRect = getHandleRect(_value);
			drawHandle(handleRect);
		} else
			s->fillRect(fillRect, kWhite);

		_window->markRectAsDirty(_bounds);
	}

	_redraw = false;
	_fullRedraw = false;
}

// There is a narrower version of these arrows, but I'm speculating that
// they're intended for a 9" Mac screen. We go with the slightly wider version
// here to make them easier to hit.

void MacGuiImpl::MacSlider::drawUpArrow(bool markAsDirty) {
	debug(1, "MacGuiImpl::MacSlider: Drawing up arrow (_upArrowPressed = %d, markAsDirty = %d)", _upArrowPressed, markAsDirty);

	const uint16 upArrow[] = {
		0x0600, 0x0900, 0x1080, 0x2040, 0x4020,
		0xF0F0, 0x1080, 0x1080, 0x1080, 0x1F80
	};

	const uint16 upArrowFilled[] = {
		0x0600, 0x0F00, 0x1F80, 0x3FC0, 0x7FE0,
		0xFFF0, 0x1F80, 0x1F80, 0x1F80, 0x1F80
	};

	drawArrow(_boundsButtonUp, (_upArrowPressed ? upArrowFilled : upArrow), markAsDirty);
}

void MacGuiImpl::MacSlider::drawDownArrow(bool markAsDirty) {
	debug(1, "MacGuiImpl::MacSlider: Drawing down arrow (_downArrowPressed = %d, markAsDirty = %d)", _downArrowPressed, markAsDirty);

	const uint16 downArrow[] = {
		0x1F80,	0x1080,	0x1080,	0x1080,	0xF0F0,
		0x4020,	0x2040,	0x1080,	0x0900,	0x0600
	};

	const uint16 downArrowFilled[] = {
		0x1F80, 0x1F80, 0x1F80, 0x1F80, 0xFFF0,
		0x7FE0, 0x3FC0, 0x1F80, 0x0F00, 0x0600
	};

	drawArrow(_boundsButtonDown, (_downArrowPressed ? downArrowFilled : downArrow), markAsDirty);
}

void MacGuiImpl::MacSlider::drawArrow(Common::Rect r, const uint16 *bitmap, bool markAsDirty) {
	Graphics::Surface *s = _window->innerSurface();

	r.grow(-1);

	s->fillRect(r, kWhite);
	drawBitmap(Common::Rect(r.left + 1, r.top + 2, r.right - 1, r.top + 12), bitmap, kBlack);

	if (markAsDirty)
		_window->markRectAsDirty(r);
}

void MacGuiImpl::MacSlider::eraseDragHandle() {
	Common::Rect r(_boundsBody.left + 1, _handlePos, _boundsBody.right - 1, _handlePos + 16);
	fill(r);
	_window->markRectAsDirty(r);
}

void MacGuiImpl::MacSlider::drawHandle(Common::Rect r) {
	debug(2, "MacGuiImpl::MacSlider::drawHandle(%d)", r.top);

	Graphics::Surface *s = _window->innerSurface();

	s->frameRect(r, kBlack);
	r.grow(-1);
	s->fillRect(r, kWhite);
}

void MacGuiImpl::MacSlider::redrawHandle(int oldValue, int newValue) {
	Common::Rect r = getHandleRect(oldValue);

	fill(r);
	_window->markRectAsDirty(r);

	r = getHandleRect(newValue);
	drawHandle(r);
	_window->markRectAsDirty(r);
}

void MacGuiImpl::MacSlider::handleMouseDown(Common::Event &event) {
	int x = event.mouse.x;
	int y = event.mouse.y;

	_clickPos.x = x;
	_clickPos.y = y;
	_paging = 0;
	_grabOffset = -1;
	_handlePos = -1;

	int oldValue = _value;

	if (_boundsButtonUp.contains(x, y)) {
		_nextRepeat = _window->_system->getMillis() + 200;
		_upArrowPressed = true;
		_value = MAX(_minValue, _value - 1);
		drawUpArrow(true);
	} else if (_boundsButtonDown.contains(x, y)) {
		_nextRepeat = _window->_system->getMillis() + 200;
		_downArrowPressed = true;
		_value = MIN(_maxValue, _value + 1);
		drawDownArrow(true);
	} else {
		Common::Rect handleRect = getHandleRect(_value);

		if (y < handleRect.top) {
			_nextRepeat = _window->_system->getMillis() + 200;
			_paging = -1;
			_value = MAX(_minValue, _value - (_pageSize - 1));
		} else if (y >= handleRect.bottom) {
			_nextRepeat = _window->_system->getMillis() + 200;
			_paging = 1;
			_value = MIN(_maxValue, _value + (_pageSize - 1));
		} else {
			_grabOffset = y - handleRect.top;
			_handlePos = handleRect.top;
		}
	}

	if (_value != oldValue)
		redrawHandle(oldValue, _value);
}

bool MacGuiImpl::MacSlider::handleMouseUp(Common::Event &event) {
	if (_upArrowPressed) {
		_upArrowPressed = false;
		drawUpArrow(true);
	} else if (_downArrowPressed) {
		_downArrowPressed = false;
		drawDownArrow(true);
	} else if (_grabOffset >= 0) {
		// Erase the drag handle, since the handle might not end up in
		// the exact same spot.
		eraseDragHandle();

		// Calculate new value and move the handle there
		int newValue = calculateValueFromPos();

		redrawHandle(_value, newValue);
		_value = newValue;
	}

	_paging = 0;
	_grabOffset = -1;
	_handlePos = -1;
	_clickPos.x = -1;
	_clickPos.y = -1;

	return false;
}

void MacGuiImpl::MacSlider::handleMouseMove(Common::Event &event) {
	int x = event.mouse.x;
	int y = event.mouse.y;

	if (_grabOffset >= 0) {
		if (!findWidget(x, y)) {
			eraseDragHandle();

			Common::Rect handleRect = getHandleRect(_value);

			if (ABS(_handlePos - handleRect.top) <= handleRect.height()) {
				drawHandle(handleRect);
				_window->markRectAsDirty(handleRect);
			}

			return;
		}

		int newHandlePos = CLIP<int>(y - _grabOffset, _boundsBody.top, _boundsBody.bottom - 16);

		// Theoretically, we could end here if the handle position has
		// not changed. However, we currently don't keep track of if
		// the handle is hidden because the mouse has moved out of the
		// widget's control.

		eraseDragHandle();

		Common::Rect handleRect = getHandleRect(_value);

		if (ABS(_handlePos - handleRect.top) <= handleRect.height()) {
			drawHandle(handleRect);
			_window->markRectAsDirty(handleRect);
		}

		_handlePos = newHandlePos;

		int x0 = _boundsBody.left + 1;
		int x1 = _boundsBody.right - 1;
		int y0 = _handlePos;
		int y1 = _handlePos + 16;

		// Drawing a solid rectangle would be easier, and probably look
		// better. But it seems the original Mac widget would draw the
		// frame as an inverted slider background, even when drawing it
		// on top of the slider handle.

		fill(Common::Rect(x0, y0, x1, y0 + 1), true);
		fill(Common::Rect(x0, y1 - 1, x1, y1), true);
		fill(Common::Rect(x0, y0 + 1, x0 + 1, y1 - 1), true);
		fill(Common::Rect(x1 - 1, y0 + 1, x1, y1 - 1), true);

		_window->markRectAsDirty(Common::Rect(x0, y0, x1, y1));
	} else {
		if (!_boundsButtonUp.contains(x, y)) {
			if (_upArrowPressed) {
				_upArrowPressed = false;
				drawUpArrow(true);
			}
		} else {
			if (_boundsButtonUp.contains(_clickPos) && !_upArrowPressed) {
				_nextRepeat = _window->_system->getMillis() + 200;
				_upArrowPressed = true;
				drawUpArrow(true);
			}
		}

		if (!_boundsButtonDown.contains(x, y)) {
			if (_downArrowPressed) {
				_downArrowPressed = false;
				drawDownArrow(true);
			}
		} else {
			if (_boundsButtonDown.contains(_clickPos) && !_downArrowPressed) {
				_nextRepeat = _window->_system->getMillis() + 200;
				_downArrowPressed = true;
				drawDownArrow(true);
			}
		}
	}
}

void MacGuiImpl::MacSlider::handleMouseHeld() {
	uint32 now = _window->_system->getMillis();
	Common::Point p = _window->getMousePos();

	if (now < _nextRepeat || !findWidget(p.x, p.y))
		return;

	int oldValue = _value;

	if (_upArrowPressed) {
		_value = MAX(_minValue, _value - 1);
		_nextRepeat = now + 80;
	}

	if (_downArrowPressed) {
		_value = MIN(_maxValue, _value + 1);
		_nextRepeat = now + 80;
	}

	if (_paging) {
		Common::Rect r = getHandleRect(_value);

		// Keep paging until at least half the scroll handle has gone
		// past the mouse cursor. This may have to be tuned.

		if (_paging == -1) {
			if (p.y < r.top + r.height() / 2 && _value > _minValue) {
				_nextRepeat = now + 100;
				_value = MAX(_minValue, _value - (_pageSize - 1));
			}
		} else if (_paging == 1) {
			if (p.y >= r.bottom - r.height() / 2 && _value < _maxValue) {
				_nextRepeat = now + 100;
				_value = MIN(_maxValue, _value + (_pageSize - 1));
			}
		}
	}

	if (_value != oldValue)
		redrawHandle(oldValue, _value);
}

void MacGuiImpl::MacSlider::handleWheelUp() {
	int oldValue = _value;

	_value = MAX(_minValue, _value - (_pageSize - 1));

	if (_value != oldValue)
		redrawHandle(oldValue, _value);
}

void MacGuiImpl::MacSlider::handleWheelDown() {
	int oldValue = _value;

	_value = MIN(_maxValue, _value + (_pageSize - 1));

	if (_value != oldValue)
		redrawHandle(oldValue, _value);
}

// ---------------------------------------------------------------------------
// Picture slider widget. This is the custom slider widget used for the Loom
// and Indy 3 options dialogs. It consists of a background image and a slider
// drag handle.
// ---------------------------------------------------------------------------

bool MacGuiImpl::MacPictureSlider::findWidget(int x, int y) const {
	// Once we start dragging the handle, any mouse position is considered
	// within the widget.

	if (_window->getFocusedWidget() == this)
		return true;

	return _bounds.contains(x, y);
}

void MacGuiImpl::MacPictureSlider::draw(bool drawFocused) {
	if (!_redraw && !_fullRedraw)
		return;

	debug(1, "MacGuiImpl::MacPictureSlider: Drawing slider %d (_fullRedraw = %d, drawFocused = %d, _value = %d)", _id, _fullRedraw, drawFocused, _value);

	if (_fullRedraw) {
		_window->drawSprite(_background->getPicture(), _bounds.left, _bounds.top);
		drawHandle();
	}

	_redraw = false;
	_fullRedraw = false;
}

void MacGuiImpl::MacPictureSlider::eraseHandle() {
	Common::Rect r = _handle->getBounds();
	int y = r.top - _bounds.top;
	int w = r.width();
	int h = r.height();

	Graphics::Surface *background = _background->getPicture();
	Graphics::Surface sprite = background->getSubArea(Common::Rect(_handlePos, y, _handlePos + w, y + h));
	_window->drawSprite(&sprite, _bounds.left + _handlePos, r.top);
}

void MacGuiImpl::MacPictureSlider::drawHandle() {
	Graphics::Surface *sprite = _handle->getPicture();
	Common::Rect r = _handle->getBounds();

	_window->drawSprite(sprite, _bounds.left + _handlePos, r.top);
}

void MacGuiImpl::MacPictureSlider::handleMouseDown(Common::Event &event) {
	int mouseX = event.mouse.x;
	int handleWidth = _handle->getBounds().width();

	if (mouseX >= _handlePos && mouseX < _handlePos + handleWidth)
		_grabOffset = event.mouse.x - _bounds.left - _handlePos;
	else
		_grabOffset = handleWidth / 2;

	handleMouseMove(event);
}

bool MacGuiImpl::MacPictureSlider::handleMouseUp(Common::Event &event) {
	// Erase the drag rect, since the handle might not end up in
	// the exact same spot.
	int newValue = calculateValueFromPos();

	// Even if the value doesn't change, we need to reposition the slider
	// handle, or it may be left between two values. This is particularly
	// noticeable for the music quality slider.
	eraseHandle();
	setValue(newValue);
	drawHandle();

	return false;
}

void MacGuiImpl::MacPictureSlider::handleMouseMove(Common::Event &event) {
	int newPos = CLIP<int>(event.mouse.x - _bounds.left - _grabOffset, _minX, _maxX);

	if (newPos != _handlePos) {
		eraseHandle();
		_handlePos = newPos;
		drawHandle();
	}
}

void MacGuiImpl::MacPictureSlider::handleWheelUp() {
	int newValue = MAX(_minValue, _value + 1);

	if (_value != newValue) {
		eraseHandle();
		setValue(newValue);
		drawHandle();
	}
}

void MacGuiImpl::MacPictureSlider::handleWheelDown() {
	int newValue = MIN(_maxValue, _value - 1);

	if (_value != newValue) {
		eraseHandle();
		setValue(newValue);
		drawHandle();
	}
}

// ---------------------------------------------------------------------------
// List box widget
// ---------------------------------------------------------------------------

MacGuiImpl::MacListBox::MacListBox(MacGuiImpl::MacDialogWindow *window, Common::Rect bounds, Common::StringArray texts, bool enabled, bool contentUntouchable) : MacWidget(window, bounds, "ListBox", enabled), _texts(texts) {
	int pageSize = _bounds.height() / 16;

	int numSlots = MIN<int>(pageSize, texts.size());

	for (int i = 0; i < numSlots; i++) {
		Common::Rect r(_bounds.left + 1, _bounds.top + 1 + 16 * i, _bounds.right - 16, _bounds.top + 1 + 16 * (i + 1));
		MacStaticText *tmp = new MacStaticText(window, r, texts[i], enabled);
		if (contentUntouchable)
			tmp->setEnabled(false);
		_textWidgets.push_back(tmp);
	}

	_slider = new MacSlider(window, Common::Rect(_bounds.right - 16, _bounds.top, _bounds.right, _bounds.bottom), 0, texts.size() - pageSize, pageSize, enabled);

	// The widget value indicates the selected element
	_value = 0;
	updateTexts();
}

MacGuiImpl::MacListBox::~MacListBox() {
	_texts.clear();
	delete _slider;

	for (uint i = 0; i < _textWidgets.size(); i++)
		delete _textWidgets[i];
}

bool MacGuiImpl::MacListBox::findWidget(int x, int y) const {
	return MacWidget::findWidget(x, y) || _slider->findWidget(x, y);
}

void MacGuiImpl::MacListBox::setRedraw(bool fullRedraw) {
	MacWidget::setRedraw(fullRedraw);
	_slider->setRedraw(fullRedraw);

	for (uint i = 0; i < _textWidgets.size(); i++)
		_textWidgets[i]->setRedraw(fullRedraw);
}

void MacGuiImpl::MacListBox::updateTexts() {
	int offset = _slider->getValue();

	for (uint i = 0; i < _textWidgets.size(); i++) {
		_textWidgets[i]->setText(_texts[i + offset]);

		if (_textWidgets[i]->isEnabled() && (int)i + offset == _value)
			_textWidgets[i]->setColor(kWhite, kBlack);
		else
			_textWidgets[i]->setColor(kBlack, kWhite);
	}
}

void MacGuiImpl::MacListBox::draw(bool drawFocused) {
	for (uint i = 0; i < _textWidgets.size(); i++)
		_textWidgets[i]->draw(drawFocused);

	_slider->draw(drawFocused);

	if (!_redraw && !_fullRedraw)
		return;

	debug(1, "MacGuiImpl::MacListBox: Drawing list box (_fullRedraw = %d, drawFocused = %d)", _fullRedraw, drawFocused);

	Graphics::Surface *s = _window->innerSurface();

	s->hLine(_bounds.left, _bounds.top, _bounds.right - 17, kBlack);
	s->hLine(_bounds.left, _bounds.bottom - 1, _bounds.right - 17, kBlack);
	s->vLine(_bounds.left, _bounds.top + 1, _bounds.bottom - 2, kBlack);

	_redraw = false;
	_fullRedraw = false;

	_window->markRectAsDirty(_bounds);
}

void MacGuiImpl::MacListBox::handleMouseDown(Common::Event &event) {
	if (_slider->findWidget(event.mouse.x, event.mouse.y)) {
		int oldValue = _slider->getValue();

		_sliderFocused = true;
		_slider->handleMouseDown(event);

		if (_slider->getValue() != oldValue)
			updateTexts();

		return;
	}

	int offset = _slider->getValue();

	for (uint i = 0; i < _textWidgets.size(); i++) {
		if (_textWidgets[i]->findWidget(event.mouse.x, event.mouse.y)) {
			setValue(i + offset);
			break;
		}
	}
}

bool MacGuiImpl::MacListBox::handleDoubleClick(Common::Event &event) {
	for (uint i = 0; i < _textWidgets.size(); i++) {
		if (_textWidgets[i]->findWidget(event.mouse.x, event.mouse.y))
			return true;
	}

	return false;
}

bool MacGuiImpl::MacListBox::handleMouseUp(Common::Event &event) {
	if (_sliderFocused) {
		int oldValue = _slider->getValue();

		_sliderFocused = false;
		_slider->handleMouseUp(event);

		if (_slider->getValue() != oldValue)
			updateTexts();
	}

	return false;
}

void MacGuiImpl::MacListBox::handleMouseMove(Common::Event &event) {
	if (_sliderFocused) {
		int oldValue = _slider->getValue();

		_slider->handleMouseMove(event);

		if (_slider->getValue() != oldValue)
			updateTexts();
	}
}

void MacGuiImpl::MacListBox::handleMouseHeld() {
	if (_sliderFocused) {
		int oldValue = _slider->getValue();

		_slider->handleMouseHeld();

		if (_slider->getValue() != oldValue)
			updateTexts();
	}
}

void MacGuiImpl::MacListBox::handleWheelUp() {
	handleWheel(-1);
}

void MacGuiImpl::MacListBox::handleWheelDown() {
	handleWheel(1);
}

void MacGuiImpl::MacListBox::handleWheel(int distance) {
	if (!_slider->isScrollable())
		return;

	Common::Point mousePos = _window->getMousePos();
	int oldValue = _slider->getValue();

	if (_slider->findWidget(mousePos.x, mousePos.y))
		distance *= _slider->getPageSize();

	_slider->setValue(oldValue + distance);

	int newValue = _slider->getValue();

	if (newValue != oldValue) {
		updateTexts();
		_slider->redrawHandle(oldValue, newValue);
	}
}

bool MacGuiImpl::MacListBox::handleKeyDown(Common::Event &event) {
	if (_texts.size() <= 1 || !_textWidgets[0]->isEnabled())
		return false;

	if (event.kbd.flags & (Common::KBD_CTRL | Common::KBD_ALT | Common::KBD_META))
		return false;

	int oldValue = _value;

	switch (event.kbd.keycode) {
	case Common::KEYCODE_UP:
		_value = MAX(_value - 1, 0);
		break;

	case Common::KEYCODE_DOWN:
		_value = MIN<int>(_value + 1, _texts.size() - 1);
		break;

	default:
		break;
	}

	if (_value != oldValue) {
		int sliderValue = _slider->getValue();
		int pageSize = _slider->getPageSize();
		int newSliderValue = sliderValue;

		if (_value < sliderValue)
			newSliderValue = _value;
		else if (_value >= sliderValue + pageSize)
			newSliderValue = _value - pageSize + 1;

		if (sliderValue != newSliderValue) {
			_slider->setValue(newSliderValue);
			_slider->redrawHandle(sliderValue, newSliderValue);
		}

		updateTexts();
	}

	return false;
}

} // End of namespace Scumm
