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

#include "common/system.h"

#include "graphics/macgui/macwindowmanager.h"
#include "graphics/macgui/macwidget.h"

namespace Graphics {

MacWidget::MacWidget(MacWidget *parent, int x, int y, int w, int h, MacWindowManager *wm, bool focusable, uint16 border, uint16 gutter, uint16 shadow, uint fgcolor, uint bgcolor) :
	_focusable(focusable), _parent(parent), _border(border), _gutter(gutter), _shadow(shadow), _wm(wm) {
	_contentIsDirty = true;
	_priority = 0;

	_dims.left = x;
	_dims.right = x + w + (2 * border) + (2 * gutter) + shadow;
	_dims.top = y;
	_dims.bottom = y + h + (2 * border) + gutter + shadow;

	_fgcolor = fgcolor;
	_bgcolor = bgcolor;

	if (parent)
		parent->_children.push_back(this);

	_composeSurface = new ManagedSurface(_dims.width(), _dims.height(), _wm->_pixelformat);
	_composeSurface->clear(_bgcolor);

	_active = false;
	_editable = false;
}

MacWidget::~MacWidget() {
	if (_parent)
		_parent->removeWidget(this, false);

	if (_wm)
		_wm->clearWidgetRefs(this);

	if (_composeSurface) {
		_composeSurface->free();
		delete _composeSurface;
	}
}

void MacWidget::setActive(bool active) {
	if (!_focusable)
		return;

	if (active == _active)
		return;

	_active = active;
}

bool MacWidget::draw(bool forceRedraw) {
	_contentIsDirty = false;

	return false;
}

bool MacWidget::draw(ManagedSurface *g, bool forceRedraw) {
	_contentIsDirty = false;

	return false;
}

void MacWidget::blit(ManagedSurface *g, Common::Rect &dest) {
	g->transBlitFrom(*_composeSurface, _composeSurface->getBounds(), dest, _wm->_colorGreen2);
}

void MacWidget::setColors(uint32 fg, uint32 bg) {
	_fgcolor = fg;
	_bgcolor = bg;

	_contentIsDirty = true;
}

bool MacWidget::processEvent(Common::Event &event) {
	return false;
}

void MacWidget::removeWidget(MacWidget *child, bool del) {
	if (_children.size() == 0)
		return;

	for (uint i = 0; i < _children.size(); i++) {
		if (_children[i] == child) {
			if (del)
				delete _children[i];

			_children.remove_at(i);
		}
	}
}

MacWidget *MacWidget::findEventHandler(Common::Event &event, int dx, int dy) {
	switch (event.type) {
	case Common::EVENT_LBUTTONDOWN:
	case Common::EVENT_LBUTTONUP:
	case Common::EVENT_RBUTTONDOWN:
	case Common::EVENT_RBUTTONUP:
	case Common::EVENT_MOUSEMOVE:
		{
			Common::Point pos;

			pos = g_system->getEventManager()->getMousePos();

			if (_dims.contains(pos.x - dx, pos.y - dy)) {
				uint priority = 0;
				MacWidget *widget = nullptr;

				for (uint i = 0; i < _children.size(); i++) {
					MacWidget *res = _children[i]->findEventHandler(event, dx + _dims.left, dy + _dims.top);
					if (res && res->_priority > priority) {
						priority = res->_priority;
						widget = res;
					}
				}
				return widget ? widget : this;
			}
			break;
		}

	case Common::EVENT_KEYDOWN:
		break;

	default:
		return nullptr;
	}

	return nullptr;
}

Common::Point MacWidget::getAbsolutePos() {
	if (!_parent)
		return Common::Point(0, 0);

	return Common::Point(_parent->_dims.left, _parent->_dims.top) + _parent->getAbsolutePos();
}

} // End of namespace Graphics
