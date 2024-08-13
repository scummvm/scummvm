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
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/files/nuvie_file_list.h"
#include "ultima/nuvie/save/obj_list.h"

#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/actors/u6_actor.h"
#include "ultima/nuvie/actors/se_actor.h"
#include "ultima/nuvie/actors/md_actor.h"
#include "ultima/nuvie/actors/u6_work_types.h"
#include "ultima/nuvie/core/tile_manager.h"
#include "ultima/nuvie/misc/u6_llist.h"
#include "ultima/nuvie/actors/actor_manager.h"
#include "ultima/nuvie/files/nuvie_io_file.h"
#include "ultima/nuvie/core/game_clock.h"
#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/core/party.h"
#include "ultima/nuvie/portraits/portrait.h"
#include "ultima/nuvie/script/script.h"
#include "ultima/nuvie/core/u6_objects.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/views/view_manager.h"

namespace Ultima {
namespace Nuvie {

static const int ACTOR_TEMP_INIT = 255;
static const int SCHEDULE_SIZE = 5;

ActorManager::ActorManager(const Configuration *cfg, Map *m, TileManager *tm, ObjManager *om, GameClock *c)
		: config(cfg), map(m), tile_manager(tm), obj_manager(om), _clock(c),
		  temp_actor_offset(224) {
	ARRAYCLEAR(actors);
	init();
}

ActorManager::~ActorManager() {
	clean();
}

void ActorManager::init() {
	player_actor = 1;

	last_obj_blk_x = cur_x = 0;
	last_obj_blk_y = cur_y = 0;
	last_obj_blk_z = cur_z = OBJ_TEMP_INIT;
	cmp_actor_loc = 0;

	update = true;
	wait_for_player = true;
	combat_movement = false;
	should_clean_temp_actors = true;

	return;
}

void ActorManager::clean() {
	uint16 i;

//delete all actors
	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		if (actors[i]) {
			delete actors[i];
			actors[i] = nullptr;
		}
	}

	init();

	return;
}

bool ActorManager::load(NuvieIO *objlist) {
	uint16 i;
	uint8 b1, b2, b3;
	int game_type;

	clean();

	config->value("config/GameType", game_type);

	objlist->seek(0x100); // Start of Actor position info
	if (game_type == NUVIE_GAME_U6)
		temp_actor_offset = 203;
	else
		temp_actor_offset = 224;

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		switch (game_type) {
		case NUVIE_GAME_U6 :
			actors[i] = new U6Actor(map, obj_manager, _clock);
			break;
		case NUVIE_GAME_MD :
			actors[i] = new MDActor(map, obj_manager, _clock);
			break;
		case NUVIE_GAME_SE :
			actors[i] = new SEActor(map, obj_manager, _clock);
			break;
		}

		b1 = objlist->read1();
		b2 = objlist->read1();
		b3 = objlist->read1();

		actors[i]->x = b1;
		actors[i]->x += (b2 & 0x3) << 8;

		actors[i]->y = (b2 & 0xfc) >> 2;
		actors[i]->y += (b3 & 0xf) << 6;

		actors[i]->z = (b3 & 0xf0) >> 4;
		actors[i]->id_n = (uint8)i;

		actors[i]->temp_actor = is_temp_actor(actors[i]->id_n);
	}

// objlist.seek(0x15f1);

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		b1 = objlist->read1();
		b2 = objlist->read1();
		actors[i]->obj_n = b1;
		actors[i]->obj_n += (b2 & 0x3) << 8;

		actors[i]->frame_n = (b2 & 0xfc) >> 2;
		actors[i]->direction = static_cast<NuvieDir>(actors[i]->frame_n / 4);
		if (actors[i]->obj_n == 0) { //Hack to get rid of Exodus.
			actors[i]->x = 0;
			actors[i]->y = 0;
			actors[i]->z = 0;
		}
	}

// Object flags.

	objlist->seek(0x000);

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		actors[i]->obj_flags = objlist->read1();
	}

// Actor status flags.

	objlist->seek(0x800);

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		actors[i]->status_flags = objlist->read1();
		actors[i]->alignment = static_cast<ActorAlignment>(((actors[i]->status_flags & ACTOR_STATUS_ALIGNMENT_MASK) >> 5) + 1);
	}

