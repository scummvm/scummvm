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

#ifndef ULTIMA6_VIEWS_DOLL_VIEW_GUMP_H
#define ULTIMA6_VIEWS_DOLL_VIEW_GUMP_H

#include "ultima/ultima6/core/nuvie_defs.h"
#include "DraggableView.h"

namespace Ultima {
namespace Ultima6 {

class Configuration;
class TileManager;
class ObjManager;
class Screen;
class Actor;
class Font;
class DollWidget;

#define DOLLVIEWGUMP_HEIGHT 136

class DollViewGump : public DraggableView {

	GUI_Button *gump_button;
	GUI_Button *combat_button;
	GUI_Button *heart_button;
	GUI_Button *party_button;
	GUI_Button *inventory_button;
	DollWidget *doll_widget;

	GUI_Font *font;

	Actor *actor;
	bool is_avatar;
	bool show_cursor;
	const Tile *cursor_tile;
	gumpCursorPos cursor_pos;
	uint8 cursor_xoff, cursor_yoff;

	SDL_Surface *actor_doll;

public:
	DollViewGump(Configuration *cfg);
	~DollViewGump();

	bool init(Screen *tmp_screen, void *view_manager, uint16 x, uint16 y, Actor *a, Font *f, Party *p, TileManager *tm, ObjManager *om);

	void set_actor(Actor *a);
	Actor *get_actor() {
		return actor;
	}

	void Display(bool full_redraw);

	virtual GUI_status MouseDown(int x, int y, int button);
	virtual GUI_status MouseUp(int x, int y, int button);
	virtual GUI_status MouseMotion(int x, int y, Uint8 state) {
		return DraggableView::MouseMotion(x, y, state);
	}
	virtual GUI_status MouseWheel(sint32 xpos, sint32 ypos);
	virtual void MoveRelative(int dx, int dy) {
		return DraggableView::MoveRelative(dx, dy);
	}


	GUI_status callback(uint16 msg, GUI_CallBack *caller, void *data);
protected:

	void displayEquipWeight();
	void displayCombatMode();

	void left_arrow();
	void right_arrow();

private:

	void activate_combat_button();
	void setColorKey(SDL_Surface *image);
	GUI_status set_cursor_pos(gumpCursorPos pos);
	GUI_status moveCursorRelative(uint8 direction);
	GUI_status KeyDown(SDL_Keysym key);

};

} // End of namespace Ultima6
} // End of namespace Ultima

#endif
