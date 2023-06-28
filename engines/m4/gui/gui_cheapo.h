
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

#ifndef M4_GUI_GUI_CHEAPO_H
#define M4_GUI_GUI_CHEAPO_H

#include "m4/graphics/gr_buff.h"
#include "m4/m4_types.h"

namespace M4 {

constexpr int16 INVENTORY_CELLS_COUNT = 128;
constexpr int16 ARROW_WIDTH = 8;
constexpr int16 MAX_BUTTONS = 20;

constexpr int16 LEFT_ARROW_TAG = 128;
constexpr int16 RIGHT_ARROW_TAG = 129;

constexpr int16 LEFT_ARROW_TAG_DORMANT = 130;
constexpr int16 RIGHT_ARROW_TAG_DORMANT = 134;
constexpr int16 LEFT_ARROW_TAG_ROLL = 131;
constexpr int16 RIGHT_ARROW_TAG_ROLL = 135;
constexpr int16 LEFT_ARROW_TAG_DOWN = 132;
constexpr int16 RIGHT_ARROW_TAG_DOWN = 136;
constexpr int16 LEFT_ARROW_TAG_NONFUNC = 133;
constexpr int16 RIGHT_ARROW_TAG_NONFUNC = 137;

enum ControlStatus {
	NOTHING, IN_CONTROL, OVER_CONTROL, SELECTED, TRACKING
};

enum ButtonState {
	BUTTON_RELAXED, BUTTON_OVER, BUTTON_PICKED
};

class RectClass;
class ButtonClass;
class InterfaceBox;
class Inventory;

class RectClass {
public:
	int16 x1 = 0, x2 = 0, y1 = 0, y2 = 0;
public:
	RectClass();
	RectClass(int16 _x1, int16 _y1, int16 _x2, int16 _y2);
	RectClass(RectClass *);
	virtual ~RectClass();
	virtual int16 inside(int16 x, int16 y);

	void copyInto(RectClass *);

	void set(int16 _x1, int16 _y1, int16 _x2, int16 _y2);
	void set(RectClass *);
};

class TextField : public RectClass {
private:
	char *string = nullptr;
	int16 string_len = 0;
public:
	bool must_redraw = false;

public:
	TextField(int16 _x1, int16 _y1, int16 _x2, int16 _y2);
	~TextField();

	void set_string(char *_string);
	void draw(GrBuff *interface_buffer);
};

class ButtonClass : public RectClass {
protected:
	int16 tag = 0;
	int16 relaxed = 0;
	int16 over = 0;
	int16 picked = 0;
	int16 tracking = 0;
	int32 sprite = 0;
	bool highlighted = false;
	bool hidden = false;

	void init();
	void zap_resources();

public:
	ButtonState	state = BUTTON_RELAXED;
	char name[20] = { 0 }; // fixme? inventory objects get 40 char strings. use MAX_PLYR_STRING_LEN?
	bool must_redraw = false;

public:
	ButtonClass();
	ButtonClass(RectClass *r, const char *btnName, int16 tag);
	~ButtonClass();

	void draw(GrBuff *interface_buffer);
	int16 inside(int16 x, int16 y);
	virtual ControlStatus track(int32 eventType, int16 x, int16 y);

	void set(ButtonClass *b);
	void set(int16 x1, int16 y1, int16 x2, int16 y2, int16 tag);
	void set(int16 x1, int16 y1, int16 x2, int16 y2, int16 tag, int16 _relaxed,
		int16 _over, int16 _picked, int32 _sprite);
	void set_name(const char *btnName);

	int16 get_tag();

	void hide();
	void unhide();
	bool is_hidden();
	void set_sprite_relaxed(int16 r);
	void set_sprite_picked(int16 p);
	void set_sprite_over(int16 o);
};

class Toggler : public ButtonClass {
public:
	Toggler();
	ControlStatus track(int32 eventType, int16 x, int16 y);
	ControlStatus toggle_state;
};

class InterfaceBox : public RectClass {
private:
	bool selected = false;
	int16 index = 0;
	ButtonClass *button[MAX_BUTTONS] = { nullptr };

public:
	InterfaceBox(RectClass *r);
	~InterfaceBox();

	void draw(GrBuff *interface_buffer);
	int16 inside(int16 x, int16 y);
	ControlStatus track(int32 eventType, int16 x, int16 y);

	void add(ButtonClass *b);
	int16 check_inventory(int16 x, int16 y);
	void highlight_button(int16 _index);
	void set_selected(bool);

	int16 highlight_index;
	bool must_redraw_all;
};

class Inventory : public RectClass {
private:
	int32 sprite = 0;
	int16 tag = 0;
	int16 num_cells = 0;
	bool right_arrow_visible = false;

	int16 cell_pos_x(int16 index);
	int16 cell_pos_y(int16 index);

public:
	int16 scroll = 0;
	int16 cells_h = 0, cells_v = 0;
	int16 cell_w = 0, cell_h = 0;
	int16 must_redraw1 = 0, must_redraw2 = 0;
	int16 highlight = 0, dehighlight = 0;
	bool must_redraw_all = false;
	bool hidden = false;

	int16 cells[INVENTORY_CELLS_COUNT] = { 0 };
	int16 cursors[INVENTORY_CELLS_COUNT] = { 0 };
	char *names[INVENTORY_CELLS_COUNT] = { nullptr };
	char *verbs[INVENTORY_CELLS_COUNT] = { nullptr };

public:
	Inventory(RectClass *, int32 _sprite, int16 _cells_h, int16 _cells_v, int16 _cell_w, int16 _cell_h, int16 _tag);
	~Inventory();

	void draw(GrBuff *interface_buffer);

	int16 inside(int16 x, int16 y);
	ControlStatus track(int32 eventType, int16 x, int16 y);

	bool add(char *name, char *verb, int32 cel, int32 cursor);
	bool remove(char *name);
	void hide(bool);
	void highlight_part(int16 _index);

	bool need_left();
	bool need_right();
	void scroll_left();
	void scroll_right();
	void set_scroll(int32 new_scroll);
};

} // End of namespace M4

#endif