//old obj_n & frame_n values

	objlist->seek(game_type == NUVIE_GAME_U6 ? 0x15f1 : 0x16f1);

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		b1 = objlist->read1();
		b2 = objlist->read1();
		actors[i]->base_obj_n = b1;
		actors[i]->base_obj_n += (b2 & 0x3) << 8;

		actors[i]->old_frame_n = (b2 & 0xfc) >> 2;

		if (actors[i]->obj_n == 0) {
			//actors[i]->obj_n = actors[i]->base_obj_n;
			//actors[i]->frame_n = actors[i]->old_frame_n;
			actors[i]->hide();
		}

		if (actors[i]->base_obj_n == 0) {
			actors[i]->base_obj_n = actors[i]->obj_n;
			actors[i]->old_frame_n = actors[i]->frame_n;
		}
	}
// Strength

	objlist->seek(0x900);

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		actors[i]->strength = objlist->read1();
	}

// Dexterity

	objlist->seek(0xa00);

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		actors[i]->dex = objlist->read1();
	}

// Intelligence

	objlist->seek(0xb00);

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		actors[i]->intelligence = objlist->read1();
	}

	// Experience

	objlist->seek(0xc00);

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		actors[i]->exp = objlist->read2();
	}

// Health

	objlist->seek(0xe00);

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		actors[i]->hp = objlist->read1();
	}

// Experience Level

	objlist->seek(0xff1);

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		actors[i]->level = objlist->read1();
	}


// Combat mode

	objlist->seek(0x12f1);

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		switch (game_type) {
		case NUVIE_GAME_U6 :
			actors[i]->combat_mode = objlist->read1();
			break;

		case NUVIE_GAME_MD : // FIXME not sure what this is supposed to be
		case NUVIE_GAME_SE :
			actors[i]->magic = objlist->read1();
			break;
		}
	}

// Magic Points

	objlist->seek(0x13f1);

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		switch (game_type) {
		case NUVIE_GAME_U6 :
			actors[i]->magic = objlist->read1();
			break;

		case NUVIE_GAME_MD :
		case NUVIE_GAME_SE :
			actors[i]->combat_mode = objlist->read1();
			break;
		}
	}

	if (game_type == NUVIE_GAME_U6) {
		objlist->seek(OBJLIST_OFFSET_U6_TALK_FLAGS); // Start of Talk flags
	} else {
		objlist->seek(OBJLIST_OFFSET_MD_TALK_FLAGS); //MD talk flags location. FIXME: check SE
	}

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		actors[i]->talk_flags = objlist->read1();
	}

	objlist->seek(game_type == NUVIE_GAME_MD ? OBJLIST_OFFSET_MD_MOVEMENT_FLAGS : OBJLIST_OFFSET_U6_MOVEMENT_FLAGS);

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) { //movement flags.
		actors[i]->movement_flags = objlist->read1();
	}

	loadActorSchedules();

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		actors[i]->inventory_parse_readied_objects();

		actors[i]->init(); //let the actor object do some init
	}

// Moves

	objlist->seek(game_type == NUVIE_GAME_MD ? OBJLIST_OFFSET_MD_MOVEMENT_POINTS : OBJLIST_OFFSET_U6_MOVEMENT_POINTS);

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		actors[i]->moves = objlist->read1();
	}

// Current Worktype

	objlist->seek(0x11f1);

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		actors[i]->set_worktype(objlist->read1(), true);
	}

//cleanup party actor if not currently set as the player.

	if (actors[ACTOR_VEHICLE_ID_N]->get_worktype() != ACTOR_WT_PLAYER) {
		Actor *a = actors[ACTOR_VEHICLE_ID_N];
		a->set_obj_n(0);
		a->x = 0;
		a->y = 0;
		a->z = 0;
		//a->status_flags = ACTOR_STATUS_DEAD;
		//a->hide();
	}

	updateSchedules();
	loadCustomTiles(game_type);

	return true;
}

bool ActorManager::save(NuvieIO *objlist) {
	uint16 i;
	uint8 b;
	int game_type;

	config->value("config/GameType", game_type);

	objlist->seek(0x100); // Start of Actor position info

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		objlist->write1(actors[i]->x & 0xff);

		b = actors[i]->x >> 8;
		b += actors[i]->y << 2;
		objlist->write1(b);

		b = actors[i]->y >> 6;
		b += actors[i]->z << 4;
		objlist->write1(b);
	}

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		objlist->write1(actors[i]->obj_n & 0xff);
		b = actors[i]->obj_n >> 8;
		b += actors[i]->frame_n << 2;
		objlist->write1(b);
	}

//old obj_n & frame_n values

	objlist->seek(game_type == NUVIE_GAME_U6 ? 0x15f1 : 0x16f1);

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		objlist->write1(actors[i]->base_obj_n & 0xff);
		b = actors[i]->base_obj_n >> 8;
		b += actors[i]->old_frame_n << 2;
		objlist->write1(b);
	}

// Strength

	objlist->seek(0x900);

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		objlist->write1(actors[i]->strength);
	}

