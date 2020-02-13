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

/*
 * Module notes:
 *
 * o Implements task return FALSE on no output, a slight extension of
 *   current jAsea behavior.
 */

/*
 * Tasks can run other tasks, leading to the possibility of an infinite loop
 * in the task calling sequence.  It's a game error, and we'll apply a limit
 * to the task recursion depth to try and catch it more controllably than
 * waiting for memory exhaustion.
 */
enum { TASK_MAXIMUM_RECURSION = 128 };

/* Trace flag, set before running. */
static sc_bool task_trace = FALSE;


/*
 * task_get_hint_common()
 * task_get_hint_question()
 * task_get_hint_subtle()
 * task_get_hint_unsubtle()
 * task_has_hints()
 *
 * Return the assorted hint text strings, and TRUE if the given task offers
 * hints.
 */
static const sc_char *task_get_hint_common(sc_gameref_t game, sc_int task, const sc_char *hint) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	const sc_char *retval;

	/* Look up and return the requested hint string. */
	vt_key[0].string = "Tasks";
	vt_key[1].integer = task;
	vt_key[2].string = hint;
	retval = prop_get_string(bundle, "S<-sis", vt_key);
	return retval;
}

const sc_char *task_get_hint_question(sc_gameref_t game, sc_int task) {
	return task_get_hint_common(game, task, "Question");
}

const sc_char *task_get_hint_subtle(sc_gameref_t game, sc_int task) {
	return task_get_hint_common(game, task, "Hint1");
}

const sc_char *task_get_hint_unsubtle(sc_gameref_t game, sc_int task) {
	return task_get_hint_common(game, task, "Hint2");
}

sc_bool task_has_hints(sc_gameref_t game, sc_int task) {
	/* A non-empty question implies hints available. */
	return !sc_strempty(task_get_hint_question(game, task));
}


/*
 * task_can_run_task_directional()
 *
 * Return TRUE if player is in a room where the task can be run and the task
 * is runnable in the given direction.
 */
sc_bool task_can_run_task_directional(sc_gameref_t game, sc_int task, sc_bool forwards) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[5];
	sc_int type;

	/* If already run, non-repeatable tasks are not re-runnable forwards. */
	if (forwards && gs_task_done(game, task)) {
		sc_bool repeatable;
		const sc_char *repeattext;

		vt_key[0].string = "Tasks";
		vt_key[1].integer = task;
		vt_key[2].string = "Repeatable";
		repeatable = prop_get_boolean(bundle, "B<-sis", vt_key);
		if (!repeatable)
			return FALSE;

		vt_key[2].string = "RepeatText";
		repeattext = prop_get_string(bundle, "S<-sis", vt_key);
		if (!sc_strempty(repeattext))
			return FALSE;
	}

	/* If checking for reverse, test the reversibility flag. */
	if (!forwards) {
		sc_bool reversible;

		vt_key[0].string = "Tasks";
		vt_key[1].integer = task;
		vt_key[2].string = "Reversible";
		reversible = prop_get_boolean(bundle, "B<-sis", vt_key);
		if (!reversible)
			return FALSE;
	}

	/* Check room list for the task and return it. */
	vt_key[0].string = "Tasks";
	vt_key[1].integer = task;
	vt_key[2].string = "Where";
	vt_key[3].string = "Type";
	type = prop_get_integer(bundle, "I<-siss", vt_key);
	switch (type) {
	case ROOMLIST_NO_ROOMS:
		return FALSE;
	case ROOMLIST_ALL_ROOMS:
		return TRUE;

	case ROOMLIST_ONE_ROOM:
		vt_key[3].string = "Room";
		return prop_get_integer(bundle,
		                        "I<-siss", vt_key) == gs_playerroom(game);

	case ROOMLIST_SOME_ROOMS:
		vt_key[3].string = "Rooms";
		vt_key[4].integer = gs_playerroom(game);
		return prop_get_boolean(bundle, "B<-sissi", vt_key);

	default:
		sc_fatal("task_can_run_task_directional: invalid type, %ld\n", type);
		return FALSE;
	}
}


/*
 * task_can_run_task()
 *
 * Returns TRUE if the task can be run in either direction.
 */
sc_bool task_can_run_task(sc_gameref_t game, sc_int task) {
	/*
	 * Testing reversible tasks first may be a little more efficient if they
	 * aren't common in games.  There is, though, probably a little bit of
	 * redundant work going on here.
	 */
	return task_can_run_task_directional(game, task, FALSE)
	       || task_can_run_task_directional(game, task, TRUE);
}


/*
 * task_move_object()
 *
 * Move an object to a place.
 */
