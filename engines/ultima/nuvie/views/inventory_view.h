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

#ifndef NUVIE_VIEWS_INVENTORY_VIEW_H
#define NUVIE_VIEWS_INVENTORY_VIEW_H

#include "ultima/nuvie/views/view.h"

namespace Ultima {
namespace Nuvie {

class Configuration;
class TileManager;
class ObjManager;
class Screen;
class Actor;
class Font;
class Party;
class DollWidget;
class InventoryWidget;

class InventoryView : public View {

	DollWidget *doll_widget;
	InventoryWidget *inventory_widget;
	GUI_Button *combat_button;
	bool is_party_member;
	bool picking_pocket;
	Actor *outside_actor;
	bool lock_actor;

	enum invarea {
		INVAREA_LIST = 0, // cursor is over the inventory list (x,y)
		INVAREA_TOP,      // the actor or container icon
		INVAREA_DOLL,     // the readied-items list (0-7)
		INVAREA_COMMAND   // the command icons (0-4)
	};
	struct invcursor_pos_s {
		enum invarea area;
		uint8 x, y; // y is only used for inventory list
		uint32 px, py;
	} cursor_pos;
	Tile *cursor_tile;
	bool show_cursor;

public:
	InventoryView(Configuration *cfg);
	~InventoryView() override;

	bool init(Screen *tmp_screen, void *view_manager, uint16 x, uint16 y, Font *f, Party *p, TileManager *tm, ObjManager *om);
	bool set_party_member(uint8 party_member) override;
	bool set_actor(Actor *actor, bool pickpocket = false);
	void set_show_cursor(bool state);
	void moveCursorToSlot(uint8 slot_num);
	void moveCursorToInventory(uint8 inv_x = 0, uint8 inv_y = 0);
	void moveCursorRelative(sint8 new_x, sint8 new_y);
	void moveCursorToButton(uint8 button_num = 0);
	void moveCursorToTop();
	bool select_obj(Obj *obj);
	void select_objAtCursor();
	Obj *get_objAtCursor();
	InventoryWidget *get_inventory_widget() {
		return (inventory_widget);
	};

	void Display(bool full_redraw) override;
	void PlaceOnScreen(Screen *s, GUI_DragManager *dm, int x, int y) override;
	GUI_status KeyDown(const Common::KeyState &key) override;
	void simulate_CB_callback();
	bool is_picking_pocket() {
		return picking_pocket;
	}
	void lock_to_actor(bool value) {
		lock_actor = value;
	}

protected:

	void display_name();
	void add_command_icons(Screen *tmp_screen, void *view_manager);
	void display_inventory_weights();
	void display_combat_mode();
	void update_cursor();
	void hide_buttons();
	void show_buttons();
	GUI_status MouseDown(int x, int y, Shared::MouseButton button) override;
	GUI_status MouseWheel(sint32 x, sint32 y) override;
	GUI_status callback(uint16 msg, GUI_CallBack *caller, void *data) override;
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