// Dexterity

	objlist->seek(0xa00);

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		objlist->write1(actors[i]->dex);
	}

// Intelligence

	objlist->seek(0xb00);

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		objlist->write1(actors[i]->intelligence);
	}

	// Experience

	objlist->seek(0xc00);

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		objlist->write2(actors[i]->exp);
	}

// Health

	objlist->seek(0xe00);

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		objlist->write1(actors[i]->hp);
	}

// Experience Level

	objlist->seek(0xff1);

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		objlist->write1(actors[i]->level);
	}


// Combat mode

	objlist->seek(0x12f1);

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		switch (game_type) {
		case NUVIE_GAME_U6 :
			objlist->write1(actors[i]->combat_mode);
			break;

		case NUVIE_GAME_MD : // FIXME not sure what this is supposed to be
		case NUVIE_GAME_SE :
			objlist->write1(actors[i]->magic);
			break;
		}
	}

// Magic Points

	objlist->seek(0x13f1);

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		switch (game_type) {
		case NUVIE_GAME_U6 :
			objlist->write1(actors[i]->magic);
			break;

		case NUVIE_GAME_MD :
		case NUVIE_GAME_SE :
			objlist->write1(actors[i]->combat_mode);
			break;
		}
	}

// Moves

	objlist->seek(0x14f1);

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		objlist->write1(actors[i]->moves);
	}

	objlist->seek(0); // Start of Obj flags

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		objlist->write1(actors[i]->obj_flags);
	}

	objlist->seek(0x800); // Start of Status flags

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		actors[i]->status_flags &= 0x9f;
		actors[i]->status_flags |= (actors[i]->alignment - 1) << 5;
		objlist->write1(actors[i]->status_flags);
	}

	if (game_type == NUVIE_GAME_U6) {
		objlist->seek(0x17f1); // Start of Talk flags
	} else {
		objlist->seek(0x18f1); //MD talk flags location. FIXME: check SE
	}

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		objlist->write1(actors[i]->talk_flags);
	}

	objlist->seek(0x19f1);

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) { //movement flags.
		objlist->write1(actors[i]->movement_flags);
	}
	/*
	 for(i=0;i < ACTORMANAGER_MAX_ACTORS; i++)
	   {
	    actors[i]->inventory_parse_readied_objects();

	    actors[i]->init(); //let the actor object do some init
	   }
	*/
// Current Worktype

	objlist->seek(0x11f1);

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		objlist->write1(actors[i]->get_worktype());
	}

	return true;
}

ActorList *ActorManager::get_actor_list() {
	ActorList *_actors = new ActorList(ACTORMANAGER_MAX_ACTORS);
	for (uint16 i = 0; i < ACTORMANAGER_MAX_ACTORS; i++)
		(*_actors)[i] = actors[i];
	return _actors;
}

Actor *ActorManager::get_actor(uint8 actor_num) const {
	return actors[actor_num];
}

Actor *ActorManager::get_actor(uint16 x, uint16 y, uint8 z, bool inc_surrounding_objs, Actor *excluded_actor) {
	// Note: Semantics have changed slightly since moving to findActorAt():
	// excluded_actor is now excluded when looking for multi-tile actors and surrounding objects
	return findActorAt(x, y, z, [=](const Actor *a) {return a != excluded_actor;}, inc_surrounding_objs, inc_surrounding_objs);
}

#if 0
// This was used as a helper method by get_actor() before it was changed to use findActorAt()
Actor *ActorManager::get_multi_tile_actor(uint16 x, uint16 y, uint8 z) {
	Actor *actor = get_actor(x + 1, y + 1, z, false); //search for 2x2 tile actor.
	if (actor) {
		Tile *tile = actor->get_tile();
		if (tile->dbl_width && tile->dbl_height)
			return actor;
	}

	actor = get_actor(x, y + 1, z, false); //search for 1x2 tile actor.
	if (actor) {
		Tile *tile = actor->get_tile();
		if (tile->dbl_height)
			return actor;
	}

	actor = get_actor(x + 1, y, z, false); //search for 1x2 tile actor.
	if (actor) {
		Tile *tile = actor->get_tile();
		if (tile->dbl_width)
			return actor;
	}


	return nullptr;
}
#endif

Actor *ActorManager::get_avatar() {
	return get_actor(ACTOR_AVATAR_ID_N);
}

Actor *ActorManager::get_player() {
	return actors[player_actor]; //FIX here for dead party leader etc.
}

void ActorManager::set_player(Actor *a) {
	player_actor = a->id_n;
}

/* Returns an actor's "look-string," a general description of their occupation
 * or appearance. (the tile description)
 */
