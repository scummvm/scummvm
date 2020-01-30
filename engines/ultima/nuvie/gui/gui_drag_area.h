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

#ifndef NUVIE_GUI_GUI_DRAG_AREA_H
#define NUVIE_GUI_GUI_DRAG_AREA_H

#include "ultima/nuvie/gui/gui_callback.h"

namespace Ultima {
namespace Nuvie {

class GUI_DragArea : public GUI_CallBack {

protected:
	bool drop_target;
	bool dragging;

public:

	GUI_DragArea() {
		dragging = false;
		drop_target = true;
	}

	bool is_drop_target() {
		return drop_target;
	}

	virtual bool drag_accept_drop(int x, int y, int message, void *data) {
		return false;
	}
	virtual void drag_perform_drop(int x, int y, int message, void *data) {
		return;
	}
	virtual void drag_drop_failed(int x, int y, int message, void *data) {
		return;
	}
	virtual void drag_drop_success(int x, int y, int message, void *data) {
		return;
	}
	virtual void drag_draw(int x, int y, int message, void *data) {
		return;
	}

};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
