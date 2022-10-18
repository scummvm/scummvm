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

#include "graphics/primitives.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/macmenu.h"
#include "graphics/macgui/macbutton.h"
#include "graphics/macgui/macwidget.h"
#include "graphics/macgui/macwindow.h"

namespace Graphics {

MacButton::MacButton(MacButtonType buttonType, TextAlign textAlignment, MacWidget *parent, int x, int y, int w, int h, MacWindowManager *wm, const Common::U32String &s, const MacFont *macFont, int fgcolor, int bgcolor) :
	MacText(parent, x, y, w, h, wm, s, macFont, fgcolor, bgcolor, w, textAlignment), _pd(Graphics::MacPlotData(_composeSurface, nullptr, &_wm->getPatterns(), 1, 0, 0, 1, 0, true)) {

	_buttonType = buttonType;
	init();
}

void MacButton::init() {
	_invertInner = false;
	_checkBoxType = 0;
	_checkBoxAccess = 0;

	switch (_buttonType) {
	case kCheckBox:
		_dims.right += 16;
		break;
	case kRound:
		_dims.right += 4;
		_dims.bottom += 4;
		break;
	case kRadio:
		_dims.right += 16;
		break;
	}

	_composeSurface->create(_dims.width(), _dims.height(), _wm->_pixelformat);
	_composeSurface->clear(_bgcolor);
}

void MacButton::setActive(bool active) {
	if (active == _active)
		return;

	MacWidget::setActive(active);
	if (_composeSurface)
		_contentIsDirty = true;
}

// whether to use getDrawPixel or getDrawInvertPixel to draw invert pixel, maybe depends on the pattle we are using
void MacButton::invertOuter() {
	Common::Rect r(_dims.width() - 1, _dims.height() - 1);

	switch (_buttonType) {
	case kCheckBox: {
		Common::Rect c = Common::Rect(r.left + 1, r.top + 3, r.left + 9, r.top + 11);
		Graphics::drawRect1(c, 0, _wm->getDrawPixel(), &_pd);
	}
		break;
	case kRound:
		Graphics::drawRoundRect1(r, 4, 0, true, _wm->getDrawPixel(), &_pd);
		break;
	case kRadio:
		Graphics::drawEllipse(r.left + 1, r.top + 3, r.left + 10, r.top + 12, 0, false, _wm->getDrawPixel(), &_pd);
		break;
	}
}

void MacButton::setCheckBoxType(int type) {
	if (_checkBoxType == type)
		return;
	_checkBoxType = type;
	_contentIsDirty = true;
}

void MacButton::invertInner() {
	Common::Rect r(_dims.width() - 1, _dims.height() - 1);
	Common::Rect checkbox;

	switch (_buttonType) {
	case kCheckBox:
		switch (_checkBoxType) {
		case kCBNormal:
			Graphics::drawLine(r.left + 1, r.top + 3, r.left + 9, r.top + 11, 0, _wm->getDrawPixel(), &_pd);
			Graphics::drawLine(r.left + 1, r.top + 11, r.left + 9, r.top + 3, 0, _wm->getDrawPixel(), &_pd);
			(_wm->getDrawInvertPixel())(5, 7, 0, &_pd);
			break;
		case kCBInsetBlack:
			checkbox = Common::Rect(r.left + 2, r.top + 4, r.left + 2 + 6, r.top + 4 + 6);
			Graphics::drawFilledRect1(checkbox, 0, _wm->getDrawPixel(), &_pd);
			break;
		case kCBFilledBlack:
			checkbox = Common::Rect(r.left + 1, r.top + 3, r.left + 1 + 8, r.top + 3 + 8);
			Graphics::drawFilledRect1(checkbox, 0, _wm->getDrawPixel(), &_pd);
			break;
		}
		break;
	case kRound:
		break;
	case kRadio:
		Graphics::drawEllipse(r.left + 3, r.top + 5, r.left + 8, r.top + 10, 0, true, _wm->getDrawPixel(), &_pd);
		break;
	}
}

void MacButton::setHilite(bool hilite) {
	if (hilite == _invertInner)
		return;
	_invertInner = hilite;
	_contentIsDirty = true;
}

bool MacButton::draw(bool forceRedraw) {
	if (!_contentIsDirty && !forceRedraw)
		return false;

	MacText::draw();

	Common::Rect r(_dims.width() - 1, _dims.height() - 1);
	Graphics::MacPlotData pd(_composeSurface, nullptr, &_wm->getPatterns(), 1, 0, 0, 1, 0);

	switch (_buttonType) {
	case kCheckBox: {
		Common::Rect c = Common::Rect(r.left, r.top + 2, r.left + 10, r.top + 2 + 10);
		Graphics::drawRect1(c, 0, _wm->getDrawPixel(), &pd);
		break;
	}
	case kRound:
		Graphics::drawRoundRect1(r, 4, 0, false, _wm->getDrawPixel(), &pd);
		break;
	case kRadio:
		Graphics::drawEllipse(r.left, r.top + 2, r.left + 11, r.top + 13, 0, false, _wm->getDrawPixel(), &pd);
		break;
	}

	if (_active)
		invertOuter();
	if (_invertInner)
		invertInner();

	return true;
}

bool MacButton::draw(ManagedSurface *g, bool forceRedraw) {
	if (!MacButton::draw(forceRedraw))
		return false;

	g->transBlitFrom(*_composeSurface, _composeSurface->getBounds(), Common::Point(_dims.left, _dims.top), _wm->_colorGreen2);

	return true;
}

bool MacButton::processEvent(Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_MOUSEMOVE:
		if (_wm->_mouseDown) {
			if (_wm->_mode & kWMModeButtonDialogStyle)
				return true;
			else if (!_wm->_hilitingWidget)
				return true;
			// hovered widget in macwindow will help us set the button status to non-active.
			// so we only care about setting active here is ok.

			setActive(true);
		}
		break;
	case Common::EVENT_LBUTTONDOWN:
		setActive(true);
		_wm->_hilitingWidget = true;
		break;
	case Common::EVENT_LBUTTONUP:
		setActive(false);

		switch (_checkBoxAccess) {
		case 0:
			_invertInner = !_invertInner;
			break;
		case 1:
			_invertInner = true;
			break;
		case 2:
			// no op, type 2 will prevent user from setting checkboxes
			break;
		default:
			warning("MacButton::processEvent can not handle checkBoxAccess with type %d", _checkBoxAccess);
			break;
		}

		_wm->_hilitingWidget = false;
		break;
	default:
		warning("MacButton:: processEvent: Event not handled");
	}
	return false;
}

// we won't have the text with _border and _gutter in macbutton now.
// so for the override of calculateOffset, we are passing the border and gutter which depends on buttonType
// maybe we can cache this for optimization
Common::Point MacButton::calculateOffset() {
	int x = 0, y = 0;
	switch (_buttonType) {
	case kCheckBox:
		x = 16;
		break;
	case kRound:
		x = 2;
		y = 2;
		break;
	case kRadio:
		x = 16;
		break;
	}
	return Common::Point(x, y);
}

} // End of namespace Graphics
