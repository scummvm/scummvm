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

#include "glk/adrift/scare.h"
#include "glk/adrift/scprotos.h"
#include "glk/adrift/scgamest.h"

namespace Glk {
namespace Adrift {

/* Trace flag, set before running. */
static sc_bool npc_trace = FALSE;


/*
 * npc_in_room()
 *
 * Return TRUE if a given NPC is currently in a given room.
 */
sc_bool npc_in_room(sc_gameref_t game, sc_int npc, sc_int room) {
	if (npc_trace) {
		sc_trace("NPC: checking NPC %ld in room %ld (NPC is in %ld)\n",
		         npc, room, gs_npc_location(game, npc));
	}

	return gs_npc_location(game, npc) - 1 == room;
}


/*
 * npc_count_in_room()
 *
 * Return the count of characters in the room, including the player.
 */
sc_int npc_count_in_room(sc_gameref_t game, sc_int room) {
	sc_int count, npc;

	/* Start with the player. */
	count = gs_player_in_room(game, room) ? 1 : 0;

	/* Total up other NPCs inhabiting the room. */
	for (npc = 0; npc < gs_npc_count(game); npc++) {
		if (gs_npc_location(game, npc) - 1 == room)
			count++;
	}
	return count;
}


/*
 * npc_start_npc_walk()
 *
 * Start the given walk for the given NPC.
 */
void npc_start_npc_walk(sc_gameref_t game, sc_int npc, sc_int walk) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[6];
	sc_int movetime;

	/* Retrieve movetime 0 for the NPC walk. */
	vt_key[0].string = "NPCs";
	vt_key[1].integer = npc;
	vt_key[2].string = "Walks";
	vt_key[3].integer = walk;
	vt_key[4].string = "MoveTimes";
	vt_key[5].integer = 0;
	movetime = prop_get_integer(bundle, "I<-sisisi", vt_key) + 1;

	/* Set up walkstep. */
	gs_set_npc_walkstep(game, npc, walk, movetime);
}


/*
 * npc_turn_update()
 * npc_setup_initial()
 *
 * Set initial values for NPC states, and update on turns.
 */
void npc_turn_update(sc_gameref_t game) {
	sc_int index_;

	/* Set current values for NPC seen states. */
	for (index_ = 0; index_ < gs_npc_count(game); index_++) {
		if (!gs_npc_seen(game, index_)
		        && npc_in_room(game, index_, gs_playerroom(game)))
			gs_set_npc_seen(game, index_, TRUE);
	}
}

void npc_setup_initial(sc_gameref_t game) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[5];
	sc_int index_;

	/* Start any walks that do not depend on a StartTask */
	vt_key[0].string = "NPCs";
	for (index_ = 0; index_ < gs_npc_count(game); index_++) {
		sc_int walk;

		/* Set up invariant parts of the properties key. */
		vt_key[1].integer = index_;
		vt_key[2].string = "Walks";

		/* Process each walk, starting at the last and working backwards. */
		for (walk = gs_npc_walkstep_count(game, index_) - 1; walk >= 0; walk--) {
			sc_int starttask;

			/* If StartTask is zero, start walk at game start. */
			vt_key[3].integer = walk;
			vt_key[4].string = "StartTask";
			starttask = prop_get_integer(bundle, "I<-sisis", vt_key);
			if (starttask == 0)
				npc_start_npc_walk(game, index_, walk);
		}
	}

	/* Update seen flags for initial states. */
	npc_turn_update(game);
}


/*
 * npc_room_in_roomgroup()
 *
 * Return TRUE if a given room is in a given group.
 */
static sc_bool npc_room_in_roomgroup(sc_gameref_t game, sc_int room, sc_int group) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[4];
	sc_int member;

	/* Check roomgroup membership. */
	vt_key[0].string = "RoomGroups";
	vt_key[1].integer = group;
	vt_key[2].string = "List";
	vt_key[3].integer = room;
	member = prop_get_integer(bundle, "I<-sisi", vt_key);
	return member != 0;
}


/* List of direction names, for printing entry/exit messages. */
static const sc_char *const DIRNAMES_4[] = {
	"the north", "the east", "the south", "the west", "above", "below",
	"inside", "outside",
	NULL
};
static const sc_char *const DIRNAMES_8[] = {
	"the north", "the east", "the south", "the west", "above", "below",
	"inside", "outside",
	"the north-east", "the south-east", "the south-west", "the north-west",
	NULL
};

/*
 * npc_random_adjacent_roomgroup_member()
 *
 * Return a random member of group adjacent to given room.
 */
