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

/* Assorted definitions and constants. */
static const sc_uint GAME_MAGIC = 0x35aed26e;

/*
 * gs_move_player_to_room()
 * gs_player_in_room()
 *
 * Move the player to a given room, and check presence in a given room.
 */
void gs_move_player_to_room(sc_gameref_t game, sc_int room) {
	assert(gs_is_game_valid(game));

	if (room < 0) {
		sc_fatal("gs_move_player_to_room: invalid room, %ld\n", room);
		return;
	} else if (room < game->room_count)
		game->playerroom = room;
	else
		game->playerroom = lib_random_roomgroup_member(game,
		                   room - game->room_count);

	game->playerparent = -1;
	game->playerposition = 0;
}

sc_bool gs_player_in_room(sc_gameref_t game, sc_int room) {
	assert(gs_is_game_valid(game));
	return game->playerroom == room;
}


/*
 * gs_in_range()
 *
 * Helper for event, room, object, and npc range assertions.
 */
static sc_bool gs_in_range(sc_int value, sc_int limit) {
	return value >= 0 && value < limit;
}


/*
 * gs_*()
 *
 * Game accessors and mutators.
 */
sc_var_setref_t gs_get_vars(sc_gameref_t gs) {
	assert(gs_is_game_valid(gs));
	return gs->vars;
}

sc_prop_setref_t gs_get_bundle(sc_gameref_t gs) {
	assert(gs_is_game_valid(gs));
	return gs->bundle;
}

sc_filterref_t gs_get_filter(sc_gameref_t gs) {
	assert(gs_is_game_valid(gs));
	return gs->filter;
}

sc_memo_setref_t gs_get_memento(sc_gameref_t gs) {
	assert(gs_is_game_valid(gs));
	return gs->memento;
}


/*
 * Game accessors and mutators for the player.
 */
void gs_set_playerroom(sc_gameref_t gs, sc_int room) {
	assert(gs_is_game_valid(gs));
	gs->playerroom = room;
}

void gs_set_playerposition(sc_gameref_t gs, sc_int position) {
	assert(gs_is_game_valid(gs));
	gs->playerposition = position;
}

void gs_set_playerparent(sc_gameref_t gs, sc_int parent) {
	assert(gs_is_game_valid(gs));
	gs->playerparent = parent;
}

sc_int gs_playerroom(sc_gameref_t gs) {
	assert(gs_is_game_valid(gs));
	return gs->playerroom;
}

sc_int gs_playerposition(sc_gameref_t gs) {
	assert(gs_is_game_valid(gs));
	return gs->playerposition;
}

sc_int gs_playerparent(sc_gameref_t gs) {
	assert(gs_is_game_valid(gs));
	return gs->playerparent;
}


/*
 * Game accessors and mutators for events.
 */
sc_int gs_event_count(sc_gameref_t gs) {
	assert(gs_is_game_valid(gs));
	return gs->event_count;
}

void gs_set_event_state(sc_gameref_t gs, sc_int event, sc_int state) {
	assert(gs_is_game_valid(gs) && gs_in_range(event, gs->event_count));
	gs->events[event].state = state;
}

void gs_set_event_time(sc_gameref_t gs, sc_int event, sc_int etime) {
	assert(gs_is_game_valid(gs) && gs_in_range(event, gs->event_count));
	gs->events[event].time = etime;
}

sc_int gs_event_state(sc_gameref_t gs, sc_int event) {
	assert(gs_is_game_valid(gs) && gs_in_range(event, gs->event_count));
	return gs->events[event].state;
}

sc_int gs_event_time(sc_gameref_t gs, sc_int event) {
	assert(gs_is_game_valid(gs) && gs_in_range(event, gs->event_count));
	return gs->events[event].time;
}

void gs_decrement_event_time(sc_gameref_t gs, sc_int event) {
	assert(gs_is_game_valid(gs) && gs_in_range(event, gs->event_count));
	gs->events[event].time--;
}


/*
 * Game accessors and mutators for rooms.
 */
sc_int gs_room_count(sc_gameref_t gs) {
	assert(gs_is_game_valid(gs));
	return gs->room_count;
}

