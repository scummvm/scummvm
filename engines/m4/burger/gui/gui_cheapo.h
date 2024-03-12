
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

#ifndef M4_BURGER_GUI_CHEAPO_H
#define M4_BURGER_GUI_CHEAPO_H

#include "common/str.h"
#include "m4/graphics/gr_buff.h"
#include "m4/m4_types.h"

namespace M4 {
namespace Burger {
namespace GUI {

constexpr int16 INVENTORY_CELLS_COUNT = 128;
constexpr int16 ARROW_WIDTH = 8;
constexpr int16 MAX_BUTTONS = 20;
constexpr int16 MAX_INVENTORY = 9;

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
	BUTTON_0, BUTTON_RELAXED, BUTTON_OVER, BUTTON_PICKED
};

class RectClass;
class ButtonClass;
class InterfaceBox;
class Inventory;

class RectClass {
public:
	int16 _x1 = 0, _x2 = 0, _y1 = 0, _y2 = 0;
public:
	RectClass();
	RectClass(int16 x1, int16 y1, int16 x2, int16 y2);
	RectClass(const RectClass *);
	virtual ~RectClass();
	virtual int16 inside(int16 x, int16 y) const;

	void copyInto(RectClass *r) const;

	void set(int16 x1, int16 y1, int16 x2, int16 y2);
	void set(const RectClass *r);
};

class TextField : public RectClass {
private:
	char *_string = nullptr;
	int16 _string_len = 0;
public:
	bool _must_redraw = false;

public:
	TextField(int16 x1, int16 y1, int16 x2, int16 y2);
	~TextField();

	void set_string(const char *string);
	void draw(GrBuff *interface_buffer);
};

class ButtonClass : public RectClass {
protected:
	int16 _tag = 0;
	int16 _unknown = 0;
	int16 _relaxed = 0;
	int16 _over = 0;
	int16 _picked = 0;
	int16 _tracking = 0;
	int32 _sprite = 0;
	bool _highlighted = false;
	bool _hidden = false;

	void init();
	void zap_resources();

public:
	ButtonState	_state = BUTTON_RELAXED;
	Common::String _name;
	bool _must_redraw = false;

public:
	ButtonClass();
	ButtonClass(const RectClass &r, const Common::String &btnName, int16 tag);
	ButtonClass(const RectClass &r, const Common::String &btnName, int16 tag,
		int16 unknown, int16 relaxed, int16 over, int16 picked, int sprite);
	~ButtonClass();

	void draw(GrBuff *interface_buffer);
	int16 inside(int16 x, int16 y) const override;
	virtual ControlStatus track(int32 eventType, int16 x, int16 y);

	void set(const ButtonClass *b);
	void set(int16 x1, int16 y1, int16 x2, int16 y2, int16 tag);
	void set(int16 x1, int16 y1, int16 x2, int16 y2, int16 tag, int16 unknown,
		int16 relaxed, int16 over, int16 picked, int32 sprite);
	void set_name(const Common::String &btnName);

	int16 get_tag() const;

	void hide();
	void unhide();
	bool is_hidden() const;
	void set_sprite_relaxed(int16 r);
	void set_sprite_picked(int16 p);
	void set_sprite_over(int16 o);
	void set_sprite_unknown(int16 val);
};

class Toggler : public ButtonClass {
public:
	ControlStatus _toggle_state;
public:
	Toggler();
	ControlStatus track(int32 eventType, int16 x, int16 y);
};

class InterfaceBox : public RectClass {
private:
	bool _selected = false;
	int16 _index = 0;
	ButtonClass *_button[MAX_BUTTONS] = { nullptr };

public:
	int16 _highlight_index = 0;
	bool _must_redraw_all = false;

public:
	InterfaceBox(const RectClass &r);
	~InterfaceBox();

	void draw(GrBuff *interface_buffer);
	int16 inside(int16 x, int16 y) const override;
	ControlStatus track(int32 eventType, int16 x, int16 y);

	void add(ButtonClass *b);
	int16 check_inventory(int16 x, int16 y);
	void highlight_button(int16 index);
	void set_selected(bool);

};

class Inventory : public RectClass {
	struct Entry {
		Common::String _name;
		Common::String _verb;
		int16 _cell = -1;
		int16 _cursor = -1;
	};
private:
	int32 _sprite = 0;
	int16 _tag = 0;
	int16 _num_cells = 0;
	bool _right_arrow_visible = false;

	int16 cell_pos_x(int16 index);
	int16 cell_pos_y(int16 index);
	int16 interface_tracking = -1;

public:
	int16 _scroll = 0;
	int16 _cells_h = 0, _cells_v = 0;
	int16 _cell_w = 0, _cell_h = 0;
	int16 _must_redraw1 = 0, _must_redraw2 = 0;
	int16 _highlight = 0;
	bool _must_redraw_all = false;

	Entry _items[INVENTORY_CELLS_COUNT];

public:
	Inventory(const RectClass &r, int32 sprite, int16 cells_h, int16 cells_v, int16 cell_w, int16 cell_h, int16 tag);
	~Inventory();

	void draw(GrBuff *interface_buffer);

	int16 inside(int16 x, int16 y) const override;
	ControlStatus track(int32 eventType, int16 x, int16 y);

	bool add(const Common::String &name, const Common::String &verb, int32 cel, int32 cursor);
	bool remove(const Common::String &name);
	void highlight_part(int16 index);

	bool need_left() const;
	bool need_right() const;
	void set_scroll(int32 new_scroll);
};

} // namespace GUI
} // namespace Burger
} // namespace M4

#endif