const char *ActorManager::look_actor(const Actor *a, bool show_prefix) {
	uint16 tile_num = obj_manager->get_obj_tile_num(a->base_obj_n);
	if (tile_num == 0) {
		uint8 actor_num = a->id_n;
		if (actor_num == 191) // U6: Statue of Exodus
			return tile_manager->lookAtTile(obj_manager->get_obj_tile_num(399), 0, show_prefix);
		else if (actor_num == 189) // Statue of Mondain
			return tile_manager->lookAtTile(obj_manager->get_obj_tile_num(397), 0, show_prefix);
		else if (actor_num == 190) // Statue of Minax
			return tile_manager->lookAtTile(obj_manager->get_obj_tile_num(398), 0, show_prefix);
		else if (a->id_n >= 192 && a->id_n <= 200) // shrines
			return tile_manager->lookAtTile(obj_manager->get_obj_tile_num(393), 0, show_prefix);

		return tile_manager->lookAtTile(obj_manager->get_obj_tile_num(a->obj_n), 0, show_prefix);
	}
	return tile_manager->lookAtTile(tile_num, 0, show_prefix);
}

// Update area, and spawn or remove actors.
void ActorManager::updateActors(uint16 x, uint16 y, uint8 z) {
// uint8 cur_hour;
// uint16 i;

// if(!update)
//  return;
//DEBUG(0,LEVEL_DEBUGGING,"updateActors()\n");

	cur_x = x;
	cur_y = y;
	cur_z = z;

	uint16 cur_blk_x = x >> 3; // x / 8;
	uint16 cur_blk_y = y >> 3; // y / 8;

	update_temp_actors(x, y, z); // Remove out of range temp actors

	last_obj_blk_x = cur_blk_x; // moved from update_temp_actors() (SB-X)
	last_obj_blk_y = cur_blk_y;
	last_obj_blk_z = z;

	return;
}

// After player/party moves, continue moving actors.
void ActorManager::startActors() {
//DEBUG(0,LEVEL_DEBUGGING,"startActors()\n");

	wait_for_player = false;
	//ERIC Game::get_game()->pause_user();
}

void ActorManager::updateSchedules(bool teleport) {
	uint8 cur_hour = _clock->get_hour();

	for (int i = 0; i < ACTORMANAGER_MAX_ACTORS; i++)
		if (!actors[i]->is_in_party()) // don't do scheduled activities while partying
			actors[i]->updateSchedule(cur_hour, teleport);
}

void ActorManager::twitchActors() {
	uint16 i;

// while Actors are part of the world, their twitching is considered animation
	if (Game::get_game()->anims_paused())
		return;

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++)
		actors[i]->twitch();

}

// Update actors. StopActors() if no one can move.
void ActorManager::moveActors() {
	if (!update || wait_for_player) {
		return;// nothing to do
	}

	Game::get_game()->pause_user();
	Game::get_game()->get_script()->call_actor_update_all();
	Game::get_game()->get_map_window()->updateAmbience();
	Game::get_game()->get_view_manager()->update();
	//updateTime();
	Game::get_game()->unpause_user();
	wait_for_player = true;

	return;
}

bool ActorManager::loadActorSchedules() {
	Common::Path filename;
	NuvieIOFileRead schedule;
	uint16 i;
	uint16 total_schedules;
	uint16 num_schedules[ACTORMANAGER_MAX_ACTORS]; // an array to hold the number of schedule entries for each Actor.
	uint32 bytes_read;
	unsigned char *sched_data;
	uint16 *sched_offsets;
	unsigned char *s_ptr;

	config_get_path(config, "schedule", filename);
	if (schedule.open(filename) == false)
		return false;

	sched_offsets = (uint16 *)malloc(ACTORMANAGER_MAX_ACTORS * sizeof(uint16));

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++)
		sched_offsets[i] = schedule.read2();

	total_schedules = schedule.read2();

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		//if (sched_offsets[i] == 0)
		//    num_schedules[i] = 0;
		//else
		if (sched_offsets[i] > (total_schedules - 1))
			num_schedules[i] = 0;
		else
			// sched_offsets[i] is valid
		{
			if (i == ACTORMANAGER_MAX_ACTORS - 1)
				num_schedules[i] = total_schedules - sched_offsets[i];
			else if (sched_offsets[i + 1] > (total_schedules - 1))
				num_schedules[i] = total_schedules - sched_offsets[i];
			else
				// sched_offsets[i+1] is valid
				num_schedules[i] = sched_offsets[i + 1] - sched_offsets[i];
		}
	}

	sched_data = schedule.readBuf(total_schedules * SCHEDULE_SIZE, &bytes_read);

	if (!sched_data || bytes_read != (uint32)(total_schedules * SCHEDULE_SIZE)) {
		if (sched_data)
			free(sched_data);
		free(sched_offsets);
		DEBUG(0, LEVEL_ERROR, "Failed to read schedules!\n");
		return false;
	}

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		s_ptr = sched_data + (sched_offsets[i] * SCHEDULE_SIZE);
		actors[i]->loadSchedule(s_ptr, num_schedules[i]);
	}

	free(sched_data);
	free(sched_offsets);

	return true;
}

