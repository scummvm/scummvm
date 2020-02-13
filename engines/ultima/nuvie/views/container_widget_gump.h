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

#ifndef NUVIE_VIEWS_CONTAINER_WIDGET_GUMP_H
#define NUVIE_VIEWS_CONTAINER_WIDGET_GUMP_H

#include "ultima/nuvie/gui/widgets/gui_widget.h"
#include "ultima/nuvie/core/obj_manager.h"
#include "ultima/nuvie/views/container_widget.h"

namespace Ultima {
namespace Nuvie {

class Configuration;
class TileManager;
class Actor;
class Font;

class ContainerWidgetGump : public ContainerWidget {

private:
	sint16 cursor_x;
	uint16 cursor_y;
	sint16 check_x, check_y;
	const Tile *cursor_tile;
	bool show_cursor;

public:
	ContainerWidgetGump(Configuration *cfg, GUI_CallBack *callback = NULL);
	~ContainerWidgetGump() override;

	bool init(Actor *a, uint16 x, uint16 y, uint8 Cols, uint8 Rows, TileManager *tm, ObjManager *om, Font *f, uint8 check_xoff, uint8 check_yoff);

	void Display(bool full_redraw) override;
	GUI_status KeyDown(const Common::KeyState &key) override;

	void set_actor(Actor *a) override;
private:

	void cursor_right();
	void cursor_left();
	void cursor_up();
	void cursor_down();
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
