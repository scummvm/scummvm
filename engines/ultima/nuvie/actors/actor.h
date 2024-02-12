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

#ifndef NUVIE_ACTORS_ACTOR_H
#define NUVIE_ACTORS_ACTOR_H

#include "ultima/shared/std/containers.h"
#include "ultima/shared/std/string.h"
#include "ultima/nuvie/misc/actor_list.h"
#include "ultima/nuvie/core/map.h"
#include "ultima/nuvie/core/obj_manager.h"

namespace Ultima {
namespace Nuvie {

using Std::list;
using Std::string;
using Std::vector;

#define ACTOR_NO_READIABLE_LOCATION -1
#define ACTOR_HEAD   0
#define ACTOR_NECK   1
#define ACTOR_BODY   2
#define ACTOR_ARM    3
#define ACTOR_ARM_2  4
#define ACTOR_HAND   5
#define ACTOR_HAND_2 6
#define ACTOR_FOOT   7
#define ACTOR_NOT_READIABLE 8

// actor alignment
enum ActorAlignment {
	ACTOR_ALIGNMENT_DEFAULT = 0,
	ACTOR_ALIGNMENT_NEUTRAL = 1,
	ACTOR_ALIGNMENT_EVIL    = 2,
	ACTOR_ALIGNMENT_GOOD    = 3,
	ACTOR_ALIGNMENT_CHAOTIC = 4,
};

// move-flags
#define ACTOR_FORCE_MOVE    1
#define ACTOR_IGNORE_OTHERS 2
#define ACTOR_OPEN_DOORS    4
#define ACTOR_IGNORE_DANGER 8
#define ACTOR_IGNORE_MOVES 0x10
#define ACTOR_IGNORE_PARTY_MEMBERS 0x20 // only used in Actor::check_move. In U6, player diagonal movement
// between two blocked tiles isn't allowed (non-party actors block)

// push-flags (exclusive)
#define ACTOR_PUSH_ANYWHERE 0
#define ACTOR_PUSH_HERE     1
#define ACTOR_PUSH_FORWARD  2

#define ACTOR_SHOW_BLOOD true
#define ACTOR_FORCE_HIT true

#define ACTOR_STATUS_PROTECTED 0x1
#define ACTOR_STATUS_PARALYZED 0x2
#define ACTOR_STATUS_ASLEEP    0x4
#define ACTOR_STATUS_POISONED  0x8
#define ACTOR_STATUS_DEAD      0x10
#define ACTOR_STATUS_ATTACK_EVIL 0x20
#define ACTOR_STATUS_ATTACK_GOOD 0x40
#define ACTOR_STATUS_IN_PARTY  0x80
#define ACTOR_STATUS_ALIGNMENT_MASK 0x60

#define ACTOR_MOVEMENT_HIT_FLAG 0x8
#define ACTOR_MOVEMENT_FLAGS_OLD_ALIGNMENT_MASK 0x60

#define ACTOR_OBJ_FLAG_
#define ACTOR_NO_ERROR 0
#define ACTOR_OUT_OF_MOVES 1
#define ACTOR_BLOCKED 2
#define ACTOR_BLOCKED_BY_OBJECT 3
#define ACTOR_BLOCKED_BY_ACTOR 4

#define ACTOR_MAX_READIED_OBJECTS 8

#define ACTOR_WT_FOLLOW 1
#define ACTOR_WT_PLAYER 2
#define ACTOR_WT_RANGED 4
#define ACTOR_WT_RETREAT 7
#define ACTOR_WT_ASSAULT 8

#define ACTOR_CHANGE_BASE_OBJ_N true

#define ACTOR_VEHICLE_ID_N 0
#define ACTOR_AVATAR_ID_N  1

#define INV_EXCLUDE_READIED_OBJECTS false
#define INV_INCLUDE_READIED_OBJECTS true

#define ACTOR_MD_OBJ_FLAG_HYPOXIA 6
#define ACTOR_MD_OBJ_FLAG_FRENZY  7

#define ACTOR_MD_STATUS_FLAG_COLD 0

class Map;
class MapCoord;
class UseCode;
class ActorPathFinder;
class U6LList;
class GameClock;
class Path;

typedef struct {
	uint16 x;
	uint16 y;
	uint8 z;
	uint8 hour;
	uint8 day_of_week; // 0 = any day 1..7
	uint8 worktype;
} Schedule;

typedef enum { ATTACK_TYPE_NONE, ATTACK_TYPE_HAND, ATTACK_TYPE_THROWN, ATTACK_TYPE_MISSLE } AttackType;

typedef struct {
	uint16 obj_n;

	union {
		uint8 defence;
		uint8 defense;
	};
	union {
		uint8 attack;
		uint8 damage;
	};

	uint16 hit_range;
	AttackType attack_type;

	uint16 missle_tile_num;
	uint16 thrown_obj_n;

	bool breaks_on_contact;
} CombatType;

typedef struct {
	Obj *obj;
	const CombatType *combat_type;
	bool double_handed;
} ReadiedObj;

typedef uint8 ActorErrorCode;
typedef struct {
	ActorErrorCode err;
	Obj *blocking_obj;
	Actor *blocking_actor;
} ActorError;

typedef uint8 ActorMoveFlags;

typedef enum {
	ACTOR_ST, // single tile
	ACTOR_DT, // double tile
	ACTOR_QT, // quad tile
	ACTOR_MT  // multi tile
} ActorTileType;

class Actor {
	friend class ActorManager;
	friend class MapWindow;
	friend class Party;
	friend class Player;
	friend class U6UseCode;

public:
	struct cmp_level {
		bool operator()(Actor *a1, Actor *a2) const {
			return (a1->level > a2->level);
		}
	};

