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

#include "m4/gui/gui_cheapo.h"
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
#include "m4/vars.h"

namespace M4 {
namespace GUI {

RectClass::RectClass() {
}

RectClass::RectClass(const RectClass *r) {
	if (!r) {
		error_show(FL, 'CGNR');
	} else {
		_x1 = r->_x1;
		_y1 = r->_y1;
		_x2 = r->_x2;
		_y2 = r->_y2;
	}
}

RectClass::RectClass(int16 x1, int16 y1, int16 x2, int16 y2) :
	_x1(x1), _y1(y1), _x2(x2), _y2(y2) {
}

RectClass::~RectClass() {
}

void RectClass::copyInto(RectClass *r) const {
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

void RectClass::set(const RectClass *r) {
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

TextField::TextField(int16 x1, int16 y1, int16 x2, int16 y2) :
	RectClass(x1, y1, x2, y2) {
	_string = nullptr;
	_string_len = 0;
	_must_redraw = true;
}

TextField::~TextField() {
	if (_string != nullptr)
		mem_free(_string);
}

void TextField::set_string(const char *string) {
	_must_redraw = true;

	if (string == nullptr && _string != nullptr) {
		_string[0] = '\0';
		return;
	}

	int16 string_len = (int16)(cstrlen(string) + 1);
	if (_string == nullptr) {
		_string = (char *)mem_alloc(string_len, "string");
	} else {
		if (_string_len < string_len) {
			_string = (char *)mem_realloc(_string, string_len, "string");
		}
	}

	if (!_string)
		error_show(FL, 'OOM!', "TextField set_string:%s", _string);

	_string_len = string_len;
	cstrcpy(_string, string);
}

void TextField::draw(GrBuff *myBuffer) {
	if (!INTERFACE_VISIBLE)
		return;

	Buffer *myBuff = myBuffer->get_buffer();
	gr_color_set(__BLACK);
	gr_buffer_rect_fill(myBuff, _x1, _y1, _x2 - _x1, _y2 - _y1);

	gr_font_set(_G(font_inter));
	font_set_colors(1, 2, 3);
	gr_font_write(myBuff, _string, _x1, _y1, 0, 1);
	myBuffer->release();

	ScreenContext *iC = vmng_screen_find(_G(gameInterfaceBuff), nullptr);
	RestoreScreensInContext(_x1, _y1, _x2, _y2, iC);
	_must_redraw = false;
}

//-------------------------------------------------------------------------------------------

void ButtonClass::init() {
	_relaxed = _over = _picked = 0;
	_tag = 0;
	_must_redraw = true;
	_state = BUTTON_RELAXED;
	_tracking = -1;
}

ButtonClass::ButtonClass(const RectClass &r, const Common::String &btnName, int16 tag) : RectClass(r) {
	init();
	_name = btnName;
	_tag = tag;
}

ButtonClass::ButtonClass(const RectClass &r, const Common::String &btnName, int16 tag,
	int16 unknown, int16 relaxed, int16 over, int16 picked, int sprite) : RectClass(r),
	_tag(tag), _unknown(unknown), _relaxed(relaxed), _over(over), _picked(picked), _sprite(sprite) {
}

ButtonClass::ButtonClass(const RectClass &r, const Common::String &btnName, int16 tag,
	int16 relaxed, int16 over, int16 picked, int sprite) : RectClass(r),
	_tag(tag), _unknown(0), _relaxed(relaxed), _over(over), _picked(picked), _sprite(sprite) {
}

ButtonClass::ButtonClass() : RectClass() {
	init();
	_name = "?";
}

ButtonClass::~ButtonClass() {
	zap_resources();
}

void ButtonClass::set_name(const Common::String &btnName) {
	_name = btnName;
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

void ButtonClass::set_sprite_over(int16 o) {
	_over = o;
}

void ButtonClass::set_sprite_unknown(int16 val) {
	_unknown = val;
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

void ButtonClass::set(const ButtonClass *b) {
	zap_resources();
	_name = b->_name;
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
	int16 unknown, int16 relaxed, int16 over, int16 picked, int32 sprite) {
	zap_resources();
	_x1 = x1;
	_y1 = y1;
	_x2 = x2;
	_y2 = y2;
	_tag = tag;
	_unknown = unknown;
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
	if (!INTERFACE_VISIBLE)
		return NOTHING;

	ButtonState old_state = _state;
	ControlStatus result = NOTHING;

	bool button_clicked = (eventType == _ME_L_click) || (eventType == _ME_L_hold) || (eventType == _ME_L_drag);

	int16 overTag = inside(x, y);

	if (overTag == _tag) {
		// if Button is pressed
		if (button_clicked) {
			if (_tracking == 1) {
				result = TRACKING;
			} else {
				_tracking = 1;
				result = IN_CONTROL;
				_state = BUTTON_PICKED;
			}

			_G(inv_suppress_click_sound) = false;

		} else {
			// if Button isn't pressed
			if (_tracking == 1) {
				result = SELECTED;
			} else {
				result = OVER_CONTROL;
			}

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
	if (!INTERFACE_VISIBLE)
		return;

	if (!_must_redraw)
		return;

	Buffer *myBuff = myBuffer->get_buffer();

	gr_color_set(__BLACK);
	gr_buffer_rect_fill(myBuff, _x1, _y1 - 2, _x2 - _x1, _y2 - _y1 + 2);

	if (_hidden == false) {
		switch (_state) {
		case BUTTON_0:
			series_show_frame(_sprite, _unknown, myBuff, _x1, _y1);
			break;

		case BUTTON_RELAXED:
			series_show_frame(_sprite, _relaxed, myBuff, _x1, _y1);
			break;

		case BUTTON_OVER:
			series_show_frame(_sprite, _over, myBuff, _x1, _y1);
			break;

		case BUTTON_PICKED:
			series_show_frame(_sprite, _picked, myBuff, _x1, _y1);
			break;
		}
	}

	myBuffer->release();

	_must_redraw = false;
	ScreenContext *iC = vmng_screen_find(_G(gameInterfaceBuff), nullptr);
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
	if (!INTERFACE_VISIBLE)
		return NOTHING;

	ButtonState old_state = _state;
	ControlStatus result = NOTHING;

	bool button_clicked = (eventType == _ME_L_click) || (eventType == _ME_L_hold) || (eventType == _ME_L_drag);

	int16 overTag = inside(x, y);

	if (overTag == _tag) {
		// if Button is pressed
		if (button_clicked) {
			_tracking = 1;
			result = IN_CONTROL;

		} else {
			// Button isn't pressed
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

InterfaceBox::InterfaceBox(const RectClass &r) {
	r.copyInto(this);
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
		// Convert to global coordinates
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
			term_message("selected button: %d", iter);
			result = SELECTED;
			break;
		}
	}

	return result;
}

void InterfaceBox::draw(GrBuff *myBuffer) {
	if (!INTERFACE_VISIBLE)
		return;

	for (int iter = 0; iter < _index; iter++) {
		_button[iter]->_must_redraw |= _must_redraw_all;
		_button[iter]->draw(myBuffer);
	}

	if (_must_redraw_all) {
		ScreenContext *iC = vmng_screen_find(_G(gameInterfaceBuff), nullptr);
		RestoreScreensInContext(_x1, _y1, _x2, _y2, iC);
	}

	_must_redraw_all = false;
}

} // namespace GUI
} // namespace M4
