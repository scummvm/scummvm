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

#include "m4/burger/gui_cheapo.h"
#include "m4/gui/gui_vmng_core.h"
#include "m4/gui/gui_vmng_screen.h"
#include "m4/graphics/gr_font.h"
#include "m4/graphics/gr_line.h"
#include "m4/graphics/gr_pal.h"
#include "m4/graphics/gr_series.h"
#include "m4/adv_r/adv_trigger.h"
#include "m4/core/cstring.h"
#include "m4/core/errors.h"
#include "m4/mem/mem.h"
#include "m4/mem/memman.h"
#include "m4/globals.h"

namespace M4 {
namespace Burger {

#define _GL(X) _G(interface).X

static void refresh_right_arrow() {
	error("TODO: refresh_right_arrow");
}

static void refresh_left_arrow() {
	error("TODO: refresh_left_arrow");
}

RectClass::RectClass() {
}

RectClass::RectClass(RectClass *r) {
	if (!r)
		error_show(FL, 'CGNR');

	_x1 = r->_x1;
	_y1 = r->_y1;
	_x2 = r->_x2;
	_y2 = r->_y2;
}

RectClass::RectClass(int16 x1, int16 y1, int16 x2, int16 y2) {
	_x1 = x1;
	_y1 = y1;
	_x2 = x2;
	_y2 = y2;
}

RectClass::~RectClass() {
}

void RectClass::copyInto(RectClass *r) {
	if (!r) {
		error_show(FL, 'CGNR');
	} else {
		r->_x1 = _x1;
		r->_y1 = _y1;
		r->_x2 = _x2;
		r->_y2 = _y2;
	}
}

void RectClass::set(int16 x1, int16 y1, int16 x2, int16 y2) {
	_x1 = x1;
	_y1 = y1;
	_x2 = x2;
	_y2 = y2;
}

void RectClass::set(RectClass *r) {
	if (!r) {
		error_show(FL, 'CGNR');
	} else {
		_x1 = r->_x1;
		_y1 = r->_y1;
		_x2 = r->_x2;
		_y2 = r->_y2;
	}
}

int16 RectClass::inside(int16 x, int16 y) const {
	if ((x >= _x1) && (x <= _x2) && (y >= _y1) && (y <= _y2))
		return 1;
	return 0;
}

//-------------------------------------------------------------------------------------------

TextField::TextField(int16 _x1, int16 _y1, int16 _x2, int16 _y2) : RectClass(_x1, _y1, _x2, _y2) {
	string = nullptr;
	string_len = 0;
	must_redraw = true;
}

TextField::~TextField() {
	if (string != nullptr)
		mem_free(string);
}

void TextField::set_string(char *_string) {

	must_redraw = true;

	if (_string == nullptr && string != nullptr) {
		string[0] = '\0';
		return;
	}
	int16 _string_len = (int16)(cstrlen(_string) + 1);
	if (string == nullptr) {
		string = (char *)mem_alloc(_string_len, "string");
	} else {
		if (string_len < _string_len) {
			string = (char *)mem_realloc(string, _string_len, "string");
		}
	}
	if (!string)
		error_show(FL, 'OOM!', "TextField set_string:%s", _string);

	string_len = _string_len;
	cstrcpy(string, _string);
}

#if defined (__WIN)
void TextField::draw(CDIBSectionBuffer *myBuffer)
#else
void TextField::draw(GrBuff *myBuffer)
#endif
{
	if (!_GL(visible))
		return;

	Buffer *myBuff = myBuffer->get_buffer();
	gr_color_set(__BLACK);
	gr_buffer_rect_fill(myBuff, _x1, _y1, _x2 - _x1, _y2 - _y1);
	gr_font_set_color(__WHITE);
	gr_font_set(_G(font_inter));
	gr_font_write(myBuff, string, _x1, _y1, 0, 1);
	myBuffer->release();

	ScreenContext *iC = vmng_screen_find(_G(interface).gameInterfaceBuff, nullptr);
	RestoreScreensInContext(_x1, _y1, _x2, _y2, iC);
	must_redraw = false;
}

//-------------------------------------------------------------------------------------------

void ButtonClass::init() {
	_relaxed = _over = _picked = 0; _tag = 0;
	_must_redraw = true;
	_state = BUTTON_RELAXED;
	_tracking = -1;
}

ButtonClass::ButtonClass(RectClass *r, const char *btnName, int16 tag) : RectClass(r) {
	init();
	cstrncpy(_name, btnName, 19);
	_tag = tag;
	_hidden = false;
}

ButtonClass::ButtonClass() : RectClass() {
	init();
	cstrcpy(_name, "?");
	_hidden = false;
}

ButtonClass::~ButtonClass() {
	zap_resources();
}

void ButtonClass::set_name(const char *btnName) {
	cstrncpy(_name, btnName, 19);
}

bool ButtonClass::is_hidden() const {
	return _hidden;
}

void ButtonClass::set_sprite_relaxed(int16 r) {
	_relaxed = r;
}

void ButtonClass::set_sprite_picked(int16 p) {
	_picked = p;
}

//aug23
void ButtonClass::set_sprite_over(int16 o) {
	_over = o;
}

int16 ButtonClass::get_tag() const {
	return _tag;
}
void ButtonClass::zap_resources() {
	if (_relaxed)
		ClearWSAssets(_WS_ASSET_CELS, _relaxed, _relaxed);
	if (_over)
		ClearWSAssets(_WS_ASSET_CELS, _over, _over);
	if (_picked)
		ClearWSAssets(_WS_ASSET_CELS, _picked, _picked);
}

void ButtonClass::set(ButtonClass *b) {
	zap_resources();
	cstrncpy(_name, b->_name, 19);
	_x1 = b->_x1;
	_y1 = b->_y1;
	_x2 = b->_x2;
	_y2 = b->_y2;
	_tag = b->_tag;
	_relaxed = b->_relaxed;
	_over = b->_over;
	_picked = b->_picked;
}

void ButtonClass::set(int16 x1, int16 y1, int16 x2, int16 y2, int16 tag) {
	_x1 = x1;
	_y1 = y1;
	_x2 = x2;
	_y2 = y2;
	_tag = tag;
}

void ButtonClass::set(int16 x1, int16 y1, int16 x2, int16 y2, int16 tag,
	int16 relaxed, int16 over, int16 picked, int32 sprite) {
	zap_resources();
	_x1 = x1;
	_y1 = y1;
	_x2 = x2;
	_y2 = y2;
	_tag = tag;
	_relaxed = relaxed;
	_over = over;
	_picked = picked;
	_sprite = sprite;
}

int16 ButtonClass::inside(int16 x, int16 y) const {
	if (RectClass::inside(x, y))
		return _tag;

	return -1;
}

ControlStatus ButtonClass::track(int32 eventType, int16 x, int16 y) {
	if (!_GL(visible))
		return NOTHING;

	ButtonState old_state = _state;
	ControlStatus result = NOTHING;

	bool button_clicked = (eventType == _ME_L_click) || (eventType == _ME_L_hold) || (eventType == _ME_L_drag);

	int16 overVal = inside(x, y);

	if (overVal == _tag) {
		// if Button is pressed
		if (button_clicked) {
			if (_tracking == 1) {
				//term_message( "BUTTON CLICKED and TRACKING" );
				//state = BUTTON_OVER; //aug23

				result = TRACKING;
			} else {
				_tracking = 1;
				result = IN_CONTROL;
				_state = BUTTON_PICKED;
				//aug28 //sep8 here.
				//if( !between_rooms && !_G(inv_suppress_click_sound) && !hidden) {
				//    digi_play( inv_click_snd, 2, 255, -1, inv_click_snd_room_lock ); //aug26: single click
					//term_message( "hidden %d", hidden ); //sep8
				//}
			}
			_G(inv_suppress_click_sound) = false;

		} else {
			// if Button isn't pressed

			if (_tracking == 1) {
				result = SELECTED;
				//term_message( "BUTTON RELEASED" );
			} else
				result = OVER_CONTROL;

			_state = BUTTON_OVER;
			_tracking = -1;
		}
	} else {
		result = NOTHING;
		_tracking = -1;
		_state = BUTTON_RELAXED;
	}

	if (old_state != _state)
		_must_redraw = true;

	return result;
}

void ButtonClass::draw(GrBuff *myBuffer) {
	if (!_GL(visible))
		return;

	if (!_must_redraw)
		return;

	Buffer *myBuff = myBuffer->get_buffer();

	gr_color_set(__BLACK);
	gr_buffer_rect_fill(myBuff, _x1, _y1 - 2, _x2 - _x1, _y2 - _y1 + 2);

	if (_hidden == false) {
		switch (_state) {
		case BUTTON_RELAXED:
			series_show_frame(_sprite, _relaxed, myBuff, _x1, _y1);
			break;

		case BUTTON_PICKED:
			series_show_frame(_sprite, _picked, myBuff, _x1, _y1);
			break;

		case BUTTON_OVER:
			series_show_frame(_sprite, _over, myBuff, _x1, _y1);
			break;
		}
	}

	myBuffer->release();

	_must_redraw = false;
	ScreenContext *iC = vmng_screen_find(_G(interface).gameInterfaceBuff, nullptr);
	RestoreScreensInContext(_x1, _y1 - 2, _x2, _y2, iC);
}

void ButtonClass::hide() {
	_hidden = true;
	_must_redraw = true;
}

void ButtonClass::unhide() {
	_hidden = false;
	_must_redraw = true;
}

//-------------------------------------------------------------------------------------------

Toggler::Toggler() : ButtonClass() {
	_toggle_state = SELECTED;
	_state = BUTTON_PICKED;
}

ControlStatus Toggler::track(int32 eventType, int16 x, int16 y) {
	if (!_GL(visible))
		return NOTHING;

	ButtonState old_state = _state;
	ControlStatus result = NOTHING;

	bool button_clicked = (eventType == _ME_L_click) || (eventType == _ME_L_hold) || (eventType == _ME_L_drag);

	int16 overVal = inside(x, y);

	if (overVal == _tag) {
		// if Button is pressed
		if (button_clicked) {
			if (eventType == _ME_L_click) {
				//aug28
					//if( !_G(inv_suppress_click_sound) && !between_rooms && !hidden ) {
					//	digi_play( inv_click_snd, 2, 255, -1, inv_click_snd_room_lock ); //aug27: single click
						//term_message( "hidden %d", hidden ); //sep8
					//}
			}

			_tracking = 1;
			result = IN_CONTROL;
		} else { // if Button isn't pressed
			if (_tracking == 1) {
				result = SELECTED;
				_toggle_state = (_toggle_state == SELECTED) ? NOTHING : SELECTED;
			} else {
				result = OVER_CONTROL;
			}
			_tracking = -1;
		}
	} else {
		if (button_clicked && _tracking == 1) {
			result = TRACKING;
		} else {
			result = NOTHING;
			_tracking = -1;
		}
	}

	_state = (_toggle_state == SELECTED) ? BUTTON_PICKED : BUTTON_RELAXED;

	if (old_state != _state)
		_must_redraw = true;

	return result;
}

//-------------------------------------------------------------------------------------------

InterfaceBox::InterfaceBox(RectClass *r) {
	if (!r) error_show(FL, 'CGNR');
	r->copyInto(this);
	_highlight_index = -1;
	_must_redraw_all = true;
	_selected = false;
	_index = 0;

	for (int16 iter = 0; iter < MAX_BUTTONS; iter++)
		_button[iter] = nullptr;
}

InterfaceBox::~InterfaceBox() {
}

int16 InterfaceBox::inside(int16 x, int16 y) const {
	if (!_index)
		return -1;

	if (!RectClass::inside(x, y))
		return -1;

	int16 iter;
	for (iter = 0; iter < _index; iter++) {
		if (_button[iter]->inside(x, y))
			return _button[iter]->get_tag();
	}

	return -1;
}

void InterfaceBox::highlight_button(int16 index) {

	if (_highlight_index == index) {
		return;
	}

	if (_highlight_index != -1)
		_button[_highlight_index]->_must_redraw = true;

	if (index == -1)
		_selected = false;

	_highlight_index = index;

	if (_highlight_index != -1)
		_button[_highlight_index]->_must_redraw = true;
}

void InterfaceBox::set_selected(bool s) {

	if (s == _selected)
		return;

	_selected = s;

	if (_highlight_index != -1)
		_button[_highlight_index]->_must_redraw = true;
}

void InterfaceBox::add(ButtonClass *b) {
	if (!b) {
		error_show(FL, 'CGIA');
	} else if (_index >= MAX_BUTTONS) {
		error_show(FL, 'CGIA');
	} else {
		// convert to global coordinates
		b->_x1 += _x1;
		b->_x2 += _x1;
		b->_y1 += _y1;
		b->_y2 += _y1;

		_button[_index] = b;
		_button[_index]->_must_redraw = true;
		++_index;
	}
}

ControlStatus InterfaceBox::track(int32 eventType, int16 x, int16 y) {
	ControlStatus result = NOTHING;

	for (int iter = 0; iter < _index; iter++) {
		if (_button[iter]->track(eventType, x, y) == SELECTED) {
			_highlight_index = _button[iter]->get_tag();
			term_message("selected button: %ld", iter);
			result = SELECTED;
		}
	}

	return result;
}

void InterfaceBox::draw(GrBuff *myBuffer) {
	if (!_GL(visible))
		return;

	if (_must_redraw_all) {
		gr_color_set(__BLACK);
		//gr_buffer_rect_fill(myBuff, x1, y1+10, x2-x1, y2-y1);
	}
	myBuffer->release();

	int16 iter;
	for (iter = 0; iter < _index; iter++) {
		_button[iter]->_must_redraw |= _must_redraw_all;
		_button[iter]->draw(myBuffer);
	}

	if (_must_redraw_all) {
		ScreenContext *iC = vmng_screen_find(_G(interface).gameInterfaceBuff, nullptr);
		RestoreScreensInContext(_x1, _y1, _x2, _y2, iC);
		kernel_trigger_dispatch(kernel_trigger_create(TRIG_INV_CLICK));
	}

	_must_redraw_all = false;
}

//-------------------------------------------------------------------------------------------

Inventory::Inventory(RectClass *r, int32 sprite, int16 cells_h, int16 cells_v, int16 cell_w, int16 cell_h, int16 tag)
		: RectClass(r) {
	_sprite = sprite;

	for (int16 iter = 0; iter < INVENTORY_CELLS_COUNT; iter++) {
		_cells[iter] = -1;
		_cursors[iter] = -1;
		_names[iter] = nullptr;
	}
	_num_cells = 0;
	_tag = tag;
	_cells_h = cells_h;
	_cells_v = cells_v;
	_cell_w = cell_w;
	_cell_h = cell_h;

	// if requested cell configuration doesn't fit, blow up.
	if ((cells_h * cell_w > (_x2 - _x1)) || (cells_v * cell_h > (_y2 - _y1))) {
		error_show(FL, 'CGIC');
	}

	_hidden = false;
	_highlight = _dehighlight = -1;
	_must_redraw_all = true;
	_must_redraw1 = -1;
	_must_redraw2 = -1;
	_scroll = 0;
	_right_arrow_visible = false;
}

Inventory::~Inventory() {
}

bool Inventory::add(char *name, char *verb, int32 invSprite, int32 cursor) {
	// Don't add something twice
	int iter;
	for (iter = 0; iter < _num_cells; iter++) {
		if (!strcmp(name, _names[iter]))
			return true;
	}

	if (_num_cells >= INVENTORY_CELLS_COUNT) {
		error_show(FL, 'CGIA');
		return false;
	}
	_names[_num_cells] = name;
	_verbs[_num_cells] = verb;

	_cells[_num_cells] = (int16)invSprite;
	_cursors[_num_cells] = (int16)cursor;
	++_num_cells;
	_must_redraw_all = true;

	return true;
}

bool Inventory::need_left() const {
	return (_scroll != 0);
}

bool Inventory::need_right() const {
	if ((_num_cells - _scroll - MAX_BUTTONS) > 0)
		return true;

	return false;
}

void Inventory::set_scroll(int32 new_scroll) {
	_scroll = new_scroll;
	_must_redraw_all = true;
}

bool Inventory::remove(char *name) {
	int iter;
	for (iter = 0; iter < _num_cells; iter++) {
		// Found the thing?
		if (!strcmp(name, _names[iter])) {

			// Eat up its slot by moving everything down
			do {

				_names[iter] = _names[iter + 1];
				_verbs[iter] = _verbs[iter + 1];	// DT oct 18
				_cells[iter] = _cells[iter + 1];
				_cursors[iter] = _cursors[iter + 1];

				++iter;

			} while (iter < _num_cells);

			--_num_cells;
			_must_redraw_all = true;
			_scroll = 0;
			return true;
		}
	}

	// Didn't find that thing.
	return false;
}


void Inventory::hide(bool hidden) {
	_hidden = hidden;
	_must_redraw_all = true;
}

int16 Inventory::inside(int16 x, int16 y) {
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
	if (_cells_h > _cells_v) {				// Horizontal orientation, fill left to right
		return (int16)((index % _cells_v) * _cell_h);
	} else {								// Vertical orientation, fill top to bottom
		return (int16)((index % _cells_h) * _cell_h);
	}
}

void Inventory::highlight_part(int16 _index) {
	if (_highlight == _index) {
		return;
	}

	_must_redraw1 = _highlight;
	_highlight = _index;
	_must_redraw2 = _highlight;
}

void Inventory::draw(GrBuff *myBuffer) {
	if (!_GL(visible))
		return;

	if (!_must_redraw1 && !_must_redraw2 && !_must_redraw_all)
		return;

	int16 offx = 0, offy = 0;
	int cell_iter;

	Buffer *myBuff = myBuffer->get_buffer();

	if (_hidden) {
		gr_color_set(__BLACK);
		gr_buffer_rect_fill(myBuff, _x1, _y1, _x2 - _x1, _y2 - _y1);
		goto done;
	}

	if (_must_redraw_all) {
		kernel_trigger_dispatch(kernel_trigger_create(TRIG_INV_CLICK));
		gr_color_set(__BLACK);
		gr_buffer_rect_fill(myBuff, _x1, _y1, _x2 - _x1, _y2 - _y1);
	}

	offy += 2;
	offx += 2;
	_right_arrow_visible = false;

	for (cell_iter = 0; (cell_iter + _scroll < _num_cells) && (cell_iter < MAX_BUTTONS); cell_iter++) {
		int16 left = (int16)(_x1 + offx + cell_pos_x(cell_iter));
		int16 top = (int16)(_y1 + offy + cell_pos_y(cell_iter));

		if (_must_redraw1 == cell_iter || _must_redraw2 == cell_iter || _must_redraw_all) {
			// This does the button update....
			refresh_right_arrow();
			refresh_left_arrow();

			// Draw_icon_here
			gr_color_set(__BLACK);
			gr_buffer_rect_fill(myBuff, left, top, _cell_w + 1, _cell_h + 1);
			series_show_frame(_sprite, _cells[cell_iter + _scroll], myBuff, left - 3, top - 3);

			// Draw_box around icon
			if (_highlight == cell_iter) {
				gr_line(left, top, left + _cell_w - 2, top + 1, __LTGRAY, myBuff);
				gr_line(left, top + _cell_h - 2, left + _cell_w - 2, top + _cell_h - 2, __LTGRAY, myBuff);
				gr_line(left, top, left, top + _cell_h - 2, __LTGRAY, myBuff);
				gr_line(left + _cell_w - 2, top + 1, left + _cell_w - 2, top + _cell_h - 2, __LTGRAY, myBuff);
			}
		}
	}
done:
	myBuffer->release();

	ScreenContext *iC = vmng_screen_find(_G(interface).gameInterfaceBuff, nullptr);
	RestoreScreensInContext(_x1, _y1, _x2, _y2, iC);
	_must_redraw1 = _must_redraw2 = -1;
	_must_redraw_all = false;
}

ControlStatus Inventory::track(int32 eventType, int16 x, int16 y) {
	if (!_GL(visible))
		return NOTHING;

	static int16 interface_tracking = -1;
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
#if 0
	switch (result) {
	case OVER_CONTROL:
		term_message("inv:over %ld", over);
		break;
	case SELECTED:
		term_message("inv:selected %ld", over);
		break;
	case IN_CONTROL:
		term_message("inv:in %ld", over);
		break;
	case NOTHING:
		term_message("inv:nothing %ld", over);
		break;
	}
#endif
	if (result == NOTHING && button_clicked)
		return TRACKING;

	return result;
}

} // End of namespace Burger
} // End of namespace M4
