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
#include "ultima/nuvie/views/container_widget.h"
#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/core/game_clock.h"
#include "ultima/nuvie/core/events.h"
#include "ultima/nuvie/gui/widgets/msg_scroll.h"
#include "ultima/nuvie/core/timed_event.h"
#include "ultima/nuvie/usecode/usecode.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/core/player.h"
#include "ultima/nuvie/actors/actor_manager.h"
#include "ultima/nuvie/script/script.h"
#include "ultima/nuvie/views/inventory_font.h"
#include "ultima/nuvie/views/view_manager.h"

namespace Ultima {
namespace Nuvie {

ContainerWidget::ContainerWidget(Configuration *cfg, GUI_CallBack *callback): GUI_Widget(NULL, 0, 0, 0, 0) {
	config = cfg;
	callback_object = callback;

	container_obj = NULL;
	tile_manager = NULL;
	obj_manager = NULL;
	selected_obj = NULL;
	target_cont = NULL;
	actor = NULL;
	target_obj = NULL;
	fill_bg = false;
	empty_tile = NULL;
	ready_obj = NULL; // FIXME: this is unused but I might need it again -- SB-X
	row_offset = 0;

	config->value("config/GameType", game_type);
}

ContainerWidget::~ContainerWidget() {

}

bool ContainerWidget::init(Actor *a, uint16 x, uint16 y, TileManager *tm, ObjManager *om, Font *f) {
	tile_manager = tm;
	obj_manager = om;

	rows = CONTAINER_WIDGET_ROWS;
	cols = CONTAINER_WIDGET_COLS;

	bg_color = Game::get_game()->get_palette()->get_bg_color();
	if (Game::get_game()->get_game_type() == NUVIE_GAME_U6)
		obj_font_color = 0x48;
	else
		obj_font_color = 0;

	fill_bg = true;

	if (Game::get_game()->get_game_type() == NUVIE_GAME_U6)
		empty_tile = tile_manager->get_tile(410);
	else if (Game::get_game()->get_game_type() == NUVIE_GAME_MD) // FIXME: different depending on npc
		empty_tile = tile_manager->get_tile(273);
	else
		empty_tile = tile_manager->get_tile(392);

	GUI_Widget::Init(NULL, x, y, cols * 16 + 8, (rows + 1) * 16);

	set_actor(a);
	set_accept_mouseclick(true, USE_BUTTON); // accept [double]clicks from button1 (even if double-click disabled we need clicks)

	return true;
}

void ContainerWidget::set_actor(Actor *a) {
	actor = a;
	container_obj = NULL;
	row_offset = 0;
	Redraw();
}

void ContainerWidget::Display(bool full_redraw) {
	if (fill_bg) {
		//clear the screen first inventory icons, 4 x 3 tiles
		screen->fill(bg_color, area.left, area.top, area.width(), area.height());
	}
	display_inventory_list();

	screen->update(area.left, area.top, area.width(), area.height());
}


void ContainerWidget::display_inventory_list() {
	const Tile *tile;
	U6LList *objlist;
	U6Link *link;
	Obj *obj = NULL;
	uint16 i, j;
	uint16 skip_num;

	if (container_obj)
		objlist = container_obj->container;
	else
		objlist = actor->get_inventory_list();
	if (objlist == NULL)
		link = NULL;
	else
		link = objlist->start();

//skip row_offset rows of objects.
	skip_num = row_offset * cols;
	for (i = 0; link != NULL && i < skip_num; link = link->next) {
		obj = (Obj *)link->data;
		if (obj->is_readied() == false)
			i++;
	}

	for (i = 0; i < rows; i++) {
		for (j = 0; j < cols; j++) {
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

			screen->blit(area.left + j * 16, area.top + i * 16, (const unsigned char *)empty_tile->data, 8, 16, 16, 16, true);
			if (tile != empty_tile) {
				//draw qty string for stackable items
				if (obj_manager->is_stackable(obj))
					display_qty_string(area.left + j * 16, area.top + i * 16, obj->qty);

				//draw special char for Keys.
				if (game_type == NUVIE_GAME_U6 && obj->obj_n == 64)
					display_special_char(area.left + j * 16, area.top + i * 16, obj->quality);
			}

			screen->blit(area.left + j * 16, area.top + i * 16, (const unsigned char *)tile->data, 8, 16, 16, 16, true);
		}
	}
}

void ContainerWidget::display_qty_string(uint16 x, uint16 y, uint16 qty) {
	uint8 len, i, offset;
	char buf[6];

	sprintf(buf, "%d", qty);
	len = strlen(buf);

	offset = (16 - len * 4) / 2;

	for (i = 0; i < len; i++)
		screen->blitbitmap(x + offset + 4 * i, y + 11, inventory_font[buf[i] - 48], 3, 5, obj_font_color, bg_color);

	return;
}

void ContainerWidget::display_special_char(uint16 x, uint16 y, uint8 quality) {
	if (quality + 9 >= NUVIE_MICRO_FONT_COUNT)
		return;

	screen->blitbitmap(x + 6, y + 11, inventory_font[quality + 9], 3, 5, obj_font_color, bg_color);
}

GUI_status ContainerWidget::MouseDown(int x, int y, Shared::MouseButton button) {
//Events *event = Game::get_game()->get_event();
//MsgScroll *scroll = Game::get_game()->get_scroll();
	x -= area.left;
	y -= area.top;

// ABOEING
	if (/*actor && */(button == USE_BUTTON || button == ACTION_BUTTON || button == DRAG_BUTTON)) {
		Obj *obj; // FIXME: duplicating code in DollWidget
		if ((obj = get_obj_at_location(x, y)) != NULL) {
			// send to View
			if (callback_object->callback(INVSELECT_CB, this, obj) == GUI_PASS
			        && button == DRAG_BUTTON)
				selected_obj = obj; // start dragging
			return GUI_YUM;
		}
	}

	return GUI_PASS;
}

inline uint16 ContainerWidget::get_list_position(int x, int y) {
	uint16 list_pos;

	list_pos = (y / 16) * cols + x / 16;
	list_pos += row_offset * cols;

	return list_pos;
}

Obj *ContainerWidget::get_obj_at_location(int x, int y) {
	uint8 location;
	U6LList *inventory;
	U6Link *link;
	Obj *obj =  NULL;
	uint16 i;


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

	return NULL;
}

// change container, ready/unready object, activate arrows
GUI_status ContainerWidget::MouseUp(int x, int y, Shared::MouseButton button) {
	if (button == USE_BUTTON) {
		x -= area.left;
		y -= area.top;

		if (selected_obj) {
			// only act now if objects can't be used with DoubleClick
			if (!Game::get_game()->get_map_window()->is_doubleclick_enabled())
				try_click();
			else {
				wait_for_mouseclick(USE_BUTTON);
				ready_obj = selected_obj;
			}

			selected_obj = NULL;

			return GUI_YUM;
		}
	}

	return GUI_PASS;
}

bool ContainerWidget::up_arrow() {
	if (row_offset > 0) {
		row_offset--;
		return true;
	}

	return false;
}

bool ContainerWidget::down_arrow() {
	uint32 num_objects;

	if (container_obj)
		num_objects = container_obj->container_count_objects();
	else
		num_objects = actor->inventory_count_objects(false);

	if ((sint32)(num_objects - row_offset * cols) > (sint32)(rows * cols)) {
		row_offset++;
		return true;
	}

	return false;
}

GUI_status ContainerWidget::MouseMotion(int x, int y, uint8 state) {
	Tile *tile;

	if (selected_obj && !dragging) {
		dragging = true;
		tile = tile_manager->get_tile(obj_manager->get_obj_tile_num(selected_obj->obj_n) + selected_obj->frame_n);
		bool out_of_range = false;
		if (!selected_obj->is_in_inventory() && Game::get_game()->get_map_window()->is_interface_fullscreen_in_combat()) {
			Obj *obj = selected_obj->is_in_container() ? selected_obj->get_container_obj(true) : selected_obj;
			MapCoord obj_loc(obj->x, obj->y, obj->z);
			Actor *player = Game::get_game()->get_player()->get_actor();
			if (player->get_location().distance(obj_loc) > 1)
				out_of_range = true;
		}
		return gui_drag_manager->start_drag(this, GUI_DRAG_OBJ, selected_obj, tile->data, 16, 16, 8, out_of_range);
	}

	return GUI_PASS;
}

void ContainerWidget::drag_drop_success(int x, int y, int message, void *data) {
	DEBUG(0, LEVEL_DEBUGGING, "ContainerWidget::drag_drop_success()\n");
	dragging = false;

// handled by drop target
// if(container_obj)
//   container_obj->container->remove(selected_obj);
// else
//   actor->inventory_remove_obj(selected_obj);

	selected_obj = NULL;
	Redraw();
}

void ContainerWidget::drag_drop_failed(int x, int y, int message, void *data) {
	DEBUG(0, LEVEL_DEBUGGING, "ContainerWidget::drag_drop_failed()\n");
	dragging = false;
	selected_obj = NULL;
}

bool ContainerWidget::drag_set_target_obj(int x, int y) {

	target_obj = get_obj_at_location(x, y);
	target_cont = get_container();

	return true;
}

bool ContainerWidget::drag_accept_drop(int x, int y, int message, void *data) {
	DEBUG(0, LEVEL_DEBUGGING, "ContainerWidget::drag_accept_drop()\n");
	if (message == GUI_DRAG_OBJ) {
		Obj *obj = (Obj *)data;
		x -= area.left;
		y -= area.top;
		if (target_obj == NULL) { //we need to check this so we don't screw up target_obj on subsequent calls
			if (drag_set_target_obj(x, y) == false) {
				DEBUG(0, LEVEL_WARNING, "ContainerWidget: Didn't hit any widget object targets!\n");
				return false;
			}
		}
		Actor *container_owner = (container_obj ? container_obj->get_actor_holding_obj() : NULL);
		if (!container_owner)
			container_owner = actor;
		if (!obj->is_in_inventory()) {
			if (container_owner) {
				Game::get_game()->get_scroll()->display_string("Get-");
				Game::get_game()->get_scroll()->display_string(obj_manager->look_obj(obj, OBJ_SHOW_PREFIX));
				if (Game::get_game()->get_script()->call_actor_get_obj(container_owner, obj) == false) {
					Game::get_game()->get_scroll()->message("\n\n");
					return false;
				}
				Game::get_game()->get_scroll()->message("\n\n");
			}
		} else if (container_owner && obj->get_actor_holding_obj() != container_owner) {
			Events *event = Game::get_game()->get_event();
			event->display_move_text(container_owner, obj);
			if (!event->can_move_obj_between_actors(obj, obj->get_actor_holding_obj(), container_owner, false)) {
				Game::get_game()->get_scroll()->message("\n\n");
				return false;
			}
			Game::get_game()->get_scroll()->message("\n\n");
		}
		if (!obj_manager->can_get_obj(obj)) {
			Game::get_game()->get_scroll()->message("Not possible.\n\n");
			return false;
		}
		Actor *grabber = actor;
		if (!grabber)
			grabber = Game::get_game()->get_player()->get_actor();
		if (container_obj && !container_obj->is_in_inventory()
		        && !Game::get_game()->get_map_window()->can_get_obj(grabber, container_obj)) {
			Game::get_game()->get_scroll()->message("\n\nblocked\n\n");
			return false;
		}
		if (!obj->is_in_inventory() && !obj->is_readied()) {
			if (!Game::get_game()->get_map_window()->can_get_obj(grabber, obj)) {
				Game::get_game()->get_scroll()->message("\n\nblocked\n\n");
				return false;
			}
			if (obj_manager->obj_is_damaging(obj, Game::get_game()->get_player()->get_actor())) {
				Game::get_game()->get_player()->subtract_movement_points(3);
				return false;
			}
		}
		UseCode *usecode = Game::get_game()->get_usecode();
		if (usecode->is_chest(obj) && obj->frame_n == 0) //open chest
			obj->frame_n = 1; //close the chest

		DEBUG(0, LEVEL_DEBUGGING, "Drop Accepted\n");
		return true;
	}

	DEBUG(0, LEVEL_DEBUGGING, "Drop Refused\n");
	return false;
}

void ContainerWidget::drag_perform_drop(int x, int y, int message, void *data) {
	DEBUG(0, LEVEL_DEBUGGING, "ContainerWidget::drag_perform_drop()\n");
	Obj *obj;

	x -= area.left;
	y -= area.top;

	if (message == GUI_DRAG_OBJ) {
		DEBUG(0, LEVEL_DEBUGGING, "Drop into inventory.\n");
		obj = (Obj *)data;
		bool moving_between_actors = obj->get_actor_holding_obj() != actor;
		if (moving_between_actors)
			Game::get_game()->get_player()->subtract_movement_points(3);
		if (target_cont && obj_manager->can_store_obj(target_cont, obj)) {
			obj_manager->moveto_container(obj, target_cont);
		} else if (target_obj && obj_manager->can_store_obj(target_obj, obj)) {
			obj_manager->moveto_container(obj, target_obj);
		} else {
			if (obj->is_readied()) {
				Game::get_game()->get_event()->unready(obj);
				if (moving_between_actors)
					obj_manager->moveto_inventory(obj, actor);
			} else
				obj_manager->moveto_inventory(obj, actor);
		}

		Redraw();
	}

	Game::get_game()->get_map_window()->updateBlacking();
	target_obj = NULL;

	return;
}

void ContainerWidget::drag_draw(int x, int y, int message, void *data) {
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


void ContainerWidget::try_click() {
	Events *event = Game::get_game()->get_event();
	UseCode *usecode = Game::get_game()->get_usecode();
	Actor *owner = NULL;
	if (!selected_obj)
		selected_obj = ready_obj;
	if (selected_obj)
		owner = Game::get_game()->get_actor_manager()->get_actor(selected_obj->x);
	if (!owner || !owner->is_in_party())
		owner = Game::get_game()->get_player()->get_actor();
	switch (event->get_mode()) {
	case MOVE_MODE:
	case EQUIP_MODE: {
		if (!selected_obj)
			return;
		bool locked_chest = (usecode->is_chest(selected_obj) && selected_obj->frame_n > 1);
		if (usecode->is_container(selected_obj) && !locked_chest) {
			row_offset = 0;
			container_obj = selected_obj;
			if (usecode->is_chest(container_obj)) {
				usecode->process_effects(container_obj, owner);
				Redraw();
			}
		} else {
			event->ready(selected_obj, owner);
			Redraw();
		}
		break;
	}
	case GET_MODE:
		event->perform_get(selected_obj, NULL, Game::get_game()->get_player()->get_actor());
		break;
	case ATTACK_MODE:
		event->close_gumps();
		break;
	default:
		event->select_view_obj(selected_obj, owner);
		break;
	}
	ready_obj = NULL;
	selected_obj = NULL;
}

/* Use object. */
GUI_status ContainerWidget::MouseDouble(int x, int y, Shared::MouseButton button) {
	// we have to check if double-clicks are allowed here, since we use single-clicks
	if (!Game::get_game()->get_map_window()->is_doubleclick_enabled())
		return (GUI_PASS);
	Obj *obj = selected_obj;

	ready_obj = NULL;
	selected_obj = NULL;

//    if(!actor)
//        return(GUI_YUM);
	if (!obj)
		return (MouseUp(x, y, button)); // probably hit an arrow
	Game::get_game()->get_view_manager()->double_click_obj(obj);
	return (GUI_PASS);
}

GUI_status ContainerWidget::MouseClick(int x, int y, Shared::MouseButton button) {
	return (MouseUp(x, y, button));
}

// change container, ready/unready object, activate arrows
GUI_status ContainerWidget::MouseDelayed(int x, int y, Shared::MouseButton button) {
	if (ready_obj)
		try_click();
	return GUI_PASS;
}

} // End of namespace Nuvie
} // End of namespace Ultima