void gs_set_room_seen(sc_gameref_t gs, sc_int room, sc_bool seen) {
	assert(gs_is_game_valid(gs) && gs_in_range(room, gs->room_count));
	gs->rooms[room].visited = seen;
}

sc_bool gs_room_seen(sc_gameref_t gs, sc_int room) {
	assert(gs_is_game_valid(gs) && gs_in_range(room, gs->room_count));
	return gs->rooms[room].visited;
}


/*
 * Game accessors and mutators for tasks.
 */
sc_int gs_task_count(sc_gameref_t gs) {
	assert(gs_is_game_valid(gs));
	return gs->task_count;
}

void gs_set_task_done(sc_gameref_t gs, sc_int task, sc_bool done) {
	assert(gs_is_game_valid(gs) && gs_in_range(task, gs->task_count));
	gs->tasks[task].done = done;
}

void gs_set_task_scored(sc_gameref_t gs, sc_int task, sc_bool scored) {
	assert(gs_is_game_valid(gs) && gs_in_range(task, gs->task_count));
	gs->tasks[task].scored = scored;
}

sc_bool gs_task_done(sc_gameref_t gs, sc_int task) {
	assert(gs_is_game_valid(gs) && gs_in_range(task, gs->task_count));
	return gs->tasks[task].done;
}

sc_bool gs_task_scored(sc_gameref_t gs, sc_int task) {
	assert(gs_is_game_valid(gs) && gs_in_range(task, gs->task_count));
	return gs->tasks[task].scored;
}


/*
 * Game accessors and mutators for objects.
 */
sc_int gs_object_count(sc_gameref_t gs) {
	assert(gs_is_game_valid(gs));
	return gs->object_count;
}

void gs_set_object_openness(sc_gameref_t gs, sc_int object, sc_int openness) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	gs->objects[object].openness = openness;
}

void gs_set_object_state(sc_gameref_t gs, sc_int object, sc_int state) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	gs->objects[object].state = state;
}

void gs_set_object_seen(sc_gameref_t gs, sc_int object, sc_bool seen) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	gs->objects[object].seen = seen;
}

void gs_set_object_unmoved(sc_gameref_t gs, sc_int object, sc_bool unmoved) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	gs->objects[object].unmoved = unmoved;
}

void gs_set_object_static_unmoved(sc_gameref_t gs, sc_int object, sc_bool unmoved) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	gs->objects[object].static_unmoved = unmoved;
}

sc_int gs_object_openness(sc_gameref_t gs, sc_int object) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	return gs->objects[object].openness;
}

sc_int gs_object_state(sc_gameref_t gs, sc_int object) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	return gs->objects[object].state;
}

sc_bool gs_object_seen(sc_gameref_t gs, sc_int object) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	return gs->objects[object].seen;
}

sc_bool gs_object_unmoved(sc_gameref_t gs, sc_int object) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	return gs->objects[object].unmoved;
}

sc_bool gs_object_static_unmoved(sc_gameref_t gs, sc_int object) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	return gs->objects[object].static_unmoved;
}

sc_int gs_object_position(sc_gameref_t gs, sc_int object) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	return gs->objects[object].position;
}

sc_int gs_object_parent(sc_gameref_t gs, sc_int object) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	return gs->objects[object].parent;
}

static void gs_object_move_onto_unchecked(sc_gameref_t gs, sc_int object, sc_int onto) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	gs->objects[object].position = OBJ_ON_OBJECT;
	gs->objects[object].parent = onto;
}

static void gs_object_move_into_unchecked(sc_gameref_t gs, sc_int object, sc_int into) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	gs->objects[object].position = OBJ_IN_OBJECT;
	gs->objects[object].parent = into;
}

static void gs_object_make_hidden_unchecked(sc_gameref_t gs, sc_int object) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	gs->objects[object].position = OBJ_HIDDEN;
	gs->objects[object].parent = -1;
}

static void gs_object_player_get_unchecked(sc_gameref_t gs, sc_int object) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	gs->objects[object].position = OBJ_HELD_PLAYER;
	gs->objects[object].parent = -1;
}

static void gs_object_npc_get_unchecked(sc_gameref_t gs, sc_int object, sc_int npc) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	gs->objects[object].position = OBJ_HELD_NPC;
	gs->objects[object].parent = npc;
}

