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
 * o Event pause and resume tasks need more testing.
 */

/* Trace flag, set before running. */
static sc_bool evt_trace = FALSE;


/*
 * evt_any_task_in_state()
 *
 * Return TRUE if any task at all matches the given completion state.
 */
static sc_bool evt_any_task_in_state(sc_gameref_t game, sc_bool state) {
	sc_int task;

	/* Scan tasks for any whose completion matches input. */
	for (task = 0; task < gs_task_count(game); task++) {
		if (gs_task_done(game, task) == state)
			return TRUE;
	}

	/* No tasks matched. */
	return FALSE;
}


/*
 * evt_can_see_event()
 *
 * Return TRUE if player is in the right room for event text.
 */
sc_bool evt_can_see_event(sc_gameref_t game, sc_int event) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[5];
	sc_int type;

	/* Check room list for the event and return it. */
	vt_key[0].string = "Events";
	vt_key[1].integer = event;
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
		return prop_get_integer(bundle, "I<-siss", vt_key)
		       == gs_playerroom(game);

	case ROOMLIST_SOME_ROOMS:
		vt_key[3].string = "Rooms";
		vt_key[4].integer = gs_playerroom(game);
		return prop_get_boolean(bundle, "B<-sissi", vt_key);

	default:
		sc_fatal("evt_can_see_event: invalid type, %ld\n", type);
		return FALSE;
	}
}


/*
 * evt_move_object()
 *
 * Move an object from within an event.
 */
static void evt_move_object(sc_gameref_t game, sc_int object, sc_int destination) {
	/* Ignore negative values of object. */
	if (object >= 0) {
		if (evt_trace) {
			sc_trace("Event: moving object %ld to room %ld\n",
			         object, destination);
		}

		/* Move object depending on destination. */
		switch (destination) {
		case -1:               /* Hidden. */
			gs_object_make_hidden(game, object);
			break;

		case 0:                /* Held by player. */
			gs_object_player_get(game, object);
			break;

		case 1:                /* Same room as player. */
			gs_object_to_room(game, object, gs_playerroom(game));
			break;

		default:
			if (destination < gs_room_count(game) + 2)
				gs_object_to_room(game, object, destination - 2);
			else {
				sc_int roomgroup, room;

				roomgroup = destination - gs_room_count(game) - 2;
				room = lib_random_roomgroup_member(game, roomgroup);
				gs_object_to_room(game, object, room);
			}
			break;
		}

		/*
		 * If static, mark as no longer unmoved.
		 *
		 * TODO Is this the only place static objects can be moved?  And just
		 * how static is a static object if it's moveable, anyway?
		 */
		if (obj_is_static(game, object))
			gs_set_object_static_unmoved(game, object, FALSE);
	}
}


/*
 * evt_fixup_v390_v380_immediate_restart()
 *
 * Versions 3.9 and 3.8 differ from version 4.0 on immediate restart; they
 * "miss" the event start actions and move one step into the event without
 * comment.  It's arguable if this is a feature or a bug; nevertheless, we
 * can do the same thing here, though it's ugly.
 */
static sc_bool evt_fixup_v390_v380_immediate_restart(sc_gameref_t game, sc_int event) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_int version;

	vt_key[0].string = "Version";
	version = prop_get_integer(bundle, "I<-s", vt_key);
	if (version < TAF_VERSION_400) {
		sc_int time1, time2;

		if (evt_trace)
			sc_trace("Event: applying 3.9/3.8 restart fixup\n");

		/* Set to running state. */
		gs_set_event_state(game, event, ES_RUNNING);

		/* Set up event time to be one less than a proper start. */
		vt_key[0].string = "Events";
		vt_key[1].integer = event;
		vt_key[2].string = "Time1";
		time1 = prop_get_integer(bundle, "I<-sis", vt_key);
		vt_key[2].string = "Time2";
		time2 = prop_get_integer(bundle, "I<-sis", vt_key);
		gs_set_event_time(game, event, sc_randomint(time1, time2) - 1);
	}

	/* Return TRUE if we applied the fixup. */
	return version < TAF_VERSION_400;
}


/*
 * evt_start_event()
 *
 * Change an event from WAITING to RUNNING.
 */
