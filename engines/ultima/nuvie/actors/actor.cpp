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
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/misc/u6_llist.h"
#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/core/game_clock.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/core/obj_manager.h"
#include "ultima/nuvie/actors/actor_manager.h"
#include "ultima/nuvie/views/view_manager.h"
#include "ultima/nuvie/usecode/u6_usecode.h"
#include "ultima/nuvie/core/party.h"
#include "ultima/nuvie/pathfinder/combat_path_finder.h"
#include "ultima/nuvie/pathfinder/seek_path.h"
#include "ultima/nuvie/core/converse.h"
#include "ultima/nuvie/core/effect.h"
#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/script/script.h"
#include "ultima/nuvie/core/events.h"
#include "ultima/nuvie/actors/u6_actor.h"
#include "ultima/shared/std/containers.h"

namespace Ultima {
namespace Nuvie {

uint8 walk_frame_tbl[4] = {0, 1, 2, 1};

class ActorManager;

Actor::Actor(Map *m, ObjManager *om, GameClock *c)
	: sched(NULL), obj_inventory(NULL) {
	map = m;
	obj_manager = om;
	usecode = NULL;
	clock = c;
	pathfinder = NULL;

	direction = 0;
	walk_frame = 0;
	ethereal = false;
	can_move = true;
	temp_actor = false;
	visible_flag = true;
	met_player = false;
// active = false;

	worktype = 0;
	sched_pos = 0;
	move_time = 0;
	num_schedules = 0;

	alignment = ACTOR_ALIGNMENT_NEUTRAL;

	memset(readied_objects, 0, sizeof(readied_objects));
	moves = 0;
	light = 0;

	name = "";
	status_flags = 0;
	talk_flags = 0;
	obj_flags = 0;
	body_armor_class = 0;
	readied_armor_class = 0;

	custom_tile_tbl = NULL;

	clear_error();
}

Actor::~Actor() {
// free sched array
	if (sched != NULL) {
		Schedule **cursched = sched;
		while (*cursched != NULL)
			free(*cursched++);

		free(sched);
	}
	if (pathfinder)
		delete pathfinder;

	for (uint8 location = 0; location < ACTOR_MAX_READIED_OBJECTS; location++) {
		if (readied_objects[location] != NULL) {
			delete readied_objects[location];
		}
	}

	if (custom_tile_tbl) {
		delete custom_tile_tbl;
	}

	return;
}

bool Actor::init(uint8 obj_status) {
	if (pathfinder)
		delete_pathfinder();
	set_moves_left(dex);
	return true;
}

void Actor::init_from_obj(Obj *obj, bool change_base_obj) {
	x = obj->x;
	y = obj->y;
	z = obj->z;

	if (change_base_obj) {
		base_obj_n = obj->obj_n;
	}
	obj_n = obj->obj_n;
	frame_n = obj->frame_n;
	obj_flags = obj->status;

	set_dead_flag(false);
	init();
	show();
	return;
}

/* Returns true if another NPC `n' is in proximity to location `where'.
 */
bool Actor::is_nearby(MapCoord &where, uint8 thresh) {
	MapCoord here(x, y, z);
	if (here.xdistance(where) <= thresh && here.ydistance(where) <= thresh && z == where.z)
		return (true);
	return (false);
}


bool Actor::is_nearby(Actor *other) {
	MapCoord there(other->get_location());
	return (is_nearby(there));
}


bool Actor::is_nearby(uint8 actor_num) {
	return (is_nearby(Game::get_game()->get_actor_manager()->get_actor(actor_num)));
}

bool Actor::is_at_position(Obj *obj) {
	if (obj->x == x && obj->y == y && obj->z == z)
		return true;

	return false;
}

bool Actor::is_passable() {
	if (ethereal)
		return true;
	Tile *tile;

	tile = obj_manager->get_obj_tile(obj_n, frame_n);

	return tile->passable;
}

bool Actor::is_in_vehicle() {
	if (is_in_party() == false)
		return false;

	return Game::get_game()->get_party()->is_in_vehicle();
}

void Actor::get_location(uint16 *ret_x, uint16 *ret_y, uint8 *ret_level) {
	if (ret_x) *ret_x = x;
	if (ret_y) *ret_y = y;
	if (ret_level) *ret_level = z;
}


MapCoord Actor::get_location() {
	return (MapCoord(x, y, z));
}


uint16 Actor::get_tile_num() {
	if (custom_tile_tbl) {
		return get_custom_tile_num(obj_n);
	}

	return obj_manager->get_obj_tile_num(obj_n);
}

uint16 Actor::get_tile_num(uint16 obj_num) {
	if (custom_tile_tbl) {
		return get_custom_tile_num(obj_num);
	}

	return obj_manager->get_obj_tile_num(obj_num);
}

uint16 Actor::get_custom_tile_num(uint16 obj_num) {
	if (custom_tile_tbl) {
		Common::HashMap<uint16, uint16>::iterator it;
		it = custom_tile_tbl->find(obj_num);
		if (it != custom_tile_tbl->end()) {
			return it->_value;
		}
	}

	return obj_manager->get_obj_tile_num(obj_num);
}

Tile *Actor::get_tile() {
	return Game::get_game()->get_tile_manager()->get_tile(get_tile_num() + frame_n);
}

uint8 Actor::get_worktype() {
	return worktype;
}

uint8 Actor::get_sched_worktype() {
	if (sched[sched_pos])
		return sched[sched_pos]->worktype;

	return 0; //no worktype
}

uint16 Actor::get_downward_facing_tile_num() {
	return obj_manager->get_obj_tile_num(obj_n) + frame_n;
}

/* Set direction faced by actor and change walk frame. */
void Actor::set_direction(uint8 d) {
	if (is_alive() == false || is_immobile())
		return;

	if (d < 4)
		direction = d;

	walk_frame = (walk_frame + 1) % 4;

	frame_n = direction * 4 + walk_frame_tbl[walk_frame];

}

/* Set direction as if moving in relative direction rel_x,rel_y. */
void Actor::set_direction(sint16 rel_x, sint16 rel_y) {
	uint8 new_direction = direction;
	if (rel_x == 0 && rel_y == 0) // nowhere (just update frame)
		new_direction = direction;
	else if (rel_x == 0) // up or down
		new_direction = (rel_y < 0) ? NUVIE_DIR_N : NUVIE_DIR_S;
	else if (rel_y == 0) // left or right
		new_direction = (rel_x < 0) ? NUVIE_DIR_W : NUVIE_DIR_E;
// Add 2 to current direction if it is opposite the new direction
	else if (rel_x < 0 && rel_y < 0) { // up-left
		if (direction != NUVIE_DIR_N && direction != NUVIE_DIR_W)
			new_direction = direction + 2;
	} else if (rel_x > 0 && rel_y < 0) { // up-right
		if (direction != NUVIE_DIR_N && direction != NUVIE_DIR_E)
			new_direction = direction + 2;
	} else if (rel_x < 0 && rel_y > 0) { // down-left
		if (direction != NUVIE_DIR_S && direction != NUVIE_DIR_W)
			new_direction = direction + 2;
	} else if (rel_x > 0 && rel_y > 0) { // down-right
		if (direction != NUVIE_DIR_S && direction != NUVIE_DIR_E)
			new_direction = direction + 2;
	}
	// wrap
	if (new_direction >= 4)
		new_direction -= 4;
	set_direction(new_direction);
}

void Actor::face_location(MapCoord &loc) {
	face_location(loc.x, loc.y);
}

/* Set direction towards an x,y location on the map. */
void Actor::face_location(uint16 lx, uint16 ly) {
	set_direction(lx - x, ly - y);
}
#if 0
/* Set direction towards an x,y location on the map.
 */
void Actor::face_location(uint16 lx, uint16 ly) {
	uint16 xdiff = abs(x - lx), ydiff = abs(y - ly);
	if (ydiff) {
		if (y < ly && direction != 2)
			set_direction(2); // down
		else if (y > ly && direction != 0)
			set_direction(0); // up
	}
	if (xdiff) {
		if (x < lx && direction != 1)
			set_direction(1); // right
		else if (x > lx && direction != 3)
			set_direction(3); // left
	}
}
#endif

void Actor::face_actor(Actor *a) {
	uint16 ax, ay;
	uint8 al;

	a->get_location(&ax, &ay, &al);
	face_location(ax, ay);
}


void Actor::set_poisoned(bool poisoned) {
	if (poisoned) {
		status_flags |= ACTOR_STATUS_POISONED;
		new HitEffect(this); // no direct hp loss
	} else {
		status_flags &= (0xff ^ ACTOR_STATUS_POISONED);
	}

	if (is_in_party())
		Game::get_game()->get_view_manager()->update();
}

/* Returns the proper (NPC) name of this actor if the Player knows it, or their
 * description if the name is unknown.
 */
const char *Actor::get_name(bool force_real_name) {
	ActorManager *actor_manager = Game::get_game()->get_actor_manager();
	Converse *converse = Game::get_game()->get_converse();
	Party *party = Game::get_game()->get_party();
	//Actor *player = Game::get_game()->get_player()->get_actor();
	const char *talk_name = NULL; // name from conversation script
	bool statue = (Game::get_game()->get_game_type() == NUVIE_GAME_U6 && id_n >= 189 && id_n <= 200);

	if (is_alive() && is_in_party()) {
		sint8 party_pos = party->get_member_num(this);
		if (party_pos != -1)
			name = party->get_actor_name((uint8)party_pos);
	} else if ((is_met() || is_in_party() || force_real_name)
	           && (talk_name = converse->npc_name(id_n)) // assignment
	           && !statue)
		name = talk_name;
	else
		name = actor_manager->look_actor(this, false);
	return (name.c_str());
}

void Actor::add_surrounding_obj(Obj *obj) {
	obj->set_actor_obj(true);
	surrounding_objects.push_back(obj);
}

void Actor::unlink_surrounding_objects(bool make_objects_temporary) {
//	if(make_objects_temporary)
	{
		Std::list<Obj *>::iterator obj;

		for (obj = surrounding_objects.begin(); obj != surrounding_objects.end(); obj++) {
			if (make_objects_temporary)
				(*obj)->set_temporary();
			(*obj)->set_actor_obj(false);
		}
	}
	surrounding_objects.clear();
}

bool Actor::moveRelative(sint16 rel_x, sint16 rel_y, ActorMoveFlags flags) {
	return move(x + rel_x, y + rel_y, z, flags);
}


bool Actor::check_move(uint16 new_x, uint16 new_y, uint8 new_z, ActorMoveFlags flags) {
	Actor *a;
	bool ignore_actors = flags & ACTOR_IGNORE_OTHERS;
	bool ignore_danger = (flags & ACTOR_IGNORE_DANGER);
// bool ignore_danger = true;
	/*
	    uint16 pitch = map->get_width(new_z);
	    if(new_x < 0 || new_x >= pitch)
	        return(false);
	    if(new_y < 0 || new_y >= pitch)
	        return(false);
	*/
	if (!ignore_actors) {
		a = map->get_actor(new_x, new_y, new_z, false);
		if (a /*&& a->is_visible()*/) {
			bool ignore_party_members = (flags & ACTOR_IGNORE_PARTY_MEMBERS);
			if (ignore_party_members && a->is_in_party())
				return true;
			return a->can_be_passed(this); // we can move over or under some actors. eg mice, dragons etc.
		}
	}

//    if(map->is_passable(new_x,new_y,new_z) == false)
//        return(false);

	if (!ignore_danger)
		if (map->is_damaging(new_x, new_y, new_z))
			return false;

	return (true);
}

bool Actor::check_moveRelative(sint16 rel_x, sint16 rel_y, ActorMoveFlags flags) {
	return check_move(x + rel_x, y + rel_y, z, flags);
}


bool Actor::can_be_moved() {
	return can_move;
}

bool Actor::can_be_passed(Actor *other) {
	return (other->is_passable() || is_passable());
}

uint8 Actor::get_object_readiable_location(Obj *obj) {
	int loc = Game::get_game()->get_script()->call_obj_get_readiable_location(obj);

	if (loc >= 0)
		return (uint8)loc;

	return ACTOR_NOT_READIABLE;
}

bool Actor::move(uint16 new_x, uint16 new_y, uint8 new_z, ActorMoveFlags flags) {
// assert(new_z < 6); // shouldn't need to check anymore

//const uint8 move_cost = 5; // base cost to move
	bool force_move = (bool)(flags & ACTOR_FORCE_MOVE);
	bool open_doors = (bool)(flags & ACTOR_OPEN_DOORS);
	bool ignore_actors = (bool)(flags & ACTOR_IGNORE_OTHERS);
	bool ignore_danger = (bool)(flags & ACTOR_IGNORE_DANGER);
// bool ignore_danger = true;
	bool ignore_moves = (bool)(flags & ACTOR_IGNORE_MOVES);
	Obj *obj = NULL;
	MapCoord oldpos(x, y, z);

	clear_error();
	if (!usecode)
		usecode = obj_manager->get_usecode();
// no moves left
	if (!(force_move || ignore_moves) && moves <= 0) {
		set_error(ACTOR_OUT_OF_MOVES);
//    return false;
		DEBUG(0, LEVEL_WARNING, "actor %d is out of moves %d\n", id_n, moves);
	}

// blocking actors are checked for later
	obj = obj_manager->get_obj(new_x, new_y, new_z, OBJ_SEARCH_TOP, OBJ_INCLUDE_IGNORED); //we include ignored objects here to pick up the sacred quest blocking object.
	if (!force_move && !check_move(new_x, new_y, new_z, ACTOR_IGNORE_DANGER | ACTOR_IGNORE_OTHERS)) {
		// open door
		if (!(obj && usecode->is_unlocked_door(obj) && open_doors)
		        || (!usecode->use_obj(obj, this))) {
			set_error(ACTOR_BLOCKED_BY_OBJECT);
			error_struct.blocking_obj = obj;
			return false; // blocked by object or map tile
		}
	}
// avoid dangerous objects
	if (!ignore_danger
	        && !force_move
	        && ((is_in_party() && map->is_damaging(new_x, new_y, new_z))
	            || (obj && obj_manager->is_damaging(new_x, new_y, new_z)))) {
		set_error(ACTOR_BLOCKED_BY_OBJECT);
		error_struct.blocking_obj = obj;
		return false;
	}
// usecode must allow movement
	if (obj && usecode->has_passcode(obj)) {
		if (!usecode->pass_obj(obj, this, new_x, new_y) && !force_move) { // calling item is this actor
			set_error(ACTOR_BLOCKED_BY_OBJECT);
			error_struct.blocking_obj = obj;
			return false;
		}
	}
	Game *game = Game::get_game();
	Actor *other = map->get_actor(new_x, new_y, new_z, false);
	if (!ignore_actors && !force_move && other && !other->can_be_passed(this)
	        && (!game->get_party()->get_autowalk() || other->is_visible())) {
		set_error(ACTOR_BLOCKED_BY_ACTOR);
		error_struct.blocking_actor = other;
		return false; // blocked by actor
	}

// move
	x = WRAPPED_COORD(new_x, new_z); // FIXME: this is probably needed because PathFinder is not wrapping coords
	y = WRAPPED_COORD(new_y, new_z);
	z = new_z;

	can_move = true;
//FIXME move this into Player::moveRelative()
	/*
	if(!(force_move || ignore_moves) && (id_n == game->get_player()->get_actor()->id_n || id_n == 0 || (is_in_party() && game->get_party()->is_in_combat_mode() == false))) // subtract from moves left for party members only. Other actors have their movement points deducted in actor_update_all()
	{
	   set_moves_left(moves - (move_cost+map->get_impedance(oldpos.x, oldpos.y, oldpos.z)));
	   if(oldpos.x != x && oldpos.y != y) // diagonal move, double cost
	       set_moves_left(moves - (move_cost+map->get_impedance(oldpos.x, oldpos.y, oldpos.z)));
	}
	*/
// post-move
// close door
	if (open_doors) {
		obj = obj_manager->get_obj(oldpos.x, oldpos.y, z);
		if (obj && usecode->is_door(obj))
			usecode->use_obj(obj, this);
	}

// re-center map if actor is player character
	if (id_n == game->get_player()->get_actor()->id_n && game->get_player()->is_mapwindow_centered())
		game->get_map_window()->centerMapOnActor(this);
// allows a delay to be set on actor movement, in lieu of using animations
	move_time = clock->get_ticks();
	return true;
}


void Actor::update() {
	if (!is_alive()) //we don't need to update dead actors.
		return;

	if (pathfinder) {
		// NOTE: don't delete pathfinder right after walking, because the scheduled
		// activity still needs to be checked, and depends on pathfinder existing
		if (pathfinder->reached_goal())
			delete_pathfinder();
		else walk_path();
	}

//    update_time = clock->get_ticks(); moved to move()
}

/* Returns true if actor moved. */
bool Actor::walk_path() {
	pathfinder->update_location(); // set location from actor, if already moved

	// validate path and get move
	MapCoord next_loc, loc(x, y, z);
	if (!pathfinder->get_next_move(next_loc)) // nothing to do here
		return false;
	// FIXME: move to SchedPathFinder (or delete; worktype will handle refresh)
	if (next_loc == loc) { // ran out of steps? get a new path
		if (pathfinder->have_path())
			pathfinder->find_path();
		return false;
	}
	if (!move(next_loc.x, next_loc.y, next_loc.z, ACTOR_OPEN_DOORS))
		return false; // don't get a new path; probably just blocked by an actor
	set_direction(x - loc.x, y - loc.y);
	pathfinder->actor_moved();
	return true;
}

// gz 255 = current map plane
void Actor::pathfind_to(uint16 gx, uint16 gy, uint8 gz) {
	if (gz == 255)
		gz = z;
	MapCoord d(gx, gy, gz);
	pathfind_to(d);
}

void Actor::pathfind_to(MapCoord &d) {
	if (pathfinder) {
		pathfinder->set_actor(this);
		pathfinder->set_goal(d);
	} else
		set_pathfinder(new ActorPathFinder(this, d), new SeekPath);
	pathfinder->update_location();
}

// actor will take management of new_pf, and delete it when no longer needed
void Actor::set_pathfinder(ActorPathFinder *new_pf, Path *path_type) {
	if (pathfinder != NULL && pathfinder != new_pf)
		delete_pathfinder();
	pathfinder = new_pf;
	if (path_type != 0)
		pathfinder->set_search(path_type);
}

void Actor::delete_pathfinder() {
	delete pathfinder;
	pathfinder = NULL;
}

void Actor::set_in_party(bool state) {
	if (Game::get_game()->is_ethereal())
		set_ethereal(state);
	//in_party = state;
	delete_pathfinder();
	if (state == true) { // joined
//        obj_n = base_obj_n; U6Actor::set_worktype
		can_move = true;
		set_worktype(0x01); // U6_IN_PARTY
		status_flags |= ACTOR_STATUS_IN_PARTY;
		if (!is_charmed())
			set_alignment(ACTOR_ALIGNMENT_GOOD);
		else
			set_old_alignment(ACTOR_ALIGNMENT_GOOD);
	} else { // left
		if (is_alive() == true) {
			if (is_invisible())
				visible_flag = false;
			set_worktype(0x8f); // U6_WANDER_AROUND
			status_flags ^= ACTOR_STATUS_IN_PARTY;
			inventory_drop_all(); // needs to be after party status change
			if (is_charmed())
				set_old_alignment(ACTOR_ALIGNMENT_NEUTRAL);
			else
				set_alignment(ACTOR_ALIGNMENT_NEUTRAL);
		}
	}
}

/*void Actor::attack(MapCoord pos)
{
 return;
}*/

Obj *Actor::get_weapon_obj(sint8 readied_obj_location) {
	if (readied_obj_location != ACTOR_NO_READIABLE_LOCATION && readied_objects[readied_obj_location] && readied_objects[readied_obj_location]->obj != NULL)
		return readied_objects[readied_obj_location]->obj;
	return NULL;
}

uint8 Actor::get_range(uint16 target_x, uint16 target_y) {
	sint16 off_x, off_y;
	uint16 map_pitch = map->get_width(z);

	if (target_x <= x)
		off_x = x - target_x;
	else { //target_x > x
		if (target_x - x < 8) //small positive offset
			off_x = target_x - x;
		else { // target wrapped around the map.
			if (map_pitch - target_x + x < 11)
				off_x = target_x - map_pitch - x; //negative offset
			else
				off_x = 9; // x out of range
		}
	}

	if (target_y <= y)
		off_y = y - target_y;
	else { //target_y > y
		if (target_y - y < 8) //small positive offset
			off_y = target_y - y;
		else { // target wrapped around the map.
			if (map_pitch - target_y + y < 11)
				off_y = target_y - map_pitch - y; //negative offset
			else
				off_y = 9; // y out of range
		}
	}

	return Game::get_game()->get_script()->call_get_combat_range(abs(off_x), abs(off_y));
}

bool Actor::weapon_can_hit(const CombatType *weapon, uint16 target_x, uint16 target_y) {
	if (!weapon)
		return false;

	Script *script = Game::get_game()->get_script();
	if (get_range(target_x, target_y) > script->call_get_weapon_range(weapon->obj_n))
		return false;
	return true;
}

// attack another actor with melee attack or a weapon (short or long range)
void Actor::attack(sint8 readied_obj_location, MapCoord target, Actor *foe) {
	const uint8 attack_cost = 10; // base cost to attack

	Game::get_game()->get_script()->call_actor_attack(this, target, get_weapon_obj(readied_obj_location), foe);

	set_moves_left(moves - attack_cost);
}
const CombatType *Actor::get_weapon(sint8 readied_obj_location) {
	if (readied_obj_location == ACTOR_NO_READIABLE_LOCATION)
		return get_hand_combat_type();

	if (readied_objects[readied_obj_location])
		return readied_objects[readied_obj_location]->combat_type;

	return NULL;
}

U6LList *Actor::get_inventory_list() {
	return obj_manager->get_actor_inventory(id_n);
}


bool Actor::inventory_has_object(uint16 objN, uint8 qual, bool match_quality, uint8 frameN, bool match_frame_n) {
	if (inventory_get_object(objN, qual, match_quality, frameN, match_frame_n))
		return (true);
	return (false);
}

uint32 Actor::inventory_count_objects(bool inc_readied_objects) {
	Obj *obj;
	uint32 count = 0;
	U6Link *link;
	U6LList *inventory = get_inventory_list();

	if (inc_readied_objects) {
		return inventory->count();
	} else {
		for (link = inventory->start(); link != NULL; link = link->next) {
			obj = (Obj *)link->data;
			if (!obj->is_readied())
				count++;
		}
	}

	return count;
}


/* Returns the number of objects in the actor's inventory with matching object
 * number and quality.
 */
uint32 Actor::inventory_count_object(uint16 objN) {
	uint32 qty = 0;
	U6Link *link = 0;
	Obj *obj = 0;
	U6LList *inv = get_inventory_list();

	for (link = inv->start(); link != NULL; link = link->next) {
		obj = (Obj *)link->data;
		if (obj)
			qty += obj->get_total_qty(objN);
	}

	return (qty);
}


/* Returns object descriptor of object in the actor's inventory, or NULL if no
 * matching object is found. */
Obj *Actor::inventory_get_object(uint16 objN, uint8 qual, bool match_quality, uint8 frameN, bool match_frame_n) {
	U6LList *inventory;
	U6Link *link;
	Obj *obj;

	inventory = get_inventory_list();
	for (link = inventory->start(); link != NULL; link = link->next) {
		obj = (Obj *)link->data;
		if (obj->obj_n == objN && (match_quality == false || obj->quality == qual)
				&& (match_frame_n == false || obj->frame_n == frameN)) //FIXME should qual = 0 be an all quality search!?
			return (obj);
		else if (obj->has_container()) {
			if ((obj = obj->find_in_container(objN, qual, match_quality)))
				return (obj);
		}
	}

	return NULL;
}

bool Actor::is_double_handed_obj_readied() {
	if (readied_objects[ACTOR_ARM] != NULL && readied_objects[ACTOR_ARM]->double_handed == true)
		return true;

	return false;
}

Obj *Actor::inventory_get_readied_object(uint8 location) {
	if (readied_objects[location] != NULL)
		return readied_objects[location]->obj;

	return NULL;
}

const CombatType *Actor::inventory_get_readied_object_combat_type(uint8 location) {
	if (readied_objects[location] != NULL)
		return readied_objects[location]->combat_type;

	return NULL;
}


bool Actor::inventory_add_object(Obj *obj, Obj *container, bool stack) {
	obj_manager->unlink_from_engine(obj);
	U6LList *inventory = get_inventory_list(), *add_to = inventory;

// we have the item now so we don't consider it stealing if we get it at any time in the future.
	obj->set_ok_to_take(true);

//remove temp flag on inventory items.
	obj->set_temporary(false);

	if (container) { // assumes actor is holding the container
		container->add(obj, stack);
	} else {
		// only objects outside containers are marked in_inventory
		/* obj->status |= OBJ_STATUS_IN_INVENTORY; */ // luteijn: don't manipulate this directly!
		obj->set_in_inventory();
		obj->x = id_n;
		obj->parent = (void *)this;

		if (obj->is_lit()) // light up actor
			add_light(TORCH_LIGHT_LEVEL);

		obj_manager->list_add_obj(add_to, obj, stack);
	}

	return true;
}

/* Stacks the new object with existing objects if possible.
   Returns a pointer to the new object in inventory. */
Obj *Actor::inventory_new_object(uint16 objN, uint32 qty, uint8 quality) {
	Obj *obj = 0;
	uint8 frameN = 0;

	if (objN >= 1024) {
		frameN = (uint8)floorf(objN / 1024);
		objN -= frameN * 1024;
	}

	obj = new Obj;
	obj->obj_n = objN;
	obj->frame_n = frameN;
	obj->quality = quality;
	obj->qty = obj_manager->is_stackable(obj) ? 1 : 0; // stackable objects must have a quantity
	if (qty > 1) // this will combine with others, only if object is stackable
		for (uint32 q = 1; q < qty; q++) {
			inventory_add_object(obj_manager->copy_obj(obj), NULL);
		}
	inventory_add_object(obj, NULL);
	return inventory_get_object(objN, quality);
}

/* Delete `qty' objects of type from inventory (or from a container).
 * Returns the number removed (may be less than requested). */
uint32 Actor::inventory_del_object(uint16 objN, uint32 qty, uint8 quality) {
	Obj *obj;
	uint16 oqty = 0;
	uint32 deleted = 0;

	while ((obj = inventory_get_object(objN, quality, false))
	        && (deleted < qty)) {
		oqty = obj->qty == 0 ? 1 : obj->qty;
		if (oqty <= (qty - deleted)) {
			inventory_remove_obj(obj);
			delete_obj(obj);
			deleted += oqty;
		} else {
			obj->qty = oqty - (qty - deleted);
			deleted += (qty - deleted);
		}
	}
	return (deleted);
}

void Actor::inventory_del_all_objs() {
	U6LList *inventory = get_inventory_list();
	if (!inventory)
		return;

	U6Link *link = inventory->start();
	for (; link != NULL;) {
		Obj *obj = (Obj *)link->data;
		link = link->next;
		inventory_remove_obj(obj);
		delete_obj(obj);
	}

}

bool Actor::inventory_remove_obj(Obj *obj, bool run_usecode) {
	U6LList *inventory;
	Obj *container = NULL;

	inventory = get_inventory_list();
	if (obj->is_readied())
		remove_readied_object(obj, run_usecode);
	if (obj->is_in_container())
		container = obj->get_container_obj();

	obj->set_noloc(); //remove engine location

	if (container) {
		return container->remove(obj);
	}

	if (obj->status & OBJ_STATUS_LIT) // remove light from actor
		subtract_light(TORCH_LIGHT_LEVEL);

	return inventory->remove(obj);
}

float Actor::get_inventory_weight() {
	U6LList *inventory;
	U6Link *link;
	Obj *obj;
	float weight = 0;

	if (obj_manager->actor_has_inventory(id_n) == false)
		return 0;

	inventory = obj_manager->get_actor_inventory(id_n);

	for (link = inventory->start(); link != NULL; link = link->next) {
		obj = (Obj *)link->data;
		weight += obj_manager->get_obj_weight(obj);
	}

	return (weight);
}

float Actor::get_inventory_equip_weight() {
	U6LList *inventory;
	U6Link *link;
	Obj *obj;
	float weight = 0;

	if (obj_manager->actor_has_inventory(id_n) == false)
		return 0;

	inventory = obj_manager->get_actor_inventory(id_n);

	for (link = inventory->start(); link != NULL; link = link->next) {
		obj = (Obj *)link->data;
		if (obj->is_readied()) //object readied
			weight += obj_manager->get_obj_weight(obj);
	}

	return (weight);
}


/* Can the actor carry a new object of this type?
 */
bool Actor::can_carry_object(uint16 objN, uint32 qty) {
	if (Game::get_game()->using_hackmove())
		return true;
	float obj_weight = obj_manager->get_obj_weight(objN);
	if (qty) obj_weight *= qty;
	return (can_carry_weight(obj_weight));
}

bool Actor::can_carry_object(Obj *obj) {
	if (Game::get_game()->using_hackmove())
		return true;
	if (obj_manager->can_get_obj(obj) == false)
		return false;

	return can_carry_weight(obj);
}

bool Actor::can_carry_weight(Obj *obj) {
	return (can_carry_weight(obj_manager->get_obj_weight(obj, OBJ_WEIGHT_INCLUDE_CONTAINER_ITEMS, OBJ_WEIGHT_DO_SCALE)));
}

/* Can the actor carry new object(s) of this weight?
 * (return from get_obj_weight())
 */
bool Actor::can_carry_weight(float obj_weight) {
	if (Game::get_game()->using_hackmove())
		return true;
	// obj_weight /= 10;
	float inv_weight = get_inventory_weight() + obj_weight;
	float max_weight = inventory_get_max_weight();
	return (inv_weight <= max_weight);
}


void Actor::inventory_parse_readied_objects() {
	U6LList *inventory;
	U6Link *link;
	Obj *obj;

	if (obj_manager->actor_has_inventory(id_n) == false)
		return;

	inventory = obj_manager->get_actor_inventory(id_n);

	for (link = inventory->start(); link != NULL;) {
		obj = (Obj *)link->data;
		link = link->next;
		obj->parent = (void *)this;
		if (obj->is_readied()) { //object readied
			add_readied_object(obj);
		}
		if (obj->status & OBJ_STATUS_LIT) { // torch
			add_light(TORCH_LIGHT_LEVEL); // light up actor
		}
	}

	return;
}

bool Actor::can_ready_obj(Obj *obj) {
	uint8 location =  get_object_readiable_location(obj);

	switch (location) {
	case ACTOR_NOT_READIABLE :
		return false;

	case ACTOR_ARM :
		if (readied_objects[ACTOR_ARM] != NULL) { //if full try other arm
			if (readied_objects[ACTOR_ARM]->double_handed)
				return false;

			location = ACTOR_ARM_2;
		}
		break;

	case ACTOR_ARM_2 :
		if (readied_objects[ACTOR_ARM] != NULL || readied_objects[ACTOR_ARM_2] != NULL)
			return false;
		location = ACTOR_ARM;
		break;

	case ACTOR_HAND :
		if (readied_objects[ACTOR_HAND] != NULL) // if full try other hand
			location = ACTOR_HAND_2;
		break;
	}

	if (readied_objects[location] != NULL)
		return false;

	return true;
}

//FIX handle not readiable, no place to put, double handed objects
bool Actor::add_readied_object(Obj *obj) {
	uint8 location;
	bool double_handed = false;

	location =  get_object_readiable_location(obj);

	switch (location) {
	case ACTOR_NOT_READIABLE :
		return false;

	case ACTOR_ARM :
		if (readied_objects[ACTOR_ARM] != NULL) { //if full try other arm
			if (readied_objects[ACTOR_ARM]->double_handed)
				return false;

			location = ACTOR_ARM_2;
		}
		break;

	case ACTOR_ARM_2 :
		if (readied_objects[ACTOR_ARM] != NULL || readied_objects[ACTOR_ARM_2] != NULL)
			return false;
		location = ACTOR_ARM;
		double_handed = true;
		break;

	case ACTOR_HAND :
		if (readied_objects[ACTOR_HAND] != NULL) // if full try other hand
			location = ACTOR_HAND_2;
		break;
	}

	if (readied_objects[location] != NULL)
		return false;

	readied_objects[location] = new ReadiedObj;

	if (obj->is_in_container())
		inventory_add_object_nostack(obj);

	readied_objects[location]->obj = obj;
	readied_objects[location]->combat_type = get_object_combat_type(obj->obj_n);
	readied_objects[location]->double_handed = double_handed;

	if (readied_objects[location]->combat_type != NULL)
		readied_armor_class += readied_objects[location]->combat_type->defence;

	obj->readied(); //set object to readied status
	return true;
}

void Actor::remove_readied_object(Obj *obj, bool run_usecode) {
	uint8 location;

	for (location = 0; location < ACTOR_MAX_READIED_OBJECTS; location++) {
		if (readied_objects[location] != NULL && readied_objects[location]->obj == obj) {
			remove_readied_object(location, run_usecode);
			break;
		}
	}

	return;
}

void Actor::remove_readied_object(uint8 location, bool run_usecode) {
	Obj *obj;

	obj = inventory_get_readied_object(location);

	if (obj) {
		if (readied_objects[location]->combat_type)
			readied_armor_class -= readied_objects[location]->combat_type->defence;
		if (obj_manager->get_usecode()->has_readycode(obj) && run_usecode)
			obj_manager->get_usecode()->ready_obj(obj, this);
		delete readied_objects[location];
		readied_objects[location] = NULL;
		//ERIC obj->status ^= 0x18; // remove "readied" bit flag.
		//ERIC obj->status |= OBJ_STATUS_IN_INVENTORY; // keep "in inventory"
		obj->set_in_inventory();

		if (location == ACTOR_ARM && readied_objects[ACTOR_ARM_2] != NULL) { //move contents of left hand to right hand.
			readied_objects[ACTOR_ARM] = readied_objects[ACTOR_ARM_2];
			readied_objects[ACTOR_ARM_2] = NULL;
		}

		if (location == ACTOR_HAND && readied_objects[ACTOR_HAND_2] != NULL) { //move contents of left hand to right hand.
			readied_objects[ACTOR_HAND] = readied_objects[ACTOR_HAND_2];
			readied_objects[ACTOR_HAND_2] = NULL;
		}
	}

	return;
}

void Actor::remove_all_readied_objects() {
	uint8 location;

	for (location = 0; location < ACTOR_MAX_READIED_OBJECTS; location++) {
		if (readied_objects[location] != NULL)
			remove_readied_object(location);
	}

	return;
}

// returns true if the actor has one or more readied objects
bool Actor::has_readied_objects() {
	uint8 location;

	for (location = 0; location < ACTOR_MAX_READIED_OBJECTS; location++) {
		if (readied_objects[location] != NULL)
			return true;
	}

	return false;

}

void Actor::inventory_drop_all() {
	U6LList *inv = NULL;
	Obj *obj = NULL;

	while (inventory_count_objects(true)) {
		inv = get_inventory_list();
		obj = (Obj *)(inv->start()->data);
		if (!inventory_remove_obj(obj))
			break;

		Tile *obj_tile = obj_manager->get_obj_tile(obj->obj_n, obj->frame_n);
		if (obj_tile && (obj_tile->flags3 & TILEFLAG_IGNORE)) { //Don't drop charges.
			delete_obj(obj);
		} else {
			if (temp_actor)
				obj->status |= OBJ_STATUS_TEMPORARY;
			obj->status |= OBJ_STATUS_OK_TO_TAKE;
			obj->x = x;
			obj->y = y;
			obj->z = z;
			obj_manager->add_obj(obj, true); // add to map
		}
	}
}

// Moves inventory and all readied items into a container object.
void Actor::all_items_to_container(Obj *container_obj, bool stack) {
	U6LList *inventory;
	U6Link *link;
	Obj *obj;

	inventory = get_inventory_list();

	if (!inventory)
		return;

	for (link = inventory->start(); link != NULL;) {
		obj = (Obj *)link->data;
		link = link->next;

		if (temp_actor)
			obj->status |= OBJ_STATUS_TEMPORARY;

		Tile *obj_tile = obj_manager->get_obj_tile(obj->obj_n, obj->frame_n);
		if (obj_tile && obj_tile->flags3 & TILEFLAG_IGNORE) {
			inventory_remove_obj(obj);
			delete_obj(obj);
		} else
			obj_manager->moveto_container(obj, container_obj, stack);
	}


	return;
}

void Actor::loadSchedule(unsigned char *sched_data, uint16 num) {
	uint16 i;
	unsigned char *sched_data_ptr;

	sched = (Schedule **)malloc(sizeof(Schedule *) * (num + 1));
	num_schedules = num;
	sched_data_ptr = sched_data;

	for (i = 0; i < num; i++) {
		sched[i] = (Schedule *)malloc(sizeof(Schedule));

		sched[i]->hour = sched_data_ptr[0] & 0x1f; // 5 bits for hour
		sched[i]->day_of_week = sched_data_ptr[0] >> 5; // 3 bits for day of week
		sched[i]->worktype = sched_data_ptr[1];

		sched[i]->x = sched_data_ptr[2];
		sched[i]->x += (sched_data_ptr[3] & 0x3) << 8;

		sched[i]->y = (sched_data_ptr[3] & 0xfc) >> 2;
		sched[i]->y += (sched_data_ptr[4] & 0xf) << 6;

		sched[i]->z = (sched_data_ptr[4] & 0xf0) >> 4;
		sched_data_ptr += 5;
#ifdef ACTOR_DEBUG
		DEBUG(0, LEVEL_DEBUGGING, "#%04d %03x,%03x,%x hour %2d day of week %2d worktype %02x\n", id_n, sched[i]->x, sched[i]->y, sched[i]->z, sched[i]->hour, sched[i]->day_of_week, sched[i]->worktype);
#endif
	}

	sched[i] = NULL;

	return;
}

//FIX for day_of_week

bool Actor::updateSchedule(uint8 hour, bool teleport) {
	//uint8 day_of_week;
	uint16 new_pos;

	if (is_alive() == false //don't update schedule for dead actors.
	        || (Game::get_game()->get_player()->get_actor() == this
	            && Game::get_game()->get_event()->using_control_cheat()))
		return false;

//hour = clock->get_hour();
//	day_of_week = clock->get_day_of_week();

	new_pos = getSchedulePos(hour);

	if (new_pos == sched_pos) // schedules are the same so we do nothing.
		return false;

	sched_pos = new_pos;

	if (sched[sched_pos] == NULL)
		return false;

// U6: temp. fix for walking statues; they shouldn't have schedules
	if (Game::get_game()->get_game_type() == NUVIE_GAME_U6 && id_n >= 188 && id_n <= 200) {
		DEBUG(0, LEVEL_WARNING, "tried to update schedule for non-movable actor %d\n", id_n);
		return (false);
	}

	set_worktype(sched[sched_pos]->worktype);
	if (teleport)
		move(sched[sched_pos]->x, sched[sched_pos]->y, sched[sched_pos]->z, ACTOR_FORCE_MOVE);
	return true;
}

// returns the current schedule entry based on hour
uint16 Actor::getSchedulePos(uint8 hour) {
	uint16 i;

	for (i = 0; sched[i] != NULL; i++) {
		if (sched[i]->hour > hour) {
			if (i != 0)
				return i - 1;
			else // i == 0 this means we are in the last schedule entry
				for (; sched[i + 1] != NULL;)
					i++;
		}
	}

	if (i == 0)
		return 0;

	return i - 1;
}

/*
// returns the current schedule entry based on hour
uint16 Actor::getSchedulePos(uint8 hour, uint8 day_of_week)
{
 uint16 i,j;
 if(id_n == 133)
  DEBUG(0,LEVEL_DEBUGGING,".");

 i = getSchedulePos(hour);

 for(j=i;sched[j] != NULL && sched[j]->hour == sched[i]->hour;j++)
  {
   if(sched[j]->day_of_week > day_of_week)
	 {
	  if(j != i)
		return j-1;
	  else // hour is in the last schedule entry.
		{
		 for(;sched[j+1] != NULL && sched[j+1]->hour == sched[i]->hour;) // move to the last schedule entry.
		  j++;
		}
	 }
  }

 if(j==i)
  return j;

 return j-1;
}

inline uint16 Actor::getSchedulePos(uint8 hour)
{
 uint16 i;
 uint8 cur_hour;

 for(i=0;sched[i] != NULL;i++)
  {
   if(sched[i]->hour > hour)
	 {
	  if(i != 0)
		return i-1;
	  else // hour is in the last schedule entry.
		{
		 for(;sched[i+1] != NULL;) // move to the last schedule entry.
		  i++;

		 if(sched[i]->day_of_week > 0) //rewind to the start of the hour set.
		   {
			cur_hour = sched[i]->hour;
			for(;i >= 1 && sched[i-1]->hour == cur_hour;)
			  i--;
		   }
		}
	 }
   else
	  for(;sched[i+1] != NULL && sched[i+1]->hour == sched[i]->hour;) //skip to next hour set.
		i++;
  }

 if(sched[i] != NULL && sched[i]->day_of_week > 0) //rewind to the start of the hour set.
   {
	cur_hour = sched[i]->hour;
	for(;i >= 1 && sched[i-1]->hour == cur_hour;)
	  i--;
   }

 if(i==0)
  return 0;

 return i-1;
}
*/

void Actor::set_combat_mode(uint8 new_mode) {
	combat_mode = new_mode;
	if (Game::get_game()->get_party()->is_in_combat_mode()) {
		set_worktype(combat_mode);
	}
}

void Actor::set_worktype(uint8 new_worktype, bool init) {
	worktype = new_worktype;
	work_location.x = x;
	work_location.y = y;
	work_location.z = z;


	return ;
}

uint8 Actor::get_flag(uint8 bitflag) {
	if (bitflag > 7)
		return 0;

	return (talk_flags >> bitflag) & 1;
}

/* Set NPC flag `bitflag' to 1.
 */
void Actor::set_flag(uint8 bitflag) {
	if (bitflag > 7)
		return;
	talk_flags = talk_flags | (1 << bitflag);
}


/* Set NPC flag `bitflag' to 0.
 */
void Actor::clear_flag(uint8 bitflag) {
	if (bitflag > 7)
		return;
	talk_flags = talk_flags & ~(1 << bitflag);
}

Obj *Actor::make_obj() {
	Obj *obj;

	obj = new Obj();

	obj->x = x;
	obj->y = y;
	obj->z = z;

	obj->obj_n = obj_n;
	obj->frame_n = frame_n;
	obj->quality = id_n;
	obj->status = obj_flags;

	return obj;
}

void Actor::clear() {
	x = 0;
	y = 0;
	z = 0;
	hide();
	Actor::set_worktype(0);
	light = 0;
	light_source.clear();
}

void Actor::show() {
	visible_flag = true;

	Std::list<Obj *>::iterator obj_iter;
	for (obj_iter = surrounding_objects.begin(); obj_iter != surrounding_objects.end(); obj_iter++) {
		(*obj_iter)->set_invisible(false);
	}

}

void Actor::hide() {
	visible_flag = false;

	Std::list<Obj *>::iterator obj_iter;
	for (obj_iter = surrounding_objects.begin(); obj_iter != surrounding_objects.end(); obj_iter++) {
		(*obj_iter)->set_invisible(true);
	}
}

/* Get pushed by `pusher' to location determined by `where'. */
bool Actor::push(Actor *pusher, uint8 where) {
	if (where == ACTOR_PUSH_HERE) { // move to pusher's square and use up moves
		MapCoord to(pusher->x, pusher->y, pusher->z), from(get_location());
		if (to.distance(from) > 1 || z != to.z)
			return (false);
		face_location(to.x, to.y);
		move(to.x, to.y, to.z, ACTOR_FORCE_MOVE); // can even move onto blocked squares
		if (moves > 0)
			set_moves_left(0); // we use up our moves exchanging positions
		return (true);
	} else if (where == ACTOR_PUSH_ANYWHERE) { // go to any neighboring direction
		MapCoord from(get_location());
		const uint16 square = 1;
		if (this->push(pusher, ACTOR_PUSH_FORWARD))
			return (true); // prefer forward push
		for (uint16 xp = (from.x - square); xp <= (from.x + square); xp += square)
			for (uint16 yp = (from.y - square); yp <= (from.y + square); yp += square)
				if (xp != from.x && yp != from.y && move(xp, yp, from.z))
					return (true);
	} else if (where == ACTOR_PUSH_FORWARD) { // move away from pusher
		MapCoord from(get_location());
		MapCoord pusher_loc(pusher->x, pusher->y, pusher->z);
		if (pusher_loc.distance(from) > 1 || z != pusher->z)
			return (false);
		sint8 rel_x = -(pusher_loc.x - from.x), rel_y = -(pusher_loc.y - from.y);
		if (moveRelative(rel_x, rel_y)) {
			set_direction(rel_x, rel_y);
			return (true);
		}
	}
	return (false);
}

/* Subtract amount from hp. May die if hp is too low. */
void Actor::reduce_hp(uint8 amount) {
	DEBUG(0, LEVEL_DEBUGGING, "hit %s for %d points\n", get_name(), amount);

	if (amount <= hp)
		set_hp(hp - amount);
	else
		set_hp(0);
// FIXME... game specific?
	if (hp == 0)
		die();
}

void Actor::set_hp(uint8 val) {
	hp = val;
	if (is_in_party()) {
		Game::get_game()->get_view_manager()->update();
	}
}

void Actor::set_obj_flag(uint8 bitFlag, bool value) {
	if (value) {
		obj_flags |= (1 << bitFlag);
	} else {
		obj_flags &= (0xff ^ (1 << bitFlag));
	}
}

void Actor::set_status_flag(uint8 bitFlag, bool value) {
	if (value) {
		status_flags |= (1 << bitFlag);
	} else {
		status_flags &= (0xff ^ (1 << bitFlag));
	}
}

void Actor::set_hit_flag(bool val) {
	if (val) {
		movement_flags |= ACTOR_MOVEMENT_HIT_FLAG;
	} else {
		movement_flags &= (0xff ^ ACTOR_MOVEMENT_HIT_FLAG);
	}
}

void Actor::die(bool create_body) {
	hp = 0;
	visible_flag = false;
	Game *game = Game::get_game();

	if (game->get_game_type() != NUVIE_GAME_U6) // set in U6 before removing items for torch usecode
		set_dead_flag(true);                   // may need to add it elsewhere for other games

	if (game->get_player()->get_actor() == this && game->get_event()->using_control_cheat())
		game->get_event()->party_mode();
	if (is_temp())
		game->get_actor_manager()->clear_actor(this);
}

void Actor::resurrect(MapCoord new_position, Obj *body_obj) {
	U6Link *link;
	bool remove_obj = false;

	if (body_obj == NULL) {
		body_obj = find_body();
		if (body_obj != NULL)
			remove_obj = true;
	}

	set_dead_flag(false);

	show();

	x = new_position.x;
	y = new_position.y;
	z = new_position.z;
	obj_n = base_obj_n;
	init((Game::get_game()->get_game_type() == NUVIE_GAME_U6 && id_n == 130)
	     ? OBJ_STATUS_MUTANT : NO_OBJ_STATUS);

	frame_n = 0;

	set_direction(NUVIE_DIR_N);
	if (Game::get_game()->get_game_type() == NUVIE_GAME_U6)
		((U6Actor *)this)->do_twitch(); // fixes actors with more than 1 tile

	set_hp(1);
	//actor->set_worktype(0x1);

	if (is_in_party()) //actor in party
		Game::get_game()->get_party()->add_actor(this);

	if (body_obj != NULL) {
		//add body container objects back into actor's inventory.
		if (body_obj->has_container()) {
			for (link = body_obj->container->start(); link != NULL;) {
				Obj *inv_obj = (Obj *)link->data;
				link = link->next;
				inventory_add_object(inv_obj);
			}

			body_obj->container->removeAll();
		}

		obj_manager->unlink_from_engine(body_obj);
	}

	if (remove_obj)
		delete_obj(body_obj);

	Game::get_game()->get_script()->call_actor_resurrect(this);

	return;
}

void Actor::display_condition() {
	MsgScroll *scroll = Game::get_game()->get_scroll();

	if (hp == get_maxhp())
		return;
	scroll->display_string(get_name());
	scroll->display_string(" ");
	if (hp < get_maxhp() / 4) // 25%
		scroll->display_string("critical!\n");
	else {
		if (hp < get_maxhp() / 2) // 50%
			scroll->display_string("heavily");
		else if (hp < get_maxhp() / 1.33) // 75%
			scroll->display_string("lightly");
		else
			scroll->display_string("barely");
		scroll->display_string(" wounded.\n");
	}
}

/* Get hit and take damage by some indirect effect. (no source) */
void Actor::hit(uint8 dmg, bool force_hit) {
	MsgScroll *scroll = Game::get_game()->get_scroll();
	uint8 total_armor_class = body_armor_class; //+ readied_armor_class;

	if (dmg == 0) {
		scroll->display_string(get_name());
		scroll->display_string(" grazed!\n");
	} else if (dmg > total_armor_class || force_hit) {
		new HitEffect(this);
		reduce_hp(force_hit ? dmg : dmg - total_armor_class);

//    if(!force_hit)
//      {
		if (hp == 0) {
			scroll->display_string(get_name());
			scroll->display_string(" killed!\n");
		} else {
			display_condition();
		}
//      }
	}
}

void Actor::attract_to(Actor *target) {
	delete_pathfinder();
	set_pathfinder(new CombatPathFinder(this));
	((CombatPathFinder *)pathfinder)->set_chase_mode(target);
}

void Actor::repel_from(Actor *target) {
	delete_pathfinder();
	set_pathfinder(new CombatPathFinder(this, target));
	((CombatPathFinder *)pathfinder)->set_flee_mode(target);
	((CombatPathFinder *)pathfinder)->set_distance(2);
}

uint8 Actor::get_light_level() {
	Tile *tile = get_tile();
	return MAX(light, GET_TILE_LIGHT_LEVEL(tile));
}

void Actor::add_light(uint8 val) {
	if (is_in_party() || (Actor *)this == Game::get_game()->get_player()->get_actor())
		Game::get_game()->get_party()->add_light_source();
//    light += val;
	light_source.push_back(val);
	if (val > light)
		light = val;
}

void Actor::subtract_light(uint8 val) {
	if (is_in_party() || (Actor *)this == Game::get_game()->get_player()->get_actor())
		Game::get_game()->get_party()->subtract_light_source();
//    if(light >= val)
//        light -= val;
//    else
//        light = 0;
	vector<uint8>::iterator l = light_source.begin();
	for (; l != light_source.end(); l++)
		if (*l == val) {
			light_source.erase(l);
			break;
		}
	light = 0; // change to next highest light source
	for (unsigned int lCtr = 0; lCtr < light_source.size(); lCtr++)
		if (light_source[lCtr] > light)
			light = light_source[lCtr];
}

void Actor::set_moves_left(sint8 val) {
	moves = clamp(val, -127, dex);
}

void Actor::set_dead_flag(bool value) {
	if (value)
		status_flags |= ACTOR_STATUS_DEAD;
	else if (!is_alive()) //if not alive then clear dead flag
		status_flags ^= ACTOR_STATUS_DEAD;

	return;
}

/* Set error/status information. */
void Actor::set_error(ActorErrorCode err) {
	clear_error();
	error_struct.err = err;
}

void Actor::clear_error() {
	error_struct.err = ACTOR_NO_ERROR;
	error_struct.blocking_obj = NULL;
	error_struct.blocking_actor = NULL;
}

ActorError *Actor::get_error() {
	return (&error_struct);
}

// frozen by worktype or status
bool Actor::is_immobile() {
	return (false);
}

void Actor::print() {
	Actor *actor = this;
	DEBUG(0, LEVEL_INFORMATIONAL, "\n");
	DEBUG(1, LEVEL_INFORMATIONAL, "%s at %x, %x, %x\n", get_name(), actor->x, actor->y, actor->z);
	DEBUG(1, LEVEL_INFORMATIONAL, "id_n: %d\n", actor->id_n);

	DEBUG(1, LEVEL_INFORMATIONAL, "obj_n: %03d    frame_n: %d\n", actor->obj_n, actor->frame_n);
	DEBUG(1, LEVEL_INFORMATIONAL, "base_obj_n: %03d    old_frame_n: %d\n", actor->base_obj_n, actor->old_frame_n);

	uint8 dir = actor->direction;
	DEBUG(1, LEVEL_INFORMATIONAL, "direction: %d (%s)\n", dir, (dir == NUVIE_DIR_N) ? "north" :
	      (dir == NUVIE_DIR_E) ? "east" :
	      (dir == NUVIE_DIR_S) ? "south" :
	      (dir == NUVIE_DIR_W) ? "west" : "???");
	DEBUG(1, LEVEL_INFORMATIONAL, "walk_frame: %d\n", actor->walk_frame);

	DEBUG(1, LEVEL_INFORMATIONAL, "can_move: %s\n", actor->can_move ? "true" : "false");
	DEBUG(1, LEVEL_INFORMATIONAL, "alive: %s\n", actor->is_alive() ? "true" : "false");
	DEBUG(1, LEVEL_INFORMATIONAL, "in_party: %s\n", is_in_party() ? "true" : "false");
	DEBUG(1, LEVEL_INFORMATIONAL, "visible_flag: %s\n", actor->visible_flag ? "true" : "false");
	DEBUG(1, LEVEL_INFORMATIONAL, "met_player: %s\n", actor->met_player ? "true" : "false");
	DEBUG(1, LEVEL_INFORMATIONAL, "is_immobile: %s\n", actor->is_immobile() ? "true" : "false");

	DEBUG(1, LEVEL_INFORMATIONAL, "moves: %d\n", actor->moves);

	const char *wt_string = get_worktype_string(actor->worktype);
	if (!wt_string) wt_string = "???";
	DEBUG(1, LEVEL_INFORMATIONAL, "worktype: 0x%02x/%03d %s\n", actor->worktype, actor->worktype, wt_string);

	DEBUG(1, LEVEL_INFORMATIONAL, "NPC stats:\n");
	DEBUG(1, LEVEL_INFORMATIONAL, " level: %d    exp: %d    hp: %d / %d\n", actor->level, actor->exp,
	      actor->hp, actor->get_maxhp());
	DEBUG(1, LEVEL_INFORMATIONAL, " strength: %d    dex: %d    int: %d\n", actor->strength, actor->dex,
	      actor->intelligence);
	DEBUG(1, LEVEL_INFORMATIONAL, " magic: %d / %d\n", actor->magic, actor->get_maxmagic());

	DEBUG(1, LEVEL_INFORMATIONAL, "alignment: %s (%d)\n", get_actor_alignment_str(actor->get_alignment()), actor->get_alignment());

	uint8 combatMode = actor->combat_mode;
	wt_string = get_worktype_string(actor->combat_mode);
	if (!wt_string) wt_string = "???";
	DEBUG(1, LEVEL_INFORMATIONAL, "combat_mode: %d %s\n", combatMode, wt_string);

	DEBUG(1, LEVEL_INFORMATIONAL, "Object flags: ");
	print_b(LEVEL_INFORMATIONAL, actor->obj_flags);
	DEBUG(1, LEVEL_INFORMATIONAL, "\n");

	DEBUG(1, LEVEL_INFORMATIONAL, "NPC flags: ");
	print_b(LEVEL_INFORMATIONAL, actor->status_flags);
	DEBUG(1, LEVEL_INFORMATIONAL, "\n");

	DEBUG(1, LEVEL_INFORMATIONAL, "Talk flags: ");
	print_b(LEVEL_INFORMATIONAL, actor->talk_flags);
	DEBUG(1, LEVEL_INFORMATIONAL, "\n");

	uint32 inv = actor->inventory_count_objects(true);
	if (inv) {
		DEBUG(1, LEVEL_INFORMATIONAL, "Inventory (+readied): %d objects\n", inv);
		U6LList *inv_list = actor->get_inventory_list();
		for (U6Link *link = inv_list->start(); link != NULL; link = link->next) {
			Obj *obj = (Obj *)link->data;
			DEBUG(1, LEVEL_INFORMATIONAL, " %24s (%03d:%d) status=%d qual=%d qty=%d    (weighs %f)\n",
			      obj_manager->look_obj(obj), obj->obj_n, obj->frame_n, obj->status, obj->quality,
			      obj->qty, obj_manager->get_obj_weight(obj, false));
		}
		DEBUG(1, LEVEL_INFORMATIONAL, "(weight %f / %f)\n", actor->get_inventory_weight(),
		      actor->inventory_get_max_weight());
	}
	if (actor->sched && *actor->sched) {
		DEBUG(1, LEVEL_INFORMATIONAL, "Schedule:\n");
		Schedule **s = actor->sched;
		uint32 sp = 0;
		do {
			wt_string = get_worktype_string(s[sp]->worktype);
			if (!wt_string) wt_string = "???";
			if (sp == actor->sched_pos && s[sp]->worktype == actor->worktype)
				DEBUG(1, LEVEL_INFORMATIONAL, "*%d: location=0x%03x,0x%03x,0x%x  time=%02d:00  day=%d  worktype=0x%02x(%s)*\n", sp, s[sp]->x, s[sp]->y, s[sp]->z, s[sp]->hour, s[sp]->day_of_week, s[sp]->worktype, wt_string);
			else
				DEBUG(1, LEVEL_INFORMATIONAL, " %d: location=0x%03x,0x%03x,0x%x  time=%02d:00  day=%d  worktype=0x%02x(%s)\n", sp, s[sp]->x, s[sp]->y, s[sp]->z, s[sp]->hour, s[sp]->day_of_week, s[sp]->worktype, wt_string);
		} while (s[++sp]);
	}

	if (!actor->surrounding_objects.empty())
		DEBUG(1, LEVEL_INFORMATIONAL, "Actor has multiple tiles\n");
	if (actor->pathfinder)
		DEBUG(1, LEVEL_INFORMATIONAL, "Actor is on a path\n");
	DEBUG(1, LEVEL_INFORMATIONAL, "\n");
}


const char *get_actor_alignment_str(uint8 alignment) {
	switch (alignment) {
	case ACTOR_ALIGNMENT_DEFAULT :
		return "default";
	case ACTOR_ALIGNMENT_NEUTRAL :
		return "neutral";
	case ACTOR_ALIGNMENT_EVIL :
		return "evil";
	case ACTOR_ALIGNMENT_GOOD :
		return "good";
	case ACTOR_ALIGNMENT_CHAOTIC :
		return "chaotic";
	default :
		break;
	}

	return "unknown";
}

void Actor::set_invisible(bool invisible) {
	if (invisible) {
		if (!is_in_party())
			visible_flag = false;
		obj_flags |= OBJ_STATUS_INVISIBLE;
	} else {
		visible_flag = true;
		obj_flags &= ~OBJ_STATUS_INVISIBLE;
	}
}

sint8 Actor::count_readied_objects(sint32 objN, sint16 frameN, sint16 quality) {
	sint8 count = 0;
	for (int o = 0; o < ACTOR_MAX_READIED_OBJECTS; o++) {
		if (readied_objects[o] == 0) continue;
		if (objN == -1
		        || (readied_objects[o]->obj->obj_n == objN
		            && (frameN == -1 || frameN == readied_objects[o]->obj->frame_n)
		            && (quality == -1 || quality == readied_objects[o]->obj->quality)))
			++count;
	}
	return count;
}

// GOOD->CHAOTIC,EVIL
// NEUTRAL->CHAOTIC
// EVIL->GOOD,CHAOTIC
// CHAOTIC->ALL except CHAOTIC
ActorList *Actor::find_enemies() {
	const uint8 in_range = 24;
	ActorManager *actor_mgr = Game::get_game()->get_actor_manager();
	ActorList *actors = actor_mgr->filter_distance(actor_mgr->get_actor_list(), x, y, z, in_range);
	actor_mgr->filter_alignment(actors, alignment); // filter own alignment
	if (alignment != ACTOR_ALIGNMENT_CHAOTIC) {
		if (alignment == ACTOR_ALIGNMENT_NEUTRAL) {
			actor_mgr->filter_alignment(actors, ACTOR_ALIGNMENT_GOOD); // filter other friendlies
			actor_mgr->filter_alignment(actors, ACTOR_ALIGNMENT_EVIL);
		} else
			actor_mgr->filter_alignment(actors, ACTOR_ALIGNMENT_NEUTRAL);
	}

	// remove party members and invisible actors FIXME: set party members to leader's alignment
	ActorIterator a = actors->begin();
	while (a != actors->end())
		if (is_in_party() && (*a)->is_in_party())
			a = actors->erase(a);
		else if ((*a)->is_invisible())
			a = actors->erase(a);
		else ++a;
	if (actors->empty()) {
		delete actors;
		return NULL; // no enemies in range
	}
	return actors;
}

Obj *Actor::find_body() {
	Party *party;
	Actor *actor;
	Obj *body_obj = NULL;
	uint8 lvl;

	party = Game::get_game()->get_party();
	actor = party->who_has_obj(339, id_n, true);

	if (actor) //get from collective party inventory if possible
		return actor->inventory_get_object(339, id_n, OBJ_MATCH_QUALITY);

	// try to find on map.
	for (lvl = 0; lvl < 5 && body_obj == NULL; lvl++)
		body_obj = obj_manager->find_obj(lvl, 339, id_n);

	return body_obj;
}

/* Change actor type. */
bool Actor::morph(uint16 objN) {
	uint8 old_dir = get_direction(); // FIXME: this should get saved through init_from_obj()

	Obj *actor_obj = make_obj();
	actor_obj->obj_n = objN;
	actor_obj->frame_n = 0;
	init_from_obj(actor_obj);
	set_dead_flag(false);
	set_direction(old_dir); // FIXME: this should get saved through init_from_obj()
	return true;
}

bool Actor::get_schedule_location(MapCoord *loc) {
	if (sched[sched_pos] == NULL)
		return false;

	loc->x = sched[sched_pos]->x;
	loc->y = sched[sched_pos]->y;
	loc->z = sched[sched_pos]->z;
	return true;
}

bool Actor::is_at_scheduled_location() {
	if (sched[sched_pos] != NULL && x == sched[sched_pos]->x && y == sched[sched_pos]->y && z == sched[sched_pos]->z)
		return true;

	return false;
}

Schedule *Actor::get_schedule(uint8 index) {
	if (index >= num_schedules)
		return NULL;

	return sched[index];
}

void Actor::cure() {
	set_poisoned(false);
	set_paralyzed(false);
	set_charmed(false);
	set_corpser_flag(false);
	set_cursed(false);
	set_asleep(false);
}

void Actor::set_custom_tile_num(uint16 obj_num, uint16 tile_num) {
	if (custom_tile_tbl == NULL) {
		custom_tile_tbl = new Common::HashMap<uint16, uint16>();
	}

	(*custom_tile_tbl)[obj_num] = tile_num;
}

} // End of namespace Nuvie
} // End of namespace Ultima
