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

#include "m4/riddle/gui/inventory.h"
#include "m4/riddle/gui/inventory.h"
#include "m4/riddle/vars.h"
#include "m4/core/errors.h"
#include "m4/graphics/gr_line.h"
#include "m4/graphics/gr_series.h"
#include "m4/gui/gui_vmng_core.h"
#include "m4/gui/gui_vmng_screen.h"

namespace M4 {
namespace Riddle {
namespace GUI {

Inventory::Inventory(const RectClass &r, int32 sprite, int16 cells_h, int16 cells_v, int16 cell_w, int16 cell_h, int16 tag)
	: RectClass(r) {
	_sprite = sprite;

	for (int16 iter = 0; iter < INVENTORY_CELLS_COUNT; iter++) {
		_items[iter]._cell = -1;
		_items[iter]._cursor = -1;
	}

	_num_cells = 0;
	_tag = tag;
	_cells_h = cells_h;
	_cells_v = cells_v;
	_cell_w = cell_w;
	_cell_h = cell_h;

	// If requested cell configuration doesn't fit, blow up.
	if ((cells_h * cell_w > (_x2 - _x1)) || (cells_v * cell_h > (_y2 - _y1))) {
		error_show(FL, 'CGIC');
	}

	_highlight = -1;
	_must_redraw_all = true;
	_must_redraw1 = -1;
	_must_redraw2 = -1;
	_scroll = 0;
	_right_arrow_visible = false;

	_btnScrollLeft = new ButtonClass(RectClass(178, -8, 198, 101), "scroll left", 9, 129, 130, 131, INTERFACE_SPRITES);
	_btnScrollRight = new ButtonClass(RectClass(551, -8, 571, 101), "scroll right", 9, 133, 134, 135, INTERFACE_SPRITES);
	refresh_left_arrow();
	refresh_right_arrow();
}

Inventory::~Inventory() {
	delete _btnScrollLeft;
	delete _btnScrollRight;
}

void Inventory::addToInterfaceBox(InterfaceBox *box) {
	box->add(_btnScrollLeft);
	box->add(_btnScrollRight);
}

bool Inventory::add(const Common::String &name, const Common::String &verb, int32 invSprite, int32 cursor) {
	// Don't add something twice
	int iter;
	for (iter = 0; iter < _num_cells; iter++) {
		if (name.equals(_items[iter]._name))
			return true;
	}

	if (_num_cells >= INVENTORY_CELLS_COUNT) {
		error_show(FL, 'CGIA');
		return false;
	}

	// Shift existing items up by one
	for (int i = _num_cells; i > 0; --i)
		_items[i] = _items[i - 1];

	auto &item = _items[0];
	item._name = name;
	item._verb = verb;
	item._cell = invSprite;
	item._cursor = cursor;
	++_num_cells;

	_must_redraw_all = true;

	if (INTERFACE_VISIBLE)
		_G(interface).show();

	return true;
}

bool Inventory::need_left() const {
	return (_scroll != 0);
}

bool Inventory::need_right() const {
	if ((_num_cells - _scroll - MAX_INVENTORY) > 0)
		return true;

	return false;
}

void Inventory::set_scroll(int32 new_scroll) {
	_scroll = new_scroll;
	_must_redraw_all = true;
}

void Inventory::toggleHidden() {
	_hidden = !_hidden;
	_must_redraw_all = true;
}

bool Inventory::remove(const Common::String &name) {
	int iter;
	for (iter = 0; iter < _num_cells; iter++) {
		// Found the thing?
		if (name.equals(_items[iter]._name)) {
			// Eat up its slot by moving everything down
			for (; iter < _num_cells; ++iter)
				_items[iter] = _items[iter + 1];

			--_num_cells;
			_must_redraw_all = true;
			_scroll = 0;

			if (INTERFACE_VISIBLE)
				_G(interface).show();

			return true;
		}
	}

	// Didn't find that thing.
	return false;
}

int16 Inventory::inside(int16 x, int16 y) const {
	if ((x < _x1) || (x >= (_x2 - 1)) || (y < _y1 + 2) ||
		(y > _y1 + _cells_v * _cell_h - 2))
		return -1;

	x -= _x1;
	y -= _y1;
	return (int16)((x / _cell_w) * _cells_v + (y / _cell_h));
}

int16 Inventory::cell_pos_x(int16 index) {
	if (_cells_h > _cells_v) {				// Horizontal orientation, fill left to right
		return (int16)((index / _cells_v) * _cell_w);
	} else {								// Vertical orientation, fill top to bottom
		return (int16)((index / _cells_h) * _cell_w);
	}
}

int16 Inventory::cell_pos_y(int16 index) {
	if (_cells_h > _cells_v) {
		// Horizontal orientation, fill left to right
		return (int16)((index % _cells_v) * _cell_h);
	} else {
		// Vertical orientation, fill top to bottom
		return (int16)((index % _cells_h) * _cell_h);
	}
}

void Inventory::highlight_part(int16 index) {
	if (_highlight == index)
		return;

	_must_redraw1 = _highlight;
	_highlight = index;
	_must_redraw2 = _highlight;
}

void Inventory::draw(GrBuff *myBuffer) {
	if (!_must_redraw1 && !_must_redraw2 && !_must_redraw_all)
		return;

	int cell_iter;

	Buffer *myBuff = myBuffer->get_buffer();

	if (_must_redraw_all || _hidden) {
		gr_color_set(__BLACK);
		gr_buffer_rect_fill(myBuff, _x1, _y1, _x2 - _x1, _y2 - _y1);
	}

	if (!_hidden) {
		_right_arrow_visible = false;
		const int X_BORDER = 2, Y_BORDER = 2;

		for (cell_iter = 0; (cell_iter + _scroll < _num_cells) && (cell_iter < MAX_INVENTORY); cell_iter++) {
			int16 left =_x1 + X_BORDER + cell_pos_x(cell_iter);
			int16 top = _y1 + Y_BORDER + cell_pos_y(cell_iter);
			int16 leftOffset = left + _cell_w;
			int16 topOffset = top + _cell_h;

			if (_must_redraw1 == cell_iter || _must_redraw2 == cell_iter || _must_redraw_all) {
				// Update the scroll buttons
				refresh_right_arrow();
				refresh_left_arrow();

				// Draw icon here
				gr_color_set(__BLACK);
				gr_buffer_rect_fill(myBuff, left, top, leftOffset - left, topOffset - top);
				series_show_frame(_sprite, _items[cell_iter + _scroll]._cell,
					myBuff, left - 3, top - 3);

				// Draw box around icon
				if (_highlight == cell_iter) {
					gr_line(left, top, left + _cell_w - 2, top, __LTGRAY, myBuff);
					gr_line(left, top + _cell_h - 2, left + _cell_w - 2, top + _cell_h - 2, __LTGRAY, myBuff);
					gr_line(left, top, left, top + _cell_w - 2, __LTGRAY, myBuff);
					gr_line(left + _cell_w - 2, top, left + _cell_w - 2, top + _cell_h - 2, __LTGRAY, myBuff);
				}
			}
		}
	}

	ScreenContext *iC = vmng_screen_find(_G(gameInterfaceBuff), nullptr);
	RestoreScreensInContext(_x1, _y1, _x2, _y2, iC);
	_must_redraw1 = _must_redraw2 = -1;
	_must_redraw_all = false;

	myBuffer->release();
}

ControlStatus Inventory::track(int32 eventType, int16 x, int16 y) {
	if (!INTERFACE_VISIBLE)
		return NOTHING;

	ControlStatus result = NOTHING;

	int16 over = inside(x, y);
	bool button_clicked = eventType == _ME_L_click || eventType == _ME_L_hold || eventType == _ME_L_drag;

	// If Button is pressed
	if (button_clicked) {
		// If we are not tracking, start tracking
		if (interface_tracking == -1) {
			highlight_part(over);
			interface_tracking = over;
			result = IN_CONTROL;
		} else {
			// Else if we are over something we are tracking
			if (interface_tracking == over) {
				highlight_part(over);
				result = IN_CONTROL;
			} else {
				// Else highlight nothing
				highlight_part(-1);
				result = NOTHING;
			}
		}
	} else {
		// If Button isn't pressed

		// If we unpressed on something we were tracking
		if (interface_tracking == over) {
			if (interface_tracking == -1)
				result = NOTHING;
			else
				result = SELECTED;
		} else {
			if (over + _scroll < _num_cells)
				result = OVER_CONTROL;
			else
				result = NOTHING;
		}

		// Stop tracking anything
		highlight_part(over);
		interface_tracking = -1;
	}

	if (result == NOTHING && button_clicked)
		return TRACKING;

	return result;
}


void Inventory::refresh_right_arrow() {
	if (need_right() || need_left()) {
		_btnScrollRight->unhide();

		if (need_right()) {
			_btnScrollRight->set_sprite_relaxed(133);
			_btnScrollRight->set_sprite_picked(135);
			_btnScrollRight->set_sprite_over(134);
		} else {
			_btnScrollRight->set_sprite_relaxed(136);
			_btnScrollRight->set_sprite_picked(136);
			_btnScrollRight->set_sprite_over(136);
		}
	} else {
		_btnScrollRight->hide();
		_btnScrollLeft->hide();
	}
}

void Inventory::refresh_left_arrow() {
	if (need_right() || need_left()) {
		_btnScrollLeft->unhide();

		if (need_left()) {
			_btnScrollLeft->set_sprite_relaxed(129);
			_btnScrollLeft->set_sprite_picked(131);
			_btnScrollLeft->set_sprite_over(130);
		} else {
			_btnScrollLeft->set_sprite_relaxed(132);
			_btnScrollLeft->set_sprite_picked(132);
			_btnScrollLeft->set_sprite_over(132);
		}
	} else {
		_btnScrollRight->hide();
		_btnScrollLeft->hide();
	}
}

void Inventory::refresh_scrollbars() {
	if (_btnScrollRight->is_hidden())
		refresh_right_arrow();
	else
		_btnScrollRight->hide();

	if (_btnScrollLeft->is_hidden())
		refresh_left_arrow();
	else
		_btnScrollLeft->hide();
}

void Inventory::check_left() {
	if (!_btnScrollLeft->is_hidden()) {
		if (need_left()) {
			_scroll = (_scroll <= 0) ? 0 : _scroll - _cells_v;
		}

		refresh_right_arrow();
		refresh_left_arrow();
		_must_redraw_all = true;
	}
}

void Inventory::check_right() {
	if (!_btnScrollRight->is_hidden()) {
		if (need_right())
			_scroll += _cells_v;

		refresh_right_arrow();
		refresh_left_arrow();
		_must_redraw_all = true;
	}
}

} // namespace GUI
} // namespace Riddle
} // namespace M4
