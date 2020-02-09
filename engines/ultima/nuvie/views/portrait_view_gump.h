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

#ifndef NUVIE_VIEWS_PORTRAIT_VIEW_GUMP_H
#define NUVIE_VIEWS_PORTRAIT_VIEW_GUMP_H

#include "ultima/nuvie/views/draggable_view.h"

namespace Ultima {
namespace Nuvie {

class Configuration;
class TileManager;
class ObjManager;
class Screen;
class Actor;
class Font;
class U6Bmp;
class Portrait;

class PortraitViewGump : public DraggableView {

	GUI_Button *gump_button;

	GUI_Font *font;

	Portrait *portrait;
	unsigned char *portrait_data;
	Actor *actor;
	bool show_cursor;
	const Tile *cursor_tile;
	gumpCursorPos cursor_pos;
	uint8 cursor_xoff, cursor_yoff;

public:
	PortraitViewGump(Configuration *cfg);
	~PortraitViewGump() override;

	bool init(Screen *tmp_screen, void *view_manager, uint16 x, uint16 y, Font *f, Party *p, TileManager *tm, ObjManager *om, Portrait *por, Actor *a);

	void Display(bool full_redraw) override;

	GUI_status callback(uint16 msg, GUI_CallBack *caller, void *data) override;

	GUI_status MouseDown(int x, int y, Shared::MouseButton button) override;
	GUI_status MouseWheel(sint32 x, sint32 y) override;

protected:

	void set_actor(Actor *a);
	void left_arrow();
	void right_arrow();
	GUI_status KeyDown(const Common::KeyState &key) override;
	GUI_status set_cursor_pos(gumpCursorPos pos);
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