static void gs_object_player_wear_unchecked(sc_gameref_t gs, sc_int object) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	gs->objects[object].position = OBJ_WORN_PLAYER;
	gs->objects[object].parent = 0;
}

static void gs_object_npc_wear_unchecked(sc_gameref_t gs, sc_int object, sc_int npc) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	gs->objects[object].position = OBJ_WORN_NPC;
	gs->objects[object].parent = npc;
}

static void gs_object_to_room_unchecked(sc_gameref_t gs, sc_int object, sc_int room) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	gs->objects[object].position = room + 1;
	gs->objects[object].parent = -1;
}

void gs_object_move_onto(sc_gameref_t gs, sc_int object, sc_int onto) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	if (gs->objects[object].position != OBJ_ON_OBJECT
	        || gs->objects[object].parent != onto) {
		gs_object_move_onto_unchecked(gs, object, onto);
		gs->objects[object].unmoved = FALSE;
	}
}

void gs_object_move_into(sc_gameref_t gs, sc_int object, sc_int into) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	if (gs->objects[object].position != OBJ_IN_OBJECT
	        || gs->objects[object].parent != into) {
		gs_object_move_into_unchecked(gs, object, into);
		gs->objects[object].unmoved = FALSE;
	}
}

void gs_object_make_hidden(sc_gameref_t gs, sc_int object) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	if (gs->objects[object].position != OBJ_HIDDEN) {
		gs_object_make_hidden_unchecked(gs, object);
		gs->objects[object].unmoved = FALSE;
	}
}

void gs_object_player_get(sc_gameref_t gs, sc_int object) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	if (gs->objects[object].position != OBJ_HELD_PLAYER) {
		gs_object_player_get_unchecked(gs, object);
		gs->objects[object].unmoved = FALSE;
	}
}

void gs_object_npc_get(sc_gameref_t gs, sc_int object, sc_int npc) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	if (gs->objects[object].position != OBJ_HELD_NPC
	        || gs->objects[object].parent != npc) {
		gs_object_npc_get_unchecked(gs, object, npc);
		gs->objects[object].unmoved = FALSE;
	}
}

void gs_object_player_wear(sc_gameref_t gs, sc_int object) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	if (gs->objects[object].position != OBJ_WORN_PLAYER) {
		gs_object_player_wear_unchecked(gs, object);
		gs->objects[object].unmoved = FALSE;
	}
}

void gs_object_npc_wear(sc_gameref_t gs, sc_int object, sc_int npc) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	if (gs->objects[object].position != OBJ_WORN_NPC
	        || gs->objects[object].parent != npc) {
		gs_object_npc_wear_unchecked(gs, object, npc);
		gs->objects[object].unmoved = FALSE;
	}
}

void gs_object_to_room(sc_gameref_t gs, sc_int object, sc_int room) {
	assert(gs_is_game_valid(gs) && gs_in_range(object, gs->object_count));
	if (gs->objects[object].position != room + 1) {
		gs_object_to_room_unchecked(gs, object, room);
		gs->objects[object].unmoved = FALSE;
	}
}


/*
 * Game accessors and mutators for NPCs.
 */
sc_int gs_npc_count(sc_gameref_t gs) {
	assert(gs_is_game_valid(gs));
	return gs->npc_count;
}

void gs_set_npc_location(sc_gameref_t gs, sc_int npc, sc_int location) {
	assert(gs_is_game_valid(gs) && gs_in_range(npc, gs->npc_count));
	gs->npcs[npc].location = location;
}

sc_int gs_npc_location(sc_gameref_t gs, sc_int npc) {
	assert(gs_is_game_valid(gs) && gs_in_range(npc, gs->npc_count));
	return gs->npcs[npc].location;
}

void gs_set_npc_position(sc_gameref_t gs, sc_int npc, sc_int position) {
	assert(gs_is_game_valid(gs) && gs_in_range(npc, gs->npc_count));
	gs->npcs[npc].position = position;
}

sc_int gs_npc_position(sc_gameref_t gs, sc_int npc) {
	assert(gs_is_game_valid(gs) && gs_in_range(npc, gs->npc_count));
	return gs->npcs[npc].position;
}