static void evt_start_event(sc_gameref_t game, sc_int event) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[4];
	sc_int time1, time2, obj1, obj1dest;

	if (evt_trace)
		sc_trace("Event: starting event %ld\n", event);

	/* If event is visible, print its start text. */
	if (evt_can_see_event(game, event)) {
		const sc_char *starttext;

		/* Get and print start text. */
		vt_key[0].string = "Events";
		vt_key[1].integer = event;
		vt_key[2].string = "StartText";
		starttext = prop_get_string(bundle, "S<-sis", vt_key);
		if (!sc_strempty(starttext)) {
			pf_buffer_string(filter, starttext);
			pf_buffer_character(filter, '\n');
		}

		/* Handle any associated resource. */
		vt_key[2].string = "Res";
		vt_key[3].integer = 0;
		res_handle_resource(game, "sisi", vt_key);
	}

	/* Move event object to destination. */
	vt_key[0].string = "Events";
	vt_key[1].integer = event;
	vt_key[2].string = "Obj1";
	obj1 = prop_get_integer(bundle, "I<-sis", vt_key) - 1;
	vt_key[2].string = "Obj1Dest";
	obj1dest = prop_get_integer(bundle, "I<-sis", vt_key) - 1;
	evt_move_object(game, obj1, obj1dest);

	/* Set the event's state and time. */
	gs_set_event_state(game, event, ES_RUNNING);

	vt_key[2].string = "Time1";
	time1 = prop_get_integer(bundle, "I<-sis", vt_key);
	vt_key[2].string = "Time2";
	time2 = prop_get_integer(bundle, "I<-sis", vt_key);
	gs_set_event_time(game, event, sc_randomint(time1, time2));

	if (evt_trace)
		sc_trace("Event: start event handling done, %ld\n", event);
}


/*
 * evt_get_starter_type()
 *
 * Return the starter type for an event.
 */
static sc_int evt_get_starter_type(sc_gameref_t game, sc_int event) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_int startertype;

	vt_key[0].string = "Events";
	vt_key[1].integer = event;
	vt_key[2].string = "StarterType";
	startertype = prop_get_integer(bundle, "I<-sis", vt_key);

	return startertype;
}


/*
 * evt_finish_event()
 *
 * Move an event to FINISHED, or restart it.
 */
static void evt_finish_event(sc_gameref_t game, sc_int event) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[4];
	sc_int obj2, obj2dest, obj3, obj3dest;
	sc_int task, startertype, restarttype;
	sc_bool taskdir;

	if (evt_trace)
		sc_trace("Event: finishing event %ld\n", event);

	/* Set up invariant parts of the key. */
	vt_key[0].string = "Events";
	vt_key[1].integer = event;

	/* If event is visible, print its finish text. */
	if (evt_can_see_event(game, event)) {
		const sc_char *finishtext;

		/* Get and print finish text. */
		vt_key[2].string = "FinishText";
		finishtext = prop_get_string(bundle, "S<-sis", vt_key);
		if (!sc_strempty(finishtext)) {
			pf_buffer_string(filter, finishtext);
			pf_buffer_character(filter, '\n');
		}

		/* Handle any associated resource. */
		vt_key[2].string = "Res";
		vt_key[3].integer = 4;
		res_handle_resource(game, "sisi", vt_key);
	}

	/* Move event objects to destination. */
	vt_key[2].string = "Obj2";
	obj2 = prop_get_integer(bundle, "I<-sis", vt_key) - 1;
	vt_key[2].string = "Obj2Dest";
	obj2dest = prop_get_integer(bundle, "I<-sis", vt_key) - 1;
	evt_move_object(game, obj2, obj2dest);

	vt_key[2].string = "Obj3";
	obj3 = prop_get_integer(bundle, "I<-sis", vt_key) - 1;
	vt_key[2].string = "Obj3Dest";
	obj3dest = prop_get_integer(bundle, "I<-sis", vt_key) - 1;
	evt_move_object(game, obj3, obj3dest);

	/* See if there is an affected task. */
	vt_key[2].string = "TaskAffected";
	task = prop_get_integer(bundle, "I<-sis", vt_key) - 1;
	if (task >= 0) {
		vt_key[2].string = "TaskFinished";
		taskdir = !prop_get_boolean(bundle, "B<-sis", vt_key);
		if (task_can_run_task_directional(game, task, taskdir)) {
			if (evt_trace) {
				sc_trace("Event: event running task %ld, %s\n",
				         task, taskdir ? "forwards" : "backwards");
			}

			task_run_task(game, task, taskdir);
		} else {
			if (evt_trace)
				sc_trace("Event: event can't run task %ld\n", task);
		}
	}

	/* Handle possible restart. */
	vt_key[2].string = "RestartType";
	restarttype = prop_get_integer(bundle, "I<-sis", vt_key);
	switch (restarttype) {
	case 0:                    /* Don't restart. */
		startertype = evt_get_starter_type(game, event);
		switch (startertype) {
		case 1:                /* Immediate. */
		case 2:                /* Random delay. */
		case 3:                /* After task. */
			gs_set_event_state(game, event, ES_FINISHED);
			gs_set_event_time(game, event, 0);
			break;

		default:
			sc_fatal("evt_finish_event:"
			         " unknown value for starter type, %ld\n", startertype);
		}
		break;

	case 1:                    /* Restart immediately. */
		if (evt_fixup_v390_v380_immediate_restart(game, event))
			break;
		else
			evt_start_event(game, event);
		break;

	case 2:                    /* Restart after delay. */
		startertype = evt_get_starter_type(game, event);
		switch (startertype) {
		case 1:                /* Immediate. */
			if (evt_fixup_v390_v380_immediate_restart(game, event))
				break;
			else
				evt_start_event(game, event);
			break;

		case 2: {              /* Random delay. */
			sc_int start, end;

			gs_set_event_state(game, event, ES_WAITING);
			vt_key[2].string = "StartTime";
			start = prop_get_integer(bundle, "I<-sis", vt_key);
			vt_key[2].string = "EndTime";
			end = prop_get_integer(bundle, "I<-sis", vt_key);
			gs_set_event_time(game, event, sc_randomint(start, end));
			break;
		}

		case 3:                /* After task. */
			gs_set_event_state(game, event, ES_AWAITING);
			gs_set_event_time(game, event, 0);
			break;

		default:
			sc_fatal("evt_finish_event: unknown StarterType\n");
		}
		break;

	default:
		sc_fatal("evt_finish_event: unknown RestartType\n");
	}

	if (evt_trace)
		sc_trace("Event: finish event handling done, %ld\n", event);
}


