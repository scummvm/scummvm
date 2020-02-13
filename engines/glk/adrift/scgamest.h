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

#ifndef ADRIFT_GAMESTATE_H
#define ADRIFT_GAMESTATE_H

namespace Glk {
namespace Adrift {

/* Room state structure, tracks rooms visited by the player. */
struct sc_roomstate_s {
	sc_bool visited;
};
typedef sc_roomstate_s sc_roomstate_t;

/*
 * Object state structure, tracks object movement, position, parent, openness
 * for openable objects, state for stateful objects, and whether seen or not
 * by the player.  The enumerations are values assigned to position when the
 * object is other than just "in a room"; otherwise position contains the
 * room number + 1.
 */
enum {
	OBJ_HIDDEN = -1, OBJ_HELD_PLAYER = 0, OBJ_HELD_NPC = -200, OBJ_WORN_PLAYER = -100,
	OBJ_WORN_NPC = -300, OBJ_PART_PLAYER = -30, OBJ_PART_NPC = -30, OBJ_ON_OBJECT = -20,
	OBJ_IN_OBJECT = -10
};
struct sc_objectstate_s {
	sc_int position;
	sc_int parent;
	sc_int openness;
	sc_int state;
	sc_bool seen;
	sc_bool unmoved;
	sc_bool static_unmoved;
};
typedef sc_objectstate_s sc_objectstate_t;

/* Task state structure, tracks task done, and if task scored. */
struct sc_taskstate_s {
	sc_bool done;
	sc_bool scored;
};

typedef sc_taskstate_s sc_taskstate_t;

/* Event state structure, holds event state, and timing information. */
enum {
	ES_WAITING = 1,
	ES_RUNNING = 2, ES_AWAITING = 3, ES_FINISHED = 4, ES_PAUSED = 5
};
struct sc_eventstate_s {
	sc_int state;
	sc_int time;
};
typedef sc_eventstate_s sc_eventstate_t;

/*
 * NPC state structure, tracks the NPC location and position, any parent
 * object, whether the NPC seen, and if the NPC walks, the count of walk
 * steps and a steps array sized to this count.
 */
struct sc_npcstate_s {
	sc_int location;
	sc_int position;
	sc_int parent;
	sc_int walkstep_count;
	sc_int *walksteps;
	sc_bool seen;
};
typedef sc_npcstate_s sc_npcstate_t;

/*
 * Resource tracking structure, holds the resource name, including any
 * trailing "##" for looping sounds, its offset into the game file, and its
 * length.  Two resources are held -- active, and requested.  The game main
 * loop compares the two, and notifies the interface on a change.
 */
struct sc_resource_s {
	const sc_char *name;
	sc_int offset;
	sc_int length;
};
typedef sc_resource_s sc_resource_t;

/*
 * Overall game state structure.  Arrays are malloc'ed for the appropriate
 * number of each of the above state structures.
 */
struct sc_game_s {
	sc_uint magic;

	/* References to assorted helper subsystems. */
	sc_var_setref_t vars;
	sc_prop_setref_t bundle;
	sc_filterref_t filter;
	sc_memo_setref_t memento;
	sc_debuggerref_t debugger;

	/* Undo information, also used by the debugger. */
	struct sc_game_s *temporary;
	struct sc_game_s *undo;
	sc_bool undo_available;

	/* Basic game state -- rooms, objects, and so on. */
	sc_int room_count;
	sc_roomstate_t *rooms;
	sc_int object_count;
	sc_objectstate_t *objects;
	sc_int task_count;
	sc_taskstate_t *tasks;
	sc_int event_count;
	sc_eventstate_t *events;
	sc_int npc_count;
	sc_npcstate_t *npcs;
	sc_int playerroom;
	sc_int playerposition;
	sc_int playerparent;
	sc_int turns;
	sc_int score;
	sc_bool bold_room_names;
	sc_bool verbose;
	sc_bool notify_score_change;
	sc_char *current_room_name;
	sc_char *status_line;
	sc_char *title;
	sc_char *author;
	sc_char *hint_text;

	/* Resource management data. */
	sc_resource_t requested_sound;
	sc_resource_t requested_graphic;
	sc_bool stop_sound;
	sc_bool sound_active;

	sc_resource_t playing_sound;
	sc_resource_t displayed_graphic;

	/* Game running and game completed flags. */
	sc_bool is_running;
	sc_bool has_completed;

	/* Player's setting for waitturns; overrides the game's. */
	sc_int waitturns;

	/* Miscellaneous library and main loop conveniences. */
	sc_int waitcounter;
	sc_bool has_notified;
	sc_bool is_admin;
	sc_bool do_again;
	sc_int redo_sequence;
	sc_bool do_restart;
	sc_bool do_restore;
	sc_bool *object_references;
	sc_bool *multiple_references;
	sc_bool *npc_references;
	sc_int it_object;
	sc_int him_npc;
	sc_int her_npc;
	sc_int it_npc;
};
typedef sc_game_s sc_game_t;

} // End of namespace Adrift
} // End of namespace Glk

#endif