void gs_set_npc_parent(sc_gameref_t gs, sc_int npc, sc_int parent) {
	assert(gs_is_game_valid(gs) && gs_in_range(npc, gs->npc_count));
	gs->npcs[npc].parent = parent;
}

sc_int gs_npc_parent(sc_gameref_t gs, sc_int npc) {
	assert(gs_is_game_valid(gs) && gs_in_range(npc, gs->npc_count));
	return gs->npcs[npc].parent;
}

void gs_set_npc_seen(sc_gameref_t gs, sc_int npc, sc_bool seen) {
	assert(gs_is_game_valid(gs) && gs_in_range(npc, gs->npc_count));
	gs->npcs[npc].seen = seen;
}

sc_bool gs_npc_seen(sc_gameref_t gs, sc_int npc) {
	assert(gs_is_game_valid(gs) && gs_in_range(npc, gs->npc_count));
	return gs->npcs[npc].seen;
}

sc_int gs_npc_walkstep_count(sc_gameref_t gs, sc_int npc) {
	assert(gs_is_game_valid(gs) && gs_in_range(npc, gs->npc_count));
	return gs->npcs[npc].walkstep_count;
}

void gs_set_npc_walkstep(sc_gameref_t gs,
                    sc_int npc, sc_int walk, sc_int walkstep) {
	assert(gs_is_game_valid(gs) && gs_in_range(npc, gs->npc_count)
	       && gs_in_range(walk, gs->npcs[npc].walkstep_count));
	gs->npcs[npc].walksteps[walk] = walkstep;
}

sc_int gs_npc_walkstep(sc_gameref_t gs, sc_int npc, sc_int walk) {
	assert(gs_is_game_valid(gs) && gs_in_range(npc, gs->npc_count)
	       && gs_in_range(walk, gs->npcs[npc].walkstep_count));
	return gs->npcs[npc].walksteps[walk];
}

void gs_decrement_npc_walkstep(sc_gameref_t gs, sc_int npc, sc_int walk) {
	assert(gs_is_game_valid(gs) && gs_in_range(npc, gs->npc_count)
	       && gs_in_range(walk, gs->npcs[npc].walkstep_count));
	gs->npcs[npc].walksteps[walk]--;
}


/*
 * Convenience functions for bulk clearance of references.
 */
void gs_clear_npc_references(sc_gameref_t gs) {
	assert(gs_is_game_valid(gs));
	memset(gs->npc_references,
	       FALSE, gs->npc_count * sizeof(*gs->npc_references));
}

void gs_clear_object_references(sc_gameref_t gs) {
	assert(gs_is_game_valid(gs));
	memset(gs->object_references,
	       FALSE, gs->object_count * sizeof(*gs->object_references));
}

void gs_set_multiple_references(sc_gameref_t gs) {
	assert(gs_is_game_valid(gs));
	memset(gs->multiple_references,
	       TRUE, gs->object_count * sizeof(*gs->multiple_references));
}

void gs_clear_multiple_references(sc_gameref_t gs) {
	assert(gs_is_game_valid(gs));
	memset(gs->multiple_references,
	       FALSE, gs->object_count * sizeof(*gs->multiple_references));
}


/*
 * gs_create()
 *
 * Create and initialize a game state.
 */
