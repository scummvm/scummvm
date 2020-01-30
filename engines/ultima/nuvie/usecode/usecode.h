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

#ifndef NUVIE_USECODE_USECODE_H
#define NUVIE_USECODE_USECODE_H

#include "ultima/shared/std/string.h"
#include "ultima/nuvie/misc/call_back.h"
#include "ultima/nuvie/misc/map_entity.h"
#include "ultima/nuvie/core/obj_manager.h"
#include "ultima/nuvie/core/player.h"

namespace Ultima {
namespace Nuvie {

// The game triggers one of these events on an object to activate its UseCode
// function(s). The return value meaning is different for each event.
#define USE_EVENT_USE     0x01 /* return value undefined */
#define USE_EVENT_LOOK    0x02 /* true: allow search, false: disallow search */
#define USE_EVENT_PASS    0x04 /* true: do normal move, false: object blocks */
#define USE_EVENT_MESSAGE 0x08 /* internal message or data return */
#define USE_EVENT_SEARCH  0x10 /*undefined (true = had objects?); might remove*/
//#define USE_EVENT_ON      0x20 /* post-move/idle */
#define USE_EVENT_MOVE    0x40 /* true: move object, false: don't move object */
#define USE_EVENT_LOAD    0x80 /* return value undefined */
#define USE_EVENT_READY   0x0100 /* true: object may be equipped */
#define USE_EVENT_GET     0x0200 /* true: do normal get */
#define USE_EVENT_DROP    0x0400 /* true: do normal drop */
#define USE_EVENT_INPUT_CANCEL 0x501 /* note this shares a bit with USE_EVENT_USE so it can pass through uc_event(). return undefined */
//#define USE_EVENT_NEAR    0x00 /* mirrors; might use ON with distance val */
//#define USE_EVENT_ATTACK  0x00 /* doors, chests, mirrors */
//#define USE_EVENT_ENTER   0x00 /* object enters view (clocks) */
//#define USE_EVENT_LEAVE   0x00 /* object leaves view */

typedef uint16 UseCodeEvent;

/* Events:
 * USE
 * Returns: undefined
 * Use the object.
 * actor_ref - the actor using it (the player actor typically)
 *
 * PASS (Quest Barrier)
 * Returns: True if actor may move, False if object blocks
 * Called when an actor attempts to step onto an object.
 * actor_ref - actor trying to pass
 * mapcoord_ref - location the actor is trying to pass (for multi-tile objects)
 *
 * LOOK (signs)
 * Returns: True if an object can be searched
 * Called when someone looks at the object. Some objects aren't searched (books
 * for example) and should return false.
 *
 * MESSAGE (fumaroles, earthquakes?, powder kegs, clocks)
 * An internal event from the engine. It must have been previously requested.
 * Includes TIMED events, and DATA returns. Be careful that the object still
 * exists.
 *
 * MOVE (cannons)
 * Returns: True to push object to the new position
 * Use this to perform special move functions for some objects. A cannon can be
 * aimed with MOVE.
 * mapcoord_ref - target location
 *
 * (UN)LOAD unimplemented (fumaroles)
 * Returns: undefined
 * Called when the object is cached in or out (or when it would have been in the
 * original game... about 16 to 32 spaces away), and when new objects are created.
 * It can be used to start timers, or to hatch eggs.
 *
 * Actor NEAR unimplemented (mirrors)
 * Set something to happen when an actor moves close to or away from object.
 * Distance will be used.
 *
 * Actor ON (chairs, traps) unimplemented
 * Returns: undefined
 * Called each turn for any objects in the view-area with actors standing on
 * them.
 *
 * ENTER view-area unimplemented
 *
 * LEAVE view-area unimplemented
 * Enter and leave will be used to start and stop sound effects. Distance will
 * be used.
 *
 * (UN)READY (Amulet of Submission)
 * Returns: True if the object may be worn, or removed
 * This is called before the object is is equipped or removed. Check the
 * object's flags to determine if it is being equipped or not. (if its readied
 * flag is set it is being removed). Special un/ready functions can be created
 * with this.
 * actor_ref - the actor un/readying it
 *
 * ATTACK (doors, chests)
 *
 * DROP (breakables, torches)
 * Returns: True to allow normal drop at the target.
 * Special drop functions can be created with this.
 * actor_ref - the actor dropping it
 * mapcoord_ref - the desired drop target
 *
 * GET (torches, runes?)
 * Returns: True if the actor can get the object.
 * Special get functions can be created with this.
 * actor_ref - the actor getting it
 *
 * SEARCH (graves, secret doors)
 * Returns: True if the object contained other objects.
 * FIXME: might remove this and add as a player action
 *
 */

typedef enum {
	USE,
	GET,
	MOVE
} UseCodeType;

const char *useCodeTypeToString(UseCodeType type);

class ActorManager;
class Configuration;
class Events;
class Game;
class Map;
class MsgScroll;
class MapCoord;
class Party;
class Player;
class Script;
class ScriptThread;

class UseCode {
private:
	ScriptThread *script_thread;

protected:
	Game *game;
	Configuration *config;
	ObjManager *obj_manager;
	Map *map;
	Player *player;
	MsgScroll *scroll;
	ActorManager *actor_manager;
	Party *party;
	Script *script;

// pass parameters to usecode functions via items (NULL itemref is unset)
	struct {
		uint32 *uint_ref;
		sint32 *sint_ref;
		Obj *obj_ref;
		Actor *actor_ref, *actor2_ref;
		MapCoord *mapcoord_ref;
		CallbackMessage *msg_ref;
		Std::string *string_ref;
		MapEntity *ent_ref;
		char *data_ref;
	} items;
	void clear_items();

public:

