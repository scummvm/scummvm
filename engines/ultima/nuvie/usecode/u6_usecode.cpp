/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ultima/nuvie/core/nuvie_defs.h"
#include "ultima/nuvie/misc/u6_llist.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/misc/map_entity.h"
#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/views/view.h"
#include "ultima/nuvie/views/view_manager.h"
#include "ultima/nuvie/actors/actor_manager.h"
#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/actors/u6_actor.h"
#include "ultima/nuvie/core/party.h"
#include "ultima/nuvie/core/player.h"
#include "ultima/nuvie/gui/widgets/msg_scroll.h"
#include "ultima/nuvie/core/map.h"
#include "ultima/nuvie/core/game_clock.h"
#include "ultima/nuvie/core/book.h"
#include "ultima/nuvie/core/events.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/core/timed_event.h"
#include "ultima/nuvie/core/egg_manager.h"
#include "ultima/nuvie/core/anim_manager.h"
#include "ultima/nuvie/sound/sound_manager.h"
#include "ultima/nuvie/core/effect.h"
#include "ultima/nuvie/core/weather.h"
#include "ultima/nuvie/script/script.h"
#include "ultima/nuvie/keybinding/keys.h"
#include "ultima/nuvie/gui/widgets/background.h"
#include "ultima/nuvie/gui/widgets/command_bar.h"
#include "ultima/nuvie/usecode/u6_usecode.h"
#include "ultima/nuvie/usecode/u6_object_types.h"
#include "ultima/nuvie/actors/u6_work_types.h"