sc_gameref_t gs_create(sc_var_setref_t vars, sc_prop_setref_t bundle, sc_filterref_t filter) {
	sc_gameref_t game;
	sc_vartype_t vt_key[4];
	sc_int index_, bytes;
	assert(vars && bundle && filter);

	/* Create the initial state structure. */
	game = (sc_gameref_t)sc_malloc(sizeof(*game));
	game->magic = GAME_MAGIC;

	/* Store the variables, properties bundle, and filter references. */
	game->vars = vars;
	game->bundle = bundle;
	game->filter = filter;

	/* Set memento to NULL for now; it's added later. */
	game->memento = NULL;

	/* Initialize for no debugger. */
	game->debugger = NULL;

	/* Initialize the undo buffers to NULL for now. */
	game->temporary = NULL;
	game->undo = NULL;
	game->undo_available = FALSE;

	/* Create rooms state array. */
	vt_key[0].string = "Rooms";
	game->room_count = prop_get_child_count(bundle, "I<-s", vt_key);
	game->rooms = (sc_roomstate_t *)sc_malloc(game->room_count * sizeof(*game->rooms));

	/* Set up initial rooms states. */
	for (index_ = 0; index_ < game->room_count; index_++)
		gs_set_room_seen(game, index_, FALSE);

	/* Create objects state array. */
	vt_key[0].string = "Objects";
	game->object_count = prop_get_child_count(bundle, "I<-s", vt_key);
	game->objects = (sc_objectstate_t *)sc_malloc(game->object_count * sizeof(*game->objects));

	/* Set up initial object states. */
	for (index_ = 0; index_ < game->object_count; index_++) {
		const sc_char *inroomdesc;
		sc_bool is_static, unmoved;

		vt_key[1].integer = index_;

		vt_key[2].string = "Static";
		is_static = prop_get_boolean(bundle, "B<-sis", vt_key);
		if (is_static) {
			sc_int type;

			vt_key[2].string = "Where";
			vt_key[3].string = "Type";
			type = prop_get_integer(bundle, "I<-siss", vt_key);
			if (type == ROOMLIST_NPC_PART) {
				sc_int parent;

				game->objects[index_].position = OBJ_PART_NPC;

				vt_key[2].string = "Parent";
				parent = prop_get_integer(bundle, "I<-sis", vt_key) - 1;
				game->objects[index_].parent = parent;
			} else
				gs_object_make_hidden_unchecked(game, index_);
		} else {
			sc_int initialparent, initialposition;

			vt_key[2].string = "Parent";
			initialparent = prop_get_integer(bundle, "I<-sis", vt_key);
			vt_key[2].string = "InitialPosition";
			initialposition = prop_get_integer(bundle, "I<-sis", vt_key);
			switch (initialposition) {
			case 0:            /* Hidden. */
				gs_object_make_hidden_unchecked(game, index_);
				break;

			case 1:            /* Held. */
				if (initialparent == 0)   /* By player. */
					gs_object_player_get_unchecked(game, index_);
				else                      /* By NPC. */
					gs_object_npc_get_unchecked(game, index_, initialparent - 1);
				break;

			case 2:            /* In container. */
				gs_object_move_into_unchecked(game, index_,
				                              obj_container_object(game, initialparent));
				break;

			case 3:            /* On surface. */
				gs_object_move_onto_unchecked(game, index_,
				                              obj_surface_object(game, initialparent));
				break;

			default:           /* In room, or worn by player/NPC. */
				if (initialposition >= 4
				        && initialposition < 4 + game->room_count) {
					gs_object_to_room_unchecked(game,
					                            index_, initialposition - 4);
				} else if (initialposition == 4 + game->room_count) {
					if (initialparent == 0)
						gs_object_player_wear_unchecked(game, index_);
					else
						gs_object_npc_wear_unchecked(game,
						                             index_, initialparent - 1);
				} else {
					sc_error("gs_create: object in out of bounds room, %ld\n",
					         initialposition - 4 - game->room_count);
					gs_object_to_room_unchecked(game, index_, -2);
				}
			}
		}

		vt_key[2].string = "CurrentState";
		gs_set_object_state(game, index_,
		                    prop_get_integer(bundle, "I<-sis", vt_key));

		vt_key[2].string = "Openable";
		gs_set_object_openness(game, index_,
		                       prop_get_integer(bundle, "I<-sis", vt_key));

		gs_set_object_seen(game, index_, FALSE);

		vt_key[2].string = "InRoomDesc";
		inroomdesc = prop_get_string(bundle, "S<-sis", vt_key);
		if (!sc_strempty(inroomdesc)) {
			vt_key[2].string = "OnlyWhenNotMoved";
			if (prop_get_integer(bundle, "I<-sis", vt_key) == 1)
				unmoved = TRUE;
			else
				unmoved = FALSE;
		} else
			unmoved = FALSE;
		gs_set_object_unmoved(game, index_, unmoved);
		gs_set_object_static_unmoved(game, index_, TRUE);
	}

	/* Create tasks state array. */
	vt_key[0].string = "Tasks";
	game->task_count = prop_get_child_count(bundle, "I<-s", vt_key);
	game->tasks = (sc_taskstate_t *)sc_malloc(game->task_count * sizeof(*game->tasks));

	/* Set up initial tasks states. */
	for (index_ = 0; index_ < game->task_count; index_++) {
		gs_set_task_done(game, index_, FALSE);
		gs_set_task_scored(game, index_, FALSE);
	}

	/* Create events state array. */
	vt_key[0].string = "Events";
	game->event_count = prop_get_child_count(bundle, "I<-s", vt_key);
	game->events = (sc_eventstate_t *)sc_malloc(game->event_count * sizeof(*game->events));

	/* Set up initial events states. */
	for (index_ = 0; index_ < game->event_count; index_++) {
		sc_int startertype;

		vt_key[1].integer = index_;
		vt_key[2].string = "StarterType";
		startertype = prop_get_integer(bundle, "I<-sis", vt_key);

		switch (startertype) {
		case 1:
			gs_set_event_state(game, index_, ES_WAITING);
			gs_set_event_time(game, index_, 0);
			break;

		case 2: {
			sc_int start, end;

			gs_set_event_state(game, index_, ES_WAITING);
			vt_key[2].string = "StartTime";
			start = prop_get_integer(bundle, "I<-sis", vt_key);
			vt_key[2].string = "EndTime";
			end = prop_get_integer(bundle, "I<-sis", vt_key);
			gs_set_event_time(game, index_, sc_randomint(start, end));
			break;
		}

		case 3:
			gs_set_event_state(game, index_, ES_AWAITING);
			gs_set_event_time(game, index_, 0);
			break;

		default:
			break;
		}
	}

	/* Create NPCs state array. */
	vt_key[0].string = "NPCs";
	game->npc_count = prop_get_child_count(bundle, "I<-s", vt_key);
	game->npcs = (sc_npcstate_t *)sc_malloc(game->npc_count * sizeof(*game->npcs));

	/* Set up initial NPCs states. */
	for (index_ = 0; index_ < game->npc_count; index_++) {
		sc_int walk, walkstep_count;

		gs_set_npc_position(game, index_, 0);
		gs_set_npc_parent(game, index_, -1);
		gs_set_npc_seen(game, index_, FALSE);

		vt_key[1].integer = index_;

		vt_key[2].string = "StartRoom";
		gs_set_npc_location(game, index_,
		                    prop_get_integer(bundle, "I<-sis", vt_key));

		vt_key[2].string = "Walks";
		walkstep_count = prop_get_child_count(bundle, "I<-sis", vt_key);

		game->npcs[index_].walkstep_count = walkstep_count;
		game->npcs[index_].walksteps = (sc_int *)sc_malloc(walkstep_count
		                               * sizeof(*game->npcs[0].walksteps));

		for (walk = 0; walk < walkstep_count; walk++)
			gs_set_npc_walkstep(game, index_, walk, 0);
	}

	/* Set up the player portions of the game state. */
	vt_key[0].string = "Header";
	vt_key[1].string = "StartRoom";
	game->playerroom = prop_get_integer(bundle, "I<-ss", vt_key);
	vt_key[0].string = "Globals";
	vt_key[1].string = "ParentObject";
	game->playerparent = prop_get_integer(bundle, "I<-ss", vt_key) - 1;
	vt_key[1].string = "Position";
	game->playerposition = prop_get_integer(bundle, "I<-ss", vt_key);

	/* Initialize score notifications from game properties. */
	vt_key[0].string = "Globals";
	vt_key[1].string = "NoScoreNotify";
	game->notify_score_change = !prop_get_boolean(bundle, "B<-ss", vt_key);

	/* Miscellaneous state defaults. */
	game->turns = 0;
	game->score = 0;
	game->bold_room_names = TRUE;
	game->verbose = FALSE;
	game->current_room_name = NULL;
	game->status_line = NULL;
	game->title = NULL;
	game->author = NULL;
	game->hint_text = NULL;

	/* Resource controls. */
	res_clear_resource(&game->requested_sound);
	res_clear_resource(&game->requested_graphic);
	res_clear_resource(&game->playing_sound);
	res_clear_resource(&game->displayed_graphic);
	game->stop_sound = FALSE;
	game->sound_active = FALSE;

	/* Initialize wait turns from game properties. */
	vt_key[0].string = "Globals";
	vt_key[1].string = "WaitTurns";
	game->waitturns = prop_get_integer(bundle, "I<-ss", vt_key);

	/* Non-game conveniences. */
	game->is_running = FALSE;
	game->has_notified = FALSE;
	game->is_admin = FALSE;
	game->has_completed = FALSE;
	game->waitcounter = 0;
	game->do_again = FALSE;
	game->redo_sequence = 0;
	game->do_restart = FALSE;
	game->do_restore = FALSE;

	bytes = game->object_count * sizeof(*game->object_references);
	game->object_references = (sc_bool *)sc_malloc(bytes);
	memset(game->object_references, FALSE, bytes);
	bytes = game->object_count * sizeof(*game->multiple_references);
	game->multiple_references = (sc_bool *)sc_malloc(bytes);
	memset(game->multiple_references, FALSE, bytes);

	bytes = game->npc_count * sizeof(*game->npc_references);
	game->npc_references = (sc_bool *)sc_malloc(bytes);
	memset(game->npc_references, FALSE, bytes);

	game->it_object = -1;
	game->him_npc = -1;
	game->her_npc = -1;
	game->it_npc = -1;

	// Return the constructed game state
	return game;
}


