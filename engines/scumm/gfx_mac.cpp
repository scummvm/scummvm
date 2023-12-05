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
#include "common/enc-internal.h"
#include "common/macresman.h"
#include "common/config-manager.h"
#include "common/ustr.h"

#include "graphics/cursorman.h"
#include "graphics/maccursor.h"
#include "graphics/macega.h"
#include "graphics/primitives.h"
#include "graphics/fonts/macfont.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macwindowmanager.h"

#include "image/pict.h"

#include "scumm/actor.h"
#include "scumm/charset.h"
#include "scumm/gfx_mac.h"
#include "scumm/players/player_v3m.h"
#include "scumm/scumm.h"
#include "scumm/scumm_v4.h"
#include "scumm/sound.h"
#include "scumm/usage_bits.h"
#include "scumm/verbs.h"

#define TEXT_END_MARKER { 0, 0, kStyleRegular, Graphics::kTextAlignLeft, nullptr }

namespace Scumm {

void ScummEngine::mac_markScreenAsDirty(int x, int y, int w, int h) {
	// Mark the virtual screen as dirty. The top and left coordinates are
	// rounded down, while the bottom and right ones are rounded up.

	VirtScreen *vs = &_virtscr[kMainVirtScreen];

	int vsTop = y / 2 - vs->topline;
	int vsBottom = (y + h) / 2 - vs->topline;
	int vsLeft = x / 2;
	int vsRight = (x + w) / 2;

	if ((y + h) & 1)
		vsBottom++;

	if ((x + w) & 1)
		vsRight++;

	markRectAsDirty(kMainVirtScreen, vsLeft, vsRight, vsTop, vsBottom);
}

void ScummEngine::mac_drawStripToScreen(VirtScreen *vs, int top, int x, int y, int width, int height) {
	// The verb screen is completely replaced with a custom GUI. While
	// it is active, all other drawing to that area is suspended.
	if (_macGui && vs->number == kVerbVirtScreen && _macGui->isVerbGuiActive())
		return;

	const byte *pixels = vs->getPixels(x, top);
	const byte *ts = (byte *)_textSurface.getBasePtr(x * 2, y * 2);
	byte *mac = (byte *)_macScreen->getBasePtr(x * 2, y * 2);

	int pixelsPitch = vs->pitch;
	int tsPitch = _textSurface.pitch;
	int macPitch = _macScreen->pitch;

	// In b/w Mac rendering mode, the shadow palette is implemented here,
	// and not as a palette manipulation. See special cases in o5_roomOps()
	// and updatePalette().
	//
	// This is used at the very least for the lightning flashes at Castle
	// Brunwald in Indy 3, as well as the scene where the dragon finds
	// Rusty in Loom.
	//
	// Interestingly, the original Mac interpreter does not seem to do
	// this, and instead just renders the scene as if the palette was
	// unmodified. At least, that's what Mini vMac did when I tried it.

	if (_renderMode == Common::kRenderMacintoshBW) {
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) {
				int color = enhancementEnabled(kEnhVisualChanges) ? _shadowPalette[pixels[w]] : pixels[w];
				if (ts[2 * w] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w] = Graphics::macEGADither[color][0];
				if (ts[2 * w + 1] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w + 1] = Graphics::macEGADither[color][1];
				if (ts[2 * w + tsPitch] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w + macPitch] = Graphics::macEGADither[color][2];
				if (ts[2 * w + tsPitch + 1] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w + macPitch + 1] = Graphics::macEGADither[color][3];
			}

			pixels += pixelsPitch;
			ts += tsPitch * 2;
			mac += macPitch * 2;
		}
	} else {
		for (int h = 0; h < height; h++) {
			for (int w = 0; w < width; w++) {
				if (ts[2 * w] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w] = pixels[w];
				if (ts[2 * w + 1] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w + 1] = pixels[w];
				if (ts[2 * w + tsPitch] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w + macPitch] = pixels[w];
				if (ts[2 * w + tsPitch + 1] == CHARSET_MASK_TRANSPARENCY)
					mac[2 * w + macPitch + 1] = pixels[w];
			}

			pixels += pixelsPitch;
			ts += tsPitch * 2;
			mac += macPitch * 2;
		}
	}

	_system->copyRectToScreen(_macScreen->getBasePtr(x * 2, y * 2), _macScreen->pitch, x * 2, y * 2, width * 2, height * 2);
}

void ScummEngine::mac_drawIndy3TextBox() {
	Graphics::Surface *s = _macGui->textArea();

	// The first two rows of the text box are padding for font rendering.
	// They are not drawn to the screen.

	int x = 96;
	int y = 32 + _screenDrawOffset * 2;
	int w = s->w;
	int h = s->h - 2;

	// The text box is drawn to the Mac screen and text surface, as if it
	// had been one giant glyph. Note that it will be drawn on the main
	// virtual screen, but we still pretend it's on the text one.

	byte *ptr = (byte *)s->getBasePtr(0, 2);
	int pitch = s->pitch;

	_macScreen->copyRectToSurface(ptr, pitch, x, y, w, h);
	_textSurface.fillRect(Common::Rect(x, y, x + w, y + h), 0);

	mac_markScreenAsDirty(x, y, w, h);
}

void ScummEngine::mac_undrawIndy3TextBox() {
	Graphics::Surface *s = _macGui->textArea();

	int x = 96;
	int y = 32 + _screenDrawOffset * 2;
	int w = s->w;
	int h = s->h - 2;

	_macScreen->fillRect(Common::Rect(x, y, x + w, y + h), 0);
	_textSurface.fillRect(Common::Rect(x, y, x + w, y + h), CHARSET_MASK_TRANSPARENCY);

	mac_markScreenAsDirty(x, y, w, h);
}

void ScummEngine::mac_undrawIndy3CreditsText() {
	// Set _masMask to make the text clear, and _textScreenID to ensure
	// that it's the main area that's cleared.
	_charset->_hasMask = true;
	_charset->_textScreenID = kMainVirtScreen;
	restoreCharsetBg();
}

Common::KeyState ScummEngine::mac_showOldStyleBannerAndPause(const char *msg, int32 waitTime) {
	char bannerMsg[512];

	_messageBannerActive = true;

	// Fetch the translated string for the message...
	convertMessageToString((const byte *)msg, (byte *)bannerMsg, sizeof(bannerMsg));

	MacGui::MacDialogWindow *window = _macGui->drawBanner(bannerMsg);

	Common::KeyState ks = Common::KEYCODE_INVALID;

	if (waitTime) {
		bool leftBtnPressed = false, rightBtnPressed = false;
		waitForBannerInput(waitTime, ks, leftBtnPressed, rightBtnPressed);
	}

	delete window;

	clearClickedStatus();

	_messageBannerActive = false;

	return ks;
}

// ===========================================================================
// Macintosh GUI
// ===========================================================================

// ---------------------------------------------------------------------------
// Simple window and widget classes. These are meant to emulate the look and
// feel of an 80s Macintosh.
// ---------------------------------------------------------------------------

// ---------------------------------------------------------------------------
// Base widget
// ---------------------------------------------------------------------------

MacGui::MacWidget::MacWidget(MacGui::MacDialogWindow *window, Common::Rect bounds, Common::String text, bool enabled) : MacGuiObject(bounds, enabled), _window(window), _text(text) {
	// Widgets are clipped to the inner surface of the dialog. If a widget
	// is clipped out of existence, make it invisible to avoid crashes.

	Graphics::Surface *s = _window->innerSurface();

	_bounds.clip(Common::Rect(s->w, s->h));

	if (_bounds.width() <= 0 || _bounds.height() <= 0)
		_visible = false;
}

Common::String MacGui::MacWidget::getText() const {
	Common::String temp = Common::U32String(_text, Common::kMacRoman).encode(Common::kUtf8);
	return temp;
}

bool MacGui::MacWidget::findWidget(int x, int y) const {
	return _enabled && _bounds.contains(x, y);
}

void MacGui::MacWidget::setRedraw(bool fullRedraw) {
	if (fullRedraw)
		_fullRedraw = true;
	else
		_redraw = true;
}

void MacGui::MacWidget::setEnabled(bool enabled) {
	_enabled = enabled;
	setRedraw(true);
}

void MacGui::MacWidget::setValue(int value) {
	_value = value;
	setRedraw();
}

void MacGui::MacWidget::drawBitmap(Common::Rect r, const uint16 *bitmap, Color color) const {
	_window->_gui->drawBitmap(_window->innerSurface(), r, bitmap, color);
}