static void task_move_object(sc_gameref_t game, sc_int object, sc_int var2, sc_int var3) {
	const sc_var_setref_t vars = gs_get_vars(game);

	/* Select action depending on var2. */
	switch (var2) {
	case 0:                    /* To room */
		if (var3 == 0) {
			if (task_trace)
				sc_trace("Task: moving object %ld to hidden\n", object);

			gs_object_make_hidden(game, object);
		} else {
			if (task_trace) {
				sc_trace("Task: moving object %ld to room %ld\n",
				         object, var3 - 1);
			}

			if (var3 == 0)
				gs_object_player_get(game, object);
			else
				gs_object_to_room(game, object, var3 - 1);
		}
		break;

	case 1:                    /* To roomgroup part */
		if (task_trace) {
			sc_trace("Task: moving object %ld to random room in group %ld\n",
			         object, var3);
		}

		gs_object_to_room(game, object,
		                  lib_random_roomgroup_member(game, var3));
		break;

	case 2:                    /* Into object */
		if (task_trace)
			sc_trace("Task: moving object %ld into %ld\n", object, var3);

		gs_object_move_into(game, object, obj_container_object(game, var3));
		break;

	case 3:                    /* Onto object */
		if (task_trace)
			sc_trace("Task: moving object %ld onto %ld\n", object, var3);

		gs_object_move_onto(game, object, obj_surface_object(game, var3));
		break;

	case 4:                    /* Held by */
		if (task_trace)
			sc_trace("Task: moving object %ld to held by %ld\n", object, var3);

		if (var3 == 0)            /* Player */
			gs_object_player_get(game, object);
		else if (var3 == 1)       /* Ref character */
			gs_object_npc_get(game, object, var_get_ref_character(vars));
		else                      /* NPC id */
			gs_object_npc_get(game, object, var3 - 2);
		break;

	case 5:                    /* Worn by */
		if (task_trace)
			sc_trace("Task: moving object %ld to worn by %ld\n", object, var3);

		if (var3 == 0)            /* Player */
			gs_object_player_wear(game, object);
		else if (var3 == 1)       /* Ref character */
			gs_object_npc_wear(game, object, var_get_ref_character(vars));
		else                      /* NPC id */
			gs_object_npc_wear(game, object, var3 - 2);
		break;

	case 6: {                  /* Same room as */
		sc_int room, npc;

		if (task_trace) {
			sc_trace("Task: moving object %ld to same room as %ld\n",
			         object, var3);
		}

		if (var3 == 0)          /* Player */
			room = gs_playerroom(game);
		else if (var3 == 1) {   /* Ref character */
			npc = var_get_ref_character(vars);
			room = gs_npc_location(game, npc) - 1;
		} else {                /* NPC id */
			npc = var3 - 2;
			room = gs_npc_location(game, npc) - 1;
		}
		gs_object_to_room(game, object, room);
		break;
	}

	default:
		sc_fatal("task_move_object: unknown move type, %ld\n", var2);
		break;
	}
}


/*
 * task_run_move_object_action()
 *
 * Demultiplex an object move action and execute it.
 */
static void task_run_move_object_action(sc_gameref_t game, sc_int var1, sc_int var2, sc_int var3) {
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_int object;

	/* Select depending on value in var1. */
	switch (var1) {
	case 0:                    /* All held */
		for (object = 0; object < gs_object_count(game); object++) {
			if (gs_object_position(game, object) == OBJ_HELD_PLAYER)
				task_move_object(game, object, var2, var3);
		}
		break;

	case 1:                    /* All worn */
		for (object = 0; object < gs_object_count(game); object++) {
			if (gs_object_position(game, object) == OBJ_WORN_PLAYER)
				task_move_object(game, object, var2, var3);
		}
		break;

	case 2:                    /* Ref object */
		object = var_get_ref_object(vars);
		task_move_object(game, object, var2, var3);
		break;

	default:                   /* Dynamic object */
		object = obj_dynamic_object(game, var1 - 3);
		task_move_object(game, object, var2, var3);
		break;
	}
}


/*
 * task_move_npc_to_room()
 *
 * Move an NPC to a given room.
 */
static void task_move_npc_to_room(sc_gameref_t game, sc_int npc, sc_int room) {
	if (task_trace)
		sc_trace("Task: moving NPC %ld to room %ld\n", npc, room);

	/* Update the NPC's state. */
	if (room < gs_room_count(game))
		gs_set_npc_location(game, npc, room + 1);
	else
		gs_set_npc_location(game, npc,
		                    lib_random_roomgroup_member(game,
		                            room - gs_room_count(game)) + 1);

	gs_set_npc_parent(game, npc, -1);
	gs_set_npc_position(game, npc, 0);
}


