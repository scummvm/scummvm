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

#ifndef NUVIE_USECODE_U6_USECODE_H
#define NUVIE_USECODE_U6_USECODE_H

#include "ultima/nuvie/core/obj_manager.h"
#include "ultima/nuvie/core/timed_event.h"
#include "ultima/nuvie/usecode/usecode.h"
//object numbers
#include "ultima/nuvie/core/u6_objects.h"

namespace Ultima {
namespace Nuvie {

#define U6_LORD_BRITISH_ACTOR_NUM 5 /*change to U6_NPC_LORD_BRITISH?*/
#define U6_SHERRY_ACTOR_NUM       9
#define U6_BEHLEM_ACTOR_NUM       164

#define U6_LORD_BRITISH_ORB_CHECK_FLAG 0x20 // this is set if the player has asked LB about the orb of moons

class U6UseCode;
class CallBack;
class Configuration;
class MsgScroll;

// U6ObjectType properties
#define OBJTYPE_NONE      0x0000
#define OBJTYPE_FOOD      0x0001 /* food or drink */
#define OBJTYPE_CONTAINER 0x0002 /* must be set on empty bag,crate,backpack */
#define OBJTYPE_BOOK      0x0004 /* has book data (signs, paintings, books) */

#define TORCH_LIGHT_LEVEL    3

// Explicit packing used below to fix MSVC warning
#include "common/pack-start.h"	// START STRUCT PACKING

struct U6ObjectType { // object properties & usecode
	uint16 obj_n; // type
	uint8 frame_n; // 0xFF matches any frame
	uint8 dist; // distance to trigger (depends on event, usually 0)
	UseCodeEvent trigger; // accepted event(s)
	bool (U6UseCode::*usefunc)(Obj *, UseCodeEvent); // usecode function
	uint16 flags; // properties (OBJTYPE)
} PACKED_STRUCT;

#include "common/pack-end.h"	// END STRUCT PACKING

typedef enum {
	LAT,
	LON
} U6UseCodeLatLonEnum;

class U6UseCode: public UseCode, public CallBack {
public:

	U6UseCode(Game *g, Configuration *cfg);
	~U6UseCode() override;

	bool use_obj(Obj *obj, Actor *actor) override;
	bool look_obj(Obj *obj, Actor *actor) override;
	bool pass_obj(Obj *obj, Actor *actor, uint16 x, uint16 y) override;
	bool search_obj(Obj *obj, Actor *actor) override;
	bool move_obj(Obj *obj, sint16 rel_x, sint16 rel_y) override;
	bool load_obj(Obj *obj) override;
	bool message_obj(Obj *obj, CallbackMessage msg, void *msg_data) override;
	bool ready_obj(Obj *obj, Actor *actor) override;
	bool get_obj(Obj *obj, Actor *actor) override;
	bool drop_obj(Obj *obj, Actor *actor, uint16 x, uint16 y, uint16 qty = 0) override;

	bool has_usecode(Obj *obj, UseCodeEvent ev = USE_EVENT_USE) override;
	bool has_usecode(Actor *actor, UseCodeEvent ev = USE_EVENT_USE) override;
	bool cannot_unready(Obj *obj) override;

	bool is_door(Obj *obj) {
		return (obj->obj_n >= 297 && obj->obj_n <= 300);
	}
	bool is_unlocked_door(Obj *obj) override {
		return (is_door(obj) && obj->frame_n != 9 && obj->frame_n != 11);
	}
	bool is_locked_door(Obj *obj) override {
		return (is_door(obj) && (obj->frame_n == 9 || obj->frame_n == 11));
	}
	bool is_magically_locked_door(Obj *obj) {
		return (is_door(obj) && (obj->frame_n == 13 || obj->frame_n == 15));
	}
	bool is_closed_door(Obj *obj) override {
		return (is_door(obj) && obj->frame_n > 3);
	}

	bool is_chest(Obj *obj) override {
		return (obj->obj_n == OBJ_U6_CHEST);
	}
	bool is_closed_chest(Obj *obj) {
		return (is_chest(obj) && obj->frame_n > 0);
	}
	bool is_locked_chest(Obj *obj) {
		return (is_chest(obj) && obj->frame_n == 2);
	}
	bool is_magically_locked_chest(Obj *obj) {
		return (is_chest(obj) && obj->frame_n == 3);
	}
	void unlock_chest(Obj *obj) {
		if (is_locked_chest(obj)) obj->frame_n = 1;
		return;
	}
	void lock_chest(Obj *obj) {
		if (is_chest(obj) && obj->frame_n == 1) obj->frame_n = 2;
		return;
	}

	bool is_locked(Obj *obj) {
		return (is_locked_door(obj) || is_locked_chest(obj));
	}
	bool is_magically_locked(Obj *obj) {
		return (is_magically_locked_door(obj) || is_magically_locked_chest(obj));
	}
	void unlock(Obj *obj);
	void lock(Obj *obj);

	bool is_food(Obj *obj) override;
	bool is_container(Obj *obj) override;
	bool is_container(uint16 obj_n, uint8 frame_n) override;
	bool is_readable(Obj *obj) override;