void ActorManager::clear_actor(Actor *actor) {
	if (is_temp_actor(actor))
		clean_temp_actor(actor);
	else
		actor->clear();

	return;
}

bool ActorManager::resurrect_actor(Obj *actor_obj, MapCoord new_position) {
	Actor *actor;

	if (!is_temp_actor(actor_obj->quality)) {
		actor = get_actor(actor_obj->quality);
		actor->resurrect(new_position, actor_obj);
	}

	return true;
}

bool ActorManager::is_temp_actor(Actor *actor) {
	if (actor)
		return is_temp_actor(actor->id_n);

	return false;
}

bool ActorManager::is_temp_actor(uint8 id_n) {
	if (id_n >= temp_actor_offset)
		return true;

	return false;
}

bool ActorManager::create_temp_actor(uint16 obj_n, uint8 obj_status, uint16 x, uint16 y, uint8 z, ActorAlignment alignment, uint8 worktype, Actor **new_actor) {
	Actor *actor;
	actor = find_free_temp_actor();


	if (actor) {
		actor->inventory_del_all_objs(); //We need to do this because the original game doesn't unset inventory flag when temp actors die.

		actor->base_obj_n = obj_n;
		actor->obj_n = obj_n;
		actor->frame_n = 0;

		actor->x = x;
		actor->y = y;
		actor->z = z;

		actor->temp_actor = true;

		actor->obj_flags = 0;
		actor->status_flags = 0;
		actor->talk_flags = 0;
		actor->movement_flags = 0;
		actor->alignment = ACTOR_ALIGNMENT_NEUTRAL;

		actor->init(obj_status);

		Game::get_game()->get_script()->call_actor_init(actor, alignment);

		// spawn double-tiled actors, like cows, facing west (SB-X)
		if (actor->get_tile_type() == ACTOR_DT)
			actor->set_direction(-1, 0);
		//if(worktype != 0)
		actor->set_worktype(worktype);
		actor->show();

		DEBUG(0, LEVEL_INFORMATIONAL, "Adding Temp Actor #%d: %s (%x,%x,%x).\n", actor->id_n, tile_manager->lookAtTile(obj_manager->get_obj_tile_num(actor->obj_n) + actor->frame_n, 0, false), actor->x, actor->y, actor->z);

		if (new_actor)
			*new_actor = actor;
		actor->handle_lightsource(_clock->get_hour());
		return true;
	} else
		DEBUG(0, LEVEL_NOTIFICATION, "***All Temp Actor Slots Full***\n");

	if (new_actor)
		*new_actor = nullptr;
	return false;
}

inline Actor *ActorManager::find_free_temp_actor() {
	uint16 i;

	for (i = temp_actor_offset; i < ACTORMANAGER_MAX_ACTORS; i++) {
		if (actors[i]->obj_n == 0)
			return actors[i];
	}

	return nullptr;
}

//FIX? should this be in Player??
// NO!
void ActorManager::update_temp_actors(uint16 x, uint16 y, uint8 z) {
	uint16 cur_blk_x, cur_blk_y;

// We're changing levels so clean out all temp actors on the current level.
	if (last_obj_blk_z != z) {
		if (last_obj_blk_z != ACTOR_TEMP_INIT) { //don't clean actors on startup.
			clean_temp_actors_from_level(last_obj_blk_z);
			return;
		}
//    last_obj_blk_z = z;
	}

	cur_blk_x = x >> 3; // x / 8;
	cur_blk_y = y >> 3; // y / 8;

	if (cur_blk_x != last_obj_blk_x || cur_blk_y != last_obj_blk_y) {
//    last_obj_blk_x = cur_blk_x;
//    last_obj_blk_y = cur_blk_y;

		clean_temp_actors_from_area(x, y);
	}

	return;
}

void ActorManager::clean_temp_actors_from_level(uint8 level) {
	uint16 i;

	for (i = temp_actor_offset; i < ACTORMANAGER_MAX_ACTORS; i++) {
		if ((actors[i]->is_visible() || actors[i]->x != 0 || actors[i]->y != 0 || actors[i]->z != 0)
		        && actors[i]->is_in_party() == false && actors[i]->z == level)
			clean_temp_actor(actors[i]);
	}

	return;
}