/*
 * task_run_move_npc_action()
 *
 * Move player or NPC.
 */
static void task_run_move_npc_action(sc_gameref_t game, sc_int var1, sc_int var2, sc_int var3) {
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_int npc, room, ref_npc = -1;

	/* Player or NPC? */
	if (var1 == 0) {
		/* Player -- decide where to move player to. */
		switch (var2) {
		case 0:                /* To room */
			gs_move_player_to_room(game, var3);
			return;

		case 1:                /* To roomgroup part */
			if (task_trace) {
				sc_trace("Task: moving player to random room in group %ld\n",
				         var3);
			}

			gs_move_player_to_room(game,
			                       lib_random_roomgroup_member(game, var3));
			return;

		case 2:                /* To same room as... */
			switch (var3) {
			case 0:            /* ...player! */
				return;
			case 1:            /* ...referenced NPC */
				npc = var_get_ref_character(vars);
				break;
			default:           /* ...specified NPC */
				npc = var3 - 2;
				break;
			}

			if (task_trace)
				sc_trace("Task: moving player to same room as NPC %ld\n", npc);

			room = gs_npc_location(game, npc) - 1;
			if (room < 0) {
				if (task_trace)
					sc_trace("Task: silently suppressed player move to hidden\n");
			} else
				gs_move_player_to_room(game, room);
			return;

		case 3:                /* To standing on */
			gs_set_playerposition(game, 0);
			gs_set_playerparent(game, obj_standable_object(game, var3 - 1));
			return;

		case 4:                /* To sitting on */
			gs_set_playerposition(game, 1);
			gs_set_playerparent(game, obj_standable_object(game, var3 - 1));
			return;

		case 5:                /* To lying on */
			gs_set_playerposition(game, 2);
			gs_set_playerparent(game, obj_lieable_object(game, var3 - 1));
			return;

		default:
			sc_fatal("task_run_move_npc_action:"
			         " unknown player move type, %ld\n", var2);
			return;
		}
	} else {
		/* NPC -- first find which NPC to move about. */
		if (var1 == 1)
			npc = var_get_ref_character(vars);
		else
			npc = var1 - 2;

		/* Decide where to move the NPC to. */
		switch (var2) {
		case 0:                /* To room */
			task_move_npc_to_room(game, npc, var3 - 1);
			return;

		case 1:                /* To roomgroup part */
			if (task_trace) {
				sc_trace("Task: moving NPC %ld to random room in group %ld\n",
				         npc, var3);
			}

			task_move_npc_to_room(game, npc,
			                      lib_random_roomgroup_member(game, var3));
			return;

		case 2:                /* To same room as... */
			switch (var3) {
			case 0:            /* ...player */
				if (task_trace) {
					sc_trace("Task: moving NPC %ld to same room as player\n",
					         npc);
				}

				task_move_npc_to_room(game, npc, gs_playerroom(game));
				break;
			case 1:            /* ...referenced NPC */
				ref_npc = var_get_ref_character(vars);
				if (task_trace) {
					sc_trace("Task: moving NPC %ld to"
					         " same room as referenced NPC %ld\n", npc, ref_npc);
				}

				room = gs_npc_location(game, ref_npc) - 1;
				task_move_npc_to_room(game, npc, room);
				break;
			default:           /* ...specified NPC */
				ref_npc = var3 - 2;
				if (task_trace) {
					sc_trace("Task: moving NPC %ld to"
					         " same room as NPC %ld\n", npc, ref_npc);
				}

				room = gs_npc_location(game, ref_npc) - 1;
				task_move_npc_to_room(game, npc, room);
				break;
			}
			return;

		case 3:                /* To standing on */
			gs_set_npc_position(game, npc, 0);
			gs_set_npc_parent(game, npc, obj_standable_object(game, var3));
			return;

		case 4:                /* To sitting on */
			gs_set_npc_position(game, npc, 1);
			gs_set_npc_parent(game, npc, obj_standable_object(game, var3));
			return;

		case 5:                /* To lying on */
			gs_set_npc_position(game, npc, 2);
			gs_set_npc_parent(game, npc, obj_lieable_object(game, var3));
			return;

		default:
			sc_fatal("task_run_move_npc_action:"
			         " unknown NPC move type, %ld\n", var2);
			return;
		}
	}
}


/*
 * task_run_change_object_status()
 *
 * Change the status of an object.
 */
