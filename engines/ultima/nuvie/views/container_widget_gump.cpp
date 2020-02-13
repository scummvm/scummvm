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

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/gui/gui.h"
#include "ultima/nuvie/views/container_widget_gump.h"
#include "ultima/nuvie/keybinding/keys.h"
#include "ultima/nuvie/views/view_manager.h"

namespace Ultima {
namespace Nuvie {

static const Tile gump_empty_tile = {
	0,
	false,
	false,
	false,
	false,
	false,
	true,
	false,
	false,
	0,
	//uint8 qty;
	//uint8 flags;

	0,
	0,
	0,

	{
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 143, 142, 141, 141, 142, 143, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 143, 141, 141, 142, 142, 141, 141, 143, 255, 255, 255, 255,
		255, 255, 255, 143, 141, 142, 143, 143, 143, 143, 142, 141, 143, 255, 255, 255,
		255, 255, 143, 141, 142, 143, 0, 0, 0, 0, 143, 142, 141, 143, 255, 255,
		255, 255, 142, 141, 143, 0, 0, 0, 0, 0, 0, 143, 141, 142, 255, 255,
		255, 255, 141, 142, 143, 0, 0, 0, 0, 0, 0, 143, 142, 141, 255, 255,
		255, 255, 141, 142, 143, 0, 0, 0, 0, 0, 0, 143, 142, 141, 255, 255,
		255, 255, 142, 141, 143, 0, 0, 0, 0, 0, 0, 143, 141, 142, 255, 255,
		255, 255, 143, 141, 142, 143, 0, 0, 0, 0, 143, 142, 141, 143, 255, 255,
		255, 255, 255, 143, 141, 142, 143, 143, 143, 143, 142, 141, 143, 255, 255, 255,
		255, 255, 255, 255, 143, 141, 141, 142, 142, 141, 141, 143, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 143, 142, 141, 141, 142, 143, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
		255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255
	}
};


ContainerWidgetGump::ContainerWidgetGump(Configuration *cfg, GUI_CallBack *callback) : ContainerWidget(cfg, callback) {
	cursor_tile = NULL;
	empty_tile = &gump_empty_tile;
	obj_font_color = 15;
	bg_color = 0;
	fill_bg = false;

	cursor_x = cursor_y = 0;
	show_cursor = true;
}

ContainerWidgetGump::~ContainerWidgetGump() {

}

bool ContainerWidgetGump::init(Actor *a, uint16 x, uint16 y, uint8 Cols, uint8 Rows, TileManager *tm, ObjManager *om, Font *f, uint8 check_xoff, uint8 check_yoff) {
	tile_manager = tm;
	obj_manager = om;

	rows = Rows;
	cols = Cols;
	check_x = - x + check_xoff;
	check_y = - y + check_yoff;
//objlist_offset_x = 0;
//objlist_offset_y = 0;

//72 =  4 * 16 + 8
	GUI_Widget::Init(NULL, x, y, cols * 16, rows * 16);

	set_actor(a);
	set_accept_mouseclick(true, 0);//USE_BUTTON); // accept [double]clicks from button1 (even if double-click disabled we need clicks)

	cursor_tile = tile_manager->get_gump_cursor_tile();

	return true;
}

void ContainerWidgetGump::Display(bool full_redraw) {
	display_inventory_list();
	if (show_cursor) {
		screen->blit(area.left + (cursor_x < 0 ? check_x : cursor_x * 16), area.top + (cursor_x < 0 ? check_y : cursor_y * 16),
			(const unsigned char *)cursor_tile->data, 8, 16, 16, 16, true);
	}
	screen->update(area.left, area.top, area.width(), area.height());
}

void ContainerWidgetGump::cursor_right() {
	if (cursor_x < cols - 1) {
		if (get_obj_at_location((cursor_x + 1) * 16, cursor_y * 16) != NULL) {
			cursor_x++;
		}
	}
}

void ContainerWidgetGump::cursor_left() {
	if (cursor_x > -1) {
		cursor_x--;
	}
}

void ContainerWidgetGump::cursor_up() {
	if (cursor_y > 0) {
		cursor_y--;
	} else {
		up_arrow();
	}
}

void ContainerWidgetGump::cursor_down() {
	if (get_obj_at_location(0, (cursor_y + 1) * 16) != NULL) { //check that we can move down one row.
		if (cursor_y < rows - 1) {
			cursor_y++;
		} else {
			down_arrow();
		}

		for (; cursor_x > 0; cursor_x--) {
			if (get_obj_at_location(cursor_x * 16, cursor_y * 16) != NULL) {
				break;
			}
		}
	}
}

GUI_status ContainerWidgetGump::KeyDown(const Common::KeyState &key) {
	KeyBinder *keybinder = Game::get_game()->get_keybinder();
	ActionType a = keybinder->get_ActionType(key);

	switch (keybinder->GetActionKeyType(a)) {
	case NORTH_KEY:
		cursor_up();
		break;
	case SOUTH_KEY:
		cursor_down();
		break;
	case WEST_KEY:
		cursor_left();
		break;
	case EAST_KEY:
		cursor_right();
		break;
	case DO_ACTION_KEY:
		if (cursor_x == -1) {
			Game::get_game()->get_view_manager()->close_gump((DraggableView *)parent);
			break;
		}
		selected_obj = get_obj_at_location(cursor_x * 16, cursor_y * 16);
		if (selected_obj) {
			try_click();
		}
		break;
	default:
		return GUI_PASS;
	}

	return GUI_YUM;
}

void ContainerWidgetGump::set_actor(Actor *a) {
	cursor_x = cursor_y = 0;
	ContainerWidget::set_actor(a);
}

} // End of namespace Nuvie
} // End of namespace Ultima
