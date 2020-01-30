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
#include "ultima/nuvie/gui/widgets/msg_scroll.h"
#include "ultima/nuvie/actors/actor_manager.h"
#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/usecode/usecode.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/script/script.h"
#include "ultima/nuvie/core/events.h"

namespace Ultima {
namespace Nuvie {

UseCode::UseCode(Game *g, Configuration *cfg) {
	game = g;
	config = cfg;
	obj_manager = NULL;
	map = NULL;
	player = NULL;
	scroll = NULL;
	actor_manager = NULL;
	obj_manager = NULL;
	party = NULL;
	script = NULL;

	script_thread = NULL;

	clear_items();
}

UseCode::~UseCode() {
	if (script_thread) {
		delete script_thread;
	}
}

bool UseCode::init(ObjManager *om, Map *m, Player *p, MsgScroll *ms) {
	obj_manager = om;
	map = m;
	player = p;
	scroll = ms;

	actor_manager = game->get_actor_manager();
	party = player->get_party();
	script = game->get_script();

	return true;
}



/* Clear items.
 */
void UseCode::clear_items() {
	memset(&items, 0, sizeof(items));
	/*    items.uint_ref = NULL;
	    items.sint_ref = NULL;
	    items.obj_ref = NULL;
	    items.actor_ref = items.actor2_ref = NULL;
	    items.mapcoord_ref = NULL;
	    items.msg_ref = NULL;
	    items.string_ref = NULL;
	    items.ent_ref = NULL;
	    items.data_ref = NULL; */
}

ScriptThread *UseCode::get_running_script() {
	if (script_thread && script_thread->is_running())
		return script_thread;

	return NULL;
}

bool UseCode::is_script_running() {
	if (script_thread && script_thread->is_running())
		return true;

	return false;
}

bool UseCode::is_container(Obj *obj) {
	return script->call_is_container_obj(obj->obj_n);
}

bool UseCode::has_usecode(Obj *obj, UseCodeEvent ev) {
	return script->call_has_usecode(obj, ev);
}

bool UseCode::use_obj(Obj *obj, Actor *actor) {
	if (script_thread) {
		delete script_thread;
		script_thread = NULL;
	}

	script_thread = script->call_use_obj(obj, actor);

	if (script_thread) {
		script_thread->start();
		if (script_thread->finished()) {
			delete script_thread;
			script_thread = NULL;
		}
	}

	return true;//script->call_use_obj(obj, actor);
}

// use obj at location with src_obj as object_ref
bool UseCode::use_obj(uint16 x, uint16 y, uint8 z, Obj *src_obj) {
	Obj *obj;

	obj = obj_manager->get_obj(x, y, z, true);

	if (obj == NULL)
		return false;

	return use_obj(obj, src_obj);
}

bool UseCode::ready_obj(Obj *obj, Actor *actor) {
	return script->call_ready_obj(obj, actor);
}

bool UseCode::move_obj(Obj *obj, sint16 rel_x, sint16 rel_y) {
	return script->call_move_obj(obj, rel_x, rel_y);
}

void UseCode::toggle_frame(Obj *obj) {
	if (obj->frame_n > 0)
		obj->frame_n--;
	else
		obj->frame_n = 1;
}


/* Print container contents and dump them on top of the container.
 */
//FIXME! some of this logic should go elsewhere.
bool UseCode::search_container(Obj *obj, bool show_string) {
	Obj *temp_obj;
	U6Link *obj_link;

	/* Test whether this object has items inside it. */
	if ((obj->container != NULL) &&
	        ((obj_link = obj->container->end()) != NULL)) {
		/* Add objects to obj_list. */
		for (; obj_link != NULL;) {
			temp_obj = (Obj *)obj_link->data;
			obj_link = obj_link->prev;
			/*
			obj_list->add(temp_obj);
			temp_obj->status |= OBJ_STATUS_OK_TO_TAKE;
			temp_obj->set_on_map(obj_list); //ERIC temp_obj->status &= ~OBJ_STATUS_IN_CONTAINER;
			temp_obj->x = obj->x;
			temp_obj->y = obj->y;
			temp_obj->z = obj->z;
			*/
			obj_manager->moveto_map(temp_obj, obj->is_in_container() ? MapCoord(obj->get_container_obj(true)) : MapCoord(obj));
			if (show_string) {
				scroll->display_string(obj_manager->look_obj(temp_obj, true));
				if (obj_link) // more objects left
					scroll->display_string(obj_link->prev ? ", " : ", and ");
			}
		}
		/* Remove objects from the container. */
		//obj->container->removeAll();
		return true;
	}
	return false;
}


/* Remove last object in container and return a pointer to it.
 */
Obj *UseCode::get_obj_from_container(Obj *obj) {
	Obj *temp_obj;
	if (obj->container && obj->container->end()) {
		temp_obj = (Obj *)obj->container->end()->data;
		obj->container->remove(temp_obj); // a pop_back() may be more efficient
		return (temp_obj);
	}
	return (NULL);
}


/* Print name of event being sent and the object receiving it.
 */
void UseCode::dbg_print_event(UseCodeEvent event, Obj *obj) {
	string do_string = "";
	switch (event) {
	case USE_EVENT_USE:
		do_string = "Use";
		break;
	case USE_EVENT_LOOK:
		do_string = "Look at";
		break;
	case USE_EVENT_PASS:
		do_string = "Pass";
		break;
	case USE_EVENT_SEARCH:
		do_string = "Search";
		break;
	case USE_EVENT_MOVE:
		do_string = "Move";
		break;
	case USE_EVENT_LOAD:
		do_string = "Load";
		break;
	case USE_EVENT_MESSAGE:
		do_string = "Message";
		break;
	case USE_EVENT_READY:
		do_string = "(Un)Equip";
		break;
	case USE_EVENT_GET:
		do_string = "Get";
		break;
	case USE_EVENT_DROP:
		do_string = "Drop";
		break;
	}
	if (do_string != "")
		DEBUG(0, LEVEL_DEBUGGING, "UseCode: %s object %d:%d (%03x,%03x,%x)\n", do_string.c_str(),
		      obj->obj_n, obj->frame_n, obj->x, obj->y, obj->z);
	else
		DEBUG(0, LEVEL_DEBUGGING, "UseCode: Events 0x%04x sent to object %d:%d (%03x,%03x,%x)\n",
		      event, obj->obj_n, obj->frame_n, obj->x, obj->y, obj->z);
}


/* Subtract `count' from object quantity. Destroy the object completely if all
 * stacked objects were removed, or the object is not stackable, or `count' is
 * 0. This means it will be removed from the world or an actor's inventory, and
 * deleted.
 * Returns the original object if it still exists, because the count was smaller
 * than the object stack, or it could not be completely destroyed for whatever
 * reason. Returns NULL if the object was destroyed.
 */
Obj *UseCode::destroy_obj(Obj *obj, uint32 count, bool run_usecode) {
	//ActorManager *actor_manager = Game::get_game()->get_actor_manager();
	//bool removed = false;

	// subtract
	if (count > 0 && obj_manager->is_stackable(obj) && obj->qty > count)
		obj->qty -= count;
	else { // destroy
		obj_manager->unlink_from_engine(obj, run_usecode);
		delete_obj(obj);
		obj = NULL;
	}

	return (obj);
}

/*
 * don't autowalk long distances to objects when foes are nearby or select obj outside of range
 */
bool UseCode::out_of_use_range(Obj *obj, bool check_enemies) {
	if (!obj) // this should be checked before you get here
		return true;
	if (obj->is_in_inventory())
		return false;

	MapCoord player_loc = player->get_actor()->get_location();
	MapCoord obj_loc = MapCoord(obj->x, obj->y, obj->z);

	if (!check_enemies) {
		if (player_loc.distance(obj_loc) > 1
		        && game->get_map_window()->get_interface() == INTERFACE_NORMAL) {
			scroll->display_string("\nOut of range.\n");
			return true;
		} else if (!game->get_map_window()->can_get_obj(player->get_actor(), obj)) {
			scroll->display_string("\nBlocked.\n");
			return true;
		} else
			return false;
	} else if (player_loc.distance(obj_loc) > 1) { // only setup for objects that already checked range and blocking limit
		ActorList *enemies = 0;

		if ((enemies = player->get_actor()->find_enemies())) {
			scroll->display_string("\nOut of range.\n");
			delete enemies;
			return true;
		}
		delete enemies;
	}
	return false;
}

const char *useCodeTypeToString(UseCodeType type) {
	switch (type) {
	case USE :
		return "use";
	case MOVE :
		return "move";
	case GET :
		return "get";
	default :
		return "other";
	}
}

} // End of namespace Nuvie
} // End of namespace Ultima