/*
 * evt_has_starter_task()
 * evt_starter_task_is_complete()
 * evt_pauser_task_is_complete()
 * evt_resumer_task_is_complete()
 *
 * Return the status of start, pause and resume states of an event.
 */
static sc_bool evt_has_starter_task(sc_gameref_t game, sc_int event) {
	sc_int startertype;

	startertype = evt_get_starter_type(game, event);
	return startertype == 3;
}

static sc_bool evt_starter_task_is_complete(sc_gameref_t game, sc_int event) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_int task;
	sc_bool start;

	vt_key[0].string = "Events";
	vt_key[1].integer = event;
	vt_key[2].string = "TaskNum";
	task = prop_get_integer(bundle, "I<-sis", vt_key);

	start = FALSE;
	if (task == 0) {
		if (evt_any_task_in_state(game, TRUE))
			start = TRUE;
	} else if (task > 0) {
		if (gs_task_done(game, task - 1))
			start = TRUE;
	}

	return start;
}

static sc_bool evt_pauser_task_is_complete(sc_gameref_t game, sc_int event) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_int pausetask;
	sc_bool completed, pause;

	vt_key[0].string = "Events";
	vt_key[1].integer = event;

	vt_key[2].string = "PauseTask";
	pausetask = prop_get_integer(bundle, "I<-sis", vt_key);
	vt_key[2].string = "PauserCompleted";
	completed = !prop_get_boolean(bundle, "B<-sis", vt_key);

	pause = FALSE;
	if (pausetask == 1) {
		if (evt_any_task_in_state(game, completed))
			pause = TRUE;
	} else if (pausetask > 1) {
		if (completed == gs_task_done(game, pausetask - 2))
			pause = TRUE;
	}

	return pause;
}

static sc_bool evt_resumer_task_is_complete(sc_gameref_t game, sc_int event) {
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[3];
	sc_int resumetask;
	sc_bool completed, resume;

	vt_key[0].string = "Events";
	vt_key[1].integer = event;

	vt_key[2].string = "ResumeTask";
	resumetask = prop_get_integer(bundle, "I<-sis", vt_key);
	vt_key[2].string = "ResumerCompleted";
	completed = !prop_get_boolean(bundle, "B<-sis", vt_key);

	resume = FALSE;
	if (resumetask == 1) {
		if (evt_any_task_in_state(game, completed))
			resume = TRUE;
	} else if (resumetask > 1) {
		if (completed == gs_task_done(game, resumetask - 2))
			resume = TRUE;
	}

	return resume;
}


