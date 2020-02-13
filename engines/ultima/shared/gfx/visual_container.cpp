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

#include "ultima/shared/gfx/visual_container.h"

namespace Ultima {
namespace Shared {
namespace Gfx {

BEGIN_MESSAGE_MAP(VisualContainer, VisualItem)
	ON_MESSAGE(MouseButtonDownMsg)
	ON_MESSAGE(MouseButtonUpMsg)
	ON_MESSAGE(MouseMoveMsg)
	ON_MESSAGE(MouseDoubleClickMsg)
	ON_MESSAGE(MouseWheelMsg)
END_MESSAGE_MAP()


bool VisualContainer::MouseButtonDownMsg(CMouseButtonDownMsg *msg) {
	if (msg->_buttons & MB_LEFT) {
		_mouseFocusItem = handleMouseMsg(msg);
		return _mouseFocusItem != nullptr;
	}

	return false;
}

bool VisualContainer::MouseButtonUpMsg(CMouseButtonUpMsg *msg) {
	if (msg->_buttons & MB_LEFT) {
		bool result = handleMouseMsg(msg) != nullptr;
		_mouseFocusItem = nullptr;
		return result;
	}

	return false;
}

bool VisualContainer::MouseDoubleClickMsg(CMouseDoubleClickMsg *msg) {
	if (msg->_buttons & MB_LEFT)
		return handleMouseMsg(msg);

	return false;
}

bool VisualContainer::MouseWheelMsg(CMouseWheelMsg *msg) {
	return handleMouseMsg(msg);
}

bool VisualContainer::MouseMoveMsg(CMouseMoveMsg *msg) {
	if (msg->_buttons & MB_LEFT)
		return handleMouseMsg(msg);

	return false;
}

bool VisualContainer::MouseDragMsg(CMouseDragMsg *msg) {
	return handleMouseMsg(msg);
}

VisualItem *VisualContainer::handleMouseMsg(CMouseMsg *msg) {
	if (_mouseFocusItem) {
		// An item currently has focus, so pass all events directly to
		// it, irrespective of whether the mouse is still in it or not
		msg->execute(_mouseFocusItem);
		return _mouseFocusItem;

	} else {
		// Iterate through each child and pass the message to the first
		// immediate child the mouse position falls within
		for (TreeItem *child = getFirstChild(); child; child = child->getNextSibling()) {
			VisualItem *item = dynamic_cast<VisualItem *>(child);
			if (item && item->getBounds().contains(msg->_mousePos)) {
				if (msg->execute(item))
					return item;
			}
		}

		return nullptr;
	}
}

void VisualContainer::draw() {
	// If the overall container is dirty, clear it
	if (_isDirty)
		getSurface().clear();

	// Iterate through each child and draw any dirty visual items
	for (TreeItem *child = getFirstChild(); child; child = child->getNextSibling()) {
		VisualItem *item = dynamic_cast<VisualItem *>(child);
		if (item && item->isDirty())
			item->draw();
	}

	_isDirty = false;
}

void VisualContainer::setDirty(bool dirty) {
	for (TreeItem *child = getFirstChild(); child; child = child->getNextSibling()) {
		VisualItem *item = dynamic_cast<VisualItem *>(child);
		if (item)
			item->setDirty(dirty);
	}

	VisualItem::setDirty(dirty);
}

bool VisualContainer::isDirty() const {
	if (_isDirty)
		return true;

	for (TreeItem *child = getFirstChild(); child; child = child->getNextSibling()) {
		VisualItem *item = dynamic_cast<VisualItem *>(child);
		if (item && item->isDirty())
			return true;
	}

	return false;
}

} // End of namespace Gfx
} // End of namespace Shared
} // End of namespace Ultima