	struct cmp_dex {
		bool operator()(Actor *a1, Actor *a2) const {
			return (a1->dex > a2->dex);
		}
	};

	struct cmp_moves {
		bool operator()(Actor *a1, Actor *a2) const {
			return (a1->moves > a2->moves);
		}
	};

	struct cmp_move_fraction {
		bool operator()(Actor *a1, Actor *a2) const {
			if (a1->dex == 0) {
				a1->dex = 1;
				DEBUG(0, LEVEL_WARNING, "%s (%d) has 0 dex!\n", a1->get_name(), a1->id_n);
			}
			if (a2->dex == 0) {
				a2->dex = 1;
				DEBUG(0, LEVEL_WARNING, "%s (%d) has 0 dex!\n", a2->get_name(), a2->id_n);
			}
			return (a1->moves / a1->dex > a2->moves / a2->dex);
		}
	};

	struct cmp_distance_to_loc {
		MapCoord cmp_loc;
		void operator()(const MapCoord &cmp_loc2) {
			cmp_loc = cmp_loc2;
		}
		bool operator()(Actor *a1, Actor *a2) {
			MapCoord loc1(a1->x, a1->y, a1->z);
			MapCoord loc2(a2->x, a2->y, a2->z);
			return (loc1.distance(cmp_loc) < loc2.distance(cmp_loc));
		}
	};

protected:

	uint8 id_n;

	Map *map;
	ObjManager *obj_manager;
	GameClock *_clock;
	UseCode *usecode;
	ActorPathFinder *pathfinder;

	uint16 x;
	uint16 y;
	uint16 z;

	uint8 worktype;
	MapCoord work_location;
	uint32 move_time; // time (in clock ticks) of last update() (for display)
	// FIXME: replace with animation
	uint16 obj_n;
	uint16 frame_n;
	uint16 base_obj_n;
	uint16 old_frame_n;

	NuvieDir direction;
	uint8 walk_frame;

	uint8 obj_flags;
	uint8 status_flags;
	uint8 talk_flags;
	uint8 movement_flags; //0x19f1

	bool ethereal;
	bool can_move;
	bool temp_actor;
	bool met_player;

	bool visible_flag;
// bool active; // "cached in"