/*
 * evt_handle_preftime_notifications()
 *
 * Print messages and handle resources for the event where we're in mid-event
 * and getting close to some number of turns from its end.
 */
static void evt_handle_preftime_notifications(sc_gameref_t game, sc_int event) {
	const sc_filterref_t filter = gs_get_filter(game);
	const sc_prop_setref_t bundle = gs_get_bundle(game);
	sc_vartype_t vt_key[4];
	sc_int preftime1, preftime2;
	const sc_char *preftext;

	vt_key[0].string = "Events";
	vt_key[1].integer = event;

	vt_key[2].string = "PrefTime1";
	preftime1 = prop_get_integer(bundle, "I<-sis", vt_key);
	if (preftime1 == gs_event_time(game, event)) {
		vt_key[2].string = "PrefText1";
		preftext = prop_get_string(bundle, "S<-sis", vt_key);
		if (!sc_strempty(preftext)) {
			pf_buffer_string(filter, preftext);
			pf_buffer_character(filter, '\n');
		}

		vt_key[2].string = "Res";
		vt_key[3].integer = 2;
		res_handle_resource(game, "sisi", vt_key);
	}

	vt_key[2].string = "PrefTime2";
	preftime2 = prop_get_integer(bundle, "I<-sis", vt_key);
	if (preftime2 == gs_event_time(game, event)) {
		vt_key[2].string = "PrefText2";
		preftext = prop_get_string(bundle, "S<-sis", vt_key);
		if (!sc_strempty(preftext)) {
			pf_buffer_string(filter, preftext);
			pf_buffer_character(filter, '\n');
		}

		vt_key[2].string = "Res";
		vt_key[3].integer = 3;
		res_handle_resource(game, "sisi", vt_key);
	}
}


/*
 * evt_tick_event()
 *
 * Attempt to advance an event by one turn.
 */
