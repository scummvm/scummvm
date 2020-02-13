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
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/misc/u6_llist.h"

#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/usecode/u6_usecode.h"
#include "ultima/nuvie/pathfinder/sched_path_finder.h"
#include "ultima/nuvie/pathfinder/u6_astar_path.h"
#include "ultima/nuvie/gui/widgets/msg_scroll.h"
#include "ultima/nuvie/actors/u6_actor.h"

#include "ultima/nuvie/core/party.h"
#include "ultima/nuvie/actors/actor_manager.h"
#include "ultima/nuvie/views/view_manager.h"
#include "ultima/nuvie/sound/sound_manager.h"
#include "ultima/nuvie/core/converse.h"
#include "ultima/nuvie/script/script.h"
#include "ultima/nuvie/core/effect.h"
#include "ultima/nuvie/pathfinder/combat_path_finder.h"

#include "ultima/nuvie/actors/u6_actor_types.h"
#include "ultima/nuvie/actors/u6_work_types.h"
#include "ultima/nuvie/core/weather.h"

namespace Ultima {
namespace Nuvie {

U6Actor::U6Actor(Map *m, ObjManager *om, GameClock *c): Actor(m, om, c), actor_type(NULL),
	base_actor_type(NULL) {
	walk_frame_inc = 1;
	current_movetype = MOVETYPE_U6_NONE;
}

U6Actor::~U6Actor() {
}

bool U6Actor::init(uint8 obj_status) {
	Actor::init();
	base_actor_type = get_actor_type(base_obj_n);
	if (base_actor_type->base_obj_n != base_obj_n) {
		base_obj_n = base_actor_type->base_obj_n;
	}

	set_actor_obj_n(obj_n); //set actor_type

	current_movetype = actor_type->movetype;

	body_armor_class = base_actor_type->body_armor_class;

	if (actor_type->tile_type == ACTOR_QT && frame_n == 0) //set the two quad tile actors to correct frame number.
		frame_n = 3;

	discover_direction();

	if (has_surrounding_objs())
		clear_surrounding_objs_list(); //clean up the old list if required.

	if (is_alive() && x != 0 && y != 0) { //only try to init multi-tile actors if they are alive.
		switch (obj_n) { //gather surrounding objects from map if required
		case OBJ_U6_SHIP :
			init_ship();
			break;

		case OBJ_U6_HYDRA :
			init_hydra();
			break;

		case OBJ_U6_DRAGON :
			init_dragon();
			break;

		case OBJ_U6_SILVER_SERPENT :
			init_silver_serpent();
			break;

		case OBJ_U6_GIANT_SCORPION :
		case OBJ_U6_GIANT_ANT :
		case OBJ_U6_COW :
		case OBJ_U6_ALLIGATOR :
		case OBJ_U6_HORSE :
		case OBJ_U6_HORSE_WITH_RIDER :
			init_splitactor(obj_status);
			break;


		default :
			break;
		}
	}

	if (actor_type->can_sit) { // For some reason U6 starts with actors standing on their chairs.
		// We need to sit them down.
		Obj *obj = obj_manager->get_obj(x, y, z);
		sit_on_chair(obj); // attempt to sit on obj.
	}

	inventory_make_all_objs_ok_to_take();

	return true;
}

bool U6Actor::init_ship() {
	Obj *obj;
	uint16 obj1_x, obj1_y, obj2_x, obj2_y;

	obj1_x = x;
	obj1_y = y;
	obj2_x = x;
	obj2_y = y;

	switch (direction) {
	case NUVIE_DIR_N :
		obj1_y = y + 1;
		obj2_y = y - 1;
		break;
	case NUVIE_DIR_E :
		obj1_x = x + 1;
		obj2_x = x - 1;
		break;
	case NUVIE_DIR_S :
		obj1_y = y - 1;
		obj2_y = y + 1;
		break;
	case NUVIE_DIR_W :
		obj1_x = x - 1;
		obj2_x = x + 1;
		break;
	}

	obj = obj_manager->get_obj(obj1_x, obj1_y, z);
	if (obj == NULL)
		return false;
	add_surrounding_obj(obj);

	obj = obj_manager->get_obj(obj2_x, obj2_y, z);
	if (obj == NULL)
		return false;
	add_surrounding_obj(obj);

	return true;
}

bool U6Actor::init_splitactor(uint8 obj_status) {
	uint16 obj_x, obj_y;

	obj_x = x;
	obj_y = y;

	switch (direction) {
	case NUVIE_DIR_N :
		obj_y = WRAPPED_COORD(y + 1, z);
		break;
	case NUVIE_DIR_E :
		obj_x = WRAPPED_COORD(x - 1, z);
		break;
	case NUVIE_DIR_S :
		obj_y = WRAPPED_COORD(y - 1, z);
		break;
	case NUVIE_DIR_W :
		obj_x = WRAPPED_COORD(x + 1, z);
		break;
	}

// init back object
	if (obj_status & OBJ_STATUS_MUTANT) {
		init_surrounding_obj(obj_x, obj_y, z, obj_n, (get_reverse_direction(direction) * actor_type->tiles_per_direction + actor_type->tiles_per_frame - 1));
	} else {
		init_surrounding_obj(obj_x, obj_y, z, obj_n, frame_n + 8);
	}

	return true;
}

bool U6Actor::init_dragon() {
	uint16 head_x, head_y, tail_x, tail_y;
	uint16 wing1_x, wing1_y, wing2_x, wing2_y;

	head_x = tail_x = x;
	wing1_x = wing2_x = x;
	head_y = tail_y = y;
	wing1_y = wing2_y = y;

	switch (direction) {
	case NUVIE_DIR_N :
		head_y = y - 1;
		tail_y = y + 1;
		wing1_x = x - 1;
		wing2_x = x + 1;
		break;
	case NUVIE_DIR_E :
		head_x = x + 1;
		tail_x = x - 1;
		wing1_y = y - 1;
		wing2_y = y + 1;
		break;
	case NUVIE_DIR_S :
		head_y = y + 1;
		tail_y = y - 1;
		wing1_x = x + 1;
		wing2_x = x - 1;
		break;
	case NUVIE_DIR_W :
		head_x = x - 1;
		tail_x = x + 1;
		wing1_y = y + 1;
		wing2_y = y - 1;
		break;
	}

	init_surrounding_obj(head_x, head_y, z, obj_n, frame_n + 8);
	init_surrounding_obj(tail_x, tail_y, z, obj_n, frame_n + 16);
	init_surrounding_obj(wing1_x, wing1_y, z, obj_n, frame_n + 24);
	init_surrounding_obj(wing2_x, wing2_y, z, obj_n, frame_n + 32);

	return true;
}

bool U6Actor::init_hydra() {
// For some reason a Hydra has a different object number for its tenticles. :-(

	init_surrounding_obj(x,   y - 1, z, OBJ_U6_HYDRA_BODY, 0);
	init_surrounding_obj(x + 1, y - 1, z, OBJ_U6_HYDRA_BODY, 4);
	init_surrounding_obj(x + 1, y, z, OBJ_U6_HYDRA_BODY, 8);
	init_surrounding_obj(x + 1, y + 1, z, OBJ_U6_HYDRA_BODY, 12);
	init_surrounding_obj(x,   y + 1, z, OBJ_U6_HYDRA_BODY, 16);
	init_surrounding_obj(x - 1, y + 1, z, OBJ_U6_HYDRA_BODY, 20);
	init_surrounding_obj(x - 1, y, z, OBJ_U6_HYDRA_BODY, 24);
	init_surrounding_obj(x - 1, y - 1, z, OBJ_U6_HYDRA_BODY, 28);

	return true;
}

bool U6Actor::init_silver_serpent() {
	uint16 sx, sy, sz;
	Obj *obj;
	uint8 tmp_frame_n = 0;

	sx = x;
	sy = y;
	sz = z;

	switch (direction) {
	case NUVIE_DIR_N :
		sy++;
		tmp_frame_n = 1;
		break;
	case NUVIE_DIR_E :
		sx--;
		tmp_frame_n = 3;
		break;
	case NUVIE_DIR_S :
		sy--;
		tmp_frame_n = 5;
		break;
	case NUVIE_DIR_W :
		sx++;
		tmp_frame_n = 7;
		break;
	}

	obj = obj_manager->get_obj_of_type_from_location(OBJ_U6_SILVER_SERPENT, 1, id_n, sx, sy, sz);

	if (obj != NULL) //old snake
		gather_snake_objs_from_map(obj, x, y, z);
	else { //new snake
		//FIXME we need to make long, randomly layed out snakes here!
		init_new_silver_serpent();
	}

	// FIXME: Unused variable
	(void)tmp_frame_n;

	return true;
}

void U6Actor::init_new_silver_serpent() {
	const struct {
		uint8 body_frame_n;
		uint8 tail_frame_n;
		sint8 x_offset;
		sint8 y_offset;
	} movetbl[4] = { {10, 1, 0, 1}, {13, 7, 1, 0}, {12, 5, 0, -1}, {11, 3, -1, 0} };

	uint8 i, j;
	uint16 nx, ny;
	Obj *obj;
	uint8 length = 4 + NUVIE_RAND() % 5; //FIXME. The original worked out length from qty in the serpent embryo obj.

	nx = x;
	ny = y;

	set_direction(NUVIE_DIR_N); //make sure we are facing north.

	for (i = 0, j = 0; i < length; i++) {
		nx += movetbl[j].x_offset;
		ny += movetbl[j].y_offset;

		init_surrounding_obj(nx, ny, z, OBJ_U6_SILVER_SERPENT, (i == length - 1 ? movetbl[j].tail_frame_n : movetbl[j].body_frame_n));

		obj = (Obj *)surrounding_objects.back();
		obj->quality = i + 1; //body segment number
		obj->qty = id_n; //actor id number

		j = (j + 1) % 4;
	}

	return;
}

void U6Actor::gather_snake_objs_from_map(Obj *start_obj, uint16 ax, uint16 ay, uint16 az) {
	Obj *obj;
	uint16 px, py; // , pz;
	uint16 nx, ny, nz;
	uint8 seg_num;

	px = ax;
	py = ay;
//	pz = az;

	obj = start_obj;
	add_surrounding_obj(obj);

	for (seg_num = 2; obj && obj->frame_n >= 8; seg_num++) {

		nx = obj->x;
		ny = obj->y;
		nz = obj->z;
		//work out the location of the next obj based on the current frame_n and relative movement.
		switch (obj->frame_n) {
		//up down
		case  8 :
			if (ny - 1 == py)
				ny++;
			else
				ny--;
			break;
		//left right
		case  9 :
			if (nx - 1 == px)
				nx++;
			else
				nx--;
			break;
		//up right
		case 10 :
			if (ny - 1 == py)
				nx++;
			else
				ny--;
			break;
		//down right
		case 11 :
			if (ny + 1 == py)
				nx++;
			else
				ny++;
			break;
		//left down
		case 12 :
			if (nx - 1 == px)
				ny++;
			else
				nx--;
			break;
		//left up
		case 13 :
			if (nx - 1 == px)
				ny--;
			else
				nx--;
			break;
		}

		px = obj->x;
		py = obj->y;
		//pz = obj->z;

		obj = obj_manager->get_obj_of_type_from_location(OBJ_U6_SILVER_SERPENT, seg_num, id_n, nx, ny, nz);

		if (obj)
			add_surrounding_obj(obj);
	}

}

uint16 U6Actor::get_downward_facing_tile_num() {
	uint8 shift = 0;

	if (base_actor_type->frames_per_direction > 1) //we want the second frame for most actor types.
		shift = 1;

	return get_tile_num(base_actor_type->base_obj_n) + base_actor_type->tile_start_offset + (NUVIE_DIR_S * base_actor_type->tiles_per_direction + base_actor_type->tiles_per_frame - 1) + shift;
}

bool U6Actor::updateSchedule(uint8 hour, bool teleport) {
	bool ret;
	handle_lightsource(hour);

	if ((ret = Actor::updateSchedule(hour, teleport)) == true) { //walk to next schedule location if required.
		if (sched[sched_pos] != NULL && (sched[sched_pos]->x != x || sched[sched_pos]->y != y || sched[sched_pos]->z != z
		                                 || worktype == WORKTYPE_U6_SLEEP)) { // needed to go underneath bed if teleporting
			set_worktype(WORKTYPE_U6_WALK_TO_LOCATION);
			MapCoord loc(sched[sched_pos]->x, sched[sched_pos]->y, sched[sched_pos]->z);
			pathfind_to(loc);
		}
	}

	return ret;
}

// workout our direction based on actor_type and frame_n
inline void U6Actor::discover_direction() {
	if (actor_type->frames_per_direction != 0)
		direction = (frame_n - actor_type->tile_start_offset) / actor_type->tiles_per_direction;
	else
		direction = NUVIE_DIR_S;
}

void U6Actor::change_base_obj_n(uint16 val) {
	Actor::change_base_obj_n(val);
	clear_surrounding_objs_list(REMOVE_SURROUNDING_OBJS);
	init();
}

void U6Actor::set_direction(uint8 d) {
	if (is_alive() == false || is_immobile())
		return;

	uint8 frames_per_dir = (actor_type->frames_per_direction != 0)
	                       ? actor_type->frames_per_direction : 4;
	if (d >= 4)
		return;

	if (walk_frame == 0)
		walk_frame_inc = 1; // loop forward
	else if (walk_frame == (frames_per_dir - 1))
		walk_frame_inc = -1; // loop backward
	walk_frame = (walk_frame + walk_frame_inc) % frames_per_dir;

	if (has_surrounding_objs()) {
		if (direction != d)
			set_direction_of_surrounding_objs(d);
		else {
			if (can_move && actor_type->twitch_rand) //only twitch actors with a non zero twitch_rand.
				twitch_surrounding_objs();
		}
	}

	direction = d;

//only change direction frame if the actor can twitch ie isn't sitting or in bed etc.
	if (can_move && obj_n != OBJ_U6_SLIME)
		frame_n = actor_type->tile_start_offset + (direction * actor_type->tiles_per_direction +
		          (walk_frame * actor_type->tiles_per_frame) + actor_type->tiles_per_frame - 1);

// tangle vines' north and east frames are in the wrong direction
// FIXME: see if the ActorType values can be changed to fix this
	if (obj_n == OBJ_U6_TANGLE_VINE)
		if (direction == NUVIE_DIR_N || direction == NUVIE_DIR_E)
			frame_n += 3;
}

void U6Actor::face_location(uint16 lx, uint16 ly) {
	if (obj_n != OBJ_U6_SILVER_SERPENT //snakes cannot turn on the spot.
	        && obj_n != OBJ_U6_TANGLE_VINE && obj_n != OBJ_U6_TANGLE_VINE_POD)
		Actor::face_location(lx, ly);

	return;
}

void U6Actor::clear() {
	if (has_surrounding_objs()) {
		remove_surrounding_objs_from_map();
		clear_surrounding_objs_list(REMOVE_SURROUNDING_OBJS);
	}

	Actor::clear();

	return;
}



bool U6Actor::move(uint16 new_x, uint16 new_y, uint8 new_z, ActorMoveFlags flags) {
	assert(new_z < 6);

// bool force_move = flags & ACTOR_FORCE_MOVE;
	bool ret;
	sint16 rel_x, rel_y;
//MsgScroll *scroll = Game::get_game()->get_scroll();
	Player *player = Game::get_game()->get_player();
//Party *party = player->get_party();
	MapCoord old_pos = get_location();

	if (has_surrounding_objs())
		remove_surrounding_objs_from_map();

	rel_x = new_x - x;
	rel_y = new_y - y;

	if ((flags & ACTOR_OPEN_DOORS) && worktype != WORKTYPE_U6_WALK_TO_LOCATION)
		flags ^= ACTOR_OPEN_DOORS; // only use doors when walking to schedule location
	ret = Actor::move(new_x, new_y, new_z, flags);

	if (ret == true) {
		if (has_surrounding_objs())
			move_surrounding_objs_relative(rel_x, rel_y);

		Obj *obj = obj_manager->get_obj(new_x, new_y, new_z, false); // Ouch, we get obj in Actor::move() too :(
		if (obj) {
			if (actor_type->can_sit)
				sit_on_chair(obj); // make the Actor sit if they are on top of a chair.
		}

		set_hit_flag(false);
		Game::get_game()->get_script()->call_actor_map_dmg(this, get_location());
	}


	// temp. fix; this too should be done with UseCode (and don't move the mirror)
	if (old_pos.y > 0 && new_y > 0) {
		Obj *old_mirror = obj_manager->get_obj_of_type_from_location(OBJ_U6_MIRROR, old_pos.x, old_pos.y - 1, old_pos.z);
		Obj *mirror = obj_manager->get_obj_of_type_from_location(OBJ_U6_MIRROR, new_x, new_y - 1, new_z);
		if (old_mirror && old_mirror->frame_n != 2) old_mirror->frame_n = 0;
		if (mirror && mirror->frame_n != 2)     mirror->frame_n = 1;
	}

	// Cyclops: shake ground if player is near
	if (actor_type->base_obj_n == OBJ_U6_CYCLOPS && is_nearby(player->get_actor())) {
		Game::get_game()->get_sound_manager()->playSfx(NUVIE_SFX_EARTH_QUAKE);
		new QuakeEffect(1, 200, player->get_actor());
	}

	if (has_surrounding_objs()) //add our surrounding objects back onto the map.
		add_surrounding_objs_to_map();

	return ret;
}

bool U6Actor::check_move(uint16 new_x, uint16 new_y, uint8 new_z, ActorMoveFlags flags) {
// bool ignore_actors = flags & ACTOR_IGNORE_OTHERS;
	Tile *map_tile;

	if (Actor::check_move(new_x, new_y, new_z, flags) == false)
		return false;

	if (obj_n == OBJ_U6_SILVER_SERPENT && check_move_silver_serpent(new_x, new_y) == false)
		return false;

	switch (current_movetype) {
	case MOVETYPE_U6_ETHEREAL:
		return true;
	case MOVETYPE_U6_NONE :
		return false;
	case MOVETYPE_U6_WATER_HIGH : // for HIGH we only want to move to open water.
		// No shorelines.
		map_tile = map->get_tile(new_x, new_y, new_z, MAP_ORIGINAL_TILE);
		if (map_tile->tile_num >= 16 && map_tile->tile_num <= 47)
			return false;

		if (!map->is_water(new_x, new_y, new_z))
			return false;
		break;

	case MOVETYPE_U6_WATER_LOW :
		if (!map->is_water(new_x, new_y, new_z))
			return false;
		break;

	case MOVETYPE_U6_AIR_LOW :
		map_tile = map->get_tile(new_x, new_y, new_z, MAP_ORIGINAL_TILE);
		if (map_tile->flags1 & TILEFLAG_WALL) //low air boundry
			return false;

		map_tile = obj_manager->get_obj_tile(new_x, new_y, new_z, false);
		if (map_tile && ((map_tile->flags1 & TILEFLAG_WALL) ||
		                 (map_tile->flags2 & (TILEFLAG_DOUBLE_WIDTH | TILEFLAG_DOUBLE_HEIGHT)) == (TILEFLAG_DOUBLE_WIDTH | TILEFLAG_DOUBLE_HEIGHT)))
			return false;
		break;

	case MOVETYPE_U6_AIR_HIGH :
		if (map->is_boundary(new_x, new_y, new_z))
			return false; //FIX for proper air boundary
		break;
	case MOVETYPE_U6_LAND :
	default :
		if (map->is_passable(new_x, new_y, new_z) == false) {
			if (obj_n == OBJ_U6_MOUSE // try to go through mousehole
			        && (obj_manager->get_obj_of_type_from_location(OBJ_U6_MOUSEHOLE, new_x, new_y, new_z) != NULL
			            || obj_manager->get_obj_of_type_from_location(OBJ_U6_BARS, new_x, new_y, new_z) != NULL
			            || obj_manager->get_obj_of_type_from_location(OBJ_U6_PORTCULLIS, new_x, new_y, new_z) != NULL))
				return (true);
			if (obj_n == OBJ_U6_SILVER_SERPENT //silver serpents can crossover themselves
			        && obj_manager->get_obj_of_type_from_location(OBJ_U6_SILVER_SERPENT, new_x, new_y, new_z) != NULL)
				return (true);

			return false;
		}

	}

	return (true);
}

bool U6Actor::check_move_silver_serpent(uint16 new_x, uint16 new_y) {
	if (new_x != x && new_y != y) //snakes can't move diagonally
		return false;

	Obj *obj = (Obj *)surrounding_objects.front(); //retrieve the first body segment.

	if (obj->x == new_x && obj->y == new_y) //snakes can't move backwards.
		return false;

	return true;
}

// attempt to sit if obj is a chair.

bool U6Actor::sit_on_chair(Obj *obj) {
	if (actor_type->can_sit && obj) {
		if (obj->obj_n == OBJ_U6_CHAIR) { // make the actor sit on a chair.
			if (obj_n == OBJ_U6_MUSICIAN_PLAYING)
				frame_n = (obj->frame_n * 2);
			else
				frame_n = (obj->frame_n * 4) + 3;
			direction = obj->frame_n;
			can_move = false;
			return true;
		}

		//make actor sit on LB's throne.
		if (obj->obj_n == OBJ_U6_THRONE  && obj->x != x) { //throne is a double width obj. We only sit on the left tile.
			frame_n = 8 + 3; //sitting facing south.
			direction = NUVIE_DIR_S;
			can_move = false;
			return true;
		}
	}

	return false;
}

uint8 U6Actor::get_object_readiable_location(Obj *obj) {
	uint16 i;

	for (i = 0; readiable_objects[i].obj_n != OBJ_U6_NOTHING; i++) {
		if (obj->obj_n == readiable_objects[i].obj_n)
			return readiable_objects[i].readiable_location;
	}

	return ACTOR_NOT_READIABLE;
}

const CombatType *U6Actor::get_object_combat_type(uint16 objN) {
	uint16 i;

	for (i = 0; u6combat_objects[i].obj_n != OBJ_U6_NOTHING; i++) {
		if (objN == u6combat_objects[i].obj_n)
			return &u6combat_objects[i];
	}

	return NULL;
}

const CombatType *U6Actor::get_hand_combat_type() {
	if (obj_n == OBJ_U6_SHIP)
		return &u6combat_ship_cannon;

	return &u6combat_hand;
}

bool U6Actor::weapon_can_hit(const CombatType *weapon, Actor *target, uint16 *hit_x, uint16 *hit_y) {
	if (Actor::weapon_can_hit(weapon, target->get_x(), target->get_y())) {
		*hit_x = target->get_x();
		*hit_y = target->get_y();
		return true;
	}

	Std::list<Obj *> *surrounding_objs = target->get_surrounding_obj_list();

	if (surrounding_objs) {
		Std::list<Obj *>::iterator obj_iter;
		for (obj_iter = surrounding_objs->begin(); obj_iter != surrounding_objs->end(); obj_iter++) {
			Obj *obj = *obj_iter;
			if (Actor::weapon_can_hit(weapon, obj->x, obj->y)) {
				*hit_x = obj->x;
				*hit_y = obj->y;
				return true;
			}
		}
	}

	uint16 target_x = target->get_x();
	uint16 target_y = target->get_y();

	Tile *tile = target->get_tile();
	if (tile->dbl_width && tile->dbl_height) {
		if (Actor::weapon_can_hit(weapon, target_x - 1, target_y - 1)) {
			*hit_x = target_x - 1;
			*hit_y = target_y - 1;
			return true;
		}
	}
	if (tile->dbl_width) {
		if (Actor::weapon_can_hit(weapon, target_x - 1, target_y)) {
			*hit_x = target_x - 1;
			*hit_y = target_y;
			return true;
		}
	}
	if (tile->dbl_height) {
		if (Actor::weapon_can_hit(weapon, target_x, target_y - 1)) {
			*hit_x = target_x;
			*hit_y = target_y - 1;
			return true;
		}
	}

	return false;
}

void U6Actor::twitch() {

	if (can_twitch() == false)
		return;

	if (NUVIE_RAND() % actor_type->twitch_rand == 1)
		do_twitch();

	return;
}

void U6Actor::do_twitch() {
	if (actor_type->frames_per_direction == 0)
		walk_frame = (walk_frame + 1) % 4;
	else
		walk_frame = NUVIE_RAND() % actor_type->frames_per_direction;

	if (has_surrounding_objs()) {
		switch (obj_n) {
		case OBJ_U6_HYDRA :
			twitch_surrounding_hydra_objs();
			break;
		case OBJ_U6_DRAGON :
		default :
			twitch_surrounding_objs();
			break;
		}
	}

	frame_n = actor_type->tile_start_offset + (direction * actor_type->tiles_per_direction + (walk_frame * actor_type->tiles_per_frame)  + actor_type->tiles_per_frame - 1);
	if (obj_n == OBJ_U6_WISP) {
		Game::get_game()->get_map_window()->updateAmbience();
	}
}

void U6Actor::set_paralyzed(bool paralyzed) {
	if (paralyzed) {
		status_flags |= ACTOR_STATUS_PARALYZED;
	} else {
		status_flags &= (0xff ^ ACTOR_STATUS_PARALYZED);
	}
}

void U6Actor::set_protected(bool val) {
	if (val) {
		status_flags |= ACTOR_STATUS_PROTECTED;
	} else {
		status_flags &= (0xff ^ ACTOR_STATUS_PROTECTED);
	}
}

void U6Actor::set_charmed(bool val) {
	if (val) {
		obj_flags |= OBJ_STATUS_CHARMED;
	} else {
		obj_flags &= (0xff ^ OBJ_STATUS_CHARMED);
	}
}

void U6Actor::set_corpser_flag(bool val) {
	if (val) {
		movement_flags |= ACTOR_MOVEMENT_FLAGS_CORPSER;
	} else {
		movement_flags &= (0xff ^ ACTOR_MOVEMENT_FLAGS_CORPSER);
	}
}

void U6Actor::set_cursed(bool val) {
	if (val) {
		obj_flags |= OBJ_STATUS_CURSED;
	} else {
		obj_flags &= (0xff ^ OBJ_STATUS_CURSED);
	}
}

void U6Actor::set_asleep(bool val) {
	if (val) {
		status_flags |= ACTOR_STATUS_ASLEEP;
		if (actor_type->dead_obj_n != OBJ_U6_NOTHING && actor_type->can_laydown) {
			obj_n = actor_type->dead_obj_n;
			frame_n = actor_type->dead_frame_n;
		}
	} else {
		status_flags &= (0xff ^ ACTOR_STATUS_ASLEEP);
		if (obj_n == base_actor_type->dead_obj_n || obj_n ==  OBJ_U6_PERSON_SLEEPING) {
			if (worktype == WORKTYPE_U6_SLEEP)
				can_move = true;
			actor_type = base_actor_type;
			obj_n = base_actor_type->base_obj_n;
			frame_n = 0;
		}
	}
}

void U6Actor::set_worktype(uint8 new_worktype, bool init) {
	if (new_worktype == worktype)
		return;

	if (worktype == WORKTYPE_U6_SLEEP || worktype == WORKTYPE_U6_PLAY_LUTE) {
		frame_n = old_frame_n;
	}

//reset to base obj_n
	if ((!is_in_party() || worktype > 0xe) && base_actor_type->base_obj_n != OBJ_U6_NOTHING) //don't revert for party worktypes as they might be riding a horse.
		set_actor_obj_n(base_actor_type->base_obj_n);

	if (worktype == WORKTYPE_U6_SLEEP && (status_flags & ACTOR_STATUS_ASLEEP)) //FIXME do we still need this??
		status_flags ^= ACTOR_STATUS_ASLEEP;

	Actor::set_worktype(new_worktype);

	if (worktype == WORKTYPE_U6_WALK_TO_LOCATION) {
		setup_walk_to_location();
	}

//FIX from here.

	switch (worktype) {
	case WORKTYPE_U6_FACE_NORTH :
		set_direction(NUVIE_DIR_N);
		break;
	case WORKTYPE_U6_FACE_EAST  :
		set_direction(NUVIE_DIR_E);
		break;
	case WORKTYPE_U6_FACE_SOUTH :
		set_direction(NUVIE_DIR_S);
		break;
	case WORKTYPE_U6_FACE_WEST  :
		set_direction(NUVIE_DIR_W);
		break;

	case WORKTYPE_U6_SLEEP :
		wt_sleep(init);
		break;
	case WORKTYPE_U6_PLAY_LUTE :
		wt_play_lute();
		break;
	}
}


void U6Actor::pathfind_to(MapCoord &d) {
	if (pathfinder) {
		pathfinder->set_actor(this);
		pathfinder->set_goal(d);
	} else
		set_pathfinder(new SchedPathFinder(this, d, new U6AStarPath));

	pathfinder->update_location();
}

void U6Actor::setup_walk_to_location() {
	if (sched[sched_pos] != NULL) {
		if (x == sched[sched_pos]->x && y == sched[sched_pos]->y
		        && z == sched[sched_pos]->z) {
			set_worktype(sched[sched_pos]->worktype);
			delete_pathfinder();
			return;
		}
		if (!pathfinder) {
			work_location.x = sched[sched_pos]->x;
			work_location.y = sched[sched_pos]->y;
			work_location.z = sched[sched_pos]->z;
//            if(!work_location.is_visible() || !get_location().is_visible())
//                set_pathfinder(new OffScreenPathFinder(this, work_location, new U6AStarPath));
//            else
			set_pathfinder(new SchedPathFinder(this, work_location, new U6AStarPath));
		}
	}
}



// wander around but don't cross boundaries or fences. Used for cows and horses.
// now that hazards are working properly, this isn't needed --SB-X
/*void U6Actor::wt_farm_animal_wander()
{
 uint8 new_direction;
 sint8 rel_x = 0, rel_y = 0;

 if(NUVIE_RAND()%8 == 1)
   {
    new_direction = NUVIE_RAND()%4;

    switch(new_direction)
      {
       case NUVIE_DIR_N : rel_y = -1; break;
       case NUVIE_DIR_E : rel_x = 1; break;
       case NUVIE_DIR_S : rel_y = 1; break;
       case NUVIE_DIR_W : rel_x = -1; break;
      }

    if(obj_manager->get_obj_of_type_from_location(OBJ_U6_FENCE,x + rel_x, y + rel_y, z) == NULL)
        {
         if(moveRelative(rel_x,rel_y))
            set_direction(new_direction);
        }

   }
 else set_moves_left(moves - 5);

 return;
}*/

void U6Actor::wt_sleep(bool init) {
	if (init && !is_sleeping())
		return;
	Obj *obj = obj_manager->get_obj(x, y, z);

	can_move = false;
	status_flags |= ACTOR_STATUS_ASLEEP;
	if (obj) {
		if (obj->obj_n == OBJ_U6_BED) {
			if (obj->frame_n == 1 || obj->frame_n == 5) { //horizontal bed
				old_frame_n = frame_n;
				obj_n = OBJ_U6_PERSON_SLEEPING;
				frame_n = 0;
			}
			if (obj->frame_n == 7 || obj->frame_n == 10) { //vertical bed
				old_frame_n = frame_n;
				obj_n = OBJ_U6_PERSON_SLEEPING;
				frame_n = 1;
			}
			return;
		}
	}

// lay down on the ground using the dead body frame
	if (actor_type->can_laydown) {
		old_frame_n = frame_n;
		obj_n = actor_type->dead_obj_n;
		frame_n = actor_type->dead_frame_n;
	}

}

void U6Actor::wt_play_lute() {
	set_actor_obj_n(OBJ_U6_MUSICIAN_PLAYING);

	frame_n = direction * actor_type->tiles_per_direction;
	Obj *obj = obj_manager->get_obj(x, y, z);
	sit_on_chair(obj); // attempt to sit on obj.

	return;
}

void U6Actor::set_actor_obj_n(uint16 new_obj_n) {
	old_frame_n = frame_n;

	obj_n = new_obj_n;
	actor_type = get_actor_type(new_obj_n);

	return;
}

inline const U6ActorType *U6Actor::get_actor_type(uint16 new_obj_n) {
	const U6ActorType *type;

	for (type = u6ActorTypes; type->base_obj_n != OBJ_U6_NOTHING; type++) {
		if (type->base_obj_n == new_obj_n)
			break;
	}

	return type;
}

inline bool U6Actor::has_surrounding_objs() {
	if (actor_type->tile_type == ACTOR_DT || actor_type->tile_type == ACTOR_MT)
		return true;

	return false;
}

inline void U6Actor::remove_surrounding_objs_from_map() {
	Std::list<Obj *>::iterator obj;

	for (obj = surrounding_objects.begin(); obj != surrounding_objects.end(); obj++)
		obj_manager->remove_obj_from_map((*obj));

	return;
}

inline void U6Actor::add_surrounding_objs_to_map() {
	Std::list<Obj *>::reverse_iterator obj;

	for (obj = surrounding_objects.rbegin(); obj != surrounding_objects.rend(); ++obj)
		obj_manager->add_obj((*obj), OBJ_ADD_TOP);

	return;
}

inline void U6Actor::move_surrounding_objs_relative(sint16 rel_x, sint16 rel_y) {
	Std::list<Obj *>::iterator obj_iter;
	Obj *obj;

	if (obj_n == OBJ_U6_SILVER_SERPENT) {
		move_silver_serpent_objs_relative(rel_x, rel_y);
	} else {
		for (obj_iter = surrounding_objects.begin(); obj_iter != surrounding_objects.end(); obj_iter++) {
			obj = *obj_iter;
			obj->x = WRAPPED_COORD(obj->x + rel_x, z);
			obj->y = WRAPPED_COORD(obj->y + rel_y, z);
		}
	}

	return;
}

inline void U6Actor::move_silver_serpent_objs_relative(sint16 rel_x, sint16 rel_y) {
	Std::list<Obj *>::iterator obj;
	uint8 objFrameN;
	uint8 tmp_frame_n;
	uint16 old_x, old_y;
	uint16 tmp_x, tmp_y;
	sint8 new_pos;
	sint8 old_pos;

	const uint8 new_frame_n_tbl[5][5] = {
		{ 8, 10, 0, 13, 0},
		{12, 9, 0, 0, 13},
		{ 0, 0, 0, 0, 0},
		{11, 0, 0, 9, 10},
		{ 0, 11, 0, 12, 8}
	};

	const uint8 new_tail_frame_n_tbl[8][6] = {
		{0, 0, 0, 0, 0, 0},
		{1, 0, 0, 3, 7, 0},
		{0, 0, 0, 0, 0, 0},
		{0, 3, 0, 0, 5, 1},
		{0, 0, 0, 0, 0, 0},
		{5, 0, 3, 0, 0, 7},
		{0, 0, 0, 0, 0, 0},
		{0, 7, 1, 5, 0, 0}
	};

	if (surrounding_objects.empty())
		return;

	obj = surrounding_objects.begin();

	new_pos = 2 + rel_x + (rel_y * 2);

	old_x = (*obj)->x;
	old_y = (*obj)->y;

	(*obj)->x = x - rel_x; // old actor x
	(*obj)->y = y - rel_y; // old actor y

	old_pos = 2 + ((*obj)->x - old_x) + (((*obj)->y - old_y) * 2);

	objFrameN = (*obj)->frame_n;
	(*obj)->frame_n = new_frame_n_tbl[new_pos][old_pos];
	obj++;
	for (; obj != surrounding_objects.end(); obj++) {
		tmp_x = (*obj)->x;
		tmp_y = (*obj)->y;
		tmp_frame_n = (*obj)->frame_n;

		(*obj)->x = old_x;
		(*obj)->y = old_y;

		if (tmp_frame_n < 8) //tail, work out new tail direction
			(*obj)->frame_n = new_tail_frame_n_tbl[tmp_frame_n][objFrameN - 8];
		else
			(*obj)->frame_n = objFrameN;

		old_x = tmp_x;
		old_y = tmp_y;
		objFrameN = tmp_frame_n;
	}

	return;
}


inline void U6Actor::set_direction_of_surrounding_objs(uint8 new_direction) {
	remove_surrounding_objs_from_map();

	switch (obj_n) {
	case OBJ_U6_SHIP :
		set_direction_of_surrounding_ship_objs(new_direction);
		break;

	case OBJ_U6_GIANT_SCORPION :
	case OBJ_U6_GIANT_ANT :
	case OBJ_U6_COW :
	case OBJ_U6_ALLIGATOR :
	case OBJ_U6_HORSE :
	case OBJ_U6_HORSE_WITH_RIDER :
		set_direction_of_surrounding_splitactor_objs(new_direction);
		break;

	case OBJ_U6_DRAGON :
		set_direction_of_surrounding_dragon_objs(new_direction);
		break;
	}

	add_surrounding_objs_to_map();

	return;
}

inline void U6Actor::set_direction_of_surrounding_ship_objs(uint8 new_direction) {
	Std::list<Obj *>::iterator obj;
	uint16 pitch = map->get_width(z);

	obj = surrounding_objects.begin();
	if (obj == surrounding_objects.end())
		return;

	(*obj)->x = x;
	(*obj)->y = y;

	(*obj)->frame_n =  new_direction * actor_type->tiles_per_direction + actor_type->tiles_per_frame - 1;
	switch (new_direction) {
	case NUVIE_DIR_N :
		if (y == 0)
			(*obj)->y = pitch - 1;
		else
			(*obj)->y = y - 1;
		break;

	case NUVIE_DIR_E :
		if (x == pitch - 1)
			(*obj)->x = 0;
		else
			(*obj)->x = x + 1;
		break;

	case NUVIE_DIR_S :
		if (y == pitch - 1)
			(*obj)->y = 0;
		else
			(*obj)->y = y + 1;
		break;

	case NUVIE_DIR_W :
		if (x == 0)
			(*obj)->x = pitch - 1;
		else
			(*obj)->x = x - 1;
		break;
	}

	obj++;
	if (obj == surrounding_objects.end())
		return;

	(*obj)->x = x;
	(*obj)->y = y;

	(*obj)->frame_n =  16 + (new_direction * actor_type->tiles_per_direction + actor_type->tiles_per_frame - 1);
	switch (new_direction) {
	case NUVIE_DIR_N :
		if (y == pitch - 1)
			(*obj)->y = 0;
		else
			(*obj)->y = y + 1;
		break;

	case NUVIE_DIR_E :
		if (x == 0)
			(*obj)->x = pitch - 1;
		else
			(*obj)->x = x - 1;
		break;

	case NUVIE_DIR_S :
		if (y == 0)
			(*obj)->y = pitch - 1;
		else
			(*obj)->y = y - 1;
		break;

	case NUVIE_DIR_W :
		if (x == pitch - 1)
			(*obj)->x = 0;
		else
			(*obj)->x = x + 1;
		break;
	}

}

inline void U6Actor::set_direction_of_surrounding_splitactor_objs(uint8 new_direction) {
	Obj *obj;
	uint16 pitch = map->get_width(z);

	if (surrounding_objects.empty())
		return;

	obj = surrounding_objects.back();

	if (obj->frame_n < 8)
		obj->frame_n = (get_reverse_direction(new_direction) * actor_type->tiles_per_direction + actor_type->tiles_per_frame - 1); //mutant actor
	else
		obj->frame_n =  8 + (new_direction * actor_type->tiles_per_direction + actor_type->tiles_per_frame - 1);

	obj->x = x;
	obj->y = y;

	switch (new_direction) {
	case NUVIE_DIR_N :
		if (y == pitch - 1)
			obj->y = 0;
		else
			obj->y = y + 1;
		break;

	case NUVIE_DIR_E :
		if (x == 0)
			obj->x = pitch - 1;
		else
			obj->x = x - 1;
		break;

	case NUVIE_DIR_S :
		if (y == 0)
			obj->y = pitch - 1;
		else
			obj->y = y - 1;
		break;

	case NUVIE_DIR_W :
		if (x == pitch - 1)
			obj->x = 0;
		else
			obj->x = x + 1;
		break;
	}

}

inline void U6Actor::set_direction_of_surrounding_dragon_objs(uint8 new_direction) {
	Std::list<Obj *>::iterator obj;
	uint8 frame_offset = (new_direction * actor_type->tiles_per_direction + actor_type->tiles_per_frame - 1);
	Obj *head, *tail, *wing1, *wing2;

//NOTE! this is dependent on the order the in which the objects are loaded in U6Actor::init_dragon()

	obj = surrounding_objects.begin();
	if (obj == surrounding_objects.end())
		return;
	head = *obj;
	head->frame_n =  8 + frame_offset;
	head->x = x;
	head->y = y;

	obj++;
	if (obj == surrounding_objects.end())
		return;
	tail = *obj;
	tail->frame_n =  16 + frame_offset;
	tail->x = x;
	tail->y = y;

	obj++;
	if (obj == surrounding_objects.end())
		return;
	wing1 = *obj;
	wing1->frame_n =  24 + frame_offset;
	wing1->x = x;
	wing1->y = y;

	obj++;
	if (obj == surrounding_objects.end())
		return;
	wing2 = *obj;
	wing2->frame_n =  32 + frame_offset;
	wing2->x = x;
	wing2->y = y;

	switch (new_direction) {
	case NUVIE_DIR_N :
		head->y = y - 1;
		tail->y = y + 1;
		wing1->x = x - 1;
		wing2->x = x + 1;
		break;

	case NUVIE_DIR_E :
		head->x = x + 1;
		tail->x = x - 1;
		wing1->y = y - 1;
		wing2->y = y + 1;
		break;

	case NUVIE_DIR_S :
		head->y = y + 1;
		tail->y = y - 1;
		wing1->x = x + 1;
		wing2->x = x - 1;
		break;

	case NUVIE_DIR_W :
		head->x = x - 1;
		tail->x = x + 1;
		wing1->y = y + 1;
		wing2->y = y - 1;
		break;
	}

}

inline void U6Actor::twitch_surrounding_objs() {
	Std::list<Obj *>::iterator obj;

	for (obj = surrounding_objects.begin(); obj != surrounding_objects.end(); obj++) {
		twitch_obj(*obj);
	}

}

inline void U6Actor::twitch_surrounding_dragon_objs() {
}

inline void U6Actor::twitch_surrounding_hydra_objs() {
	uint8 i;
	Std::list<Obj *>::iterator obj;

//Note! list order is important here. As it corresponds to the frame order in the tile set. This is defined in init_hydra()

	for (i = 0, obj = surrounding_objects.begin(); obj != surrounding_objects.end(); obj++, i += 4) {
		if (NUVIE_RAND() % 4 == 0)
			(*obj)->frame_n = i + (((*obj)->frame_n - i + 1) % 4);
	}
}

inline void U6Actor::twitch_obj(Obj *obj) {
	if (actor_type->frames_per_direction == 0) {
		DEBUG(0, LEVEL_WARNING, "FIXME: %s frames_per_direction == 0\n", get_name());
		obj->frame_n = (obj->frame_n / (1 * 4) * (1 * 4)) + direction * actor_type->tiles_per_direction +
		               walk_frame * actor_type->tiles_per_frame;
		return;
	}

	switch (obj->obj_n) {
	case OBJ_U6_GIANT_SCORPION :
	case OBJ_U6_GIANT_ANT :
	case OBJ_U6_COW :
	case OBJ_U6_ALLIGATOR :
	case OBJ_U6_HORSE :
		if (obj->frame_n < 8) { //mutant actor with two heads
			obj->frame_n = get_reverse_direction(direction) * actor_type->tiles_per_direction +
			               walk_frame * actor_type->tiles_per_frame;
			return;
		}
		break;
	default :
		break;
	}


	obj->frame_n = (obj->frame_n / (actor_type->frames_per_direction * 4) * (actor_type->frames_per_direction * 4)) + direction * actor_type->tiles_per_direction +
	               walk_frame * actor_type->tiles_per_frame;
}

inline void U6Actor::clear_surrounding_objs_list(bool delete_objs) {
	Std::list<Obj *>::iterator obj;

	if (surrounding_objects.empty())
		return;

	if (delete_objs == false) {
		surrounding_objects.clear();
		return;
	}

	obj = surrounding_objects.begin();

	for (; !surrounding_objects.empty();) {
		obj_manager->remove_obj_from_map(*obj);
		delete_obj(*obj);
		obj = surrounding_objects.erase(obj);
	}

	return;
}

inline void U6Actor::init_surrounding_obj(uint16 x_, uint16 y_, uint8 z_, uint16 actor_obj_n, uint16 objFrame_n) {
	Obj *obj;

	obj = obj_manager->get_obj_of_type_from_location(actor_obj_n, id_n, -1, x_, y_, z_);
	if (obj == NULL)
		obj = obj_manager->get_obj_of_type_from_location(actor_obj_n, 0, -1, x_, y_, z_);

	if (obj == NULL) {
		obj = new Obj();
		obj->x = x_;
		obj->y = y_;
		obj->z = z_;
		obj->obj_n = actor_obj_n;
		obj->frame_n = objFrame_n;
		obj_manager->add_obj(obj);
	}

	obj->quality = id_n;
	add_surrounding_obj(obj);

	return;
}


void U6Actor::die(bool create_body) {
	Game *game = Game::get_game();
	Party *party = game->get_party();
	Player *player = game->get_player();
	MapCoord actor_loc = get_location();

	if (party->get_member_num(this) == 0) //avatar
		return; //The avatar can't die. They just get teleported back to LB's castle.

	if (has_surrounding_objs())
		clear_surrounding_objs_list(true);


	set_dead_flag(true); // needed sooner for unready usecode of torches
	if (game->is_armageddon())
		inventory_drop_all();
	else if (base_actor_type->dead_obj_n != OBJ_U6_NOTHING) {
		if (create_body) {
			Obj *dead_body = new Obj;
			dead_body->obj_n = base_actor_type->dead_obj_n;
			if (base_actor_type->dead_frame_n == 255) // dog, cat, mouse, deer, wolf, drake, mongbat
				dead_body->frame_n = frame_n; // same frame the actor died
			else if (base_actor_type->dead_obj_n == OBJ_U6_BLOOD)
				dead_body->frame_n = NUVIE_RAND() % 3;
			else
				dead_body->frame_n = base_actor_type->dead_frame_n;
			dead_body->x = actor_loc.x;
			dead_body->y = actor_loc.y;
			dead_body->z = actor_loc.z;
			dead_body->quality = id_n;
			dead_body->status = OBJ_STATUS_OK_TO_TAKE;
			if (temp_actor)
				dead_body->status |= OBJ_STATUS_TEMPORARY;

			if (base_actor_type->dead_obj_n == OBJ_U6_BLOOD)
				inventory_drop_all();
			else // move my inventory into the dead body container
				all_items_to_container(dead_body, false);
			obj_manager->add_obj(dead_body, true);
		}
	} else if (create_body)
		inventory_drop_all();

	Actor::die();

	if (is_in_party()) {
		party->remove_actor(this, true);
		if (player->get_actor() == this)
			player->set_party_mode(party->get_actor(0)); //set party mode with the avatar as the leader.
	}

	if (party->get_member_num(this) != 0)
		move(0, 0, 0, ACTOR_FORCE_MOVE); // FIXME: move to another plane, same coords
}

// frozen by worktype or status
bool U6Actor::is_immobile() {
	return (((worktype == WORKTYPE_U6_MOTIONLESS
	          || worktype == WORKTYPE_U6_IMMOBILE) && !is_in_party())
	        || get_corpser_flag() == true
	        || is_sleeping() == true
	        || is_paralyzed() == true
	        /*|| can_move == false*/); // can_move really means can_twitch/animate
}

bool U6Actor::can_twitch() {
	return ((can_move == true || obj_n == OBJ_U6_MUSICIAN_PLAYING)
	        && visible_flag == true
	        && actor_type->twitch_rand != 0
	        && get_corpser_flag() == false
	        && is_sleeping() == false
	        && is_paralyzed() == false);
}

bool U6Actor::can_be_passed(Actor *other) {
	U6Actor *other_ = static_cast<U6Actor *>(other);
	return (Actor::can_be_passed(other_) && other_->current_movetype != current_movetype);
}

void U6Actor::print() {
	Actor::print();
// might print U6Actor members here
}

/* Returns name of NPC worktype/activity (game specific) or NULL. */
const char *U6Actor::get_worktype_string(uint32 wt) {
	const char *wt_string = NULL;
	if (wt == WORKTYPE_U6_MOTIONLESS) wt_string = "Motionless";
	else if (wt == WORKTYPE_U6_PLAYER) wt_string = "Player";
	else if (wt == WORKTYPE_U6_IN_PARTY) wt_string = "In Party";
	else if (wt == WORKTYPE_U6_ANIMAL_WANDER) wt_string = "Graze (animal wander)";
	else if (wt == WORKTYPE_U6_WALK_TO_LOCATION) wt_string = "Walk to Schedule";
	else if (wt == WORKTYPE_U6_FACE_NORTH) wt_string = "Stand (North)";
	else if (wt == WORKTYPE_U6_FACE_SOUTH) wt_string = "Stand (South)";
	else if (wt == WORKTYPE_U6_FACE_EAST) wt_string = "Stand (East)";
	else if (wt == WORKTYPE_U6_FACE_WEST) wt_string = "Stand (West)";
	else if (wt == WORKTYPE_U6_WALK_NORTH_SOUTH) wt_string = "Guard North/South";
	else if (wt == WORKTYPE_U6_WALK_EAST_WEST) wt_string = "Guard East/West";
	else if (wt == WORKTYPE_U6_WANDER_AROUND) wt_string = "Wander";
	else if (wt == WORKTYPE_U6_WORK) wt_string = "Loiter (work)";
	else if (wt == WORKTYPE_U6_SLEEP) wt_string = "Sleep";
	else if (wt == WORKTYPE_U6_PLAY_LUTE) wt_string = "Play";
	else if (wt == WORKTYPE_U6_BEG) wt_string = "Converse";
	else if (wt == WORKTYPE_U6_COMBAT_FRONT) wt_string = "Combat Front";
	else if (wt == 0x04) wt_string = "Combat Rear";
	else if (wt == 0x05) wt_string = "Combat Flank";
	else if (wt == 0x06) wt_string = "Combat Berserk";
	else if (wt == 0x07) wt_string = "Combat Retreat";
	else if (wt == 0x08) wt_string = "Combat Assault/Wild";
	else if (wt == 0x09) wt_string = "Shy";
	else if (wt == 0x0a) wt_string = "Like";
	else if (wt == 0x0b) wt_string = "Unfriendly";
	else if (wt == 0x0d) wt_string = "Tangle";
	else if (wt == 0x0e) wt_string = "Immobile";
	else if (wt == 0x92) wt_string = "Sit";
	else if (wt == 0x93) wt_string = "Eat";
	else if (wt == 0x94) wt_string = "Farm";
	else if (wt == 0x98) wt_string = "Ring Bell";
	else if (wt == 0x99) wt_string = "Brawl";
	else if (wt == 0x9a) wt_string = "Mousing";
	else if (wt == 0x9b) wt_string = "Attack Party";
	return (wt_string);
}

/* Return the first food or drink object in inventory. */
Obj *U6Actor::inventory_get_food(Obj *container) {
	U6UseCode *uc = (U6UseCode *)Game::get_game()->get_usecode();
	U6LList *inv = container ? container->container : get_inventory_list();
	U6Link *link = 0;
	Obj *obj = 0;
	for (link = inv->start(); link != NULL; link = link->next) {
		obj = (Obj *)link->data;
		if (uc->is_food(obj))
			return obj;
		if (obj->container) { // search within container
			if ((obj = inventory_get_food(obj)))
				return obj;
		}
	}
	return 0;
}

void U6Actor::inventory_make_all_objs_ok_to_take() {
	U6LList *inventory;
	U6Link *link;
	Obj *obj;

	inventory = get_inventory_list();

	if (!inventory)
		return;

	for (link = inventory->start(); link != NULL;) {
		obj = (Obj *)link->data;
		link = link->next;

		obj->set_ok_to_take(true, true);
	}

	return;
}
/* Set worktype to normal non-combat activity. */
void U6Actor::revert_worktype() {
	Party *party = Game::get_game()->get_party();
	if (is_in_party())
		set_worktype(WORKTYPE_U6_IN_PARTY);
	if (party->get_leader_actor() == this)
		set_worktype(WORKTYPE_U6_PLAYER);
}

/* Maximum magic points is derived from Intelligence and base_obj_n. */
uint8 U6Actor::get_maxmagic() {
	return Game::get_game()->get_script()->actor_get_max_magic_points(this);
}

bool U6Actor::will_not_talk() {
	if (worktype == WORKTYPE_U6_COMBAT_RETREAT || worktype == 0x12 // guard arrest player
	        || Game::get_game()->is_armageddon()
	        || worktype == WORKTYPE_U6_ATTACK_PARTY || worktype == 0x13) // repel undead and retreat
		return true;
	return false;
}

void U6Actor::handle_lightsource(uint8 hour) {
	Obj *torch = inventory_get_readied_object(ACTOR_ARM);
	if (torch && torch->obj_n != OBJ_U6_TORCH)
		torch = NULL;
	Obj *torch2 = inventory_get_readied_object(ACTOR_ARM_2);
	if (torch2 && torch2->obj_n != OBJ_U6_TORCH)
		torch2 = NULL;
	if (torch || torch2) {
		U6UseCode *useCode = (U6UseCode *)Game::get_game()->get_usecode();
		if ((hour < 6 || hour > 18 || (z != 0 && z != 5)
		        || Game::get_game()->get_weather()->is_eclipse())) {
			if (torch && torch->frame_n == 0) {
				if (torch->qty != 1)
					torch->qty = 1;
				useCode->torch(torch, USE_EVENT_USE);
			}
			if (torch2 && torch2->frame_n == 0) {
				if (torch2->qty != 1)
					torch2->qty = 1;
				useCode->torch(torch2, USE_EVENT_USE);
			}
		} else {
			if (torch && torch->frame_n == 1)
				useCode->torch(torch, USE_EVENT_USE);
			if (torch2 && torch2->frame_n == 1)
				useCode->torch(torch2, USE_EVENT_USE);
		}
	}
}

uint8 U6Actor::get_hp_text_color() {
	uint8 hp_text_color = 0x48; //standard text color)

	if (is_poisoned()) //actor is poisoned, display their hp in green
		hp_text_color = 0xa;
	else if (get_hp() < 10) //actor is critical, display their hp in red.
		hp_text_color = 0x0c;

	return hp_text_color;
}

} // End of namespace Nuvie
} // End of namespace Ultima