static sc_int npc_random_adjacent_roomgroup_member(sc_gameref_t game, sc_int room, sc_int group) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[5];
	sc_bool eightpointcompass;
	sc_int roomlist[12], count, length, index_;

	/* If given room is "hidden", return nothing. */
	if (room == -1)
		return -1;

	/* How many exits to consider? */
	vt_key[0].string = "Globals";
	vt_key[1].string = "EightPointCompass";
	eightpointcompass = prop_get_boolean(bundle, "B<-ss", vt_key);
	if (eightpointcompass)
		length = sizeof(DIRNAMES_8) / sizeof(DIRNAMES_8[0]) - 1;
	else
		length = sizeof(DIRNAMES_4) / sizeof(DIRNAMES_4[0]) - 1;

	/* Poll adjacent rooms. */
	vt_key[0].string = "Rooms";
	vt_key[1].integer = room;
	vt_key[2].string = "Exits";
	count = 0;
	for (index_ = 0; index_ < length; index_++) {
		sc_int adjacent;

		vt_key[3].integer = index_;
		vt_key[4].string = "Dest";
		adjacent = prop_get_child_count(bundle, "I<-sisis", vt_key);

		if (adjacent > 0 && npc_room_in_roomgroup(game, adjacent - 1, group)) {
			roomlist[count] = adjacent - 1;
			count++;
		}
	}

	/* Return a random adjacent room, or -1 if nothing is adjacent. */
	return (count > 0) ? roomlist[sc_randomint(0, count - 1)] : -1;
}


/*
 * npc_announce()
 *
 * Helper for npc_tick_npc().
 */
static void npc_announce(sc_gameref_t game, sc_int npc, sc_int room, sc_bool is_exit, sc_int npc_room) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[5], vt_rvalue;
	const sc_char *text, *name, *const *dirnames;
	sc_int dir, dir_match;
	sc_bool eightpointcompass, showenterexit, found;

	/* If no announcement required, return immediately. */
	vt_key[0].string = "NPCs";
	vt_key[1].integer = npc;
	vt_key[2].string = "ShowEnterExit";
	showenterexit = prop_get_boolean(bundle, "B<-sis", vt_key);
	if (!showenterexit)
		return;

	/* Get exit or entry text, and NPC name. */
	vt_key[2].string = is_exit ? "ExitText" : "EnterText";
	text = prop_get_string(bundle, "S<-sis", vt_key);
	vt_key[2].string = "Name";
	name = prop_get_string(bundle, "S<-sis", vt_key);

	/* Decide on four or eight point compass names list. */
	vt_key[0].string = "Globals";
	vt_key[1].string = "EightPointCompass";
	eightpointcompass = prop_get_boolean(bundle, "B<-ss", vt_key);
	dirnames = eightpointcompass ? DIRNAMES_8 : DIRNAMES_4;

	/* Set invariant key for room exit search. */
	vt_key[0].string = "Rooms";
	vt_key[1].integer = room;
	vt_key[2].string = "Exits";

	/* Find the room exit that matches the NPC room. */
	found = FALSE;
	dir_match = 0;
	for (dir = 0; dirnames[dir]; dir++) {
		vt_key[3].integer = dir;
		if (prop_get(bundle, "I<-sisi", &vt_rvalue, vt_key)) {
			sc_int dest;

			/* Get room's direction destination, and compare. */
			vt_key[4].string = "Dest";
			dest = prop_get_integer(bundle, "I<-sisis", vt_key) - 1;
			if (dest == npc_room) {
				dir_match = dir;
				found = TRUE;
				break;
			}
		}
	}

	/* Print NPC exit/entry details. */
	pf_buffer_character(filter, '\n');
	pf_new_sentence(filter);
	pf_buffer_string(filter, name);
	pf_buffer_character(filter, ' ');
	pf_buffer_string(filter, text);
	if (found) {
		pf_buffer_string(filter, is_exit ? " to " : " from ");
		pf_buffer_string(filter, dirnames[dir_match]);
	}
	pf_buffer_string(filter, ".\n");

	/* Handle any associated resource. */
	vt_key[0].string = "NPCs";
	vt_key[1].integer = npc;
	vt_key[2].string = "Res";
	vt_key[3].integer = is_exit ? 3 : 2;
	res_handle_resource(game, "sisi", vt_key);
}


/*
 * npc_tick_npc_walk()
 *
 * Helper for npc_tick_npc().
 */
