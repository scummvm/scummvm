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
#include "common/translation.h"
#include "graphics/paletteman.h"
#include "common/file.h"
#include "image/bmp.h"
#include "bagel/hodjnpodj/gfx/button.h"

namespace Bagel {
namespace HodjNPodj {

void BmpButton::loadBitmaps(const char *base, const char *selected,
		const char *focused, const char *disabled) {
	_base.loadBitmap(base);
	_selected.loadBitmap(selected);
	_focused.loadBitmap(focused);
	_disabled.loadBitmap(disabled);
}

void BmpButton::clear() {
	_base.clear();
	_selected.clear();
	_focused.clear();
	_disabled.clear();
}

void BmpButton::draw() {
	GfxSurface s = getSurface();
	s.blitFrom(_base);
}

/*------------------------------------------------------------------------*/

void ColorButton::draw() {
	GfxSurface s = getSurface();
	COLORREF myTextColor, myPen, myInversePen, myBrush;
	Common::String text;
	int i, x, y, w, h, fw, fh;
	Common::Point textInfo, letterInfo,
		underscoreInfo;
	Common::Rect focusRect;
	size_t nUnderscore;
	byte color;

	// Check for any ampersand character, which indicates underline
	// for the keyboard key associated with the button
	text = _text;
	if ((nUnderscore = text.findFirstOf('&')) != Common::String::npos)
		text.deleteChar(nUnderscore);

	x = 0;
	y = 0;

	if ((_itemState & ODS_GRAYED) ||
		(_itemState & ODS_DISABLED)) {
		myPen = _cButtonHighlight;
		myInversePen = _cButtonShadow;
		myBrush = _cButtonFace;
		myTextColor = _cButtonTextDisabled;
	} else if (_itemState & ODS_SELECTED) {
		myPen = _cButtonShadow;
		myInversePen = _cButtonHighlight;
		myBrush = _cButtonFace;
		myTextColor = _cButtonText;
		x = SELECTED_TEXT_DX;
		y = SELECTED_TEXT_DY;
	} else {
		myPen = _cButtonHighlight;
		myInversePen = _cButtonShadow;
		myBrush = _cButtonFace;
		myTextColor = _cButtonText;
	}

	color = getPaletteIndex(myPen);
	Common::Rect area(0, 0, _bounds.width() - 1, _bounds.height() - 1);
	s.fillRect(area, color);

	// Draw the button edges
	color = getPaletteIndex(myInversePen);
	for (i = 1; i <= BUTTON_EDGE_WIDTH; i++) {              
		s.hLine(i, _bounds.height() - i, _bounds.width() - i, color);
		s.vLine(_bounds.width() - i, i - 1, _bounds.height() - i, color);
	}

	color = getPaletteIndex(_cButtonOutline);
	s.frameRect(Common::Rect(0, 0, _bounds.width(), _bounds.height()), color);

	s.setFontSize(8);
	w = _bounds.width();
	h = _bounds.height();
	fw = s.getStringWidth(text);
	fh = s.getStringHeight();
	x += (w - fw) / 2;
	y += (h - fh) / 2;

	color = getPaletteIndex(myTextColor);
	s.writeString(text, Common::Point(x, y), color);

	if (nUnderscore != Common::String::npos) {
		Common::String str;
		while (nUnderscore-- > 0)
			str += ' ';
		str += '_';
		s.writeString(str, Common::Point(x + 1, y + 1), color);
	}

	if (_itemState & ODS_FOCUS) {
		focusRect = Common::Rect(x - FOCUS_RECT_DX,
			y - FOCUS_RECT_DY,
			x + fw + FOCUS_RECT_DX,
			y + fh + FOCUS_RECT_DY + 1);
		s.frameRect(focusRect, 3);	// Dark grey
	}
}

bool ColorButton::msgMouseDown(const MouseDownMessage &msg) {
	if (msg._button == MouseDownMessage::MB_LEFT) {
		_itemState = ODS_SELECTED;
		redraw();
	}

	return true;
}

bool ColorButton::msgMouseUp(const MouseUpMessage &msg) {
	if (msg._button == MouseUpMessage::MB_LEFT && _itemState == ODS_SELECTED) {
		_itemState = 0;
		redraw();

		// Notify parent dialog that the button was pressed
		_parent->send(GameMessage("BUTTON"));
	}

	return true;
}

bool ColorButton::msgUnfocus(const UnfocusMessage &msg) {
	// If the mouse cursor moves outside the button whilst
	// it's being depressed, reset it to unpressed
	if (_itemState == ODS_SELECTED) {
		_itemState = 0;
		redraw();
	}

	return true;
}

bool ColorButton::msgKeypress(const KeypressMessage &msg) {
	size_t ampPos = _text.findFirstOf('&');

	if (ampPos != Common::String::npos &&
			(msg.flags & Common::KBD_ALT) &&
			(msg.ascii == tolower(_text[ampPos + 1]))) {
		// Notify parent dialog that the button was pressed
		_parent->send(GameMessage("BUTTON"));
		return true;
	}

	return false;
}

/*------------------------------------------------------------------------*/

OkButton::OkButton(UIElement *parent) :
		ColorButton(_s("&OK"), parent) {
}

} // namespace HodjNPodj
} // namespace Bagel
