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
#include "ultima/nuvie/screen/screen.h"
#include "ultima/nuvie/misc/u6_llist.h"
#include "ultima/nuvie/gui/gui_button.h"
#include "ultima/nuvie/views/doll_widget.h"
#include "ultima/nuvie/views/inventory_widget.h"
#include "ultima/nuvie/views/inventory_view.h"
#include "ultima/nuvie/core/party.h"
#include "ultima/nuvie/fonts/font.h"
#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/core/events.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/usecode/usecode.h"
#include "ultima/nuvie/views/view_manager.h"
#include "ultima/nuvie/keybinding/keys.h"

namespace Ultima {
namespace Nuvie {

static const char combat_mode_tbl[][8] = {"COMMAND", " FRONT", "  REAR", " FLANK", "BERSERK", "RETREAT", "ASSAULT"};
static const char combat_mode_tbl_se[][6] = {"CMND", "RANGE", "FLEE", "CLOSE"};
static const char combat_mode_tbl_md[][6] = {"CMND", "RANGE", "FLEE", "ATTK"};
#define MD Game::get_game()->get_game_type()==NUVIE_GAME_MD

InventoryView::InventoryView(Configuration *cfg) : View(cfg),
	doll_widget(NULL), inventory_widget(NULL), combat_button(NULL) {
	cursor_pos.area = INVAREA_LIST;
	cursor_pos.x = cursor_pos.y = 0;
	cursor_pos.px = cursor_pos.py = 0;
	cursor_tile = NULL;
	show_cursor = false;
	is_party_member = false;
	picking_pocket = false;
	outside_actor = NULL;
	lock_actor = false;
}

InventoryView::~InventoryView() {
}

bool InventoryView::set_party_member(uint8 party_member) {
	if (lock_actor || party_member >= party->get_party_size()) {
		return false;
	}

	picking_pocket = false;

	if (View::set_party_member(party_member)
	        && !Game::get_game()->get_event()->using_control_cheat()) {
		is_party_member = true;
		if (doll_widget)
			doll_widget->set_actor(party->get_actor(cur_party_member));
		if (inventory_widget)
			inventory_widget->set_actor(party->get_actor(cur_party_member));

		show_buttons();

		if (combat_button) {
			if (party_member == 0)
				combat_button->Hide();
			else
				combat_button->Show();
		}

		return true;
	}
	is_party_member = false;
	hide_buttons();

	if (actor_button) actor_button->Show();

	return false;
}

bool InventoryView::set_actor(Actor *actor, bool pickpocket) {
	if (lock_actor)
		return false;

	if (party->contains_actor(actor)) {
		set_party_member(party->get_member_num(actor));
		return true;
	}
	picking_pocket = pickpocket;
	is_party_member = false;
	outside_actor = actor;
	if (doll_widget)
		doll_widget->set_actor(actor);
	if (inventory_widget)
		inventory_widget->set_actor(actor);

	if (picking_pocket) {
		if (actor_button) actor_button->Hide();
	}
	hide_buttons();

	return true;
}

bool InventoryView::init(Screen *tmp_screen, void *view_manager, uint16 x, uint16 y, Font *f, Party *p, TileManager *tm, ObjManager *om) {
	if (Game::get_game()->get_game_type() == NUVIE_GAME_U6)
		View::init(x, y, f, p, tm, om);
	else
		View::init(x, y - 2, f, p, tm, om);

	doll_widget = new DollWidget(config, this);
	doll_widget->init(party->get_actor(cur_party_member), 0, 8, tile_manager, obj_manager, true);

	AddWidget(doll_widget);

	inventory_widget = new InventoryWidget(config, this);
	inventory_widget->init(party->get_actor(cur_party_member), 64, 8, tile_manager, obj_manager, font);

	AddWidget(inventory_widget);

	add_command_icons(tmp_screen, view_manager);

	cursor_tile = tile_manager->get_cursor_tile();


	return true;
}


void InventoryView::PlaceOnScreen(Screen *s, GUI_DragManager *dm, int x, int y) {
	GUI_Widget::PlaceOnScreen(s, dm, x, y);
	update_cursor(); // initial position; uses area
}


void InventoryView::Display(bool full_redraw) {
	full_redraw = true;
	if (full_redraw || update_display) {
		if (MD)
			fill_md_background(lock_actor ? 7 : bg_color, area);
		else
			screen->fill(bg_color, area.left, area.top, area.width(), area.height());

		if (is_party_member)
			display_combat_mode();
		display_name();
		display_inventory_weights();
	}

	DisplayChildren(full_redraw);

	if (full_redraw || update_display) {
		update_display = false;
		screen->update(area.left, area.top, area.width(), area.height());
	}

	if (show_cursor && cursor_tile != NULL) {
		screen->blit(cursor_pos.px, cursor_pos.py, (unsigned char *)cursor_tile->data,
		             8, 16, 16, 16, true, NULL);
		screen->update(cursor_pos.px, cursor_pos.py, 16, 16);
	}

	return;
}

void InventoryView::display_name() {
	const char *name;
	int y_off = 0;
	if (Game::get_game()->get_game_type() == NUVIE_GAME_SE)
		y_off = 1;
	if (is_party_member)
		name = party->get_actor_name(cur_party_member);
	else if (picking_pocket)
		name = outside_actor->get_name();
	else
		name = Game::get_game()->get_player()->get_actor()->get_name(true);
	if (name == NULL)
		return;

	font->drawString(screen, name, area.left + ((136) - strlen(name) * 8) / 2, area.top + y_off);

	return;
}

void InventoryView::add_command_icons(Screen *tmp_screen, void *view_manager) {
	Tile *tile;
	int y = 96;
	if (MD)
		y = 100;
	else if (Game::get_game()->get_game_type() == NUVIE_GAME_U6)
		y = 80;
	Graphics::ManagedSurface *button_image;
	Graphics::ManagedSurface *button_image2;
//FIX need to handle clicked button image, check image free on destruct.

	tile = tile_manager->get_tile(MD ? 282 : 387); //left arrow icon
	button_image = tmp_screen->create_sdl_surface_from(tile->data, 8, 16, 16, 16);
	button_image2 = tmp_screen->create_sdl_surface_from(tile->data, 8, 16, 16, 16);
	left_button = new GUI_Button(this, 0, y, button_image, button_image2, this);
	this->AddWidget(left_button);

	tile = tile_manager->get_tile(MD ? 279 : 384); //party view icon
	button_image = tmp_screen->create_sdl_surface_from(tile->data, 8, 16, 16, 16);
	button_image2 = tmp_screen->create_sdl_surface_from(tile->data, 8, 16, 16, 16);
	party_button = new GUI_Button(view_manager, 16, y, button_image, button_image2, this);
	this->AddWidget(party_button);

	tile = tile_manager->get_tile(MD ? 280 : 385); //actor view icon
	button_image = tmp_screen->create_sdl_surface_from(tile->data, 8, 16, 16, 16);
	button_image2 = tmp_screen->create_sdl_surface_from(tile->data, 8, 16, 16, 16);
	actor_button = new GUI_Button(view_manager, 2 * 16, y, button_image, button_image2, this);
	this->AddWidget(actor_button);

	tile = tile_manager->get_tile(MD ? 283 : 388); //right arrow icon
	button_image = tmp_screen->create_sdl_surface_from(tile->data, 8, 16, 16, 16);
	button_image2 = tmp_screen->create_sdl_surface_from(tile->data, 8, 16, 16, 16);
	right_button = new GUI_Button(this, 3 * 16, y, button_image, button_image2, this);
	this->AddWidget(right_button);

	if (MD)
		tile = tile_manager->get_tile(285); //combat icon
	else if (Game::get_game()->get_game_type() == NUVIE_GAME_SE)
		tile = tile_manager->get_tile(365); //combat icon
	else
		tile = tile_manager->get_tile(391); //combat icon
	button_image = tmp_screen->create_sdl_surface_from(tile->data, 8, 16, 16, 16);
	button_image2 = tmp_screen->create_sdl_surface_from(tile->data, 8, 16, 16, 16);
	combat_button = new GUI_Button(this, 4 * 16, y, button_image, button_image2, this); //FIX combat
	this->AddWidget(combat_button);

}

void InventoryView::display_inventory_weights() {
	uint8 strength;
	unsigned int inv_weight;
	unsigned int equip_weight;
	Actor *actor;
	if (is_party_member)
		actor = party->get_actor(cur_party_member);
	else if (picking_pocket)
		actor = outside_actor;
	else
		actor = Game::get_game()->get_player()->get_actor();
	char string[9]; //  "E:xx/xxs"

	strength = actor->get_strength();

	ViewManager *vm = Game::get_game()->get_view_manager();
	inv_weight = vm->get_display_weight(actor->get_inventory_weight());
	equip_weight = vm->get_display_weight(actor->get_inventory_equip_weight());

	snprintf(string, 9, "E:%u/%us", equip_weight, strength);
	font->drawString(screen, string, area.left, area.top + 72);

	snprintf(string, 9, "I:%u/%us", inv_weight, strength * 2);
	if (Game::get_game()->get_game_type() == NUVIE_GAME_U6)
		font->drawString(screen, string, area.left + 4 * 16 + 8, area.top + 72);
	else
		font->drawString(screen, string, area.left, area.top + 80);
}

void InventoryView::display_combat_mode() {
	Actor *actor = party->get_actor(cur_party_member);

	uint8 index = get_combat_mode_index(actor);
	if (Game::get_game()->get_game_type() != NUVIE_GAME_U6) {
		int y_off = 96;
		if (MD)
			y_off = 100;
		Tile *tile;

// Avatar combat text background (where command button is for other party members)
		if (actor->get_actor_num() == 1 || actor->get_actor_num() == 0) {
			if (MD)
				tile = tile_manager->get_tile(284);
			else
				tile = tile_manager->get_tile(364);
			screen->blit(area.left + 4 * 16, area.top + y_off, tile->data, 8, 16, 16, 16, true);
		}
		if (MD) // combat text background
			tile = tile_manager->get_tile(286);
		else // SE
			tile = tile_manager->get_tile(366);
		screen->blit(area.left + 5 * 16, area.top + y_off, tile->data, 8, 16, 16, 16, true);
		screen->blit(area.left + 6 * 16, area.top + y_off, tile->data, 8, 16, 16, 16, true); // reuse

		if (MD) // last combat text background
			tile = tile_manager->get_tile(287);
		else
			tile = tile_manager->get_tile(368);
		screen->blit(area.left + 7 * 16, area.top + y_off, tile->data, 8, 16, 16, 16, true);

		if (MD)
			font->drawString(screen, combat_mode_tbl_md[index], area.left + 5 * 16, area.top + 101);
		else
			font->drawString(screen, combat_mode_tbl_se[index], area.left + 5 * 16, area.top + 98);
	} else
		font->drawString(screen, combat_mode_tbl[index], area.left + 5 * 16, area.top + 88);
}

/* Move the cursor around, ready or unready objects, select objects, switch
 * to container view, use command icons.
 */
GUI_status InventoryView::KeyDown(const Common::KeyState &key) {
	if (!show_cursor) // FIXME: don't rely on show_cursor to get/pass focus
		return (GUI_PASS);
	KeyBinder *keybinder = Game::get_game()->get_keybinder();
	ActionType a = keybinder->get_ActionType(key);

	switch (keybinder->GetActionKeyType(a)) {
	//  keypad arrow keys (moveCursorRelative doesn't accept diagonals)
	case SOUTH_WEST_KEY:
		moveCursorRelative(0, 1);
		moveCursorRelative(-1, 0);
		break;
	case SOUTH_EAST_KEY:
		moveCursorRelative(0, 1);
		moveCursorRelative(1, 0);
		break;
	case NORTH_WEST_KEY:
		moveCursorRelative(0, -1);
		moveCursorRelative(-1, 0);
		break;
	case NORTH_EAST_KEY:
		moveCursorRelative(0, -1);
		moveCursorRelative(1, 0);
		break;
	case NORTH_KEY:
		moveCursorRelative(0, -1);
		break;
	case SOUTH_KEY:
		moveCursorRelative(0, 1);
		break;
	case WEST_KEY:
		moveCursorRelative(-1, 0);
		break;
	case EAST_KEY:
		moveCursorRelative(1, 0);
		break;
	case DO_ACTION_KEY:
		select_objAtCursor();
		break;
	case TOGGLE_CURSOR_KEY :
		if (is_party_member) { // when in pickpocket mode we don't want to allow tabing to map window.
			set_show_cursor(false);
			return GUI_PASS;
		}
		break;
	default:
//            set_show_cursor(false); // newAction() can move cursor here
		return GUI_PASS;
	}
	return (GUI_YUM);
}


/* Put cursor over one of the readied-item slots. */
void InventoryView::moveCursorToSlot(uint8 slot_num) {
	cursor_pos.area = INVAREA_DOLL;
	cursor_pos.x = slot_num;
}

/* Put cursor over one of the visible inventory slots. (column inv_x, row inv_y) */
void InventoryView::moveCursorToInventory(uint8 inv_x, uint8 inv_y) {
	cursor_pos.area = INVAREA_LIST;
	cursor_pos.x = inv_x;
	cursor_pos.y = inv_y;
}

/* Put cursor over one of the command icons. */
void InventoryView::moveCursorToButton(uint8 button_num) {
	cursor_pos.area = INVAREA_COMMAND;
	cursor_pos.x = button_num;
}

/* Put cursor over the container or actor icon above the inventory widget. */
void InventoryView::moveCursorToTop() {
	cursor_pos.area = INVAREA_TOP;
}

/* Put cursor over the next slot or icon in relative direction new_x, new_y. */
void InventoryView::moveCursorRelative(sint8 new_x, sint8 new_y) {
	uint32 x = cursor_pos.x, y = cursor_pos.y;
	if (cursor_pos.area == INVAREA_LIST) {
		if (x == 0 && new_x < 0) {
			if (y == 0)
				moveCursorToSlot(2);
			else if (y == 1)
				moveCursorToSlot(4);
			else if (y == 2)
				moveCursorToSlot(6);
			else if (y == 3) {
				if (!picking_pocket)
					moveCursorToButton(3);
			}
		} else if (y == 0 && new_y < 0) {
			if (inventory_widget->up_arrow()) // scroll up
				update_display = true;
			else
				moveCursorToTop(); // move to container icon
		} else if (y == (uint8)(inventory_widget->get_num_rows() - 1) && new_y > 0) {
			if (inventory_widget->down_arrow()) // scroll down
				update_display = true;
			else if (!picking_pocket)
				moveCursorToButton((x == 0) ? 3 : 4); // move to command icon
		} else if ((x + new_x) <= 3)
			moveCursorToInventory(x + new_x, y + new_y);
	} else if (cursor_pos.area == INVAREA_DOLL) {
		// moves from these readied items can jump to inventory list
		if (new_x > 0 && x == 2)
			moveCursorToInventory(0, 0);
		else if (new_x > 0 && x == 4)
			moveCursorToInventory(0, 1);
		else if (new_x > 0 && x == 6)
			moveCursorToInventory(0, 2);
		// moves from these readied items can jump to command icons
		else if (new_y > 0 && x == 5 && !picking_pocket)
			moveCursorToButton(0);
		else if (new_y > 0 && x == 6 && !picking_pocket)
			moveCursorToButton(2);
		else if (new_y > 0 && x == 7 && !picking_pocket)
			moveCursorToButton(1);
		// the rest move between readied items
		else if (x == 0)
			moveCursorToSlot((new_x < 0) ? 1
			                 : (new_x > 0) ? 2
			                 : (new_y > 0) ? 7 : 0);
		else if (x == 7)
			moveCursorToSlot((new_x < 0) ? 5
			                 : (new_x > 0) ? 6
			                 : (new_y < 0) ? 0 : 7);
		else if (x == 1)
			moveCursorToSlot((new_x > 0) ? 0
			                 : (new_y > 0) ? 3 : 1);
		else if (x == 3)
			moveCursorToSlot((new_x > 0) ? 4
			                 : (new_y < 0) ? 1
			                 : (new_y > 0) ? 5 : 3);
		else if (x == 5)
			moveCursorToSlot((new_x > 0) ? 7
			                 : (new_y < 0) ? 3 : 5);
		else if (x == 2)
			moveCursorToSlot((new_x < 0) ? 0
			                 : (new_y > 0) ? 4 : 2);
		else if (x == 4)
			moveCursorToSlot((new_x < 0) ? 3
			                 : (new_y < 0) ? 2
			                 : (new_y > 0) ? 6 : 4);
		else if (x == 6)
			moveCursorToSlot((new_x < 0) ? 7
			                 : (new_y < 0) ? 4 : 6);
	} else if (cursor_pos.area == INVAREA_COMMAND) {
		if (new_y < 0) {
			if (x == 0)
				moveCursorToSlot(5);
			else if (x == 1)
				moveCursorToSlot(7);
			else if (x == 2)
				moveCursorToSlot(6);
			else if (x == 3)
				moveCursorToInventory(0, inventory_widget->get_num_rows() - 1);
			else if (x == 4)
				moveCursorToInventory(1, inventory_widget->get_num_rows() - 1);
		} else if (((sint16)x + new_x) >= 0 && (x + new_x) <= 4)
			moveCursorToButton(x + new_x);
		update_display = true;
	} else if (cursor_pos.area == INVAREA_TOP)
		if (new_y > 0) {
			moveCursorToInventory(cursor_pos.x, 0);
			update_display = true;
		}
	update_cursor();
}


/* Update on-screen location (px,py) of cursor.
 */
void InventoryView::update_cursor() {
	Common::Rect *ready_loc;
	nuvie_game_t gametype = Game::get_game()->get_game_type();
	switch (cursor_pos.area) {
	case INVAREA_LIST:
		if (gametype == NUVIE_GAME_U6) {
			cursor_pos.px = area.left + (4 * 16 + 8) + cursor_pos.x * 16;
		} else {
			cursor_pos.px = inventory_widget->area.left + cursor_pos.x * 16;
		}
		cursor_pos.py = area.top + 16 + 8 + cursor_pos.y * 16;
		break;
	case INVAREA_TOP:
		cursor_pos.px = inventory_widget->area.left + (gametype == NUVIE_GAME_U6 ? 32 : (inventory_widget->area.width() - 16) / 2);
		cursor_pos.py = inventory_widget->area.top;
		break;
	case INVAREA_DOLL:
		ready_loc = doll_widget->get_item_hit_rect(cursor_pos.x);
		cursor_pos.px = ready_loc->left + doll_widget->area.left;
		cursor_pos.py = ready_loc->top + doll_widget->area.top;
		break;
	case INVAREA_COMMAND:
		cursor_pos.px = ((cursor_pos.x + 1) * 16) - 16;
		cursor_pos.py = left_button->area.top; //80;
		cursor_pos.px += area.left;
		//cursor_pos.py += area.top;
		break;
	}
}

void InventoryView::set_show_cursor(bool state) {

	show_cursor = state;
	update_display = true;
	ViewManager *vm = Game::get_game()->get_view_manager();
	if (state == true && vm->get_current_view() != this) // second comparison prevents endless loop
		vm->set_inventory_mode();

}

void InventoryView::hide_buttons() {
	if (left_button) left_button->Hide();
	if (right_button) right_button->Hide();
//	if(actor_button) actor_button->Hide();
	if (party_button) party_button->Hide();
	if (combat_button) combat_button->Hide();
}

void InventoryView::show_buttons() {
//	if(left_button) left_button->Show(); //   these two shouldn't be needed
//	if(right_button) right_button->Show(); // and cause problems
	if (actor_button) actor_button->Show();
	if (party_button) party_button->Show();
	if (combat_button) combat_button->Show();
}

/* Returns pointer to object at cursor position, or NULL.
 */
Obj *InventoryView::get_objAtCursor() {
	// emulate mouse; use center of cursor
	uint32 hit_x = cursor_pos.px + 8 - inventory_widget->area.left,
	       hit_y = cursor_pos.py + 8 - inventory_widget->area.top;
	if (cursor_pos.area == INVAREA_LIST)
		return (inventory_widget->get_obj_at_location(hit_x, hit_y));
	else if (cursor_pos.area == INVAREA_DOLL)
		return (inventory_widget->get_actor()->inventory_get_readied_object(cursor_pos.x));

	return (NULL);
}


/* Do an action with the object under the cursor, or call the function for a
   selected button. This is called when pressing ENTER. */
void InventoryView::select_objAtCursor() {
	Events *event = Game::get_game()->get_event();
	ViewManager *view_manager = Game::get_game()->get_view_manager();
	Obj *obj = get_objAtCursor();

	if (cursor_pos.area == INVAREA_TOP && event->can_target_icon()) {
		if (inventory_widget->is_showing_container() && event->get_last_mode() != PUSH_MODE)
			select_obj((Obj *)inventory_widget->get_container());
		else if (inventory_widget->is_showing_container()
		         &&  inventory_widget->get_container()->get_engine_loc() == OBJ_LOC_CONT)
			select_obj((Obj *)inventory_widget->get_container()->parent);
		else
			event->select_actor(inventory_widget->get_actor());
		return;
	}

	if (is_party_member) {
		// special areas
		if (cursor_pos.area == INVAREA_COMMAND) {
			if (cursor_pos.x == 0) // left
				View::callback(BUTTON_CB, left_button, view_manager);
			if (cursor_pos.x == 1) // party
				View::callback(BUTTON_CB, party_button, view_manager);
			if (cursor_pos.x == 2) // status
				View::callback(BUTTON_CB, actor_button, view_manager);
			if (cursor_pos.x == 3) // right
				View::callback(BUTTON_CB, right_button, view_manager);
			if (cursor_pos.x == 4) // strategy
				callback(BUTTON_CB, combat_button, view_manager);
			return;
		} else if (cursor_pos.area == INVAREA_TOP) {
			if (inventory_widget->is_showing_container())
				inventory_widget->set_prev_container();
			else
				Game::get_game()->get_view_manager()->set_party_mode();

			return;
		}
	}

	if (cursor_pos.area == INVAREA_DOLL || cursor_pos.area == INVAREA_LIST)
		select_obj(obj); // do action with an object
}


/* Ready an object or pass it to Events. Pass NULL if an empty space is selected.
 * Returns true if the object was "used". The caller is free to handle the
 * object if false is returned.
 */
bool InventoryView::select_obj(Obj *obj) {
	Events *event = Game::get_game()->get_event();
	switch (event->get_mode()) {
	case MOVE_MODE:
	case EQUIP_MODE: {
		if (!obj)
			break;
		bool locked_chest = (Game::get_game()->get_usecode()->is_chest(obj) && obj->frame_n > 1);
		if (Game::get_game()->get_usecode()->is_container(obj) && !locked_chest)
			inventory_widget->set_container(obj);
		else {
			if (obj->is_readied())
				return event->unready(obj);
			else
				return event->ready(obj, inventory_widget->get_actor());
		}
		break;
	}
	case ATTACK_MODE:
		set_show_cursor(false);
		event->cancelAction();
		return false;
	default:
		event->select_view_obj(obj, inventory_widget->get_actor());
		return true;
	}
	return false;
}

GUI_status InventoryView::MouseDown(int x, int y, Shared::MouseButton button) {
	return GUI_PASS;
}

GUI_status InventoryView::MouseWheel(sint32 x, sint32 y) {

	if (!is_party_member)
		return GUI_PASS;

	int xpos, ypos;
	screen->get_mouse_location(&xpos, &ypos);

	xpos -= area.left;
	ypos -= area.top;
	bool wheel_range = (xpos >= 0 && ypos < area.top + area.height() - 6);

	if (y > 0 && wheel_range) {
		View::callback(BUTTON_CB, left_button, Game::get_game()->get_view_manager());
		return GUI_YUM;
	} else if (y < 0 && wheel_range) {
		View::callback(BUTTON_CB, right_button, Game::get_game()->get_view_manager());
		return GUI_YUM;
	}
	return GUI_PASS; // goes to MsgScroll
}

/* Messages from child widgets, Inventory & Doll.
 **INVSELECT is called when an object is selected with MouseDown.*
 **BUTTON is called when one of the command buttons is selected.*
 * Returns GUI_PASS if the data was not used.
 */
GUI_status InventoryView::callback(uint16 msg, GUI_CallBack *caller, void *data) {
	if (msg != INVSELECT_CB) { // hit one of the command buttons
		if (caller == combat_button) {
			if (cur_party_member != 0) { // You can't change combat modes for the avatar.
				Actor *actor = party->get_actor(cur_party_member);
				set_combat_mode(actor);
				update_display = true;
			}

			return GUI_YUM;
		} else
			return View::callback(msg, caller, data);
	}

	// selecting an object from InventoryWidget only works while getting input
	Events *event = Game::get_game()->get_event();
	if (event->get_mode() == INPUT_MODE) {
		if (select_obj((Obj *)data))
			return GUI_YUM;
	}
	return GUI_PASS;
}

void InventoryView::simulate_CB_callback() {
	callback(BUTTON_CB, combat_button, Game::get_game()->get_view_manager());
}

} // End of namespace Nuvie
} // End of namespace Ultima