/*
 * gs_is_game_valid()
 *
 * Return TRUE if pointer is a valid game, FALSE otherwise.
 */
sc_bool gs_is_game_valid(const sc_gameref_t game) {
	return game && game->magic == GAME_MAGIC;
}


/*
 * gs_string_copy()
 *
 * Helper for gs_copy(), copies one malloc'ed string to another, or NULL
 * if from is NULL, taking care not to leak memory.
 */
static void gs_string_copy(sc_char **to_string, const sc_char *from_string) {
	/* Free any current contents of to_string. */
	sc_free(*to_string);

	/* Copy from_string if set, otherwise set to_string to NULL. */
	if (from_string) {
		*to_string = (sc_char *)sc_malloc(strlen(from_string) + 1);
		strcpy(*to_string, from_string);
	} else
		*to_string = NULL;
}


/*
 * gs_copy()
 *
 * Deep-copy the dynamic parts of a game onto another existing
 * game structure.
 */
void gs_copy(sc_gameref_t to, sc_gameref_t from) {
	const sc_prop_setref_t bundle = from->bundle;
	sc_vartype_t vt_key[3];
	sc_int var_count, var, npc;
	assert(gs_is_game_valid(to) && gs_is_game_valid(from));

	/*
	 * Copy over references to the properties bundle and filter.  The debugger
	 * is specifically excluded, as it's considered to be tied to the game.
	 */
	to->bundle = from->bundle;
	to->filter = from->filter;

	/* Copy over references to the undo buffers. */
	to->temporary = from->temporary;
	to->undo = from->undo;
	to->undo_available = from->undo_available;

	/* Copy over all variables values. */
	vt_key[0].string = "Variables";
	var_count = prop_get_child_count(bundle, "I<-s", vt_key);

	for (var = 0; var < var_count; var++) {
		const sc_char *name;
		sc_int var_type;

		vt_key[1].integer = var;

		vt_key[2].string = "Name";
		name = prop_get_string(bundle, "S<-sis", vt_key);
		vt_key[2].string = "Type";
		var_type = prop_get_integer(bundle, "I<-sis", vt_key);

		switch (var_type) {
		case TAFVAR_NUMERIC:
			var_put_integer(to->vars, name, var_get_integer(from->vars, name));
			break;

		case TAFVAR_STRING:
			var_put_string(to->vars, name, var_get_string(from->vars, name));
			break;

		default:
			sc_fatal("gs_copy: unknown variable type, %ld\n", var_type);
		}
	}

	/* Copy over the variable timestamp. */
	var_set_elapsed_seconds(to->vars, var_get_elapsed_seconds(from->vars));

	/* Copy over room states. */
	assert(to->room_count == from->room_count);
	memcpy(to->rooms, from->rooms, from->room_count * sizeof(*from->rooms));

	/* Copy over object states. */
	assert(to->object_count == from->object_count);
	memcpy(to->objects, from->objects,
	       from->object_count * sizeof(*from->objects));

	/* Copy over task states. */
	assert(to->task_count == from->task_count);
	memcpy(to->tasks, from->tasks, from->task_count * sizeof(*from->tasks));

	/* Copy over event states. */
	assert(to->event_count == from->event_count);
	memcpy(to->events, from->events, from->event_count * sizeof(*from->events));

	/* Copy over NPC states individually, to avoid walks problems. */
	for (npc = 0; npc < from->npc_count; npc++) {
		to->npcs[npc].location = from->npcs[npc].location;
		to->npcs[npc].position = from->npcs[npc].position;
		to->npcs[npc].parent = from->npcs[npc].parent;
		to->npcs[npc].seen = from->npcs[npc].seen;
		to->npcs[npc].walkstep_count = from->npcs[npc].walkstep_count;

		/* Copy over NPC walks information. */
		assert(to->npcs[npc].walkstep_count == from->npcs[npc].walkstep_count);
		memcpy(to->npcs[npc].walksteps, from->npcs[npc].walksteps,
		       from->npcs[npc].walkstep_count
		       * sizeof(*from->npcs[npc].walksteps));
	}

	/* Copy over player information. */
	to->playerroom = from->playerroom;
	to->playerposition = from->playerposition;
	to->playerparent = from->playerparent;

	/*
	 * Copy over miscellaneous other details.  Specifically exclude bold rooms,
	 * verbose, and score notification, so that they are invariant across copies,
	 * particularly undo/restore.
	 */
	to->turns = from->turns;
	to->score = from->score;

	gs_string_copy(&to->current_room_name, from->current_room_name);
	gs_string_copy(&to->status_line, from->status_line);
	gs_string_copy(&to->title, from->title);
	gs_string_copy(&to->author, from->author);
	gs_string_copy(&to->hint_text, from->hint_text);

	/*
	 * Specifically exclude playing sound and displayed graphic from the copy
	 * so that they remain invariant across game copies.
	 */
	to->requested_sound = from->requested_sound;
	to->requested_graphic = from->requested_graphic;
	to->stop_sound = from->stop_sound;

	to->is_running = from->is_running;
	to->has_notified = from->has_notified;
	to->is_admin = from->is_admin;
	to->has_completed = from->has_completed;

	to->waitturns = from->waitturns;

	to->waitcounter = from->waitcounter;
	to->do_again = from->do_again;
	to->redo_sequence = from->redo_sequence;
	to->do_restart = from->do_restart;
	to->do_restore = from->do_restore;

	memcpy(to->object_references, from->object_references,
	       from->object_count * sizeof(*from->object_references));
	memcpy(to->multiple_references, from->multiple_references,
	       from->object_count * sizeof(*from->multiple_references));
	memcpy(to->npc_references, from->npc_references,
	       from->npc_count * sizeof(*from->npc_references));

	to->it_object = from->it_object;
	to->him_npc = from->him_npc;
	to->her_npc = from->her_npc;
	to->it_npc = from->it_npc;
}


/*
 * gs_destroy()
 *
 * Free all the memory associated with a game state.
 */
void gs_destroy(sc_gameref_t game) {
	sc_int npc;
	assert(gs_is_game_valid(game));

	/* Free the malloc'ed state arrays. */
	sc_free(game->rooms);
	sc_free(game->objects);
	sc_free(game->tasks);
	sc_free(game->events);
	for (npc = 0; npc < game->npc_count; npc++)
		sc_free(game->npcs[npc].walksteps);
	sc_free(game->npcs);

	/* Free the malloc'ed object and NPC references. */
	sc_free(game->object_references);
	sc_free(game->multiple_references);
	sc_free(game->npc_references);

	/* Free malloc'ed game strings. */
	sc_free(game->current_room_name);
	sc_free(game->status_line);
	sc_free(game->title);
	sc_free(game->author);
	sc_free(game->hint_text);

	/* Poison and free the game state itself. */
	memset(game, 0xaa, sizeof(*game));
	sc_free(game);
}

} // End of namespace Adrift
} // End of namespace Glk