void ActorManager::clean_temp_actors_from_area(uint16 x, uint16 y) {
	uint16 i;
	uint16 dist_x, dist_y;

	if (!should_clean_temp_actors)
		return;

	for (i = temp_actor_offset; i < ACTORMANAGER_MAX_ACTORS; i++) {
		if ((actors[i]->is_visible() || actors[i]->x != 0 || actors[i]->y != 0 || actors[i]->z != 0)
		        && actors[i]->is_in_party() == false) {
			dist_x = abs((sint16)actors[i]->x - x);
			dist_y = abs((sint16)actors[i]->y - y);

			if (dist_x > 19 || dist_y > 19) {
				clean_temp_actor(actors[i]);
			}
		}
	}

	return;
}

inline void ActorManager::clean_temp_actor(Actor *actor) {
	DEBUG(0, LEVEL_INFORMATIONAL, "Removing Temp Actor #%d: %s (%x,%x,%x).\n", actor->id_n, tile_manager->lookAtTile(obj_manager->get_obj_tile_num(actor->obj_n) + actor->frame_n, 0, false), actor->x, actor->y, actor->z);
	actor->obj_n = 0;
	actor->clear();

	return;
}

bool ActorManager::clone_actor(Actor *actor, Actor **new_actor, MapCoord new_location) {
	if (actor == nullptr)
		return false;

	if (create_temp_actor(actor->obj_n, NO_OBJ_STATUS, new_location.x, new_location.y, new_location.z, actor->alignment, actor->worktype, new_actor) == false)
		return false;

	(*new_actor)->strength = actor->strength;
	(*new_actor)->dex = actor->dex;
	(*new_actor)->intelligence = actor->intelligence;

	(*new_actor)->magic = actor->magic;
	(*new_actor)->exp = actor->exp;
	(*new_actor)->hp = actor->hp;

	return true;
}

/* Move an actor to a random location within range.
 * Returns true when tossed.
 */
bool ActorManager::toss_actor(Actor *actor, uint16 xrange, uint16 yrange) {
	// maximum number of tries
	const uint32 toss_max = MAX(xrange, yrange) * MIN(xrange, yrange) * 2;
	uint32 t = 0;
	LineTestResult lt;
	if (xrange > 0) --xrange; // range includes the starting location
	if (yrange > 0) --yrange;
	while (t++ < toss_max) { // TRY RANDOM LOCATION
		sint16 x = (actor->x - xrange) + (NUVIE_RAND() % ((actor->x + xrange) - (actor->x - xrange) + 1)),
		       y = (actor->y - yrange) + (NUVIE_RAND() % ((actor->y + yrange) - (actor->y - yrange) + 1));
		if (!map->lineTest(actor->x, actor->y, x, y, actor->z, LT_HitUnpassable, lt))
			if (!get_actor(x, y, actor->z))
				return actor->move(x, y, actor->z);
	}
	// TRY ANY LOCATION
	for (int y = actor->y - yrange; y < actor->y + yrange; y++)
		for (int x = actor->x - xrange; x < actor->x + xrange; x++)
			if (!map->lineTest(actor->x, actor->y, x, y, actor->z, LT_HitUnpassable, lt))
				if (!get_actor(x, y, actor->z))
					return actor->move(x, y, actor->z);
	return false;
}

/* Find a location to put actor within range.
 * Returns true when tossed.
 */
bool ActorManager::toss_actor_get_location(uint16 start_x, uint16 start_y, uint8 start_z, uint16 xrange, uint16 yrange, MapCoord *location) {
	// maximum number of tries
	const uint32 toss_max = MAX(xrange, yrange) * MIN(xrange, yrange) * 2;
	uint32 t = 0;
	LineTestResult lt;
	if (xrange > 0) --xrange; // range includes the starting location
	if (yrange > 0) --yrange;
	while (t++ < toss_max) { // TRY RANDOM LOCATION
		sint16 x = (start_x - xrange) + (NUVIE_RAND() % ((start_x + xrange) - (start_x - xrange) + 1)),
		       y = (start_y - yrange) + (NUVIE_RAND() % ((start_y + yrange) - (start_y - yrange) + 1));
		if (!map->lineTest(start_x, start_y, x, y, start_z, LT_HitUnpassable, lt)) {
			if (!get_actor(x, y, start_z)) {
				location->x = x;
				location->y = y;
				location->z = start_z;
				return can_put_actor(*location);
			}

		}
	}
	// TRY ANY LOCATION
	for (int y = start_y - yrange; y < start_y + yrange; y++)
		for (int x = start_x - xrange; x < start_x + xrange; x++)
			if (!map->lineTest(start_x, start_y, x, y, start_z, LT_HitUnpassable, lt)) {
				if (!get_actor(x, y, start_z)) {
					location->x = x;
					location->y = y;
					location->z = start_z;
					return can_put_actor(*location);
				}
			}

	return false;
}