	UseCode(Game *g, Configuration *cfg);
	virtual ~UseCode();

	virtual bool init(ObjManager *om, Map *m, Player *p, MsgScroll *ms);

	bool use_obj(uint16 x, uint16 y, uint8 z, Obj *src_obj = NULL);
	bool use_obj(Obj *obj, Obj *src_obj = NULL) {
		return (use_obj(obj, player->get_actor()));    // ??
	}

	virtual bool use_obj(Obj *obj, Actor *actor);
	virtual bool look_obj(Obj *obj, Actor *actor)   {
		return (false);
	}
	virtual bool pass_obj(Obj *obj, Actor *actor, uint16 x, uint16 y) {
		return (false);
	}
	virtual bool search_obj(Obj *obj, Actor *actor) {
		return (false);
	}
	virtual bool move_obj(Obj *obj, sint16 rel_x, sint16 rel_y);
	virtual bool load_obj(Obj *obj)                 {
		return (false);
	}
	virtual bool message_obj(Obj *obj, CallbackMessage msg, void *msg_data) {
		return (false);
	}
	virtual bool ready_obj(Obj *obj, Actor *actor);
	virtual bool get_obj(Obj *obj, Actor *actor)    {
		return (false);
	}
	virtual bool drop_obj(Obj *obj, Actor *actor, uint16 x, uint16 y, uint16 qty = 0) {
		return (false);
	}

	virtual bool has_usecode(Obj *obj, UseCodeEvent ev = USE_EVENT_USE);
	virtual bool has_usecode(Actor *actor, UseCodeEvent ev = USE_EVENT_USE)  {
		return (false);
	}
	virtual bool has_lookcode(Obj *obj) {
		return (has_usecode(obj, USE_EVENT_LOOK));
	}
	virtual bool has_passcode(Obj *obj) {
		return (has_usecode(obj, USE_EVENT_PASS));
	}
	virtual bool has_movecode(Obj *obj) {
		return (has_usecode(obj, USE_EVENT_MOVE));
	}
	virtual bool has_loadcode(Obj *obj) {
		return (has_usecode(obj, USE_EVENT_LOAD));
	}
	virtual bool has_readycode(Obj *obj) {
		return (has_usecode(obj, USE_EVENT_READY));
	}
	virtual bool cannot_unready(Obj *obj) {
		return false;
	}
	virtual bool has_getcode(Obj *obj)  {
		return (has_usecode(obj, USE_EVENT_GET));
	}
	virtual bool has_dropcode(Obj *obj) {
		return (has_usecode(obj, USE_EVENT_DROP));
	}

	bool is_door(Obj *obj) {
		return (is_locked_door(obj) || is_unlocked_door(obj));
	}
	virtual bool is_locked_door(Obj *obj)   {
		return (false);
	}
	virtual bool is_unlocked_door(Obj *obj) {
		return (false);
	}
	virtual bool is_closed_door(Obj *obj)   {
		return (false);
	}
	virtual bool process_effects(Obj *container_obj, Actor *actor) {
		return (false);
	}
	virtual bool is_food(Obj *obj)          {
		return (false);
	}
	virtual bool is_container(Obj *obj);
	virtual bool is_container(uint16 obj_n, uint8 frame_n) {
		return (false);
	}
	virtual bool is_readable(Obj *obj)      {
		return (false);
	}
	virtual bool is_chest(Obj *obj)         {
		return (false);
	}

	void set_itemref(sint32 *val) {
		items.sint_ref = val;
	}
	void set_itemref(Obj *val)   {
		items.obj_ref = val;
	}
	void set_itemref(Actor *val, Actor *val2 = NULL) {
		items.actor_ref = val;
		items.actor2_ref = val2;
	}
	void set_itemref(MapCoord *val)   {
		items.mapcoord_ref = val;
	}

	Obj *get_obj_from_container(Obj *obj);
	bool search_container(Obj *obj, bool show_string = true);
	Obj *destroy_obj(Obj *obj, uint32 count = 0, bool run_usecode = true);
	bool out_of_use_range(Obj *obj, bool check_enemies);

	ScriptThread *get_running_script();
	bool is_script_running();

protected:

	void toggle_frame(Obj *obj);
	void dbg_print_event(UseCodeEvent event, Obj *obj);

};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
