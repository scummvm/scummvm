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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/timer.h"
#include "common/system.h"

#include "graphics/primitives.h"
#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macfontmanager.h"
#include "graphics/macgui/maceditabletext.h"
#include "graphics/macgui/macmenu.h"
#include "graphics/macgui/macbutton.h"
#include "graphics/macgui/macwidget.h"
#include "graphics/macgui/macwindow.h"

namespace Graphics {

MacButton::MacButton(MacButtonType buttonType, TextAlign textAlignment, MacWidget *parent, int x, int y, int w, int h, MacWindowManager *wm, const Common::U32String &s, const MacFont *macFont, int fgcolor, int bgcolor) :
	MacEditableText(parent, x, y, w, h, wm, s, macFont, fgcolor, bgcolor, w, textAlignment) {

	_buttonType = buttonType;
	delete _composeSurface;

	int offset;
	switch (buttonType) {
	case kCheckBox:
		offset = 16;
		break;
	case kRound:
		offset = 4;
		_dims.top -= 2;
		_dims.bottom += 2;
		_alignOffset.y += 2;
		break;
	case kRadio:
		offset = 16;
		break;
	}

	_alignOffset.x += offset;
	_dims.right += offset;
	_composeSurface = new ManagedSurface(_dims.width(), _dims.height());
}

void MacButton::setActive(bool active) {
	if (active == _active)
		return;

	MacWidget::setActive(active);
	if (_composeSurface)
		invertOuter();
}

void MacButton::invertOuter() {
	Common::Rect r(_dims.width() - 1, _dims.height() - 1);

	switch (_buttonType) {
	case kCheckBox: {
		Common::Rect c = Common::Rect(r.left + 1, r.top + 3, r.left + 9, r.top + 11);
		Graphics::drawRect(c, 0, Graphics::macInvertPixel, _composeSurface);
	}
		break;
	case kRound:
		Graphics::drawRoundRect(r, 4, 0, true, Graphics::macInvertPixel, _composeSurface);
		break;
	case kRadio:
		Graphics::drawEllipse(r.left + 1, r.top + 3, r.left + 10, r.top + 12, 0, false, Graphics::macInvertPixel, _composeSurface);
		break;
	}
}

void MacButton::invertInner() {
	Common::Rect r(_dims.width() - 1, _dims.height() - 1);

	switch (_buttonType) {
	case kCheckBox:
		Graphics::drawLine(r.left + 1, r.top + 3, r.left + 9, r.top + 11, 0, Graphics::macInvertPixel, _composeSurface);
		Graphics::drawLine(r.left + 1, r.top + 11, r.left + 9, r.top + 3, 0, Graphics::macInvertPixel, _composeSurface);
		Graphics::macInvertPixel(5, 7, 0, _composeSurface);
		break;
	case kRound:
		break;
	case kRadio:
		Graphics::drawEllipse(r.left + 3, r.top + 5, r.left + 8, r.top + 10, 0, true, Graphics::macInvertPixel, _composeSurface);
		break;
	}
}

bool MacButton::draw(bool forceRedraw) {
	if (!_contentIsDirty && !forceRedraw)
		return false;

	MacEditableText::draw();

	Common::Rect r(_dims.width() - 1, _dims.height() - 1);
	Graphics::MacPlotData pd(_composeSurface, nullptr, &_wm->getPatterns(), 1, 0, 0, 1, 0);

	switch (_buttonType) {
	case kCheckBox: {
		Common::Rect c = Common::Rect(r.left, r.top + 2, r.left + 10, r.top + 2 + 10);
		Graphics::drawRect(c, 0, Graphics::macDrawPixel, &pd);
		break;
	}
	case kRound:
		Graphics::drawRoundRect(r, 4, 0, _active, Graphics::macDrawPixel, &pd);
		break;
	case kRadio:
		Graphics::drawEllipse(r.left, r.top + 2, r.left + 11, r.top + 13, 0, false, Graphics::macDrawPixel, &pd);
		break;
	}

	_contentIsDirty = false;
	return true;
}

bool MacButton::draw(ManagedSurface *g, bool forceRedraw) {
	if (!MacButton::draw(forceRedraw))
		return false;

	g->transBlitFrom(*_composeSurface, _composeSurface->getBounds(), Common::Point(_dims.left - 2, _dims.top - 2), kColorGreen2);

	return true;
}

void MacButton::blit(ManagedSurface *g, Common::Rect &dest) {
	g->transBlitFrom(*_composeSurface, _composeSurface->getBounds(), dest, kColorGreen2);
}

bool MacButton::processEvent(Common::Event &event) {
	switch (event.type) {
	case Common::EVENT_MOUSEMOVE:
		break;
	case Common::EVENT_LBUTTONDOWN:
		setActive(true);
		break;
	case Common::EVENT_LBUTTONUP:
		setActive(false);
		invertInner();
		break;
	default:
		warning("MacButton:: processEvent: Event not handled");
	}
	return false;
}

} // End of namespace Graphics
