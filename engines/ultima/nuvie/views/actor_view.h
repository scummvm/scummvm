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

#ifndef NUVIE_VIEWS_ACTOR_VIEW_H
#define NUVIE_VIEWS_ACTOR_VIEW_H

#include "ultima/nuvie/views/view.h"

namespace Ultima {
namespace Nuvie {

class Configuration;
class Screen;
class Font;
class TileManager;
class ObjManager;
class Portrait;
class Party;

class ActorView : public View {

	Portrait *portrait;

	unsigned char *portrait_data;
	struct actcursor_pos_s {
		uint8 x;
		uint32 px, py;
	} cursor_pos;
	Tile *cursor_tile;
	bool show_cursor;

public:
	ActorView(Configuration *cfg);
	~ActorView() override;

	bool init(Screen *tmp_screen, void *view_manager, uint16 x, uint16 y, Font *f, Party *p, TileManager *tm, ObjManager *om, Portrait *port);

	bool set_party_member(uint8 party_member) override;

	void Display(bool full_redraw) override;
	void update() {
		update_display = true;
	}
	void set_show_cursor(bool state);
	void moveCursorToButton(sint8 button_num);

protected:

	void add_command_icons(Screen *tmp_screen, void *view_manager);
	void display_name();
	void display_actor_stats();
	bool in_party;
	GUI_status MouseDown(int x, int y, Shared::MouseButton button) override;
	GUI_status KeyDown(const Common::KeyState &key) override;
	GUI_status MouseWheel(sint32 x, sint32 y) override;
	void update_cursor();
	void select_button();
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