static void task_run_change_object_status(sc_gameref_t game, sc_int var1, sc_int var2) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_int object, openable, lockable;

	if (task_trace) {
		sc_trace("Task: setting status of stateful object %ld to %ld\n",
		         var1, var2);
	}

	/* Identify the target object. */
	object = obj_stateful_object(game, var1);

	/* See if openable. */
	vt_key[0].string = "Objects";
	vt_key[1].integer = object;
	vt_key[2].string = "Openable";
	openable = prop_get_integer(bundle, "I<-sis", vt_key);
	if (openable > 0) {
		/* See if lockable. */
		vt_key[2].string = "Key";
		lockable = prop_get_integer(bundle, "I<-sis", vt_key);
		if (lockable >= 0) {
			/* Lockable. */
			if (var2 <= 2)
				gs_set_object_openness(game, object, var2 + 5);
			else
				gs_set_object_state(game, object, var2 - 2);
		} else {
			/* Not lockable, though openable. */
			if (var2 <= 1)
				gs_set_object_openness(game, object, var2 + 5);
			else
				gs_set_object_state(game, object, var2 - 1);
		}
	} else
		/* Not openable. */
		gs_set_object_state(game, object, var2 + 1);

	if (task_trace) {
		sc_trace("Task: openness of object %ld is now %ld\n",
		         object, gs_object_openness(game, object));
		sc_trace("Task: state of object %ld is now %ld\n",
		         object, gs_object_state(game, object));
	}
}


/*
 * task_run_change_variable_action()
 *
 * Change a variable's value in inscrutable ways.
 */
static void task_run_change_variable_action(sc_gameref_t game,
		sc_int var1, sc_int var2, sc_int var3, const sc_char *expr, sc_int var5) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	const sc_var_setref_t vars = gs_get_vars(game);
	sc_vartype_t vt_key[3];
	const sc_char *name, *string;
	sc_char *mutable_string;
	sc_int type, value;

	/*
	 * At this point, we need to checkpoint the filter.  We're about to change
	 * a variable value, so interpolating here before doing that ensures that
	 * any currently buffered text gets the values that were set when the text
	 * was buffered.
	 */
	pf_checkpoint(filter, vars, bundle);

	/* Get the name and type of the variable being addressed. */
	vt_key[0].string = "Variables";
	vt_key[1].integer = var1;
	vt_key[2].string = "Name";
	name = prop_get_string(bundle, "S<-sis", vt_key);
	vt_key[2].string = "Type";
	type = prop_get_integer(bundle, "I<-sis", vt_key);

	/* Select first based on variable type. */
	switch (type) {
	case TAFVAR_NUMERIC:       /* Integer */

		/* Select again based on action type. */
		switch (var2) {
		case 0:                /* Var = */
			if (task_trace)
				sc_trace("Task: variable %ld (%s) = %ld\n", var1, name, var3);

			var_put_integer(vars, name, var3);
			return;

		case 1:                /* Var += */
			if (task_trace)
				sc_trace("Task: variable %ld (%s) += %ld\n", var1, name, var3);

			value = var_get_integer(vars, name) + var3;
			var_put_integer(vars, name, value);
			return;

		case 2:                /* Var = rnd(range) */
			if (task_trace) {
				sc_trace("Task: variable %ld (%s) = random(%ld,%ld)\n",
				         var1, name, var3, var5);
			}

			value = sc_randomint(var3, var5);
			var_put_integer(vars, name, value);
			return;

		case 3:                /* Var += rnd(range) */
			if (task_trace) {
				sc_trace("Task: variable %ld (%s) += random(%ld,%ld)\n",
				         var1, name, var3, var5);
			}

			value = var_get_integer(vars, name) + sc_randomint(var3, var5);
			var_put_integer(vars, name, value);
			return;

		case 4:                /* Var = ref */
			value = var_get_ref_number(vars);
			if (task_trace) {
				sc_trace("Task: variable %ld (%s) = ref, %ld\n",
				         var1, name, value);
			}

			var_put_integer(vars, name, value);
			return;

		case 5:                /* Var = expr */
			if (!expr_eval_numeric_expression(expr, vars, &value)) {
				sc_error("task_run_change_variable_action:"
				         " invalid expression, %s\n", expr);
				value = 0;
			}
			if (task_trace) {
				sc_trace("Task: variable %ld (%s) = %s, %ld\n",
				         var1, name, expr, value);
			}

			var_put_integer(vars, name, value);
			return;

		default:
			sc_fatal("task_run_change_variable_action:"
			         " unknown integer change type, %ld\n", var2);
			break;
		}
		break;

	case TAFVAR_STRING:        /* String */

		/* Select again based on action type. */
		switch (var2) {
		case 0:                /* Var = text literal */
			if (task_trace) {
				sc_trace("Task: variable %ld (%s) = \"%s\"\n",
				         var1, name, expr);
			}

			var_put_string(vars, name, expr);
			return;

		case 1:                /* Var = ref */
			string = var_get_ref_text(vars);
			if (task_trace) {
				sc_trace("Task: variable %ld (%s) = ref, \"%s\"\n",
				         var1, name, string);
			}

			var_put_string(vars, name, string);
			return;

		case 2:                /* Var = expr */
			if (!expr_eval_string_expression(expr, vars, &mutable_string)) {
				sc_error("task_run_change_variable_action:"
				         " invalid string expression, %s\n", expr);
				mutable_string = (sc_char *)sc_malloc(strlen("[expr error]") + 1);
				strcpy(mutable_string, "[expr error]");
			}
			if (task_trace) {
				sc_trace("Task: variable %ld (%s) = %s, %s\n",
				         var1, name, expr, mutable_string);
			}

			var_put_string(vars, name, mutable_string);
			sc_free(mutable_string);
			return;

		default:
			sc_fatal("task_run_change_variable_action:"
			         " unknown string change type, %ld\n", var2);
			break;
		}
		break;

	default:
		sc_fatal("task_run_change_variable_action:"
		         " invalid variable type, %ld\n", type);
		break;
	}
}