static void evt_tick_event(sc_gameref_t game, sc_int event) {
	if (evt_trace) {
		sc_trace("Event: ticking event %ld: state %ld, time %ld\n", event,
		         gs_event_state(game, event), gs_event_time(game, event));
	}

	/* Handle call based on current event state. */
	switch (gs_event_state(game, event)) {
	case ES_WAITING: {
		if (evt_trace)
			sc_trace("Event: ticking waiting event %ld\n", event);

		/*
		 * Because we also tick an event that goes from waiting to running,
		 * events started here will tick through RUNNING too, and have their
		 * time decremented.  To get around this, so that the timer for one-
		 * shot events doesn't look one lower than it should after this
		 * transition, we need to set the initial time for events that start
		 * as soon as the game starts to one greater than that set by
		 * evt_start_time().  Here's the hack to do that; if the event starts
		 * immediately, its time will already be zero, even before decrement,
		 * which is how we tell which events to apply this hack to.
		 *
		 * TODO This seems to work, but also seems very dodgy.
		 */
		if (gs_event_time(game, event) == 0) {
			evt_start_event(game, event);

			/* If the event time was set to zero, finish immediately. */
			if (gs_event_time(game, event) <= 0)
				evt_finish_event(game, event);
			else
				gs_set_event_time(game, event, gs_event_time(game, event) + 1);
			break;
		}

		/*
		 * Decrement the event's time, and if it goes to zero, start running
		 * the event.
		 */
		gs_decrement_event_time(game, event);

		if (gs_event_time(game, event) <= 0) {
			evt_start_event(game, event);

			/* If the event time was set to zero, finish immediately. */
			if (gs_event_time(game, event) <= 0)
				evt_finish_event(game, event);
		}
	}
	break;

	case ES_RUNNING: {
		if (evt_trace)
			sc_trace("Event: ticking running event %ld\n", event);

		/*
		 * Re-check the starter task; if it's no longer completed, we need
		 * to set the event back to waiting on task.
		 */
		if (evt_has_starter_task(game, event)) {
			if (!evt_starter_task_is_complete(game, event)) {
				if (evt_trace)
					sc_trace("Event: starter task not complete\n");

				gs_set_event_state(game, event, ES_AWAITING);
				gs_set_event_time(game, event, 0);
				break;
			}
		}

		/* If the pauser has completed, but resumer not, pause this event. */
		if (evt_pauser_task_is_complete(game, event)
		        && !evt_resumer_task_is_complete(game, event)) {
			if (evt_trace)
				sc_trace("Event: pause complete\n");

			gs_set_event_state(game, event, ES_PAUSED);
			break;
		}

		/*
		 * Decrement the event's time, and print any notifications for a set
		 * number of turns from the event end.
		 */
		gs_decrement_event_time(game, event);

		if (evt_can_see_event(game, event))
			evt_handle_preftime_notifications(game, event);

		/* If the time goes to zero, finish running the event. */
		if (gs_event_time(game, event) <= 0)
			evt_finish_event(game, event);
	}
	break;

	case ES_AWAITING: {
		if (evt_trace)
			sc_trace("Event: ticking awaiting event %ld\n", event);

		/*
		 * Check the starter task.  If it's completed, start running the
		 * event.
		 */
		if (evt_starter_task_is_complete(game, event)) {
			evt_start_event(game, event);

			/* If the event time was set to zero, finish immediately. */
			if (gs_event_time(game, event) <= 0)
				evt_finish_event(game, event);
			else {
				/*
				 * If the pauser has completed, but resumer not, immediately
				 * also pause this event.
				 */
				if (evt_pauser_task_is_complete(game, event)
				        && !evt_resumer_task_is_complete(game, event)) {
					if (evt_trace)
						sc_trace("Event: pause complete, immediate pause\n");

					gs_set_event_state(game, event, ES_PAUSED);
				}
			}
		}
	}
	break;

	case ES_FINISHED: {
		if (evt_trace)
			sc_trace("Event: ticking finished event %ld\n", event);

		/*
		 * Check the starter task; if it's not completed, we need to set the
		 * event back to waiting on task.
		 *
		 * A completed event needs to go back to waiting on its task, but we
		 * don't want to set it there as soon as the event finishes.  We need
		 * to wait for the starter task to first become undone, otherwise the
		 * event just cycles endlessly, and they don't in Adrift itself.  Here
		 * is where we wait for starter tasks to become undone.
		 */
		if (evt_has_starter_task(game, event)) {
			if (!evt_starter_task_is_complete(game, event)) {
				if (evt_trace)
					sc_trace("Event: starter task not complete\n");

				gs_set_event_state(game, event, ES_AWAITING);
				gs_set_event_time(game, event, 0);
				break;
			}
		}
	}
	break;

	case ES_PAUSED: {
		if (evt_trace)
			sc_trace("Event: ticking paused event %ld\n", event);

		/* If the resumer has completed, resume this event. */
		if (evt_resumer_task_is_complete(game, event)) {
			if (evt_trace)
				sc_trace("Event: resume complete\n");

			gs_set_event_state(game, event, ES_RUNNING);
			break;
		}
	}
	break;

	default:
		sc_fatal("evt_tick: invalid event state\n");
	}

	if (evt_trace) {
		sc_trace("Event: after ticking event %ld: state %ld, time %ld\n", event,
		         gs_event_state(game, event), gs_event_time(game, event));
	}
}


/*
 * evt_tick_events()
 *
 * Attempt to advance each event by one turn.
 */
void evt_tick_events(sc_gameref_t game) {
	sc_int event;

	/*
	 * Tick all events.  If an event transitions into a running state from a
	 * paused or waiting state, tick that event again.
	 */
	for (event = 0; event < gs_event_count(game); event++) {
		sc_int prior_state, state;

		/* Note current state, and tick event forwards. */
		prior_state = gs_event_state(game, event);
		evt_tick_event(game, event);

		/*
		 * If the event went from paused or waiting to running, tick again.
		 * This looks dodgy, and probably is, but it does keep timers correct
		 * by only re-ticking events that have transitioned from non-running
		 * states to a running one, and not already-running events.  This is
		 * in effect just adding a bit of turn processing to a tick that would
		 * otherwise change state alone; a bit of laziness, in other words.
		 */
		state = gs_event_state(game, event);
		if (state == ES_RUNNING
		        && (prior_state == ES_PAUSED || prior_state == ES_WAITING))
			evt_tick_event(game, event);
	}
}


/*
 * evt_debug_trace()
 *
 * Set event tracing on/off.
 */
void evt_debug_trace(sc_bool flag) {
	evt_trace = flag;
}

} // End of namespace Adrift
} // End of namespace Glk
