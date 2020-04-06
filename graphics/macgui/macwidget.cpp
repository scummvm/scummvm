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

#include "graphics/macgui/macwidget.h"

namespace Graphics {

MacWidget::MacWidget(MacWidget *parent, int x, int y, int w, int h, bool focusable) :
		_focusable(focusable), _parent(parent) {
	_contentIsDirty = true;

	_dims.left = x;
	_dims.right = x + w;
	_dims.top = y;
	_dims.bottom = y + h;

	if (parent)
		parent->_children.push_back(this);
}

MacWidget::~MacWidget() {
	if (_parent)
		_parent->removeWidget(this, false);
}

void MacWidget::removeWidget(MacWidget *child, bool del) {
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
				for (uint i = 0; i < _children.size(); i++) {
					MacWidget *res = _children[i]->findEventHandler(event, dx + _dims.left, dy + _dims.top);
					if (res)
						return res;
				}
				return this;
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