/*
 * task_run_change_score_action()
 *
 * Change game score.
 */
static void task_run_change_score_action(sc_gameref_t game, sc_int task, sc_int var1) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);

	/* Increasing or decreasing the score? */
	if (var1 > 0) {
		sc_bool increase_score;

		/* See if this task is already scored. */
		increase_score = !gs_task_scored(game, task);
		if (!increase_score) {
			sc_vartype_t vt_key[3];
			sc_int version;

			if (task_trace)
				sc_trace("Task: already scored task %ld\n", var1);

			/* Version 3.8 games permit tasks to rescore. */
			vt_key[0].string = "Version";
			version = prop_get_integer(bundle, "I<-s", vt_key);
			if (version == TAF_VERSION_380) {
				vt_key[0].string = "Tasks";
				vt_key[1].integer = task;
				vt_key[2].string = "SingleScore";
				increase_score = !prop_get_boolean(bundle, "B<-sis", vt_key);

				if (increase_score) {
					if (task_trace)
						sc_trace("Task: rescoring version 3.8 task anyway\n");
				}
			}
		}

		/*
		 * Increase the score if not yet scored or a version 3.8 multiple
		 * scoring task, and note as a scored task.
		 */
		if (increase_score) {
			if (task_trace)
				sc_trace("Task: increased score by %ld\n", var1);

			game->score += var1;
			gs_set_task_scored(game, task, TRUE);
		}
	} else if (var1 < 0) {
		/* Decrease the score. */
		if (task_trace)
			sc_trace("Task: decreased score by %ld\n", -(var1));

		game->score += var1;
	}
}


/*
 * task_run_set_task_action()
 *
 * Redirect to another task.
 */
static sc_bool task_run_set_task_action(sc_gameref_t game, sc_int var1, sc_int var2) {
	sc_bool status = FALSE;

	/* Select based on var1. */
	if (var1 == 0) {
		/* Redirect forwards. */
		if (task_can_run_task_directional(game, var2, TRUE)) {
			if (task_trace)
				sc_trace("Task: redirecting to task %ld\n", var2);

			status = task_run_task(game, var2, TRUE);
		} else {
			if (task_trace)
				sc_trace("Task: can't redirect to task %ld\n", var2);
		}
	} else {
		/* Undo task. */
		gs_set_task_done(game, var2, FALSE);
		if (task_trace)
			sc_trace("Task: reversing task %ld\n", var2);
	}

	return status;
}


/*
 * task_run_end_game_action()
 *
 * End of game task action.
 */
static sc_bool task_run_end_game_action(sc_gameref_t game, sc_int var1) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_bool status = FALSE;

	/* Print a message based on var1. */
	switch (var1) {
	case 0: {
		sc_vartype_t vt_key[2];
		const sc_char *wintext;

		/* Get game WinText. */
		vt_key[0].string = "Header";
		vt_key[1].string = "WinText";
		wintext = prop_get_string(bundle, "S<-ss", vt_key);

		/* Print WinText, if any defined, otherwise a default. */
		if (!sc_strempty(wintext)) {
			pf_buffer_string(filter, wintext);
			pf_buffer_character(filter, '\n');
		} else
			pf_buffer_string(filter, "Congratulations!\n");

		/* Handle any associated WinRes resource. */
		vt_key[0].string = "Globals";
		vt_key[1].string = "WinRes";
		res_handle_resource(game, "ss", vt_key);

		status = TRUE;
		break;
	}

	case 1:
		pf_buffer_string(filter, "Better luck next time.\n");
		status = TRUE;
		break;

	case 2:
		pf_buffer_string(filter, "I'm afraid you are dead!\n");
		status = TRUE;
		break;

	case 3:
		break;

	default:
		sc_fatal("task_run_end_game_action: invalid type, %ld\n", var1);
		break;
	}

	/* Stop the game, and note that it's not resumeable. */
	game->is_running = FALSE;
	game->has_completed = TRUE;

	return status;
}


