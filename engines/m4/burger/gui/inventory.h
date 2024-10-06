
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

#ifndef M4_BURGER_GUI_INVENTORY_H
#define M4_BURGER_GUI_INVENTORY_H

#include "m4/gui/gui_cheapo.h"

namespace M4 {
namespace Burger {
namespace GUI {

using namespace M4::GUI;

constexpr int16 INVENTORY_CELLS_COUNT = 128;
constexpr int16 ARROW_WIDTH = 8;
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
