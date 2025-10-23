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

#include "common/timer.h"
#include "common/system.h"

#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/mactextwindow.h"
#include "graphics/macgui/macmenu.h"

namespace Graphics {

enum {
	kConWOverlap = 20,
	kConHOverlap = 20,
	kConWPadding = 3,
	kConHPadding = 4,
	kConOverscan = 3,
	kConScrollStep = 12,
};

MacTextWindow::MacTextWindow(MacWindowManager *wm, const MacFont *font, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment, MacMenu *menu, int padding) :
		MacWindow(wm->getLastId(), true, true, true, wm), _bgcolor(bgcolor), _maxWidth(maxWidth), _menu(menu) {

	_font = font;
	_mactext = new MacText(this, 0, 0, 0, 0, _wm, Common::U32String(""), font, fgcolor, bgcolor, maxWidth, textAlignment, 0, padding);

	_fontRef = wm->_fontMan->getFont(*font);

	init();
}

MacTextWindow::MacTextWindow(MacWindowManager *wm, const Font *font, int fgcolor, int bgcolor, int maxWidth, TextAlign textAlignment, MacMenu *menu, int padding) :
		MacWindow(wm->getLastId(), true, true, true, wm), _bgcolor(bgcolor), _maxWidth(maxWidth), _menu(menu) {

	_font = nullptr;
	_mactext = new MacText(Common::U32String(""), _wm, font, fgcolor, bgcolor, maxWidth, textAlignment, 0, padding);

	_fontRef = font;

	init();
}

void MacTextWindow::init() {
	_inputTextHeight = 0;

	_inputIsDirty = true;

	_editable = true;
	_selectable = true;

	_textColorRGB = 0;

	// Disable autoselect on activation
	_mactext->setAutoSelect(false);

	if (_wm->_mode & kWMModeWin95) {
		// in win95 mode, we set scrollbar as default
		_hasScrollBar = true;
		setBorderType(kWinBorderWin95Scrollbar);
		loadInternalBorder(kWindowBorderScrollbar | kWindowBorderActive);
	}
}

void MacTextWindow::resize(int w, int h) {
	if (_composeSurface->w == w && _composeSurface->h == h)
		return;

	undrawInput();

	MacWindow::resize(w, h);

	_maxWidth = getInnerDimensions().width();
	_mactext->resize(_maxWidth, h);
}

void MacTextWindow::setDimensions(const Common::Rect &r) {
	resize(r.width(), r.height());
	_dims.moveTo(r.left, r.top);
	updateInnerDims();

	_contentIsDirty = true;
	_wm->setFullRefresh(true);

	_mactext->setDimensions(Common::Rect(_innerDims.width(), _innerDims.height()));
}

void MacTextWindow::appendText(const Common::U32String &str, const MacFont *macFont, bool skipAdd) {
	// the reason we put undrawInput here before appendText, is we don't want the appended text affect our input
	// thus, we first delete all of out input, and we append new text, and we redraw the input
	undrawInput();
	// we calc the rgb though fgcolor
	uint16 red = (_textColorRGB >> 16) & 0xFF;
	uint16 green = (_textColorRGB >> 8) & 0xFF;
	uint16 blue = (_textColorRGB) & 0xFF;

	if (macFont)
		_mactext->appendText(str, macFont->getId(), macFont->getSize(), macFont->getSlant(), red, green, blue, skipAdd);
	else {
		_mactext->appendText(str, _fontRef, red, green, blue, skipAdd);
	}

	_contentIsDirty = true;
	_inputIsDirty = true;	//force it to redraw input

	if (_editable) {
		_mactext->_scrollPos = MAX<int>(0, _mactext->getTextHeight() - getInnerDimensions().height());
	}

	if (_wm->_mode & kWMModeWin95)
		calcScrollBar();
	// if we enable the dynamic scrollbar, and the text height is smaller than window height, then we disable the border
	// if the window is editable, then we don't disable the border, because in editable window, the area you scroll is always bigger
	// than the window
	if (!_editable && (_mode & kWindowModeDynamicScrollbar) && _mactext->getTextHeight() < getInnerDimensions().height()) {
		int w = getDimensions().width();
		int h = getDimensions().height();
		enableScrollbar(false);
		disableBorder();
		resize(w, h);
		_mactext->_fullRefresh = true;
	}
	// if the text height is bigger than the window, then we enable the scrollbar again
	if (!_editable && (_mode & kWindowModeDynamicScrollbar) && _mactext->getTextHeight() > getInnerDimensions().height()) {
		enableScrollbar(true);
		int w = getDimensions().width();
		int h = getDimensions().height();
		resize(w, h);
		_mactext->_fullRefresh = true;
	}
}

void MacTextWindow::appendText(const Common::String &str, const MacFont *macFont, bool skipAdd) {
	appendText(Common::U32String(str), macFont, skipAdd);
}

void MacTextWindow::setMarkdownText(const Common::U32String &str) {
	_mactext->setMarkdownText(str);

	_contentIsDirty = true;
}

void MacTextWindow::clearText() {
	_mactext->clearText();

	_contentIsDirty = true;
	_borderIsDirty = true;
}

MacTextWindow::~MacTextWindow() {
	delete _mactext;
}

void MacTextWindow::setTextWindowFont(const MacFont *font) {
	_font = font;

	_fontRef = _wm->_fontMan->getFont(*font);

	_mactext->setDefaultFormatting(font->getId(), font->getSlant(), font->getSize(), 0, 0, 0);
}

const MacFont *MacTextWindow::getTextWindowFont() {
	return _font;
}

bool MacTextWindow::draw(bool forceRedraw) {
	if (!_borderIsDirty && !_contentIsDirty && !_mactext->needsRedraw() && !_inputIsDirty && !forceRedraw)
		return false;

	if (_borderIsDirty || forceRedraw) {
		drawBorder();

		if (_wm->_mode & kWMModeWin95) {
			_composeSurface->clear(_bgcolor);
		} else {
			_composeSurface->clear(_wm->_colorWhite);
		}
	}

	if (_inputIsDirty || forceRedraw) {
		drawInput();
		_inputIsDirty = false;
	}

	_contentIsDirty = false;

	// Compose
	_mactext->draw(_composeSurface, _inputIsDirty || forceRedraw);

	return true;
}

bool MacTextWindow::draw(ManagedSurface *g, bool forceRedraw) {
	if (!draw(forceRedraw))
		return false;

	g->blitFrom(*_composeSurface, Common::Rect(0, 0, _composeSurface->w, _composeSurface->h), Common::Point(_innerDims.left, _innerDims.top));
	uint32 transcolor = (_wm->_pixelformat.bytesPerPixel == 1) ? _wm->_colorGreen : 0;
	g->transBlitFrom(_borderSurface, Common::Rect(0, 0, _borderSurface.w, _borderSurface.h), Common::Point(_dims.left, _dims.top), transcolor);

	return true;
}

void MacTextWindow::blit(ManagedSurface *g, Common::Rect &dest) {
	g->transBlitFrom(*_composeSurface, _composeSurface->getBounds(), dest, _wm->_colorGreen2);
}

Common::U32String MacTextWindow::getSelection(bool formatted, bool newlines) {
	return _mactext->getSelection(formatted, newlines);
}

int MacTextWindow::getMouseLine(int x, int y) {
	// TODO: Improve the algorithm here, since after long scrolling there is
	// sometimes error of +2 rows
	x -= getInnerDimensions().left;
	y -= getInnerDimensions().top + kConScrollStep;
	return _mactext->getMouseLine(x, y);
}

void MacTextWindow::calcScrollBar() {
	// since this function only able for the window which has scroll bar
	// thus, if it doesn't has scrollbar, then we don't have to calc it
	if (!_hasScrollBar)
		return;

	int maxText = 0, maxScrollbar = 0, displayHeight = 0;

	displayHeight = getInnerDimensions().height();

	maxScrollbar = getDimensions().height() - getBorderOffsets().upperScrollHeight - getBorderOffsets().lowerScrollHeight;

	// if we enable the win95 mode but the text height is smaller than window height, then we don't draw the scrollbar
	if (_wm->_mode & kWMModeWin95 && displayHeight > _mactext->getTextHeight() && !_editable)
		return;

	if (_editable)
		maxText = _mactext->getTextHeight() + getInnerDimensions().height();
	else
		maxText = MAX<int>(_mactext->getTextHeight(), displayHeight);

	float scrollSize = (float)maxScrollbar * (float)displayHeight / (float)maxText;
	float scrollPos = (float)_mactext->_scrollPos * (float)maxScrollbar / (float)maxText;
	setScroll(scrollPos, scrollSize);
}

bool MacTextWindow::processEvent(Common::Event &event) {
	WindowClick click = isInBorder(event.mouse.x, event.mouse.y);

	if (event.type == Common::EVENT_KEYDOWN) {
		if (!_editable)
			return false;

		_wm->setActiveWindow(getId());

		if (event.kbd.flags & (Common::KBD_ALT | Common::KBD_CTRL | Common::KBD_META)) {
			if (_mactext->processEvent(event))
				return true;
		}

		switch (event.kbd.keycode) {
		case Common::KEYCODE_BACKSPACE:
			if (!_inputText.empty()) {
				if (_mactext->hasSelection()) {
					_mactext->cutSelection();
				} else {
					_inputText.deleteLastChar();
				}
				_inputIsDirty = true;
			}
			return true;

		case Common::KEYCODE_RETURN:
			undrawInput();
			_inputIsDirty = true; // we force it to redraw input
			return false; // Pass it to the higher level for processing

		default:
			if (event.kbd.ascii == '~')
				return false;

			if (event.kbd.ascii >= 0x20 && event.kbd.ascii <= 0x7f) {
				if (_mactext->hasSelection())
					_mactext->cutSelection();
				_inputText += (char)event.kbd.ascii;
				_inputIsDirty = true;

				return true;
			}

			break;
		}
	}

	if (hasAllFocus())	// We are being dragged or resized
		return MacWindow::processEvent(event);	// Pass it to upstream

	if (event.type == Common::EVENT_WHEELUP) {
		//setHighlight(kBorderScrollUp);
		_mactext->scroll(-2);
		calcScrollBar();
		return true;
	}

	if (event.type == Common::EVENT_WHEELDOWN) {
		//setHighlight(kBorderScrollDown);
		_mactext->scroll(2);
		calcScrollBar();
		return true;
	}

	if (click == kBorderScrollUp || click == kBorderScrollDown) {
		if (event.type == Common::EVENT_LBUTTONDOWN) {
			setHighlight(click);
			_mactext->scroll(0);
			calcScrollBar();
			return true;
		} else if (event.type == Common::EVENT_LBUTTONUP) {
			switch (click) {
			case kBorderScrollUp:
				setHighlight(kBorderNone);
				_mactext->scroll(-1);
				break;
			case kBorderScrollDown:
				setHighlight(kBorderNone);
				_mactext->scroll(1);
				break;
			default:
				return false;
			}

			return true;
		}

		return false;
	}

	if (click == kBorderInner) {
		// Call callback for processing any events
		if (_callback)
			(*_callback)(click, event, _dataPtr);

		if (!_selectable)
			return false;

		event.mouse.x -= _innerDims.left;
		event.mouse.y -= _innerDims.top;
		return _mactext->processEvent(event);
	}

	return MacWindow::processEvent(event);
}

void MacTextWindow::undrawInput() {
	for (uint i = 0; i < _inputTextHeight; i++)
		_mactext->removeLastLine();

	_inputTextHeight = 0;
}

void MacTextWindow::drawInput() {
	if (!_editable)
		return;

	int oldLen = _mactext->getLineCount() - _inputTextHeight;

	// add new input line to the text
	appendText(_inputText, _font, true);

	// Now recalc new text height
	int newLen = _mactext->getLineCount();
	_inputTextHeight = newLen - oldLen;

	_contentIsDirty = true;
}

void MacTextWindow::clearInput() {
	_inputText.clear();
}

void MacTextWindow::appendInput(const Common::U32String &str) {
	_inputText += str;

	drawInput();
}

void MacTextWindow::appendInput(const Common::String &str) {
	appendInput(Common::U32String(str));
}

} // End of namespace Graphics