	sint8 moves; // number of moves actor has this turn
	uint8 light; // level of light around actor (normally 0)
	vector<uint8> light_source;

	ActorError error_struct; // error/status; result of previous action

	uint8 strength;
	uint8 dex;
	uint8 intelligence;
	uint8 hp;
	uint8 level;
	uint16 exp;
	uint8 magic;
	uint8 combat_mode;
	ActorAlignment alignment;

	uint8 body_armor_class;
	uint8 readied_armor_class;

	string name;

	ReadiedObj *readied_objects[ACTOR_MAX_READIED_OBJECTS];

	Schedule **sched;
	int num_schedules;

	U6LList *obj_inventory;

//current schedule pos;
	uint16 sched_pos;

	list<Obj *> surrounding_objects; //used for multi-tile actors.
	Common::HashMap<uint16, uint16> *custom_tile_tbl;

public:

	Actor(Map *m, ObjManager *om, GameClock *c);
	virtual ~Actor();

	virtual bool init(uint8 obj_status = NO_OBJ_STATUS);
	void init_from_obj(Obj *obj, bool change_base_obj = false);

	bool is_avatar() const {
		return id_n == ACTOR_AVATAR_ID_N;
	}
	bool is_onscreen() const {
		return MapCoord(x, y, z).is_visible();
	}
	bool is_in_party() const {
		return ((status_flags & ACTOR_STATUS_IN_PARTY) == ACTOR_STATUS_IN_PARTY);
	}
	bool is_in_vehicle() const;
	bool is_visible() const {
		return visible_flag;
	}
	bool is_alive() const {
		return (status_flags & ACTOR_STATUS_DEAD) ? false : true;
	}
	bool is_nearby(const Actor *other) const;
	bool is_nearby(uint8 actor_num) const;
	bool is_nearby(const MapCoord &where, uint8 thresh = 5) const;
	bool is_at_position(const Obj *obj) const;
	virtual bool is_passable() const;
	bool is_temp() const {
		return temp_actor;
	}
	virtual bool isFlying() const {
		return false;
	}
	virtual bool isNonBlocking() const {
		return false;
	}
	/**
	 * @brief Does any tile of this actor occupy the given world location?
	 * @param lx world coordinate
	 * @param ly world coordinate
	 * @param lz level
	 * @param incDoubleTile include all tiles of double width/height actors
	 * @param incSurroundingObjs include surrounding actor objects
	 * @return true if actor occupies location, false otherwise
	 */
	virtual bool doesOccupyLocation(uint16 lx, uint16 ly, uint8 lz, bool incDoubleTile = true, bool incSurroundingObjs = true) const;

//for lack of a better name:
	bool is_met() const {
		return talk_flags & 0x01;
	}
	bool is_poisoned() const {
		return status_flags & ACTOR_STATUS_POISONED;
	}
	bool is_invisible() const {
		return obj_flags & OBJ_STATUS_INVISIBLE;
	}
	virtual bool is_immobile() const; // frozen by worktype or status
	virtual bool is_sleeping() const {
		return status_flags & ACTOR_STATUS_ASLEEP;
	}
	virtual bool is_paralyzed() const {
		return status_flags & ACTOR_STATUS_PARALYZED;
	}
	virtual bool is_protected() const {
		return status_flags & ACTOR_STATUS_PROTECTED;
	}
	virtual bool is_charmed() const {
		return obj_flags & OBJ_STATUS_CHARMED;
	}
	virtual bool is_cursed() const {
		return obj_flags & OBJ_STATUS_CURSED;
	}
	virtual bool get_corpser_flag() const {
		return false;
	}
	bool is_hit() const {
		return movement_flags & ACTOR_MOVEMENT_HIT_FLAG;
	}

	void set_name(const char *actor_name) {
		name = actor_name;
	}
	const char *get_name(bool force_real_name = false);

	void get_location(uint16 *ret_x, uint16 *ret_y, uint8 *ret_level) const;
	MapCoord get_location() const;

