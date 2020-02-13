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
#include "ultima/nuvie/files/nuvie_io.h"
#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/core/converse.h"
#include "ultima/nuvie/core/timed_event.h"
#include "ultima/nuvie/conf/configuration.h"
#include "ultima/nuvie/actors/actor_manager.h"
#include "ultima/nuvie/sound/sound_manager.h"
#include "ultima/nuvie/views/view_manager.h"
#include "ultima/nuvie/core/player.h"
#include "ultima/nuvie/core/map.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/usecode/u6_usecode.h"
#include "ultima/nuvie/gui/widgets/command_bar.h"
#include "ultima/nuvie/pathfinder/party_path_finder.h"
#include "ultima/nuvie/core/party.h"
#include "ultima/nuvie/views/view.h"
#include "ultima/nuvie/save/obj_list.h"
#include "ultima/nuvie/core/events.h"

namespace Ultima {
namespace Nuvie {

Party::Party(Configuration *cfg) {
	config = cfg;
	game = NULL;
	actor_manager = NULL;
	map = NULL;
	pathfinder = NULL;
	rest_campfire = NULL;

	formation = PARTY_FORM_STANDARD;
	num_in_party = 0;
	prev_leader_x = prev_leader_y = 0;
	defer_removing_dead_members = false;
	autowalk = false;
	in_vehicle = false;
	in_combat_mode = false;
	lightsources = 0;

	memset(&member, 0, sizeof member);
}

Party::~Party() {
	if (pathfinder)
		delete pathfinder;
}

bool Party::init(Game *g, ActorManager *am) {
	Std::string formation_string;

	game = g;
	actor_manager = am;
	map = g->get_game_map();
	if (!pathfinder)
		pathfinder = new PartyPathFinder(this);

	autowalk = false;
	in_vehicle = false;

	config->value("config/general/party_formation", formation_string, "");
	if (formation_string == "row")
		formation = PARTY_FORM_ROW;
	else if (formation_string == "column")
		formation = PARTY_FORM_COLUMN;
	else if (formation_string == "delta")
		formation = PARTY_FORM_DELTA;
	else
		formation = PARTY_FORM_STANDARD;

	config->value("config/audio/combat_changes_music", combat_changes_music, true);
	config->value("config/audio/vehicles_change_music", vehicles_change_music, true);

	return true;
}

bool Party::load(NuvieIO *objlist) {
	uint8 actor_num;
	uint16 i;

	autowalk = false;
	in_vehicle = false;

	objlist->seek(OBJLIST_OFFSET_NUM_IN_PARTY);
	num_in_party = objlist->read1();


	objlist->seek(OBJLIST_OFFSET_PARTY_NAMES);
	for (i = 0; i < num_in_party; i++) {
		objlist->readToBuf((unsigned char *)member[i].name, PARTY_NAME_MAX_LENGTH + 1); // read in Player name.
	}
	objlist->seek(OBJLIST_OFFSET_PARTY_ROSTER);
	for (i = 0; i < num_in_party; i++) {
		actor_num = objlist->read1();
		member[i].actor = actor_manager->get_actor(actor_num);
		member[i].actor->set_in_party(true);
		//member[i].inactive = false; // false unless actor is asleep, or paralyzed (is_immobile)
	}

	objlist->seek(OBJLIST_OFFSET_U6_COMBAT_MODE); // combat mode flag. NOTE! this offset is probably U6 specifix FIXME
	in_combat_mode = (bool)objlist->read1();

	MapCoord leader_loc = get_leader_location(); // previous leader location
	prev_leader_x = leader_loc.x;
	prev_leader_y = leader_loc.y;

	reform_party();

	autowalk = false;

	if (actor_manager->get_actor(ACTOR_VEHICLE_ID_N)->get_worktype() == ACTOR_WT_PLAYER) { // WT_U6_PLAYER
		set_in_vehicle(true);
		hide();
	}

	for (i = 0; i < PARTY_MAX_MEMBERS; i++) {
		clear_combat_target(i);
	}

	update_light_sources();
	update_music();

	return true;
}

bool Party::save(NuvieIO *objlist) {
	uint16 i;

	objlist->seek(OBJLIST_OFFSET_NUM_IN_PARTY);
	objlist->write1(num_in_party);


	objlist->seek(OBJLIST_OFFSET_PARTY_NAMES);
	for (i = 0; i < num_in_party; i++) {
		objlist->writeBuf((unsigned char *)member[i].name, PARTY_NAME_MAX_LENGTH + 1);
	}

	objlist->seek(OBJLIST_OFFSET_PARTY_ROSTER);
	for (i = 0; i < num_in_party; i++) {
		objlist->write1(member[i].actor->get_actor_num());
	}

	objlist->seek(OBJLIST_OFFSET_U6_COMBAT_MODE); // combat mode flag. NOTE! this offset is probably U6 specifix FIXME
	objlist->write1((uint8)in_combat_mode);

	return true;
}

bool Party::add_actor(Actor *actor) {
	Converse *converse = game->get_converse();

	if (num_in_party < PARTY_MAX_MEMBERS) {
		actor->set_in_party(true);
		member[num_in_party].actor = actor;
		//member[num_in_party].inactive = false;
		strncpy(member[num_in_party].name, converse->npc_name(actor->id_n), PARTY_NAME_MAX_LENGTH + 1);
		member[num_in_party].name[PARTY_NAME_MAX_LENGTH] = '\0'; // make sure name is terminated
		member[num_in_party].combat_position = 0;
//    member[num_in_party].leader_x = member[0].actor->get_location().x;
//    member[num_in_party].leader_y = member[0].actor->get_location().y;

		num_in_party++;
		reform_party();
		return true;
	}

	return false;
}


// remove actor from member array shuffle remaining actors down if required.
bool Party::remove_actor(Actor *actor, bool keep_party_flag) {
	if (defer_removing_dead_members) //we don't want to remove member while inside the Party::follow() method.
		return true;
	Game::get_game()->get_event()->set_control_cheat(false);
	uint8 i;

	for (i = 0; i < num_in_party; i++) {
		if (member[i].actor->id_n == actor->id_n) {
			if (keep_party_flag == false) {
				for (int j = 0; j < member[i].actor->get_num_light_sources(); j++)
					subtract_light_source();
				member[i].actor->set_in_party(false);
			}
			if (i != (num_in_party - 1)) {
				for (; i + 1 < num_in_party; i++)
					member[i] = member[i + 1];
			}
			num_in_party--;

			reform_party();
			if (game->is_new_style()) {
				Game::get_game()->get_event()->close_gumps();
				return true;
			}
			//FIXME this is a bit hacky we need a better way to refresh views when things change
			//maybe using callbacks.
			//If the last actor dies and was being displayed in a view then we need to set the
			//view to the new last party member.
			View *cur_view = Game::get_game()->get_view_manager()->get_current_view();
			if (cur_view && cur_view->get_party_member_num() >= num_in_party)
				cur_view->set_party_member(num_in_party - 1);
			else if (cur_view)
				cur_view->set_party_member(cur_view->get_party_member_num()); // needed if a middle party member dies
			return true;
		}
	}

	return false;
}

bool Party::remove_dead_actor(Actor *actor) {
	return remove_actor(actor, PARTY_KEEP_PARTY_FLAG);
}

bool Party::resurrect_dead_members() {
	uint16 i;
	Actor *actor;
	MapCoord new_pos = get_leader_location();
	if (Game::get_game()->get_event()->using_control_cheat()) {
		Game::get_game()->get_event()->set_control_cheat(false);
		if (!Game::get_game()->is_new_style()) {
			Game::get_game()->get_view_manager()->set_inventory_mode();
			Game::get_game()->get_view_manager()->get_current_view()->set_party_member(0);
		}
	}

	for (i = 0; i < ACTORMANAGER_MAX_ACTORS; i++) {
		actor = actor_manager->get_actor(i);
		if (actor->is_in_party() && actor->is_alive() == false)
			actor->resurrect(new_pos);
	}
	update_light_sources(); // should only be needed for control cheat but it won't hurt to recheck
	return true;
}


void Party::split_gold() {
}

void Party::gather_gold() {
}

uint8 Party::get_party_size() {
	return num_in_party;
}

Actor *Party::get_leader_actor() {
	sint8 leader = get_leader();
	if (leader < 0) {
		return NULL;
	}

	return get_actor(leader);
}

Actor *Party::get_actor(uint8 member_num) {
	if (num_in_party <= member_num)
		return NULL;

	return member[member_num].actor;
}

char *Party::get_actor_name(uint8 member_num) {
	if (num_in_party <= member_num)
		return NULL;

	return member[member_num].name;
}


/* Returns position of actor in party or -1.
 */
sint8 Party::get_member_num(Actor *actor) {
	for (int i = 0; i < num_in_party; i++) {
		if (member[i].actor->id_n == actor->id_n)
			return (i);
	}
	return (-1);
}

sint8 Party::get_member_num(uint8 a) {
	return (get_member_num(actor_manager->get_actor(a)));
}

uint8 Party::get_actor_num(uint8 member_num) {
	if (num_in_party <= member_num)
		return 0; // hmm how should we handle this error.

	return member[member_num].actor->id_n;
}

/* Rearrange member slot positions based on the number of members and the
 * selected formation. Used only when adding or removing actors.
 */
void Party::reform_party() {
	uint32 n;
	sint32 x, y, max_x;
	bool even_row;
	sint8 leader = get_leader();
	if (leader < 0 || num_in_party == 1)
		return;
	member[leader].form_x = 0;
	member[leader].form_y = 0;
	switch (formation) {
	case PARTY_FORM_COLUMN: // line up behind Avatar
		x = 0;
		y = 1;
		for (n = leader + 1; n < num_in_party; n++) {
			member[n].form_x = x;
			member[n].form_y = y++;
			if (y == 5) {
				x += 1;
				y = 0;
			}
		}
		break;
	case PARTY_FORM_ROW: // line up left of Avatar
		x = -1;
		y = 0;
		for (n = leader + 1; n < num_in_party; n++) {
			member[n].form_x = x--;
			member[n].form_y = y;
			if (x == -5) {
				y += 1;
				x = 0;
			}
		}
		break;
	case PARTY_FORM_DELTA: // open triangle formation with Avatar at front
		x = -1;
		y = 1;
		for (n = leader + 1; n < num_in_party; n++) {
			member[n].form_x = x;
			member[n].form_y = y;
			// alternate X once, then move down
			x = -x;
			if (x == 0 || x < 0) {
				x -= 1;
				++y;
			}
			if (y == 5) { // start at top of triangle
				y -= ((-x) - 1);
				x = 0;
			}
		}
		break;
//        case PARTY_FORM_COMBAT: // positions determined by COMBAT mode
//            break;
	case PARTY_FORM_REST: // special formation used while Resting
		member[1].form_x = 0;
		member[1].form_y = -2;
		member[2].form_x = 1;
		member[2].form_y = -1;
		member[3].form_x = -1;
		member[3].form_y = -1;
		member[4].form_x = 1;
		member[4].form_y = 0;
		member[5].form_x = -1;
		member[5].form_y = -2;
		member[6].form_x = 1;
		member[6].form_y = -2;
		member[7].form_x = -1;
		member[7].form_y = 0;
		break;
	case PARTY_FORM_STANDARD: // U6
	default:
		// put first follower behind or behind and to the left of Avatar
		member[leader + 1].form_x = (num_in_party >= 3) ? -1 : 0;
		member[leader + 1].form_y = 1;
		x = y = 1;
		even_row = false;
		for (n = leader + 2, max_x = 1; n < num_in_party; n++) {
			member[n].form_x = x;
			member[n].form_y = y;
			// alternate & move X left/right by 2
			x = (x == 0) ? x - 2 : (x < 0) ? -x : -x - 2;
			if (x > max_x || (x < 0 && -x > max_x)) { // reached row max.
				++y;
				even_row = !even_row; // next row
				++max_x; // increase row max.
				x = even_row ? 0 : -1; // next guy starts at center or left by 2
			}
		}
	}
}

/* Returns number of person leading the party (the first active member), or -1
 * if the party has no leader and can't move. */
sint8 Party::get_leader() {
	for (int m = 0; m < num_in_party; m++)
		if (member[m].actor->is_immobile() == false && member[m].actor->is_charmed() == false)
			return m;
	return -1;
}

/* Get map location of a party member. */
MapCoord Party::get_location(uint8 m) {
	return (member[m].actor->get_location());
}

/* Get map location of the first active party member. */
MapCoord Party::get_leader_location() {
	sint8 m = get_leader();
	MapCoord loc;
	if (m >= 0)
		loc = member[m].actor->get_location();
	return (loc);
}

/* Returns absolute location where party member `m' SHOULD be (based on party
 * formation and leader location.
 */
MapCoord Party::get_formation_coords(uint8 m) {
	MapCoord a = get_location(m); // my location
	MapCoord l = get_leader_location(); // leader location
	sint8 leader = get_leader();
	if (leader < 0)
		return (a);
	uint8 ldir = member[leader].actor->get_direction(); // leader direction
	// intended location
	uint16 x = (ldir == NUVIE_DIR_N) ? l.x + member[m].form_x : // X
	           (ldir == NUVIE_DIR_E) ? l.x - member[m].form_y :
	           (ldir == NUVIE_DIR_S) ? l.x - member[m].form_x :
	           (ldir == NUVIE_DIR_W) ? l.x + member[m].form_y : a.x;
	uint16 y = (ldir == NUVIE_DIR_N) ? l.y + member[m].form_y : // Y
	           (ldir == NUVIE_DIR_E) ? l.y + member[m].form_x :
	           (ldir == NUVIE_DIR_S) ? l.y - member[m].form_y :
	           (ldir == NUVIE_DIR_W) ? l.y - member[m].form_x : a.y;
	return (MapCoord(WRAPPED_COORD(x, a.z),
	                 WRAPPED_COORD(y, a.z),
	                 a.z // Z
	                ));
}


/* Update the actual locations of the party actors on the map, so that they are
 * in the proper formation. */
void Party::follow(sint8 rel_x, sint8 rel_y) {
	Common::Array<bool> try_again;
	try_again.resize(get_party_max());

	sint8 leader = get_leader();
	if (leader <= -1)
		return;

	if (is_in_combat_mode()) { // just update everyone's combat mode
		for (int p = 0; p < get_party_size(); p++)
			get_actor(p)->set_worktype(get_actor(p)->get_combat_mode());
		return;
	}

	defer_removing_dead_members = true;

	// set previous leader location first, just in case the leader changed
	prev_leader_x = WRAPPED_COORD(member[leader].actor->x - rel_x, member[leader].actor->z);
	prev_leader_y = member[leader].actor->y - rel_y;
	// PASS 1: Keep actors chained together.
	for (uint32 p = (leader + 1); p < num_in_party; p++) {
		if (member[p].actor->is_immobile()) continue;

		try_again[p] = false;
		if (!pathfinder->follow_passA(p))
			try_again[p] = true;
	}
	// PASS 2: Catch up to party.
	for (uint32 p = (leader + 1); p < num_in_party; p++) {
		if (member[p].actor->is_immobile()) continue;

		if (try_again[p])
			pathfinder->follow_passA(p);
		pathfinder->follow_passB(p);
		if (!pathfinder->is_contiguous(p)) {
			sint8 l = get_leader();
			if (l >= 0) {
				DEBUG(0, LEVEL_DEBUGGING, "%s is looking for %s.\n", get_actor_name(p), get_actor_name(l));
			}
			pathfinder->seek_leader(p); // enter/update seek mode
		} else if (member[p].actor->get_pathfinder())
			pathfinder->end_seek(p);

		get_actor(p)->set_moves_left(get_actor(p)->get_moves_left() - 10);
		get_actor(p)->set_worktype(0x01); // revert to normal worktype
	}

	defer_removing_dead_members = false;

	//remove party members that died during follow routine.
	for (int p = get_party_size() - 1; p >= 0; p--) {
		Actor *a = get_actor(p);
		if (a->is_alive() == false)
			remove_actor(a, PARTY_KEEP_PARTY_FLAG);
	}
}

// Returns true if anyone in the party has a matching object.
bool Party::has_obj(uint16 obj_n, uint8 quality, bool match_zero_qual) {
	uint16 i;

	for (i = 0; i < num_in_party; i++) {
		if (member[i].actor->inventory_get_object(obj_n, quality, match_zero_qual) != NULL) // we got a match
			return true;
	}

	return false;
}

// Removes the first occurence of an object in the party.
bool Party::remove_obj(uint16 obj_n, uint8 quality) {
	uint16 i;
	Obj *obj;

	for (i = 0; i < num_in_party; i++) {
		obj = member[i].actor->inventory_get_object(obj_n, quality);
		if (obj != NULL) {
			if (member[i].actor->inventory_remove_obj(obj)) {
				delete_obj(obj);
				return true;
			}
		}
	}

	return false;
}

// Returns the actor id of the first person in the party to have a matching object.
Actor *Party::who_has_obj(uint16 obj_n, uint8 quality, bool match_qual_zero) {
	uint16 i;
	for (i = 0; i < num_in_party; i++) {
		if (member[i].actor->inventory_get_object(obj_n, quality, match_qual_zero) != NULL)
			return (member[i].actor);
	}
	return NULL;
}

Obj *Party::get_obj(uint16 obj_n, uint8 quality, bool match_qual_zero, uint8 frame_n, bool match_frame_n) {
	Obj *obj;
	for (uint16 i = 0; i < num_in_party; i++) {
		obj = member[i].actor->inventory_get_object(obj_n, quality, match_qual_zero, frame_n, match_frame_n);
		if (obj)
			return obj;
	}
	return NULL;
}

/* Is EVERYONE in the party at or near the coordinates?
 */
bool Party::is_at(uint16 x, uint16 y, uint8 z, uint32 threshold) {
	for (uint32 p = 0; p < num_in_party; p++) {
		MapCoord loc(x, y, z);
		if (!member[p].actor->is_nearby(loc, threshold))
			return (false);
	}
	return (true);
}

bool Party::is_at(MapCoord &xyz, uint32 threshold) {
	return (is_at(xyz.x, xyz.y, xyz.z, threshold));
}

/* Is ANYONE in the party at or near the coordinates? */
bool Party::is_anyone_at(uint16 x, uint16 y, uint8 z, uint32 threshold) {
	for (uint32 p = 0; p < num_in_party; p++) {
		MapCoord loc(x, y, z);
		if (member[p].actor->is_nearby(loc, threshold))
			return (true);
	}
	return (false);
}

bool Party::is_anyone_at(MapCoord &xyz, uint32 threshold) {
	return (is_anyone_at(xyz.x, xyz.y, xyz.z, threshold));
}

bool Party::contains_actor(Actor *actor) {
	if (get_member_num(actor) >= 0)
		return (true);

	return (false);
}

bool Party::contains_actor(uint8 actor_num) {
	return (contains_actor(actor_manager->get_actor(actor_num)));
}

void Party::set_in_combat_mode(bool value) {
	in_combat_mode = value;
	actor_manager->set_combat_movement(value);

	if (in_combat_mode) {
		for (int p = 0; p < get_party_size(); p++)
			get_actor(p)->set_worktype(get_actor(p)->get_combat_mode()); //set combat worktype
	} else {
		for (int p = 0; p < get_party_size(); p++)
			get_actor(p)->set_worktype(ACTOR_WT_FOLLOW); //set back to follow party leader.
	}
//  if(combat_changes_music)
	update_music();
	if (game->get_command_bar() != NULL) {
		game->get_command_bar()->set_combat_mode(in_combat_mode);
	}
}

void Party::update_music() {
	SoundManager *s = Game::get_game()->get_sound_manager();
	MapCoord pos;

	if (in_vehicle && vehicles_change_music) {
		s->musicPlayFrom("boat");
		return;
	} else if (in_combat_mode && combat_changes_music) {
		s->musicPlayFrom("combat");
		return;
	}

	pos = get_leader_location();

	switch (pos.z) {
	case 0 :
		s->musicPlayFrom("random");
		break;
	case 5 :
		s->musicPlayFrom("gargoyle");
		break;
	default :
		s->musicPlayFrom("dungeon");
		break;
	}

	return;
}

void Party::heal() {
	uint16 i;

	for (i = 0; i < num_in_party; i++) {
		member[i].actor->heal();
	}

	return;

}

void Party::cure() {
	for (uint16 i = 0; i < num_in_party; i++) {
		member[i].actor->cure();
	}
}

void Party::set_ethereal(bool ethereal) {
	for (uint16 i = 0; i < num_in_party; i++) {
		member[i].actor->set_ethereal(ethereal);
	}
}

void Party::show() {
	uint16 i;

	for (i = 0; i < num_in_party; i++) {
		member[i].actor->show();
	}

	return;
}

void Party::hide() {
	uint16 i;

	for (i = 0; i < num_in_party; i++) {
		member[i].actor->hide();
	}

	return;
}

/* Move and center everyone in the party to one location.
 */
bool Party::move(uint16 dx, uint16 dy, uint8 dz) {
	for (sint32 m = 0; m < num_in_party; m++)
		if (!member[m].actor->move(dx, dy, dz, ACTOR_FORCE_MOVE))
			return (false);
	return (true);
}


/* Automatically walk (timed) to a destination, and then teleport to new
 * location (optional). Used to enter/exit dungeons.
 * (step_delay 0 = default speed)
 */
void Party::walk(MapCoord *walkto, MapCoord *teleport, uint32 step_delay) {
	if (step_delay)
		new TimedPartyMove(walkto, teleport, step_delay);
	else
		new TimedPartyMove(walkto, teleport);

	game->pause_world(); // other actors won't move
	game->pause_user(); // don't allow input
	// view will snap back to player after everyone has moved
	game->get_player()->set_mapwindow_centered(false);
	autowalk = true;
}


/* Enter a moongate and teleport to a new location.
 * (step_delay 0 = default speed)
 */
void Party::walk(Obj *moongate, MapCoord *teleport, uint32 step_delay) {
	MapCoord walkto(moongate->x, moongate->y, moongate->z);
	if (step_delay)
		new TimedPartyMove(&walkto, teleport, moongate, step_delay);
	else
		new TimedPartyMove(&walkto, teleport, moongate);

	game->pause_world(); // other actors won't move
	game->pause_user(); // don't allow input
	// view will snap back to player after everyone has moved
	game->get_player()->set_mapwindow_centered(false);
	autowalk = true;
}



/* Automatically walk (timed) to vehicle. (step_delay 0 = default speed)
 */
void Party::enter_vehicle(Obj *ship_obj, uint32 step_delay) {
	MapCoord walkto(ship_obj->x, ship_obj->y, ship_obj->z);

	dismount_from_horses();

	if (step_delay)
		new TimedPartyMoveToVehicle(&walkto, ship_obj, step_delay);
	else
		new TimedPartyMoveToVehicle(&walkto, ship_obj);

	game->pause_world(); // other actors won't move
	game->pause_user(); // don't allow input
	// view will snap back to player after everyone has moved
	game->get_player()->set_mapwindow_centered(false);
	autowalk = true;
}

void Party::exit_vehicle(uint16 x, uint16 y, uint16 z) {
	if (is_in_vehicle() == false)
		return;

	Actor *vehicle_actor = actor_manager->get_actor(0);

	show();
	vehicle_actor->unlink_surrounding_objects();
	vehicle_actor->hide();
	vehicle_actor->set_worktype(0);

	Player *player = game->get_player();

	player->set_actor(get_actor(0));
	player->move(x, y, z, false);
	vehicle_actor->obj_n = 0;//OBJ_U6_NO_VEHICLE;
	vehicle_actor->frame_n = 0;
	vehicle_actor->init();
	vehicle_actor->move(0, 0, 0, ACTOR_FORCE_MOVE);

	set_in_vehicle(false);
}

void Party::set_in_vehicle(bool value) {
	in_vehicle = value;
	if (vehicles_change_music)
		update_music();
	if (value) {
		if (in_combat_mode == true)
			set_in_combat_mode(false); // break off combat when boarding a vehicle
	}

	return;
}

/* Done automatically walking, return view to player character.
 */
void Party::stop_walking(bool force_music_change) {
	game->get_player()->set_mapwindow_centered(true);
	game->unpause_world(); // allow user input, unfreeze actors
	game->unpause_user();
	autowalk = false;
	if (force_music_change || vehicles_change_music)
		update_music();
}

void Party::dismount_from_horses() {
	UseCode *usecode = Game::get_game()->get_usecode();

	for (uint32 m = 0; m < num_in_party; m++) {
		if (member[m].actor->obj_n == OBJ_U6_HORSE_WITH_RIDER) {
			Obj *my_obj = member[m].actor->make_obj();
			usecode->use_obj(my_obj, member[m].actor);
			delete_obj(my_obj);
		}
	}

	return;
}

Actor *Party::get_slowest_actor() {
	Actor *actor = 0;
	sint8 begin = get_leader();
	if (begin >= 0) {
		actor = member[begin].actor;
		sint8 moves = actor->get_moves_left();
		for (uint32 m = begin + 1; m < num_in_party; m++) {
			sint8 select_moves = member[m].actor->get_moves_left();
			if (member[m].actor->is_immobile() == false && (select_moves < moves)) {
				moves = select_moves;
				actor = member[m].actor;
			}
		}
	}
	return actor;
}

/* Gather everyone around a campfire to Rest. */
void Party::rest_gather() {
	Actor *player_actor = get_leader_actor();
	if (player_actor) {
		MapCoord player_loc = player_actor->get_location();
		rest_campfire = new_obj(OBJ_U6_CAMPFIRE, 1, player_loc.x, player_loc.y, player_loc.z);
		rest_campfire->set_temporary();
		rest_campfire->qty = 1; //this is set so the campfire may be destroyed by being attacked.
		game->get_obj_manager()->add_obj(rest_campfire, true); // addOnTop

		game->get_player()->set_mapwindow_centered(false);
		game->pause_user();
		new TimedRestGather(player_loc.x, player_loc.y);
	}
}

/* Start Resting for the specified number of hours, optionally with a party
 * member standing guard. */
void Party::rest_sleep(uint8 hours, sint16 guard) {
	// FIXME: change music to Stones when asking "How many hours?", change to
	// a random song when finished camping (or if cancelled)
	new TimedRest(hours, guard >= 0 ? member[guard].actor : 0, rest_campfire);
}

bool Party::can_rest(Std::string &err_str) {
	Map *map_ = game->get_game_map();
	Player *player = game->get_player();
	Actor *pActor = player->get_actor();
	MapCoord loc = pActor->get_location();

	ActorList *enemies = 0;
	ActorList *all_actors = 0;

	if (is_in_combat_mode()) {
		if (Game::get_game()->get_game_type() == NUVIE_GAME_SE)
			err_str = "\nNot while in Combat mode!";
		else if (Game::get_game()->get_game_type() == NUVIE_GAME_MD)
			err_str = "- Not while in Combat!";
		else
			err_str = "-Not while in Combat!";
	} else if (is_in_vehicle()
	           && pActor->get_obj_n() != OBJ_U6_SHIP) // player is a vehicle
		err_str = "-Can not be repaired!";
	else if (Game::get_game()->get_game_type() == NUVIE_GAME_U6
	         && game->get_map_window()->in_town())
		err_str = "-Only in the wilderness!";
	else if ((enemies = pActor->find_enemies())) {
		if (Game::get_game()->get_game_type() == NUVIE_GAME_MD)
			err_str = "\nNot while foes are near!";
		if (Game::get_game()->get_game_type() == NUVIE_GAME_SE)
			err_str = "- Not while foes are near!";
		else
			err_str = "-Not while foes are near!";
	} else if ((all_actors = actor_manager->filter_party(actor_manager->filter_distance(actor_manager->get_actor_list(),
	                         loc.x, loc.y, loc.z, 5)))
	           && !all_actors->empty() && !is_in_vehicle()) {
		if (Game::get_game()->get_game_type() == NUVIE_GAME_U6)
			err_str = "-Not while others are near!";
		else
			err_str = "\nIt's too noisy to sleep here!";
		delete all_actors;
	} else if (!player->in_party_mode())
		err_str = "-Not in solo mode!";
	else if (!is_in_vehicle() && !map_->is_passable(loc.x - 1, loc.y - 1, loc.x + 1, loc.y + 1, loc.z)
	         && Game::get_game()->get_game_type() != NUVIE_GAME_SE)
		err_str = "-Not enough room!"; // FIXME: for ships the original checks all squares around the ship. Do we really need this?
	else if (is_horsed())
		err_str = "-Dismount first!";
	else
		return true;
	delete enemies;
	return false;
}

bool Party::is_horsed() {
	for (int p = 0; p < num_in_party; p++)
		if (member[p].actor->get_obj_n() == OBJ_U6_HORSE_WITH_RIDER)
			return true;
	return false;
}

bool Party::is_everyone_horsed() {
	for (int p = 0; p < num_in_party; p++)
		if (member[p].actor->get_obj_n() != OBJ_U6_HORSE_WITH_RIDER)
			return false;
	return true;
}

Obj *Party::get_food() {
	for (int p = 0; p < num_in_party; p++) {
		Obj *food = member[p].actor->inventory_get_food();
		if (food)
			return food;
	}
	return 0;
}

void Party::set_combat_target(uint8 member_num, Actor *target) {
	if (num_in_party <= member_num)
		return;

	member[member_num].target.type = TARGET_ACTOR;
	member[member_num].target.actor_num = target->get_actor_num();
}

void Party::set_combat_target(uint8 member_num, MapCoord target) {
	if (num_in_party <= member_num)
		return;

	member[member_num].target.type = TARGET_LOCATION;
	member[member_num].target.loc = target;
}

void Party::clear_combat_target(uint8 member_num) {
	if (member_num >= PARTY_MAX_MEMBERS)
		return;

	member[member_num].target.type = TARGET_NONE;
	member[member_num].target.loc = MapCoord();
	member[member_num].target.actor_num = 0;
}

CombatTarget Party::get_combat_target(uint8 member_num) {
	if (num_in_party <= member_num) {
		CombatTarget noTarget;
		noTarget.type = TARGET_NONE;
		noTarget.loc = MapCoord();
		noTarget.actor_num = 0;
		return noTarget;
	}

	return member[member_num].target;
}

void Party::update_light_sources() {
	lightsources = 0;
	for (int i = 0; i < num_in_party; i++) {
		for (int j = 0; j < member[i].actor->get_num_light_sources(); j++)
			add_light_source();
	}
	if (game->get_event()->using_control_cheat()) {
		for (int i = 0; i < game->get_player()->get_actor()->get_num_light_sources(); i++)
			add_light_source();
	}
	game->get_map_window()->updateAmbience();
}

bool Party::has_light_source() {
	if (!game->get_player()->get_actor())
		return false;
	if (lightsources > 0) { // the original engine didn't care about distance
		if (game->get_event()->using_control_cheat()) {
			if (game->get_player()->get_actor()->get_num_light_sources() > 0)
				return true;
			else
				return false;
		}
		for (int i = 0; i < num_in_party; i++) {
			if (member[i].actor->get_num_light_sources() > 0) {
				if (!game->get_map_window()->tile_is_black(member[i].actor->x, member[i].actor->y)
				        && member[i].actor->is_nearby(game->get_player()->get_actor())) // within 5 tiles of player
					return true;
			}
		}
	}
	return false;
}

} // End of namespace Nuvie
} // End of namespace Ultima