/*
 * task_run_task_action()
 *
 * Demultiplexer for task actions.
 */
static sc_bool task_run_task_action(sc_gameref_t game, sc_int task, sc_int action) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[5];
	sc_int type, var1, var2, var3, var5;
	const sc_char *expr;
	sc_bool status = FALSE;

	/* Get the task action type. */
	vt_key[0].string = "Tasks";
	vt_key[1].integer = task;
	vt_key[2].string = "Actions";
	vt_key[3].integer = action;
	vt_key[4].string = "Type";
	type = prop_get_integer(bundle, "I<-sisis", vt_key);

	/* Demultiplex depending on type. */
	switch (type) {
	case 0:                    /* Move object. */
		vt_key[4].string = "Var1";
		var1 = prop_get_integer(bundle, "I<-sisis", vt_key);
		vt_key[4].string = "Var2";
		var2 = prop_get_integer(bundle, "I<-sisis", vt_key);
		vt_key[4].string = "Var3";
		var3 = prop_get_integer(bundle, "I<-sisis", vt_key);
		task_run_move_object_action(game, var1, var2, var3);
		break;

	case 1:                    /* Move player/NPC. */
		vt_key[4].string = "Var1";
		var1 = prop_get_integer(bundle, "I<-sisis", vt_key);
		vt_key[4].string = "Var2";
		var2 = prop_get_integer(bundle, "I<-sisis", vt_key);
		vt_key[4].string = "Var3";
		var3 = prop_get_integer(bundle, "I<-sisis", vt_key);
		task_run_move_npc_action(game, var1, var2, var3);
		break;

	case 2:                    /* Change object status. */
		vt_key[4].string = "Var1";
		var1 = prop_get_integer(bundle, "I<-sisis", vt_key);
		vt_key[4].string = "Var2";
		var2 = prop_get_integer(bundle, "I<-sisis", vt_key);
		task_run_change_object_status(game, var1, var2);
		break;

	case 3:                    /* Change variable. */
		vt_key[4].string = "Var1";
		var1 = prop_get_integer(bundle, "I<-sisis", vt_key);
		vt_key[4].string = "Var2";
		var2 = prop_get_integer(bundle, "I<-sisis", vt_key);
		vt_key[4].string = "Var3";
		var3 = prop_get_integer(bundle, "I<-sisis", vt_key);
		vt_key[4].string = "Expr";
		expr = prop_get_string(bundle, "S<-sisis", vt_key);
		vt_key[4].string = "Var5";
		var5 = prop_get_integer(bundle, "I<-sisis", vt_key);
		task_run_change_variable_action(game, var1, var2, var3, expr, var5);
		break;

	case 4:                    /* Change score. */
		vt_key[4].string = "Var1";
		var1 = prop_get_integer(bundle, "I<-sisis", vt_key);
		task_run_change_score_action(game, task, var1);
		break;

	case 5:                    /* Execute/unset task. */
		vt_key[4].string = "Var1";
		var1 = prop_get_integer(bundle, "I<-sisis", vt_key);
		vt_key[4].string = "Var2";
		var2 = prop_get_integer(bundle, "I<-sisis", vt_key);
		status = task_run_set_task_action(game, var1, var2);
		break;

	case 6:                    /* End game. */
		vt_key[4].string = "Var1";
		var1 = prop_get_integer(bundle, "I<-sisis", vt_key);
		status = task_run_end_game_action(game, var1);
		break;

	case 7:                    /* Battle options, ignored for now... */
		break;

	default:
		sc_fatal("task_run_task_action: unknown action type %ld\n", type);
		break;
	}

	return status;
}


/*
 * task_run_task_actions()
 *
 * Run every task action associated with the task.  If any action ends the
 * game, return immediately.  Returns TRUE if any action ran and itself
 * returned TRUE.
 */