	uint16 get_tile_num() const;
	Tile *get_tile() const;
	virtual uint16 get_downward_facing_tile_num() const;
	uint8 get_actor_num() const {
		return id_n;
	}
	uint8 get_talk_flags() const {
		return talk_flags;
	}
	virtual ActorTileType get_tile_type() const {
		return ACTOR_ST;
	}

	uint16 get_frame_n() const {
		return frame_n;
	}
	uint16 get_old_frame_n() const {
		return old_frame_n;
	}
	uint16 get_x() const {
		return x;
	}
	uint16 get_y() const {
		return y;
	}
	uint8  get_z() const {
		return z;
	}

	uint8 get_strength() const {
		return strength;
	}
	uint8 get_dexterity() const {
		return dex;
	}
	uint8 get_intelligence() const {
		return intelligence;
	}
	uint8 get_hp() const {
		return hp;
	}
	virtual uint8 get_hp_text_color() const {
		return 0;
	}
	virtual uint8 get_str_text_color() const {
		return 0;
	}
	virtual uint8 get_dex_text_color() const {
		return 0;
	}

	uint8 get_level() const {
		return level;
	}
	uint16 get_exp() const {
		return exp;
	}
	uint8 get_magic() const {
		return magic;
	}
	ActorAlignment get_alignment() const {
		return alignment;
	}
	uint8 get_old_alignment() const {
		return ((movement_flags & ACTOR_MOVEMENT_FLAGS_OLD_ALIGNMENT_MASK) >> 5) + 1;
	}
	sint8 get_moves_left() const {
		return moves;
	}
	virtual uint8 get_maxhp() const {
		return 0;
	}
	virtual uint8 get_maxmagic() const {
		return 0;
	}
	bool get_obj_flag(uint8 bitFlag) const {
		return bitFlag < 8 ? (obj_flags & (1 << bitFlag)) : false;
	}
	bool get_status_flag(uint8 bitFlag) const {
		return bitFlag < 8 ? (status_flags & (1 << bitFlag)) : false;
	}

	uint16 get_base_obj_n() const {
		return base_obj_n;
	}
	virtual void change_base_obj_n(uint16 val) {
		base_obj_n = obj_n = val;
		frame_n = 0;
	}
	void set_obj_n(uint16 val) {
		obj_n = val;
	}
	void set_frame_n(uint16 val) {
		frame_n = val;
	}
	void set_strength(uint8 val) {
		strength = val;
	}
	void set_dexterity(uint8 val) {
		dex = val;
	}
	void set_intelligence(uint8 val) {
		intelligence = val;
	}
	void set_hp(uint8 val);
	void set_level(uint8 val) {
		level = val;
	}
	void set_exp(uint16 val) {
		exp = clamp_max(val, 9999);
	}
	void set_magic(uint8 val) {
		magic = val;
	}
	void set_alignment(ActorAlignment a) {
		alignment = a;
	}
	void set_old_alignment(ActorAlignment a) {
		if (a > 0 && a < 5) {
			movement_flags |= (a - 1) << 5;
		}
	}
	uint8 get_light_level() const;
	void add_light(uint8 val);
	void subtract_light(uint8 val);
	void heal() {
		set_hp(get_maxhp());
	}
	void cure();
	void set_moves_left(sint8 val);
	void set_dead_flag(bool value);
	virtual void update_time() {
		set_moves_left(get_moves_left() + get_dexterity());
	}
	void set_poisoned(bool poisoned);
	virtual void set_paralyzed(bool paralyzed) {
		return;
	}
	virtual void set_protected(bool val) {
		return;
	}
	virtual void set_charmed(bool val) {
		return;
	}
	virtual void set_corpser_flag(bool val) {
		return;
	}
	virtual void set_cursed(bool val) {
		return;
	}
	virtual void set_asleep(bool val) {
		return;
	}
	void set_obj_flag(uint8 bitFlag, bool value);
	void set_status_flag(uint8 bitFlag, bool value);
	void set_hit_flag(bool val);

