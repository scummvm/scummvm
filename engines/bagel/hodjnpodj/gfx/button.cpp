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
#include "bagel/metaengine.h"

namespace Bagel {
namespace HodjNPodj {

bool Button::msgMouseDown(const MouseDownMessage &msg) {
	if (!(_itemState & ODS_GRAYED) &&
		!(_itemState & ODS_DISABLED)) {
		if (msg._button == MouseDownMessage::MB_LEFT) {
			_itemState |= ODS_SELECTED;
			redraw();
		}
	}

	return true;
}

bool Button::msgMouseUp(const MouseUpMessage &msg) {
	if (!(_itemState & ODS_GRAYED) &&
		!(_itemState & ODS_DISABLED)) {
		if (msg._button == MouseUpMessage::MB_LEFT && (_itemState & ODS_SELECTED)) {
			_itemState &= ~ODS_SELECTED;
			redraw();

			// Notify to owner that button was pressed
			buttonPressed();
		}
	}

	return true;
}

bool Button::msgMouseEnter(const MouseEnterMessage &msg) {
	if (!(_itemState & ODS_GRAYED) &&
		!(_itemState & ODS_DISABLED)) {
		_itemState |= ODS_FOCUS;
		redraw();
	}

	return true;
}

bool Button::msgMouseLeave(const MouseLeaveMessage &msg) {
	if (!(_itemState & ODS_GRAYED) &&
		!(_itemState & ODS_DISABLED)) {
		_itemState &= ~ODS_FOCUS;
		redraw();
	}

	return true;
}

bool Button::msgKeypress(const KeypressMessage &msg) {
	if (!(_itemState & ODS_GRAYED) &&
		!(_itemState & ODS_DISABLED)) {
		size_t ampPos = _text.findFirstOf('&');

		if (ampPos != Common::String::npos &&
			(msg.flags & Common::KBD_ALT) &&
			(msg.ascii == tolower(_text[ampPos + 1]))) {
			// Notify parent dialog that the button was pressed
			_itemState &= ~ODS_SELECTED;
			buttonPressed();
			return true;
		}
	}

	return false;
}

void Button::buttonPressed() {
	_parent->send(GameMessage("BUTTON", _name));
}

void Button::setText(const Common::String &text) {
	_text = text;
	redraw();
}

/*------------------------------------------------------------------------*/

void BmpButton::loadBitmaps(const char *base, const char *selected,
		const char *focused, const char *disabled) {
	_base.loadBitmap(base);
	_selected.loadBitmap(selected);
	if (focused)
		_focused.loadBitmap(focused);
	if (disabled)
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

	if (((_itemState & ODS_DISABLED) ||
		(_itemState & ODS_GRAYED)) && !_disabled.empty())
		s.blitFrom(_disabled);
	else if (_itemState & ODS_SELECTED)
		s.blitFrom(_selected);
	else if ((_itemState & ODS_FOCUS) && !_focused.empty())
		s.blitFrom(_focused);
	else
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

/*------------------------------------------------------------------------*/

void CheckButton::draw() {
	GfxSurface s = getSurface();
	Common::Rect checkRect = getCheckRect();
	Common::String text;
	uint nUnderscore;

	s.clear(_cButtonFace);
	s.frameRect(Common::Rect(0, 0, s.w, s.h), _cButtonOutline);
	s.frameRect(checkRect, _cButtonControl);

	if (_checked) {
		// Draw x shape
		s.drawLine(checkRect.left, checkRect.top,
			checkRect.right - 1, checkRect.bottom - 1, _cButtonControl);
		s.drawLine(checkRect.left, checkRect.bottom - 1,
			checkRect.right - 1, checkRect.top, _cButtonControl);
	}

	// Check for any ampersand character, which indicates underline
	// for the keyboard key associated with the button
	text = _text;
	if ((nUnderscore = text.findFirstOf('&')) != Common::String::npos)
		text.deleteChar(nUnderscore);

	uint color = !(_itemState & ODS_DISABLED) ?
		_cButtonText : _cButtonTextDisabled;
	int x = 20, y = 2;

	s.setFontSize(8);
	s.writeString(text, Common::Point(x, y), color);

	if (nUnderscore != Common::String::npos) {
		Common::String str;
		while (nUnderscore-- > 0)
			str += ' ';
		str += '_';
		s.writeString(str, Common::Point(x + 1, y + 1), color);
	}
}

Common::Rect CheckButton::getCheckRect() const {
	int checkSize = _bounds.height() - 6;
	return Common::Rect(3, 3, 3 + checkSize, 3 + checkSize);
}

void CheckButton::buttonPressed() {
	// For checkboxes, if this is called due to
	// a hotkey match, trigger toggling the checkbox
	setCheck(!_checked);
}

bool CheckButton::msgMouseUp(const MouseUpMessage &msg) {
	if (!(_itemState & ODS_GRAYED) &&
		!(_itemState & ODS_DISABLED)) {
		setCheck(!_checked);
	}

	return true;
}

void CheckButton::setCheck(bool checked) {
	_checked = checked;
	redraw();
	_parent->send(GameMessage("CHECKBOX", _name,
		_checked ? 1 : 0));
}

/*------------------------------------------------------------------------*/

OkButton::OkButton(UIElement *parent) :
		ColorButton("OK", _s("&OK"), parent) {
}

OkButton::OkButton(const Common::Rect &r, UIElement *parent) :
		ColorButton("OK", _s("&OK"), parent) {
	setBounds(r);
}

bool OkButton::msgMouseEnter(const MouseEnterMessage &msg) {
	g_events->setCursor(IDC_ARROW);
	return ColorButton::msgMouseEnter(msg);
}

bool OkButton::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_SELECT) {
		_parent->send(GameMessage("BUTTON", _name));
		return true;
	}
	return false;
}

/*------------------------------------------------------------------------*/

CancelButton::CancelButton(UIElement *parent) :
		ColorButton("CANCEL", _s("Cancel"), parent) {
}

CancelButton::CancelButton(const Common::Rect &r, UIElement *parent) :
		ColorButton("CANCEL", _s("Cancel"), parent) {
	setBounds(r);
}

bool CancelButton::msgAction(const ActionMessage &msg) {
	if (msg._action == KEYBIND_ESCAPE) {
		_parent->send(GameMessage("BUTTON", _name));
		return true;
	}
	return false;
}

/*------------------------------------------------------------------------*/

DefaultsButton::DefaultsButton(UIElement *parent) :
		ColorButton("DEFAULTS", _s("Defaults"), parent) {
}

DefaultsButton::DefaultsButton(const Common::Rect &r, UIElement *parent) :
		ColorButton("DEFAULTS", _s("Defaults"), parent) {
	setBounds(r);
}

} // namespace HodjNPodj
} // namespace Bagel
