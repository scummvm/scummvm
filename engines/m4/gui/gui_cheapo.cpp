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
#include "m4/globals.h"

namespace M4 {

#define _GL(X) _G(interface).X

static void refresh_right_arrow() {
	error("TODO: refresh_right_arrow");
}

static void refresh_left_arrow() {
	error("TODO: refresh_left_arrow");
}

RectClass::RectClass() {
	x1 = y1 = x2 = y2 = 0;
}

RectClass::RectClass(RectClass *r) {
	if (!r)
		error_show(FL, 'CGNR');

	x1 = r->x1;
	y1 = r->y1;
	x2 = r->x2;
	y2 = r->y2;
}

RectClass::RectClass(int16 _x1, int16 _y1, int16 _x2, int16 _y2) {
	x1 = _x1;
	y1 = _y1;
	x2 = _x2;
	y2 = _y2;
}

RectClass::~RectClass() {
}

void RectClass::copyInto(RectClass *r) {
	if (!r)
		error_show(FL, 'CGNR');
	r->x1 = x1;
	r->y1 = y1;
	r->x2 = x2;
	r->y2 = y2;
}

void RectClass::set(int16 _x1, int16 _y1, int16 _x2, int16 _y2) {
	x1 = _x1;
	y1 = _y1;
	x2 = _x2;
	y2 = _y2;
}

void RectClass::set(RectClass *r) {
	if (!r) error_show(FL, 'CGNR');
	x1 = r->x1; y1 = r->y1; x2 = r->x2; y2 = r->y2;
}

int16 RectClass::inside(int16 x, int16 y) {
	if ((x >= x1) && (x <= x2) && (y >= y1) && (y <= y2))
		return 1;
	return 0;
}

//-------------------------------------------------------------------------------------------

TextField::TextField(int16 _x1, int16 _y1, int16 _x2, int16 _y2) : RectClass(_x1, _y1, _x2, _y2) {
	string = NULL;
	string_len = 0;
	must_redraw = true;
}

TextField::~TextField() {
	if (string != NULL)
		mem_free(string);
}

void TextField::set_string(char *_string) {

	must_redraw = true;

	if (_string == NULL && string != NULL) {
		string[0] = '\0';
		return;
	}
	int16 _string_len = (int16)(cstrlen(_string) + 1);
	if (string == NULL) {
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
	gr_buffer_rect_fill(myBuff, x1, y1, x2 - x1, y2 - y1);
	gr_font_set_color(__WHITE);
	gr_font_set(_G(font_inter));
	gr_font_write(myBuff, string, x1, y1, 0, 1);
	myBuffer->release();

	ScreenContext *iC = vmng_screen_find(_G(interface).gameInterfaceBuff, NULL);
	RestoreScreensInContext(x1, y1, x2, y2, iC);
	must_redraw = false;
}

//-------------------------------------------------------------------------------------------

void ButtonClass::init() {
	relaxed = over = picked = 0; tag = 0;
	must_redraw = true;
	state = BUTTON_RELAXED;
	tracking = -1;
}

ButtonClass::ButtonClass(RectClass *r, const char *btnName, int16 _tag) : RectClass(r) {
	init();
	cstrncpy(name, btnName, 19);
	tag = _tag;
	hidden = false; //aug21
}

ButtonClass::ButtonClass() : RectClass() {
	init();
	cstrcpy(name, "?");
	hidden = false; //aug21
}

ButtonClass::~ButtonClass() {
	zap_resources();
}

void ButtonClass::set_name(const char *btnName) {
	cstrncpy(name, btnName, 19);
}

//aug23
bool ButtonClass::is_hidden() {
	return hidden;
}

//aug23
void ButtonClass::set_sprite_relaxed(int16 r) {
	relaxed = r;
}

//aug23
void ButtonClass::set_sprite_picked(int16 p) {
	picked = p;
}

//aug23
void ButtonClass::set_sprite_over(int16 o) {
	over = o;
}

int16 ButtonClass::get_tag() {
	return tag;
}
void ButtonClass::zap_resources() {
	if (relaxed)
		ClearWSAssets(_WS_ASSET_CELS, relaxed, relaxed);
	if (over)
		ClearWSAssets(_WS_ASSET_CELS, over, over);
	if (picked)
		ClearWSAssets(_WS_ASSET_CELS, picked, picked);
}

void ButtonClass::set(ButtonClass *b) {
	zap_resources();
	cstrncpy(name, b->name, 19);
	x1 = b->x1; y1 = b->y1; x2 = b->x2; y2 = b->y2; tag = b->tag;
	relaxed = b->relaxed; over = b->over; picked = b->picked;
}

void ButtonClass::set(int16 _x1, int16 _y1, int16 _x2, int16 _y2, int16 _tag) {
	x1 = _x1;
	y1 = _y1;
	x2 = _x2;
	y2 = _y2;
	tag = _tag;
}

void ButtonClass::set(int16 _x1, int16 _y1, int16 _x2, int16 _y2, int16 _tag,
	int16 _relaxed, int16 _over, int16 _picked, int32 _sprite) {
	zap_resources();
	x1 = _x1;
	y1 = _y1;
	x2 = _x2;
	y2 = _y2;
	tag = _tag;
	relaxed = _relaxed;
	over = _over;
	picked = _picked;
	sprite = _sprite;
}

int16 ButtonClass::inside(int16 x, int16 y) {
	if (RectClass::inside(x, y))
		return tag;
	return -1;
}

ControlStatus ButtonClass::track(int32 eventType, int16 x, int16 y) {
	if (!_GL(visible))
		return NOTHING;

	ButtonState old_state = state;
	ControlStatus result = NOTHING;

	bool button_clicked = (eventType == _ME_L_click) || (eventType == _ME_L_hold) || (eventType == _ME_L_drag);

	//term_message( "BUTTONCLASS::TRACK" );

	int16 overVal = inside(x, y);

	if (overVal == tag) {
		// if Button is pressed
		if (button_clicked) {
			if (tracking == 1) {
				//term_message( "BUTTON CLICKED and TRACKING" );
				//state = BUTTON_OVER; //aug23

				result = TRACKING;
			} else {
				tracking = 1;
				result = IN_CONTROL;
				state = BUTTON_PICKED;
				//aug28 //sep8 here.
				//if( !between_rooms && !_G(inv_suppress_click_sound) && !hidden) {
				//    digi_play( inv_click_snd, 2, 255, -1, inv_click_snd_room_lock ); //aug26: single click
					//term_message( "hidden %d", hidden ); //sep8
				//}
			}
			_G(inv_suppress_click_sound) = false;

		} else { // if Button isn't pressed

			if (tracking == 1) {
				result = SELECTED;
				//term_message( "BUTTON RELEASED" );
			} else
				result = OVER_CONTROL;

			state = BUTTON_OVER;

			tracking = -1;
		}
	} else {
		result = NOTHING;
		tracking = -1;
		state = BUTTON_RELAXED;
	}

	if (old_state != state)
		must_redraw = true;

	return result;
}

#if defined(__WIN)
void ButtonClass::draw(CDIBSectionBuffer *myBuffer)
#else
void ButtonClass::draw(GrBuff *myBuffer)
#endif
{
	if (!_GL(visible))
		return;

	if (!must_redraw)
		return;

	Buffer *myBuff = myBuffer->get_buffer();

	gr_color_set(__BLACK);
	gr_buffer_rect_fill(myBuff, x1, y1 - 2, x2 - x1, y2 - y1 + 2);

	if (hidden == false) {
		switch (state) {
		case BUTTON_RELAXED:
			series_show_frame(sprite, relaxed, myBuff, x1, y1);
			break;

		case BUTTON_PICKED:
			series_show_frame(sprite, picked, myBuff, x1, y1);
			break;

		case BUTTON_OVER:
			series_show_frame(sprite, over, myBuff, x1, y1);
			break;
		}
	}

	myBuffer->release();

	must_redraw = false;
	ScreenContext *iC = vmng_screen_find(_G(interface).gameInterfaceBuff, NULL);
	RestoreScreensInContext(x1, y1 - 2, x2, y2, iC);
}

void ButtonClass::hide() {
	hidden = true;
	must_redraw = true;
}

//aug21
void ButtonClass::unhide() {
	hidden = false;
	must_redraw = true;
}

//-------------------------------------------------------------------------------------------

Toggler::Toggler() : ButtonClass() {
	toggle_state = SELECTED; state = BUTTON_PICKED;
}

ControlStatus Toggler::track(int32 eventType, int16 x, int16 y) {
	if (!_GL(visible))
		return NOTHING;

	ButtonState old_state = state;
	ControlStatus result = NOTHING;

	bool button_clicked = (eventType == _ME_L_click) || (eventType == _ME_L_hold) || (eventType == _ME_L_drag);

	int16 overVal = inside(x, y);

	if (overVal == tag) {
		// if Button is pressed
		if (button_clicked) {
			if (eventType == _ME_L_click) {
				//aug28
					//if( !_G(inv_suppress_click_sound) && !between_rooms && !hidden ) {
					//	digi_play( inv_click_snd, 2, 255, -1, inv_click_snd_room_lock ); //aug27: single click
						//term_message( "hidden %d", hidden ); //sep8
					//}
			}

			tracking = 1;
			result = IN_CONTROL;
		} else { // if Button isn't pressed
			if (tracking == 1) {
				result = SELECTED;
				toggle_state = (toggle_state == SELECTED) ? NOTHING : SELECTED;
			} else {
				result = OVER_CONTROL;
			}
			tracking = -1;
		}
	} else {
		if (button_clicked && tracking == 1) {
			result = TRACKING;
		} else {
			result = NOTHING;
			tracking = -1;
		}
	}

	state = (toggle_state == SELECTED) ? BUTTON_PICKED : BUTTON_RELAXED;

	if (old_state != state)
		must_redraw = true;

	return result;
}

//-------------------------------------------------------------------------------------------

InterfaceBox::InterfaceBox(RectClass *r) {
	if (!r) error_show(FL, 'CGNR');
	r->copyInto(this);
	highlight_index = -1;
	must_redraw_all = true;
	selected = false;
	index = 0;
	for (int16 iter = 0; iter < MAX_BUTTONS; iter++)
		button[iter] = NULL;
}

InterfaceBox::~InterfaceBox() {
}

int16 InterfaceBox::inside(int16 x, int16 y) {
	if (!index)
		return -1;

	if (!RectClass::inside(x, y))
		return -1;

	int16 iter;
	for (iter = 0; iter < index; iter++) {
		if (button[iter]->inside(x, y))
			return button[iter]->get_tag();
	}
	return -1;
}

void InterfaceBox::highlight_button(int16 _index) {

	if (highlight_index == _index) {
		return;
	}

	if (highlight_index != -1)
		button[highlight_index]->must_redraw = true;

	if (_index == -1)
		selected = false;

	highlight_index = _index;

	if (highlight_index != -1)
		button[highlight_index]->must_redraw = true;
}

void InterfaceBox::set_selected(bool _s) {

	if (_s == selected)
		return;

	selected = _s;

	if (highlight_index != -1)
		button[highlight_index]->must_redraw = true;
}

void InterfaceBox::add(ButtonClass *b) {
	if (!b)
		error_show(FL, 'CGIA');
	if (index >= MAX_BUTTONS)
		error_show(FL, 'CGIA');

	// convert to global coordinates
	b->x1 += x1; b->x2 += x1;
	b->y1 += y1; b->y2 += y1;

	button[index] = b;
	button[index]->must_redraw = true;
	++index;
}

ControlStatus InterfaceBox::track(int32 eventType, int16 x, int16 y) {
	ControlStatus result = NOTHING;

	for (int iter = 0; iter < index; iter++) {
		if (button[iter]->track(eventType, x, y) == SELECTED) {
			highlight_index = button[iter]->get_tag();
			term_message("selected button: %ld", iter);
			result = SELECTED;
		}
	}
	return result;
}

void InterfaceBox::draw(GrBuff *myBuffer) {
	if (!_GL(visible))
		return;
#if 0
	Buffer *myBuff = myBuffer->get_buffer();
#endif

	if (must_redraw_all) {
		gr_color_set(__BLACK);
		//gr_buffer_rect_fill(myBuff, x1, y1+10, x2-x1, y2-y1);
	}
	myBuffer->release();

	int16 iter;
	for (iter = 0; iter < index; iter++) {
		button[iter]->must_redraw |= must_redraw_all;
		button[iter]->draw(myBuffer);
	}

	if (must_redraw_all) {
		ScreenContext *iC = vmng_screen_find(_G(interface).gameInterfaceBuff, NULL);
		RestoreScreensInContext(x1, y1, x2, y2, iC);
		kernel_trigger_dispatch(kernel_trigger_create(TRIG_INV_CLICK));
	}
	must_redraw_all = false;
}

//-------------------------------------------------------------------------------------------

Inventory::Inventory(RectClass *r, int32 _sprite, int16 _cells_h, int16 _cells_v, int16 _cell_w, int16 _cell_h, int16 _tag)
	: RectClass(r) {

	sprite = _sprite;

	for (int16 iter = 0; iter < INVENTORY_CELLS_COUNT; iter++) {
		cells[iter] = -1;
		cursors[iter] = -1;
		names[iter] = NULL;
	}
	num_cells = 0;
	tag = _tag;
	cells_h = _cells_h; cells_v = _cells_v; cell_w = _cell_w; cell_h = _cell_h;

	// if requested cell configuration doesn't fit, blow up.
	if ((cells_h * cell_w > (x2 - x1)) || (cells_v * cell_h > (y2 - y1))) {
		error_show(FL, 'CGIC');
	}

	hidden = false;
	highlight = dehighlight = -1;
	must_redraw_all = true; must_redraw1 = -1; must_redraw2 = -1;
	scroll = 0;
	right_arrow_visible = false;
}

Inventory::~Inventory() {
}

// Inventory::add always adds at the end

bool Inventory::add(char *name, char *verb, int32 invSprite, int32 cursor) {

	// don't add something twice
	int iter;
	for (iter = 0; iter < num_cells; iter++) {
		if (!strcmp(name, names[iter]))
			return true;
	}

	if (num_cells >= INVENTORY_CELLS_COUNT) {
		error_show(FL, 'CGIA');
		return false;
	}
	names[num_cells] = name;
	verbs[num_cells] = verb;	// DT oct 18

	cells[num_cells] = (int16)invSprite;
	cursors[num_cells] = (int16)cursor;
	++num_cells;
	must_redraw_all = true;
	return true;
}

bool Inventory::need_left() {
	return (scroll != 0);
}

bool Inventory::need_right() {
	//term_message( "num_cells %d scroll %d", num_cells, scroll );
	if ((num_cells - scroll - MAX_BUTTONS) > 0)
		return true;
	return false;
}

void Inventory::set_scroll(int32 new_scroll) {
	scroll = new_scroll;
	must_redraw_all = true;
}

bool Inventory::remove(char *name) {

	int iter;
	for (iter = 0; iter < num_cells; iter++) {
		// found the thing?
		if (!strcmp(name, names[iter])) {

			// eat up its slot by moving everything down
			do {

				names[iter] = names[iter + 1];
				verbs[iter] = verbs[iter + 1];	// DT oct 18
				cells[iter] = cells[iter + 1];
				cursors[iter] = cursors[iter + 1];

				++iter;

			} while (iter < num_cells);

			--num_cells;
			must_redraw_all = true;
			scroll = 0;
			return true;
		}
	}

	// didn't find that thing.
	return false;
}


void Inventory::hide(bool _hidden) {
	hidden = _hidden;
	must_redraw_all = true;
}

int16 Inventory::inside(int16 x, int16 y) {
	if ((x < x1) || (x >= (x2 - 1)) || (y < y1 + 2) ||
		(y > y1 + cells_v * cell_h - 2))
		return -1;

	x -= x1;
	y -= y1;
	//term_message( "BOX # %d", (int16)((x/cell_w) * cells_v + (y/cell_h)) );
	return (int16)((x / cell_w) * cells_v + (y / cell_h));
}

int16 Inventory::cell_pos_x(int16 index) {
	if (cells_h > cells_v) {				// horizontal orientation, fill left to right
		return (int16)((index / cells_v) * cell_w);
	} else {									// vertical orientation, fill top to bottom
		return (int16)((index / cells_h) * cell_w);
	}
}

int16 Inventory::cell_pos_y(int16 index) {
	if (cells_h > cells_v) {				// horizontal orientation, fill left to right
		return (int16)((index % cells_v) * cell_h);
	} else {									// vertical orientation, fill top to bottom
		return (int16)((index % cells_h) * cell_h);
	}
}

void Inventory::highlight_part(int16 _index) {

	if (highlight == _index) {
		return;
	}

	must_redraw1 = highlight;
	highlight = _index;
	must_redraw2 = highlight;
}

void Inventory::draw(GrBuff *myBuffer) {
	if (!_GL(visible))
		return;

	if (!must_redraw1 && !must_redraw2 && !must_redraw_all)
		return;

	int16 offx = 0, offy = 0;
	int cell_iter;

	Buffer *myBuff = myBuffer->get_buffer();

	if (hidden) {
		gr_color_set(__BLACK);
		gr_buffer_rect_fill(myBuff, x1, y1, x2 - x1, y2 - y1);
		goto done;
	}

	if (must_redraw_all) {
		kernel_trigger_dispatch(kernel_trigger_create(TRIG_INV_CLICK));
		gr_color_set(__BLACK);
		gr_buffer_rect_fill(myBuff, x1, y1, x2 - x1, y2 - y1);
	}

	offy += 2;
	offx += 2;
	right_arrow_visible = false;

	for (cell_iter = 0; (cell_iter + scroll < num_cells) && (cell_iter < MAX_BUTTONS); cell_iter++) {
		int16 left = (int16)(x1 + offx + cell_pos_x(cell_iter));
		//int16 right = (int16)(left + cell_w);
		int16 top = (int16)(y1 + offy + cell_pos_y(cell_iter));
		//int16 bottom = (int16)(top + cell_h);

		if (must_redraw1 == cell_iter || must_redraw2 == cell_iter || must_redraw_all) {
			// this does the button update....
			refresh_right_arrow();
			refresh_left_arrow();

			//	draw_icon_here
			gr_color_set(__BLACK);
			gr_buffer_rect_fill(myBuff, left, top, cell_w + 1, cell_h + 1);
			series_show_frame(sprite, cells[cell_iter + scroll], myBuff, left - 3, top - 3);

			//	draw_box around icon
			if (highlight == cell_iter) {
				gr_line(left, top, left + cell_w - 2, top + 1, __LTGRAY, myBuff);
				gr_line(left, top + cell_h - 2, left + cell_w - 2, top + cell_h - 2, __LTGRAY, myBuff);
				gr_line(left, top, left, top + cell_h - 2, __LTGRAY, myBuff);
				gr_line(left + cell_w - 2, top + 1, left + cell_w - 2, top + cell_h - 2, __LTGRAY, myBuff);
			}
		}
	}
done:
	myBuffer->release();

	ScreenContext *iC = vmng_screen_find(_G(interface).gameInterfaceBuff, NULL);
	RestoreScreensInContext(x1, y1, x2, y2, iC);
	must_redraw1 = must_redraw2 = -1;
	must_redraw_all = false;
}

ControlStatus Inventory::track(int32 eventType, int16 x, int16 y) {
	if (!_GL(visible))
		return NOTHING;

	static int16 interface_tracking = -1;
	ControlStatus result = NOTHING;

	int16 over = inside(x, y);
	bool button_clicked = eventType == _ME_L_click || eventType == _ME_L_hold || eventType == _ME_L_drag;

	// if Button is pressed
	if (button_clicked) {
		// if we are not tracking, start tracking
		if (interface_tracking == -1) {
			highlight_part(over);
			interface_tracking = over;
			result = IN_CONTROL;
		}
		// else if we are over something we are tracking
		else {
			if (interface_tracking == over) {
				highlight_part(over);
				result = IN_CONTROL;
			}
			// else highlight nothing
			else {
				highlight_part(-1);
				result = NOTHING;
			}
		}
	}
	// if Button isn't pressed
	else {
		// if we unpressed on something we were tracking
		if (interface_tracking == over) {
			if (interface_tracking == -1)
				result = NOTHING;
			else
				result = SELECTED;
		} else {
			if (over + scroll < num_cells)
				result = OVER_CONTROL;
			else
				result = NOTHING;
		}
		// stop tracking anything
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

} // End of namespace M4