static sc_bool task_run_task_actions(sc_gameref_t game, sc_int task) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_int action_count, action;
	sc_bool status, muted;

	/* Get the count of task actions. */
	vt_key[0].string = "Tasks";
	vt_key[1].integer = task;
	vt_key[2].string = "Actions";
	action_count = prop_get_child_count(bundle, "I<-sis", vt_key);

	if (action_count > 0) {
		if (task_trace) {
			sc_trace("Task: task %ld running %ld action%s\n",
			         task, action_count, action_count == 1 ? "" : "s");
		}
	}

	/*
	 * Run all task actions, capturing any TRUE status returned.  If any task
	 * ends the game, run the remaining tasks silently.
	 *
	 * This seems a little counterintuitive; a more conventional thing would be
	 * to just exit the actions loop early.  However, Adrift appears to plough
	 * on, and there may be an action that changes the score in here somewhere,
	 * so we'll do the same.
	 */
	status = FALSE;
	muted = FALSE;
	for (action = 0; action < action_count; action++) {
		sc_bool was_running;

		was_running = game->is_running;
		status |= task_run_task_action(game, task, action);

		/* Did this action end the game? */
		if (was_running && !game->is_running) {
			if (task_trace) {
				sc_trace("Task: task %ld action %ld ended game\n",
				         task, action);
			}

			/* Mute the filter, and note that we did it, but continue. */
			pf_mute(filter);
			muted = TRUE;
		}
	}

	/* If this stack frame muted the filter, un-mute it now. */
	if (muted)
		pf_clear_mute(filter);

	/* Return TRUE if any task action returned TRUE. */
	return status;
}


/*
 * task_start_npc_walks()
 *
 * Start NPC walks based on alerts.
 */
static void task_start_npc_walks(sc_gameref_t game, sc_int task) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[4];
	sc_int alert_count, alert;

	/* Get a count of NPC walk alerts. */
	vt_key[0].string = "Tasks";
	vt_key[1].integer = task;
	vt_key[2].string = "NPCWalkAlert";
	alert_count = prop_get_child_count(bundle, "I<-sis", vt_key);

	/* Check alerts, and start any walks that need starting. */
	for (alert = 0; alert < alert_count; alert += 2) {
		sc_int npc, walk;

		vt_key[3].integer = alert;
		npc = prop_get_integer(bundle, "I<-sisi", vt_key);
		vt_key[3].integer = alert + 1;
		walk = prop_get_integer(bundle, "I<-sisi", vt_key);
		npc_start_npc_walk(game, npc, walk);
	}
}


/*
 * task_run_task_unrestricted()
 *
 * Run a task, providing restrictions permit, in the given direction.  Return
 * TRUE if the task ran, or we handled it in some complete way, for example by
 * outputting a message describing what prevented it, or why it couldn't be
 * done.
 */
static sc_bool task_run_task_unrestricted(sc_gameref_t game, sc_int task, sc_bool forwards) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	const sc_char *completetext, *additionalmessage;
	sc_int action_count, showroomdesc;
	sc_bool status;

	/* Start considering task output tracking. */
	status = FALSE;

	/*
	 * If reversing, print any reverse message for the task, and undo the task,
	 * then return.
	 */
	if (!forwards) {
		const sc_char *reversemessage;

		/* If not yet done, we can hardly reverse it. */
		if (gs_task_done(game, task)) {
			vt_key[0].string = "Tasks";
			vt_key[1].integer = task;
			vt_key[2].string = "ReverseMessage";
			reversemessage = prop_get_string(bundle, "S<-sis", vt_key);
			if (!sc_strempty(reversemessage)) {
				pf_buffer_string(filter, reversemessage);
				pf_buffer_character(filter, '\n');
				status |= TRUE;
			}

			/* Undo the task. */
			gs_set_task_done(game, task, FALSE);
		}

		/* Return status of undo. */
		return status;
	}

	/* See if we are trying to repeat a task that's not repeatable. */
	if (gs_task_done(game, task)) {
		sc_bool repeatable;

		vt_key[0].string = "Tasks";
		vt_key[1].integer = task;
		vt_key[2].string = "Repeatable";
		repeatable = prop_get_boolean(bundle, "B<-sis", vt_key);
		if (!repeatable) {
			const sc_char *repeattext;

			vt_key[2].string = "RepeatText";
			repeattext = prop_get_string(bundle, "S<-sis", vt_key);
			if (!sc_strempty(repeattext)) {
				if (task_trace) {
					sc_trace("Task:"
					         " trying to repeat completed action, aborting\n");
				}

				pf_buffer_string(filter, repeattext);
				pf_buffer_character(filter, '\n');
				status |= TRUE;
				return status;
			}

			/*
			 * Task done, yet not repeatable, so don't consider this case
			 * handled.
			 */
			return status;
		}
	}

	/* Mark the task as done. */
	gs_set_task_done(game, task, TRUE);

	/* Print any task completion text. */
	vt_key[0].string = "Tasks";
	vt_key[1].integer = task;
	vt_key[2].string = "CompleteText";
	completetext = prop_get_string(bundle, "S<-sis", vt_key);
	if (!sc_strempty(completetext)) {
		pf_buffer_string(filter, completetext);
		pf_buffer_character(filter, '\n');
		status |= TRUE;
	}

	/* Handle any task completion resource. */
	vt_key[2].string = "Res";
	res_handle_resource(game, "sis", vt_key);

	/*
	 * Things get slightly tricky here.  We need to filter the completion text
	 * for the task using any final variable values generated or modified by
	 * task actions, but other task text, run by actions, according to the
	 * variable value in effect when it runs.
	 *
	 * To do this, we take a local copy of the filter's current buffer at this
	 * point, remove it from the filter, run task actions with checkpointing,
	 * then prepend it back into the filter after all the actions are done.
	 *
	 * As an optimization, we can avoid doing this if there are no task actions.
	 */
	vt_key[2].string = "Actions";
	action_count = prop_get_child_count(bundle, "I<-sis", vt_key);
	if (action_count > 0) {
		sc_char *buffer;

		/*
		 * Take ownership of the current filter buffer text, then start NPC
		 * walks based on alerts, and run any and all task actions.  Note that
		 * the buffer transferred out of the filter may be NULL if there is no
		 * text currently in the filter.
		 */
		buffer = pf_transfer_buffer(filter);
		task_start_npc_walks(game, task);
		status |= task_run_task_actions(game, task);

		/* Prepend the saved buffer data back onto the front of the filter. */
		if (buffer) {
			pf_prepend_string(filter, buffer);
			sc_free(buffer);
		}
	} else {
		/* Start NPC walks only; there are no task actions. */
		task_start_npc_walks(game, task);
	}

	/* Append any room description and additional message for the task. */
	vt_key[2].string = "ShowRoomDesc";
	showroomdesc = prop_get_integer(bundle, "I<-sis", vt_key);
	if (showroomdesc != 0) {
		lib_print_room_name(game, showroomdesc - 1);
		lib_print_room_description(game, showroomdesc - 1);
		status |= TRUE;
	}

	vt_key[2].string = "AdditionalMessage";
	additionalmessage = prop_get_string(bundle, "S<-sis", vt_key);
	if (!sc_strempty(additionalmessage)) {
		pf_buffer_string(filter, additionalmessage);
		pf_buffer_character(filter, '\n');
		status |= TRUE;
	}

	/* Return status -- TRUE if matched and we output something. */
	return status;
}


