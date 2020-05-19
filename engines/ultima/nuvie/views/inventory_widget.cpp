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
#include "ultima/nuvie/misc/u6_llist.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/gui/gui.h"
#include "ultima/nuvie/screen/game_palette.h"
#include "ultima/nuvie/views/inventory_widget.h"
#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/fonts/font.h"
#include "ultima/nuvie/core/game_clock.h"
#include "ultima/nuvie/core/events.h"
#include "ultima/nuvie/gui/widgets/msg_scroll.h"
#include "ultima/nuvie/core/timed_event.h"
#include "ultima/nuvie/usecode/usecode.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/core/player.h"
#include "ultima/nuvie/gui/widgets/command_bar.h"
#include "ultima/nuvie/views/inventory_font.h"
#include "ultima/nuvie/views/view_manager.h"

namespace Ultima {
namespace Nuvie {

InventoryWidget::InventoryWidget(Configuration *cfg, GUI_CallBack *callback): GUI_Widget(NULL, 0, 0, 0, 0) {
	config = cfg;
	callback_object = callback;

	container_obj = NULL;
	tile_manager = NULL;
	obj_manager = NULL;
	selected_obj = NULL;
	font = NULL;
	actor = NULL;
	target_obj = NULL;
	target_cont = NULL;
	empty_tile = NULL;
	ready_obj = NULL; // FIXME: this is unused but I might need it again -- SB-X
	row_offset = 0;

	config->value("config/GameType", game_type);

	arrow_rects[0] = Common::Rect(0, 16, 0 + 8, 16 + 8);
	arrow_rects[1] = Common::Rect(0, 3 * 16 + 8, 0 + 8, 3 * 16 + 8 + 8);
}

InventoryWidget::~InventoryWidget() {

}

bool InventoryWidget::init(Actor *a, uint16 x, uint16 y, TileManager *tm, ObjManager *om, Font *f) {
	tile_manager = tm;
	obj_manager = om;
	font = f;

	bg_color = Game::get_game()->get_palette()->get_bg_color();
	obj_font_color = 0;

	if (Game::get_game()->get_game_type() == NUVIE_GAME_U6) {
		icon_x = 32;
		obj_font_color = 0x48;
		objlist_offset_x = 8;
	} else {
		icon_x = 23;
		objlist_offset_x = 0;
	}
	objlist_offset_y = 16;

	if (Game::get_game()->get_game_type() == NUVIE_GAME_U6) {
		empty_tile = tile_manager->get_tile(410);
		GUI_Widget::Init(NULL, x, y, 72, 64); //72 =  4 * 16 + 8
	} else if (Game::get_game()->get_game_type() == NUVIE_GAME_MD) { // FIXME: different depending on npc
		empty_tile = tile_manager->get_tile(273);
		GUI_Widget::Init(NULL, x, y, 64, 82);
	} else { // SE
		empty_tile = tile_manager->get_tile(392);
		GUI_Widget::Init(NULL, x + 2, y, 64, 82);
	}

	set_actor(a);
	set_accept_mouseclick(true, USE_BUTTON); // accept [double]clicks from button1 (even if double-click disabled we need clicks)

	return true;
}

void InventoryWidget::set_actor(Actor *a) {
	actor = a;
	container_obj = NULL;
	Redraw();
}

Obj *InventoryWidget::get_prev_container() {
	if (container_obj && container_obj->get_engine_loc() == OBJ_LOC_CONT)
		return (Obj *)container_obj->parent;

	return NULL;
}

void InventoryWidget::set_prev_container() {
	if (!container_obj)
		return;

	if (container_obj->get_engine_loc() == OBJ_LOC_CONT)
		set_container((Obj *)container_obj->parent);
	else
		set_container(NULL);

	return;
}

void InventoryWidget::Display(bool full_redraw) {
	if (full_redraw || update_display) {
//   screen->fill(bg_color, area.left, area.top, area.width(), area.height());
		display_inventory_container();
		if (Game::get_game()->get_game_type() == NUVIE_GAME_U6)
			display_arrows();
	}
	//screen->blit(area.left+40,area.top+16,portrait_data,8,56,64,56,false);

//clear the screen first inventory icons, 4 x 3 tiles
// screen->fill(bg_color, area.left +objlist_offset_x, area.top + objlist_offset_y, 16 * 4, 16 * 3); // doesn't seem to be needed
	display_inventory_list();

	if (full_redraw || update_display) {
		update_display = false;
		screen->update(area.left, area.top, area.width(), area.height());
	} else {
		screen->update(area.left + objlist_offset_x, area.top + 16, area.width() - objlist_offset_x, area.height() - 16); // update only the inventory list
	}

}

//either an Actor or an object container.
void InventoryWidget::display_inventory_container() {
	Tile *tile;
	if (!container_obj) //display actor
		tile = tile_manager->get_tile(actor->get_downward_facing_tile_num());
	else // display container object
		tile = tile_manager->get_tile(obj_manager->get_obj_tile_num(container_obj) + container_obj->frame_n);

	screen->blit(area.left + icon_x, area.top, tile->data, 8, 16, 16, 16, true);

	return;
}

void InventoryWidget::display_inventory_list() {
	const Tile *tile;
	U6LList *inventory;
	U6Link *link;
	Obj *obj = NULL;
	uint16 i, j;
	uint16 skip_num;
	int max_rows = 4;
	if (Game::get_game()->get_game_type() == NUVIE_GAME_U6)
		max_rows = 3;

	if (container_obj)
		inventory = container_obj->container;
	else
		inventory = actor->get_inventory_list();
	if (inventory == NULL)
		link = NULL;
	else
		link = inventory->start();

//skip row_offset rows of objects.
	skip_num = row_offset * 4;
	for (i = 0; link != NULL && i < skip_num; link = link->next) {
		obj = (Obj *)link->data;
		if (obj->is_readied() == false)
			i++;
	}





	for (i = 0; i < max_rows; i++) {
		for (j = 0; j < 4; j++) {
			if (link != NULL) {
				obj = (Obj *)link->data;
				if (obj->is_readied()) { //skip any readied objects
					for (; link != NULL && obj->is_readied(); link = link->next)
						obj = (Obj *)link->data;
				} else
					link = link->next;

				tile = tile_manager->get_tile(obj_manager->get_obj_tile_num(obj) + obj->frame_n);
				if (link == NULL) {
					if (obj->is_readied()) //last object is readied so skip it.
						tile = empty_tile;
				}
			} else
				tile = empty_tile;

			//tile = tile_manager->get_tile(actor->indentory_tile());
			if (tile == empty_tile)
				screen->blit((area.left + objlist_offset_x) + j * 16, area.top + objlist_offset_y + i * 16, (const unsigned char *)empty_tile->data, 8, 16, 16, 16, true);
			if (tile != empty_tile) {
				//draw qty string for stackable items
				if (obj_manager->is_stackable(obj))
					display_qty_string((area.left + objlist_offset_x) + j * 16, area.top + objlist_offset_y + i * 16, obj->qty);

				//draw special char for Keys.
				if (game_type == NUVIE_GAME_U6 && obj->obj_n == 64)
					display_special_char((area.left + objlist_offset_x) + j * 16, area.top + objlist_offset_y + i * 16, obj->quality);
			}

			screen->blit((area.left + objlist_offset_x) + j * 16, area.top + objlist_offset_y + i * 16, (const unsigned char *)tile->data, 8, 16, 16, 16, true);
		}
	}
}

void InventoryWidget::display_qty_string(uint16 x, uint16 y, uint16 qty) {
	uint8 len, i, offset;
	char buf[6];

	sprintf(buf, "%d", qty);
	len = strlen(buf);

	offset = (16 - len * 4) / 2;

	for (i = 0; i < len; i++)
		screen->blitbitmap(x + offset + 4 * i, y + 11, inventory_font[buf[i] - 48], 3, 5, obj_font_color, bg_color);

	return;
}

void InventoryWidget::display_special_char(uint16 x, uint16 y, uint8 quality) {
	if (quality + 9 >= NUVIE_MICRO_FONT_COUNT)
		return;

	screen->blitbitmap(x + 6, y + 11, inventory_font[quality + 9], 3, 5, obj_font_color, bg_color);
}

void InventoryWidget::display_arrows() {
	uint32 num_objects;

	if (is_showing_container()) {
		if (container_obj->container)
			num_objects = container_obj->container->count();
		else
			num_objects = 0;
	} else
		num_objects = actor->inventory_count_objects(false);

	if (num_objects <= 12) //reset row_offset if we only have one page of objects
		row_offset = 0;

	if (row_offset > 0) //display top arrow
		font->drawChar(screen, 24, area.left, area.top + 16);

	if (num_objects - row_offset * 4 > 12) //display bottom arrow
		font->drawChar(screen, 25, area.left, area.top + 3 * 16 + 8);
}

GUI_status InventoryWidget::MouseDown(int x, int y, Shared::MouseButton button) {
	Events *event = Game::get_game()->get_event();
	CommandBar *command_bar = Game::get_game()->get_command_bar();
	x -= area.left;
	y -= area.top;

	if (y < 17)
		return GUI_PASS;

	Obj *obj = get_obj_at_location(x, y);

	if (button == ACTION_BUTTON && event->get_mode() == MOVE_MODE
	        && command_bar->get_selected_action() > 0) { // Exclude attack mode too
		if (command_bar->try_selected_action() == false) // start new action
			return GUI_PASS; // false if new event doesn't need target
	}
	if (button == ACTION_BUTTON && command_bar->get_selected_action() > 0
	        && event->get_mode() == INPUT_MODE) {
		if (obj)
			event->select_obj(obj); // the returned location
		else {
			Game::get_game()->get_scroll()->display_string("nothing!\n");
			event->endAction(true);
			event->set_mode(MOVE_MODE);
		}
		return  GUI_PASS;
	}

// ABOEING
	if (actor && (button == USE_BUTTON || button == ACTION_BUTTON || button == DRAG_BUTTON)) {
		if (obj) { // FIXME: duplicating code in DollWidget
			// send to View
			if (callback_object->callback(INVSELECT_CB, this, obj) == GUI_PASS
			        && button == DRAG_BUTTON)
				selected_obj = obj; // start dragging
			return GUI_YUM;
		}
	}
	return GUI_PASS;
}

inline uint16 InventoryWidget::get_list_position(int x, int y) {
	uint16 list_pos;

	list_pos = ((y - objlist_offset_y) / 16) * 4 + (x - objlist_offset_x) / 16;
	list_pos += row_offset * 4;

	return list_pos;
}

Obj *InventoryWidget::get_obj_at_location(int x, int y) {
	uint8 location;
	U6LList *inventory;
	U6Link *link;
	Obj *obj =  NULL;
	uint16 i;

	if (x >= objlist_offset_x && y >= objlist_offset_y) {
		location = get_list_position(x, y); //find the postion of the object we hit in the inventory

		if (container_obj)
			inventory = container_obj->container;
		else
			inventory = actor->get_inventory_list();
		if (inventory == NULL)
			link = NULL;
		else
			link = inventory->start();

		for (i = 0; link != NULL && i <= location; link = link->next) {
			obj = (Obj *)link->data;
			if (obj->is_readied() == false)
				i++;
		}

		if (i > location && obj && obj->is_readied() == false) // don't return readied or non existent objects
			return obj;
	}

	return NULL;
}

GUI_status InventoryWidget::MouseWheel(sint32 x, sint32 y) {
	int xpos, ypos;
	screen->get_mouse_location(&xpos, &ypos);

	xpos -= area.left;
	ypos -= area.top;
	if (xpos < 0 || ypos > area.top + area.height() - 10)
		return GUI_PASS; // goes to InventoryView

	if (Game::get_game()->get_game_type() == NUVIE_GAME_U6) {
		if (y > 0) {
			up_arrow();
		}
		if (y < 0) {
			down_arrow();
		}

		selected_obj = NULL;
	}
	return GUI_YUM;
}

// change container, ready/unready object, activate arrows
GUI_status InventoryWidget::MouseUp(int x, int y, Shared::MouseButton button) {

	CommandBar *command_bar = Game::get_game()->get_command_bar();

	if (button == USE_BUTTON || (button == ACTION_BUTTON
	                             && command_bar->get_selected_action() > 0)) { // Exclude attack mode too
		x -= area.left;
		y -= area.top;
		if (x >= icon_x && x <= icon_x + 15 && // hit top icon either actor or container
		        y >= 0 && y <= 15) {
			Events *event = Game::get_game()->get_event();

			if (button == ACTION_BUTTON && event->get_mode() == MOVE_MODE) {
				if (command_bar->try_selected_action() == false) // start new action
					return GUI_PASS; // false if new event doesn't need target
			}

			if (event->can_target_icon()) {
				if (is_showing_container() && event->get_last_mode() != PUSH_MODE)
					event->select_obj((Obj *)container_obj, actor);
				else if (is_showing_container() && get_container()->get_engine_loc() == OBJ_LOC_CONT)
					event->select_obj((Obj *)get_container()->parent, actor);
				else
					event->select_actor(actor);
				return GUI_YUM;
			}

			if (is_showing_container())
				set_prev_container(); //return to previous container or main actor inventory
			else if (!event->using_control_cheat())
				Game::get_game()->get_view_manager()->set_party_mode();

			Redraw();
		}
		if (Game::get_game()->get_game_type() == NUVIE_GAME_U6) {
			if (HitRect(x, y, arrow_rects[0])) //up arrow hit rect
				up_arrow();
			else if (HitRect(x, y, arrow_rects[1])) //down arrow hit rect
				down_arrow();
		}

		// only act now if objects can't be used with DoubleClick
		if (selected_obj && !Game::get_game()->get_map_window()->is_doubleclick_enabled())
			try_click();
		else if (selected_obj) {
			wait_for_mouseclick(USE_BUTTON);
			ready_obj = selected_obj;
		}
	}

	selected_obj = NULL;

	return GUI_YUM;
}

bool InventoryWidget::up_arrow() {
	if (row_offset > 0) {
		row_offset--;
		Redraw();
		return true;
	}

	return false;
}

bool InventoryWidget::down_arrow() {
	uint32 num_objects;
	if (container_obj)
		num_objects = container_obj->container_count_objects();
	else
		num_objects = actor->inventory_count_objects(false);

	if (num_objects - row_offset * 4 > 12) {
		row_offset++;
		Redraw();
		return true;
	}

	return false;
}

GUI_status InventoryWidget::MouseMotion(int x, int y, uint8 state) {
	Tile *tile;

	if (selected_obj && !dragging && Game::get_game()->is_dragging_enabled()) {
		dragging = true;
		tile = tile_manager->get_tile(obj_manager->get_obj_tile_num(selected_obj) + selected_obj->frame_n);
		return gui_drag_manager->start_drag(this, GUI_DRAG_OBJ, selected_obj, tile->data, 16, 16, 8);
	}

	return GUI_PASS;
}

void InventoryWidget::drag_drop_success(int x, int y, int message, void *data) {
	DEBUG(0, LEVEL_DEBUGGING, "InventoryWidget::drag_drop_success()\n");
	dragging = false;

// handled by drop target
// if(container_obj)
//   container_obj->container->remove(selected_obj);
// else
//   actor->inventory_remove_obj(selected_obj);

	selected_obj = NULL;
	Redraw();
}

void InventoryWidget::drag_drop_failed(int x, int y, int message, void *data) {
	DEBUG(0, LEVEL_DEBUGGING, "InventoryWidget::drag_drop_failed()\n");
	dragging = false;
	selected_obj = NULL;
}

bool InventoryWidget::drag_set_target_obj(int x, int y) {
	if (x >= 32 && x < 48 && y < 16) {
		target_obj = NULL;
		target_cont = get_prev_container(); //returns parent container or NULL if we're back at the inventory.
	} else if (x >= objlist_offset_x && y >= objlist_offset_y) {
		target_obj = get_obj_at_location(x, y);
		target_cont = get_container();
	} else {
		return false;
	}

	return true;
}

bool InventoryWidget::drag_accept_drop(int x, int y, int message, void *data) {
	GUI::get_gui()->force_full_redraw();
	DEBUG(0, LEVEL_DEBUGGING, "InventoryWidget::drag_accept_drop()\n");
	if (message == GUI_DRAG_OBJ) {
		Obj *obj = (Obj *)data;
		x -= area.left;
		y -= area.top;
		if (target_obj == NULL) { //we need to check this so we don't screw up target_obj on subsequent calls
			if (drag_set_target_obj(x, y) == false) {
				DEBUG(0, LEVEL_WARNING, "InventoryWidget: Didn't hit any widget object targets!\n");
				return false;
			}
		}
		Actor *src_actor = Game::get_game()->get_player()->get_actor();

		if (obj->get_actor_holding_obj() == actor)
			src_actor = actor;
		else if (!obj->is_in_inventory() && actor == Game::get_game()->get_player()->get_actor()) {
			Game::get_game()->get_scroll()->display_string("Get-");
			Game::get_game()->get_scroll()->display_string(obj_manager->look_obj(obj, OBJ_SHOW_PREFIX));
		} else
			Game::get_game()->get_event()->display_move_text(actor, obj);

		if (!obj->is_in_inventory()
		        && !Game::get_game()->get_map_window()->can_get_obj(actor, obj)) {
			Game::get_game()->get_scroll()->message("\n\nblocked\n\n");
			return false;
		}
		if ((Game::get_game()->get_usecode()->has_getcode(obj)
		        && !Game::get_game()->get_usecode()->get_obj(obj, actor))
		        || !Game::get_game()->get_event()->can_move_obj_between_actors(obj, src_actor, actor)) {
			Game::get_game()->get_scroll()->message("\n\n");
			return false;
		} else if (!obj->is_in_inventory()
		           && obj_manager->obj_is_damaging(obj, Game::get_game()->get_player()->get_actor())) {
			Game::get_game()->get_player()->subtract_movement_points(3);
			return false;
		} else if (src_actor != actor || !obj->is_in_inventory())
			Game::get_game()->get_scroll()->message("\n\n");

		if (src_actor != actor) // get plus move
			Game::get_game()->get_player()->subtract_movement_points(8);
		else if (!obj->is_in_inventory()) // get
			Game::get_game()->get_player()->subtract_movement_points(3);

		UseCode *usecode = Game::get_game()->get_usecode();
		if (usecode->is_chest(obj) && obj->frame_n == 0) //open chest
			obj->frame_n = 1; //close the chest

		DEBUG(0, LEVEL_DEBUGGING, "Drop Accepted\n");
		return true;
	}

	DEBUG(0, LEVEL_DEBUGGING, "Drop Refused\n");
	return false;
}

void InventoryWidget::drag_perform_drop(int x, int y, int message, void *data) {
	DEBUG(0, LEVEL_DEBUGGING, "InventoryWidget::drag_perform_drop()\n");
	Obj *obj;

	x -= area.left;
	y -= area.top;

	if (message == GUI_DRAG_OBJ) {
		DEBUG(0, LEVEL_DEBUGGING, "Drop into inventory.\n");
		obj = (Obj *)data;

		if (target_obj && obj_manager->can_store_obj(target_obj, obj)) {
			obj_manager->moveto_container(obj, target_obj);
		} else if (target_cont && obj_manager->can_store_obj(target_cont, obj)) {
			obj_manager->moveto_container(obj, target_cont);
		} else {
			if (obj->is_readied())
				Game::get_game()->get_event()->unready(obj);
			else
				obj_manager->moveto_inventory(obj, actor);
		}

		Redraw();
	}

	Game::get_game()->get_map_window()->updateBlacking();
	target_obj = NULL;

	return;
}

void InventoryWidget::drag_draw(int x, int y, int message, void *data) {
	Tile *tile;

	if (!selected_obj)
		return;

	tile = tile_manager->get_tile(obj_manager->get_obj_tile_num(selected_obj) + selected_obj->frame_n);

	int nx = x - 8;
	int ny = y - 8;

	if (nx + 16 >= screen->get_width())
		nx = screen->get_width() - 17;
	else if (nx < 0)
		nx = 0;

	if (ny + 16 >= screen->get_height())
		ny = screen->get_height() - 17;
	else if (ny < 0)
		ny = 0;

	screen->blit(nx, ny, tile->data, 8, 16, 16, 16, true);
	screen->update(nx, ny, 16, 16);
}

void InventoryWidget::try_click() {
	Events *event = Game::get_game()->get_event();
	UseCode *usecode = Game::get_game()->get_usecode();
	if (!selected_obj)
		selected_obj = ready_obj;
	if (!selected_obj)
		return;
	bool locked_chest = (usecode->is_chest(selected_obj) && selected_obj->frame_n > 1);
	if (event->get_mode() == ATTACK_MODE)
		event->cancelAction();
	else if (usecode->is_container(selected_obj) && !locked_chest) { // open up the container.
		container_obj = selected_obj;
		if (usecode->is_chest(container_obj) && selected_obj->frame_n == 1)
			usecode->process_effects(container_obj, actor);
		Redraw();
	} else { // attempt to ready selected object.
		event->ready(selected_obj, actor);
		Redraw();
	}
	ready_obj = NULL;
	selected_obj = NULL;
}

/* Use object. */
GUI_status InventoryWidget::MouseDouble(int x, int y, Shared::MouseButton button) {
	// we have to check if double-clicks are allowed here, since we use single-clicks
	if (!Game::get_game()->get_map_window()->is_doubleclick_enabled())
		return (GUI_PASS);
	Obj *obj = selected_obj;

	ready_obj = NULL;
	selected_obj = NULL;

	if (!actor)
		return (GUI_YUM);
	if (!obj)
		return (MouseUp(x, y, button)); // probably hit an arrow

	Game::get_game()->get_view_manager()->double_click_obj(obj);
	return (GUI_PASS);
}

GUI_status InventoryWidget::MouseClick(int x, int y, Shared::MouseButton button) {
	return (MouseUp(x, y, button));
}

// change container, ready/unready object, activate arrows
GUI_status InventoryWidget::MouseDelayed(int x, int y, Shared::MouseButton button) {
	if (ready_obj)
		try_click();
	return GUI_PASS;
}

} // End of namespace Nuvie
} // End of namespace Ultima