static void npc_tick_npc_walk(sc_gameref_t game, sc_int npc, sc_int walk) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[6];
	sc_int roomgroups, movetimes, walkstep, start, dest, destnum;
	sc_int chartask, objecttask;

	if (npc_trace) {
		sc_trace("NPC: ticking NPC %ld, walk %ld: step %ld\n",
		         npc, walk, gs_npc_walkstep(game, npc, walk));
	}

	/* Count roomgroups for later use. */
	vt_key[0].string = "RoomGroups";
	roomgroups = prop_get_child_count(bundle, "I<-s", vt_key);

	/* Get move times array length. */
	vt_key[0].string = "NPCs";
	vt_key[1].integer = npc;
	vt_key[2].string = "Walks";
	vt_key[3].integer = walk;
	vt_key[4].string = "MoveTimes";
	movetimes = prop_get_child_count(bundle, "I<-sisis", vt_key);

	/* Find a step to match the movetime. */
	for (walkstep = 0; walkstep < movetimes - 1; walkstep++) {
		sc_int  movetime;

		vt_key[5].integer = walkstep + 1;
		movetime = prop_get_integer(bundle, "I<-sisisi", vt_key);
		if (gs_npc_walkstep(game, npc, walk) > movetime)
			break;
	}

	/* Sort out a destination. */
	dest = start = gs_npc_location(game, npc) - 1;

	vt_key[4].string = "Rooms";
	vt_key[5].integer = walkstep;
	destnum = prop_get_integer(bundle, "I<-sisisi", vt_key);

	if (destnum == 0)          /* Hidden. */
		dest = -1;
	else if (destnum == 1)     /* Follow player. */
		dest = gs_playerroom(game);
	else if (destnum < gs_room_count(game) + 2)
		dest = destnum - 2;      /* To room. */
	else if (destnum < gs_room_count(game) + 2 + roomgroups) {
		sc_int initial;

		/* For roomgroup walks, move only if walksteps has just refreshed. */
		vt_key[4].string = "MoveTimes";
		vt_key[5].integer = 0;
		initial = prop_get_integer(bundle, "I<-sisisi", vt_key);
		if (gs_npc_walkstep(game, npc, walk) == initial) {
			sc_int group;

			group = destnum - 2 - gs_room_count(game);
			dest = npc_random_adjacent_roomgroup_member(game, start, group);
			if (dest == -1)
				dest = lib_random_roomgroup_member(game, group);
		}
	}

	/* See if the NPC actually moved. */
	if (start != dest) {
		if (npc_trace)
			sc_trace("NPC: walking NPC %ld moved to %ld\n", npc, dest);

		/* Move NPC to destination. */
		gs_set_npc_location(game, npc, dest + 1);

		/* Announce NPC movements, and handle meeting characters and objects. */
		if (gs_player_in_room(game, start))
			npc_announce(game, npc, start, TRUE, dest);
		else if (gs_player_in_room(game, dest))
			npc_announce(game, npc, dest, FALSE, start);
	}

	/* Handle meeting characters and objects. */
	vt_key[4].string = "CharTask";
	chartask = prop_get_integer(bundle, "I<-sisis", vt_key) - 1;
	if (chartask >= 0) {
		sc_int meetchar;

		/* Run meetchar task if appropriate. */
		vt_key[4].string = "MeetChar";
		meetchar = prop_get_integer(bundle, "I<-sisis", vt_key) - 1;
		if ((meetchar == -1 && gs_player_in_room(game, dest))
		        || (meetchar >= 0 && dest == gs_npc_location(game, meetchar) - 1)) {
			if (task_can_run_task(game, chartask))
				task_run_task(game, chartask, TRUE);
		}
	}

	vt_key[4].string = "ObjectTask";
	objecttask = prop_get_integer(bundle, "I<-sisis", vt_key) - 1;
	if (objecttask >= 0) {
		sc_int meetobject;

		/* Run meetobject task if appropriate. */
		vt_key[4].string = "MeetObject";
		meetobject = prop_get_integer(bundle, "I<-sisis", vt_key) - 1;
		if (meetobject >= 0 && obj_directly_in_room(game, meetobject, dest)) {
			if (task_can_run_task(game, objecttask))
				task_run_task(game, objecttask, TRUE);
		}
	}
}


/*
 * npc_tick_npc()
 *
 * Move an NPC one step along current walk.
 */