/* Returns the actor whose inventory contains an object. */
Actor *ActorManager::get_actor_holding_obj(Obj *obj) {
	assert(obj->is_in_inventory());

	while (obj->is_in_container())
		obj = obj->get_container_obj();
	return (Actor *)obj->parent;
}

// Remove list actors who fall out of a certain range from a location.
ActorList *ActorManager::filter_distance(ActorList *list, uint16 x, uint16 y, uint8 z, uint16 dist) {
	ActorIterator i = list->begin();
	while (i != list->end()) {
		Actor *actor = *i;
		MapCoord loc(x, y, z);
		MapCoord actor_loc(actor->x, actor->y, actor->z);
		if (loc.distance(actor_loc) > dist || loc.z != actor_loc.z)
			i = list->erase(i);
		else ++i;
	}
	return list;
}

// Remove actors who don't need to move in this turn. That includes anyone not
// in a certain range of xyz, and actors that are already out of moves.
inline ActorList *ActorManager::filter_active_actors(ActorList *list, uint16 x, uint16 y, uint8 z) {
	const uint8 dist = 24;
	ActorIterator i = list->begin();
	while (i != list->end()) {
		Actor *actor = *i;
		MapCoord loc(x, y, z);
		MapCoord actor_loc(actor->x, actor->y, actor->z);
		if (!actor->is_in_party()) {
			if ((loc.distance(actor_loc) > dist || loc.z != actor_loc.z)
			        && actor->worktype != WORKTYPE_U6_WALK_TO_LOCATION)
				actor->set_moves_left(0);
			if (actor->is_sleeping() || actor->is_immobile() || !actor->is_alive())
				actor->set_moves_left(0);
		}
		if ((actor->is_in_party() == true && combat_movement == false) || actor->moves <= 0)
			i = list->erase(i);
		else ++i;
	}
	return list;
}

// Sort list by distance to a location. Remove actors on different planes.
ActorList *ActorManager::sort_nearest(ActorList *list, uint16 x, uint16 y, uint8 z) {
	struct Actor::cmp_distance_to_loc cmp_func; // comparison function object
	MapCoord loc(x, y, z);
	cmp_func(loc); // set location in function object
	Common::sort(list->begin(), list->end(), cmp_func);

	ActorIterator a = list->begin();
	while (a != list->end())
		if ((*a)->z != z)
			a = list->erase(a); // only return actors on the same map
		else ++a;
	return list;
}

void ActorManager::print_actor(Actor *actor) {
	actor->print();
}

bool ActorManager::can_put_actor(const MapCoord &location) {
	if (!map->is_passable(location.x, location.y, location.z))
		return false;

	if (get_actor(location.x, location.y, location.z) != nullptr)
		return false;

	return true;
}

// Remove actors with a certain alignment from the list. Returns the same list.
ActorList *ActorManager::filter_alignment(ActorList *list, ActorAlignment align) {
	ActorIterator i = list->begin();
	while (i != list->end()) {
		Actor *actor = *i;
		if (actor->alignment == align)
			i = list->erase(i);
		else ++i;
	}
	return list;
}

// Remove actors in the party. Returns the original list pointer.
ActorList *ActorManager::filter_party(ActorList *list) {
	ActorIterator i = list->begin();
	while (i != list->end()) {
		Actor *actor = *i;
		if (actor->is_in_party() == true || actor->id_n == 0) // also remove vehicle
			i = list->erase(i);
		else ++i;
	}
	return list;
}

void ActorManager::set_combat_movement(bool c) {
	combat_movement = c;
}

bool ActorManager::loadCustomTiles(nuvie_game_t game_type) {
	Common::Path datadir = "images";
	Common::Path path;

	build_path(datadir, "tiles", path);
	datadir = path;
	build_path(datadir, get_game_tag(game_type), path);
	datadir = path;

	tile_manager->freeCustomTiles(); //FIXME this might need to change if we start using custom tiles outside of ActorManager. eg custom map/object tilesets

	loadCustomBaseTiles();
	if (obj_manager->use_custom_actor_tiles()) {
		loadAvatarTiles(datadir);
		loadNPCTiles(datadir);
	}

	return true;
}

void ActorManager::loadCustomBaseTiles() {
	Common::Path datadir = "mods";
	Common::Path imagefile;
	build_path(datadir, "custom_tiles.bmp", imagefile);

	//attempt to load custom base tiles if the file exists.
	tile_manager->loadCustomTiles(imagefile, true, true, 0);
}