	uint16 callback(uint16 msg, CallBack *caller, void *data = NULL) override;

protected:
	bool uc_event(const U6ObjectType *type, UseCodeEvent ev, Obj *obj);
	inline const U6ObjectType *get_object_type(uint16 n, uint8 f, UseCodeEvent ev = 0);

public:
// usecode
	bool use_orb(Obj *obj, UseCodeEvent ev);
	bool use_moonstone(Obj *obj, UseCodeEvent ev);
	bool use_door(Obj *obj, UseCodeEvent ev);
	bool use_secret_door(Obj *obj, UseCodeEvent ev);
	bool use_ladder(Obj *obj, UseCodeEvent ev);
	bool use_passthrough(Obj *obj, UseCodeEvent ev);
	bool use_switch(Obj *obj, UseCodeEvent ev);
	bool use_crank(Obj *obj, UseCodeEvent ev);
	bool use_food(Obj *obj, UseCodeEvent ev);
	bool use_firedevice(Obj *obj, UseCodeEvent ev);
	bool use_fan(Obj *obj, UseCodeEvent ev);
	bool use_sextant(Obj *obj, UseCodeEvent ev);
	bool use_staff(Obj *obj, UseCodeEvent ev);
	bool use_container(Obj *obj, UseCodeEvent ev);
	bool use_rune(Obj *obj, UseCodeEvent ev);
	bool use_vortex_cube(Obj *obj, UseCodeEvent ev);
	bool use_key(Obj *obj, UseCodeEvent ev);
	bool use_boat(Obj *obj, UseCodeEvent ev);
	inline Obj *use_boat_find_center(Obj *obj);
	inline bool use_boat_find_land(uint16 *x, uint16 *y, uint8 *z);
	bool use_balloon_plans(Obj *obj, UseCodeEvent ev);
	bool use_balloon(Obj *obj, UseCodeEvent ev);
	bool use_cow(Obj *obj, UseCodeEvent ev);
	bool use_well(Obj *obj, UseCodeEvent ev);
	bool fill_bucket(uint16 filled_bucket_obj_n);
	bool use_churn(Obj *obj, UseCodeEvent ev);
	bool use_beehive(Obj *obj, UseCodeEvent ev);
	bool use_horse(Obj *obj, UseCodeEvent ev);
	bool use_potion(Obj *obj, UseCodeEvent ev);
	bool use_bell(Obj *obj, UseCodeEvent ev);
	bool use_fishing_pole(Obj *obj, UseCodeEvent ev);
	bool use_shovel(Obj *obj, UseCodeEvent ev);
	bool use_fountain(Obj *obj, UseCodeEvent ev);
	bool use_rubber_ducky(Obj *obj, UseCodeEvent ev);
	bool use_crystal_ball(Obj *obj, UseCodeEvent ev);
	bool use_harpsichord(Obj *obj, UseCodeEvent ev);
	bool play_instrument(Obj *obj, UseCodeEvent ev);
	bool look_mirror(Obj *obj, UseCodeEvent ev);
	bool look_sign(Obj *obj, UseCodeEvent ev);
	bool look_clock(Obj *obj, UseCodeEvent ev);
	bool pass_quest_barrier(Obj *obj, UseCodeEvent ev);
	bool enter_dungeon(Obj *obj, UseCodeEvent ev);
	bool enter_moongate(Obj *obj, UseCodeEvent ev);
// bool search_container(Obj *obj, UseCodeEvent ev);
	bool use_powder_keg(Obj *obj, UseCodeEvent ev);
	bool use_cannon(Obj *obj, UseCodeEvent ev);
	bool use_egg(Obj *obj, UseCodeEvent ev);
	bool sundial(Obj *obj, UseCodeEvent ev);
	bool torch(Obj *obj, UseCodeEvent ev);
	bool use_spellbook(Obj *obj, UseCodeEvent ev);
	bool use_peer_gem(Obj *obj, UseCodeEvent ev);
	bool magic_ring(Obj *obj, UseCodeEvent ev);
	bool storm_cloak(Obj *obj, UseCodeEvent ev);
	bool amulet_of_submission(Obj *obj, UseCodeEvent ev);
	bool gargish_vocabulary(Obj *obj, UseCodeEvent ev);
	bool holy_flame(Obj *obj, UseCodeEvent ev);

protected:
// supplementary
	void remove_gargoyle_egg(uint16 x, uint16 y, uint8 z);
	Obj *drawbridge_find(Obj *crank_obj);
	void drawbridge_open(uint16 x, uint16 y, uint8 level, uint16 b_width);
	void drawbridge_close(uint16 x, uint16 y, uint8 level, uint16 b_width);
	void drawbridge_remove(uint16 x, uint16 y, uint8 level, uint16 *bridge_width);
	bool use_firedevice_message(Obj *obj, bool lit);
	void lock_door(Obj *obj);
	void unlock_door(Obj *obj);
	Obj *bell_find(Obj *chain_obj);
	void sundial_set_shadow(Obj *sundial, uint8 hour);
	void extinguish_torch(Obj *obj);
	void light_torch(Obj *obj);
	bool process_effects(Obj *container_obj, Actor *actor) override;
	sint16 parseLatLongString(U6UseCodeLatLonEnum mode, Std::string *input);

	inline bool use_find_water(uint16 *x, uint16 *y, uint8 *z);
	inline bool lock_pick_dex_check();
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