	void set_invisible(bool invisible);
	void set_custom_tile_num(uint16 obj_num, uint16 tile_num);

	uint8 get_worktype();
	uint8 get_sched_worktype();
	virtual void set_worktype(uint8 new_worktype, bool init = false);
	uint8 get_combat_mode() const {
		return combat_mode;
	}
	void set_combat_mode(uint8 new_mode);
	virtual void revert_worktype() { }

	NuvieDir get_direction() const {
		return direction;
	}
	void set_direction(sint16 rel_x, sint16 rel_y);
	virtual void set_direction(NuvieDir d);
	void face_location(const MapCoord &loc);
	virtual void face_location(uint16 lx, uint16 ly);
	void face_actor(Actor *a);

	void set_talk_flags(uint8 newflags) {
		talk_flags = newflags;
	}
	uint8 get_flag(uint8 bitflag);
	void set_flag(uint8 bitflag);
	void clear_flag(uint8 bitflag);
	void show();
	void hide();
	void set_error(ActorErrorCode err);
	void clear_error();
	ActorError *get_error();

	const list<Obj *> &get_surrounding_obj_list() const {
		return surrounding_objects;
	}
	void add_surrounding_obj(Obj *obj);
	void unlink_surrounding_objects(bool make_objects_temporary = false);

	bool moveRelative(sint16 rel_x, sint16 rel_y, ActorMoveFlags flags = 0);
	virtual bool move(uint16 new_x, uint16 new_y, uint8 new_z, ActorMoveFlags flags = 0);
	virtual bool check_move(uint16 new_x, uint16 new_y, uint8 new_z, ActorMoveFlags flags = 0);
	bool check_moveRelative(sint16 rel_x, sint16 rel_y, ActorMoveFlags flags = 0);

	virtual bool can_be_moved();
	virtual bool can_be_passed(const Actor *other, bool ignoreParty = false) const;
	virtual void update();
	void set_in_party(bool state);
	void set_pathfinder(ActorPathFinder *new_pf, Path *path_type = 0);
	ActorPathFinder *get_pathfinder() {
		return pathfinder;
	}
	void delete_pathfinder();
	virtual void pathfind_to(const MapCoord &d);
	void pathfind_to(uint16 gx, uint16 gy, uint8 gz = 255);
	bool walk_path();
	virtual void preform_worktype() {
		return;
	}

// combat methods
	//void attack(const MapCoord &pos); // attack at a given map location
	Obj *get_weapon_obj(sint8 readied_obj_location);
	void attack(sint8 readied_obj_location, MapCoord target, Actor *foe = nullptr);
	const CombatType *get_weapon(sint8 readied_obj_location);
	void attract_to(Actor *target);
	void repel_from(Actor *target);

	void hit(uint8 dmg, bool force_hit = false);
	void reduce_hp(uint8 amount);
	virtual void die(bool create_body = true);
	void resurrect(const MapCoord &new_position, Obj *body_obj = nullptr);
	uint8 get_range(uint16 target_x, uint16 target_y);
	bool weapon_can_hit(const CombatType *weapon, uint16 target_x, uint16 target_y);
	virtual bool weapon_can_hit(const CombatType *weapon, Actor *target, uint16 *hit_x, uint16 *hit_y) {
		*hit_x = target->get_x();
		*hit_y = target->get_y();
		return true;
	}
	void display_condition();
	ActorList *find_enemies(); // returns list or 0 if no enemies nearby