void ActorManager::loadAvatarTiles(const Common::Path &datadir) {
	Common::Path imagefile;

	uint8 avatar_portrait = Game::get_game()->get_portrait()->get_avatar_portrait_num();

	Std::set<Std::string> files = getCustomTileFilenames(datadir, "avatar_###_####.bmp");

	for (const Std::string &filename : files) {
		if (filename.length() != 19) { // avatar_nnn_nnnn.bmp
			continue;
		}
		Std::string num_str = filename.substr(7, 3);
		uint8 portrait_num = (uint8)strtol(num_str.c_str(), nullptr, 10);

		if (portrait_num == avatar_portrait) {
			num_str = filename.substr(11, 4);
			uint16 obj_n = (uint16)strtol(num_str.c_str(), nullptr, 10);

			Common::Path path;
			build_path(datadir, filename, path);
			imagefile = Game::get_game()->get_data_file_path(path);
			Tile *start_tile = tile_manager->loadCustomTiles(imagefile, false, true, actors[1]->get_tile_num());
			if (start_tile) {
				actors[1]->set_custom_tile_num(obj_n, start_tile->tile_num);
			}
		}
	}
	return;
}

void ActorManager::loadNPCTiles(const Common::Path &datadir) {
	Common::Path imagefile;

	Std::set<Std::string> files = getCustomTileFilenames(datadir, "actor_###_####.bmp");

	for (const Std::string &filename : files) {
		if (filename.length() != 18) { // actor_nnn_nnnn.bmp
			continue;
		}
		Std::string num_str = filename.substr(6, 3);
		uint8 actor_num = (uint8)strtol(num_str.c_str(), nullptr, 10);

		num_str = filename.substr(10, 4);
		uint16 obj_n = (uint16)strtol(num_str.c_str(), nullptr, 10);

		Common::Path path;
		build_path(datadir, filename, path);
		imagefile = Game::get_game()->get_data_file_path(path);
		Tile *start_tile = tile_manager->loadCustomTiles(imagefile, false, true, actors[actor_num]->get_tile_num());
		if (start_tile) {
			actors[actor_num]->set_custom_tile_num(obj_n, start_tile->tile_num);
		}
	}
	return;
}

Std::set<Std::string> ActorManager::getCustomTileFilenames(const Common::Path &datadir, const Std::string &filenamePrefix) {
	NuvieFileList filelistDataDir;
	NuvieFileList filelistSaveGameDir;
	Common::Path path;

	path = GUI::get_gui()->get_data_dir().joinInPlace(datadir);
	filelistDataDir.open(path, filenamePrefix.c_str(), NUVIE_SORT_NAME_ASC);

	path = "data";
	path.joinInPlace(datadir);
	filelistSaveGameDir.open(path, filenamePrefix.c_str(), NUVIE_SORT_NAME_ASC);

	Std::set<Std::string> files = filelistSaveGameDir.get_filenames();
	Std::set<Std::string> dataFiles = filelistDataDir.get_filenames();
	files.insert(dataFiles.begin(), dataFiles.end());
	return files;
}

Actor *ActorManager::findActorAtImpl(uint16 x, uint16 y, uint8 z, bool (*predicateWrapper)(void *predicate, const Actor *), bool incDoubleTile, bool incSurroundingObjs, void *predicate) const {

	for (uint16 i = 0; i < ACTORMANAGER_MAX_ACTORS; ++i)
		// Exclude surrounding objects here since we can get them directly via the AVL tree instead of going through earch actor's surrounding objects list
		if (actors[i] && actors[i]->doesOccupyLocation(x, y, z, incDoubleTile, false) && predicateWrapper(predicate, actors[i]))
			return actors[i];

	if (incSurroundingObjs) {
		// Look for actor objects (e.g. Silver Serpent body, Hydra parts, etc.)
		const U6LList *const obj_list = obj_manager->get_obj_list(x, y, z);

		if (obj_list) {
			for (const U6Link *link = obj_list->start(); link != nullptr; link = link->next) {
				const Obj *obj = (Obj *)link->data;

				if (obj->is_actor_obj()) {
					const uint8 actorNum = obj->obj_n == OBJ_U6_SILVER_SERPENT
							&& Game::get_game()->get_game_type() == NUVIE_GAME_U6 ? obj->qty : obj->quality;
					Actor *actor = get_actor(actorNum);
					if (actor && predicateWrapper(predicate, actor))
						return actor;
				}
			}
		}
	}

	// No match
	return nullptr;
}

} // End of namespace Nuvie
} // End of namespace Ultima