int MacGui::MacWidget::drawText(Common::String text, int x, int y, int w, Color fg, Color bg, Graphics::TextAlign align, bool wordWrap, int deltax) const {
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

void MacGui::MacButton::draw(bool drawFocused) {
	if (!_redraw && !_fullRedraw)
		return;

	debug(1, "MacGui::MacButton: Drawing button %d (_fullRedraw = %d, drawFocused = %d, _value = %d)", _id, _fullRedraw, drawFocused, _value);

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

void MacGui::MacButton::hLine(int x0, int y0, int x1, bool enabled) {
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

void MacGui::MacButton::vLine(int x0, int y0, int y1, bool enabled) {
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

void MacGui::MacButton::drawCorners(Common::Rect r, CornerLine *corner, bool enabled) {
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

MacGui::MacCheckbox::MacCheckbox(MacGui::MacDialogWindow *window, Common::Rect bounds, Common::String text, bool enabled) : MacWidget(window, bounds, text, enabled) {
	// The DITL may define a larger than necessary area for the checkbox,
	// so we need to calculate the hit bounds.

	const Graphics::Font *font = _window->_gui->getFont(kSystemFont);

	_hitBounds.left = _bounds.left;
	_hitBounds.top = _bounds.bottom - _bounds.height() / 2 - 8;
	_hitBounds.bottom = _hitBounds.top + 16;
	_hitBounds.right = _bounds.left + 18 + font->getStringWidth(_text) + 2;
}

bool MacGui::MacCheckbox::findWidget(int x, int y) const {
	return _hitBounds.contains(x, y);
}

void MacGui::MacCheckbox::draw(bool drawFocused) {
	if (!_redraw && !_fullRedraw)
		return;

	debug(1, "MacGui::MacCheckbox: Drawing checkbox %d (_fullRedraw = %d, drawFocused = %d, _value = %d)", _id, _fullRedraw, drawFocused, _value);

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

bool MacGui::MacCheckbox::handleMouseUp(Common::Event &event) {
	_value = _value ? 0 : 1;
	setRedraw();
	return true;
}

// ---------------------------------------------------------------------------
// Static text widget. Text is encoded as MacRoman, so any outside strings
// (e.g.save file names or hard-coded texts) have to be re-encoded.
// ---------------------------------------------------------------------------

void MacGui::MacStaticText::draw(bool drawFocused) {
	if (!_redraw && !_fullRedraw)
		return;

	debug(1, "MacGui::MacStaticText: Drawing text %d (_fullRedraw = %d, drawFocused = %d, _value = %d)", _id, _fullRedraw, drawFocused, _value);

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

MacGui::MacEditText::MacEditText(MacGui::MacDialogWindow *window, Common::Rect bounds, Common::String text, bool enabled) : MacWidget(window, bounds, text, enabled) {
	_font = _window->_gui->getFont(kSystemFont);

	// This widget gets its own text surface, to ensure that the text is
	// properly clipped to the widget's bounds. Technically, the other
	// widgets that draw text could use this too, but there we assume that
	// the texts are chosen to fit without clipping.

	Common::Rect textBounds = _bounds;
	textBounds.left++;

	_textSurface = _window->innerSurface()->getSubArea(textBounds);
}

bool MacGui::MacEditText::findWidget(int x, int y) const {
	// Once we start dragging the handle, any mouse position is considered
	// within the widget.

	if (_window->getFocusedWidget() == this)
		return true;

	return _bounds.contains(x, y);
}

int MacGui::MacEditText::getTextPosFromMouse(int x, int y) {
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

void MacGui::MacEditText::updateSelection(int x, int y) {
	int oldSelectLen = _selectLen;

	int pos = getTextPosFromMouse(x, y);

	_selectLen = pos - _caretPos;

	if (_selectLen != oldSelectLen)
		setRedraw();
}

void MacGui::MacEditText::deleteSelection() {
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

void MacGui::MacEditText::selectAll() {
	_caretPos = 0;
	_selectLen = _text.size();
	setRedraw();
}

void MacGui::MacEditText::draw(bool drawFocused) {
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

void MacGui::MacEditText::handleMouseDown(Common::Event &event) {
	int oldSelectLen = _selectLen;
	int oldCaretPos = _caretPos;

	_caretPos = getTextPosFromMouse(event.mouse.x, event.mouse.y);
	_selectLen = 0;

	if (_selectLen != oldSelectLen || _caretPos != oldCaretPos)
		setRedraw();
}

bool MacGui::MacEditText::handleDoubleClick(Common::Event &event) {
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

bool MacGui::MacEditText::handleKeyDown(Common::Event &event) {
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

void MacGui::MacEditText::handleMouseHeld() {
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

void MacGui::MacEditText::handleMouseMove(Common::Event &event) {
	updateSelection(event.mouse.x, event.mouse.y);
}

// ---------------------------------------------------------------------------
// Picture widget
// ---------------------------------------------------------------------------

MacGui::MacPicture::MacPicture(MacGui::MacDialogWindow *window, Common::Rect bounds, int id, bool enabled) : MacWidget(window, bounds, "Picture", enabled) {
	_picture = _window->_gui->loadPict(id);
}

MacGui::MacPicture::~MacPicture() {
	if (_picture) {
		_picture->free();
		delete _picture;
	}
}

void MacGui::MacPicture::draw(bool drawFocused) {
	if (!_redraw && !_fullRedraw)
		return;

	debug(1, "MacGui::MacPicture: Drawing picture %d (_fullRedraw = %d, drawFocused = %d, _value = %d)", _id, _fullRedraw, drawFocused, _value);

	_window->drawSprite(_picture, _bounds.left, _bounds.top);

	_redraw = false;
	_fullRedraw = false;
}

// ---------------------------------------------------------------------------
// Slider base class
// ---------------------------------------------------------------------------

void MacGui::MacSliderBase::setValue(int value) {
	_value = CLIP(value, _minValue, _maxValue);
	_handlePos = calculatePosFromValue();
}

int MacGui::MacSliderBase::calculateValueFromPos() const {
	int posRange = _maxPos - _minPos;
	int posOffset = _handlePos - _minPos;

	int valueRange = _maxValue - _minValue;
	int valueOffset = (posRange / 2 + valueRange * posOffset) / posRange;

	return _minValue + valueOffset;
}

int MacGui::MacSliderBase::calculatePosFromValue() const {
	int valueRange = _maxValue - _minValue;
	int valueOffset = _value - _minValue;

	int posRange = _maxPos - _minPos;
	int posOffset = (valueRange / 2 + posRange * valueOffset) / valueRange;

	return _minPos + posOffset;
}

// ---------------------------------------------------------------------------
// Standard slider widget
// ---------------------------------------------------------------------------

MacGui::MacSlider::MacSlider(MacGui::MacDialogWindow *window, Common::Rect bounds, int minValue, int maxValue, int pageSize, bool enabled)
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

bool MacGui::MacSlider::findWidget(int x, int y) const {
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

Common::Rect MacGui::MacSlider::getHandleRect(int value) {
	int handlePos = value * (_boundsBody.bottom - _boundsBody.top - 16) / (_maxValue - _minValue);

	Common::Rect handleRect;

	handleRect.left = _boundsBody.left + 1;
	handleRect.top = _boundsBody.top + handlePos;
	handleRect.right = _boundsBody.right - 1;
	handleRect.bottom = handleRect.top + 16;

	return handleRect;
}

void MacGui::MacSlider::fill(Common::Rect r, bool inverted) {
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

void MacGui::MacSlider::draw(bool drawFocused) {
	if (!_redraw && !_fullRedraw)
		return;

	// There are several things that will trigger a redraw, but unlike
	// other widgets this one only handles full redraws. Everything else
	// is handled outside of draw().

	if (_fullRedraw) {
		debug(1, "MacGui::MacSlider: Drawing slider (_fullRedraw = %d, drawFocused = %d, _value = %d)", _fullRedraw, drawFocused, _value);

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

void MacGui::MacSlider::drawUpArrow(bool markAsDirty) {
	debug(1, "MacGui::MacSlider: Drawing up arrow (_upArrowPressed = %d, markAsDirty = %d)", _upArrowPressed, markAsDirty);

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

void MacGui::MacSlider::drawDownArrow(bool markAsDirty) {
	debug(1, "MacGui::MacSlider: Drawing down arrow (_downArrowPressed = %d, markAsDirty = %d)", _downArrowPressed, markAsDirty);

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

void MacGui::MacSlider::drawArrow(Common::Rect r, const uint16 *bitmap, bool markAsDirty) {
	Graphics::Surface *s = _window->innerSurface();

	r.grow(-1);

	s->fillRect(r, kWhite);
	drawBitmap(Common::Rect(r.left + 1, r.top + 2, r.right - 1, r.top + 12), bitmap, kBlack);

	if (markAsDirty)
		_window->markRectAsDirty(r);
}

void MacGui::MacSlider::eraseDragHandle() {
	Common::Rect r(_boundsBody.left + 1, _handlePos, _boundsBody.right - 1, _handlePos + 16);
	fill(r);
	_window->markRectAsDirty(r);
}

void MacGui::MacSlider::drawHandle(Common::Rect r) {
	debug(2, "MacGui::MacSlider::drawHandle(%d)", r.top);

	Graphics::Surface *s = _window->innerSurface();

	s->frameRect(r, kBlack);
	r.grow(-1);
	s->fillRect(r, kWhite);
}

void MacGui::MacSlider::redrawHandle(int oldValue, int newValue) {
	Common::Rect r = getHandleRect(oldValue);

	fill(r);
	_window->markRectAsDirty(r);

	r = getHandleRect(newValue);
	drawHandle(r);
	_window->markRectAsDirty(r);
}

void MacGui::MacSlider::handleMouseDown(Common::Event &event) {
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

bool MacGui::MacSlider::handleMouseUp(Common::Event &event) {
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

void MacGui::MacSlider::handleMouseMove(Common::Event &event) {
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
		// better. But it seems the orginal Mac widget would draw the
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

void MacGui::MacSlider::handleMouseHeld() {
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

void MacGui::MacSlider::handleWheelUp() {
	int oldValue = _value;

	_value = MAX(_minValue, _value - (_pageSize - 1));

	if (_value != oldValue)
		redrawHandle(oldValue, _value);
}

void MacGui::MacSlider::handleWheelDown() {
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

bool MacGui::MacPictureSlider::findWidget(int x, int y) const {
	// Once we start dragging the handle, any mouse position is considered
	// within the widget.

	if (_window->getFocusedWidget() == this)
		return true;

	return _bounds.contains(x, y);
}

void MacGui::MacPictureSlider::draw(bool drawFocused) {
	if (!_redraw && !_fullRedraw)
		return;

	debug(1, "MacGui::MacPictureSlider: Drawing slider %d (_fullRedraw = %d, drawFocused = %d, _value = %d)", _id, _fullRedraw, drawFocused, _value);

	if (_fullRedraw) {
		_window->drawSprite(_background->getPicture(), _bounds.left, _bounds.top);
		drawHandle();
	}

	_redraw = false;
	_fullRedraw = false;
}

void MacGui::MacPictureSlider::eraseHandle() {
	Common::Rect r = _handle->getBounds();
	int y = r.top - _bounds.top;
	int w = r.width();
	int h = r.height();

	Graphics::Surface *background = _background->getPicture();
	Graphics::Surface sprite = background->getSubArea(Common::Rect(_handlePos, y, _handlePos + w, y + h));
	_window->drawSprite(&sprite, _bounds.left + _handlePos, r.top);
}

void MacGui::MacPictureSlider::drawHandle() {
	Graphics::Surface *sprite = _handle->getPicture();
	Common::Rect r = _handle->getBounds();

	_window->drawSprite(sprite, _bounds.left + _handlePos, r.top);
}

void MacGui::MacPictureSlider::handleMouseDown(Common::Event &event) {
	int mouseX = event.mouse.x;
	int handleWidth = _handle->getBounds().width();

	if (mouseX >= _handlePos && mouseX < _handlePos + handleWidth)
		_grabOffset = event.mouse.x - _bounds.left - _handlePos;
	else
		_grabOffset = handleWidth / 2;

	handleMouseMove(event);
}

bool MacGui::MacPictureSlider::handleMouseUp(Common::Event &event) {
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

void MacGui::MacPictureSlider::handleMouseMove(Common::Event &event) {
	int newPos = CLIP<int>(event.mouse.x - _bounds.left - _grabOffset, _minX, _maxX);

	if (newPos != _handlePos) {
		eraseHandle();
		_handlePos = newPos;
		drawHandle();
	}
}

void MacGui::MacPictureSlider::handleWheelUp() {
	int newValue = MAX(_minValue, _value + 1);

	if (_value != newValue) {
		eraseHandle();
		setValue(newValue);
		drawHandle();
	}
}

void MacGui::MacPictureSlider::handleWheelDown() {
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

MacGui::MacListBox::MacListBox(MacGui::MacDialogWindow *window, Common::Rect bounds, Common::StringArray texts, bool enabled, bool contentUntouchable) : MacWidget(window, bounds, "ListBox", enabled), _texts(texts) {
	int pageSize = _bounds.height() / 16;

	int numSlots = MIN<int>(pageSize, texts.size());

	MacStaticText *tmp;
	for (int i = 0; i < numSlots; i++) {
		Common::Rect r(_bounds.left + 1, _bounds.top + 1 + 16 * i, _bounds.right - 16, _bounds.top + 1 + 16 * (i + 1));
		tmp = new MacStaticText(window, r, texts[i], enabled);
		if (contentUntouchable)
			tmp->setEnabled(false);
		_textWidgets.push_back(tmp);
	}

	_slider = new MacSlider(window, Common::Rect(_bounds.right - 16, _bounds.top, _bounds.right, _bounds.bottom), 0, texts.size() - pageSize, pageSize, enabled);

	// The widget value indicates the selected element
	_value = 0;
	updateTexts();
}

MacGui::MacListBox::~MacListBox() {
	_texts.clear();
	delete _slider;

	for (uint i = 0; i < _textWidgets.size(); i++)
		delete _textWidgets[i];
}

bool MacGui::MacListBox::findWidget(int x, int y) const {
	return MacWidget::findWidget(x, y) || _slider->findWidget(x, y);
}

void MacGui::MacListBox::setRedraw(bool fullRedraw) {
	MacWidget::setRedraw(fullRedraw);
	_slider->setRedraw(fullRedraw);

	for (uint i = 0; i < _textWidgets.size(); i++)
		_textWidgets[i]->setRedraw(fullRedraw);
}

void MacGui::MacListBox::updateTexts() {
	int offset = _slider->getValue();

	for (uint i = 0; i < _textWidgets.size(); i++) {
		_textWidgets[i]->setText(_texts[i + offset]);

		if (_textWidgets[i]->isEnabled() && (int)i + offset == _value)
			_textWidgets[i]->setColor(kWhite, kBlack);
		else
			_textWidgets[i]->setColor(kBlack, kWhite);
	}
}

void MacGui::MacListBox::draw(bool drawFocused) {
	for (uint i = 0; i < _textWidgets.size(); i++)
		_textWidgets[i]->draw(drawFocused);

	_slider->draw(drawFocused);

	if (!_redraw && !_fullRedraw)
		return;

	debug(1, "MacGui::MacListBox: Drawing list box (_fullRedraw = %d, drawFocused = %d)", _fullRedraw, drawFocused);

	Graphics::Surface *s = _window->innerSurface();

	s->hLine(_bounds.left, _bounds.top, _bounds.right - 17, kBlack);
	s->hLine(_bounds.left, _bounds.bottom - 1, _bounds.right - 17, kBlack);
	s->vLine(_bounds.left, _bounds.top + 1, _bounds.bottom - 2, kBlack);

	_redraw = false;
	_fullRedraw = false;

	_window->markRectAsDirty(_bounds);
}

void MacGui::MacListBox::handleMouseDown(Common::Event &event) {
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

bool MacGui::MacListBox::handleDoubleClick(Common::Event &event) {
	for (uint i = 0; i < _textWidgets.size(); i++) {
		if (_textWidgets[i]->findWidget(event.mouse.x, event.mouse.y))
			return true;
	}

	return false;
}

bool MacGui::MacListBox::handleMouseUp(Common::Event &event) {
	if (_sliderFocused) {
		int oldValue = _slider->getValue();

		_sliderFocused = false;
		_slider->handleMouseUp(event);

		if (_slider->getValue() != oldValue)
			updateTexts();
	}

	return false;
}

void MacGui::MacListBox::handleMouseMove(Common::Event &event) {
	if (_sliderFocused) {
		int oldValue = _slider->getValue();

		_slider->handleMouseMove(event);

		if (_slider->getValue() != oldValue)
			updateTexts();
	}
}

void MacGui::MacListBox::handleMouseHeld() {
	if (_sliderFocused) {
		int oldValue = _slider->getValue();

		_slider->handleMouseHeld();

		if (_slider->getValue() != oldValue)
			updateTexts();
	}
}

void MacGui::MacListBox::handleWheelUp() {
	handleWheel(-1);
}

void MacGui::MacListBox::handleWheelDown() {
	handleWheel(1);
}

void MacGui::MacListBox::handleWheel(int distance) {
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

bool MacGui::MacListBox::handleKeyDown(Common::Event &event) {
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

// ---------------------------------------------------------------------------
// Dialog window
//
// This can either be used as a modal dialog (options, etc.), or as a framed
// drawing area (about and pause). It can not be dragged.
// ---------------------------------------------------------------------------

MacGui::MacDialogWindow::MacDialogWindow(MacGui *gui, OSystem *system, Graphics::Surface *from, Common::Rect bounds, MacDialogWindowStyle style) : _gui(gui), _system(system), _from(from), _bounds(bounds) {
	_pauseToken = _gui->_vm->pauseEngine();

	// Remember if the screen was shaking. We don't use the SCUMM engine's
	// own _shakeTempSavedState to remember this, because there may be
	// nested dialogs. They each need their own.

	_shakeWasEnabled = _gui->_vm->_shakeEnabled;
	_gui->_vm->setShake(0);

	_backup = new Graphics::Surface();
	_backup->create(bounds.width(), bounds.height(), Graphics::PixelFormat::createFormatCLUT8());
	_backup->copyRectToSurface(*_from, 0, 0, bounds);

	_margin = (style == kStyleNormal) ? 6 : 4;

	_surface = _from->getSubArea(bounds);
	bounds.grow(-_margin);
	_innerSurface = _from->getSubArea(bounds);

	_dirtyRects.clear();

	Graphics::Surface *s = surface();
	Common::Rect r = Common::Rect(s->w, s->h);

	r.grow(-1);
	s->fillRect(r, kWhite);

	if (style == MacGui::kStyleNormal) {
		int growths[] = { 1, -3, -1 };

		for (int i = 0; i < ARRAYSIZE(growths); i++) {
			r.grow(growths[i]);

			s->hLine(r.left, r.top, r.right - 1, kBlack);
			s->hLine(r.left, r.bottom - 1, r.right - 1, kBlack);
			s->vLine(r.left, r.top + 1, r.bottom - 2, kBlack);
			s->vLine(r.right - 1, r.top + 1, r.bottom - 2, kBlack);
		}
	} else if (style == MacGui::kStyleRounded) {
		r.grow(1);

		for (int i = 0; i < 2; i++) {
			s->hLine(r.left + 2, r.top, r.right - 3, kBlack);
			s->hLine(r.left + 2, r.bottom - 1, r.right - 3, kBlack);
			s->vLine(r.left, r.top + 2, r.bottom - 3, kBlack);
			s->vLine(r.right - 1, r.top + 2, r.bottom - 3, kBlack);
			s->setPixel(r.left + 1, r.top + 1, kBlack);
			s->setPixel(r.left + 1, r.bottom - 2, kBlack);
			s->setPixel(r.right - 2, r.top + 1, kBlack);
			s->setPixel(r.right - 2, r.bottom - 2, kBlack);
			r.grow(-2);
		}
	}
}

MacGui::MacDialogWindow::~MacDialogWindow() {
	if (!CursorMan.isVisible())
		CursorMan.showMouse(true);

	CursorMan.showMouse(_cursorWasVisible);
	_gui->_windowManager->popCursor();

	copyToScreen(_backup);
	_backup->free();
	delete _backup;

	for (uint i = 0; i < _widgets.size(); i++)
		delete _widgets[i];

	_widgets.clear();
	_pauseToken.clear();
	_gui->_vm->setShake(_shakeWasEnabled);
}

void MacGui::MacDialogWindow::copyToScreen(Graphics::Surface *s) const {
	if (s) {
		_from->copyRectToSurface(*s, _bounds.left, _bounds.top, Common::Rect(_bounds.width(), _bounds.height()));
	}
	_system->copyRectToScreen(_from->getBasePtr(_bounds.left, _bounds.top), _from->pitch, _bounds.left, _bounds.top, _bounds.width(), _bounds.height());
}

void MacGui::MacDialogWindow::show() {
	_visible = true;
	copyToScreen();
	_dirtyRects.clear();
	_gui->_windowManager->pushCursor(Graphics::MacGUIConstants::kMacCursorArrow);
	_cursorWasVisible = CursorMan.showMouse(true);
}

void MacGui::MacDialogWindow::setFocusedWidget(int x, int y) {
	int nr = findWidget(x, y);
	if (nr >= 0) {
		_focusedWidget = _widgets[nr];
		_focusClick.x = x;
		_focusClick.y = y;
		_focusedWidget->getFocus();
	} else
		clearFocusedWidget();
}

void MacGui::MacDialogWindow::clearFocusedWidget() {
	if (_focusedWidget) {
		_focusedWidget->loseFocus();
		_focusedWidget = nullptr;
		_focusClick.x = -1;
		_focusClick.y = -1;
	}
}

int MacGui::MacDialogWindow::findWidget(int x, int y) const {
	for (uint i = 0; i < _widgets.size(); i++) {
		if (_widgets[i]->isEnabled() && _widgets[i]->isVisible() && _widgets[i]->findWidget(x, y))
			return i;
	}

	return -1;
}

MacGui::MacButton *MacGui::MacDialogWindow::addButton(Common::Rect bounds, Common::String text, bool enabled) {
	MacGui::MacButton *button = new MacButton(this, bounds, text, enabled);
	_widgets.push_back(button);
	return button;
}

MacGui::MacCheckbox *MacGui::MacDialogWindow::addCheckbox(Common::Rect bounds, Common::String text, bool enabled) {
	MacGui::MacCheckbox *checkbox = new MacCheckbox(this, bounds, text, enabled);
	_widgets.push_back(checkbox);
	return checkbox;
}

MacGui::MacStaticText *MacGui::MacDialogWindow::addStaticText(Common::Rect bounds, Common::String text, bool enabled) {
	MacGui::MacStaticText *staticText = new MacStaticText(this, bounds, text, enabled);
	_widgets.push_back(staticText);
	return staticText;
}

MacGui::MacEditText *MacGui::MacDialogWindow::addEditText(Common::Rect bounds, Common::String text, bool enabled) {
	MacGui::MacEditText *editText = new MacEditText(this, bounds, text, enabled);
	_widgets.push_back(editText);
	return editText;
}

MacGui::MacPicture *MacGui::MacDialogWindow::addPicture(Common::Rect bounds, int id, bool enabled) {
	MacGui::MacPicture *picture = new MacPicture(this, bounds, id, false);
	_widgets.push_back(picture);
	return picture;
}

MacGui::MacSlider *MacGui::MacDialogWindow::addSlider(int x, int y, int h, int minValue, int maxValue, int pageSize, bool enabled) {
	MacGui::MacSlider *slider = new MacSlider(this, Common::Rect(x, y, x + 16, y + h), minValue, maxValue, pageSize, enabled);
	_widgets.push_back(slider);
	return slider;
}

MacGui::MacPictureSlider *MacGui::MacDialogWindow::addPictureSlider(int backgroundId, int handleId, bool enabled, int minX, int maxX, int minValue, int maxValue, int leftMargin, int rightMargin) {
	MacPicture *background = (MacPicture *)_widgets[backgroundId];
	MacPicture *handle = (MacPicture *)_widgets[handleId];

	background->setVisible(false);
	handle->setVisible(false);

	MacGui::MacPictureSlider *slider = new MacPictureSlider(this, background, handle, enabled, minX, maxX, minValue, maxValue, leftMargin, rightMargin);
	_widgets.push_back(slider);
	return slider;
}

void MacGui::MacDialogWindow::markRectAsDirty(Common::Rect r) {
	_dirtyRects.push_back(r);
}

MacGui::MacListBox *MacGui::MacDialogWindow::addListBox(Common::Rect bounds, Common::StringArray texts, bool enabled, bool contentUntouchable) {
	MacGui::MacListBox *listBox = new MacListBox(this, bounds, texts, enabled, contentUntouchable);
	_widgets.push_back(listBox);
	return listBox;
}

void MacGui::MacDialogWindow::drawBeamCursor() {
	int x0 = _beamCursorPos.x - _beamCursorHotspotX;
	int y0 = _beamCursorPos.y - _beamCursorHotspotY;
	int x1 = x0 + _beamCursor->w;
	int y1 = y0 + _beamCursor->h;

	_beamCursor->copyRectToSurface(*(_gui->surface()), 0, 0, Common::Rect(x0, y0, x1, y1));

	const byte beam[] = {
		0,  0,  1,  0,  5,  0,  6,  0,  2,  1,  4,  1,  3,  2,  3,  3,
		3,  4,  3,  5,  3,  6,  3,  7,  3,  8,  3,  9,  3, 10,  3, 11,
		3, 12,  3, 13,  2, 14,  4, 14,  0, 15,  1, 15,  5, 15,  6, 15
	};

	for (int i = 0; i < ARRAYSIZE(beam); i += 2) {
		uint32 color = _beamCursor->getPixel(beam[i], beam[i + 1]);

		// From looking at a couple of colors, it seems this is how
		// the colors are inverted for 0-15. I'm just going to assume
		// that the same method will work reasonably well for the
		// custom colors. If there's anything else, just make it black.

		if (color <= 15)
			color = 15 - color;
		else if (color > kCustomColor && color <= kCustomColor + 15)
			color = kCustomColor + 15 - color;
		else
			color = kBlack;

		_beamCursor->setPixel(beam[i], beam[i + 1], color);
	}

	int srcX = 0;
	int srcY = 0;

	if (x0 < 0) {
		srcX = -x0;
		x0 = 0;
	}

	x1 = MIN(x1, 640);

	if (y0 < 0) {
		srcY = -y0;
		y0 = 0;
	}

	y1 = MIN(y1, 400);

	_system->copyRectToScreen(_beamCursor->getBasePtr(srcX, srcY), _beamCursor->pitch, x0, y0, x1 - x0, y1 - y0);
}

void MacGui::MacDialogWindow::undrawBeamCursor() {
		int x0 = _beamCursorPos.x - _beamCursorHotspotX;
		int y0 = _beamCursorPos.y - _beamCursorHotspotY;
		int x1 = x0 + _beamCursor->w;
		int y1 = y0 + _beamCursor->h;

		x0 = MAX(x0, 0);
		x1 = MIN(x1, 640);
		y0 = MAX(y0, 0);
		y1 = MIN(y1, 400);

		Graphics::Surface *screen = _gui->surface();

		_system->copyRectToScreen(screen->getBasePtr(x0, y0), screen->pitch, x0, y0, x1 - x0, y1 - y0);
}

void MacGui::MacDialogWindow::update(bool fullRedraw) {
	for (uint i = 0; i < _widgets.size(); i++) {
		if (_widgets[i]->isVisible())
			_widgets[i]->draw();
	}

	if (fullRedraw) {
		_dirtyRects.clear();
		markRectAsDirty(Common::Rect(_innerSurface.w, _innerSurface.h));
	}

	for (uint i = 0; i < _dirtyRects.size(); i++) {
		_system->copyRectToScreen(
			_innerSurface.getBasePtr(_dirtyRects[i].left, _dirtyRects[i].top),
			_innerSurface.pitch,
			_bounds.left + _margin + _dirtyRects[i].left, _bounds.top + _margin + _dirtyRects[i].top,
			_dirtyRects[i].width(), _dirtyRects[i].height());
	}

	_dirtyRects.clear();

	if (_beamCursor) {
		if (_beamCursorVisible)
			undrawBeamCursor();

		_beamCursorPos = _realMousePos;

		if (_beamCursorVisible)
			drawBeamCursor();
	}
}

void MacGui::MacDialogWindow::drawDottedHLine(int x0, int y, int x1) {
	Graphics::Surface *s = innerSurface();

	Color color[2];

	if (_gui->_vm->_renderMode == Common::kRenderMacintoshBW) {
		color[0] = kBlack;
		color[1] = kWhite;
	} else {
		color[0] = kDarkGray;
		color[1] = kLightGray;
	}

	for (int x = x0; x <= x1; x++)
		s->setPixel(x, y, color[x & 1]);
}

void MacGui::MacDialogWindow::fillPattern(Common::Rect r, uint16 pattern) {
	for (int y = r.top; y < r.bottom; y++) {
		for (int x = r.left; x < r.right; x++) {
			int bit = 0x8000 >> (4 * (y % 4) + (x % 4));
			_innerSurface.setPixel(x, y, (pattern & bit) ? kBlack : kWhite);
		}
	}

	markRectAsDirty(r);
}

int MacGui::MacDialogWindow::runDialog(Common::Array<int> &deferredActionIds) {
	// The first time the function is called, show the dialog and redraw
	// all widgets completely.

	deferredActionIds.clear();

	if (!_visible) {
		show();

		Common::Rect windowBounds(_innerSurface.w, _innerSurface.h);

		for (uint i = 0; i < _widgets.size(); i++) {
			_widgets[i]->setId(i);

			if (_widgets[i]->isVisible()) {
				_widgets[i]->setRedraw(true);
				_widgets[i]->draw();
			}
		}
	}

	// Run the dialog until something interesting happens to a widget. It's
	// up to the caller to repeat the calls to runDialog() until the dialog
	// has ended.

	bool buttonPressed = false;
	uint32 nextMouseRepeat = 0;

	while (!_gui->_vm->shouldQuit()) {
		Common::Event event;
		int widgetId = -1;

		while (_system->getEventManager()->pollEvent(event)) {
			// Adjust mouse coordinates to the dialog window

			if (Common::isMouseEvent(event)) {
				_realMousePos.x = event.mouse.x;
				_realMousePos.y = event.mouse.y;

				event.mouse.x -= (_bounds.left + _margin);
				event.mouse.y -= (_bounds.top + _margin);

				_oldMousePos = _mousePos;

				_mousePos.x = event.mouse.x;
				_mousePos.y = event.mouse.y;

				// Update engine mouse position
				_gui->_vm->_mouse.x = _realMousePos.x / 2;
				_gui->_vm->_mouse.y = _realMousePos.y / 2;
			}

			int w;

			switch (event.type) {
			case Common::EVENT_LBUTTONDOWN:
				// When a widget is clicked, it becomes the
				// focused widget. Focused widgets are often
				// indicated by some sort of highlight, e.g.
				// buttons become inverted.
				//
				// This highlight is usually only shown while
				// the mouse is within the widget bounds, but
				// as long as it remains focused it can regain
				// the highlight by moving the cursor back into
				// the widget bounds again.
				//
				// It's unclear to me if Macs should handle
				// double clicks on mouse down, mouse up or
				// both.

				buttonPressed = true;
				nextMouseRepeat = _system->getMillis() + 40;
				setFocusedWidget(event.mouse.x, event.mouse.y);
				if (_focusedWidget) {
					_focusedWidget->handleMouseDown(event);

					uint32 now = _system->getMillis();
					bool doubleClick =
						(now - _lastClickTime < 500 &&
						ABS(event.mouse.x - _lastClickPos.x) < 5 &&
						ABS(event.mouse.y - _lastClickPos.y) < 5);

					_lastClickTime = _system->getMillis();
					_lastClickPos.x = event.mouse.x;
					_lastClickPos.y = event.mouse.y;

					if (doubleClick && _focusedWidget->handleDoubleClick(event))
						return _focusedWidget->getId();
				}
				break;

			case Common::EVENT_LBUTTONUP:
				buttonPressed = false;
				updateCursor();

				// Only the focused widget receives the button
				// up event. If the widget handles the event,
				// control is passed back to the caller of
				// runDialog() so that it can react, e.g. to
				// the user clicking the "Okay" button.

				if (_focusedWidget) {
					MacWidget *widget = _focusedWidget;

					updateCursor();

					if (widget->findWidget(event.mouse.x, event.mouse.y)) {
						widgetId = widget->getId();
						if (widget->handleMouseUp(event)) {
							clearFocusedWidget();
							return widgetId;
						}
					}

					clearFocusedWidget();
				}

				break;

			case Common::EVENT_MOUSEMOVE:
				// The "beam" cursor can be hidden, but will
				// become visible again when the user moves
				// the mouse.

				if (_beamCursor)
					_beamCursorVisible = true;

				// Only the focused widget receives mouse move
				// events, and then only if the mouse is within
				// the widget's area of control. This are of
				// control is usually the widget bounds, but
				// widgets can redefine findWidget() to change
				// this, e.g. for slider widgets.

				if (_focusedWidget) {
					bool wasActive = _focusedWidget->findWidget(_oldMousePos.x, _oldMousePos.y);
					bool isActive = _focusedWidget->findWidget(_mousePos.x, _mousePos.y);

					if (wasActive != isActive)
						_focusedWidget->setRedraw();

					// The widget gets mouse events while
					// it's active, but also one last one
					// when it becomes inactive.

					if (isActive || wasActive)
						_focusedWidget->handleMouseMove(event);
				} else {
					updateCursor();
				}

				break;

			case Common::EVENT_WHEELUP:
				if (!_gui->_vm->enhancementEnabled(kEnhUIUX) || _focusedWidget)
					break;

				w = findWidget(event.mouse.x, event.mouse.y);
				if (w >= 0)
					_widgets[w]->handleWheelUp();

				break;

			case Common::EVENT_WHEELDOWN:
				if (!_gui->_vm->enhancementEnabled(kEnhUIUX) || _focusedWidget)
					break;

				w = findWidget(event.mouse.x, event.mouse.y);
				if (w >= 0)
					_widgets[w]->handleWheelDown();

				break;

			case Common::EVENT_KEYDOWN:
				// Ignore keyboard while mouse is pressed
				if (buttonPressed)
					break;

				// Handle default button
				if (event.kbd.keycode == Common::KEYCODE_RETURN) {
					MacWidget *widget = getDefaultWidget();
					if (widget && widget->isEnabled() && widget->isVisible()) {
						for (int i = 0; i < 2; i++) {
							widget->setRedraw();
							widget->draw(i == 0);
							update();

							for (int j = 0; j < 10; j++) {
								_system->delayMillis(10);
								_system->updateScreen();
							}
						}

						return widget->getId();
					}
				}

				// Otherwise, give widgets a chance to react
				// to key presses. All widgets get a chance,
				// whether or not they are focused. This may
				// be a bad idea, if there is ever more than
				// one edit text widget in the window.
				//
				// Typing hides the "beam" cursor.

				for (uint i = 0; i < _widgets.size(); i++) {
					if (_widgets[i]->isVisible() && _widgets[i]->isEnabled() && _widgets[i]->handleKeyDown(event)) {
						if (_beamCursor) {
							_beamCursorVisible = false;
							undrawBeamCursor();
						}

						if (_widgets[i]->shouldDeferAction())
							deferredActionIds.push_back(_widgets[i]->getId());

						break;
					}
				}

				if (!deferredActionIds.empty())
					return -2;

				break;

			default:
				break;
			}
		}

		// A focused widget implies that the mouse button is being
		// held down. It must be active and visible, so it can receive
		// mouse repeat events, e.g. for holding down scroll buttons
		// on a slider widget.

		if (_focusedWidget && _system->getMillis() > nextMouseRepeat) {
			nextMouseRepeat = _system->getMillis() + 40;
			_focusedWidget->handleMouseHeld();
		}

		_system->delayMillis(10);
		update();
		_system->updateScreen();
	}

	return -1;
}

void MacGui::MacDialogWindow::updateCursor() {
	int mouseOverWidget = findWidget(_mousePos.x, _mousePos.y);
	bool useBeamCursor = (mouseOverWidget >= 0 && _widgets[mouseOverWidget]->useBeamCursor());

	if (useBeamCursor && !_beamCursor) {
		CursorMan.showMouse(false);
		_beamCursor = new Graphics::Surface();
		_beamCursor->create(7, 16, Graphics::PixelFormat::createFormatCLUT8());
		_beamCursorVisible = true;
		_beamCursorPos = _realMousePos;
	} else if (!useBeamCursor && _beamCursor) {
		CursorMan.showMouse(true);

		undrawBeamCursor();
		_beamCursor->free();
		delete _beamCursor;
		_beamCursor = nullptr;
		_beamCursorVisible = false;
	}
}

void MacGui::MacDialogWindow::drawSprite(const Graphics::Surface *sprite, int x, int y) {
	_innerSurface.copyRectToSurface(*sprite, x, y, Common::Rect(sprite->w, sprite->h));
	markRectAsDirty(Common::Rect(x, y, x + sprite->w, y + sprite->h));
}

void MacGui::MacDialogWindow::drawSprite(const Graphics::Surface *sprite, int x, int y, Common::Rect(clipRect)) {
	Common::Rect subRect(sprite->w, sprite->h);

	if (x < clipRect.left) {
		subRect.left += (clipRect.left - x);
		x = clipRect.left;
	}

	if (y < clipRect.top) {
		subRect.top += (clipRect.top - y);
		y = clipRect.top;
	}

	if (x + sprite->w >= clipRect.right) {
		subRect.right -= (x + sprite->w - clipRect.right);
	}

	if (y + sprite->h >= clipRect.bottom) {
		subRect.bottom -= (y + sprite->h - clipRect.bottom);
	}

	if (subRect.width() > 0 && subRect.height() > 0) {
		_innerSurface.copyRectToSurface(*sprite, x, y, subRect);
		markRectAsDirty(Common::Rect(x, y, x + subRect.width(), y + subRect.height()));
	}
}

void MacGui::MacDialogWindow::plotPixel(int x, int y, int color, void *data) {
	MacGui::MacDialogWindow *window = (MacGui::MacDialogWindow *)data;
	Graphics::Surface *s = window->innerSurface();
	s->setPixel(x, y, color);
}

// I don't know if the original actually used two different plot functions, one
// to fill and one to darken (used to draw over the text screens). It's such a
// subtle effect that I suspect it was just doing some different magic, maybe
// with XOR, but I couldn't get that to work by eye only.

void MacGui::MacDialogWindow::plotPattern(int x, int y, int pattern, void *data) {
	const uint16 patterns[] = {
		0x0000, 0x2828, 0xA5A5, 0xD7D7,
		0xFFFF,	0xD7D7, 0xA5A5, 0x2828
	};

	MacGui::MacDialogWindow *window = (MacGui::MacDialogWindow *)data;
	Graphics::Surface *s = window->innerSurface();
	int bit = 0x8000 >> (4 * (y % 4) + (x % 4));
	if (patterns[pattern] & bit)
		s->setPixel(x, y, kBlack);
	else
		s->setPixel(x, y, kWhite);
}

void MacGui::MacDialogWindow::plotPatternDarkenOnly(int x, int y, int pattern, void *data) {
	const uint16 patterns[] = {
		0x0000, 0x2828, 0xA5A5, 0xD7D7, 0xFFFF
	};

	MacGui::MacDialogWindow *window = (MacGui::MacDialogWindow *)data;
	Graphics::Surface *s = window->innerSurface();
	int bit = 0x8000 >> (4 * (y % 4) + (x % 4));
	if (patterns[pattern] & bit)
		s->setPixel(x, y, kBlack);
}

void MacGui::MacDialogWindow::drawTexts(Common::Rect r, const TextLine *lines) {
	if (!lines)
		return;

	Graphics::Surface *s = innerSurface();

	for (int i = 0; lines[i].str; i++) {
		const Graphics::Font *f1 = nullptr;
		const Graphics::Font *f2 = nullptr;

		switch (lines[i].style) {
		case kStyleHeader:
			f1 = _gui->getFont(kAboutFontHeaderOutside);
			f2 = _gui->getFont(kAboutFontHeaderInside);
			break;
		case kStyleBold:
			f1 = _gui->getFont(kAboutFontBold);
			break;
		case kStyleExtraBold:
			f1 = _gui->getFont(kAboutFontExtraBold);
			break;
		case kStyleRegular:
			f1 = _gui->getFont(kAboutFontRegular);
			break;
		}

		const char *msg = lines[i].str;
		int x = r.left + lines[i].x;
		int y = r.top + lines[i].y;
		Graphics::TextAlign align = lines[i].align;
		int width = r.right - x;

		if (lines[i].style == kStyleHeader) {
			f1->drawString(s, msg, x - 1, y + 1, width, kBlack, align);
			f2->drawString(s, msg, x + 1, y + 1, width, kBlack, align);
			f1->drawString(s, msg, x - 2, y, width, kBlack, align);
			f2->drawString(s, msg, x, y, width, kWhite, align);
		} else {
			f1->drawString(s, msg, x, y, width, kBlack, align);

			if (lines[i].style == kStyleExtraBold)
				f1->drawString(s, msg, x + 1, y, width, kBlack, align);
		}
	}
}

void MacGui::MacDialogWindow::drawTextBox(Common::Rect r, const TextLine *lines, int arc) {
	Graphics::drawRoundRect(r, arc, kWhite, true, plotPixel, this);
	Graphics::drawRoundRect(r, arc, kBlack, false, plotPixel, this);
	markRectAsDirty(r);

	drawTexts(r, lines);
}

// ===========================================================================
// Base class for Macintosh game user interface. Handles menus, fonts, image
// loading, etc.
// ===========================================================================

MacGui::MacGui(ScummEngine *vm, Common::String resourceFile) : _vm(vm), _system(_vm->_system), _surface(_vm->_macScreen), _resourceFile(resourceFile) {
	_fonts.clear();

	// kMacRomanConversionTable is a conversion table from Mac Roman
	// 128-255 to unicode. What we need, however, is a mapping from
	// unicode 160-255 to Mac Roman.

	for (int i = 0; i < ARRAYSIZE(_unicodeToMacRoman); i++)
		_unicodeToMacRoman[i] = 0;

	for (int i = 0; i < ARRAYSIZE(Common::kMacRomanConversionTable); i++) {
		int unicode = Common::kMacRomanConversionTable[i];

		if (unicode >= 160 && unicode <= 255)
			_unicodeToMacRoman[unicode - 160] = 128 + i;
	}
}

MacGui::~MacGui() {
	delete _windowManager;
}

int MacGui::toMacRoman(int unicode) const {
	if (unicode >= 32 && unicode <= 127)
		return unicode;

	if (unicode < 160 || unicode > 255)
		return 0;

	int macRoman = _unicodeToMacRoman[unicode - 160];

	// These characters are defined in Mac Roman, but apparently not
	// present in older fonts like Chicago?

	if (macRoman >= 0xD9 && macRoman != 0xF0)
		macRoman = 0;

	return macRoman;
}

void MacGui::setPalette(const byte *palette, uint size) {
	_windowManager->passPalette(palette, size);
}

bool MacGui::handleEvent(Common::Event event) {
	return _windowManager->processEvent(event);
}

int MacGui::delay(uint32 ms) {
	uint32 to;

	to = _system->getMillis() + ms;

	while (ms == 0 || _system->getMillis() < to) {
		Common::Event event;

		while (_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_QUIT:
				return 2;

			case Common::EVENT_LBUTTONDOWN:
				return 1;

			default:
				break;
			}
		}

		uint32 delta = to - _system->getMillis();

		if (delta > 0) {
			_system->delayMillis(MIN<uint32>(delta, 10));
			_system->updateScreen();
		}
	}

	return 0;
}

// --------------------------------------------------------------------------
// Menu handling
//
// In the original, the menu was activated by pressing the "Command key".
// This does not seem very friendly to touch devices, so we instead use the
// "mouse over" method in the Mac Window Manager class.
//
// TODO: Ideally we should handle both, but I don't know if there's a way for
//       them to coexist.
// --------------------------------------------------------------------------

void MacGui::menuCallback(int id, Common::String &name, void *data) {
	((MacGui *)data)->handleMenu(id, name);
}

void MacGui::initialize() {
	uint32 menuMode = Graphics::kWMModeNoDesktop | Graphics::kWMModeAutohideMenu | Graphics::kWMModalMenuMode | Graphics::kWMModeNoCursorOverride;

	// Allow a more modern UX: the menu doesn't close if the mouse accidentally goes outside the menu area
	if (_vm->enhancementEnabled(kEnhUIUX))
		menuMode |= Graphics::kWMModeWin95 | Graphics::kWMModeForceMacFontsInWin95 | Graphics::kWMModeForceMacBorder;

	_windowManager = new Graphics::MacWindowManager(menuMode);
	_windowManager->setEngine(_vm);
	_windowManager->setScreen(640, _vm->_useMacScreenCorrectHeight ? 480 : 400);

	if (_vm->isUsingOriginalGUI()) {
		_windowManager->setMenuHotzone(Common::Rect(640, 23));
		_windowManager->setMenuDelay(250000);

		Common::MacResManager resource;
		Graphics::MacMenu *menu = _windowManager->addMenu();

		resource.open(_resourceFile);

		// Add the Apple menu

		const Graphics::MacMenuData menuSubItems[] = {
			{ 0, NULL, 0, 0, false }
		};

		// TODO: This can be found in the STRS resource
		Common::String aboutMenuDef = "About " + name() + "...<B;(-";

		if (_vm->_game.id == GID_LOOM) {
			aboutMenuDef += ";";

			if (!_vm->enhancementEnabled(kEnhUIUX))
				aboutMenuDef += "(";

			aboutMenuDef += "Drafts Inventory";
		}

		menu->addStaticMenus(menuSubItems);
		menu->createSubMenuFromString(0, aboutMenuDef.c_str(), 0);

		menu->setCommandsCallback(menuCallback, this);

		for (int i = 129; i <= 130; i++) {
			Common::SeekableReadStream *res = resource.getResource(MKTAG('M', 'E', 'N', 'U'), i);

			if (!res)
				continue;

			Common::StringArray *menuDef = Graphics::MacMenu::readMenuFromResource(res);
			Common::String name = menuDef->operator[](0);
			Common::String string = menuDef->operator[](1);
			int id = menu->addMenuItem(nullptr, name);
			menu->createSubMenuFromString(id, string.c_str(), 0);

			delete menuDef;
			delete res;
		}

		resource.close();

		// Assign sensible IDs to the menu items

		int numberOfMenus = menu->numberOfMenus();

		for (int i = 0; i < numberOfMenus; i++) {
			Graphics::MacMenuItem *item = menu->getMenuItem(i);
			int numberOfMenuItems = menu->numberOfMenuItems(item);
			int id = 100 * (i + 1);
			for (int j = 0; j < numberOfMenuItems; j++) {
				Graphics::MacMenuItem *subItem = menu->getSubMenuItem(item, j);
				Common::String name = menu->getName(subItem);

				if (!name.empty())
					menu->setAction(subItem, id++);
			}
		}
	}

	// Register custom fonts. The font family just happens to match the
	// printed name of the game.

	const Common::String fontFamily = name();

	const Common::Array<Graphics::MacFontFamily *> &fontFamilies = _windowManager->_fontMan->getFontFamilies();

	_windowManager->_fontMan->loadFonts(_resourceFile);

	for (uint i = 0; i < fontFamilies.size(); i++) {
		if (fontFamilies[i]->getName() == fontFamily) {
			_gameFontId = _windowManager->_fontMan->registerFontName(fontFamily, fontFamilies[i]->getFontFamilyId());
			break;
		}
	}
}

bool MacGui::handleMenu(int id, Common::String &name) {
	// This menu item (e.g. a menu separator) has no action, so it's
	// handled trivially.
	if (id == 0)
		return true;

	// This is how we keep the menu bar visible.
	Graphics::MacMenu *menu = _windowManager->getMenu();
	menu->closeMenu();
	menu->setActive(true);
	menu->setVisible(true);
	updateWindowManager();

	int saveSlotToHandle = -1;
	Common::String savegameName;

	switch (id) {
	case 100:	// About
		runAboutDialog();
		return true;

	case 200:	// Open
		if (runOpenDialog(saveSlotToHandle)) {
			if (saveSlotToHandle > -1) {
				_vm->loadGameState(saveSlotToHandle);
				if (_vm->_game.id == GID_INDY3)
					((ScummEngine_v4 *)_vm)->updateIQPoints();
			}
		}

		return true;

	case 201:	// Save
		if (runSaveDialog(saveSlotToHandle, savegameName)) {
			if (saveSlotToHandle > -1) {
				_vm->saveGameState(saveSlotToHandle, savegameName);
			}
		}

		return true;

	case 202:	// Restart
		if (runRestartDialog())
			_vm->restart();
		return true;

	case 203:	// Pause
		if (!_vm->_messageBannerActive)
			_vm->mac_showOldStyleBannerAndPause(_vm->getGUIString(gsPause), -1);
		return true;

	// In the original, the Edit menu is active during save dialogs, though
	// only Cut, Copy and Paste.

	case 300:	// Undo
	case 301:	// Cut
	case 302:	// Copy
	case 303:	// Paste
	case 304:	// Clear
		return true;
	}

	return false;
}

void MacGui::updateWindowManager() {
	Graphics::MacMenu *menu = _windowManager->getMenu();

	if (!menu)
		return;

	// We want the arrow cursor for menus. Note that the menu triggers even
	// when the mouse is invisible, which may or may not be a bug. But the
	// original did allow you to open the menu with Alt even when the
	// cursor was visible, so for now it's a feature.

	bool isActive = _windowManager->isMenuActive();

	bool saveCondition = true;
	bool loadCondition = true;

	if (_vm->_game.id == GID_INDY3) {
		// Taken from Mac disasm...
		// The VAR(94) part tells us whether the copy protection has
		// failed or not, while the VAR(58) part uses bitmasks to enable
		// or disable saving and loading during normal gameplay.
		saveCondition = (_vm->VAR(58) & 0x01) && !(_vm->VAR(94) & 0x10);
		loadCondition = (_vm->VAR(58) & 0x02) && !(_vm->VAR(94) & 0x10);
	} else {
		// TODO: Complete LOOM with the rest of the proper code from disasm,
		// for now we only have the copy protection code and a best guess in place...
		//
		// Details:
		// VAR(221) & 0x4000:           Copy protection bit (the only thing I could confirm from the disasm)
		// VAR(VAR_VERB_SCRIPT) == 5:   Best guess... it prevents saving/loading from e.g. difficulty selection screen
		// _userPut > 0:                Best guess... it prevents saving/loading during cutscenes

		saveCondition = loadCondition =
			!(_vm->VAR(221) & 0x4000) &&
			(_vm->VAR(_vm->VAR_VERB_SCRIPT) == 5) &&
			(_vm->_userPut > 0);
	}

	bool canLoad = _vm->canLoadGameStateCurrently() && saveCondition;
	bool canSave = _vm->canSaveGameStateCurrently() && loadCondition;

	Graphics::MacMenuItem *gameMenu = menu->getMenuItem("Game");
	Graphics::MacMenuItem *loadMenu = menu->getSubMenuItem(gameMenu, 0);
	Graphics::MacMenuItem *saveMenu = menu->getSubMenuItem(gameMenu, 1);

	loadMenu->enabled = canLoad;
	saveMenu->enabled = canSave;

	if (isActive) {
		if (!_menuIsActive) {
			_cursorWasVisible = CursorMan.showMouse(true);
			_windowManager->pushCursor(Graphics::MacGUIConstants::kMacCursorArrow);
		}
	} else {
		if (_menuIsActive) {
			if (_windowManager->getCursorType() == Graphics::MacGUIConstants::kMacCursorArrow)
				_windowManager->popCursor();
			CursorMan.showMouse(_cursorWasVisible);
		}
	}

	_menuIsActive = isActive;
	_windowManager->draw();
}

// ---------------------------------------------------------------------------
// Font handling
// ---------------------------------------------------------------------------

const Graphics::Font *MacGui::getFont(FontId fontId) {
	const Graphics::Font *font = _fonts.getValOrDefault((int)fontId);

	if (font)
		return font;

	int id;
	int size;
	int slant;

	switch (fontId) {
	case kSystemFont:
		id = Graphics::kMacFontChicago;
		size = 12;
		slant = Graphics::kMacFontRegular;
		break;

	default:
		getFontParams(fontId, id, size, slant);
		break;
	}


	font = _windowManager->_fontMan->getFont(Graphics::MacFont(id, size, slant));
	_fonts[(int)fontId] = font;

	return font;
}

bool MacGui::getFontParams(FontId fontId, int &id, int &size, int &slant) const {
	switch (fontId) {
	case FontId::kAboutFontHeaderOutside:
		id = _gameFontId;
		size = 12;
		slant = Graphics::kMacFontItalic | Graphics::kMacFontBold | Graphics::kMacFontOutline;
		return true;

	case FontId::kAboutFontHeaderInside:
		id = _gameFontId;
		size = 12;
		slant = Graphics::kMacFontItalic | Graphics::kMacFontBold | Graphics::kMacFontExtend;
		return true;

	case FontId::kAboutFontBold:
		id = _gameFontId;
		size = 9;
		slant = Graphics::kMacFontRegular;
		return true;

	case FontId::kAboutFontExtraBold:
		id = _gameFontId;
		size = 9;
		slant = Graphics::kMacFontRegular | Graphics::kMacFontExtend;
		return true;

	case FontId::kAboutFontRegular:
		id = Graphics::kMacFontGeneva;
		size = 9;
		slant = Graphics::kMacFontRegular;
		return true;

	default:
		return false;
	}
}

// ---------------------------------------------------------------------------
// PICT loader
//
// ScummVM already has a PICT v2 loader, and we use that when necessary. But
// for PICT v1 we have our own for now.
//
// TODO: Investigate if PICT v1 and v2 can be handled by the standard PICT
//       loader.
// ---------------------------------------------------------------------------

Graphics::Surface *MacGui::loadPict(int id) {
	Common::MacResManager resource;
	Graphics::Surface *s = nullptr;

	resource.open(_resourceFile);

	Common::SeekableReadStream *res = resource.getResource(MKTAG('P', 'I', 'C', 'T'), id);

	// IQ logos are PICT v2
	if (id == 4000 || id == 4001) {
		Image::PICTDecoder pict;
		if (pict.loadStream(*res)) {
			const Graphics::Surface *s1 = pict.getSurface();
			const byte *palette = pict.getPalette();

			s = new Graphics::Surface();
			s->create(s1->w, s1->h, Graphics::PixelFormat::createFormatCLUT8());

			// The palette doesn't match the game's palette at all, so remap
			// the colors to the custom area of the palette. It's assumed that
			// only one such picture will be loaded at a time.
			//
			// But we still match black and white to 0 and 15 to make sure they
			// mach exactly.

			int black = -1;
			int white = -1;

			for (int i = 0; i < pict.getPaletteColorCount(); i++) {
				int r = palette[3 * i];
				int g = palette[3 * i + 1];
				int b = palette[3 * i + 2];

				if (r == 0 && g == 0 && b == 0)
					black = i;
				else if (r == 255 && g == 255 && b == 255)
					white = i;
			}

			if (palette) {
				_system->getPaletteManager()->setPalette(palette, kCustomColor, pict.getPaletteColorCount());

				for (int y = 0; y < s->h; y++) {
					for (int x = 0; x < s->w; x++) {
						int color = s1->getPixel(x, y);

						if (color == black)
							color = kBlack;
						else if (color == white)
							color = kWhite;
						else
							color = kCustomColor + color;

						s->setPixel(x, y, color);
					}
				}
			} else
				s->copyFrom(*s1);

		}
	} else {
		s = decodePictV1(res);
	}

	delete res;
	resource.close();

	return s;
}

Graphics::Surface *MacGui::decodePictV1(Common::SeekableReadStream *res) {
	uint16 size = res->readUint16BE();

	uint16 top = res->readUint16BE();
	uint16 left = res->readUint16BE();
	uint16 bottom = res->readUint16BE();
	uint16 right = res->readUint16BE();

	int width = right - left;
	int height = bottom - top;

	Graphics::Surface *s = new Graphics::Surface();
	s->create(right - left, bottom - top, Graphics::PixelFormat::createFormatCLUT8());

	bool endOfPicture = false;

	while (!endOfPicture) {
		byte opcode = res->readByte();
		byte value;
		int x1, x2, y1, y2;

		int x = 0;
		int y = 0;
		bool compressed = false;

		switch (opcode) {
		case 0x01: // clipRgn
			// The clip region is not important
			res->skip(res->readUint16BE() - 2);
			break;

		case 0x11: // picVersion
			value = res->readByte();
			assert(value == 1);
			break;

		case 0x99: // PackBitsRgn
			compressed = true;
			// Fall through

		case 0x91: // BitsRgn
			res->skip(2);	// Skip rowBytes

			y1 = res->readSint16BE();
			x1 = res->readSint16BE();
			y2 = res->readSint16BE();
			x2 = res->readSint16BE();

			res->skip(8);	// Skip srcRect
			res->skip(8);	// Skip dstRect
			res->skip(2);	// Skip mode
			res->skip(res->readUint16BE() - 2);	// Skip maskRgn

			if (!compressed) {
				for (y = y1; y < y2 && y < height; y++) {
					byte b = res->readByte();
					byte bit = 0x80;

					for (x = x1; x < x2 && x < width; x++) {
						if (b & bit)
							s->setPixel(x, y, kBlack);
						else
							s->setPixel(x, y, kWhite);

						bit >>= 1;

						if (bit == 0) {
							b = res->readByte();
							bit = 0x80;
						}
					}
				}
			} else {
				for (y = y1; y < y2 && y < height; y++) {
					x = x1;
					size = res->readByte();

					while (size > 0) {
						byte count = res->readByte();
						size--;

						bool repeat;

						if (count >= 128) {
							// Repeat value
							count = 256 - count;
							repeat = true;
							value = res->readByte();
							size--;
						} else {
							// Copy values
							repeat = false;
							value = 0;
						}

						for (int j = 0; j <= count; j++) {
							if (!repeat) {
								value = res->readByte();
								size--;
							}
							for (int k = 7; k >= 0 && x < x2 && x < width; k--, x++) {
								if (value & (1 << k))
									s->setPixel(x, y, kBlack);
								else
									s->setPixel(x, y, kWhite);
							}
						}
					}
				}
			}

			break;

		case 0xA0: // shortComment
			res->skip(2);
			break;

		case 0xFF: // EndOfPicture
			endOfPicture = true;
			break;

		default:
			warning("decodePictV1: Unknown opcode: 0x%02x", opcode);
			break;
		}
	}

	return s;
}

// ---------------------------------------------------------------------------
// Window handling
// ---------------------------------------------------------------------------

MacGui::MacDialogWindow *MacGui::createWindow(Common::Rect bounds, MacDialogWindowStyle style) {
	if (bounds.left < 0 || bounds.top < 0 || bounds.right >= 640 || bounds.bottom >= 400) {
		// This happens with the Last Crusade file dialogs.
		bounds.moveTo((640 - bounds.width()) / 2, 27);
	}

	// Adjust the dialog to the actual screen size. This is slightly
	// wasteful since we've already adjusted the dialog coordinates for
	// 640x400 pixels, but that may not be a bad thing if we want to keep
	// support for that resolution later.

	bounds.translate(0, 2 * _vm->_screenDrawOffset);

	return new MacDialogWindow(this, _system, _surface, bounds, style);
}

Common::String MacGui::getDialogString(Common::SeekableReadStream *res, int len) {
	Common::String str;

	for (int i = 0; i < len; i++)
		str += res->readByte();

	return str;
}

MacGui::MacDialogWindow *MacGui::createDialog(int dialogId) {
	Common::MacResManager resource;
	Common::SeekableReadStream *res;

	resource.open(_resourceFile);

	Common::Rect bounds;

	bool isOpenDialog = dialogId == 4000 || dialogId == 4001;
	bool isSaveDialog = dialogId == 3998 || dialogId == 3999;

	res = resource.getResource(MKTAG('D', 'L', 'O', 'G'), dialogId);
	if (res) {
		bounds.top = res->readUint16BE();
		bounds.left = res->readUint16BE();
		bounds.bottom = res->readUint16BE();
		bounds.right = res->readUint16BE();

		// Grow the window to include the outer bounds
		bounds.grow(8);

		// Compensate for the original not drawing the game at the very top of
		// the screen.
		bounds.translate(0, -40);
	} else {
		bounds.top = 0;
		bounds.left = 0;
		bounds.bottom = 86;
		bounds.right = 340;

		bounds.translate(86, 88);
	}

	delete res;

	MacDialogWindow *window = createWindow(bounds);

	res = resource.getResource(MKTAG('D', 'I', 'T', 'L'), dialogId);

	if (res) {
		int numItems = res->readUint16BE() + 1;

		for (int i = 0; i < numItems; i++) {
			res->skip(4);	// Placeholder for handle or procedure pointer

			Common::Rect r;

			r.top = res->readUint16BE();
			r.left = res->readUint16BE();
			r.bottom = res->readUint16BE();
			r.right = res->readUint16BE();

			// Move to appropriate position on inner surface
			r.translate(2, 2);

			int type = res->readByte();
			int len = res->readByte();

			Common::String str;
			bool enabled = ((type & 0x80) == 0);

			switch (type & 0x7F) {
			case 0:
			{
				// User item

				// Skip drive label box and listbox
				bool doNotDraw = (isOpenDialog && (i == 6 || i == 7)) || ((isOpenDialog || isSaveDialog) && i == 3);
				if (!doNotDraw)
					window->innerSurface()->frameRect(r, kBlack);

				break;
			}
			case 4:
				// Button
				str = getDialogString(res, len);
				if ((isOpenDialog || isSaveDialog) && (i == 4 || i == 5)) // "Eject" and "Drive"
					enabled = false;

				window->addButton(r, str, enabled);
				break;

			case 5:
				// Checkbox
				str = getDialogString(res, len);
				window->addCheckbox(r, str, enabled);
				break;

			case 8:
				// Static text
				str = getDialogString(res, len);
				if (isSaveDialog && i == 2)
					str = "Save Game File as...";

				window->addStaticText(r, str, enabled);
				break;

			case 16:
			{
				// Editable text

				// Adjust for pixel accuracy...
				r.left -= 1;

				MacGui::MacEditText *editText = window->addEditText(r, "Game file", enabled);
				editText->selectAll();

				window->innerSurface()->frameRect(Common::Rect(r.left - 2, r.top - 3, r.right + 3, r.bottom + 3), kBlack);
				res->skip(len);
				break;
			}
			case 64:
				// Picture
				window->addPicture(r, res->readUint16BE(), enabled);
				break;

			default:
				warning("MacGui::createDialog(): Unknown item type %d", type);
				res->skip(len);
				break;
			}

			if (len & 1)
				res->skip(1);
		}
	}

	delete res;
	resource.close();

	return window;
}

// ---------------------------------------------------------------------------
// Standard dialogs
// ---------------------------------------------------------------------------

void MacGui::prepareSaveLoad(Common::StringArray &savegameNames, bool *availSlots, int *slotIds, int size) {
	int saveCounter = 0;

	for (int i = 0; i < size; i++) {
		slotIds[i] = -1;
	}

	Common::String name;
	_vm->listSavegames(availSlots, size);

	for (int i = 0; i < size; i++) {
		if (availSlots[i]) {
			// Save the slot ids for slots which actually contain savegames...
			slotIds[saveCounter] = i;
			saveCounter++;
			if (_vm->getSavegameName(i, name)) {
				Common::String temp = Common::U32String(name, _vm->getDialogCodePage()).encode(Common::kMacRoman);
				savegameNames.push_back(temp);
			} else {
				// The original printed "WARNING... old savegame", but we do support old savegames :-)
				savegameNames.push_back(Common::String::format("%s", "WARNING: wrong save version"));
			}
		}
	}
}

bool MacGui::runOkCancelDialog(Common::String text) {
	// Widgets:
	//
	// 0 - Okay button
	// 1 - Cancel button
	// 2 - "^0" text

	MacDialogWindow *window = createDialog(502);

	window->setDefaultWidget(0);
	window->addSubstitution(text);

	MacStaticText *widget = (MacStaticText *)window->getWidget(2);
	widget->setWordWrap(true);

	// When quitting, the default action is to quit
	bool ret = true;

	Common::Array<int> deferredActionsIds;

	while (!_vm->shouldQuit()) {
		int clicked = window->runDialog(deferredActionsIds);

		if (clicked == 0)
			break;

		if (clicked == 1) {
			ret = false;
			break;
		}
	}

	delete window;
	return ret;
}

bool MacGui::runQuitDialog() {
	return runOkCancelDialog("Are you sure you want to quit?");
}

bool MacGui::runRestartDialog() {
	return runOkCancelDialog("Are you sure you want to restart this game from the beginning?");
}

MacGui::MacDialogWindow *MacGui::drawBanner(char *message) {
	MacGui::MacDialogWindow *window = createWindow(
		Common::Rect(70, 189, 570, 211),
		kStyleRounded);
	const Graphics::Font *font = getFont(_vm->_game.id == GID_INDY3 ? kIndy3FontMedium : kLoomFontMedium);

	Graphics::Surface *s = window->innerSurface();
	font->drawString(s, (char *)message, 0, 0, s->w, kBlack, Graphics::kTextAlignCenter);

	window->show();
	return window;
}

void MacGui::drawBitmap(Graphics::Surface *s, Common::Rect r, const uint16 *bitmap, Color color) const {
	assert(r.width() <= 16);

	for (int y = 0; y < r.height(); y++) {
		uint16 bit = 0x8000;
		for (int x = 0; x < r.width(); x++) {
			if (bitmap[y] & bit)
				s->setPixel(r.left + x, r.top + y, color);
			bit >>= 1;
		}
	}
}

// ===========================================================================
// The Mac Loom GUI. This one is pretty simple.
// ===========================================================================

MacLoomGui::MacLoomGui(ScummEngine *vm, Common::String resourceFile) : MacGui(vm, resourceFile) {
	// The practice box can be moved, but this is its default position on
	// a large screen, and it's not saved.

	_practiceBoxPos = Common::Point(215, 376 + 2 * _vm->_screenDrawOffset);
}

MacLoomGui::~MacLoomGui() {
	if (_practiceBox) {
		_practiceBox->free();
		delete _practiceBox;
	}
}

const Graphics::Font *MacLoomGui::getFontByScummId(int32 id) {
	switch (id) {
	case 0:
		return getFont(kLoomFontLarge);

	default:
		error("MacLoomGui::getFontByScummId: Invalid font id %d", id);
	}
}

bool MacLoomGui::getFontParams(FontId fontId, int &id, int &size, int &slant) const {
	if (MacGui::getFontParams(fontId, id, size, slant))
		return true;

	// Loom uses only font size 13 for in-game text, but size 12 is used
	// for system messages, e.g. the original pause dialog.
	//
	// Special characters:
	//
	// 16-23 are the note names c through c'.
	// 60 is an upside-down note, i.e. the one used for c'.
	// 95 is a used for the rest of the notes.

	switch (fontId) {
	case FontId::kLoomFontSmall:
		id = _gameFontId;
		size = 9;
		slant = Graphics::kMacFontRegular;
		return true;

	case FontId::kLoomFontMedium:
		id = _gameFontId;
		size = 12;
		slant = Graphics::kMacFontRegular;
		return true;

	case FontId::kLoomFontLarge:
		id = _gameFontId;
		size = 13;
		slant = Graphics::kMacFontRegular;
		return true;

	default:
		error("MacLoomGui: getFontParams: Unknown font id %d", (int)fontId);
	}

	return false;
}

void MacLoomGui::setupCursor(int &width, int &height, int &hotspotX, int &hotspotY, int &animate) {
	Common::MacResManager resource;
	Graphics::MacCursor macCursor;

	resource.open(_resourceFile);

	Common::SeekableReadStream *curs = resource.getResource(MKTAG('C', 'U', 'R', 'S'), 1000);

	if (macCursor.readFromStream(*curs)) {
		width = macCursor.getWidth();
		height = macCursor.getHeight();
		hotspotX = macCursor.getHotspotX();
		hotspotY = macCursor.getHotspotY();
		animate = 0;

		_windowManager->replaceCursor(Graphics::MacGUIConstants::kMacCursorCustom, &macCursor);
	}

	delete curs;
	resource.close();
}

bool MacLoomGui::handleMenu(int id, Common::String &name) {
	if (MacGui::handleMenu(id, name))
		return true;

	switch (id) {
	case 101:	// Drafts inventory
		runDraftsInventory();
		break;

	case 204:	// Options
		runOptionsDialog();
		break;

	case 205:	// Quit
		if (runQuitDialog())
			_vm->quitGame();
		break;

	default:
		warning("Unknown menu command: %d", id);
		break;
	}

	return false;
}

void MacLoomGui::runAboutDialog() {
	// The About window is not a a dialog resource. Its size appears to be
	// hard-coded (416x166), and it's drawn centered. The graphics are in
	// PICT 5000 and 5001.

	int width = 416;
	int height = 166;
	int x = (640 - width) / 2;
	int y = (400 - height) / 2;

	Common::Rect bounds(x, y, x + width, y + height);
	MacDialogWindow *window = createWindow(bounds);
	Graphics::Surface *lucasFilm = loadPict(5000);
	Graphics::Surface *loom = loadPict(5001);

	// TODO: These strings are part of the STRS resource, but I don't know
	// how to safely read them from there yet. So hard-coded it is for now.

	const TextLine page1[] = {
		{ 0, 23, kStyleExtraBold, Graphics::kTextAlignCenter, "PRESENTS" },
		TEXT_END_MARKER
	};

	const TextLine page2[] = {
		{ 1, 59, kStyleRegular, Graphics::kTextAlignCenter, "TM & \xA9 1990 LucasArts Entertainment Company.  All rights reserved." },
		{ 0, 70, kStyleRegular, Graphics::kTextAlignCenter, "Release Version 1.2  25-JAN-91 Interpreter version 5.1.6" },
		TEXT_END_MARKER
	};

	const TextLine page3[] = {
		{ 1, 11, kStyleBold, Graphics::kTextAlignCenter, "Macintosh version by" },
		{ 0, 25, kStyleHeader, Graphics::kTextAlignCenter, "Eric Johnston" },
		{ 0, 49, kStyleBold, Graphics::kTextAlignCenter, "Macintosh scripting by" },
		{ 1, 63, kStyleHeader, Graphics::kTextAlignCenter, "Ron Baldwin" },
		TEXT_END_MARKER
	};

	const TextLine page4[] = {
		{ 0, 26, kStyleBold, Graphics::kTextAlignCenter, "Original game created by" },
		{ 1, 40, kStyleHeader, Graphics::kTextAlignCenter, "Brian Moriarty" },
		TEXT_END_MARKER
	};

	const TextLine page5[] = {
		{ 1, 11, kStyleBold, Graphics::kTextAlignCenter, "Produced by" },
		{ 0, 25, kStyleHeader, Graphics::kTextAlignCenter, "Gregory D. Hammond" },
		{ 0, 49, kStyleBold, Graphics::kTextAlignCenter, "Macintosh Version Produced by" },
		{ 1, 63, kStyleHeader, Graphics::kTextAlignCenter, "David Fox" },
		TEXT_END_MARKER
	};

	const TextLine page6[] = {
		{ 1, 6, kStyleBold, Graphics::kTextAlignCenter, "SCUMM Story System" },
		{ 1, 16, kStyleBold, Graphics::kTextAlignCenter, "created by" },
		{ 97, 35, kStyleHeader, Graphics::kTextAlignLeft, "Ron Gilbert" },
		{ 1, 51, kStyleBold, Graphics::kTextAlignCenter, "and" },
		{ 122, 65, kStyleHeader, Graphics::kTextAlignLeft, "Aric Wilmunder" },
		TEXT_END_MARKER
	};

	const TextLine page7[] = {
		{ 1, 16, kStyleBold, Graphics::kTextAlignCenter, "Stumped?  Loom hint books are available!" },
		{ 76, 33, kStyleRegular, Graphics::kTextAlignLeft, "In the U.S. call" },
		{ 150, 34, kStyleBold, Graphics::kTextAlignLeft, "1 (800) STAR-WARS" },
		{ 150, 43, kStyleRegular, Graphics::kTextAlignLeft, "that\xD5s  1 (800) 782-7927" },
		{ 80, 63, kStyleRegular, Graphics::kTextAlignLeft, "In Canada call" },
		{ 150, 64, kStyleBold, Graphics::kTextAlignLeft, "1 (800) 828-7927" },
		TEXT_END_MARKER
	};

	const TextLine page8[] = {
		{ 1, 11, kStyleBold, Graphics::kTextAlignCenter, "Need a hint NOW?  Having problems?" },
		{ 81, 25, kStyleRegular, Graphics::kTextAlignLeft, "For technical support call" },
		{ 205, 26, kStyleBold, Graphics::kTextAlignLeft, "1 (415) 721-3333" },
		{ 137, 35, kStyleRegular, Graphics::kTextAlignLeft, "For hints call" },

		{ 205, 36, kStyleBold, Graphics::kTextAlignLeft, "1 (900) 740-JEDI" },
		{ 1, 50, kStyleRegular, Graphics::kTextAlignCenter, "The charge for the hint line is 75\xA2 per minute." },
		{ 1, 60, kStyleRegular, Graphics::kTextAlignCenter, "(You must have your parents\xD5 permission to" },
		{ 1, 70, kStyleRegular, Graphics::kTextAlignCenter, "call this number if you are under 18.)" },
		TEXT_END_MARKER
	};

	// I've based the animation speed on what it looks like when Mini vMac
	// emulates an old black-and-white Mac at normal speed. It looks a bit
	// different in Basilisk II, but that's probably because it emulates a
	// much faster Mac.

	window->show();

	int scene = 0;
	int status = 0;

	Common::Rect r(0, 0, 404, 154);
	int growth = -2;
	int pattern;
	bool darkenOnly = false;
	int waitFrames = 0;

	int innerBounce = 72;
	int targetTop = 48;
	int targetGrowth = 2;

	bool changeScene = false;
	bool fastForward = false;

	while (!_vm->shouldQuit()) {
		if ((scene % 2) == 0) {
			// This appears to be pixel perfect or at least nearly
			// so for the outer layers, but breaks down slightly
			// near the middle.
			//
			// Also, the original does an inexplicable skip in the
			// first animation that I haven't bothered to
			// implement. I don't know if it was intentional or
			// not, but I think it looks awkward. And I wasn't able
			// to get it quite right anyway.

			pattern = (r.top / 2) % 8;

			if (pattern > 4)
				darkenOnly = false;

			Graphics::drawRoundRect(r, 7, pattern, true, darkenOnly ? MacDialogWindow::plotPatternDarkenOnly : MacDialogWindow::plotPattern, window);

			if (!fastForward)
				window->markRectAsDirty(r);

			if (r.top == targetTop && growth == targetGrowth) {
				changeScene = true;
			} else {
				r.grow(growth);

				if (growth < 0 && r.top >= innerBounce)
					growth = -growth;
			}
		} else {
			if (--waitFrames <= 0)
				changeScene = true;
		}

		if (!fastForward) {
			window->update();
			status = delay(50);
		}

		// We can't actually skip to the end of a scene, because the
		// animation has to be drawn.

		if (status == 1)
			fastForward = true;

		if (status == 2)
			break;

		if (changeScene) {
			changeScene = false;
			scene++;

			switch (scene) {
			case 1:
				fastForward = false;
				waitFrames = 60;	// ~3 seconds
				darkenOnly = true;
				window->drawSprite(lucasFilm, 134, 61);
				break;

			case 2:
			case 6:
			case 8:
			case 10:
			case 12:
			case 14:
			case 16:
				growth = -2;
				break;

			case 3:
				fastForward = false;
				darkenOnly = true;
				waitFrames = 40;	// ~2 seconds
				window->drawTexts(r, page1);
				break;

			case 4:
				growth = -2;
				innerBounce -= 8;
				targetTop -= 16;
				break;

			case 5:
				fastForward = false;
				darkenOnly = true;
				waitFrames = 130;	// ~6.5 seconds
				window->drawSprite(loom, 95, 38);
				window->drawTexts(r, page2);
				break;

				growth = -2;
				break;

			case 7:
				fastForward = false;
				darkenOnly = true;
				waitFrames = 80;	// ~4 seconds
				window->drawTexts(r, page3);
				break;

			case 9:
				fastForward = false;
				darkenOnly = true;
				waitFrames = 80;
				window->drawTexts(r, page4);
				break;

			case 11:
				fastForward = false;
				darkenOnly = true;
				waitFrames = 80;
				window->drawTexts(r, page5);
				break;

			case 13:
				fastForward = false;
				darkenOnly = true;
				waitFrames = 80;
				window->drawTexts(r, page6);
				break;

			case 15:
				fastForward = false;
				darkenOnly = true;
				waitFrames = 260;	// ~13 seconds
				window->drawTexts(r, page7);
				break;

			case 17:
				fastForward = false;
				window->drawTexts(r, page8);
				break;
			}


			window->update(true);

			if (scene >= 17)
				break;
		}
	}

	if (status != 2)
		delay();

	_windowManager->popCursor();

	lucasFilm->free();
	loom->free();

	delete lucasFilm;
	delete loom;
	delete window;
}

void MacLoomGui::runDraftsInventory() {
	int base, xPos, textHeight, heightMultiplier, draft, inactiveColor,
		unlockedColor, newDraftColor, notesColor;

	char notesBuf[6];
	const char *names[18] = {
		"Drafts",
		"Opening:", "Straw Into Gold:", "Dyeing:",
		"Night Vision:", "Twisting:", "Sleep:",
		"Emptying:", "Invisibility:", "Terror:",
		"Sharpening:", "Reflection:", "Healing:",
		"Silence:", "Shaping:", "Unmaking:",
		"Transcendence:",
		"Unknown:"
	};

	const char *notes = "cdefgabC";

	// ACT 1: Draw the Mac dialog window
	MacGui::MacDialogWindow *window = createWindow(Common::Rect(110, 20, 540, 252));
	const Graphics::Font *font = getFont(kSystemFont);

	Graphics::Surface *s = window->innerSurface();

	// ACT 2: Draw the drafts text
	//
	// Drafts are stored in SCUMM global variables; we choose the appropriate
	// first entry in the variables at which these drafts start.
	base = 55;

	// TODO: Can these be drawn in different styles? (e.g. Checkerboard)
	unlockedColor = kBlack;
	inactiveColor = kBlack;
	newDraftColor = kBlack;

	for (int i = 0; i < 16; i++) {
		draft = _vm->_scummVars[base + i * 2];

		// In which row are we rendering our text?
		heightMultiplier = i < 8 ? i : (i % 8);
		textHeight = 24;

		// Has the draft been unlocked by the player?
		//int titleColor = (draft & 0x2000) ? unlockedColor : inactiveColor;

		// Has the new draft been used at least once?
		notesColor = (draft & 0x4000) ? unlockedColor : newDraftColor;

		// Has the draft been unlocked? Great: put it in our text buffer
		// otherwise just prepare to render the "????" string.
		if (draft & 0x2000) {
			Common::sprintf_s(notesBuf, sizeof(notesBuf), "%c%c%c%c",
							  notes[draft & 0x0007],
							  notes[(draft & 0x0038) >> 3],
							  notes[(draft & 0x01c0) >> 6],
							  notes[(draft & 0x0e00) >> 9]);
		} else {
			notesColor = inactiveColor;
			Common::sprintf_s(notesBuf, sizeof(notesBuf), "????");
		}

		// Where are we positioning the text?
		// Left column or right column?
		xPos = i < 8 ? 40 : 260;

		// Draw the titles of the drafts...
		if (draft & 0x2000) {
			font->drawString(s, (const char *)names[i + 1], xPos - 20, 24 + textHeight * heightMultiplier, s->w, notesColor, Graphics::kTextAlignLeft); // FIXME: titleColor, not notesColor?
		} else {
			// Draw "Unknown:" as the title of the draft
			font->drawString(s, (const char *)names[17], xPos - 20, 24 + textHeight * heightMultiplier, s->w, notesColor, Graphics::kTextAlignLeft); // FIXME: titleColor, not notesColor?
		}

		// Draw the notes of the draft...
		font->drawString(s, (const char *)notesBuf, xPos + 100, 24 + textHeight * heightMultiplier, s->w, notesColor, Graphics::kTextAlignLeft);
	}

	// Draw "Drafts" on top of the dialog
	font->drawString(s, (const char *)names[0], 0, 4, s->w, kBlack, Graphics::kTextAlignCenter);

	// Draw a vertical line to separate the two columns
	s->drawLine(210, 44, 210, 184, kBlack);

	// Update the screen with all the new stuff!
	window->show();
	delay();
	delete window;
}

// A standard file picker dialog doesn't really make sense in ScummVM, so we
// make something that just looks similar to one.

bool MacLoomGui::runOpenDialog(int &saveSlotToHandle) {
	Common::Rect bounds(88, 28, 448, 208);

	MacDialogWindow *window = createWindow(bounds);

	window->addButton(Common::Rect(254, 135, 334, 155), "Open", true);
	window->addButton(Common::Rect(254, 104, 334, 124), "Cancel", true);
	window->addButton(Common::Rect(254, 59, 334, 79), "Delete", true);

	window->drawDottedHLine(253, 91, 334);

	bool availSlots[100];
	int slotIds[100];
	Common::StringArray savegameNames;
	prepareSaveLoad(savegameNames, availSlots, slotIds, ARRAYSIZE(availSlots));

	window->addListBox(Common::Rect(14, 13, 217, 159), savegameNames, true);

	window->setDefaultWidget(0);

	// When quitting, the default action is to not open a saved game
	bool ret = false;
	Common::Array<int> deferredActionsIds;

	while (!_vm->shouldQuit()) {
		int clicked = window->runDialog(deferredActionsIds);

		if (clicked == 0 || clicked == 3) {
			saveSlotToHandle =
				window->getWidgetValue(3) < ARRAYSIZE(slotIds) ?
				slotIds[window->getWidgetValue(3)] : -1;
			ret = true;
			break;
		}

		if (clicked == 1)
			break;

		if (clicked == 2) {
			if (runOkCancelDialog("Are you sure you want to delete the saved game?"))
				runOkCancelDialog("Deleting savegames is currently unsupported in ScummVM.");
		}
	}

	delete window;
	return ret;
}

bool MacLoomGui::runSaveDialog(int &saveSlotToHandle, Common::String &name) {
	Common::Rect bounds(110, 27, 470, 231);

	MacDialogWindow *window = createWindow(bounds);

	window->addButton(Common::Rect(254, 159, 334, 179), "Save", true);
	window->addButton(Common::Rect(254, 128, 334, 148), "Cancel", true);
	window->addButton(Common::Rect(254, 83, 334, 103), "Delete", false);

	bool busySlots[100];
	int slotIds[100];
	Common::StringArray savegameNames;
	prepareSaveLoad(savegameNames, busySlots, slotIds, ARRAYSIZE(busySlots));

	int firstAvailableSlot = -1;
	for (int i = 1; i < ARRAYSIZE(busySlots); i++) { // Skip the autosave slot
		if (!busySlots[i]) {
			firstAvailableSlot = i;
			break;
		}
	}

	window->addListBox(Common::Rect(14, 9, 217, 139), savegameNames, true, true);

	MacGui::MacEditText *editText = window->addEditText(Common::Rect(16, 164, 229, 180), "Game file", true);

	Graphics::Surface *s = window->innerSurface();
	const Graphics::Font *font = getFont(kSystemFont);

	s->frameRect(Common::Rect(14, 161, 232, 183), kBlack);

	window->drawDottedHLine(253, 115, 334);

	font->drawString(s, "Save Game File as...", 14, 143, 218, kBlack, Graphics::kTextAlignLeft, 4);

	window->setDefaultWidget(0);
	editText->selectAll();

	// When quitting, the default action is to not open a saved game
	bool ret = false;
	Common::Array<int> deferredActionsIds;

	while (!_vm->shouldQuit()) {
		int clicked = window->runDialog(deferredActionsIds);

		if (clicked == 0) {
			ret = true;
			name = editText->getText();
			saveSlotToHandle = firstAvailableSlot;
			break;
		}

		if (clicked == 1)
			break;

		if (clicked == -2) {
			// Cycle through deferred actions
			for (uint i = 0; i < deferredActionsIds.size(); i++) {
				// Edit text widget
				if (deferredActionsIds[i] == 4) {
					MacGui::MacWidget *wid = window->getWidget(deferredActionsIds[i]);

					// Disable "Save" button when text is empty
					window->getWidget(0)->setEnabled(!wid->getText().empty());
				}
			}
		}
	}

	delete window;
	return ret;
}

bool MacLoomGui::runOptionsDialog() {
	// Widgets:
	//
	// 0 - Okay button
	// 1 - Cancel button
	// 2 - Sound checkbox
	// 3 - Music checkbox
	// 4 - Picture (text speed background)
	// 5 - Picture (text speed handle)
	// 6 - Scrolling checkbox
	// 7 - Full Animation checkbox
	// 8 - Picture (music quality background)
	// 9 - Picture (music quality handle)
	// 10 - "Machine Speed:  ^0" text
	// 11 - Text speed slider (manually created)
	// 12 - Music quality slider (manually created)

	int sound = 1;
	int music = 1;
	if (_vm->VAR(167) == 2) {
		sound = music = 0;
	} else if (_vm->VAR(167) == 1) {
		music = 0;
	}

	int scrolling = _vm->_snapScroll == 0;
	int fullAnimation = _vm->VAR(_vm->VAR_MACHINE_SPEED) == 1 ? 0 : 1;
	int textSpeed = _vm->_defaultTextSpeed;
	int musicQuality = _vm->VAR(_vm->VAR_SOUNDCARD) == 10 ? 0 : 2;

	MacDialogWindow *window = createDialog(1000);

	window->setWidgetValue(2, sound);
	window->setWidgetValue(3, music);
	window->setWidgetValue(6, scrolling);
	window->setWidgetValue(7, fullAnimation);

	if (!sound)
		window->setWidgetEnabled(3, false);

	window->addPictureSlider(4, 5, true, 5, 105, 0, 9);
	window->setWidgetValue(11, textSpeed);

	window->addPictureSlider(8, 9, true, 5, 69, 0, 2, 6, 4);
	window->setWidgetValue(12, musicQuality);

	// Machine rating
	window->addSubstitution(Common::String::format("%d", _vm->VAR(53)));

	// When quitting, the default action is not to not apply options
	bool ret = false;
	Common::Array<int> deferredActionsIds;

	while (!_vm->shouldQuit()) {
		int clicked = window->runDialog(deferredActionsIds);

		if (clicked == 0) {
			ret = true;
			break;
		}

		if (clicked == 1)
			break;

		if (clicked == 2)
			window->setWidgetEnabled(3, window->getWidgetValue(2) != 0);
	}

	if (ret) {
		// Update settings

		// TEXT SPEED
		_vm->_defaultTextSpeed = CLIP<int>(window->getWidgetValue(11), 0, 9);
		ConfMan.setInt("original_gui_text_speed", _vm->_defaultTextSpeed);
		_vm->setTalkSpeed(_vm->_defaultTextSpeed);

		// SOUND&MUSIC ACTIVATION
		// 0 - Sound&Music on
		// 1 - Sound on, music off
		// 2 - Sound&Music off
		int musicVariableValue = 0;

		if (window->getWidgetValue(2) == 0) {
			musicVariableValue = 2;
		} else if (window->getWidgetValue(2) == 1 && window->getWidgetValue(3) == 0) {
			musicVariableValue = 1;
		}

		_vm->VAR(167) = musicVariableValue;

		if (musicVariableValue != 0) {
			if (_vm->VAR(169) != 0) {
				_vm->_sound->stopSound(_vm->VAR(169));
				_vm->VAR(169) = 0;
			}
		}

		// SCROLLING ACTIVATION
		_vm->_snapScroll = window->getWidgetValue(6) == 0;

		if (_vm->VAR_CAMERA_FAST_X != 0xFF)
			_vm->VAR(_vm->VAR_CAMERA_FAST_X) = _vm->_snapScroll;

		// FULL ANIMATION ACTIVATION
		_vm->VAR(_vm->VAR_MACHINE_SPEED) = window->getWidgetValue(7) == 1 ? 0 : 1;

		// MUSIC QUALITY SELECTOR
		//
		// (selections 1 and 2 appear to be the same music
		// files but rendered at a different bitrate, while
		// selection 0 activates the low quality channel in
		// the sequence files and mutes everything else)
		//
		// This is currently incomplete. Let's just set the proper
		// value for VAR_SOUNDCARD...
		_vm->VAR(_vm->VAR_SOUNDCARD) = window->getWidgetValue(12) == 0 ? 10 : 11;
		((Player_V3M *)_vm->_musicEngine)->overrideQuality(_vm->VAR(_vm->VAR_SOUNDCARD) == 10);

		debug(6, "MacLoomGui::runOptionsDialog(): music quality: %d - unimplemented!", window->getWidgetValue(12));


		_vm->syncSoundSettings();
		ConfMan.flushToDisk();
	}

	delete window;
	return ret;
}

void MacLoomGui::resetAfterLoad() {
	reset();

	// We used to use verb 53 for the Loom practice box, and while it's
	// still the verb we pretend to use when clicking on it we no longer
	// use the actual verb slot.
	//
	// Apparently the practice box isn't restored on saving, so it seems
	// that savegame compatibility isn't broken. And if it is, it happened
	// shortly after the savegame version was increased for other reasons,
	// so the damage would be very limited.

	for (int i = 0; i < _vm->_numVerbs; i++) {
		if (_vm->_verbs[i].verbid == 53)
			_vm->killVerb(i);
	}
}

void MacLoomGui::update(int delta) {
	// Unlike the PC version, the Macintosh version of Loom appears to
	// hard-code the drawing of the practice mode box. This is handled by
	// script 27 in both versions, but whereas the PC version draws the
	// notes, the Mac version just sets variables 50 and 54.
	//
	// In this script, the variables are set to the same value but it
	// appears that only variable 50 is cleared when the box is supposed to
	// disappear. I don't know what the purpose of variable 54 is.
	//
	// Variable 128 is the game difficulty:
	//
	// 0 - Practice
	// 1 - Standard
	// 2 - Expert
	//
	// Note that the practice mode box is never inscribed on the "Mac
	// screen" surface. It's drawn last on every update, so it floats
	// above everything else.

	int notes = _vm->VAR(50);

	if (_vm->VAR(128) == 0) {
		if (notes) {
			int w = 64;
			int h = 24;

			bool bw = (_vm->_renderMode == Common::kRenderMacintoshBW);

			if (!_practiceBox) {
				debug(1, "MacLoomGui: Creating practice mode box");

				_practiceBox = new Graphics::Surface();
				_practiceBox->create(w, h, Graphics::PixelFormat
::createFormatCLUT8());

				_practiceBox->fillRect(Common::Rect(w, h), kBlack);

				Color color = bw ? kWhite : kLightGray;

				_practiceBox->hLine(2, 1, w - 3, color);
				_practiceBox->hLine(2, h - 2, w - 3, color);
				_practiceBox->vLine(1, 2, h - 3, color);
				_practiceBox->vLine(w - 2, 2, h - 3, color);
				_practiceBoxNotes = 0;
			}

			if (notes != _practiceBoxNotes) {
				debug(1, "MacLoomGui: Drawing practice mode notes");

				_practiceBoxNotes = notes;

				_practiceBox->fillRect(Common::Rect(2, 2, w - 2, h - 2), kBlack);

				const Graphics::Font *font = getFont(kLoomFontLarge);
				Color colors[] = { kRed, kBrightRed, kBrightYellow, kBrightGreen, kBrightCyan, kCyan, kBrightBlue, kWhite };

				for (int i = 0; i < 4; i++) {
					int note = (notes >> (4 * i)) & 0x0F;

					if (note >= 2 && note <= 9) {
						font->drawChar(_practiceBox, 14 + note, 9 + i * 13, 5, bw ? kWhite : colors[note - 2]);
					}
				}
			}

			_system->copyRectToScreen(_practiceBox->getBasePtr(0, 0), _practiceBox->pitch, _practiceBoxPos.x, _practiceBoxPos.y, w, h);
		} else {
			if (_practiceBox) {
				debug(1, "MacLoomGui: Deleting practice mode box");

				_system->copyRectToScreen(_surface->getBasePtr(_practiceBoxPos.x, _practiceBoxPos.y), _surface->pitch, _practiceBoxPos.x, _practiceBoxPos.y, _practiceBox->w, _practiceBox->h);

				_practiceBox->free();
				delete _practiceBox;
				_practiceBox = nullptr;
			}
		}
	}
}

bool MacLoomGui::handleEvent(Common::Event event) {
	if (MacGui::handleEvent(event))
		return true;

	if (!_practiceBox || _vm->_userPut <= 0)
		return false;

	// Perhaps the silliest feature in Mac Loom, that literally only one
	// person has ever asked for: You can drag the Loom practice box.
	//
	// The game will freeze while the button is held down, but that's how
	// the original acted as well. Should sounds keep playing? I don't know
	// if that situation can even occur. I think it's nicer to let them
	// play if it does.

	if (event.type != Common::EVENT_LBUTTONDOWN)
		return false;

	Common::Rect bounds;

	bounds.left = _practiceBoxPos.x;
	bounds.top = _practiceBoxPos.y;
	bounds.right = _practiceBoxPos.x + _practiceBox->w;
	bounds.bottom = _practiceBoxPos.y + _practiceBox->h;

	if (!bounds.contains(event.mouse))
		return false;

	int clickX = event.mouse.x;
	int clickY = event.mouse.y;
	bool dragMode = false;

	while (!_vm->shouldQuit()) {
		bool dragging = false;
		int dragX = 0;
		int dragY = 0;

		while (_system->getEventManager()->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_LBUTTONUP:
				if (!dragMode)
					_vm->runInputScript(kVerbClickArea, 53, 1);
				return true;

			case Common::EVENT_MOUSEMOVE:
				if (ABS(event.mouse.x - clickX) >= 3 || ABS(event.mouse.y - clickY) >= 3)
					dragMode = true;

				if (dragMode) {
					dragging = true;
					dragX = event.mouse.x;
					dragY = event.mouse.y;
				}

				break;

			default:
				break;
			}
		}

		if (dragging) {
			// How much has the mouse moved since the initial
			// click? Calculate new position from that.

			int newX = bounds.left + (dragX - clickX);
			int newY = bounds.top + (dragY - clickY);

			// The box has to stay completely inside the screen.
			// Also, things get weird if you move the box into the
			// menu hotzone, so don't allow that.

			int maxY = _surface->h - _practiceBox->h - 2 * _vm->_screenDrawOffset;
			int minY = 2 * _vm->_screenDrawOffset;

			if (_vm->isUsingOriginalGUI() && minY < 23)
				minY = 23;

			newX = CLIP(newX, 0, _surface->w - _practiceBox->w);
			newY = CLIP(newY, minY, maxY);

			// For some reason, X coordinates can only change in
			// increments of 16 pixels. As an enhancement, we allow
			// any X coordinate.

			if (!_vm->enhancementEnabled(kEnhUIUX))
				newX &= ~0xF;

			if (newX != _practiceBoxPos.x || newY != _practiceBoxPos.y) {
				int w = _practiceBox->w;
				int h = _practiceBox->h;

				// The old and new rect will almost certainly
				// overlap, so it's possible to optimize this.
				// But increasing the delay in the event loop
				// was a better optimization than removing one
				// of the copyRectToScreen() calls completely,
				// so I doubt it's worth it.

				_system->copyRectToScreen(_surface->getBasePtr(_practiceBoxPos.x, _practiceBoxPos.y), _surface->pitch, _practiceBoxPos.x, _practiceBoxPos.y, w, h);
				_system->copyRectToScreen(_practiceBox->getBasePtr(0, 0), _practiceBox->pitch, newX, newY, w, h);

				_practiceBoxPos = Common::Point(newX, newY);
			}

			_system->delayMillis(20);
			_system->updateScreen();
		}
	}

	return false;
}

// ===========================================================================
// The Mac GUI for Indiana Jones and the Last Crusade, including the infamous
// verb GUI.
//
// It's likely that the original interpreter used more hooks from the engine
// into the GUI. In particular, the inventory script uses a variable that I
// haven't been able to find in any of the early scripts of the game.
//
// But the design goal here is to keep things as simple as possible, even if
// that means using brute force. So the GUI figures out which verbs are active
// by scanning all the verbs, and which objects are in the inventory by
// scanning all objects.
//
// We used to coerce the Mac verb GUI into something that looked like the
// GUI from all other versions. This used a number of variables and hard-coded
// verbs. The only thing still used of all this is variable 67, to keep track
// of the inventory scroll position. The fake verbs are removed when loading
// old savegames, but the variables are assumed to be harmless.
// ===========================================================================

#define WIDGET_TIMER_JIFFIES	12
#define REPEAT_TIMER_JIFFIES	18

// ---------------------------------------------------------------------------
// The basic building block of the GUI is the Widget. It has a bounding box, a
// timer, a couple of drawing primitives, etc.
// ---------------------------------------------------------------------------

ScummEngine *MacIndy3Gui::Widget::_vm = nullptr;
Graphics::Surface *MacIndy3Gui::Widget::_surface = nullptr;
MacIndy3Gui *MacIndy3Gui::Widget::_gui = nullptr;

MacIndy3Gui::Widget::Widget(int x, int y, int width, int height) : MacGuiObject(Common::Rect(x, y, x + width, y + height), false) {
}

void MacIndy3Gui::Widget::reset() {
	clearTimer();
	_enabled = false;
	setRedraw(false);
}

void MacIndy3Gui::Widget::updateTimer(int delta) {
	if (hasTimer()) {
		if (delta > _timer)
			delta = _timer;

		_timer -= delta;

		if (_timer == 0)
			timeOut();
	}
}

void MacIndy3Gui::Widget::draw() {
	markScreenAsDirty(_bounds);
	_redraw = false;
}

void MacIndy3Gui::Widget::undraw() {
	fill(_bounds);
	markScreenAsDirty(_bounds);
	_redraw = false;
}

void MacIndy3Gui::Widget::markScreenAsDirty(Common::Rect r) const {
	_gui->markScreenAsDirty(r);
}

byte MacIndy3Gui::Widget::translateChar(byte c) const {
	if (c == '^')
		return 0xC9;
	return c;
}

void MacIndy3Gui::Widget::fill(Common::Rect r) {
	_gui->fill(r);
}

void MacIndy3Gui::Widget::drawBitmap(Common::Rect r, const uint16 *bitmap, Color color) const {
	_gui->drawBitmap(_surface, r, bitmap, color);
}

// The shadow box is the basic outline of the verb buttons and the inventory
// widget. A slightly rounded rectangle with a shadow and a highlight.

void MacIndy3Gui::Widget::drawShadowBox(Common::Rect r) const {
	_surface->hLine(r.left + 1, r.top, r.right - 3, kBlack);
	_surface->hLine(r.left + 1, r.bottom - 2, r.right - 3, kBlack);
	_surface->vLine(r.left, r.top + 1, r.bottom - 3, kBlack);
	_surface->vLine(r.right - 2, r.top + 1, r.bottom - 3, kBlack);

	_surface->hLine(r.left + 2, r.bottom - 1, r.right - 1, kBlack);
	_surface->vLine(r.right - 1, r.top + 2, r.bottom - 2, kBlack);

	_surface->hLine(r.left + 1, r.top + 1, r.right - 3, kWhite);
	_surface->vLine(r.left + 1, r.top + 2, r.bottom - 3, kWhite);
}

// The shadow frame is a rectangle with a highlight. It can be filled or
// unfilled.

void MacIndy3Gui::Widget::drawShadowFrame(Common::Rect r, Color shadowColor, Color fillColor) {
	_surface->hLine(r.left, r.top, r.right - 1, kBlack);
	_surface->hLine(r.left, r.bottom - 1, r.right - 1, kBlack);
	_surface->vLine(r.left, r.top + 1, r.bottom - 2, kBlack);
	_surface->vLine(r.right - 1, r.top + 1, r.bottom - 2, kBlack);

	_surface->hLine(r.left + 1, r.top + 1, r.right - 2, shadowColor);
	_surface->vLine(r.left + 1, r.top + 2, r.bottom - 2, shadowColor);

	if (fillColor != kTransparency) {
		Common::Rect fillRect(r.left + 2, r.top + 2, r.right - 1, r.bottom - 1);

		if (fillColor == kBackground)
			fill(fillRect);
		else
			_surface->fillRect(fillRect, fillColor);
	}
}

// ---------------------------------------------------------------------------
// The VerbWidget is what the user interacts with. Each one is connected to a
// verb id and slot. A VerbWidget can consist of several Widgets.
// ---------------------------------------------------------------------------

void MacIndy3Gui::VerbWidget::reset() {
	MacIndy3Gui::Widget::reset();
	_verbslot = -1;
	_visible = false;
	_kill = false;
}

void MacIndy3Gui::VerbWidget::updateVerb(int verbslot) {
	VerbSlot *vs = &_vm->_verbs[verbslot];
	bool enabled = (vs->curmode == 1);

	if (!_visible || _enabled != enabled)
		setRedraw(true);

	_verbslot = verbslot;
	_verbid = vs->verbid;
	_enabled = enabled;
	_kill = false;
}

void MacIndy3Gui::VerbWidget::draw() {
	Widget::draw();
	_visible = true;
}

void MacIndy3Gui::VerbWidget::undraw() {
	debug(1, "VerbWidget: Undrawing [%d]", _verbid);

	Widget::undraw();
	_visible = false;
}

// ---------------------------------------------------------------------------
// The most common type of VerbWidget is the button, which is used for verbs
// and conversation options.
// ---------------------------------------------------------------------------

MacIndy3Gui::Button::Button(int x, int y, int width, int height) : VerbWidget(x, y, width, height) {
}

void MacIndy3Gui::Button::reset() {
	MacIndy3Gui::VerbWidget::reset();
	_text.clear();
}

bool MacIndy3Gui::Button::handleEvent(Common::Event &event) {
	if (!_enabled || !_verbid)
		return false;

	VerbSlot *vs = &_vm->_verbs[_verbslot];

	if (vs->saveid)
		return false;

	bool caughtEvent = false;

	if (event.type == Common::EVENT_KEYDOWN) {
		if (!(event.kbd.flags & (Common::KBD_CTRL | Common::KBD_ALT | Common::KBD_META)) && event.kbd.keycode == vs->key)
			caughtEvent = true;
	} else if (event.type == Common::EVENT_LBUTTONDOWN) {
		if (_bounds.contains(event.mouse))
			caughtEvent = true;
	}

	// Events are handled at the end of the animation. This blatant attack
	// on speedrunners all over the world was done because that's what the
	// original seems to do, and it looks better. Based on tests in Mac
	// emulators, the speed of the animation depended on the speed of your
	// computer, so we just set something that looks good here.

	if (caughtEvent) {
		setRedraw(true);
		setTimer(WIDGET_TIMER_JIFFIES);
	}

	return caughtEvent;
}

void MacIndy3Gui::Button::timeOut() {
	if (_visible) {
		_vm->runInputScript(kVerbClickArea, _verbid, 1);
		setRedraw(true);
	}
}

void MacIndy3Gui::Button::updateVerb(int verbslot) {
	MacIndy3Gui::VerbWidget::updateVerb(verbslot);

	const byte *ptr = _vm->getResourceAddress(rtVerb, verbslot);
	byte buf[270];

	_vm->convertMessageToString(ptr, buf, sizeof(buf));
	if (_text != (char *)buf) {
		_text = (char *)buf;
		clearTimer();
		setRedraw(true);
	}
}

void MacIndy3Gui::Button::draw() {
	if (!getRedraw())
		return;

	debug(1, "Button: Drawing [%d] %s", _verbid, _text.c_str());

	MacIndy3Gui::VerbWidget::draw();
	fill(_bounds);

	if (!hasTimer()) {
		drawShadowBox(_bounds);
	} else {
		// I have only been able to capture a screenshot of the pressed
		// button in black and white, where the checkerboard background
		// makes it hard to see exactly which pixels should be drawn.
		// Basilisk II runs it too fast, and I haven't gotten Mini vMac
		// to run it in 16-color mode.
		//
		// All I can say for certain is that the upper left corner is
		// rounded while the lower right is not. I'm going to assume
		// that the shadow is always drawn, and the rest of the button
		// is just shifted down to the right. That would make the other
		// two corners rounded, so only the lower right one isn't.

		int x0 = _bounds.left + 1;
		int x1 = _bounds.right - 1;
		int y0 = _bounds.top + 1;
		int y1 = _bounds.bottom - 1;

		_surface->hLine(x0 + 1, y0, x1 - 1, kBlack);
		_surface->hLine(x0 + 1, y1, x1, kBlack);
		_surface->vLine(x0, y0 + 1, y1 - 1, kBlack);
		_surface->vLine(x1, y0 + 1, y1 - 1, kBlack);

		_surface->hLine(x0 + 1, y0 + 1, x1 - 1, kWhite);
		_surface->vLine(x0 + 1, y0 + 2, y1 - 1, kWhite);
	}

	// The text is drawn centered. Based on experimentation, I think the
	// width is always based on the outlined font, and the button shadow is
	// not counted as part of the button width.
	//
	// This gives us pixel perfect rendering for the English verbs.

	if (!_text.empty()) {
		const Graphics::Font *boldFont = _gui->getFont(kIndy3VerbFontBold);
		const Graphics::Font *outlineFont = _gui->getFont(kIndy3VerbFontOutline);

		int stringWidth = 0;
		for (uint i = 0; i < _text.size(); i++)
			stringWidth += outlineFont->getCharWidth(_text[i]);

		int x = (_bounds.left + (_bounds.width() - 1 - stringWidth) / 2) - 1;
		int y = _bounds.top + 2;
		Color color = _enabled ? kWhite : kBlack;

		if (hasTimer()) {
			x++;
			y++;
		}

		for (uint i = 0; i < _text.size() && x < _bounds.right; i++) {
			byte c = translateChar(_text[i]);
			if (x >= _bounds.left) {
				if (_enabled)
					outlineFont->drawChar(_surface, c, x, y, kBlack);
				boldFont->drawChar(_surface, c, x + 1, y, color);
			}
			x += boldFont->getCharWidth(c);
		}
	}
}

// ---------------------------------------------------------------------------
// Unlike in the DOS version, where each inventory object shown on screen is
// its own verb, in the Macintosh version the entire Inventory widget is one
// single verb. It consists of the widget itself, the inventory slots, and
// the scrollbar.
// ---------------------------------------------------------------------------

MacIndy3Gui::Inventory::Inventory(int x, int y, int width, int height) : MacIndy3Gui::VerbWidget(x, y, width, height) {
	x = _bounds.left + 6;
	y = _bounds.top + 6;

	// There are always six slots, no matter how many objects you are
	// carrying.
	//
	// Each slot is 12 pixels tall (as seen when they are highlighted),
	// which means they have to overlap slightly to fit. It is assumed
	// that this will never interfere with the text drawing.

	for (int i = 0; i < ARRAYSIZE(_slots); i++) {
		_slots[i] = new Slot(i, x, y, 128, 12);
		y += 11;
	}

	x = _bounds.right - 20;

	_scrollBar = new ScrollBar(x, _bounds.top + 19, 16, 40);

	_scrollButtons[0] = new ScrollButton(x, _bounds.top + 4, 16, 16, kScrollUp);
	_scrollButtons[1] = new ScrollButton(x, _bounds.bottom - 20, 16, 16, kScrollDown);
}

MacIndy3Gui::Inventory::~Inventory() {
	for (int i = 0; i < ARRAYSIZE(_slots); i++)
		delete _slots[i];

	delete _scrollBar;

	for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++)
		delete _scrollButtons[i];
}

void MacIndy3Gui::Inventory::reset() {
	MacIndy3Gui::VerbWidget::reset();

	for (int i = 0; i < ARRAYSIZE(_slots); i++)
		_slots[i]->reset();

	_scrollBar->reset();

	for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++)
		_scrollButtons[i]->reset();
}

void MacIndy3Gui::Inventory::setRedraw(bool redraw) {
	MacIndy3Gui::Widget::setRedraw(redraw);

	for (int i = 0; i < ARRAYSIZE(_slots); i++)
		_slots[i]->setRedraw(redraw);

	_scrollBar->setRedraw(redraw);

	for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++)
		_scrollButtons[i]->setRedraw(redraw);
}

bool MacIndy3Gui::Inventory::handleEvent(Common::Event &event) {
	if (!_enabled || !_verbid)
		return false;

	if (_vm->enhancementEnabled(kEnhUIUX)) {
		if ((event.type == Common::EVENT_WHEELUP || event.type == Common::EVENT_WHEELDOWN) && _bounds.contains(event.mouse.x, event.mouse.y)) {
			if (event.type == Common::EVENT_WHEELUP) {
				_scrollBar->scroll(kScrollUp);
			} else {
				_scrollBar->scroll(kScrollDown);
			}
			return true;
		}
	}

	if (event.type != Common::EVENT_LBUTTONDOWN)
		return false;

	for (int i = 0; i < ARRAYSIZE(_slots); i++) {
		if (_slots[i]->handleEvent(event))
			return true;
	}

	for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++) {
		ScrollButton *b = _scrollButtons[i];

		// Scrolling is done by one object at a time, though you can
		// hold down the mouse button to repeat.

		if (b->handleEvent(event)) {
			_scrollBar->scroll(b->_direction);
			return true;
		}
	}

	if (_scrollBar->handleEvent(event))
		return true;

	return false;
}

bool MacIndy3Gui::Inventory::handleMouseHeld(Common::Point &pressed, Common::Point &held) {
	if (!_enabled || !_verbid)
		return false;

	for (int i = 0; i < ARRAYSIZE(_slots); i++) {
		if (_slots[i]->handleMouseHeld(pressed, held))
			return true;
	}

	for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++) {
		ScrollButton *b = _scrollButtons[i];

		if (b->handleMouseHeld(pressed, held)) {
			_scrollBar->scroll(b->_direction);
			return true;
		}
	}

	// It would be possible to handle dragging the scrollbar handle, but
	// the original didn't.

	if (_scrollBar->handleMouseHeld(pressed, held))
		return true;

	return false;
}

void MacIndy3Gui::Inventory::updateTimer(int delta) {
	Widget::updateTimer(delta);

	for (int i = 0; i < ARRAYSIZE(_slots); i++)
		_slots[i]->updateTimer(delta);

	_scrollBar->updateTimer(delta);

	for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++)
		_scrollButtons[i]->updateTimer(delta);
}

void MacIndy3Gui::Inventory::updateVerb(int verbslot) {
	MacIndy3Gui::VerbWidget::updateVerb(verbslot);

	int owner = _vm->VAR(_vm->VAR_EGO);

	int invCount = _vm->getInventoryCount(owner);
	int invOffset = _gui->getInventoryScrollOffset();
	int numSlots = ARRAYSIZE(_slots);

	// The scroll offset must be non-negative and if there are numSlots or
	// less objects in the inventory, the inventory is fixed in the top
	// position.

	if (invOffset < 0 || invCount <= numSlots)
		invOffset = 0;

	// If there are more than numSlots objects in the inventory, clamp the
	// scroll offset so that the inventory does not go past the last
	// numSlots objets.

	if (invCount > numSlots && invOffset > invCount - numSlots)
		invOffset = invCount - numSlots;

	_scrollButtons[0]->setEnabled(invOffset > 0);
	_scrollButtons[1]->setEnabled(invCount > numSlots && invOffset < invCount - numSlots);

	_scrollBar->setEnabled(invCount > numSlots);
	_scrollBar->setInventoryParameters(invCount, invOffset);

	_gui->setInventoryScrollOffset(invOffset);

	int invSlot = 0;

	// Assign the objects to the inventory slots

	for (int i = 0; i < _vm->_numInventory && invSlot < numSlots; i++) {
		int obj = _vm->_inventory[i];
		if (obj && _vm->getOwner(obj) == owner) {
			if (--invOffset < 0) {
				_slots[invSlot]->setObject(obj);
				invSlot++;
			}
		}
	}

	// Clear the remaining slots

	for (int i = invSlot; i < numSlots; i++)
		_slots[i]->clearObject();
}

void MacIndy3Gui::Inventory::draw() {
	if (getRedraw()) {
		debug(1, "Inventory: Drawing [%d]", _verbid);

		MacIndy3Gui::VerbWidget::draw();

		drawShadowBox(_bounds);
		drawShadowFrame(Common::Rect(_bounds.left + 4, _bounds.top + 4, _bounds.right - 22, _bounds.bottom - 4), kBlack, kWhite);

		const uint16 upArrow[] = {
			0x0000, 0x0000, 0x0000, 0x0080,
			0x0140, 0x0220, 0x0410, 0x0808,
			0x1C1C, 0x0410, 0x0410, 0x0410,
			0x07F0, 0x0000, 0x0000, 0x0000
		};

		const uint16 downArrow[] = {
			0x0000, 0x0000, 0x0000, 0x0000,
			0x07F0, 0x0410, 0x0410, 0x0410,
			0x1C1C, 0x0808, 0x0410, 0x0220,
			0x0140, 0x0080, 0x0000, 0x0000
		};

		for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++) {
			ScrollButton *s = _scrollButtons[i];
			const uint16 *arrow = (s->_direction == kScrollUp) ? upArrow : downArrow;

			drawShadowFrame(s->getBounds(), kWhite, kTransparency);
			drawBitmap(s->getBounds(), arrow, kBlack);
			s->draw();
		}
	}

	// Since the inventory slots overlap, draw the highlighted ones (and
	// there should really only be one at a time) last.

	for (int i = 0; i < ARRAYSIZE(_slots); i++) {
		if (!_slots[i]->hasTimer())
			_slots[i]->draw();
	}

	for (int i = 0; i < ARRAYSIZE(_slots); i++) {
		if (_slots[i]->hasTimer())
			_slots[i]->draw();
	}

	_scrollBar->draw();

	for (int i = 0; i < ARRAYSIZE(_scrollButtons); i++)
		_scrollButtons[i]->draw();
}

// ---------------------------------------------------------------------------
// Inventory::Slot is a widget for a single inventory object.
// ---------------------------------------------------------------------------

MacIndy3Gui::Inventory::Slot::Slot(int slot, int x, int y, int width, int height) : MacIndy3Gui::Widget(x, y, width, height) {
	_slot = slot;
}

void MacIndy3Gui::Inventory::Slot::reset() {
	Widget::reset();
	clearName();
	clearObject();
}

void MacIndy3Gui::Inventory::Slot::clearObject() {
	_obj = -1;
	setEnabled(false);

	if (hasName()) {
		clearName();
		setRedraw(true);
	}
}

void MacIndy3Gui::Inventory::Slot::setObject(int obj) {
	_obj = obj;

	const byte *ptr = _vm->getObjOrActorName(obj);

	if (ptr) {
		byte buf[270];
		_vm->convertMessageToString(ptr, buf, sizeof(buf));

		if (_name != (const char *)buf) {
			setEnabled(true);
			_name = (const char *)buf;
			clearTimer();
			setRedraw(true);
		}
	} else if (hasName()) {
		setEnabled(false);
		clearName();
		clearTimer();
		setRedraw(true);
	}
}

bool MacIndy3Gui::Inventory::Slot::handleEvent(Common::Event &event) {
	if (!_enabled || event.type != Common::EVENT_LBUTTONDOWN)
		return false;

	if (_bounds.contains(event.mouse)) {
		setRedraw(true);
		if (hasTimer())
			timeOut();
		setTimer(WIDGET_TIMER_JIFFIES);
		return true;
	}

	return false;
}

void MacIndy3Gui::Inventory::Slot::timeOut() {
	_vm->runInputScript(kInventoryClickArea, getObject(), 1);
	setRedraw(true);
}

void MacIndy3Gui::Inventory::Slot::draw() {
	if (!getRedraw())
		return;

	debug(1, "Inventory::Slot: Drawing [%d] %s", _slot, _name.c_str());

	Widget::draw();

	Color fg, bg;

	if (hasTimer()) {
		fg = kWhite;
		bg = kBlack;
	} else {
		fg = kBlack;
		bg = kWhite;
	}

	_surface->fillRect(_bounds, bg);

	if (hasName()) {
		const Graphics::Font *font = _gui->getFont(kIndy3VerbFontRegular);

		int y = _bounds.top - 1;
		int x = _bounds.left + 4;

		for (uint i = 0; i < _name.size() && x < _bounds.right; i++) {
			byte c = translateChar(_name[i]);

			font->drawChar(_surface, c, x, y, fg);
			x += font->getCharWidth(c);
		}
	}
}

// ---------------------------------------------------------------------------
// Inventory::ScrollBar is the slider which shows if the inventory contains
// more objects than are visible on screen.
// ---------------------------------------------------------------------------

// NB: This class makes several references to ARRAYSIZE(_slots), but accessing
//     members of the enclosing class like that should be ok in C++11.

MacIndy3Gui::Inventory::ScrollBar::ScrollBar(int x, int y, int width, int height) : MacIndy3Gui::Widget(x, y, width, height) {
}

bool MacIndy3Gui::Inventory::ScrollBar::handleEvent(Common::Event &event) {
	if (!_enabled || event.type != Common::EVENT_LBUTTONDOWN)
		return false;

	// Clicking on the scrollbar scrolls it to the top or the bottom, not
	// one page as one might suspect. Though you're rarely carrying enough
	// objects for this to make a difference.
	//
	// The direction depends on if you click above or below the handle.
	// Clicking on the handle also works, though the behavior strikes me
	// as a bit unintuitive. If you click on Y coordinate pos + 5, nothing
	// happens at all.

	if (_bounds.contains(event.mouse)) {
		int pos = _bounds.top + getHandlePosition();

		if (event.mouse.y <= pos + 4)
			_invOffset = 0;
		else if (event.mouse.y >= pos + 6)
			_invOffset = _invCount - ARRAYSIZE(_slots);

		_gui->setInventoryScrollOffset(_invOffset);
		setRedraw(true);
	}

	return false;
}

void MacIndy3Gui::Inventory::ScrollBar::setInventoryParameters(int invCount, int invOffset) {
	if (invOffset != _invOffset)
		setRedraw(true);

	if (invCount != _invCount && _invCount >= ARRAYSIZE(_slots))
		setRedraw(true);

	_invCount = invCount;
	_invOffset = invOffset;
}

void MacIndy3Gui::Inventory::ScrollBar::scroll(ScrollDirection dir) {
	int newOffset = _invOffset;
	int maxOffset = _invCount - ARRAYSIZE(_slots);

	if (dir == kScrollUp)
		newOffset--;
	else
		newOffset++;

	if (newOffset < 0)
		newOffset = 0;
	else if (newOffset > maxOffset)
		newOffset = maxOffset;

	if (newOffset != _invOffset) {
		_invOffset = newOffset;
		_gui->setInventoryScrollOffset(_invOffset);
		setRedraw(true);
	}
}

int MacIndy3Gui::Inventory::ScrollBar::getHandlePosition() {
	if (_invOffset == 0)
		return 0;

	// Hopefully this matches the original scroll handle position.

	int maxPos = _bounds.height() - 8;
	int maxOffset = _invCount - ARRAYSIZE(_slots);

	if (_invOffset >= maxOffset)
		return maxPos;

	return maxPos * _invOffset / maxOffset;
}

void MacIndy3Gui::Inventory::ScrollBar::reset() {
	MacIndy3Gui::Widget::reset();
	_invCount = 0;
	_invOffset = 0;
}

void MacIndy3Gui::Inventory::ScrollBar::draw() {
	if (!getRedraw())
		return;

	debug(1, "Inventory::Scrollbar: Drawing");

	Widget::draw();
	drawShadowFrame(_bounds, kBlack, kBackground);

	// The scrollbar handle is only drawn when there are enough inventory
	// objects to scroll.

	if (_enabled) {
		debug(1, "Inventory::Scrollbar: Drawing handle");

		int y = _bounds.top + getHandlePosition();

		// The height of the scrollbar handle never changes, regardless
		// of how many items you are carrying.
		drawShadowFrame(Common::Rect(_bounds.left, y, _bounds.right, y + 8), kWhite, kTransparency);
	}

	setRedraw(false);
	markScreenAsDirty(_bounds);
}

// ---------------------------------------------------------------------------
// Inventory::ScrollButton is the buttons used to scroll the inventory up and
// down. They're only responsible for drawing the filled part of the arrow,
// since the rest of the buttons are drawn by the Inventory widget itself.
// ---------------------------------------------------------------------------

MacIndy3Gui::Inventory::ScrollButton::ScrollButton(int x, int y, int width, int height, ScrollDirection direction) : MacIndy3Gui::Widget(x, y, width, height) {
	_direction = direction;
}

bool MacIndy3Gui::Inventory::ScrollButton::handleEvent(Common::Event &event) {
	if (!_enabled || event.type != Common::EVENT_LBUTTONDOWN)
		return false;

	if (_bounds.contains(event.mouse)) {
		setRedraw(true);
		setTimer(WIDGET_TIMER_JIFFIES);
		return true;
	}

	return false;
}

bool MacIndy3Gui::Inventory::ScrollButton::handleMouseHeld(Common::Point &pressed, Common::Point &held) {
	if (!_enabled)
		return false;

	// The scroll button doesn't care if the mouse has moved outside while
	// being held.

	return _bounds.contains(pressed);
}

void MacIndy3Gui::Inventory::ScrollButton::timeOut() {
	// Nothing happens, but the button changes state.
	setRedraw(true);
}

void MacIndy3Gui::Inventory::ScrollButton::draw() {
	if (!getRedraw())
		return;

	debug(1, "Inventory::ScrollButton: Drawing [%d]", _direction);

	Widget::draw();

	const uint16 upArrow[] = {
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0080, 0x01C0, 0x03E0, 0x07F0,
		0x03E0, 0x03E0, 0x03E0, 0x03E0,
		0x0000, 0x0000, 0x0000, 0x0000
	};

	const uint16 downArrow[] = {
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x03E0, 0x03E0, 0x03E0,
		0x03E0, 0x07F0,	0x03E0, 0x01C0,
		0x0080, 0x0000, 0x0000, 0x0000
	};

	const uint16 *arrow = (_direction == kScrollUp) ? upArrow : downArrow;
	Color color = hasTimer() ? kBlack : kWhite;

	drawBitmap(_bounds, arrow, color);

	setRedraw(false);
	markScreenAsDirty(_bounds);
}

// ---------------------------------------------------------------------------
// The MacIndy3Gui class ties the whole thing together, mostly by delegating
// the work to the individual widgets.
// ---------------------------------------------------------------------------

MacIndy3Gui::MacIndy3Gui(ScummEngine *vm, Common::String resourceFile) :
	MacGui(vm, resourceFile), _visible(false) {

	Common::Rect verbGuiArea(640, 112);
	verbGuiArea.translate(0, 288 + 2 * _vm->_screenDrawOffset);

	_verbGuiTop = verbGuiArea.top;
	_verbGuiSurface = _surface->getSubArea(verbGuiArea);

	// There is one widget for every verb in the game. Verbs include the
	// inventory widget and conversation options.

	Widget::_vm = _vm;
	Widget::_surface = &_verbGuiSurface;
	Widget::_gui = this;

	_widgets[  1] = new Button(137, 24,  68, 18); // Open
	_widgets[  2] = new Button(137, 44,  68, 18); // Close
	_widgets[  3] = new Button( 67, 64,  68, 18); // Give
	_widgets[  4] = new Button(277, 44,  68, 18); // Turn on
	_widgets[  5] = new Button(277, 64,  68, 18); // Turn off
	_widgets[  6] = new Button( 67, 24,  68, 18); // Push
	_widgets[  7] = new Button( 67, 44,  68, 18); // Pull
	_widgets[  8] = new Button(277, 24,  68, 18); // Use
	_widgets[  9] = new Button(137, 64,  68, 18); // Look at
	_widgets[ 10] = new Button(207, 24,  68, 18); // Walk to
	_widgets[ 11] = new Button(207, 44,  68, 18); // Pick up
	_widgets[ 12] = new Button(207, 64,  68, 18); // What is
	_widgets[ 13] = new Button(347, 24,  68, 18); // Talk
	_widgets[ 14] = new Button( 97, 24, 121, 18); // Never mind.
	_widgets[ 32] = new Button(347, 44,  68, 18); // Travel
	_widgets[ 33] = new Button(347, 64,  68, 18); // To Indy
	_widgets[ 34] = new Button(347, 64,  68, 18); // To Henry
	_widgets[ 90] = new Button( 67,  4, 507, 18); // Travel 1
	_widgets[ 91] = new Button( 67, 24, 507, 18); // Travel 2
	_widgets[ 92] = new Button( 67, 44, 507, 18); // Travel 3
	_widgets[100] = new Button( 67,  4, 348, 18); // Sentence
	_widgets[101] = new Inventory(417, 4, 157, 78);
	_widgets[119] = new Button(324, 24,  91, 18); // Take this:
	_widgets[120] = new Button( 67,  4, 507, 18); // Converse 1
	_widgets[121] = new Button( 67, 24, 507, 18); // Converse 2
	_widgets[122] = new Button( 67, 44, 507, 18); // Converse 3
	_widgets[123] = new Button( 67, 64, 507, 18); // Converse 4
	_widgets[124] = new Button( 67, 64, 151, 18); // Converse 5
	_widgets[125] = new Button(423, 64, 151, 18); // Converse 6

	for (auto &it: _widgets)
		it._value->setVerbid(it._key);

	_dirtyRects.clear();
	_textArea.create(448, 47, Graphics::PixelFormat::createFormatCLUT8());
}

MacIndy3Gui::~MacIndy3Gui() {
	for (auto &it: _widgets)
		delete it._value;
	_textArea.free();
}

void MacIndy3Gui::setupCursor(int &width, int &height, int &hotspotX, int &hotspotY, int &animate) {
	const byte buf[15 * 15] = {
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		0, 0, 0, 0, 0, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0,
		1, 1, 1, 1, 1, 1, 0, 3, 0, 1, 1, 1, 1, 1, 1,
		0, 0, 0, 0, 0, 0, 3, 0, 3, 0, 0, 0, 0, 0, 0,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3,
		3, 3, 3, 3, 3, 3, 0, 1, 0, 3, 3, 3, 3, 3, 3
	};

	width = height = 15;
	hotspotX = hotspotY = 7;
	animate = 0;

	_windowManager->replaceCustomCursor(buf, width, height, hotspotX, hotspotY, 3);
}

const Graphics::Font *MacIndy3Gui::getFontByScummId(int32 id) {
	// The game seems to use font 0 most of the time, but during the intro
	// it switches to font 1 to print "BARNETT COLLEGE,", "NEW YORK," and
	// "1938". By the look of it, these map to the same font.
	//
	// This is different from the DOS version, where font 1 is bolder.
	switch (id) {
	case 0:
	case 1:
		return getFont(kIndy3FontMedium);

	default:
		error("MacIndy3Gui::getFontByScummId: Invalid font id %d", id);
	}
}

bool MacIndy3Gui::getFontParams(FontId fontId, int &id, int &size, int &slant) const {
	if (MacGui::getFontParams(fontId, id, size, slant))
		return true;

	// Indy 3 provides an "Indy" font in two sizes, 9 and 12, which are
	// used for the text boxes. The smaller font can be used for a
	// headline. The rest of the Indy 3 verb GUI uses Geneva.

	switch (fontId) {
	case FontId::kIndy3FontSmall:
		id = _gameFontId;
		size = 9;
		slant = Graphics::kMacFontRegular;
		return true;

	case FontId::kIndy3FontMedium:
		id = _gameFontId;
		size = 12;
		slant = Graphics::kMacFontRegular;
		return true;

	case FontId::kIndy3VerbFontRegular:
		id = Graphics::kMacFontGeneva;
		size = 9;
		slant = Graphics::kMacFontRegular;
		return true;

	case FontId::kIndy3VerbFontBold:
		id = Graphics::kMacFontGeneva;
		size = 9;
		slant = Graphics::kMacFontBold;
		return true;

	case FontId::kIndy3VerbFontOutline:
		id = Graphics::kMacFontGeneva;
		size = 9;
		slant = Graphics::kMacFontBold | Graphics::kMacFontOutline | Graphics::kMacFontCondense;
		return true;

	default:
		error("MacIndy3Gui: getFontParams: Unknown font id %d", (int)fontId);
	}

	return false;
}

void MacIndy3Gui::initTextAreaForActor(Actor *a, byte color) {
	int width = _textArea.w;
	int height = _textArea.h;

	_textArea.fillRect(Common::Rect(width, height), kBlack);

	int nameWidth = 0;

	if (a) {
		const Graphics::Font *font = getFont(kIndy3FontSmall);

		const char *name = (const char *)a->getActorName();
		int charX = 25;

		if (_vm->_renderMode == Common::kRenderMacintoshBW)
			color = kWhite;

		for (int i = 0; name[i] && nameWidth < width - 50; i++) {
			font->drawChar(&_textArea, name[i], charX, 0, color);
			nameWidth += font->getCharWidth(name[i]);
			charX += font->getCharWidth(name[i]);
		}

		font->drawChar(&_textArea, ':', charX, 0, color);
	}

	if (nameWidth) {
		_textArea.hLine(2, 3, 20, 15);
		_textArea.hLine(32 + nameWidth, 3, width - 3, 15);
	} else
		_textArea.hLine(2, 3, width - 3, 15);

	_textArea.vLine(1, 4, height - 3, 15);
	_textArea.vLine(width - 2, 4, height - 3, 15);
	_textArea.hLine(2, height - 2, width - 3, 15);
}

void MacIndy3Gui::printCharToTextArea(int chr, int x, int y, int color) {
	// In black and white mode, all text is white. Text is never disabled.
	if (_vm->_renderMode == Common::kRenderMacintoshBW)
		color = 15;

	// Since we're working with unscaled coordinates most of the time, the
	// lines of the text box weren't spaced quite as much as in the
	// original. I thought no one would notice, but I was wrong. This is
	// the best way I can think of to fix that.

	if (y > 0)
		y = 17;

	const Graphics::Font *font = getFont(kIndy3FontMedium);

	font->drawChar(&_textArea, chr, x + 5, y + 11, color);
}

bool MacIndy3Gui::handleMenu(int id, Common::String &name) {
	if (MacGui::handleMenu(id, name))
		return true;

	Common::StringArray substitutions;

	switch (id) {
	case 204:	// IQ Points
		runIqPointsDialog();
		break;

	case 205:	// Options
		runOptionsDialog();
		break;

	case 206:	// Quit
		if (runQuitDialog())
			_vm->quitGame();
		break;

	default:
		debug("MacIndy3Gui::handleMenu: Unknown menu command: %d", id);
		break;
	}

	return false;
}

void MacIndy3Gui::runAboutDialog() {
	// The About window is not a a dialog resource. Its size appears to be
	// hard-coded (416x166), and it's drawn centered. The graphics are in
	// PICT 2000.

	int width = 416;
	int height = 166;
	int x = (640 - width) / 2;
	int y = (400 - height) / 2;

	Common::Rect bounds(x, y, x + width, y + height);
	MacDialogWindow *window = createWindow(bounds);
	Graphics::Surface *pict = loadPict(2000);

	// For the background of the sprites to match the background of the
	// window, we have to move them at multiples of 4 pixels each step. We
	// cut out enough of the background so that each time they are drawn,
	// the visible remains of the previous frame is overdrawn.

	Graphics::Surface train = pict->getSubArea(Common::Rect(249, 93));

	Graphics::Surface trolley[3];

	for (int i = 0; i < 3; i++)
		trolley[i] = pict->getSubArea(Common::Rect(251 + 92 * i, 38, 335 + 92 * i, 93));

	clearAboutDialog(window);
	window->show();

	Graphics::Surface *s = window->innerSurface();
	Common::Rect clipRect(2, 2, s->w - 4, s->h - 4);

	Common::Rect r1(22, 6, 382, 102);
	Common::Rect r2(22, 6, 382, 70);

	// Judging by recordings of Basilisk II, the internal frame rate is
	// 10 fps.

	int scene = 0;
	int status = 0;

	int trainX = -2;
	int trolleyX = width + 1;
	int trolleyFrame = 1;
	int trolleyFrameDelta = 1;
	int trolleyWaitFrames = 20;	// ~2 seconds
	int waitFrames = 0;

	// TODO: These strings are part of the STRS resource, but I don't know
	// how to safely read them from there yet. So hard-coded it is for now.

	const TextLine page1[] = {
		{ 0, 4, kStyleHeader, Graphics::kTextAlignCenter, "Indiana Jones and the Last Crusade" },
		{ 0, 22, kStyleBold, Graphics::kTextAlignCenter, "The Graphic Adventure" },
		{ 0, 49, kStyleBold, Graphics::kTextAlignCenter, "Mac 1.7 8/17/90, Interpreter version 5.1.6" },
		{ 1, 82, kStyleRegular, Graphics::kTextAlignCenter, "TM & \xA9 1990 LucasArts Entertainment Company.  All rights reserved." },
		TEXT_END_MARKER
	};

	const TextLine page2[] = {
		{ 1, 7, kStyleBold, Graphics::kTextAlignCenter, "Macintosh version by" },
		{ 70, 21, kStyleHeader, Graphics::kTextAlignLeft, "Eric Johnston" },
		{ 194, 32, kStyleBold, Graphics::kTextAlignLeft, "and" },
		{ 216, 41, kStyleHeader, Graphics::kTextAlignLeft, "Dan Filner" },
		TEXT_END_MARKER
	};

	const TextLine page3[] = {
		{ 1, 7, kStyleBold, Graphics::kTextAlignCenter, "Macintosh scripting by" },
		{ 75, 21, kStyleHeader, Graphics::kTextAlignLeft, "Ron Baldwin" },
		{ 186, 32, kStyleBold, Graphics::kTextAlignLeft, "and" },
		{ 214, 41, kStyleHeader, Graphics::kTextAlignLeft, "David Fox" },
		TEXT_END_MARKER
	};

	const TextLine page4[] = {
		{ 1, 7, kStyleBold, Graphics::kTextAlignCenter, "Designed and scripted by" },
		{ 77, 24, kStyleHeader, Graphics::kTextAlignLeft, "Noah Falstein" },
		{ 134, 44, kStyleHeader, Graphics::kTextAlignLeft, "David Fox" },
		{ 167, 64, kStyleHeader, Graphics::kTextAlignLeft, "Ron Gilbert" },
		TEXT_END_MARKER
	};

	const TextLine page5[] = {
		{ 1, 7, kStyleBold, Graphics::kTextAlignCenter, "SCUMM Story System" },
		{ 1, 17, kStyleBold, Graphics::kTextAlignCenter, "created by" },
		{ 107, 36, kStyleHeader, Graphics::kTextAlignLeft, "Ron Gilbert" },
		{ 170, 52, kStyleBold, Graphics::kTextAlignLeft, "and" },
		{ 132, 66, kStyleHeader, Graphics::kTextAlignLeft, "Aric Wilmunder" },
		TEXT_END_MARKER
	};

	const TextLine page6[] = {
		{ 1, 19, kStyleBold, Graphics::kTextAlignCenter, "Stumped?  Indy hint books are available!" },
		{ 86, 36, kStyleRegular, Graphics::kTextAlignLeft, "In the U.S. call" },
		{ 160, 37, kStyleBold, Graphics::kTextAlignLeft, "1 (800) STAR-WARS" },
		{ 160, 46, kStyleRegular, Graphics::kTextAlignLeft, "that\xD5s  1 (800) 782-7927" },
		{ 90, 66, kStyleRegular, Graphics::kTextAlignLeft, "In Canada call" },
		{ 160, 67, kStyleBold, Graphics::kTextAlignLeft, "1 (800) 828-7927" },
		TEXT_END_MARKER
	};

	const TextLine page7[] = {
		{ 1, 17, kStyleBold, Graphics::kTextAlignCenter, "Need a hint NOW?  Having problems?" },
		{ 53, 31, kStyleRegular, Graphics::kTextAlignLeft, "For hints or technical support call" },
		{ 215, 32, kStyleBold, Graphics::kTextAlignLeft, "1 (900) 740-JEDI" },
		{ 1, 46, kStyleRegular, Graphics::kTextAlignCenter, "The charge is 75\xA2 per minute." },
		{ 1, 56, kStyleRegular, Graphics::kTextAlignCenter, "(You must have your parents\xD5 permission to" },
		{ 1, 66, kStyleRegular, Graphics::kTextAlignCenter, "call this number if you are under 18.)" },
		TEXT_END_MARKER
	};

	const TextLine page8[] = {
		{ 1, 1, kStyleBold, Graphics::kTextAlignCenter, "Click to continue" },
		TEXT_END_MARKER
	};

	bool changeScene = false;

	while (!_vm->shouldQuit()) {
		switch (scene) {
		case 0:
			window->drawSprite(&train, trainX, 40, clipRect);
			trainX -= 4;

			if (trainX < -train.w)
				changeScene = true;

			break;

		case 1:
		case 4:
		case 5:
		case 6:
		case 7:
			if (--waitFrames <= 0)
				changeScene = true;
			break;

		case 2:
		case 3:
			window->drawSprite(&trolley[trolleyFrame], trolleyX, 78, clipRect);

			if (scene == 2 && trolleyX == 161 && trolleyWaitFrames > 0) {
				if (--trolleyWaitFrames == 0)
					changeScene = true;
			} else {
				trolleyX -= 4;
				trolleyFrame += trolleyFrameDelta;
				if (trolleyFrame < 0 || trolleyFrame > 2) {
					trolleyFrame = 1;
					trolleyFrameDelta = -trolleyFrameDelta;
				}

				if (trolleyX < -85)
					changeScene = true;
			}

			break;
		}

		window->update();
		status = delay((scene == 0) ? 33 : 100);

		if (status == 2)
			break;

		if (status == 1 || changeScene) {
			changeScene = false;
			scene++;
			waitFrames = 50;	// ~5 seconds

			switch (scene) {
			case 1:
				clearAboutDialog(window);
				window->drawTextBox(r1, page1);
				break;

			case 2:
				clearAboutDialog(window);
				window->drawTextBox(r2, page2);
				break;

			case 3:
				// Don't clear. The trolley is still on screen
				// and only the text changes.
				window->drawTextBox(r2, page3);
				break;

			case 4:
				clearAboutDialog(window);
				window->drawTextBox(r1, page4);
				break;

			case 5:
				window->drawTextBox(r1, page5);
				break;

			case 6:
				waitFrames = 100;	// ~10 seconds
				window->drawTextBox(r1, page6);
				break;

			case 7:
				waitFrames = 30;	// ~3 seconds
				window->drawTextBox(r1, page7);
				break;

			case 8:
				window->drawTextBox(Common::Rect(142, 106, 262, 119), page8, 3);
				break;
			}

			window->update();

			if (scene >= 8)
				break;
		}
	}

	if (status != 2)
		delay();

	_windowManager->popCursor();

	pict->free();
	delete pict;
	delete window;
}

void MacIndy3Gui::clearAboutDialog(MacDialogWindow *window) {
	Graphics::Surface *s = window->innerSurface();

	window->fillPattern(Common::Rect(2, 2, s->w - 2, 132), 0x8020);
	window->fillPattern(Common::Rect(2, 130, s->w - 2, 133), 0xA5A5);
	window->fillPattern(Common::Rect(2, 133, s->w - 2, 136), 0xFFFF);
	window->fillPattern(Common::Rect(2, 136, s->w - 2, s->h - 4), 0xA5A5);
}

bool MacIndy3Gui::runOpenDialog(int &saveSlotToHandle) {
	// Widgets:
	//
	// 0 - Open button
	// 1 - Weird button outside the dialog (folder dropdown?)
	// 2 - Cancel button
	// [skipped] - User item (disk label?)
	// 3 - Eject button
	// 4 - Drive button
	// [skipped] - User item (file list?)
	// [skipped] - User item (scrollbar?)
	// [skipped] - User item (line between Desktop and Open buttons?)
	// 5 - Empty text
	// 6 - "IQ" picture
	// 7 - "Episode: ^0" text
	// 8 - "Series: ^1" text
	// 9 - "(Indy Quotient)" text

	MacDialogWindow *window = createDialog((_vm->_renderMode == Common::kRenderMacintoshBW) ? 4000 : 4001);

	window->setDefaultWidget(0);
	window->addSubstitution(Common::String::format("%d", _vm->VAR(244)));
	window->addSubstitution(Common::String::format("%d", _vm->VAR(245)));

	bool availSlots[100];
	int slotIds[100];
	Common::StringArray savegameNames;
	prepareSaveLoad(savegameNames, availSlots, slotIds, ARRAYSIZE(availSlots));

	MacGui::MacListBox *listBox = window->addListBox(Common::Rect(14, 41, 248, 187), savegameNames, true);

	// When quitting, the default action is to not open a saved game
	bool ret = false;
	Common::Array<int> deferredActionsIds;

	while (!_vm->shouldQuit()) {
		int clicked = window->runDialog(deferredActionsIds);

		if (clicked == 0 || clicked == 10) {
			ret = true;
			saveSlotToHandle =
				listBox->getValue() < ARRAYSIZE(slotIds) ? slotIds[listBox->getValue()] : -1;
			break;
		}

		if (clicked == 2)
			break;
	}

	delete window;
	return ret;
}

bool MacIndy3Gui::runSaveDialog(int &saveSlotToHandle, Common::String &name) {
	// Widgets:
	//
	// 0 - Save button
	// 1 - Cancel button
	// 2 - "Save Game File as..." text
	// [skipped] - User item (disk label?)
	// 3 - Eject button
	// 4 - Drive button
	// 5 - Editable text (save file name)
	// [skipped] - User item (file list?)
	// 6 - "IQ" picture
	// 7 - "Episode: ^0" text
	// 8 - "Series: ^1" text
	// 9 - "(Indy Quotient)" text

	MacDialogWindow *window = createDialog((_vm->_renderMode == Common::kRenderMacintoshBW) ? 3998 : 3999);

	window->setDefaultWidget(0);
	window->addSubstitution(Common::String::format("%d", _vm->VAR(244)));
	window->addSubstitution(Common::String::format("%d", _vm->VAR(245)));

	bool busySlots[100];
	int slotIds[100];
	Common::StringArray savegameNames;
	prepareSaveLoad(savegameNames, busySlots, slotIds, ARRAYSIZE(busySlots));

	int firstAvailableSlot = -1;
	for (int i = 0; i < ARRAYSIZE(busySlots); i++) {
		if (!busySlots[i]) {
			firstAvailableSlot = i;
			break;
		}
	}

	window->addListBox(Common::Rect(16, 31, 199, 129), savegameNames, true, true);

	// When quitting, the default action is not to save a game
	bool ret = false;
	Common::Array<int> deferredActionsIds;

	while (!_vm->shouldQuit()) {
		int clicked = window->runDialog(deferredActionsIds);

		if (clicked == 0) {
			ret = true;
			name = window->getWidget(5)->getText(); // Edit text widget
			saveSlotToHandle = firstAvailableSlot;
			break;
		}

		if (clicked == 1)
			break;

		if (clicked == -2) {
			// Cycle through deferred actions
			for (uint i = 0; i < deferredActionsIds.size(); i++) {
				// Edit text widget
				if (deferredActionsIds[i] == 5) {
					MacGui::MacWidget *wid = window->getWidget(deferredActionsIds[i]);

					// Disable "Save" button when text is empty
					window->getWidget(0)->setEnabled(!wid->getText().empty());
				}
			}
		}
	}

	delete window;
	return ret;
}

bool MacIndy3Gui::runOptionsDialog() {
	// Widgets:
	//
	// 0 - Okay button
	// 1 - Cancel button
	// 2 - Sound checkbox
	// 3 - Music checkbox
	// 4 - Picture (text speed background)
	// 5 - Picture (text speed handle)
	// 6 - "Machine speed rating:" text
	// 7 - "^0" text
	// 8 - Scrolling checkbox
	// 9 - Text speed slider (manually created)

	int sound = _vm->_mixer->isSoundTypeMuted(Audio::Mixer::SoundType::kSFXSoundType) ? 0 : 1;
	int music = _vm->_mixer->isSoundTypeMuted(Audio::Mixer::SoundType::kPlainSoundType) ? 0 : 1;

	int scrolling = _vm->_snapScroll == 0;
	int textSpeed = _vm->_defaultTextSpeed;

	MacDialogWindow *window = createDialog(1000);

	window->setWidgetValue(2, sound);
	window->setWidgetValue(3, music);
	window->setWidgetValue(8, scrolling);

	if (!sound)
		window->setWidgetEnabled(3, false);

	window->addPictureSlider(4, 5, true, 5, 105, 0, 9);
	window->setWidgetValue(9, textSpeed);

	window->addSubstitution(Common::String::format("%d", _vm->VAR(_vm->VAR_MACHINE_SPEED)));

	// When quitting, the default action is not to not apply options
	bool ret = false;
	Common::Array<int> deferredActionsIds;

	while (!_vm->shouldQuit()) {
		int clicked = window->runDialog(deferredActionsIds);

		if (clicked == 0) {
			ret = true;
			break;
		}

		if (clicked == 1)
			break;

		if (clicked == 2)
			window->setWidgetEnabled(3, window->getWidgetValue(2) != 0);
	}

	if (ret) {
		// Update settings

		// TEXT SPEED
		_vm->_defaultTextSpeed = CLIP<int>(window->getWidgetValue(9), 0, 9);
		ConfMan.setInt("original_gui_text_speed", _vm->_defaultTextSpeed);
		_vm->setTalkSpeed(_vm->_defaultTextSpeed);
		_vm->syncSoundSettings();

		// SOUND&MUSIC ACTIVATION
		// 0 - Sound&Music on
		// 1 - Sound on, music off
		// 2 - Sound&Music off
		bool disableSound = window->getWidgetValue(2) == 0;
		bool disableMusic = window->getWidgetValue(3) == 0;
		_vm->_mixer->muteSoundType(Audio::Mixer::SoundType::kSFXSoundType, disableSound);
		_vm->_mixer->muteSoundType(Audio::Mixer::SoundType::kPlainSoundType, disableMusic || disableSound);

		// SCROLLING ACTIVATION
		_vm->_snapScroll = window->getWidgetValue(8) == 0;

		ConfMan.flushToDisk();
	}

	delete window;
	return ret;
}

bool MacIndy3Gui::runIqPointsDialog() {
	// Widgets
	//
	// 0 - Done button
	// 1 - Reset Series IQ button
	// 2 - "(Indy Quotient)" text
	// 3 - "Episode: ^0" text
	// 4 - "Series: ^1" text
	// 5 - "IQ" picture

	MacDialogWindow *window = createDialog((_vm->_renderMode == Common::kRenderMacintoshBW) ? 1001 : 1002);

	((ScummEngine_v4 *)_vm)->updateIQPoints();
	window->addSubstitution(Common::String::format("%d", _vm->VAR(244)));
	window->addSubstitution(Common::String::format("%d", _vm->VAR(245)));

	Common::Array<int> deferredActionsIds;

	while (!_vm->shouldQuit()) {
		int clicked = window->runDialog(deferredActionsIds);

		if (clicked == 0)
			break;

		if (clicked == 1) {
			((ScummEngine_v4 *)_vm)->clearSeriesIQPoints();
			window->replaceSubstitution(1, Common::String::format("%d", _vm->VAR(245)));
			window->redrawWidget(4);
		}
	}

	delete window;
	return true;
}

// Before the GUI rewrite, the scroll offset was saved in variable 67. Let's
// continue that tradition, just in case. If nothing else, it gives us an easy
// way to still store it in savegames.

int MacIndy3Gui::getInventoryScrollOffset() const {
	return _vm->VAR(67);
}

void MacIndy3Gui::setInventoryScrollOffset(int n) const {
	_vm->VAR(67) = n;
}

bool MacIndy3Gui::isVerbGuiAllowed() const {
	// The GUI is only allowed if the verb area has the expected size. That
	// really seems to be all that's needed.

	VirtScreen *vs = &_vm->_virtscr[kVerbVirtScreen];
	if (vs->topline != 144 + _vm->_screenDrawOffset || vs->h != 56 + _vm->_screenDrawOffset)
		return false;

	// HACK: Don't allow the GUI during fist fights. Usually this is not a
	//       problem, in my experience, but I've had it happening when
	//       offering an item to a guard led directly to one.

	if (_vm->VAR(_vm->VAR_VERB_SCRIPT) == 19)
		return false;

	return true;
}

bool MacIndy3Gui::isVerbGuiActive() const {
	// The visibility flag may not have been updated yet, so better check
	// that the GUI is still allowed.

	return _visible && isVerbGuiAllowed();
}

void MacIndy3Gui::reset() {
	_visible = false;

	for (auto &it: _widgets)
		it._value->reset();
}

void MacIndy3Gui::resetAfterLoad() {
	reset();

	// In the DOS version, verb ID 102-106 were used for the visible
	// inventory items, and 107-108 for inventory arrow buttons. In the
	// Macintosh version, the entire inventory widget is verb ID 101.
	//
	// In old savegames, the DOS verb IDs may still be present, and have
	// to be removed.

	for (int i = 0; i < _vm->_numVerbs; i++) {
		if (_vm->_verbs[i].verbid >= 102 && _vm->_verbs[i].verbid <= 108)
			_vm->killVerb(i);
	}

	int charsetId = _vm->_charset->getCurID();
	if (charsetId < 0 || charsetId > 1)
		_vm->_charset->setCurID(0);
}

void MacIndy3Gui::update(int delta) {
	if (isVerbGuiAllowed() && updateVerbs(delta)) {
		if (!_visible)
			show();

		updateMouseHeldTimer(delta);
		drawVerbs();
	} else {
		if (_visible)
			hide();
	}

	copyDirtyRectsToScreen();
}

bool MacIndy3Gui::updateVerbs(int delta) {
	// Tentatively mark the verb widgets for removal. Any widget that wants
	// to stay has to say so.

	for (auto &it: _widgets) {
		VerbWidget *w = it._value;

		if (delta > 0)
			w->updateTimer(delta);

		w->threaten();
	}

	bool hasActiveVerbs = false;

	// Collect all active verbs. Verb slot 0 is special, apparently, so we
	// don't look at that one.

	for (int i = 1; i < _vm->_numVerbs; i++) {
		VerbSlot *vs = &_vm->_verbs[i];

		if (!vs->saveid && vs->curmode && vs->verbid) {
			VerbWidget *w = _widgets.getValOrDefault(vs->verbid);

			if (w) {
				w->updateVerb(i);
				hasActiveVerbs = true;
			} else {
				const byte *ptr = _vm->getResourceAddress(rtVerb, i);
				byte buf[270];
				_vm->convertMessageToString(ptr, buf, sizeof(buf));
				warning("MacIndy3Gui: Unknown verb: %d %s", vs->verbid, buf);
			}
		}
	}

	return hasActiveVerbs;
}

void MacIndy3Gui::updateMouseHeldTimer(int delta) {
	if (delta > 0 && _leftButtonIsPressed) {
		_timer -= delta;

		if (_timer <= 0) {
			debug(2, "MacIndy3Gui: Left button still down");

			_timer = REPEAT_TIMER_JIFFIES;

			for (auto &it: _widgets) {
				if (it._value->handleMouseHeld(_leftButtonPressed, _leftButtonHeld))
					break;
			}
		}
	}
}

void MacIndy3Gui::drawVerbs() {
	// The possible verbs overlap each other. Remove the dead ones first, then draw the live ones.

	for (auto &it: _widgets) {
		VerbWidget *w = it._value;

		if (w->isDying() && w->isVisible()) {
			w->undraw();
			w->reset();
		}
	}

	for (auto &it: _widgets) {
		VerbWidget *w = it._value;

		if (w->hasVerb())
			w->draw();
	}
}

bool MacIndy3Gui::handleEvent(Common::Event event) {
	if (MacGui::handleEvent(event))
		return true;

	bool isPauseEvent = event.type == Common::EVENT_KEYDOWN &&
		event.kbd == Common::KEYCODE_SPACE;

	if (!isPauseEvent && (!isVerbGuiActive() || _vm->_userPut <= 0))
		return false;

	// Make all mouse events relative to the verb GUI area

	if (Common::isMouseEvent(event))
		event.mouse.y -= _verbGuiTop;

	if (event.type == Common::EVENT_LBUTTONDOWN) {
		if (!_leftButtonIsPressed) {
			debug(2, "MacIndy3Gui: Left button down");

			_leftButtonIsPressed = true;
			_leftButtonPressed = event.mouse;
			_leftButtonHeld = event.mouse;
			_timer = REPEAT_TIMER_JIFFIES;
		}
	} else if (event.type == Common::EVENT_LBUTTONUP) {
		if (_leftButtonIsPressed) {
			debug(2, "MacIndy3Gui: Left button up");

			_leftButtonIsPressed = false;
			_timer = 0;
		}
	} else if (event.type == Common::EVENT_MOUSEMOVE) {
		if (_leftButtonIsPressed) {
			_leftButtonHeld.x = event.mouse.x;
			_leftButtonHeld.y = event.mouse.y;
		}
	}

	// It probably doesn't make much of a difference, but if a widget
	// responds to an event, and marks itself as wanting to be redrawn,
	// we do that redrawing immediately, not on the next update.

	for (auto &it: _widgets) {
		Widget *w = it._value;

		if (w->handleEvent(event)) {
			if (w->getRedraw()) {
				w->draw();
				copyDirtyRectsToScreen();
			}
			return true;
		}
	}

	return false;
}

void MacIndy3Gui::show() {
	if (_visible)
		return;

	debug(1, "MacIndy3Gui: Showing");

	_visible = true;

	// The top and bottom of the verb GUI area black. On a screen that's
	// 400 pixels tall, the verb GUI extends all the way to the bottom. On
	// a 480 pixel tall screen there will be an additional 40 pixels below
	// that, but nothing should ever be drawn there and if it ever is, it's
	// not the verb GUI's responsibility to clear it.

	_verbGuiSurface.fillRect(Common::Rect(0, 0, 640, 1), kBlack);
	_verbGuiSurface.fillRect(Common::Rect(0, 85, 640, _verbGuiSurface.h), kBlack);

	fill(Common::Rect(0, 2, 640, 85));

	const uint16 ulCorner[] = { 0xF000, 0xC000, 0x8000, 0x8000 };
	const uint16 urCorner[] = { 0xF000, 0x3000, 0x1000, 0x1000 };
	const uint16 llCorner[] = { 0x8000, 0x8000, 0xC000, 0xF000 };
	const uint16 lrCorner[] = { 0x1000, 0x1000, 0x3000, 0xF000 };

	drawBitmap(&_verbGuiSurface, Common::Rect(  0,  2,   4,  6), ulCorner, kBlack);
	drawBitmap(&_verbGuiSurface, Common::Rect(636,  2, 640,  6), urCorner, kBlack);
	drawBitmap(&_verbGuiSurface, Common::Rect(  0, 81,   4, 85), llCorner, kBlack);
	drawBitmap(&_verbGuiSurface, Common::Rect(636, 81, 640, 85), lrCorner, kBlack);

	markScreenAsDirty(Common::Rect(_verbGuiSurface.w, _verbGuiSurface.h));
}

void MacIndy3Gui::hide() {
	if (!_visible)
		return;

	debug(1, "MacIndy3Gui: Hiding");

	_leftButtonIsPressed = false;
	_timer = 0;

	reset();

	// If the verb GUI is allowed, the area should be cleared. If the verb
	// GUI is not allowed, the game is presumably using it for its own
	// drawing, and we should leave it alone.

	if (isVerbGuiAllowed()) {
		_verbGuiSurface.fillRect(Common::Rect(_verbGuiSurface.w, _verbGuiSurface.h), kBlack);
		markScreenAsDirty(Common::Rect(_verbGuiSurface.w, _verbGuiSurface.h));
	}
}

void MacIndy3Gui::markScreenAsDirty(Common::Rect r) {
	// As long as we always call this with the most encompassing rect
	// first, it is trivial to filter out unnecessary calls.

	for (uint i = 0; i < _dirtyRects.size(); i++) {
		if (_dirtyRects[i].contains(r))
			return;
	}

	_dirtyRects.push_back(r);
}

void MacIndy3Gui::copyDirtyRectsToScreen() {
	for (uint i = 0; i < _dirtyRects.size(); i++) {
		_system->copyRectToScreen(
			_verbGuiSurface.getBasePtr(_dirtyRects[i].left, _dirtyRects[i].top),
			_verbGuiSurface.pitch,
			_dirtyRects[i].left, _verbGuiTop + _dirtyRects[i].top,
			_dirtyRects[i].width(), _dirtyRects[i].height());
	}

	_dirtyRects.clear();
}

void MacIndy3Gui::fill(Common::Rect r) {
	int pitch = _verbGuiSurface.pitch;

	// Fill the screen with either gray of a checkerboard pattern.

	if (_vm->_renderMode == Common::kRenderMacintoshBW) {
		byte *row = (byte *)_verbGuiSurface.getBasePtr(r.left, r.top);

		for (int y = r.top; y < r.bottom; y++) {
			byte *ptr = row;
			for (int x = r.left; x < r.right; x++)
				*ptr++ = ((x + y) & 1) ? kWhite : kBlack;
			row += pitch;
		}
	} else
		_verbGuiSurface.fillRect(r, kLightGray);
}

} // End of namespace Scumm