static void npc_tick_npc(sc_gameref_t game, sc_int npc) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[6];
	sc_int walk;
	sc_bool has_moved = FALSE;

	if (npc_trace)
		sc_trace("NPC: ticking NPC %ld\n", npc);

	/* Set up invariant key parts. */
	vt_key[0].string = "NPCs";
	vt_key[1].integer = npc;
	vt_key[2].string = "Walks";

	/* Find active walk, and if any found, make a step along it. */
	for (walk = gs_npc_walkstep_count(game, npc) - 1; walk >= 0; walk--) {
		sc_int starttask, stoppingtask;

		/* Ignore finished walks. */
		if (gs_npc_walkstep(game, npc, walk) <= 0)
			continue;

		/* Get start task. */
		vt_key[3].integer = walk;
		vt_key[4].string = "StartTask";
		starttask = prop_get_integer(bundle, "I<-sisis", vt_key) - 1;

		/*
		 * Check that the starter is still complete, and if not, stop walk.
		 * Then keep on looking for an active walk.
		 */
		if (starttask >= 0 && !gs_task_done(game, starttask)) {
			if (npc_trace)
				sc_trace("NPC: stopping NPC %ld walk, start task undone\n", npc);

			gs_set_npc_walkstep(game, npc, walk, -1);
			continue;
		}

		/* Get stopping task. */
		vt_key[4].string = "StoppingTask";
		stoppingtask = prop_get_integer(bundle, "I<-sisis", vt_key) - 1;

		/*
		 * If any stopping task has completed, ignore this walk but don't
		 * actually finish it; more like an event pauser, then.
		 *
		 * TODO Is this right?
		 */
		if (stoppingtask >= 0 && gs_task_done(game, stoppingtask)) {
			if (npc_trace)
				sc_trace("NPC: ignoring NPC %ld walk, stop task done\n", npc);

			continue;
		}

		/* Decrement steps. */
		gs_decrement_npc_walkstep(game, npc, walk);

		/* If we just hit a walk end, loop if called for. */
		if (gs_npc_walkstep(game, npc, walk) == 0) {
			sc_bool is_loop;

			/* If walk is a loop, restart it. */
			vt_key[4].string = "Loop";
			is_loop = prop_get_boolean(bundle, "B<-sisis", vt_key);
			if (is_loop) {
				vt_key[4].string = "MoveTimes";
				vt_key[5].integer = 0;
				gs_set_npc_walkstep(game, npc, walk,
				                    prop_get_integer(bundle,
				                                     "I<-sisisi", vt_key));
			} else
				gs_set_npc_walkstep(game, npc, walk, -1);
		}

		/*
		 * If not yet made a move on this walk, make one, and once made, make
		 * no other
		 */
		if (!has_moved) {
			npc_tick_npc_walk(game, npc, walk);
			has_moved = TRUE;
		}
	}
}


/*
 * npc_tick_npcs()
 *
 * Move each NPC one step along current walk.
 */
void npc_tick_npcs(sc_gameref_t game) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	const sc_gameref_t undo = game->undo;
	sc_int npc;

	/*
	 * Compare the player location to last turn, to see if the player has moved
	 * this turn.  If moved, look for meetings with NPCs.
	 *
	 * TODO Is this the right place to do this.  After ticking each NPC, rather
	 * than before, seems more appropriate.  But the messages come out in the
	 * right order by putting it here.
	 *
	 * Also, note that we take the shortcut of using the undo gamestate here,
	 * rather than properly recording the prior location of the player, and
	 * perhaps also NPCs, in the live gamestate.
	 */
	if (undo && !gs_player_in_room(undo, gs_playerroom(game))) {
		for (npc = 0; npc < gs_npc_count(game); npc++) {
			sc_int walk;

			/* Iterate each NPC's walks. */
			for (walk = gs_npc_walkstep_count(game, npc) - 1; walk >= 0; walk--) {
				sc_vartype_t vt_key[5];
				sc_int chartask;

				/* Ignore finished walks. */
				if (gs_npc_walkstep(game, npc, walk) <= 0)
					continue;

				/* Retrieve any character meeting task for the NPC. */
				vt_key[0].string = "NPCs";
				vt_key[1].integer = npc;
				vt_key[2].string = "Walks";
				vt_key[3].integer = walk;
				vt_key[4].string = "CharTask";
				chartask = prop_get_integer(bundle, "I<-sisis", vt_key) - 1;
				if (chartask >= 0) {
					sc_int meetchar;

					/* Run meetchar task if appropriate. */
					vt_key[4].string = "MeetChar";
					meetchar = prop_get_integer(bundle, "I<-sisis", vt_key) - 1;
					if (meetchar == -1 &&
					        gs_player_in_room(game, gs_npc_location(game, npc) - 1)) {
						if (task_can_run_task(game, chartask))
							task_run_task(game, chartask, TRUE);
					}
				}
			}
		}
	}

	/* Iterate and tick each individual NPC. */
	for (npc = 0; npc < gs_npc_count(game); npc++)
		npc_tick_npc(game, npc);
}


/*
 * npc_debug_trace()
 *
 * Set NPC tracing on/off.
 */
void npc_debug_trace(sc_bool flag) {
	npc_trace = flag;
}

} // End of namespace Adrift
} // End of namespace Glk