namespace Ultima {
namespace Nuvie {

#define MESG_ANIM_HIT_WORLD  ANIM_CB_HIT_WORLD
#define MESG_ANIM_HIT        ANIM_CB_HIT
#define MESG_TEXT_READY      MSGSCROLL_CB_TEXT_READY
#define MESG_DATA_READY      CB_DATA_READY
#define MESG_EFFECT_COMPLETE EFFECT_CB_COMPLETE
#define MESG_TIMED           CB_TIMED
#define MESG_INPUT_CANCELED  CB_INPUT_CANCELED

// numbered by entrance quality, "" = no name
static const char *u6_dungeons[21] = {
	"",
	"Deceit",
	"Despise",
	"Destard",
	"Wrong",
	"Covetous",
	"Shame",
	"Hythloth",
	"GSA",
	"Control",
	"Passion",
	"Diligence",
	"Tomb of Kings",
	"Ant Mound",
	"Swamp Cave",
	"Spider Cave",
	"Cyclops Cave",
	"Heftimus Cave",
	"Heroes' Hole",
	"Pirate Cave",
	"Buccaneer's Cave"
};

// Red moongate teleport locations.
static const struct {
	uint16 x;
	uint16 y;
	uint8 z;
} red_moongate_tbl[] = {
	{0x0,   0x0,   0x0},
	{0x383, 0x1f3, 0x0},
	{0x3a7, 0x106, 0x0},
	{0x1b3, 0x18b, 0x0},
	{0x1f7, 0x166, 0x0},
	{0x93,  0x373, 0x0},
	{0x397, 0x3a6, 0x0},
	{0x44,  0x2d,  0x5},
	{0x133, 0x160, 0x0},
	{0xbc,  0x2d,  0x5},
	{0x9f,  0x3ae, 0x0},
	{0x2e3, 0x2bb, 0x0},
	{0x0,   0x0,   0x0},
	{0x0,   0x0,   0x0},
	{0x0,   0x0,   0x0},
	{0xe3,  0x83,  0x0},
	{0x17,  0x16,  0x1},
	{0x80,  0x56,  0x5},
	{0x6c,  0xdd,  0x5},
	{0x39b, 0x36c, 0x0},
	{0x127, 0x26,  0x0},
	{0x4b,  0x1fb, 0x0},
	{0x147, 0x336, 0x0},
	{0x183, 0x313, 0x0},
	{0x33f, 0xa6,  0x0},
	{0x29b, 0x43,  0x0}
};

static const uint8 USE_U6_POTION_BLUE   = 0;
static const uint8 USE_U6_POTION_RED    = 1;
static const uint8 USE_U6_POTION_YELLOW = 2;
static const uint8 USE_U6_POTION_GREEN  = 3;
static const uint8 USE_U6_POTION_ORANGE = 4;
static const uint8 USE_U6_POTION_PURPLE = 5;
static const uint8 USE_U6_POTION_BLACK  = 6;
static const uint8 USE_U6_POTION_WHITE  = 7;

// numbered by potion object frame number
static const char *u6_potions[8] = {
	"an awaken", // blue
	"a cure", // red
	"a heal", // yellow
	"a poison", // green
	"a sleep", // orange
	"a protection", // purple
	"an invisibility", // black
	"an xray vision" // white
};

// convenient macro to grab a target object - when first called the player will
// be prompted with P, and upon target selection the object will be set to O
#define USECODE_SELECT_OBJ(O, P) \
	{ \
		static bool selected_obj = false; \
		if(!selected_obj) \
		{ \
			game->get_event()->get_direction(P); \
			game->get_event()->request_input(this, obj); \
			selected_obj = true; \
			return false; \
		} \
		else { O = items.obj_ref; selected_obj = false; } \
	}
#define USECODE_SELECT_ACTOR(O, P) \
	{ \
		static bool selected_obj = false; \
		if(!selected_obj) \
		{ \
			game->get_event()->get_target(P); \
			game->get_event()->request_input(this, obj); \
			selected_obj = true; \
			return false; \
		} \
		else { A = items.actor2_ref; selected_obj = false; } \
	}
#define USECODE_SELECT_TARGET(T, P) \
	{ \
		static bool selected_obj = false; \
		if(!selected_obj) \
		{ \
			game->get_event()->get_target(P); \
			game->get_event()->request_input(this, obj); \
			selected_obj = true; \
			return false; \
		} \
		else { T = items.mapcoord_ref; selected_obj = false; } \
	}
#define USECODE_SELECT_DIRECTION(T, P) \
	{ \
		static bool selected_obj = false; \
		if(!selected_obj) \
		{ \
			game->get_event()->get_target(P); \
			game->get_event()->request_input(this, obj); \
			selected_obj = true; \
			return false; \
		} \
		else { T = items.mapcoord_ref; selected_obj = false; } \
	}


U6UseCode::U6UseCode(Game *g, const Configuration *cfg) : UseCode(g, cfg) {
}

U6UseCode::~U6UseCode() {
}


/* Is the object a food (or drink) item? */
bool U6UseCode::is_food(const Obj *obj) const {
	const U6ObjectType *type = get_object_type(obj->obj_n, obj->frame_n);
	return (type && (type->flags & OBJTYPE_FOOD));
}


bool U6UseCode::is_container(const Obj *obj) const {
	const U6ObjectType *type = get_object_type(obj->obj_n, obj->frame_n);
	return (type && (type->flags & OBJTYPE_CONTAINER));
}

bool U6UseCode::is_container(uint16 obj_n, uint8 frame_n) const {
	const U6ObjectType *type = get_object_type(obj_n, frame_n);
	return (type && (type->flags & OBJTYPE_CONTAINER));
}

bool U6UseCode::is_readable(const Obj *obj) const {
	const U6ObjectType *type = get_object_type(obj->obj_n, obj->frame_n);
	return ((type && (type->flags & OBJTYPE_BOOK)) || obj->obj_n == OBJ_U6_CLOCK
	        || obj->obj_n == OBJ_U6_SUNDIAL);
}


/* Is there `ev' usecode for an object? */
bool U6UseCode::has_usecode(Obj *obj, UseCodeEvent ev) {
	const U6ObjectType *type = get_object_type(obj->obj_n, obj->frame_n, ev);
	if (!type && !UseCode::has_usecode(obj, ev))
		return false;
	return true;
}

bool U6UseCode::has_usecode(Actor *actor, UseCodeEvent ev) {
	const U6ObjectType *type = get_object_type(actor->get_obj_n(), actor->get_frame_n(), ev);
	if (!type || type->flags == OBJTYPE_CONTAINER)
		return false;
	return true;
}

/* USE object. Actor is the actor using the object. */
bool U6UseCode::use_obj(Obj *obj, Actor *actor) {
	if (UseCode::has_usecode(obj, USE_EVENT_USE)) { //use script based usecode.
		return UseCode::use_obj(obj, actor);
	}

	const U6ObjectType *type = get_object_type(obj->obj_n, obj->frame_n, USE_EVENT_USE);
	set_itemref(actor, items.actor2_ref);
	return (uc_event(type, USE_EVENT_USE, obj));
}

/* LOOK at object. Actor is the actor looking at the object. */
bool U6UseCode::look_obj(Obj *obj, Actor *actor) {
	const U6ObjectType *type = get_object_type(obj->obj_n, obj->frame_n, USE_EVENT_LOOK);
	set_itemref(actor);
	return (uc_event(type, USE_EVENT_LOOK, obj));
}

/* PASS object. Actor is the actor trying to pass the object. It takes
   target coordinates in case the object has multiple tiles. */
bool U6UseCode::pass_obj(Obj *obj, Actor *actor, uint16 x, uint16 y) {
	const U6ObjectType *type = get_object_type(obj->obj_n, obj->frame_n, USE_EVENT_PASS);
	static MapCoord loc;
	loc.x = x;
	loc.y = y;
	loc.z = obj->z;
	set_itemref(actor);
	set_itemref(&loc);
	return (uc_event(type, USE_EVENT_PASS, obj));
}

/* SEARCH nearby object. Actor is the actor searching. */
bool U6UseCode::search_obj(Obj *obj, Actor *actor) {
	const U6ObjectType *type = get_object_type(obj->obj_n, obj->frame_n, USE_EVENT_SEARCH);
	set_itemref(actor);
	return (uc_event(type, USE_EVENT_SEARCH, obj));
}


/* Callback from timer or other class. User_data is the object which will
 * receive the message (if applicable). */
uint16 U6UseCode::callback(uint16 msg, CallBack *caller, void *msg_data) {
	Obj *obj = (Obj *)callback_user_data;
	if (!obj) {
		DEBUG(0, LEVEL_ERROR, "UseCode: internal message %d sent to nullptr object\n", msg);
		return 0;
	}
	return (message_obj(obj, (CallbackMessage)msg, msg_data));
}


/* Call MESSAGE function for an object. Msg_data is assigned to the appropriate
 * itemref. The USE function is called in response to a DATA_READY message.
 * Returns false if there is no usecode for that object.
 */
bool U6UseCode::message_obj(Obj *obj, CallbackMessage msg, void *msg_data) {
	const U6ObjectType *type = get_object_type(obj->obj_n, obj->frame_n, USE_EVENT_MESSAGE);
	items.msg_ref = &msg;
	switch (msg) { // set itemref from msgdata
	case MESG_TIMED:
		items.uint_ref = (uint32 *)msg_data;
		break;
	case MESG_ANIM_HIT_WORLD:
		items.mapcoord_ref = (MapCoord *)msg_data;
		break;
	case MESG_ANIM_HIT:
		items.ent_ref = (MapEntity *)msg_data;
		break;
	case MESG_TEXT_READY:
		items.string_ref = (string *)msg_data;
		break;
	case MESG_DATA_READY:
		items.data_ref = (char *)msg_data; // pointer to EventInput structure
		items.obj_ref = ((EventInput *)items.data_ref)->obj;
		items.actor2_ref = ((EventInput *)items.data_ref)->actor;
		items.mapcoord_ref = ((EventInput *)items.data_ref)->loc;
		items.string_ref = ((EventInput *)items.data_ref)->str;
		return uc_event(get_object_type(obj->obj_n, obj->frame_n),
		                USE_EVENT_USE, obj);
	case MESG_INPUT_CANCELED:
		return uc_event(get_object_type(obj->obj_n, obj->frame_n),
		                USE_EVENT_INPUT_CANCEL, obj);
	default :
		break;
	}

	return uc_event(type, USE_EVENT_MESSAGE, obj);
}


/* MOVE nearby object in a relative direction. */
bool U6UseCode::move_obj(Obj *obj, sint16 rel_x, sint16 rel_y) {
	const U6ObjectType *type = get_object_type(obj->obj_n, obj->frame_n, USE_EVENT_MOVE);
	static MapCoord dir;
	dir.sx = rel_x;
	dir.sy = rel_y;
	set_itemref(&dir);
	return (uc_event(type, USE_EVENT_MOVE, obj));
}


/* Call post-LOAD or UNLOAD usecode for an object.
 * Returns false if there is no usecode for that object. */
bool U6UseCode::load_obj(Obj *obj) {
	const U6ObjectType *type = get_object_type(obj->obj_n, obj->frame_n, USE_EVENT_LOAD);
	return (uc_event(type, USE_EVENT_LOAD, obj));
}


/* Call READY or UNREADY usecode for an object. */
bool U6UseCode::ready_obj(Obj *obj, Actor *actor) {
	const U6ObjectType *type = get_object_type(obj->obj_n, obj->frame_n, USE_EVENT_READY);
	set_itemref(actor);
	return (uc_event(type, USE_EVENT_READY, obj));
}


/* Call GET usecode for an object. */
bool U6UseCode::get_obj(Obj *obj, Actor *actor) {
	const U6ObjectType *type = get_object_type(obj->obj_n, obj->frame_n, USE_EVENT_GET);
	set_itemref(actor);
	return (uc_event(type, USE_EVENT_GET, obj));
}


/* Call DROP usecode for an object. */
bool U6UseCode::drop_obj(Obj *obj, Actor *actor, uint16 x, uint16 y, uint16 qty) {
	const U6ObjectType *type = get_object_type(obj->obj_n, obj->frame_n, USE_EVENT_DROP);
	static MapCoord loc; // use statics for pointers
	static uint32 obj_qty;
	loc.x = x;
	loc.y = y;
	set_itemref(actor);
	set_itemref(&loc);
	items.uint_ref = &obj_qty;
	return (uc_event(type, USE_EVENT_DROP, obj));
}


/* Return pointer to object-type in list for object N:F, or nullptr if none. */
inline const U6ObjectType *U6UseCode::get_object_type(uint16 n, uint8 f, UseCodeEvent ev) const {
	const U6ObjectType *type = U6ObjectTypes;
	while (type->obj_n != OBJ_U6_NOTHING) {
		if (type->obj_n == n && (type->frame_n == 0xFF || type->frame_n == f)
		        && ((type->trigger & ev) || ev == 0))
			return type;
		++type;
	}
	return nullptr;
}


/* Call usecode function of the U6ObjectType, with event `ev', for `obj'.
 * The meaning of the return value is different for each event.
 * Returns false if the type is invalid or doesn't respond to the event.
 */
bool U6UseCode::uc_event(const U6ObjectType *type, UseCodeEvent ev, Obj *obj) {
	if (!type || type->obj_n == OBJ_U6_NOTHING)
		return false;
	if (type->trigger & ev) {
		dbg_print_event(ev, obj);
		bool ucret = (this->*type->usefunc)(obj, ev);
		clear_items(); // clear references for next call
		return (ucret); // return from usecode function
	}
	return false; // doesn't respond to event
}


void U6UseCode::lock_door(Obj *obj) {
	if (is_unlocked_door(obj))
		obj->frame_n += 4;
}

void U6UseCode::unlock_door(Obj *obj) {
	if (is_locked_door(obj))
		obj->frame_n -= 4;
}

void U6UseCode::unlock(Obj *obj) {
	if (is_locked_door(obj)) {
		unlock_door(obj);
	} else if (is_locked_chest(obj)) {
		unlock_chest(obj);
	}
}

void U6UseCode::lock(Obj *obj) {
	if (is_magically_locked(obj) || is_locked(obj))
		return;

	if (is_closed_door(obj)) {
		lock_door(obj);
	} else if (is_closed_chest(obj)) {
		lock_chest(obj);
	}
}

// USE: unlock locked doors, open/close other doors
bool U6UseCode::use_door(Obj *obj, UseCodeEvent ev) {
	Obj *key_obj;
	bool print = (items.actor_ref == player->get_actor());

	if (is_magically_locked_door(obj)) {
		if (print) scroll->display_string("\nmagically locked\n");
		return true;
	}

	if (is_locked_door(obj)) { // locked door
		key_obj = player->get_actor()->inventory_get_object(OBJ_U6_KEY, obj->quality);
		if (obj->quality != 0 && key_obj != nullptr) { // we have the key for this door so lets unlock it.
			unlock_door(obj);
			if (print) scroll->display_string("\nunlocked\n");
		} else if (print) scroll->display_string("\nlocked\n");

		return true;
	}

	if (obj->frame_n <= 3) { //the door is open
		if (!map->is_passable(obj->x, obj->y, obj->z) || map->actor_at_location(obj->x, obj->y, obj->z)) { //don't close door if blocked
			if (print) scroll->display_string("\nNot now!\n");
		} else { //close the door
			obj->frame_n += 4;
			if (print) scroll->display_string("\nclosed!\n");
		}
	} else {
		process_effects(obj, items.actor_ref); //process traps.
		obj->frame_n -= 4;
		if (print) scroll->display_string("\nopened!\n");
	}

	return true;
}

// USE: climb up or down a ladder (entire party)
bool U6UseCode::use_ladder(Obj *obj, UseCodeEvent ev) {
	uint16 x = obj->x, y = obj->y;
	uint8 z;
	if (!player->in_party_mode()) {
		scroll->display_string("\nNot in solo mode.\n");
		return true;
	}

	if (UseCode::out_of_use_range(obj, true))
		return true;

	if (obj->frame_n == 0) { // DOWN
		if (obj->z == 0) { //handle the transition from the surface to the first dungeon level
			x = (obj->x & 0x07) | (obj->x >> 2 & 0xF8);
			y = (obj->y & 0x07) | (obj->y >> 2 & 0xF8);
		}
		z = obj->z + 1;
	} else { //UP
		if (obj->z == 1) { //we use obj->quality to tell us which surface chunk to come up in.
			x = obj->x / 8 * 8 * 4 + ((obj->quality & 0x03) * 8) + (obj->x - obj->x / 8 * 8);
			y = obj->y / 8 * 8 * 4 + ((obj->quality >> 2 & 0x03) * 8) + (obj->y - obj->y / 8 * 8);
		}
		z = obj->z - 1;
	}

	party->dismount_from_horses();

	MapCoord ladder(obj->x, obj->y, obj->z), destination(x, y, z);
	party->walk(&ladder, &destination, 100);
	if (z != 0 && z != 5)
		game->get_weather()->set_wind_dir(NUVIE_DIR_NONE);
	return true;
}


// USE: Open the passhtrough! Close the passthrough!
bool U6UseCode::use_passthrough(Obj *obj, UseCodeEvent ev) {
	uint16 new_x, new_y;
	uint8 new_frame_n;
	char action_string[6]; // either 'Open' or 'Close'
	bool print = (items.actor_ref == player->get_actor());

	new_x = obj->x;
	new_y = obj->y;
	new_frame_n = obj->frame_n;

	if (obj->frame_n < 2) { // the pass through is currently closed.
		if (obj->obj_n == OBJ_U6_V_PASSTHROUGH)
			new_y--;
		else // OBJ_U6_H_PASSTHROUGH
			new_x--;

		new_frame_n = 2; // open the pass through
		Common::strcpy_s(action_string, "Open");
	} else { // the pass through is currently open.
		if (obj->obj_n == OBJ_U6_V_PASSTHROUGH)
			new_y++;
		else // OBJ_U6_H_PASSTHROUGH
			new_x++;

		new_frame_n = 0; // close the pass through
		Common::strcpy_s(action_string, "Close");
	}

	if (!map->actor_at_location(new_x, new_y, obj->z)) {
		obj_manager->move(obj, new_x, new_y, obj->z);
		obj->frame_n = new_frame_n;
		if (print) {
			scroll->display_string("\n");
			scroll->display_string(action_string);
			scroll->display_string(" the passthrough.\n");
		}
	} else if (print)
		scroll->display_string("\nNot now!\n");

	return true;
}

// for use with levers and switches, target_obj_n is either OBJ_U6_PORTCULLIS or OBJ_U6_ELECTRIC_FIELD
bool U6UseCode::use_switch(Obj *obj, UseCodeEvent ev) {
	Obj *doorway_obj;
	Obj *portc_obj;
	U6LList *obj_list;
	U6Link *link;
	uint16 target_obj_n = 0;
	const char *message = nullptr;
	const char *fail_message = nullptr;
	bool success = false;
	bool print = (items.actor_ref == player->get_actor());

	if (obj->obj_n == OBJ_U6_LEVER) {
		target_obj_n = OBJ_U6_PORTCULLIS;
		message = "\nSwitch the lever, you hear a noise.\n";
		fail_message = "\nSwitch the lever, strange, nothing happened.\n";
	} else if (obj->obj_n == OBJ_U6_SWITCH) {
		if (obj->quality == 113 && obj->x == 139 && obj->y == 0 && obj->z == 1) { // hack for Covetous
			doorway_obj = obj_manager->get_obj_of_type_from_location(OBJ_U6_DOORWAY, 0, 0, 160, 3, 1);
			if (doorway_obj)
				doorway_obj->quality = 113;
		}
		target_obj_n = OBJ_U6_ELECTRIC_FIELD;
		message = "\nOperate the switch, you hear a noise.\n";
		fail_message = "\nOperate the switch, strange, nothing happened.\n";
	}

	doorway_obj = obj_manager->find_obj(obj->z, OBJ_U6_DOORWAY, obj->quality);
	for (; doorway_obj != nullptr; doorway_obj = obj_manager->find_next_obj(obj->z, doorway_obj)) {
		obj_list = obj_manager->get_obj_list(doorway_obj->x, doorway_obj->y, doorway_obj->z);

		for (portc_obj = nullptr, link = obj_list->start(); link != nullptr; link = link->next) { // find existing portcullis.
			if (((Obj *)link->data)->obj_n == target_obj_n) {
				portc_obj = (Obj *)link->data;
				break;
			}
		}
		success = true;

		if (portc_obj == nullptr) { //no barrier object, so lets create one.
			portc_obj = obj_manager->copy_obj(doorway_obj);
			portc_obj->obj_n = target_obj_n;
			portc_obj->quality = 0;
			if (target_obj_n == OBJ_U6_PORTCULLIS) {
				if (portc_obj->frame_n == 9) //FIX Hack for cream buildings might need one for virt wall.
					portc_obj->frame_n = 1;
			} else
				portc_obj->frame_n = 0;

			obj_manager->add_obj(portc_obj, true);
		} else { //delete barrier object.
			obj_list->remove(portc_obj);
			delete_obj(portc_obj);
		}
	}

	toggle_frame(obj);
	if (print)
		scroll->display_string(success ? message : fail_message);
	return true;
}


/* USE: light or douse various fire objects (toggling their frame number)
 */
bool U6UseCode::use_firedevice(Obj *obj, UseCodeEvent ev) {
	if (obj->obj_n == OBJ_U6_BRAZIER && obj->frame_n == 2)
		return true; // holy flames can't be doused
	if (obj->obj_n == OBJ_U6_FIREPLACE) {
		if (obj->frame_n == 1 || obj->frame_n == 3) {
			use_firedevice_message(obj, false);
			obj->frame_n--;
		} else {
			use_firedevice_message(obj, true);
			obj->frame_n++;
		}
	} else {
		toggle_frame(obj);
		use_firedevice_message(obj, (bool)obj->frame_n);
	}
	return true;
}


/* SEARCH: discover and open door
 * USE: open or close door
 */
bool U6UseCode::use_secret_door(Obj *obj, UseCodeEvent ev) {
	if (ev == USE_EVENT_USE) {
		if (obj->frame_n == 1 || obj->frame_n == 3)
			obj->frame_n--;
		else
			obj->frame_n++;
		return true;
	} else if (ev == USE_EVENT_SEARCH) {
		scroll->display_string("a secret door");
		if (obj->frame_n == 0 || obj->frame_n == 2)
			obj->frame_n++;
		return true;
	}
	return true;
}


/* Use: Open/close container. If container is open, Search.
 * Search: Dump container contents.
 */
bool U6UseCode::use_container(Obj *obj, UseCodeEvent ev) {
	if (ev == USE_EVENT_USE) {
		if (is_locked_chest(obj) || is_magically_locked_chest(obj)) {
			if (is_locked_chest(obj) && obj->quality != 0) {
				Obj *key_obj = player->get_actor()->inventory_get_object(OBJ_U6_KEY, obj->quality);
				if (key_obj != nullptr) { // we have the key for this chest so lets unlock it.
					unlock_chest(obj);
					scroll->display_string("\nunlocked\n");
					return true;
				}
			}

			scroll->display_string("\nNo effect\n");
			return true;
		}
		if ((obj->obj_n == OBJ_U6_CHEST && !obj->is_in_inventory()) || obj->obj_n == OBJ_U6_CRATE || obj->obj_n == OBJ_U6_BARREL)
			toggle_frame(obj); //open / close object
		if (obj->frame_n == 0 || (obj->obj_n != OBJ_U6_CHEST && obj->obj_n != OBJ_U6_CRATE && obj->obj_n != OBJ_U6_BARREL)
		        || (obj->obj_n == OBJ_U6_CHEST && obj->frame_n < 2 && obj->is_in_inventory())) {
			process_effects(obj, items.actor_ref); //run any effects that might be stored in this container. Eg Poison explosion.

			if (Game::get_game()->doubleclick_opens_containers() && obj->obj_n != OBJ_U6_DEER
			        && obj->obj_n != OBJ_U6_SHIP && obj->obj_n != OBJ_U6_STONE_LION) { // just search for these
				game->get_view_manager()->open_container_view(obj);
			} else if (obj->is_in_inventory()) {
				scroll->display_string("\nNot usable\n");
				return true;
			} else {
				scroll->display_string("\nSearching here, you find ");
				bool found_objects = search_obj(obj, items.actor_ref);
				scroll->display_string(found_objects ? ".\n" : "nothing.\n");
			}
		}
		return true;
	} else if (ev == USE_EVENT_SEARCH) { // search message already printed
		return UseCode::search_container(obj);
//        if(obj->container && obj->container->end())
//        {
//            new TimedContainerSearch(obj);
//            return(true);
//        }
	} else if (ev == USE_EVENT_GET) {
		if (is_chest(obj) && obj->frame_n == 0) //open chest
			obj->frame_n = 1; //close the chest
		return true;
	}
	return false;
}

/* Use rune to free shrine */
bool U6UseCode::use_rune(Obj *obj, UseCodeEvent ev) {
	char mantras[][8] = {"AHM", "MU", "RA", "BEH", "CAH", "SUMM", "OM", "LUM"};
	Obj *force_field = nullptr;
	uint16 rune_obj_offset = obj->obj_n - OBJ_U6_RUNE_HONESTY;
	MapCoord player_location = player->get_actor()->get_location();


	scroll->cancel_input_request();

	if (ev == USE_EVENT_USE) {
		scroll->display_string("Mantra: ");

		scroll->set_input_mode(true, nullptr, true);
		scroll->request_input(this, obj);

		return false;
	} else if (ev == USE_EVENT_MESSAGE && items.string_ref) {
		scroll->display_string("\n");

		size_t mantraSize = items.string_ref->size() + 1;
		char *mantra = new char[mantraSize];
		Common::strcpy_s(mantra, mantraSize, items.string_ref->c_str());

		if (scumm_stricmp(mantra,  mantras[rune_obj_offset]) == 0) {
			// find the matching force field for this shrine. match rune offset against force field quality
			force_field = obj_manager->find_obj(player_location.z, OBJ_U6_FORCE_FIELD, rune_obj_offset);

			// make sure the player is right next to the force field.
			if (force_field && abs(player_location.x - force_field->x) < 2 && abs(player_location.y - force_field->y) < 2) {
				game->get_sound_manager()->playSfx(NUVIE_SFX_CASTING_MAGIC_P1, SFX_PLAY_ASYNC);
				game->get_sound_manager()->playSfx(NUVIE_SFX_CASTING_MAGIC_P2, SFX_PLAY_SYNC);
				AsyncEffect *e = new AsyncEffect(new XorEffect(1000));
				e->run();

				remove_gargoyle_egg(force_field->x, force_field->y, force_field->z);

				obj_manager->remove_obj_from_map(force_field);
				delete force_field;


				scroll->display_string("\nDone!\n");
			} else
				scroll->display_string("\nNo effect!\n");
		} else
			scroll->display_string("\nWrong mantra!\n");

		scroll->display_string("\n");
		scroll->display_prompt();
		delete[] mantra;
	}

	return true;
}

void U6UseCode::remove_gargoyle_egg(uint16 x, uint16 y, uint8 z) {
	Std::list<Egg *> *egg_list = game->get_egg_manager()->get_egg_list();
	Std::list<Egg *>::iterator egg_itr;

	for (egg_itr = egg_list->begin(); egg_itr != egg_list->end();) {
		Egg *egg = *egg_itr;
		Obj *egg_obj = egg->obj;
		egg_itr++;  // increment here, it might get removed from the list below.
		if (abs(x - egg_obj->x) < 20 && abs(y - egg_obj->y) < 20 && z == egg_obj->z) {
			if (egg_obj->find_in_container(OBJ_U6_GARGOYLE, 0, false, 0, false) || egg_obj->find_in_container(OBJ_U6_WINGED_GARGOYLE, 0, false, 0, false)) {
				DEBUG(0, LEVEL_DEBUGGING, "Removed egg at (%x,%x,%x)", egg_obj->x, egg_obj->y, egg_obj->z);
				game->get_egg_manager()->remove_egg(egg_obj, false);
				obj_manager->unlink_from_engine(egg_obj);
				delete_obj(egg_obj);
			}
		}
	}
}

bool U6UseCode::use_vortex_cube(Obj *obj, UseCodeEvent ev) {
	if (ev == USE_EVENT_SEARCH)
		return UseCode::search_container(obj);

	Obj *britannian_lens, *gargoyle_lens;
	Obj *container_obj;
	Obj *codex;
	U6Link *link;
	uint8 moonstone_check = 0;
	MapCoord player_location = player->get_actor()->get_location();
	codex = obj_manager->find_obj(player_location.z, OBJ_U6_CODEX, 128); // 128 = codex's book id

	if (Game::get_game()->doubleclick_opens_containers() && (obj->is_in_inventory() || !codex
	        || abs(player_location.x - codex->x) > 11 || abs(player_location.y - codex->y) > 11)) { //FIXME this should probably be mapwindow size)
		game->get_view_manager()->open_container_view(obj);
		return true;
	}
	if (obj->container != nullptr || player_location.z == 0) { // make sure we've got all 8 moonstones in our vortex cube.
		britannian_lens = obj_manager->find_obj(player_location.z, OBJ_U6_BRITANNIAN_LENS, 0, OBJ_NOMATCH_QUALITY);
		gargoyle_lens = obj_manager->find_obj(player_location.z, OBJ_U6_GARGOYLE_LENS, 0, OBJ_NOMATCH_QUALITY);

		// make sure the player is close to the codex
		if (codex && abs(player_location.x - codex->x) < 11 && abs(player_location.y - codex->y) < 11) { //FIXME this should probably be mapwindow size
			// check that the lenses are in the correct place.
			if (britannian_lens && gargoyle_lens &&
			        britannian_lens->x == 0x399 && britannian_lens->y == 0x353 && britannian_lens->z == 0 &&
			        gargoyle_lens->x == 0x39d && gargoyle_lens->y == 0x353 && gargoyle_lens->z == 0) {
				for (link = obj->container->start(); link != nullptr; link = link->next) {
					container_obj = (Obj *)link->data;
					if (container_obj->obj_n == OBJ_U6_MOONSTONE) {
						moonstone_check |= 1 << container_obj->frame_n;
					}
				}

				if (moonstone_check == 0xff) { // have we got all 8 moonstones?
					obj_manager->remove_obj_from_map(codex);
					delete codex;

					scroll->display_string("\nThe Codex has vanished!\n");

					//FIXME put weird swirl effect in here.

					game->get_map_window()->Hide();
					game->get_scroll()->Hide();
					game->get_background()->Hide();
					game->get_command_bar()->Hide();
					game->get_event()->close_gumps();
					if (game->get_view_manager()->get_current_view())
						game->get_view_manager()->get_current_view()->Hide();

					game->get_script()->play_cutscene("/ending.lua");
					game->quit();

					return true;
				}
			}
		}
	}

	DEBUG(0, LEVEL_DEBUGGING, "moonstone_check = %d\n", moonstone_check);
	scroll->display_string("\nNo Effect!\n");

	return true;
}


/* Use bell or pull-chain, ring and animate nearby bell.
 */
bool U6UseCode::use_bell(Obj *obj, UseCodeEvent ev) {
	Obj *bell = nullptr;
	if (ev != USE_EVENT_USE)
		return false;
	if (obj->obj_n == OBJ_U6_BELL)
		bell = obj;
	else
		bell = bell_find(obj);
	if (bell) {
		obj_manager->animate_forwards(bell, 2);
	}
	Game::get_game()->get_sound_manager()->playSfx(NUVIE_SFX_BELL);

	return true;
}


/* Find bell near its pull-chain.
 */
Obj *U6UseCode::bell_find(Obj *chain_obj) {
	Obj *bell = nullptr;
	for (uint16 x = chain_obj->x - 8; x <= chain_obj->x + 8; x++)
		for (uint16 y = chain_obj->y - 8; y <= chain_obj->y + 8 && !bell; y++)
			bell = obj_manager->get_obj_of_type_from_location(OBJ_U6_BELL, x, y, chain_obj->z);
	return bell;
}


//cranks control drawbridges.
bool U6UseCode::use_crank(Obj *obj, UseCodeEvent ev) {
	uint16 x, y;
	uint8 level;
	uint16 b_width;
	bool bridge_open;
	Obj *start_obj;

	start_obj = drawbridge_find(obj);

	if (start_obj->frame_n == 3) // bridge open
		bridge_open = true;
	else
		bridge_open = false;

	x = start_obj->x;
	y = start_obj->y;
	level = start_obj->z;
	drawbridge_remove(x, y, level, &b_width);

// find and animate chain
	if (!(start_obj = obj_manager->get_obj_of_type_from_location(OBJ_U6_CHAIN, obj->x + 1, obj->y, obj->z)))
		start_obj = obj_manager->get_obj_of_type_from_location(OBJ_U6_CHAIN, obj->x - 1, obj->y, obj->z);
	if (start_obj)
		obj_manager->animate_forwards(start_obj, 3);

	if (bridge_open) {
		obj_manager->animate_backwards(obj, 3); // animate crank
		drawbridge_close(x, y, level, b_width);
	} else {
		obj_manager->animate_forwards(obj, 3);
		drawbridge_open(x, y, level, b_width);
	}

	return true;
}

Obj *U6UseCode::drawbridge_find(Obj *crank_obj) {
	uint16 i, j;
	Obj *start_obj, *tmp_obj;

	for (i = 0; i < 6; i++) { // search on right side of crank.
		start_obj = obj_manager->get_obj_of_type_from_location(OBJ_U6_DRAWBRIDGE, crank_obj->x + 1, crank_obj->y + i,  crank_obj->z);
		if (start_obj != nullptr) // this means we are using the left crank.
			return start_obj;
	}

	for (i = 0; i < 6; i++) { // search on left side of crank.
		tmp_obj = obj_manager->get_obj_of_type_from_location(OBJ_U6_DRAWBRIDGE, crank_obj->x - 1, crank_obj->y + i,  crank_obj->z);

		if (tmp_obj != nullptr) { // this means we are using the right crank.
			//find the start of the drawbridge on the left.
			// we do this by searching to the left of the crank till we hit the crank on the otherside.
			// we then move right 1 tile and down 'i' tiles to the start object. :)
			for (j = 1; j < crank_obj->x; j++) {
				tmp_obj = obj_manager->get_obj_of_type_from_location(OBJ_U6_CRANK, crank_obj->x - j, crank_obj->y,  crank_obj->z);
				if (tmp_obj && tmp_obj->obj_n == OBJ_U6_CRANK) {
					start_obj = obj_manager->get_obj_of_type_from_location(OBJ_U6_DRAWBRIDGE, tmp_obj->x + 1, tmp_obj->y + i,  tmp_obj->z);
					return start_obj;
				}
			}
		}
	}

	return nullptr;
}

void U6UseCode::drawbridge_open(uint16 x, uint16 y, uint8 level, uint16 b_width) {
	uint16 i, j;
	Obj *obj;

	y++;

	for (i = 0;; i++) {
		obj = new_obj(OBJ_U6_DRAWBRIDGE, 3, x, y + i, level); //left side chain
		obj_manager->add_obj(obj, true);

		obj = new_obj(OBJ_U6_DRAWBRIDGE, 5, x + b_width - 1, y + i, level); //right side chain
		obj_manager->add_obj(obj, true);

		for (j = 0; j < b_width - 2; j++) {
			obj = new_obj(OBJ_U6_DRAWBRIDGE, 4, x + 1 + j, y + i, level);
			obj_manager->add_obj(obj, true);
		}

		if (map->is_passable(x, y + i + 1, level)) //we extend the drawbridge until we hit a passable tile.
			break;
	}

	i++;

	for (j = 0; j < b_width - 2; j++) { //middle bottom tiles
		obj = new_obj(OBJ_U6_DRAWBRIDGE, 1, x + 1 + j, y + i, level);
		obj_manager->add_obj(obj, true);
	}

	obj = new_obj(OBJ_U6_DRAWBRIDGE, 0, x, y + i, level); //bottom left
	obj_manager->add_obj(obj, true);

	obj = new_obj(OBJ_U6_DRAWBRIDGE, 2, x + b_width - 1, y + i, level); // bottom right
	obj_manager->add_obj(obj, true);

	scroll->display_string("\nOpen the drawbridge.\n");

	return;
}

void U6UseCode::drawbridge_close(uint16 x, uint16 y, uint8 level, uint16 b_width) {
	uint16 i;
	Obj *obj;

	y--;

	obj = new_obj(OBJ_U6_DRAWBRIDGE, 6, x - 1, y, level); //left side
	obj_manager->add_obj(obj, true);

	obj = new_obj(OBJ_U6_DRAWBRIDGE, 8, x + b_width - 1, y, level); //right side
	obj_manager->add_obj(obj, true);

	for (i = 0; i < b_width - 1; i++) {
		obj = new_obj(OBJ_U6_DRAWBRIDGE, 7, x + i, y, level); //middle
		obj_manager->add_obj(obj, true);
	}

	scroll->display_string("\nClose the drawbridge.\n");
}

/* GET: clear buried moonstone location
 *      (we never know if it is buried or just lying there)
 * USE: drop at user's current location, and update buried location
 */
bool U6UseCode::use_moonstone(Obj *obj, UseCodeEvent ev) {
	if (ev == USE_EVENT_GET) {
		Weather *weather = game->get_weather();
		/* FIXME: need to check weights here already?
		 * Check original's behavior when moonstone cannot be gotten due
		 * to weight limitations. Moving it normally doesn't unbury it,
		 * so probably failing to get it shouldn't either.
		 */
		weather->set_moonstone(obj->frame_n, MapCoord(0, 0, 0)) ;
		//scroll->display_string("\nMoonstone dug up. (FIXME)\n");
		weather->update_moongates();
		return true;
	} else if (ev == USE_EVENT_USE) {
		Weather *weather = game->get_weather();
		MapCoord loc = Game::get_game()->get_player()->get_actor()->get_location();
		const Tile *map_tile = map->get_tile(loc.x, loc.y, loc.z);

		if ((map_tile->tile_num < 1 || map_tile->tile_num > 7) && (map_tile->tile_num < 0x10 || map_tile->tile_num > 0x6f)) {
			scroll->display_string("Cannot be buried here!\n");
			return true;
		}

		weather->set_moonstone(obj->frame_n, loc) ;
		scroll->display_string("buried.\n");

		obj_manager->moveto_map(obj, loc);
		obj->status |= OBJ_STATUS_OK_TO_TAKE;
		weather->update_moongates();
		return true;
	}
	return false;
}

/* USE: select location on ground to bury orb; when location is passed, open
 *      a red moongate to a new location
 */
bool U6UseCode::use_orb(Obj *obj, UseCodeEvent ev) {
	Obj *gate;
	uint16 x, y, ox, oy;
	uint8 px, py, z, oz;
	uint8 position;
	Actor *lord_british;
	MapCoord *mapcoord_ref = items.mapcoord_ref;

// can't use obj->is_on_map() since new container gumps will
// have corpse items in an npc's inventoty
	if (!party->has_obj(87, 0, false)) { // make sure orb of moons is in party inventory
		scroll->display_string("\nNot usable\n");
		return true;
	}

	player->get_actor()->get_location(&x, &y, &z);
	lord_british = actor_manager->get_actor(U6_LORD_BRITISH_ACTOR_NUM);

// The player must ask Lord British about the orb before it can be used.
// This sets the flag 0x20 in LB's flags field which allows the orb to be used.

	if ((lord_british->get_talk_flags() & U6_LORD_BRITISH_ORB_CHECK_FLAG) == 0) {
		scroll->display_string("\nYou can't figure out how to use it.\n");
		return true;
	}

// if(!orb_activated)
//   {
//    scroll->display_string("\nYou must recharge it first!\n");
//    return true;
//   }

	if (ev == USE_EVENT_INPUT_CANCEL ||
	        (items.obj_ref && !items.obj_ref->is_on_map())) { // selected item in inventory
		scroll->display_string("Failed\n");
		return true;
	}

	if (!mapcoord_ref) {
		game->get_event()->get_target(MapCoord(x, y, z), "Where: ");
		game->get_event()->request_input(this, obj);
		return false; // no prompt
	}

	ox = mapcoord_ref->x;
	oy = mapcoord_ref->y;
	oz = mapcoord_ref->z;

	px = 3 + ox - x;
	py = 2 + oy - y;

	if (px > 5 || py > 4 ||           // Moongate out of range.
	        items.actor2_ref ||           // Actor at location.
	        !map->is_passable(ox, oy, oz)) { // Location not passable.
		scroll->display_string("Failed.\n");
		return true;
	}

	position = px + py * 5;

	if (position >= 12 && position <= 14) // The three middle positions go noware.
		position = 0;

	gate = new_obj(OBJ_U6_RED_GATE, 1, ox, oy, z);
	gate->quality = position;
	gate->set_temporary();

	new VanishEffect(VANISH_WAIT);
	obj_manager->add_obj(gate, true);
	game->get_map_window()->updateBlacking(); // next update not until Effect completes
	scroll->display_string("a red moon gate appears.\n");

	return true;
}


void U6UseCode::drawbridge_remove(uint16 x, uint16 y, uint8 level, uint16 *bridge_width) {
	uint16 w, h;

//remove end of closed drawbridge.
// if present.
	if (x > 0)
		obj_manager->remove_obj_type_from_location(OBJ_U6_DRAWBRIDGE, x - 1, y, level);

	*bridge_width = 0;

//remove the rest of the bridge.
	for (h = 0, w = 1; w != 0; h++) {
		for (w = 0;; w++) {
			if (obj_manager->remove_obj_type_from_location(OBJ_U6_DRAWBRIDGE, x + w, y + h, level) == false) {
				if (w != 0)
					*bridge_width = w;
				break;
			}
		}
	}

	return;
}

// USE: fishing pole. Attempt to catch a fish from an adjacent water square.
bool U6UseCode::use_fishing_pole(Obj *obj, UseCodeEvent ev) {
	ViewManager *view_manager = game->get_view_manager();
	Actor *player_actor;
	Obj *fish;
	uint16 x, y;
	uint8 z;

	player_actor = player->get_actor();

	player_actor->get_location(&x, &y, &z);

	if (use_find_water(&x, &y, &z) == false) {
		scroll->display_string("\nYou need to stand next to water.\n");
		return true;
	}

	if (NUVIE_RAND() % 100 <= 20) {
		fish = new Obj();

		fish->obj_n = OBJ_U6_FISH;
		if (!player_actor->can_carry_object(fish)) {
			scroll->display_string("\nGot it, but can't carry it.\n");
			if (use_boat_find_land(&x, &y, &z) == false) { //we couldn't find an empty spot for the fish.
				//so back into the water with thee.
				delete fish;
				return true;
			}

			fish->x = x;
			fish->y = y;
			fish->z = z;
			fish->set_ok_to_take(true);
			obj_manager->add_obj(fish);
			return true;
		}

		player_actor->inventory_add_object(fish);

		if (!game->is_new_style())
			view_manager->set_inventory_mode();

		view_manager->update(); //FIX this should be moved higher up in UseCode

		scroll->display_string("\nGot it!\n");
	} else
		scroll->display_string("\nDidn't get a fish.\n");

	return true;
}

inline bool U6UseCode::use_find_water(uint16 *x, uint16 *y, uint8 *z) {
	if (map->is_water(*x, *y - 1, *z)) { //UP
		*y = *y - 1;
		return true;
	}
	if (map->is_water(*x + 1, *y, *z)) { //RIGHT
		*x = *x + 1;
		return true;
	}
	if (map->is_water(*x, *y + 1, *z)) { //DOWN
		*y = *y + 1;
		return true;
	}
	if (map->is_water(*x - 1, *y, *z)) { //LEFT
		*x = *x - 1;
		return true;
	}

	return false;
}


/* Use shovel in one of 8 directions. If used in a dungeon level get a chance of
 * finding gold or a fountain (to make a wish).
 */
bool U6UseCode::use_shovel(Obj *obj, UseCodeEvent ev) {
	Obj *dug_up_obj = nullptr;
	Obj *ladder_obj;
	MapCoord from, dig_at, ladder;

	if (ev == USE_EVENT_INPUT_CANCEL) {
		scroll->display_string("nowhere.\n");
		return true;
	}

	if (!items.mapcoord_ref) { // get direction (FIXME: should return relative dir)
		if (!obj->is_readied()) {
			scroll->display_string("\nNot readied.\n");
			return true;
		}
		if (items.actor_ref == nullptr) { // happens when you use on a widget
			scroll->display_string("nowhere.\n");
			return true;
		}
		Actor *parent = obj->get_actor_holding_obj();
		from = parent->get_location();

//        game->get_event()->useselect_mode(obj, "Direction: ");
		game->get_event()->get_direction(from, "Direction: ");
		if (game->get_map_window()->get_interface() == INTERFACE_NORMAL)
			game->get_event()->do_not_show_target_cursor = true;
		game->get_event()->request_input(this, obj);
		return false;
	}

	Actor *parent = obj->get_actor_holding_obj();
	from = parent->get_location();

	dig_at = *items.mapcoord_ref;

	if (game->get_map_window()->get_interface() < INTERFACE_FULLSCREEN) {
		dig_at.sx = (dig_at.sx == 0) ? 0 : (dig_at.sx < 0) ? -1 : 1;
		dig_at.sy = (dig_at.sy == 0) ? 0 : (dig_at.sy < 0) ? -1 : 1;
	}

	scroll->display_string(get_direction_name(dig_at.x, dig_at.y));
	if (dig_at.sx == 0 && dig_at.sy == 0) {
		scroll->display_string(".\n");
		return true; // display prompt
	}
	scroll->display_string(".\n\n");

	dig_at.x += from.x;
	dig_at.y += from.y;
	dig_at.z = from.z;
	if (!dig_at.is_visible()) {
		scroll->display_string("Not on screen.\n");
		return true; //  display prompt
	} else if (!from.is_visible() && from.distance(dig_at) > 5) {
		scroll->display_string("Out of range.\n");
		return true; //  display prompt
	} else if (game->get_map_window()->get_interface() != INTERFACE_IGNORE_BLOCK) {
		LineTestResult lt;
		if (map->lineTest(from.x, from.y, dig_at.x, dig_at.y, dig_at.z, LT_HitUnpassable, lt)) {
			MapCoord hit_loc = MapCoord(lt.hit_x, lt.hit_y, lt.hit_level);
			if (hit_loc != dig_at) {
				scroll->display_string("Blocked\n");
				return true; // display prompt
			}
		}
	}
	Obj *hole = obj_manager->get_obj_of_type_from_location(OBJ_U6_HOLE, dig_at.x, dig_at.y, dig_at.z);
	if (hole || dig_at.z == 5 // we can't go anywhere from the gargoyle world.
	        || game->get_map_window()->tile_is_black(dig_at.x, dig_at.y)
	        || (dig_at.z == 0 && (dig_at.x != 0x2c3 || dig_at.y != 0x343))) {
		scroll->display_string("No effect\n");
		return true; // ??
	}

	// try to conenct with a ladder on a lower level.
	ladder = dig_at;
// This is to inacurate. It will detect an extra ladder 8 tiles east
// plus another one 8 tiles north and 8 tiles east
	if (dig_at.z == 0) { //handle the transition from the surface to the first dungeon level
		ladder.x = (dig_at.x & 0x07) | (dig_at.x >> 2 & 0xF8);
		ladder.y = (dig_at.y & 0x07) | (dig_at.y >> 2 & 0xF8);
	}

	ladder.z = dig_at.z + 1;

//    if(dig_at.z != 5) already checked
	{
		ladder_obj = obj_manager->get_obj_of_type_from_location(OBJ_U6_LADDER, ladder.x, ladder.y, ladder.z);
		if (ladder_obj && ladder_obj->frame_n == 1) { // ladder up.
			scroll->display_string("You dig a hole.\n");
			dug_up_obj = new_obj(OBJ_U6_HOLE, 0, dig_at.x, dig_at.y, dig_at.z); //found a connecting ladder, dig a hole
		}
	}
	const Tile *tile = map->get_tile(dig_at.x, dig_at.y, dig_at.z, true);
// uncomment first check if the coord conversion gets added back
	if (/*(!dug_up_obj && dig_at.z == 0) ||*/ !tile // original might have checked for earth desc and no wall mask
	        || !((tile->tile_num <= 111 && tile->tile_num >= 108) || tile->tile_num == 540)) {
		scroll->display_string("No Effect.\n");
		return true;
	}

	if (!dug_up_obj) {
		// 10% chance of anything
		if (NUVIE_RAND() % 10) {
			scroll->display_string("Failed\n");
			return true;
		}

		// Door #1 or Door #2?
		Obj *fountain = obj_manager->get_obj_of_type_from_location(OBJ_U6_FOUNTAIN, dig_at.x, dig_at.y, dig_at.z);
		if ((NUVIE_RAND() % 2)) { // original lets you stack fountains
			scroll->display_string("You find a water fountain.\n");
			if (!fountain) // don't actually add another one
				dug_up_obj = new_obj(OBJ_U6_FOUNTAIN, 0, dig_at.x, dig_at.y, dig_at.z);
		} else {
			scroll->display_string("You find a gold nugget.\n");
			dug_up_obj = new_obj(OBJ_U6_GOLD_NUGGET, 0, dig_at.x, dig_at.y, dig_at.z);
			dug_up_obj->status |= OBJ_STATUS_OK_TO_TAKE;
		}
	}
	if (dug_up_obj) {
		dug_up_obj->set_temporary();
		obj_manager->add_obj(dug_up_obj, true);
	}
	return true;
}


/* USE: Magic fountain. Make a wish!
 */
bool U6UseCode::use_fountain(Obj *obj, UseCodeEvent ev) {
	static bool get_wish = false;
	static Actor *wish_actor = nullptr; // person receiving gift

	scroll->cancel_input_request();
	if (ev == USE_EVENT_USE) {
		scroll->display_string("Make a wish? ");
		// get Y/N single char, no ENTER (FIXME: no printing)
		scroll->set_input_mode(true, "yn", false);
		scroll->request_input(this, obj);
		wish_actor = items.actor_ref;
		assert(wish_actor);
		return false;
	} else if (ev == USE_EVENT_MESSAGE && items.string_ref) {
		scroll->display_string("\n");
		if (!get_wish) { // answered with Y/N
			// Y:
			if (*items.string_ref == "y" || *items.string_ref == "Y") {
				scroll->display_string("Wish for: ");
				// get string
				scroll->set_input_mode(true);
				scroll->request_input(this, obj);
				get_wish = true;
			} else { // N: won't wish
				scroll->display_string("\n");
				scroll->display_prompt();
			}
		} else { // answered with wish
			get_wish = false;
			bool wished_for_food = false;
			size_t wishSize = items.string_ref->size() + 1;
			char *wish = (char *)malloc(wishSize);
			Common::strcpy_s(wish, wishSize, items.string_ref->c_str());
			if (scumm_stricmp(wish, "Food") == 0 || scumm_stricmp(wish, "Mutton") == 0
			        || scumm_stricmp(wish, "Wine") == 0 || scumm_stricmp(wish, "Fruit") == 0
			        || scumm_stricmp(wish, "Mead") == 0)
				wished_for_food = true;
			free(wish);
			if (!wished_for_food) {
				scroll->display_string("\nFailed\n\n");
				scroll->display_prompt();
				return true;
			}
			// 25% chance of anything
			if ((NUVIE_RAND() % 4) != 0) {
				scroll->display_string("\nNo effect\n\n");
				scroll->display_prompt();
				return true;
			}
			scroll->display_string("\nYou got food");
			// must be able to carry it
			if (!wish_actor->can_carry_object(OBJ_U6_MEAT_PORTION, 1)) {
				scroll->display_string(", but you can't carry it.\n\n");
				scroll->display_prompt();
				return true;
			}
			scroll->display_string(".\n\n");
			scroll->display_prompt();
			assert(wish_actor);
			wish_actor->inventory_new_object(OBJ_U6_MEAT_PORTION, 1);
		}
	} else
		get_wish = false;
	return false;
}


/* USE: Make a rubber ducky sound. */
bool U6UseCode::use_rubber_ducky(Obj *obj, UseCodeEvent ev) {
	if (items.actor_ref == player->get_actor())
		scroll->display_string("\nSqueak!\n");
	Game::get_game()->get_sound_manager()->playSfx(NUVIE_SFX_RUBBER_DUCK); //FIXME towns says "Quack! Quack!" and plays sfx twice.
	return true;
}

sint16 U6UseCode::parseLatLongString(U6UseCodeLatLonEnum mode, Std::string *input) {
	uint16 len = input->length();
	sint16 val = 0;
	for (uint16 i = 0; i < len; i++) {
		char c = (*input)[i];
		if (c < '0' || c > '9') {
			c = toupper(c);
			if (mode == LAT) {
				if (c == 'N' || c == 'S') {
					if (c == 'N')
						val = -val;
				} else {
					val = 100;
				}
			} else {
				if (c == 'E' || c == 'W') {
					if (c == 'W')
						val = -val;
				} else {
					val = 100;
				}
			}
			break;
		}

		val = val * 10 + (*input)[i] - 48;
	}

	return val;
}

/* USE: Crystal ball
 */
bool U6UseCode::use_crystal_ball(Obj *obj, UseCodeEvent ev) {
	static enum { GET_LAT, GET_LON} mode = GET_LAT;
	static MapCoord loc;
	static Actor *actor = nullptr;

	scroll->cancel_input_request();
	if (ev == USE_EVENT_USE) {
		actor = items.actor_ref;
		if ((int)NUVIE_RAND() % 30 < (45 - actor->get_intelligence()) / 2) { //use crystal ball saving roll.
			game->get_script()->call_actor_hit(actor, (NUVIE_RAND() % 10) + 1, SCRIPT_DISPLAY_HIT_MSG);
			scroll->display_string("\n");
			scroll->display_prompt();
			return false;
		}

		mode = GET_LAT;
		scroll->display_string("Enter degrees followed by N, S, E or W.\n\nAt latitude=");
		scroll->set_input_mode(true);
		scroll->request_input(this, obj);

		return false;
	} else if (ev == USE_EVENT_MESSAGE && items.string_ref) {
		if (mode == GET_LAT) {
			sint16 lat = parseLatLongString(LAT, items.string_ref);
			if (lat > 80 || lat < -44) {
				scroll->display_string("\n\n");
				scroll->display_prompt();
				return false;
			}

			loc.y = lat * 8 + 360;
			scroll->display_string("\n");
			scroll->display_string("  longitude=");
			scroll->set_input_mode(true);
			scroll->request_input(this, obj);
			mode = GET_LON;
		} else if (mode == GET_LON) {
			scroll->display_string("\n");
			sint16 lon = parseLatLongString(LON, items.string_ref);
			if (lon > 88 || lon < -37) {
				scroll->display_string("\n\n");
				scroll->display_prompt();
				return false;
			}

			loc.x = lon * 8 + 304;

			actor->get_location(nullptr, nullptr, &loc.z);
			if (loc.z != 0) {
				loc.x = loc.x / 4;
				loc.y = loc.y / 4;
			}

			AsyncEffect *e = new AsyncEffect(new WizardEyeEffect(loc, 0x28));
			e->run(EFFECT_PROCESS_GUI_INPUT);
			scroll->display_string("\nDone\n\n");
			scroll->display_prompt();
		}

	}

	return false;
}

/* USE: Enter instrument playing mode, with sound for used object. */
bool U6UseCode::play_instrument(Obj *obj, UseCodeEvent ev) {
// FIXME: need instrument sounds AND a config option to simply change music
// track when an instrument is played. Maybe NORTH_KEY and SOUTH_KEY can cycle through sounds/music and DO_ACTION_KEY can play it.
/// FIXME: also some floating music note icons like in U7
	game->get_event()->close_gumps(); // gumps will steal input
	const char *musicmsg = (obj->obj_n == OBJ_U6_PANPIPES) ? "panpipes"
	                       : (obj->obj_n == OBJ_U6_HARPSICHORD) ? "harpsichord"
	                       : (obj->obj_n == OBJ_U6_HARP) ? "harp"
	                       : (obj->obj_n == OBJ_U6_LUTE) ? "lute"
	                       : (obj->obj_n == OBJ_U6_XYLOPHONE) ? "xylophone"
	                       : "musical instrument";
	if (items.data_ref) {
		Common::KeyCode key = ((EventInput *)items.data_ref)->key;
		ActionKeyType key_type = ((EventInput *)items.data_ref)->action_key_type;
		if (key == Common::KEYCODE_0) DEBUG(0, LEVEL_WARNING, "FIXME: %s: modulate 0\n", musicmsg);
		if (key == Common::KEYCODE_1) DEBUG(0, LEVEL_WARNING, "FIXME: %s: modulate 1\n", musicmsg);
		if (key == Common::KEYCODE_2) DEBUG(0, LEVEL_WARNING, "FIXME: %s: modulate 2\n", musicmsg);
		if (key == Common::KEYCODE_3) DEBUG(0, LEVEL_WARNING, "FIXME: %s: modulate 3\n", musicmsg);
		if (key == Common::KEYCODE_4) DEBUG(0, LEVEL_WARNING, "FIXME: %s: modulate 4\n", musicmsg);
		if (key == Common::KEYCODE_5) DEBUG(0, LEVEL_WARNING, "FIXME: %s: modulate 5\n", musicmsg);
		if (key == Common::KEYCODE_6) DEBUG(0, LEVEL_WARNING, "FIXME: %s: modulate 6\n", musicmsg);
		if (key == Common::KEYCODE_7) DEBUG(0, LEVEL_WARNING, "FIXME: %s: modulate 7\n", musicmsg);
		if (key == Common::KEYCODE_8) DEBUG(0, LEVEL_WARNING, "FIXME: %s: modulate 8\n", musicmsg);
		if (key == Common::KEYCODE_9) DEBUG(0, LEVEL_WARNING, "FIXME: %s: modulate 9\n", musicmsg);
		return (key_type != DO_ACTION_KEY && key_type != CANCEL_ACTION_KEY);
	} else
		game->get_event()->key_redirect(this, obj);
	return false;
}


// use_firedevice()
bool U6UseCode::use_firedevice_message(Obj *obj, bool lit) {
	if (items.actor_ref != player->get_actor())
		return true;
	scroll->display_string("\n");
	scroll->display_string(obj_manager->get_obj_name(obj));
	if (lit)
		scroll->display_string(" is lit.\n");
	else
		scroll->display_string(" is doused.\n");

	return true;
}

/* USE: Eat/drink food object. Hic!
 */
bool U6UseCode::use_food(Obj *obj, UseCodeEvent ev) {
	if (ev == USE_EVENT_USE) {
		if (items.actor_ref == player->get_actor()) {
			if (obj->obj_n == OBJ_U6_WINE || obj->obj_n == OBJ_U6_MEAD
			        || obj->obj_n == OBJ_U6_ALE) {
				scroll->display_string("\nYou drink it.\n");

				player->add_alcohol(); // add to drunkenness
			} else
				scroll->display_string("\nYou eat the food.\n");
		}
		destroy_obj(obj, 1);
	}
	return true;
}


/* USE: Use potion. If actor2 is passed, give them the potion, else select
 * actor2. */
bool U6UseCode::use_potion(Obj *obj, UseCodeEvent ev) {
	ActorManager *am = actor_manager;

	if (ev == USE_EVENT_USE) {
		if (!items.actor2_ref && !items.obj_ref && !items.mapcoord_ref) {
			game->get_event()->get_target(items.actor_ref->get_location(), "On whom: ");
			game->get_event()->request_input(this, obj);
		} else if (!items.actor2_ref) { // no selection
			scroll->display_string("nobody\n");
			return true;
		} else { // use potion
			sint8 party_num = party->get_member_num(items.actor2_ref);
			scroll->display_string(party_num >= 0 ? party->get_actor_name(party_num)
			                       : am->look_actor(items.actor2_ref));
			scroll->display_string("\n");

			if (party_num < 0) // can't force potions on non-party members
				scroll->display_string("No effect\n");
			else {
				switch (obj->frame_n) {
				case USE_U6_POTION_RED:
					((U6Actor *)items.actor2_ref)->set_poisoned(false);
					destroy_obj(obj);
					break;
				case USE_U6_POTION_YELLOW:
					((U6Actor *)items.actor2_ref)->heal();
					destroy_obj(obj);
					break;
				case USE_U6_POTION_GREEN:
					((U6Actor *)items.actor2_ref)->set_poisoned(true);
					destroy_obj(obj);
					break;
				case USE_U6_POTION_BLUE:
					((U6Actor *)items.actor2_ref)->set_asleep(false);
					destroy_obj(obj);
					break;
				case USE_U6_POTION_PURPLE:
					((U6Actor *)items.actor2_ref)->set_protected(true);
					destroy_obj(obj);
					break;
				case USE_U6_POTION_WHITE:
					new U6WhitePotionEffect(2500, 6000, obj);
					break; // wait for message to delete potion
				case USE_U6_POTION_BLACK:
					//new SpellTargetEffect(items.actor2_ref, obj);
					// or effect_mgr->wait_for_effect(new SpellTargetEffect(items.actor2_ref), this, obj);
					items.actor2_ref->set_invisible(true);
					destroy_obj(obj);
					break;
				case USE_U6_POTION_ORANGE:
					//items.actor2_ref->set_worktype(WORKTYPE_U6_SLEEP);
					items.actor2_ref->set_asleep(true);
					//party->set_active(party_num, !(items.actor2_ref->is_sleeping() || items.actor2_ref->is_paralyzed()));
					player->set_actor(party->get_leader_actor());
					player->set_mapwindow_centered(true);
					destroy_obj(obj);
					break;

				default:
					if (obj->frame_n <= 7) {
						scroll->display_string("Drink %s potion!\n", u6_potions[obj->frame_n]);
						//scroll->display_string(u6_potions[obj->frame_n]);
						//scroll->display_string(" potion!\n");
					} else
						scroll->display_string("\nNo effect\n");
					destroy_obj(obj);
				}
			}
			return true;
		}
	} else if (ev == USE_EVENT_INPUT_CANCEL) {
		scroll->display_string("No effect\n");
		return true;
	} else if (ev == USE_EVENT_MESSAGE) { // assume message is from potion effect
		if (*items.msg_ref == MESG_EFFECT_COMPLETE && obj->frame_n == USE_U6_POTION_WHITE) { // white
			destroy_obj(obj);
		}
	}
	return false;
}

bool U6UseCode::lock_pick_dex_check() {
	int dex = player->get_actor()->get_dexterity();
	if (player->get_actor()->is_cursed()) {
		if (dex <= 3)
			dex = 1;
		else
			dex -= 3;
	}

	if ((int)NUVIE_RAND() % 30 < (45 - dex) / 2)
		return true;

	return false;
}

/* Use a key on obj_ref (a door). */
bool U6UseCode::use_key(Obj *obj, UseCodeEvent ev) {
	Obj *door_obj = nullptr;
	if (ev == USE_EVENT_USE) {
		USECODE_SELECT_OBJ(door_obj, "On "); // door_obj <- items.obj_ref or from user
		if (!door_obj) {
			scroll->display_string("nothing\n");
			return true;
		} else {
			if (UseCode::out_of_use_range(door_obj, false))
				return true;

			scroll->display_string(obj_manager->get_obj_name(door_obj));
			scroll->display_string("\n");

			if (!is_door(door_obj) && !is_chest(door_obj)) {
				scroll->display_string("No effect\n");
				return true;
			}

			if (obj->obj_n == OBJ_U6_LOCK_PICK && lock_pick_dex_check() == true) {
				Game::get_game()->get_sound_manager()->playSfx(NUVIE_SFX_FAILURE);
				scroll->display_string("\nKey broke.\n");
				if (obj->qty > 1) {
					obj->qty -= 1;
				} else {
					UseCode::search_container(obj, false); //need to remove rolling pin if there is one
					obj_manager->unlink_from_engine(obj);
					delete_obj(obj);
				}

				return true;
			}

//FIXME need to handle locked chests.
			if (((obj->obj_n == OBJ_U6_KEY && door_obj->quality != 0 && door_obj->quality == obj->quality)
			        || (obj->obj_n == OBJ_U6_LOCK_PICK && door_obj->quality == 0))
			        && (is_closed_door(door_obj) || is_closed_chest(door_obj))
			        && !is_magically_locked(door_obj)) {
				if (is_locked(door_obj)) {
					unlock(door_obj);
					scroll->display_string("\nunlocked!\n");
				} else {
					lock(door_obj);
					scroll->display_string("\nlocked!\n");
				}
			} else if (is_door(door_obj) && door_obj->frame_n <= 3
			           && ((obj->obj_n == OBJ_U6_KEY && door_obj->quality != 0 && door_obj->quality == obj->quality)
			               || (obj->obj_n == OBJ_U6_LOCK_PICK && door_obj->quality == 0)))
				scroll->display_string("\nCan't (Un)lock an opened door\n");
			else
				scroll->display_string("\nNo effect\n");
			return true;
		}
	} else if (ev == USE_EVENT_INPUT_CANCEL) {
		scroll->display_string("nothing\n");
		return true;
	} else if (ev == USE_EVENT_GET && obj->obj_n == OBJ_U6_LOCK_PICK) { //need to remove rolling pin if there is one
		UseCode::search_container(obj, false);
		return true;
	} else if (ev == USE_EVENT_SEARCH && obj->obj_n == OBJ_U6_LOCK_PICK) //need to remove rolling pin if there is one
		return UseCode::search_container(obj);

	return false;
}


/* USE: Enter and exit sea-going vessels. (entire party)
 */
bool U6UseCode::use_boat(Obj *obj, UseCodeEvent ev) {
	Actor *ship_actor;
	uint16 lx, ly;
	uint8 lz;

	if (ev == USE_EVENT_SEARCH)
		return UseCode::search_container(obj);
	else if (ev == USE_EVENT_USE && obj->has_container())
		return use_container(obj, USE_EVENT_USE);
	else if (ev == USE_EVENT_LOOK || ev == USE_EVENT_GET) {
		if (obj->quality != 0 && party->has_obj(OBJ_U6_SHIP_DEED, obj->quality)) {
			if (obj->obj_n == OBJ_U6_SKIFF)
				obj->set_ok_to_take(true);
			obj->quality = 0;
		}
		if (ev == USE_EVENT_GET)
			return true;
	}

	if (ev != USE_EVENT_USE)
		return false;
	ship_actor = actor_manager->get_actor(0); //get the vehicle actor.

// get out of boat
	if (party->is_in_vehicle()) {
		ship_actor->get_location(&lx, &ly, &lz); //retrieve actor position for land check.

		if (use_boat_find_land(&lx, &ly, &lz)) { //we must be next to land to disembark
			Obj *objP = ship_actor->make_obj();
			objP->qty = ship_actor->get_hp(); // Hull Strength

			party->exit_vehicle(lx, ly, lz);

			obj_manager->add_obj(objP);
		} else {
			scroll->display_string("\nOnly next to land.\n");
			return true;
		}

		return true;
	}

	if (obj->is_on_map() == false) {
		scroll->display_string("\nNot usable\n");
		return true;
	}
	if ((obj->obj_n == OBJ_U6_SKIFF || obj->obj_n == OBJ_U6_RAFT)
	        && !map->is_water(obj->x, obj->y, obj->z, true)) {
		scroll->display_string("\nYou must place it in water first.\n");
		return true;
	}
	if (!player->in_party_mode()) {
		scroll->display_string("\nNot in solo mode.\n");
		return true;
	}

	if (obj->obj_n == OBJ_U6_SHIP) { //If we are using a ship we need to use its center object.
		obj = use_boat_find_center(obj); //return the center object
		if (obj == nullptr) {
			scroll->display_string("\nShip not usable\n");
			return true;
		}
	}

	if (obj->quality != 0) { //deed check
		if (party->has_obj(OBJ_U6_SHIP_DEED, obj->quality) == false) {
			scroll->display_string("\nA deed is required.\n");
			return true;
		}
		if (obj->obj_n == OBJ_U6_SKIFF)
			obj->set_ok_to_take(true);
		obj->quality = 0;
	}

	if (UseCode::out_of_use_range(obj, true))
		return true;

// walk to vehicle if necessary
	if (!party->is_at(obj->x, obj->y, obj->z)) {
		party->enter_vehicle(obj);
		return true;
	}

// use it (replace ship with vehicle actor)
	ship_actor->init_from_obj(obj, ACTOR_CHANGE_BASE_OBJ_N);
	if (obj->obj_n == OBJ_U6_SHIP)
		ship_actor->set_hp(obj->qty); // Hull Strength
	ship_actor->show(); // Swift!
	obj_manager->remove_obj_from_map(obj);
	delete_obj(obj);

	party->hide(); // set in-vehicle
	player->set_actor(ship_actor);
	party->set_in_vehicle(true);
	return true;
}

inline Obj *U6UseCode::use_boat_find_center(Obj *obj) {
	Obj *new_obj;
	uint16 x, y;
	uint8 ship_direction = (obj->frame_n % 8) / 2; //find the direction based on the frame_n

	if (obj->frame_n >= 8 && obj->frame_n < 16) //center obj
		return obj;

	x = obj->x;
	y = obj->y;

	if (obj->frame_n < 8) { //front obj
		switch (ship_direction) {
		case NUVIE_DIR_N :
			y++;
			break;
		case NUVIE_DIR_E :
			x--;
			break;
		case NUVIE_DIR_S :
			y--;
			break;
		case NUVIE_DIR_W :
			x++;
			break;
		}
	} else {
		if (obj->frame_n >= 16 && obj->frame_n < 24) { //back obj
			switch (ship_direction) {
			case NUVIE_DIR_N :
				y--;
				break;
			case NUVIE_DIR_E :
				x++;
				break;
			case NUVIE_DIR_S :
				y++;
				break;
			case NUVIE_DIR_W :
				x--;
				break;
			}
		}
	}

	new_obj = obj_manager->get_objBasedAt(x, y, obj->z, true);

	if (new_obj != nullptr && new_obj->obj_n == OBJ_U6_SHIP)
		return new_obj;

	return nullptr;
}

inline bool U6UseCode::use_boat_find_land(uint16 *x, uint16 *y, uint8 *z) {
	if (map->is_passable(*x, *y - 1, *z)) { //UP
		*y = *y - 1;
		return true;
	}
	if (map->is_passable(*x + 1, *y, *z)) { //RIGHT
		*x = *x + 1;
		return true;
	}
	if (map->is_passable(*x, *y + 1, *z)) { //DOWN
		*y = *y + 1;
		return true;
	}
	if (map->is_passable(*x - 1, *y, *z)) { //LEFT
		*x = *x - 1;
		return true;
	}

	return false;
}

/* construct a balloon using the balloon plans */
bool U6UseCode::use_balloon_plans(Obj *obj, UseCodeEvent ev) {
	if (ev == USE_EVENT_LOOK)
		return look_sign(obj, ev);

	MapCoord player_location = player->get_actor()->get_location();
	bool missing_obj = false;
	Obj *balloon;

	if (ev != USE_EVENT_USE)
		return false;

	scroll->display_string("\n");

//make sure the party is carrying the required parts.

	if (!party->has_obj(OBJ_U6_MAMMOTH_SILK_BAG, 0)) {
		scroll->display_string("Missing a mammoth silk bag.\n");
		missing_obj = true;
	}

	if (!party->has_obj(OBJ_U6_BALLOON_BASKET, 0)) {
		scroll->display_string("Missing a balloon basket.\n");
		missing_obj = true;
	}

	if (!party->has_obj(OBJ_U6_CAULDRON, 0)) {
		scroll->display_string("Missing a cauldron.\n");
		missing_obj = true;
	}

	if (!party->has_obj(OBJ_U6_ROPE, 0)) {
		scroll->display_string("Missing a rope.\n");
		missing_obj = true;
	}

// Make the balloon if we have all the parts.
	if (!missing_obj) {
		party->remove_obj(OBJ_U6_MAMMOTH_SILK_BAG, 0);
		party->remove_obj(OBJ_U6_BALLOON_BASKET, 0);
		party->remove_obj(OBJ_U6_CAULDRON, 0);
		party->remove_obj(OBJ_U6_ROPE, 0);

		balloon = new_obj(OBJ_U6_BALLOON, 0, player_location.x, player_location.y, player_location.z);

		if (balloon && obj_manager->add_obj(balloon)) {
			balloon->set_ok_to_take(true);
			scroll->display_string("Done!\n");
		}
	}

	return true;
}


/* USE: balloon. (entire party)
 */
bool U6UseCode::use_balloon(Obj *obj, UseCodeEvent ev) {
	Actor *balloon_actor;
	Actor *balloonist;
	MapCoord spot(0, 0, 0);
	uint16 lx, ly;
	uint8 lz;

	if (ev != USE_EVENT_USE)
		return false;

	if (Game::get_game()->get_player()->in_party_mode()) {
		balloonist = Game::get_game()->get_party()->get_leader_actor();
	} else {
		balloonist = Game::get_game()->get_player()->get_actor();
	}
	spot = balloonist->get_location();
	if ((spot.z > 0) && (spot.z < 5)) {
		scroll->display_string("\nNot usable\n");
		return true;
	}

	if (obj->obj_n == OBJ_U6_BALLOON) {
		if (!obj->is_on_map()) {
			// if in party mode, find a spot around the avatar that is_passable,
			// else a spot around the person using it.
			// drop the balloon there, and inflate it.

			uint16 x, y;
			x = spot.x;
			y = spot.y;

			bool dropped = false;
			for (sint8 iy = -1; iy < 2; iy++) { // FIXME scan order
				for (sint8 ix = -1; ix < 2; ix++) {
					DEBUG(0, LEVEL_DEBUGGING, "can drop at %d %d?\n", ix, iy);
					if (Game::get_game()->get_map_window()->can_drop_or_move_obj(x + ix, y + iy, balloonist, obj) == MSG_SUCCESS) {
						DEBUG(0, LEVEL_DEBUGGING, "yes, can drop at %d %d.\n", x + ix, y + iy);
						obj_manager->unlink_from_engine(obj);
						obj->x = x + ix;
						obj->y = y + iy;
						obj->z = spot.z;

						dropped = true;
						iy = 2;
						ix = 2;
					}
				}
			}
			if (!dropped) {
// drop on 'spot' instead.
				obj_manager->unlink_from_engine(obj);
				obj->x = spot.x;
				obj->y = spot.y;
				obj->z = spot.z;
				dropped = true;
			}
			obj->status |= OBJ_STATUS_OK_TO_TAKE;
			obj_manager->add_obj(obj, OBJ_ADD_TOP);
		}

		obj->obj_n = OBJ_U6_INFLATED_BALLOON;
		obj->frame_n = 3;
		scroll->display_string("\nDone!\n");
		return true;
	}

	balloon_actor = actor_manager->get_actor(0); //get the vehicle actor.

// get out of balloon
	if (party->is_in_vehicle()) { // FIXME: use balloon when in skiff...
		balloon_actor->get_location(&lx, &ly, &lz); //retrieve actor position for land check.

		if (use_boat_find_land(&lx, &ly, &lz)) { //we must be next to land to disembark
			Obj *objP;

			party->show();
			balloon_actor->hide();
			balloon_actor->set_worktype(0);

			player->set_actor(party->get_actor(0));
			player->move(lx, ly, lz, false);
			balloon_actor->obj_n = OBJ_U6_NO_VEHICLE;
			balloon_actor->frame_n = 0;
			balloon_actor->init();
			balloon_actor->move(0, 0, 0, ACTOR_FORCE_MOVE);

			objP = new_obj(OBJ_U6_BALLOON, 0, lx, ly, lz);
			objP->status |= OBJ_STATUS_OK_TO_TAKE;
			obj_manager->add_obj(objP, OBJ_ADD_TOP);
		} else {
			scroll->display_string("\nOnly next to land.\n");
			return true;
		}

		party->set_in_vehicle(false);

		return true;
	}

	if (!player->in_party_mode()) {
		scroll->display_string("\nNot in solo mode.\n");
		return true;
	}

	if (UseCode::out_of_use_range(obj, true))
		return true;
// walk to vehicle if necessary
	if (!party->is_at(obj->x, obj->y, obj->z)) {
		party->enter_vehicle(obj);
		return true; // display prompt
	}

// use it (replace ship with vehicle actor)
	balloon_actor->init_from_obj(obj, ACTOR_CHANGE_BASE_OBJ_N);
	balloon_actor->show(); // Swift!
	obj_manager->remove_obj_from_map(obj);
	delete_obj(obj);

	party->hide(); // set in-vehicle
	player->set_actor(balloon_actor);
	party->set_in_vehicle(true);
	return true;
}


/* using a cow fills an empty bucket in the player's inventory with milk */
bool U6UseCode::use_cow(Obj *obj, UseCodeEvent ev) {
	if (ev != USE_EVENT_USE)
		return false;

// return fill_bucket(OBJ_U6_BUCKET_OF_MILK);
	fill_bucket(OBJ_U6_BUCKET_OF_MILK);
	return true;
}

/* using a well fills an empty bucket in the player's inventory with water */
bool U6UseCode::use_well(Obj *obj, UseCodeEvent ev) {
	if (ev != USE_EVENT_USE)
		return false;

// return fill_bucket(OBJ_U6_BUCKET_OF_WATER);
	fill_bucket(OBJ_U6_BUCKET_OF_WATER);
	return true;
}

// fill an empty bucket in the player actor's inventory with some liquid
bool U6UseCode::fill_bucket(uint16 filled_bucket_obj_n) {
	Actor *player_actor;
	Obj *bucket;

	player_actor = player->get_actor();

	if (!player_actor->inventory_has_object(OBJ_U6_BUCKET)) {
		if (player_actor->inventory_has_object(OBJ_U6_BUCKET_OF_MILK) ||
		        player_actor->inventory_has_object(OBJ_U6_BUCKET_OF_WATER)) {
			scroll->display_string("\nYou need an empty bucket.\n");
			return true;
		} else {
			scroll->display_string("\nYou need a bucket.\n");
			return true;
		}
	}

// Fill the first empty bucket in player's inventory.

	bucket = player_actor->inventory_get_object(OBJ_U6_BUCKET);
	player_actor->inventory_remove_obj(bucket);

	bucket->obj_n = filled_bucket_obj_n;

	player_actor->inventory_add_object(bucket);

	scroll->display_string("\nDone\n");

	return true;
}


// USE: replace a bucket of milk in the player's inventory with butter
bool U6UseCode::use_churn(Obj *obj, UseCodeEvent ev) {
	ViewManager *view_manager = game->get_view_manager();
	Actor *player_actor;
	Obj *bucket;
	Obj *butter;

	player_actor = player->get_actor();

	if (!player_actor->inventory_has_object(OBJ_U6_BUCKET_OF_MILK)) {
		scroll->display_string("\nYou need some milk.\n");
		return true;
	}

	bucket = player_actor->inventory_get_object(OBJ_U6_BUCKET_OF_MILK);
	player_actor->inventory_remove_obj(bucket);

	bucket->obj_n = OBJ_U6_BUCKET;

	butter = new Obj();

	butter->obj_n = OBJ_U6_BUTTER;
	player_actor->inventory_add_object(butter);
	player_actor->inventory_add_object(bucket);
	if (!game->is_new_style())
		view_manager->set_inventory_mode();

	view_manager->update(); //FIX this should be moved higher up in UseCode

	scroll->display_string("\nDone\n");
	return true;
}


// USE: fill an empty honey jar in the player's inventory
bool U6UseCode::use_beehive(Obj *obj, UseCodeEvent ev) {
	ViewManager *view_manager = game->get_view_manager();
	Actor *player_actor;
	Obj *honey_jar;

	player_actor = player->get_actor();

	if (!player_actor->inventory_has_object(OBJ_U6_HONEY_JAR)) {
		if (player_actor->inventory_has_object(OBJ_U6_JAR_OF_HONEY)) {
			scroll->display_string("\nYou need an empty honey jar.\n");
		} else {
			scroll->display_string("\nYou need a honey jar.\n");
		}

		return true;
	}

	honey_jar = player_actor->inventory_get_object(OBJ_U6_HONEY_JAR);
	player_actor->inventory_remove_obj(honey_jar);

	honey_jar->obj_n = OBJ_U6_JAR_OF_HONEY; //fill the empty jar with honey.

	player_actor->inventory_add_object(honey_jar);   // add the filled jar back to the player's inventory
	if (!game->is_new_style())
		view_manager->set_inventory_mode();

	view_manager->update(); //FIX this should be moved higher up in UseCode

	scroll->display_string("\nDone\n");
	return true;
}


/* USE: Mount or dismount from a horse. Don't allow using another horse if
 * already riding one.
 */
bool U6UseCode::use_horse(Obj *obj, UseCodeEvent ev) {
	Actor *actor, *player_actor;
	Obj *actor_obj;

	if (ev != USE_EVENT_USE)
		return false;

	actor = actor_manager->get_actor(obj->quality); // horse or horse with rider
	if (!actor)
		return false;

	player_actor = items.actor_ref;
	if (player_actor->get_actor_num() == U6_SHERRY_ACTOR_NUM) {
		scroll->display_string("Sherry says: \"Eeek!!! I'm afraid of horses!\"\n");
		return true;
	} else if (player_actor->get_actor_num() == U6_BEHLEM_ACTOR_NUM) {
		scroll->display_string("BehLem says: \"Horses are for food!\"\n");
		return true;
	} else if (obj->obj_n == OBJ_U6_HORSE && player_actor->obj_n == OBJ_U6_HORSE_WITH_RIDER) {
		scroll->display_string("You're already on a horse!\n");
		return true;
	} else if (party->is_in_vehicle()) {
		Game::get_game()->get_event()->display_not_aboard_vehicle(false);
		return true;
	}

	actor_obj = actor->make_obj();

//dismount from horse. revert to original actor type.
//Add a temporary horse actor onto the map.
	if (obj->obj_n == OBJ_U6_HORSE_WITH_RIDER) {
		actor->clear();
		if (actor == player_actor)
			actor->set_worktype(0x02); // PLAYER

		actor_obj->obj_n = actor->base_obj_n; //revert to normal actor type
		actor_obj->frame_n = actor->old_frame_n;

		actor->init_from_obj(actor_obj);

		// create a temporary horse on the map.
		actor_manager->create_temp_actor(OBJ_U6_HORSE, NO_OBJ_STATUS, obj->x, obj->y, obj->z, ACTOR_ALIGNMENT_DEFAULT, WORKTYPE_U6_ANIMAL_WANDER);
	} else if (!actor_manager->is_temp_actor(actor)) { // Try to mount horse. Don't use permanent Actors eg Smith, push-me pull-you
		scroll->display_string("\nHorse not boardable!\n");
	} else { // mount up.
		if (UseCode::out_of_use_range(obj, true))
			return true;
		actor_manager->clear_actor(actor); //clear the temp horse actor from the map.

		actor_obj->obj_n = OBJ_U6_HORSE_WITH_RIDER;

		player_actor->move(actor_obj->x, actor_obj->y, actor_obj->z); //this will center the map window
		player_actor->init_from_obj(actor_obj);

		delete_obj(actor_obj);
	}

	return true;
}

bool U6UseCode::use_fan(Obj *obj, UseCodeEvent ev) {
	// Directions rotated clockwise by 45 deg.
	NuvieDir next_wind_dir_tbl[] = {
		NUVIE_DIR_NE, NUVIE_DIR_SE, NUVIE_DIR_SW, NUVIE_DIR_NW,
		NUVIE_DIR_E, NUVIE_DIR_S, NUVIE_DIR_W, NUVIE_DIR_N};
	Weather *weather = game->get_weather();
	scroll->display_string("\nYou feel a breeze.\n");

	NuvieDir wind_dir = weather->get_wind_dir();

	if (wind_dir == NUVIE_DIR_NONE)
		wind_dir = NUVIE_DIR_NW;

	//cycle through the wind directions.
	weather->set_wind_dir(next_wind_dir_tbl[wind_dir]);

	return true;
}

/* USE: Sextant. Display Latitude/Longitude coords centered on LB's castle.
 */
bool U6UseCode::use_sextant(Obj *obj, UseCodeEvent ev) {
	MapCoord location;
	char buf[18]; // "\nxxoS, xxoE\n"
	char lat, lon;
	uint16 x, y;

	if (ev != USE_EVENT_USE)
		return false;

	location = player->get_actor()->get_location();

//only use sextant on surface level or in the gargoyle underworld.
	if (location.z == 0 || location.z == 5) {
		x = location.x / (location.z ? 2 : 8);
		if (x > 38) {
			lon = 'E';
			x -= 38;
		} else {
			x = 38 - x;
			lon = 'W';
		}

		y = location.y / (location.z ? 2 : 8);
		if (y > 45) {
			lat = 'S';
			y -= 45;
		} else {
			y = 45 - y;
			lat = 'N';
		}

		Common::sprintf_s(buf, "\n%d{%c, %d{%c\n", y, lat, x, lon);
		scroll->display_string(buf);
	} else
		scroll->display_string("\nNot usable\n");

	return true;
}

bool U6UseCode::use_staff(Obj *obj, UseCodeEvent ev) {
	if (ev != USE_EVENT_USE)
		return false;

	if (obj->is_readied() == false) {
		scroll->display_string("\nNot readied.\n");
		return true;
	}

	Obj *charge = obj->find_in_container(OBJ_U6_CHARGE, 0, OBJ_NOMATCH_QUALITY);

	if (charge) {
		uint8 spell_num = charge->quality;
		obj_manager->unlink_from_engine(charge);
		delete_obj(charge);

		Game::get_game()->get_event()->cast_spell_directly(spell_num);
	}

	return true;
}

/* Pass: Allow normal move if player's Quest Flag is set.
 */
bool U6UseCode::pass_quest_barrier(Obj *obj, UseCodeEvent ev) {
	if (ev == USE_EVENT_PASS)
		if (player->get_quest_flag() == 0) {
			// block everyone, only print message when player attempts to pass
			if (items.actor_ref == player->get_actor())
				scroll->message("\n\"Thou art not upon a Sacred Quest!\n"
				                "Passage denied!\"\n\n");
			return false;
		}
	return true;
}


/* LOOK: Get (possibly translate) book data for readable object. Disallow search
 * (return true) if book data was displayed.
 */
bool U6UseCode::look_sign(Obj *obj, UseCodeEvent ev) {
	char *data;
	Book *book = game->get_book(); // ??

	if (ev == USE_EVENT_LOOK) {
		MapCoord obj_loc = MapCoord(obj->x, obj->y, obj->z);
		MapCoord player_loc = player->get_actor()->get_location();
		InterfaceType interface = game->get_map_window()->get_interface();
		bool too_far = (player_loc.distance(obj_loc) > 1 && interface == INTERFACE_NORMAL);
		bool blocked = (interface != INTERFACE_IGNORE_BLOCK
		                && !game->get_map_window()->can_get_obj(player->get_actor(), obj));
		if ((obj->quality == 0 && obj->obj_n != OBJ_U6_BOOK) || (!obj->is_in_inventory()
		        && (obj->obj_n == OBJ_U6_BOOK || obj->obj_n == OBJ_U6_SCROLL) && (too_far || blocked))) {
			scroll->display_string("\n");
			return true; // display prompt
		}
		// read
		if (items.actor_ref == player->get_actor()) {
			scroll->display_string(":\n\n");
			uint8 book_num = obj->quality - 1;
			if (obj->quality == 0)
				book_num = 126;
			if ((data = book->get_book_data(book_num))) {
				/*
				             // FIX Any alternate-font text is in < >, Runic is capitalized,
				             // Gargish is lower-case. Translations follow untranslated text and
				             // are wrapped in & &.
				             if(data[0] == '<' && data[strlen(data)-1] == '>') //Britannian text is wrapped in '<' '>' chars
				                {
				                 scroll->display_string(&data[1],strlen(data)-2, 1); // 1 for britannian font.
				                 scroll->display_string("\n",1);
				                }
				             else
				                {
				*/
				bool using_gump = game->is_using_text_gumps();
				if (using_gump) {
					switch (obj->obj_n) {
					case OBJ_U6_BOOK:
					case OBJ_U6_PICTURE:
					case OBJ_U6_SCROLL:
					case OBJ_U6_GRAVE:
					case OBJ_U6_CROSS: // wooden cross used as grave marker (text like grave)
					case OBJ_U6_BALLOON_PLANS:
					case OBJ_U6_BOOK_OF_CIRCLES:
					case OBJ_U6_CODEX:
						game->get_view_manager()->open_scroll_gump(data, strlen(data));
						break;
					case OBJ_U6_SIGN:
						if (strlen(data) > 20) { // FIXME sign text needs to fit on multiple lines
							using_gump = false;
							break;
						}
						game->get_view_manager()->open_sign_gump(data, strlen(data));
						break;
					case OBJ_U6_SIGN_ARROW:

					default:
						using_gump = false;
					}
				}
				if (!using_gump) {
					scroll->set_autobreak(true);
					scroll->display_string(data, strlen(data)); //normal font
					scroll->display_string("\n\t"); // '\t' = auto break off.
				}
				//scroll->set_autobreak(false);
//                }
				free(data);
			}

		}
		return true;
	}
	return false;
}


/* LOOK: Display the current time. Disallow search.
 */
bool U6UseCode::look_clock(Obj *obj, UseCodeEvent ev) {
	GameClock *clock = game->get_clock();
	if (obj->obj_n == OBJ_U6_SUNDIAL
	        && (clock->get_hour() < 5 || clock->get_hour() > 19))
		return true; // don't get time from sundial at night
	if (ev == USE_EVENT_LOOK && items.actor_ref == player->get_actor()) {
		scroll->display_string("\nThe time is ");
		scroll->display_string(clock->get_time_string());
		scroll->display_string("\n");
	}
	return true;
}


/* test (need to determine use of true/false return)
 */
bool U6UseCode::look_mirror(Obj *obj, UseCodeEvent ev) {
//    ViewManager *view_manager = game->get_view_manager();
	if (ev == USE_EVENT_LOOK && items.actor_ref == player->get_actor()) {
		uint16 x, y;
		uint8 z;
		items.actor_ref->get_location(&x, &y, &z);
		if (x == obj->x && y > obj->y && y <= (obj->y + 2)) {
			scroll->display_string("\nYou can see yourself!");
			game->get_event()->display_portrait(items.actor_ref);
		}
		scroll->display_string("\n");
		return true;
	}
	return false;
}


/* PASS: if not in party mode, say that you cannot enter and do normal move
 * else walk all party members to cave, give dungeon name, and move to dungeon
 */
bool U6UseCode::enter_dungeon(Obj *obj, UseCodeEvent ev) {
	if (!party->contains_actor(items.actor_ref))
		return false;

	const char *prefix = "", *dungeon_name = "";
	uint16 x = obj->x, y = obj->y;
	uint8 z = obj->z;

	if (party->is_in_vehicle()) //don't enter if in a balloon.
		return true;

	if (!player->in_party_mode()) {
		scroll->display_string("\n\nNot in solo mode.\n");
		return true;
	}

	if (ev == USE_EVENT_USE && UseCode::out_of_use_range(obj, true))
		return true;

	if (obj->quality < 21)
		dungeon_name = u6_dungeons[obj->quality];
	if (obj->quality >= 1 && obj->quality <= 7)
		prefix = "dungeon ";
	else if (obj->quality >= 9 && obj->quality <= 11)
		prefix = "shrine of ";
	else
		prefix = "";

	party->dismount_from_horses();

	// don't activate if autowalking from linking exit
	if ((ev == USE_EVENT_PASS || ev == USE_EVENT_USE) && items.actor_ref == player->get_actor() && !party->get_autowalk()) {
		ActorManager *actorMan = Game::get_game()->get_actor_manager();
		if (obj->quality != 0 && party->contains_actor(3) && actorMan->get_actor(3)->is_alive()) {
			// scroll->printf("%s says, \"This is the %s%s.\"\n\n",blah->name, prefix, dungeon_name);
			scroll->display_string("Shamino says, \"This is the ");
			scroll->display_string(prefix);
			scroll->display_string(dungeon_name);
			scroll->display_string(".\"\n\n");
			scroll->display_prompt();
		}
		MapCoord entrance(x, y, z);
		// going down
		if (z == 0) { // from surface, do superchunk translation
			x = (x & 0x07) | (x >> 2 & 0xF8);
			y = (y & 0x07) | (y >> 2 & 0xF8);
		}
		if (z < 5)
			z += 1;
		else
			z -= 1;

		MapCoord exitPos(x, y, z);
//        if(obj->obj_n == OBJ_U6_HOLE) // fall down hole faster
//            party->walk(&entrance, &exitPos, 100);
//        else
//            party->walk(&entrance, &exitPos);
		party->walk(&entrance, &exitPos, 100);
		game->get_weather()->set_wind_dir(NUVIE_DIR_NONE);
		return true;
	} else if ((ev == USE_EVENT_PASS || ev == USE_EVENT_USE) && party->get_autowalk()) // party can use now
		return true;
	return false;
}

bool U6UseCode::enter_moongate(Obj *obj, UseCodeEvent ev) {
	/* shared between blue and red gates */
	/* PASS: if not in party mode, say that you cannot enter and do normal move
	 * else walk all party members to moongate and teleport.
	 */
	uint16 x = obj->x, y = obj->y;
	uint8 z = obj->z;
	MapCoord exitPos(0, 0, 0);

	if (party->is_in_vehicle())
		return true;
	if (items.mapcoord_ref->x != x)
		return true; // don't step onto the left tile of a moongate

	if (!player->in_party_mode()) {
		scroll->display_string("\nYou must be in party mode to enter.\n\n");
		scroll->display_prompt();
		return true;
	}

	// don't activate if autowalking from linking exitPos
	if (ev == USE_EVENT_PASS && items.actor_ref == player->get_actor() && !party->get_autowalk()) {
		if (obj->obj_n == OBJ_U6_RED_GATE) {
			if (obj->quality > 25) {
				DEBUG(0, LEVEL_ERROR, "invalid moongate destination %d\n", obj->quality);
				return false;
			}
			if (!party->has_obj(87, 0, false)) { // make sure orb of moons is in party inventory
				scroll->display_string("\nYou forgot the Orb of the Moons!\n");
				return true;
			}
			if ((obj->quality > 0 && obj->quality < 12) ||
			        (obj->quality > 14 && obj->quality < 26)) { //positions 0, 12, 13 and 14 go nowhere.
				x = red_moongate_tbl[obj->quality].x; // set our moongate destination from the lookup table.
				y = red_moongate_tbl[obj->quality].y;
				z = red_moongate_tbl[obj->quality].z;
			}
			exitPos = MapCoord(x, y, z);
		} else if (obj->obj_n == OBJ_U6_MOONGATE) {
			// FIXME: Duplication from PartyView, this ought to be separated
			/* we don't care if the moons are in the sky,
			 * (to make permanent moongates work)
			 * if the moongate is there, it goes somewhere
			 */
			Weather *weather = game->get_weather();
			GameClock *clock = Game::get_game()->get_clock();
			uint8 day = clock->get_day();
			uint8 hour = clock->get_hour();
			uint8 phaseTrammel = uint8(nearbyint((day - 1) / TRAMMEL_PHASE)) % 8;
			sint8 phaseb = (day - 1) % uint8(nearbyint(FELUCCA_PHASE * 8)) - 1;
			uint8 phaseFelucca = (phaseb >= 0) ? phaseb : 0;
			uint8 posTrammel = ((hour + 1) + 3 * phaseTrammel) % 24;
			uint8 posFelucca = ((hour - 1) + 3 * phaseFelucca) % 24;
			uint8 absTrammel = abs(posTrammel - 12);
			uint8 absFelucca = abs(posFelucca - 12);
			if (absTrammel < absFelucca) {
				// Trammel wins.
				exitPos = weather->get_moonstone(8 - phaseTrammel);
			} else {
				// Feluccality!
				exitPos = weather->get_moonstone(8 - phaseFelucca);
			}
			if (exitPos.x == 0 && exitPos.y == 0 && exitPos.z == 0) {
				exitPos = MapCoord(x, y, z); // stay put.
			}
		}
		party->walk(obj, &exitPos);
		return true;
	} else if (ev == USE_EVENT_PASS && party->get_autowalk()) // party can use now
		if (party->contains_actor(items.actor_ref))
			return true;
	return true;
}


/* USE: Light powder keg if unlit
 * MESSAGE: Timed: Explode; Effect complete: delete powder keg
 */
bool U6UseCode::use_powder_keg(Obj *obj, UseCodeEvent ev) {
	if (ev == USE_EVENT_USE) {
		game->get_script()->call_use_keg(obj);
	}
	return true;
}


/* Use: Fire! (block input, start cannonball anim, release input on hit)
 * Message: Effect complete. Return to prompt.
 * Move: Change direction if necessary
 */
bool U6UseCode::use_cannon(Obj *obj, UseCodeEvent ev) {
	MapCoord *mapcoord_ref = items.mapcoord_ref;

	if (ev == USE_EVENT_USE) {
		scroll->display_string("\nFire!\n");
// FIXME: new UseCodeEffect(obj, cannonballtile, dir) // sets WAIT mode
		new CannonballEffect(obj); // sets WAIT mode
		// Note: waits for effect to complete and sends MESG_EFFECT_COMPLETE
		return false;
	} else if (ev == USE_EVENT_MESSAGE) {
		if (*items.msg_ref == MESG_EFFECT_COMPLETE) {
			scroll->display_string("\n");
			scroll->display_prompt();
		}
		return true;
	} else if (ev == USE_EVENT_MOVE) {
		// allow normal move
		if ((obj->frame_n == 0 && mapcoord_ref->sy < 0)
		        || (obj->frame_n == 1 && mapcoord_ref->sx > 0)
		        || (obj->frame_n == 2 && mapcoord_ref->sy > 0)
		        || (obj->frame_n == 3 && mapcoord_ref->sx < 0))
			return true;
		else { // aim cannon in new direction
			if (mapcoord_ref->sy < 0)
				obj->frame_n = 0;
			else if (mapcoord_ref->sy > 0)
				obj->frame_n = 2;
			else if (mapcoord_ref->sx < 0)
				obj->frame_n = 3;
			else if (mapcoord_ref->sx > 0)
				obj->frame_n = 1;
			return false;
		}
	}
	return false;
}


/* USE: Hatch egg.
 */
bool U6UseCode::use_egg(Obj *obj, UseCodeEvent ev) {
	EggManager *egg_manager = obj_manager->get_egg_manager();
	bool success = egg_manager->spawn_egg(obj, NUVIE_RAND() % 100);
	if (items.actor_ref)
		scroll->display_string(success ? "\nSpawned!\n" : "\nNo effect.\n");
	return true;
}

/* USE: Open spellbook for casting, if equipped.
 * LOOK: Open for spell inspection.
 */
bool U6UseCode::use_spellbook(Obj *obj, UseCodeEvent ev) {
	if (ev == USE_EVENT_USE) {
		Game::get_game()->get_event()->endAction(); // FIXME: this should call Magic directly
		Game::get_game()->get_event()->newAction(CAST_MODE);
		if (obj->is_readied()) {
			/* TODO open spellbook for casting */

		}
	} else if (ev == USE_EVENT_LOOK) {
		scroll->display_string("\n");
		/* TODO open spellbook for reading */

	}
	return true;
}

/* Use: Light torch if readied or on the ground.
 * Ready: Get a torch from a stack and equip it.
 * Unready: Unlight torch.
 * Get: Equip torch if lit
 * Drop: Unlight torch
 */
bool U6UseCode::torch(Obj *obj, UseCodeEvent ev) {
	if (ev == USE_EVENT_USE) {
		if (obj->frame_n == 1) {
			extinguish_torch(obj);
			return true;
		}
		// light
		if (obj->is_on_map()) {
			Obj *torch = obj_manager->get_obj_from_stack(obj, 1);
			if (torch != obj)
				obj_manager->add_obj(torch, true); // keep new one on map
			light_torch(torch);
			return true;
		} else { // so is readied or in inventory
			Obj *torch = obj;
			Actor *actor;
			if (obj->is_in_inventory() == false) // container on map
				actor = actor_manager->get_player();
			else
				actor = actor_manager->get_actor_holding_obj(obj);
			bool can_light_it = true; // only set FALSE on some error
			bool in_container = obj->is_in_container();

			if (!obj->is_readied()) {
				torch = obj_manager->get_obj_from_stack(obj, 1);
				if (torch != obj) // keep new one in inventory
					actor->inventory_add_object_nostack(torch);

				// ready it
//                actor = actor_manager->get_actor_holding_obj(torch);
				can_light_it = actor->add_readied_object(torch);
			}

			if (can_light_it) { // assume torch is readied
				assert(torch->is_readied());
				light_torch(torch);
			} else {
				assert(torch->qty == 1);
				if (in_container) // need old location
					obj_manager->moveto_container(torch, obj->get_container_obj());
				else if (torch->is_in_inventory()) { //  assume it's not stacked
					actor->inventory_remove_obj(torch);
					actor->inventory_add_object(torch); // restack here
				}
				scroll->display_string("\nNo free hand to hold the torch.\n");
			}
		}
	} else if (ev == USE_EVENT_READY) {
		if (obj->is_readied()) { // remove
			if (obj->frame_n == 1) {
				extinguish_torch(obj);
				return false; // destroyed
			}
		} else { // equip (get one from the stack)
			if (obj->qty > 1 && obj->frame_n == 0) { // don't change the quantity of lit torches
				Obj *torch = obj_manager->get_obj_from_stack(obj, obj->qty - 1);
				assert(torch != obj); // got a new object from the obj stack
				if (obj->is_in_container())
					obj_manager->moveto_container(torch, obj->get_container_obj(), false);
				else if (obj->is_in_inventory()) { // keep extras in inventory
					actor_manager->get_actor_holding_obj(torch)->inventory_add_object_nostack(torch);
				}
			}
		}
		return true; // equip or remove to inventory
	} else if (ev == USE_EVENT_GET) {
		if (obj->frame_n == 0) // unlit: may get normally
			return true;
		toggle_frame(obj); // unlight
		obj->qty = 1;
		obj_manager->remove_obj_from_map(obj); // add to inventory and USE
		items.actor_ref->inventory_add_object(obj); // will unstack in USE
		scroll->display_string("\n");
		torch(obj, USE_EVENT_USE);
		return false; // ready or not, handled by usecode
	} else if (ev == USE_EVENT_DROP) {
		if (obj->frame_n == 0) // unlit: normal drop
			return true;
		extinguish_torch(obj);
		return false; // destroyed
	}

	return true;
}

/* Torches disappear when extinguished. */
void U6UseCode::extinguish_torch(Obj *obj) {
	assert(obj->frame_n == 1);

//  handled by Actor::inventory_remove_obj()
//    if(obj->is_in_inventory_old())
//        actor_manager->get_actor_holding_obj(obj)->subtract_light(TORCH_LIGHT_LEVEL);
	if (obj->is_readied()) {
		Actor *owner = actor_manager->get_actor_holding_obj(obj);
		if ((owner->is_in_party() || owner == player->get_actor()) && owner->is_alive()) {
			if (owner->get_hp() == 0) { // Avatar during Kal Lor item removal
				owner->remove_readied_object(obj, false);
				party->subtract_light_source();
				game->get_map_window()->updateBlacking();
				return;
			}
		} else { // don't extinguish on death or leaving the party
			game->get_map_window()->updateBlacking(); // might need this on death
			return;
		}
	}

	scroll->display_string("\nA torch burned out.\n");
	destroy_obj(obj, 0, false);
	game->get_map_window()->updateBlacking();
}

void U6UseCode::light_torch(Obj *obj) {
	assert(obj->qty == 1);
	assert(obj->frame_n == 0);
	assert(obj->is_readied() || obj->is_on_map());
	toggle_frame(obj); // light
	obj->status |= OBJ_STATUS_LIT;
	Actor *owner = nullptr;
	if (obj->is_readied()) {
		owner = actor_manager->get_actor_holding_obj(obj);
		owner->add_light(TORCH_LIGHT_LEVEL);
	}

	obj->qty = 0xc8; //torch duration. updated in lua advance_time()
	if (!owner || owner->is_in_party() || owner == player->get_actor())
		scroll->display_string("\nTorch is lit.\n");
	game->get_map_window()->updateBlacking();
}

bool U6UseCode::process_effects(Obj *container_obj, Actor *actor) {
	Obj *temp_obj;
	U6Link *obj_link, *temp_link;

	/* Test whether this object has items inside it. */
	if (container_obj->container != nullptr) {
		for (obj_link = container_obj->container->end(); obj_link != nullptr;) {
			temp_obj = (Obj *)obj_link->data;

			if (temp_obj->obj_n == OBJ_U6_EFFECT) {
				temp_link = obj_link->prev;
				game->get_script()->call_actor_use_effect(temp_obj, actor); //Note this call unlinks the effect object.
				obj_link = temp_link;
			} else
				obj_link = obj_link->prev;
		}
	}

	return true;
}

/* Use: Display Peer effect, showing a map of the area around the player.
   Message: Delete 1 gem. */
bool U6UseCode::use_peer_gem(Obj *obj, UseCodeEvent ev) {
	if (ev == USE_EVENT_MESSAGE && *items.msg_ref == MESG_EFFECT_COMPLETE) {
		destroy_obj(obj, 1);
		scroll->display_string("\n");
		scroll->display_prompt();
		return true;
	}

	if (ev != USE_EVENT_USE)
		return true;

	uint16 x, y;
	uint8 z;
	player->get_location(&x, &y, &z);
	game->get_event()->close_gumps();
	new PeerEffect(x - (x % 8) - 18, y - (y % 8) - 18, z, obj); // wrap to chunk boundary,
	// and center in 11x11 MapWindow
	return false; // no prompt
}

/* Ready: Apply ring's status effect to actor.
   Unready: Cancel status effect. */
bool U6UseCode::magic_ring(Obj *obj, UseCodeEvent ev) {
	Actor *actor = obj->get_actor_holding_obj();
	if (!actor)
		actor = player->get_actor();
	if (actor->inventory_get_readied_object(ACTOR_HAND) != nullptr
	        && actor->inventory_get_readied_object(ACTOR_HAND) != obj
	        && actor->inventory_get_readied_object(ACTOR_HAND_2) != nullptr
	        && actor->inventory_get_readied_object(ACTOR_HAND_2) != obj)
		return true;
	uint8 num_readied = actor->count_readied_objects(obj->obj_n, 0);
//    if(obj->obj_n == OBJ_U6_REGENERATION_RING)
//        actor_manager->get_actor_holding_obj(obj)->??? no visual effect
//    if(obj->obj_n == OBJ_U6_PROTECTION_RING)
//        actor_manager->get_actor_holding_obj(obj)->??? no visual effect
	if (obj->obj_n == OBJ_U6_INVISIBILITY_RING)
		actor->set_invisible((obj->is_readied() && num_readied == 1) ? false : true);
	return true; // do normal ready/unready
}

bool U6UseCode::storm_cloak(Obj *obj, UseCodeEvent ev) {
	Actor *actor = obj->get_actor_holding_obj();
	if (!actor)
		actor = player->get_actor();
	if (actor->inventory_get_readied_object(ACTOR_BODY) != nullptr
	        && actor->inventory_get_readied_object(ACTOR_BODY) != obj)
		return true;

	AsyncEffect *e = new AsyncEffect(new TileBlackFadeEffect(actor, 9, 20)); //FIXME hardcoded values.
	e->run();

	if (obj->is_readied() == false) {
		Game::get_game()->get_clock()->set_timer(GAMECLOCK_TIMER_U6_STORM, 0x14);
	} else {
		Game::get_game()->get_clock()->set_timer(GAMECLOCK_TIMER_U6_STORM, 0);
	}

	return true;
}

/* Unready/Drop/Move: Don't allow removal. */
bool U6UseCode::amulet_of_submission(Obj *obj, UseCodeEvent ev) {
	if (obj->is_readied()) {
		scroll->display_string("\nMagical energy prevents you from removing the amulet.\n");
		return false;
	}
	return true;
}

/* Use: Learn Gargish! */
bool U6UseCode::gargish_vocabulary(Obj *obj, UseCodeEvent ev) {
	if (ev == USE_EVENT_USE) {
		scroll->display_string("\n");
		scroll->display_string("You study the scroll!\n");
		player->set_gargish_flag(true);
	}
	return true;
}

/* LOOK: Print the name of a holy brazier, and not the normal description. */
bool U6UseCode::holy_flame(Obj *obj, UseCodeEvent ev) {
	if (obj->quality == 0 || obj->quality > 3)
		return true; // use normal description
	scroll->display_string("\nThe flame of ");
	if (obj->quality == 1)
		scroll->display_string("truth");
	if (obj->quality == 2)
		scroll->display_string("love");
	if (obj->quality == 3)
		scroll->display_string("courage");
	scroll->display_string(".\n");
	return false;
}

bool U6UseCode::cannot_unready(const Obj *obj) const {
	if (!obj->is_readied())
		return false;
	if (obj->obj_n == OBJ_U6_AMULET_OF_SUBMISSION
	        || (obj->obj_n == OBJ_U6_TORCH && obj->frame_n == 1))
		return true;

	return false;
}
bool U6UseCode::use_harpsichord(Obj *obj, UseCodeEvent ev) {
	if (ev == USE_EVENT_SEARCH) {
		return search_container(obj);
	}
	return play_instrument(obj, ev);
}

} // End of namespace Nuvie
} // End of namespace Ultima