	U6LList *get_inventory_list();
	const U6LList *get_inventory_list() const;
	bool inventory_has_object(uint16 obj_n, uint8 qual = 0, bool match_quality = OBJ_MATCH_QUALITY, uint8 frame_n = 0, bool match_frame_n = OBJ_NOMATCH_FRAME_N);
	uint32 inventory_count_objects(bool inc_readied_objects) const;
	uint32 inventory_count_object(uint16 obj_n);
	Obj *inventory_get_object(uint16 obj_n, uint8 qual = 0, bool match_quality = OBJ_MATCH_QUALITY, uint8 frame_n = 0, bool match_frame_n = OBJ_NOMATCH_FRAME_N);
	bool is_double_handed_obj_readied();
	Obj *inventory_get_readied_object(uint8 location);
	sint16 inventory_get_readied_obj_n(uint8 location) {
		return (inventory_get_readied_object(location) == nullptr ? -1 : inventory_get_readied_object(location)->obj_n);
	}
	virtual Obj *inventory_get_food(Obj *container = 0) {
		return 0;
	}
	const CombatType *inventory_get_readied_object_combat_type(uint8 location);
	bool inventory_add_object(Obj *obj, Obj *container = 0, bool stack = true);
	bool inventory_add_object_nostack(Obj *obj, Obj *container = 0) {
		return inventory_add_object(obj, container, false);
	}
	void inventory_del_all_objs();
	bool inventory_remove_obj(Obj *obj, bool run_usecode = true);
	Obj *inventory_new_object(uint16 obj_n, uint32 qty, uint8 quality = 0);
	uint32 inventory_del_object(uint16 obj_n, uint32 qty, uint8 quality);
	float inventory_get_max_weight() const {
		return strength * 2;
	}
	float get_inventory_weight() const;
	float get_inventory_equip_weight();
	void inventory_drop_all();
	void all_items_to_container(Obj *container_obj, bool stack);
	bool can_carry_weight(Obj *obj) const;
	bool can_carry_weight(float obj_weight) const; // return from get_obj_weight()
	virtual bool can_carry_object(uint16 obj_n, uint32 qty = 0) const;
	virtual bool can_carry_object(Obj *obj) const;

	virtual uint8 get_object_readiable_location(Obj *obj);
	virtual const CombatType *get_object_combat_type(uint16 objN) {
		return nullptr;
	}

	bool can_ready_obj(Obj *obj);
	bool add_readied_object(Obj *obj);
	void remove_readied_object(Obj *obj, bool run_usecode = true); // run_usecode to stop from running usecode twice or an infinite loop
	void remove_readied_object(uint8 location, bool run_usecode = true);

	void remove_all_readied_objects();
	bool has_readied_objects();
	sint8 count_readied_objects(sint32 obj_n = -1, sint16 frame_n = -1, sint16 quality = -1);

	virtual void twitch() {
		return;
	}
	bool push(Actor *pusher, uint8 where = ACTOR_PUSH_ANYWHERE);

	Obj *make_obj();
	uint16 get_obj_n() const {
		return obj_n;
	}
	virtual void clear();
	virtual bool morph(uint16 obj_n); // change actor type

	bool get_schedule_location(MapCoord *loc) const;
	bool is_at_scheduled_location() const;
	int get_number_of_schedules() const {
		return num_schedules;
	}
	Schedule *get_schedule(uint8 index);
	virtual bool will_not_talk() const {
		return false;
	}
	uint16 get_custom_tile_num(uint16 obj_num) const;
protected:

	void loadSchedule(const unsigned char *schedule_data, uint16 num);
	virtual bool updateSchedule(uint8 hour, bool teleport = false);
	uint16 getSchedulePos(uint8 hour);
// uint16 getSchedulePos(uint8 hour, uint8 day_of_week);
// inline uint16 Actor::getSchedulePos(uint8 hour);

	void inventory_parse_readied_objects(); //this is used to initialise the readied_objects array on load.

	virtual const CombatType *get_hand_combat_type() const {
		return nullptr;
	}

	virtual void set_ethereal(bool val) {
		ethereal = val;
	}
	virtual void print();
	virtual void handle_lightsource(uint8 hour) {
		return;
	}
	virtual const char *get_worktype_string(uint32 wt) const {
		return nullptr;
	}

	Obj *find_body();
	uint16 get_tile_num(uint16 obj_num) const;
	uint8 get_num_light_sources() const {
		return light_source.size();
	}

private:

};

const char *get_actor_alignment_str(ActorAlignment alignment);

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
