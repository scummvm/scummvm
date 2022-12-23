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

#include "ultima/ultima8/gumps/resizable_gump.h"
#include "ultima/ultima8/kernel/mouse.h"

namespace Ultima {
namespace Ultima8 {

#define RESIZE_BORDER 5

ResizableGump::ResizableGump(int x, int y, int width, int height)
	: Gump(x, y, width, height, 0, FLAG_DRAGGABLE, LAYER_NORMAL),
	_dragPosition(Gump::CENTER), _mousePosition(Gump::CENTER), _minWidth(20), _minHeight(20) {
}

ResizableGump::ResizableGump() : Gump(),
	_dragPosition(Gump::CENTER), _mousePosition(Gump::CENTER), _minWidth(20), _minHeight(20) {
}

ResizableGump::~ResizableGump() {
}


Gump *ResizableGump::onMouseMotion(int32 mx, int32 my) {
	_mousePosition = getPosition(mx, my);
	return Gump::onMouseMotion(mx, my);
}

void ResizableGump::onMouseLeft() {
	_mousePosition = Gump::CENTER;
}

bool ResizableGump::onDragStart(int32 mx, int32 my) {
	if (Gump::onDragStart(mx, my)) {
		_dragPosition = getPosition(mx, my);
		return true;
	}
	return false;
}

void ResizableGump::onDragStop(int32 mx, int32 my) {
	_dragPosition = Gump::CENTER;
}

void ResizableGump::onDrag(int32 mx, int32 my) {
	int32 x = _x;
	int32 y = _y;
	int32 w = _dims.width();
	int32 h = _dims.height();

	int32 dx, dy;
	Mouse::get_instance()->getDraggingOffset(dx, dy);

	int32 px = mx, py = my;
	ParentToGump(px, py);

	switch (_dragPosition) {
	case Gump::CENTER:
		x = mx - dx;
		y = my - dy;
		break;
	case Gump::TOP_LEFT:
		w -= px - dx;
		h -= py - dy;
		x = mx - dx;
		y = my - dy;
		break;
	case Gump::TOP_RIGHT:
		w = px;
		h -= py - dy;
		y = my - dy;
		break;
	case Gump::BOTTOM_LEFT:
		w -= px - dx;
		h = py;
		x = mx - dx;
		break;
	case Gump::BOTTOM_RIGHT:
		w = px;
		h = py;
		break;
	case Gump::TOP_CENTER:
		h -= py - dy;
		y = my - dy;
		break;
	case Gump::BOTTOM_CENTER:
		h = py;
		break;
	case Gump::LEFT_CENTER:
		w -= px - dx;
		x = mx - dx;
		break;
	case Gump::RIGHT_CENTER:
		w = px;
		break;
	default:
		break;
	}

	if (w >= _minWidth) {
		_dims.setWidth(w);
		_x = x;
	}

	if (h >= _minHeight) {
		_dims.setHeight(h);
		_y = y;
	}
}

Gump::Position ResizableGump::getPosition(int32 mx, int32 my) {
	Gump::Position position = Gump::CENTER;
	ParentToGump(mx, my);
	if (mx < _dims.left + RESIZE_BORDER && my < _dims.top + RESIZE_BORDER) {
		position = Gump::TOP_LEFT;
	} else if (mx >= _dims.right - RESIZE_BORDER && my < _dims.top + RESIZE_BORDER) {
		position = Gump::TOP_RIGHT;
	} else if (mx < _dims.left + RESIZE_BORDER && my >= _dims.bottom - RESIZE_BORDER) {
		position = Gump::BOTTOM_LEFT;
	} else if (mx >= _dims.right - RESIZE_BORDER && my >= _dims.bottom - RESIZE_BORDER) {
		position = Gump::BOTTOM_RIGHT;
	} else if (my < _dims.top + RESIZE_BORDER) {
		position = Gump::TOP_CENTER;
	} else if (my >= _dims.bottom - RESIZE_BORDER) {
		position = Gump::BOTTOM_CENTER;
	} else if (mx < _dims.left + RESIZE_BORDER) {
		position = Gump::LEFT_CENTER;
	} else if (mx >= _dims.right - RESIZE_BORDER) {
		position = Gump::RIGHT_CENTER;
	}
	return position;
}

} // End of namespace Ultima8
} // End of namespace Ultima