/*
 * task_run_task()
 *
 * Run a task, providing restrictions permit, in the given direction.  At the
 * same time, check for signs of an infinite loop in game tasks, and fail the
 * task with an error message if we seem to be in one.  Checked by counting
 * the call depth.
 */
sc_bool task_run_task(sc_gameref_t game, sc_int task, sc_bool forwards) {
	static sc_int recursion_depth = 0;

	const sc_filterref_t filter = gs_get_filter(game);
	const sc_char *fail_message;
	sc_bool restrictions_passed, status;

	if (task_trace) {
		sc_trace("Task: running task %ld %s, depth %ld\n",
		         task, forwards ? "forwards" : "backwards", recursion_depth);
	}

	/* Check restrictions. */
	if (!restr_eval_task_restrictions(game, task,
	                                  &restrictions_passed, &fail_message)) {
		sc_error("task_run_task: restrictions error, %ld\n", task);
		return FALSE;
	}
	if (!restrictions_passed) {
		if (task_trace) {
			sc_trace("Task: restrictions failed, task %s\n",
			         fail_message ? "failed" : "aborted");
		}

		if (fail_message) {
			/*
			 * Print a message, and return TRUE since we can consider this task
			 * "done" (more accurately, we've output text, so the task command
			 * searching in the main run loop can exit...).
			 */
			pf_buffer_string(filter, fail_message);
			pf_buffer_character(filter, '\n');
			return TRUE;
		}

		/* Task not done; look for more possibilities. */
		return FALSE;
	}

	/* Check for infinite recursion. */
	if (recursion_depth > TASK_MAXIMUM_RECURSION) {
		sc_error("task_run_task: maximum recursion depth exceeded --"
		         " game task loop?\n");
		return FALSE;
	}

	/* Increment depth, run the task, then decrement depth. */
	recursion_depth++;
	status = task_run_task_unrestricted(game, task, forwards);
	recursion_depth--;

	if (task_trace) {
		sc_trace("Task: task %ld finished, return %s, depth %ld\n",
		         task, status ? "true" : "false", recursion_depth);
	}

	/* Return the task's status. */
	return status;
}


/*
 * task_debug_trace()
 *
 * Set task tracing on/off.
 */
void task_debug_trace(sc_bool flag) {
	task_trace = flag;
}

} // End of namespace Adrift
} // End of namespace Glk
