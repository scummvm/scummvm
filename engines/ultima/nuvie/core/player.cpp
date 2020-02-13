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
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/misc/u6_misc.h"
#include "ultima/nuvie/files/nuvie_io.h"
#include "ultima/nuvie/actors/actor_manager.h"
#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/core/obj_manager.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/core/game_clock.h"
#include "ultima/nuvie/gui/widgets/msg_scroll.h"
#include "ultima/nuvie/core/party.h"
#include "ultima/nuvie/save/obj_list.h"
#include "ultima/nuvie/core/u6_objects.h"
#include "ultima/nuvie/sound/sound_manager.h"
#include "ultima/nuvie/core/weather.h"
#include "ultima/nuvie/script/script.h"
#include "ultima/nuvie/core/player.h"

namespace Ultima {
namespace Nuvie {

#define PLAYER_BASE_MOVEMENT_COST 5

Player::Player(Configuration *cfg) {
	config = cfg;
	config->value("config/GameType", game_type);

	clock = NULL;
	party = NULL;
	actor = NULL;
	actor_manager = NULL;
	obj_manager = NULL;
	map_window = NULL;
	karma = 0;
	gender = 0;
	questf = 0;
	gargishf = 0;
	alcohol = 0;
	current_weapon = 0;
	party_mode = false;
	mapwindow_centered = false;
}

bool Player::init(ObjManager *om, ActorManager *am, MapWindow *mw, GameClock *c, Party *p) {

	clock = c;
	actor_manager = am;
	obj_manager = om;
	map_window = mw;
	party = p;

	current_weapon = -1;

	init();

	return true;
}

void Player::init() {
	actor = NULL;

	party_mode = true;
	mapwindow_centered = true;
}

bool Player::load(NuvieIO *objlist) {
	uint8 solo_member_num = OBJLIST_PARTY_MODE;

	init();

// We can get the name from the player actor. --SB-X
	/* objlist->seek(0xf00);

	 objlist->readToBuf((unsigned char *)name,14); // read in Player name.*/

	if (game_type == NUVIE_GAME_U6) {
		objlist->seek(OBJLIST_OFFSET_U6_QUEST_FLAG); // U6 Quest Flag
		questf = objlist->read1();

		objlist->seek(OBJLIST_OFFSET_U6_KARMA); // Player Karma.
		karma = objlist->read1();

		objlist->seek(OBJLIST_OFFSET_U6_ALCOHOL); // Alcohol consumed
		alcohol = objlist->read1();

		objlist->seek(OBJLIST_OFFSET_U6_GARGISH_LANG); // U6 Gargish Flag
		gargishf = objlist->read1();

		objlist->seek(OBJLIST_OFFSET_U6_SOLO_MODE); //Party Mode = 0xff other wise it is solo mode party member number starting from 0.
		solo_member_num = objlist->read1();

		objlist->seek(OBJLIST_OFFSET_U6_GENDER); // Player Gender.
		gender = objlist->read1();
	}

	if (game_type == NUVIE_GAME_MD) {
		objlist->seek(OBJLIST_OFFSET_MD_GENDER); // Player Gender.
		gender = objlist->read1();
	}

	if (solo_member_num == OBJLIST_PARTY_MODE) {
		party_mode = true;
		set_party_mode(find_actor());
	} else
		set_solo_mode(party->get_actor(solo_member_num));

	return true;
}

bool Player::save(NuvieIO *objlist) {
	if (game_type == NUVIE_GAME_U6) {
		objlist->seek(OBJLIST_OFFSET_U6_QUEST_FLAG); // U6 Quest Flag
		objlist->write1(questf);

		objlist->seek(OBJLIST_OFFSET_U6_KARMA); // Player Karma.
		objlist->write1(karma);

		objlist->seek(OBJLIST_OFFSET_U6_ALCOHOL); // Alcohol consumed
		objlist->write1(alcohol);

		objlist->seek(OBJLIST_OFFSET_U6_GARGISH_LANG); // U6 Gargish Flag
		objlist->write1(gargishf);

		objlist->seek(OBJLIST_OFFSET_U6_SOLO_MODE); // solo member num.
		if (party_mode)
			objlist->write1(OBJLIST_PARTY_MODE); // 0xff
		else
			objlist->write1(party->get_member_num(actor)); //write the party member number of the solo actor

		objlist->seek(OBJLIST_OFFSET_U6_GENDER); // Player Gender.
		objlist->write1(gender);
	}

	if (game_type == NUVIE_GAME_MD) {
		objlist->seek(OBJLIST_OFFSET_MD_GENDER); // Player Gender.
		objlist->write1(gender);
	}

	return true;
}

Actor *Player::find_actor() {

	for (uint32 p = 0; p < ACTORMANAGER_MAX_ACTORS; p++) {
		Actor *theActor = actor_manager->get_actor(p);
		if (theActor->get_worktype() == 0x02 && theActor->is_immobile() == false) // WT_U6_PLAYER
			return (theActor);
	}

	sint8 party_leader = party->get_leader();

	if (party_leader != -1)
		return party->get_actor(party_leader);

	return (actor_manager->get_avatar());
}


// keep MapWindow focused on Player actor, or remove focus
void Player::set_mapwindow_centered(bool state) {
	uint16 x, y;
	uint8 z;

	mapwindow_centered = state;
	if (mapwindow_centered == false)
		return;
	map_window->centerMapOnActor(actor); // center immediately

	get_location(&x, &y, &z);
	actor_manager->updateActors(x, y, z);
	obj_manager->update(x, y, z); // spawn eggs when teleporting. eg red moongate.
}

void Player::set_actor(Actor *new_actor) {
	MsgScroll *scroll = Game::get_game()->get_scroll();
	if (new_actor == NULL) {
		return;
	}

	if (actor != NULL) {
		if (party->contains_actor(actor))
			actor->set_worktype(0x01); //WT_U6_IN_PARTY
		else
			actor->set_worktype(0x00); //no worktype
	}
	bool same_actor = (actor == new_actor);
	actor = new_actor;

	actor->set_worktype(0x02); // WT_U6_PLAYER
	actor->delete_pathfinder();

	current_weapon = ACTOR_NO_READIABLE_LOCATION;
	map_window->centerCursor();
	if (same_actor)
		return;
	actor_manager->set_player(actor);
	Std::string prompt = get_name();

	if (game_type == NUVIE_GAME_U6) {
		prompt += ":\n";
	}

	prompt += ">";

	scroll->set_prompt(prompt.c_str());
}

Actor *Player::get_actor() {
	return actor;
}

void Player::get_location(uint16 *ret_x, uint16 *ret_y, uint8 *ret_level) {
	actor->get_location(ret_x, ret_y, ret_level);
}

uint8 Player::get_location_level() {
	return actor->z;
}

const char *Player::get_name() {
	if (actor->get_actor_num() == ACTOR_VEHICLE_ID_N)
		return actor_manager->get_avatar()->get_name();

	return actor->get_name(true);
}


/* Add to Player karma. Handle appropriately the karma min/max limits. */
void Player::add_karma(uint8 val) {
	karma = ((karma + val) <= 99) ? karma + val : 99;
}

/* Subtract from Player karma. Handle appropriately the karma min/max limits. */
void Player::subtract_karma(uint8 val) {
	karma = ((karma - val) >= 0) ? karma - val : 0;
}

void Player::subtract_movement_points(uint8 points) {
	Game::get_game()->get_script()->call_actor_subtract_movement_points(get_actor(), points);
}

const char *Player::get_gender_title() {
	switch (game_type) {
	case NUVIE_GAME_U6 :
		if (gender == 0)
			return "milord";
		else
			return "milady";
	case NUVIE_GAME_MD :
		if (gender == 0)
			return "Sir";
		else
			return "Madam";
	default :
		break;
	}

	return "Sir"; //FIXME is this needed for SE?
}

bool Player::check_moveRelative(sint16 rel_x, sint16 rel_y) {
	if (!actor->moveRelative(rel_x, rel_y, ACTOR_IGNORE_DANGER)) { /**MOVE**/
		ActorError *ret = actor->get_error();
		if (ret->err == ACTOR_BLOCKED_BY_ACTOR
		        && party->contains_actor(ret->blocking_actor) && ret->blocking_actor->is_immobile() == false)
			ret->blocking_actor->push(actor, ACTOR_PUSH_HERE);
		if (!actor->moveRelative(rel_x, rel_y, ACTOR_IGNORE_DANGER)) /**MOVE**/
			return false;
	}
	return true;
}

// walk to adjacent square
void Player::moveRelative(sint16 rel_x, sint16 rel_y, bool mouse_movement) {
	const uint8 raft_movement_tbl[] = {
		NUVIE_DIR_N, NUVIE_DIR_NE, NUVIE_DIR_N, NUVIE_DIR_NW, NUVIE_DIR_N, NUVIE_DIR_NE, NUVIE_DIR_NW, NUVIE_DIR_N,
		NUVIE_DIR_NE, NUVIE_DIR_NE, NUVIE_DIR_E, NUVIE_DIR_N, NUVIE_DIR_NE, NUVIE_DIR_E, NUVIE_DIR_NE, NUVIE_DIR_N,
		NUVIE_DIR_NE, NUVIE_DIR_E, NUVIE_DIR_SE, NUVIE_DIR_E, NUVIE_DIR_E, NUVIE_DIR_E, NUVIE_DIR_SE, NUVIE_DIR_NE,
		NUVIE_DIR_E, NUVIE_DIR_SE, NUVIE_DIR_SE, NUVIE_DIR_S, NUVIE_DIR_E, NUVIE_DIR_SE, NUVIE_DIR_S, NUVIE_DIR_SE,
		NUVIE_DIR_S, NUVIE_DIR_SE, NUVIE_DIR_S, NUVIE_DIR_SW, NUVIE_DIR_SE, NUVIE_DIR_S, NUVIE_DIR_S, NUVIE_DIR_SW,
		NUVIE_DIR_W, NUVIE_DIR_S, NUVIE_DIR_SW, NUVIE_DIR_SW, NUVIE_DIR_SW, NUVIE_DIR_S, NUVIE_DIR_SW, NUVIE_DIR_W,
		NUVIE_DIR_NW, NUVIE_DIR_W, NUVIE_DIR_SW, NUVIE_DIR_W, NUVIE_DIR_NW, NUVIE_DIR_SW, NUVIE_DIR_W, NUVIE_DIR_W,
		NUVIE_DIR_NW, NUVIE_DIR_N, NUVIE_DIR_W, NUVIE_DIR_NW, NUVIE_DIR_N, NUVIE_DIR_NW, NUVIE_DIR_W, NUVIE_DIR_NW
	};
	const uint8 ship_cost[8] = {0xA, 5, 3, 4, 5, 4, 3, 5};
	const uint8 skiff_cost[8] = {3, 4, 5, 7, 0xA, 7, 5, 4};

	MovementStatus movementStatus = CAN_MOVE;
	bool can_change_rel_dir = true;
	uint8 wind_dir = Game::get_game()->get_weather()->get_wind_dir();
	uint16 x, y;
	uint8 z;
	actor->get_location(&x, &y, &z);

	if (game_type == NUVIE_GAME_U6) {
		if (actor->id_n == 0) { // vehicle actor
			if (actor->obj_n == OBJ_U6_INFLATED_BALLOON &&
			        (!Game::get_game()->has_free_balloon_movement() || !party->has_obj(OBJ_U6_FAN, 0, false))) {
				can_change_rel_dir = false;
				uint8 dir = get_reverse_direction(Game::get_game()->get_weather()->get_wind_dir());
				if (dir == NUVIE_DIR_NONE) {
					Game::get_game()->get_scroll()->display_string("Thou canst not move without wind!\n\n");
					Game::get_game()->get_scroll()->display_prompt();
					actor->set_moves_left(0);
					rel_x = 0;
					rel_y = 0;
				} else {
					get_relative_dir(dir, &rel_x, &rel_y);
				}
			} else if (actor->obj_n == OBJ_U6_RAFT) {
				uint8 dir = 0;
				can_change_rel_dir = false;
				Tile *t = Game::get_game()->get_game_map()->get_tile(x, y, z, true);
				if (t->flags1 & TILEFLAG_BLOCKING) { //deep water tiles are blocking. Shore tiles should allow player movement.
					//deep water, so take control away from player.
					if (t->tile_num >= 8 && t->tile_num < 16) {
						dir = t->tile_num - 8;
					}
					if (wind_dir != NUVIE_DIR_NONE) {
						dir = raft_movement_tbl[dir * 8 + get_reverse_direction(wind_dir)];
					} else {
						dir = get_nuvie_dir_code(dir);
					}

					get_relative_dir(dir, &rel_x, &rel_y);
				}
			}
		} else { // normal actor
			if (alcohol > 3 && NUVIE_RAND() % 4 != 0) {
				rel_x = NUVIE_RAND() % 3 - 1; // stumble and change direction
				rel_y = NUVIE_RAND() % 3 - 1;
				can_change_rel_dir = false;
				Game::get_game()->get_scroll()->display_string("Hic!\n");
			}
		}

		ActorMoveFlags move_flags = ACTOR_IGNORE_DANGER | ACTOR_IGNORE_PARTY_MEMBERS;
		// don't allow diagonal move between blocked tiles (player only)
		if (rel_x && rel_y && !actor->check_move(x + rel_x, y + 0, z, move_flags)
		        && !actor->check_move(x + 0, y + rel_y, z, move_flags)) {
			movementStatus = BLOCKED;
		}
	} else if (game_type == NUVIE_GAME_MD) {
		if (Game::get_game()->get_clock()->get_timer(GAMECLOCK_TIMER_MD_BLUE_BERRY) != 0 && NUVIE_RAND() % 2 == 0) {
			rel_x = NUVIE_RAND() % 3 - 1; // stumble and change direction
			rel_y = NUVIE_RAND() % 3 - 1;
			can_change_rel_dir = false;
			Game::get_game()->get_scroll()->display_string("you are dizzy!\n"); //FIXME need i18n support here.
		}
	}

	if (actor->is_immobile() && actor->id_n != 0)
		movementStatus = BLOCKED;

	if (movementStatus != BLOCKED && game_type != NUVIE_GAME_U6) {
		movementStatus = Game::get_game()->get_script()->call_player_before_move_action(&rel_x, &rel_y);
	}

	if (movementStatus != BLOCKED) {
		if (movementStatus == FORCE_MOVE) {
			actor->moveRelative(rel_x, rel_y, ACTOR_FORCE_MOVE);
		} else if (!check_moveRelative(rel_x, rel_y)) {
			movementStatus = BLOCKED;
			if (mouse_movement && rel_x != 0 && rel_y != 0 && can_change_rel_dir) {
				if (check_moveRelative(rel_x, 0)) { // try x movement only
					rel_y = 0;
					movementStatus = CAN_MOVE;
				} else if (check_moveRelative(0, rel_y)) { // try y movement only
					rel_x = 0;
					movementStatus = CAN_MOVE;
				}
			}
			// Try opening a door FIXME: shouldn't be U6 specific
			if (movementStatus == BLOCKED) {
				if (obj_manager->is_door(x + rel_x, y + rel_y, z))
					try_open_door(x + rel_x, y + rel_y, z);
			}
			if (movementStatus == BLOCKED) {
				Game::get_game()->get_sound_manager()->playSfx(NUVIE_SFX_BLOCKED);
				if (actor->id_n == 0) //vehicle actor.
					actor->set_moves_left(0); //zero movement points here so U6 can change wind direction by advancing game time.
			}
		}
	}
	actor->set_direction(rel_x, rel_y);

	// post-move
	if (movementStatus != BLOCKED) {
		if (party_mode && party->is_leader(actor)) { // lead party
			party->follow(rel_x, rel_y);
		} else if (actor->id_n == 0 && game_type != NUVIE_GAME_MD) { // using vehicle; drag party along
			MapCoord new_xyz = actor->get_location();
			party->move(new_xyz.x, new_xyz.y, new_xyz.z);
		}

		if (game_type == NUVIE_GAME_U6 && (actor->obj_n == OBJ_U6_INFLATED_BALLOON || actor->obj_n == OBJ_U6_RAFT)) {
			actor->set_moves_left(actor->get_moves_left() - PLAYER_BASE_MOVEMENT_COST);
		} else if (game_type == NUVIE_GAME_U6 && actor->obj_n == OBJ_U6_SHIP && wind_dir != WEATHER_WIND_CALM) {
			uint8 nuvie_dir = get_direction_code(rel_x, rel_y);
			if (nuvie_dir != NUVIE_DIR_NONE) {
				sint8 dir = get_original_dir_code(nuvie_dir);

				actor->set_moves_left(actor->get_moves_left() - ship_cost[abs(dir - wind_dir)]);
				//DEBUG(0, LEVEL_DEBUGGING, "Ship movement cost = %d\n", ship_cost[abs(dir-wind_dir)]);
			}
		} else if (game_type == NUVIE_GAME_U6 && actor->obj_n == OBJ_U6_SKIFF) {
			uint8 nuvie_dir = get_direction_code(rel_x, rel_y);
			if (nuvie_dir != NUVIE_DIR_NONE) {
				sint8 dir = get_original_dir_code(nuvie_dir);
				sint8 water_dir = dir;
				Tile *t = Game::get_game()->get_game_map()->get_tile(x, y, z, true);
				if (t->tile_num >= 8 && t->tile_num < 16) {
					dir = t->tile_num - 8;
				}
				actor->set_moves_left(actor->get_moves_left() - skiff_cost[abs(dir - water_dir)]);
				//DEBUG(0, LEVEL_DEBUGGING, "Skiff movement cost = %d\n", skiff_cost[abs(dir-water_dir)]);
			}
		} else {
			actor->set_moves_left(actor->get_moves_left() - (PLAYER_BASE_MOVEMENT_COST + Game::get_game()->get_game_map()->get_impedance(x, y, z)));
			if (rel_x != 0 && rel_y != 0) // diagonal move, double cost
				actor->set_moves_left(actor->get_moves_left() - PLAYER_BASE_MOVEMENT_COST);
		}
	}

	if (game_type != NUVIE_GAME_U6) {
		Game::get_game()->get_script()->call_player_post_move_action(movementStatus != BLOCKED);
		actor->get_location(&x, &y, &z); //update location in case we have moved.
	}

	// update world around player
	actor_manager->updateActors(x, y, z);
	obj_manager->update(x, y, z); // remove temporary objs, hatch eggs
	clock->inc_move_counter(); // doesn't update time
	actor_manager->startActors(); // end player turn
}

void Player::try_open_door(uint16 x, uint16 y, uint8 z) {
	UseCode *usecode = Game::get_game()->get_usecode();
	Obj *obj = obj_manager->get_obj(x, y, z);
	if (!usecode->is_door(obj))
		return;
	usecode->use_obj(obj, get_actor());
	subtract_movement_points(MOVE_COST_USE);
	map_window->updateBlacking();
}

// teleport-type move
void Player::move(sint16 new_x, sint16 new_y, uint8 new_level, bool teleport) {
	if (actor->move(new_x, new_y, new_level, ACTOR_FORCE_MOVE)) {
		//map_window->centerMapOnActor(actor);
		if (party->is_leader(actor)) { // lead party
			if (actor_manager->get_avatar()->get_hp() == 0) { // need to end turn if Avatar died
				actor_manager->startActors();
				return;
			}
			party->move(new_x, new_y, new_level); // center everyone first
			party->follow(0, 0); // then try to move them to correct positions
		}
		actor_manager->updateActors(new_x, new_y, new_level);
		if (teleport && new_level != 0 && new_level != 5)
			Game::get_game()->get_weather()->set_wind_dir(NUVIE_DIR_NONE);
		obj_manager->update(new_x, new_y, new_level, teleport); // remove temporary objs, hatch eggs
		// it's still the player's turn
	}
}

void Player::moveLeft() {
	moveRelative(-1, 0);
}

void Player::moveRight() {
	moveRelative(1, 0);
}

void Player::moveUp() {
	moveRelative(0, -1);
}

void Player::moveDown() {
	moveRelative(0, 1);
}

void Player::pass() {
	Game::get_game()->get_script()->call_player_pass();
// uint16 x = actor->x, y = actor->y;
// uint8 z = actor->z;

// Move balloon / raft if required.
	if (game_type == NUVIE_GAME_U6 && (actor->obj_n == OBJ_U6_INFLATED_BALLOON || actor->obj_n == OBJ_U6_RAFT)) {
		if (Game::get_game()->get_weather()->get_wind_dir() != NUVIE_DIR_NONE) {
			moveRelative(0, 0);
			//return;
		}
	}

	if (actor->get_moves_left() > 0)
		actor->set_moves_left(0); // Pass and use up moves

// update world around player
	if (party_mode && party->is_leader(actor)) // lead party
		party->follow(0, 0);
// actor_manager->updateActors(x, y, z); // not needed because position is unchanged
	clock->inc_move_counter_by_a_minute(); // doesn't update time
	actor_manager->startActors(); // end player turn
//actor_manager->moveActors();
	Game::get_game()->time_changed();
}



/* Enter party mode, with everyone following actor. (must be in the party)
 */
bool Player::set_party_mode(Actor *new_actor) {
	if (party->contains_actor(new_actor) || party->is_in_vehicle()) {
		party_mode = true;
		set_actor(new_actor);
		return (true);
	}
	return (false);
}


/* Enter solo mode as actor. (must be in the party)
 */
bool Player::set_solo_mode(Actor *new_actor) {
	if (party->contains_actor(new_actor)) {
		if (new_actor->is_charmed()) {
			Game::get_game()->get_scroll()->display_fmt_string("%s fails to respond.\n\n", new_actor->get_name());
			return false;
		}
		party_mode = false;
		set_actor(new_actor);
		return (true);
	}
	return (false);
}


/* Returns the delay in continuous movement for the actor type we control.
 */
uint32 Player::get_walk_delay() {
	if (game_type != NUVIE_GAME_U6)
		return (125); // normal movement about 8 spaces per second

	if (actor->obj_n == OBJ_U6_BALLOON_BASKET)
		return (10); // 10x normal (wow!)
	else if (actor->obj_n == OBJ_U6_SHIP)
		return (20); // 5x normal
	else if (actor->obj_n == OBJ_U6_SKIFF)
		return (50); // 2x normal
	else if (actor->obj_n == OBJ_U6_RAFT)
		return (100); // normal
	else if (actor->obj_n == OBJ_U6_HORSE_WITH_RIDER && party->is_everyone_horsed())
		return (50); // 2x normal
	else
		return (125); // normal movement about 8 spaces per second
}


/* Returns true if it's time for the player to take another step.
 * (call during continuous movement)
 */
bool Player::check_walk_delay() {
	static uint32 walk_delay = 0, // start with no delay
	              last_time = clock->get_ticks();
	uint32 this_time = clock->get_ticks();
	uint32 time_passed = this_time - last_time;

	// subtract time_passed until delay is 0
	if (sint32(walk_delay - time_passed) < 0)
		walk_delay = 0;
	else
		walk_delay -= time_passed;
	last_time = this_time; // set each call to get time_passed
	if (walk_delay == 0) {
		walk_delay = get_walk_delay(); // reset
		return (true);
	}
	return (false); // not time yet
}

bool Player::weapon_can_hit(uint16 x, uint16 y) {
	return actor->weapon_can_hit(actor->get_weapon(current_weapon), x, y);
}

void Player::attack_select_init(bool use_attack_text) {
	uint16 x, y;
	uint8 z;

	current_weapon = ACTOR_NO_READIABLE_LOCATION;

	if (attack_select_next_weapon(false, use_attack_text) == false)
		attack_select_weapon_at_location(ACTOR_NO_READIABLE_LOCATION, use_attack_text); // attack with hands.

	map_window->centerCursor();

	CombatTarget target = party->get_combat_target(actor->id_n == 0 ? 0 : party->get_member_num(actor));
	Actor *target_actor = NULL;

	switch (target.type) {
	case TARGET_ACTOR :
		target_actor = actor_manager->get_actor(target.actor_num);
		uint16 target_x, target_y;
		map_window->get_pos(&x, &y, &z);
		target_x = x;
		target_y = y;
		if (target_actor && target_actor->is_onscreen() && target_actor->is_alive() && target_actor->is_visible() && actor->weapon_can_hit(actor->get_weapon(current_weapon), target_actor, &target_x, &target_y)) {
			map_window->moveCursor(target_x - x, target_y - y);
		} else {
			party->clear_combat_target(actor->id_n == 0 ? 0 : party->get_member_num(actor));
		}
		break;

	case TARGET_LOCATION :
		if (target.loc.z == actor->get_z() && weapon_can_hit(target.loc.x, target.loc.y)) {
			map_window->get_pos(&x, &y, &z);
			map_window->moveCursor(target.loc.x - x, target.loc.y - y);
		} else {
			party->clear_combat_target(actor->id_n == 0 ? 0 : party->get_member_num(actor));
		}
		break;

	default :
		break;
	}

	return;
}

bool Player::attack_select_next_weapon(bool add_newline, bool use_attack_text) {
	sint8 i;

	for (i = current_weapon + 1; i < ACTOR_MAX_READIED_OBJECTS; i++) {
		if (attack_select_weapon_at_location(i, add_newline, use_attack_text) == true)
			return true;
	}

	return false;
}

bool Player::attack_select_weapon_at_location(sint8 location, bool add_newline, bool use_attack_text) {
	const CombatType *weapon;
	MsgScroll *scroll = Game::get_game()->get_scroll();

	if (location == ACTOR_NO_READIABLE_LOCATION) {
		current_weapon = location;
		if (use_attack_text == false)
			return true;
		if (add_newline)
			scroll->display_string("\n");
		if (game_type == NUVIE_GAME_U6 && actor->obj_n == OBJ_U6_SHIP)
			scroll->display_string("Attack with ship cannons-");
		else
			scroll->display_string("Attack with bare hands-");

		return true;
	}

	weapon = actor->get_weapon(location);

	if (weapon && weapon->attack > 0) {
		current_weapon = location;
		if (use_attack_text == false)
			return true;
		if (add_newline)
			scroll->display_string("\n");
		scroll->display_fmt_string("Attack with %s-", obj_manager->get_obj_name(weapon->obj_n));
		return true;
	}

	return false;
}

void Player::attack(MapCoord target, Actor *target_actor) {
	MsgScroll *scroll = Game::get_game()->get_scroll();

	if (weapon_can_hit(target.x, target.y)) {
		if (!target_actor)
			target_actor = actor_manager->get_actor(target.x, target.y, actor->get_z());

		actor->attack(current_weapon, target, target_actor);

		if (target_actor) {
			party->set_combat_target(actor->id_n == 0 ? 0 : party->get_member_num(actor), target_actor);
		} else {
			Obj *target_obj = obj_manager->get_obj(target.x, target.y, actor->get_z());
			if (target_obj) {
				party->set_combat_target(actor->id_n == 0 ? 0 : party->get_member_num(actor), MapCoord(target_obj));
			}
		}
	} else
		scroll->display_string("\nOut of range!\n");

//actor_manager->startActors(); // end player turn
	return;
}

// Switch to controlling another actor
void Player::update_player(Actor *next_player) {
	MsgScroll *scroll = Game::get_game()->get_scroll();
	bool same_actor = (next_player == get_actor());
	set_actor(next_player); // redirects to ActorManager::set_player()
	set_mapwindow_centered(true);

	if (!scroll->can_display_prompt() && same_actor)
		return;
	scroll->display_string("\n");
	scroll->display_prompt();
}

/* Rest and repair ship. */
void Player::repairShip() {
	MsgScroll *scroll = Game::get_game()->get_scroll();
	Actor *ship = get_actor();

	if (ship->get_obj_n() != OBJ_U6_SHIP)
		return;
	// ship actor's health is hull strength
	if (ship->get_hp() + 5 > 100) ship->set_hp(100);
	else                       ship->set_hp(ship->get_hp() + 5);

	scroll->display_fmt_string("Hull Strength: %d%%\n", ship->get_hp());

	Game::get_game()->get_script()->call_advance_time(5);
	Game::get_game()->time_changed();
}

} // End of namespace Nuvie
} // End of namespace Ultima
