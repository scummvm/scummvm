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
#include "ultima/nuvie/core/game.h"
#include "ultima/nuvie/actors/actor.h"
#include "ultima/nuvie/gui/widgets/map_window.h"
#include "ultima/nuvie/core/map.h"
#include "ultima/nuvie/core/party.h"
#include "ultima/nuvie/core/events.h"
#include "ultima/nuvie/usecode/usecode.h"
#include "ultima/nuvie/core/u6_objects.h"
#include "ultima/nuvie/actors/u6_work_types.h"
#include "ultima/nuvie/misc/u6_llist.h"
#include "ultima/nuvie/gui/widgets/msg_scroll.h"
#include "ultima/nuvie/core/game_clock.h"
#include "ultima/nuvie/gui/widgets/command_bar.h"
#include "ultima/nuvie/views/view_manager.h"
#include "ultima/nuvie/views/party_view.h"
#include "ultima/nuvie/actors/actor_manager.h"
// FIXME: effects use timers, not the other way around (make a movement effect?)
#include "ultima/nuvie/core/effect_manager.h"
#include "ultima/nuvie/core/effect.h"

#include "ultima/nuvie/core/timed_event.h"

namespace Ultima {
namespace Nuvie {

#define MESG_TIMED CB_TIMED

/* Activate all events for the current time, deleting those that have fired
 * and are of no more use. Repeated timers are requeued.
 */
void TimeQueue::call_timers(uint32 now) {
	while (!empty() && call_timer(now)) {

	}
}

void TimeQueue::clear() {
	while (!empty()) {
		TimedEvent *event = pop_timer();
		if (event->tq_can_delete)
			delete event;
	}
}

/* Add new timed event to queue, which will activate `event' when time is
 * `evtime'.
 */
void TimeQueue::add_timer(TimedEvent *tevent) {
	Std::list<TimedEvent *>::iterator t;
	if (tq.empty()) {
		tq.push_front(tevent);
		return;
	}
	// in case it's already queued, remove the earlier instance(s)
	remove_timer(tevent);
	// add after events with earlier/equal time
	t = tq.begin();
	while (t != tq.end() && (*t)->time <= tevent->time) t++;
	tq.insert(t, tevent);
}


/* Remove timed event from queue.
 */
void TimeQueue::remove_timer(TimedEvent *tevent) {
	Std::list<TimedEvent *>::iterator t;
	t = tq.begin();
	while (t != tq.end()) {
		if (*t == tevent) {
			t = tq.erase(t);
		} else ++t; // this deletes all duplicates
	}
}


/* Remove and return timed event at front of queue, or NULL if empty.
 */
TimedEvent *TimeQueue::pop_timer() {
	TimedEvent *first = NULL;
	if (!empty()) {
		first = tq.front();
		tq.pop_front(); // remove it
	}
	return (first);
}


/* Call timed event at front of queue, whose time is <= `now'.
 * Returns true if an event handler was called. (false if time isn't up yet)
 */
bool TimeQueue::call_timer(uint32 now) {
	if (empty())
		return (false);
	TimedEvent *tevent = tq.front();
	if (tevent->defunct) {
		assert(pop_timer() == tevent);
		delete_timer(tevent);
		return (false);
	}
	if (tevent->time > now)
		return (false);

	//dequeue event here
	pop_timer(); // remove timer in case we have recursion in the timed() call.

	tevent->timed(now); // fire

	//re-queue if repeating timer.

	if (tevent->repeat_count != 0) { // repeat! same delay, add time
		// use updated time so it isn't repeated too soon
		tevent->set_time();
//            tevent->time = _clock->get_ticks() + tevent->delay;
//            tevent->time = now + tevent->delay;
		add_timer(tevent);
		if (tevent->repeat_count > 0) // don't reduce count if infinite (-1)
			--tevent->repeat_count;
	} else
		delete_timer(tevent); // if not repeated, safe to delete

	return (true);
}


/* Delete a timer, if its can_delete flag is true. Remove from the queue first!
 */
bool TimeQueue::delete_timer(TimedEvent *tevent) {
	if (tevent->tq_can_delete) {
		delete tevent;
		return true;
	}

	return false;
}


/* Accepts delay, immediate toggle(false), and realtime switch(true). It must
 * be queued afterwards to start.
 */
TimedEvent::TimedEvent(uint32 reltime, bool immediate, bool realtime)
	: delay(reltime), repeat_count(0), ignore_pause(false),
	  real_time(realtime), tq_can_delete(true), defunct(false) {
	tq = NULL;

	if (immediate) // start now (useful if repeat is true)
		time = 0;
	else
		set_time();
}


/* Add myself to the TimeQueue.
 */
void TimedEvent::queue() {
	Events *event = Game::get_game()->get_event();
	if (tq == NULL) {
		if (real_time)
			tq = event->get_time_queue();
		else
			tq = event->get_game_time_queue();
		tq->add_timer(this);
	}
}


/* Remove myself from the TimeQueue.
 */
void TimedEvent::dequeue() {
	if (tq) {
		tq->remove_timer(this);
		tq = NULL;
	}
}


/* Add delay to current time and set absolute time.
 */
void TimedEvent::set_time() {
	GameClock *_clock = Game::get_game()->get_clock();
	time = delay + (real_time ? _clock->get_ticks()
	                : _clock->get_game_ticks());

}


/*** TimedPartyMove ***/

/* Party movement to/from dungeon or moongate, with a certain number of
 * milliseconds between each step.
 */
TimedPartyMove::TimedPartyMove(MapCoord *d, MapCoord *t, uint32 step_delay)
	: TimedEvent(step_delay, true) {
	init(d, t, NULL);
}

/* Movement through temporary moongate.
 */
TimedPartyMove::TimedPartyMove(MapCoord *d, MapCoord *t, Obj *use_obj, uint32 step_delay)
	: TimedEvent(step_delay, true) {
	init(d, t, use_obj);
}

TimedPartyMove::TimedPartyMove(uint32 step_delay)
	: TimedEvent(step_delay, true) {
	map_window = NULL;
	party = NULL;
	dest = NULL;
	target = NULL;
	moongate = NULL;
	actor_to_hide = NULL;
	moves_left = 0;
	wait_for_effect = 0;
	falling_in = false;
}

TimedPartyMove::~TimedPartyMove() {
	delete dest;
	delete target;
}

/* Set destination.
 */
void TimedPartyMove::init(MapCoord *d, MapCoord *t, Obj *use_obj) {
	map_window = Game::get_game()->get_map_window();
	party = Game::get_game()->get_party();
	target = NULL;
	moves_left = party->get_party_size() * 2; // step timeout
	wait_for_effect = 0;
	actor_to_hide = NULL;
	falling_in = false;

	dest = new MapCoord(*d);
	if (t)
		target = new MapCoord(*t);
	moongate = use_obj;

	queue(); // start
}

/* Party movement to/from dungeon or to moongate. Repeated until everyone has
 * entered, then the entire party is moved to the destination, and this waits
 * until the visual effects complete.
 */
void TimedPartyMove::timed(uint32 evtime) {
	if (wait_for_effect != 0) { // ignores "falling_in"
		repeat(); // repeat once more (callback() must call stop())
		return;
	}
	stop(); // cancelled further down with repeat(), if still moving

	if (moves_left) {
		if (((falling_in == false) && move_party())
		        || ((falling_in == true) && fall_in()))
			repeat(); // still moving
	} else // timed out, make sure nobody is walking
		for (uint32 m = 0; m < party->get_party_size(); m++)
			party->get_actor(m)->delete_pathfinder();

	// NOTE: set by repeat() or stop()
	if (repeat_count == 0) { // everyone is in position
		if (falling_in == false) { // change location, get in formation
			change_location(); // fade map, move and show party
			party->stop_walking(true); // return control (and change viewpoint)

			// wait for effect or line up now; Party called unpause_user()
			Game::get_game()->pause_user();
			if (wait_for_effect == 0) {
				delay = 50;
				set_time(); // fall-in as fast as possible (but visibly)
				moves_left = party->get_party_size() - 1; // followers
				falling_in = true;
			}
			repeat(); // don't stop yet!
		} else { // already changed location
			Game::get_game()->unpause_user();
			stop_timer(); // done
		}
	}
	if (moves_left > 0)
		--moves_left;
}

/* Assume the teleport-effect is complete. (don't bother checking msg)
 */
uint16 TimedPartyMove::callback(uint16 msg, CallBack *caller, void *data) {
	if (wait_for_effect == 1) { // map-change
		wait_for_effect = 0;
		Game::get_game()->unpause_anims();

		delay = 50;
		set_time(); // fall-in as fast as possible (but visibly)
		moves_left = party->get_party_size() - 1; // followers
		falling_in = true;
	} else if (wait_for_effect == 2) { // vanish
		wait_for_effect = 0;
		Game::get_game()->unpause_anims();
//        move_party();
	}
	return (0);
}

/* Returns true if people are still walking.
 */
bool TimedPartyMove::move_party() {
	bool moving = false; // moving or waiting
	Actor *used_gate = NULL; // someone just stepped into the gate (for effect)

	if (actor_to_hide) {
		hide_actor(actor_to_hide);
		moving = true; // allow at least one more tick so we see last actor hide
	}
	actor_to_hide = NULL;

	for (uint32 a = 0; a < party->get_party_size(); a++) {
		Actor *person = party->get_actor(a);

		if (person->is_visible()) {
			MapCoord loc(person->get_location());
			bool really_visible = map_window->in_window(loc.x, loc.y, loc.z);

			if (a == 0) // FIXME: should be done automatically, but world is frozen
				map_window->centerMapOnActor(person);

			if (loc != *dest && really_visible) {
				// nobody has just used the gate (who may still be vanishing)
				if (!used_gate || loc.distance(*dest) > 1) { // or we aren't close to gate yet
					if (!person->get_pathfinder())
						person->pathfind_to(*dest); // start (or continue) going to gate
					person->update(); // ActorManager is paused
					loc = person->get_location(); // don't use the old location
				} else
					person->delete_pathfinder(); // wait for whoever is at gate
			}
			if (loc == *dest // actor may have just arrived this turn
			        || !really_visible) {
				person->delete_pathfinder();
				if (moongate) used_gate = person; // hide after this turn
				else if (!actor_to_hide) actor_to_hide = person; // hide before next turn
			}
			moving = true; // even if at gate, might not be hidden yet
		}
	}

	if (used_gate) // wait until now (instead of in loop) so others can catch up before effect
		hide_actor(used_gate);
	return (moving);
}

/* Start a visual effect and hide the party member.
 */
void TimedPartyMove::hide_actor(Actor *person) {
	EffectManager *effect_mgr = Game::get_game()->get_effect_manager();
	if (wait_for_effect != 2) {
		if (moongate) { // vanish
			effect_mgr->watch_effect(this, new VanishEffect()); // wait for callback
			wait_for_effect = 2;
			delay = 1;
			set_time(); // effect will be longer than original delay
		}
		person->hide();
		person->move(target->x, target->y, target->z);
	}
}

/* Start a visual effect and move the party to the target.
 */
void TimedPartyMove::change_location() {
	EffectManager *effect_mgr = Game::get_game()->get_effect_manager();
	Graphics::ManagedSurface *mapwindow_capture = NULL;
	if (wait_for_effect != 1) {
		bool is_moongate = moongate != NULL;
		if (moongate && moongate->obj_n == OBJ_U6_RED_GATE) { // leave blue moongates
			// get image before deleting moongate
			mapwindow_capture = map_window->get_sdl_surface();
			// must delete moongate here because dest may be the same as target...
			// remove moongate before moving so the tempobj cleanup doesn't bite us
			Game::get_game()->get_obj_manager()->remove_obj_from_map(moongate);
			delete_obj(moongate);
		}

		if (is_moongate)
			Game::get_game()->get_player()->move(target->x, target->y, target->z, true);
		else
			party->move(target->x, target->y, target->z);
		party->show(); // unhide everyone

		Game::get_game()->get_view_manager()->update(); // we do this to update party view sun moon display if visible.

		if (mapwindow_capture) { // could check this or moongate again
			// start fade-to
			effect_mgr->watch_effect(this, /* call me */
			                         new FadeEffect(FADE_PIXELATED, FADE_OUT, mapwindow_capture));
			SDL_FreeSurface(mapwindow_capture);

			Game::get_game()->pause_anims();
			wait_for_effect = 1;
		}
	}
}


/* Pass a few times so everyone in the party can get into formation.
 * Returns true if party needs to move more to get into formation. */
bool TimedPartyMove::fall_in() {
	bool not_in_position = false; // assume false until someone checks true
	party->follow(0, 0);
	for (uint8 p = 1; p < party->get_party_size(); p++) {
		Actor *follower = party->get_actor(p);
		MapCoord at = follower->get_location(),
		         desired = party->get_formation_coords(p);
		follower->update();
		if (at != desired)
			not_in_position = true;
	}
	return (not_in_position);
}


/*** TimedPartyMoveToVehicle ***/


/* Party movement to vehicle. Second target is unused.
 */
TimedPartyMoveToVehicle::TimedPartyMoveToVehicle(MapCoord *d, Obj *obj,
        uint32 step_delay)
	: TimedPartyMove(d, NULL, step_delay) {
	ship_obj = obj;
}


/* Repeat until everyone is in the boat, then start it up.
 */
void TimedPartyMoveToVehicle::timed(uint32 evtime) {
	stop(); // cancelled further down with repeat(), if still moving
	for (uint32 a = 0; a < party->get_party_size(); a++) {
		Actor *person = party->get_actor(a);
		MapCoord loc(person->get_location());
		// not at boat location
		if (loc != *dest) {
			// offscreen (or timed out), teleport to target
			MapWindow *mapWindow = Game::get_game()->get_map_window();
			if (!mapWindow->in_window(loc.x, loc.y, loc.z) || moves_left == 0)
				person->move(dest->x, dest->y, dest->z, ACTOR_FORCE_MOVE);
			else // keep walking
				person->pathfind_to(*dest);
			person->update();
			repeat(); // repeat once more
		} else { // at destination
			person->delete_pathfinder();
			person->hide(); // set in-vehicle
		}
	}

	if (repeat_count == 0) { // everyone is in the boat
		Game::get_game()->get_usecode()->use_obj(ship_obj);
		party->stop_walking(false); // return control to player
	}
	if (moves_left > 0)
		--moves_left;
}


/* Dump one item at a time out of a container, and print it's name to MsgScroll.
 */
TimedContainerSearch::TimedContainerSearch(Obj *obj) : TimedEvent(500, TIMER_DELAYED) {
	Game *game = Game::get_game();
	scroll = game->get_scroll();
	uc = game->get_usecode();
	om = game->get_obj_manager();

	container_obj = obj;
	prev_obj = NULL;

	//game->set_pause_flags((GamePauseState)(game->get_pause_flags() | PAUSE_USER));
	game->pause_user();
	queue(); // start
}


void TimedContainerSearch::timed(uint32 evtime) {
	prev_obj = uc->get_obj_from_container(container_obj);
	if (prev_obj) {
		scroll->display_string(om->look_obj(prev_obj, true));
		if (container_obj->container->end()) // more objects left
			scroll->display_string(container_obj->container->end()->prev
			                       ? ", " : ", and ");
		repeat();
	} else {
		Game::get_game()->unpause_user();
		stop();
	}
}



/*** TimedCallback ***/
TimedCallback::TimedCallback(CallBack *t, void *d, uint32 wait_time, bool repeat)
	: TimedEvent(wait_time, TIMER_DELAYED, TIMER_REALTIME) {
	set_target(t);
	set_user_data(d);
	repeat_count = repeat ? -1 : 0;

	queue(); // start
}


void TimedCallback::timed(uint32 evtime) {
	if (callback_target)
		message(MESG_TIMED, &evtime);
	else
		stop();
}


GameTimedCallback::GameTimedCallback(CallBack *t, void *d, uint32 wait_time, bool repeating)
	: TimedCallback(t, d, wait_time, repeating) {
	// re-queue timer using game ticks
	dequeue();
	real_time = TIMER_GAMETIME;
	set_time();// change to game time
	queue(); // start
}



/*** TimedAdvance: Advance game time by rate until hours has passed. **/
#define TIMEADVANCE_PER_SECOND 1000 /* frequency of timer calls */
TimedAdvance::TimedAdvance(uint8 hours, uint16 r)
	: TimedCallback(NULL, NULL, 1000 / TIMEADVANCE_PER_SECOND, true),
	  _clock(Game::get_game()->get_clock()),
	  minutes_this_hour(0), minutes(0) {
	init(hours * 60, r);
}


/* Advance to time indicated by timestring, of the format "HH:MM".
 */
TimedAdvance::TimedAdvance(Std::string timestring, uint16 r)
	: TimedCallback(NULL, NULL, 1000 / TIMEADVANCE_PER_SECOND, true),
	  _clock(Game::get_game()->get_clock()),
	  minutes_this_hour(0), minutes(0) {
	uint8 hour = 0, minute = 0;

	get_time_from_string(hour, minute, timestring); // set stop time

	// set number of hours and minutes to advance
	uint16 advance_h = (_clock->get_hour() == hour) ? 24
	                   : (_clock->get_hour() < hour) ? (hour - _clock->get_hour())
	                   : (24 - (_clock->get_hour() - hour));
	uint16 advance_m;
	if (_clock->get_minute() <= minute)
		advance_m = minute - _clock->get_minute();
	else {
		advance_m = (60 - (_clock->get_minute() - minute));
		if (advance_h > 0)
			advance_h -= 1;
		else
			advance_h = 23;
	}
	// go
	init((advance_h * 60) + advance_m, r);
}

/* Set time advance.
 */
void TimedAdvance::init(uint16 min, uint16 r) {
	advance = min;
	rate = r;
	prev_evtime = _clock->get_ticks();
	DEBUG(0, LEVEL_DEBUGGING, "TimedAdvance(): %02d:%02d + %02d:%02d (rate=%d)\n",
	      _clock->get_hour(), _clock->get_minute(), advance / 60, advance % 60, rate);
}


/* Advance game time by rate each second. Timer is stopped after after the time
 * has been advanced as requested.
 */
void TimedAdvance::timed(uint32 evtime) {
	uint32 milliseconds = (evtime - prev_evtime) > 0 ? (evtime - prev_evtime) : 1;
	uint32 fraction = 1000 / milliseconds; // % of second
	uint32 minutes_per_fraction = rate / (fraction > 0 ? fraction : 1);
	bool hour_passed = false; // another hour has passed
	prev_evtime = evtime;

	for (uint32 m = 0; m < minutes_per_fraction; m++) {
		_clock->inc_minute();
		minutes += 1;
		if (++minutes_this_hour > 59) {
			minutes_this_hour = 0;
			hour_passed = true;
		}
		if (time_passed())
			break;
	}
	Game::get_game()->time_changed();

	if (hour_passed && callback_target) // another hour has passed
		message(MESG_TIMED, &evtime);

	if (time_passed()) {
		DEBUG(0, LEVEL_DEBUGGING, "~TimedAdvance(): now %02d:%02d\n", _clock->get_hour(), _clock->get_minute());
		if (callback_target && !hour_passed) // make sure to call target
			message(MESG_TIMED, &evtime);
		stop(); // done
	}
}


/* Returns true when the requested amount of time has passed.
 */
bool TimedAdvance::time_passed() {
	return (minutes >= advance);
}


/* Set hour and minute from "HH:MM" string.
 */
void TimedAdvance::get_time_from_string(uint8 &hour, uint8 &minute, Std::string timestring) {
	char *hour_s = NULL, *minute_s = NULL;
	hour_s = scumm_strdup(timestring.c_str());
	for (uint32 c = 0; c < strlen(hour_s); c++)
		if (hour_s[c] == ':') { // get minutes
			minute_s = scumm_strdup(&hour_s[c + 1]);
			hour_s[c] = '\0';
			break;
		}

	if (hour_s) {
		hour = strtol(hour_s, NULL, 10);
		free(hour_s);
	}
	if (minute_s) {
		minute = strtol(minute_s, NULL, 10);
		free(minute_s);
	}
}


TimedRestGather::TimedRestGather(uint16 x, uint16 y)
	: TimedPartyMove(50) {
	MapCoord center = MapCoord(x, y);
	init(&center, 0, 0); // set dest to campfire location
	Game::get_game()->get_map_window()->updateAmbience();
	check_campfire();
}

/* Repeat until everyone is in the circle. */
void TimedRestGather::timed(uint32 evtime) {
	stop(); // cancelled further down with repeat(), if still moving

	if (moves_left) {
		if (move_party())
			repeat(); // still moving
	} else // timed out, make sure nobody is walking
		for (uint32 m = 0; m < party->get_party_size(); m++)
			party->get_actor(m)->delete_pathfinder();

	if (repeat_count == 0) {
		check_campfire();
		Game::get_game()->get_event()->rest();
	}

	if (moves_left > 0)
		--moves_left;
}

void TimedRestGather::check_campfire() {
	ActorManager *actor_manager = Game::get_game()->get_actor_manager();
	for (sint32 a = 0; a < party->get_party_size(); a++) {
		Actor *actor = party->get_actor(a);
		MapCoord loc = actor->get_location();
		if (loc.x == dest->x && loc.y == dest->y) {
			for (int x = 0; x < 3; x++)
				for (int y = 0; y < 3; y++) {
					if (x == 1 && y == 1)
						continue;
					if (actor_manager->get_actor(dest->x + x - 1, dest->y + y - 1, loc.z) == NULL) {
						actor->move(dest->x + x - 1, dest->y + y - 1, loc.z);

					}
				}

		}
		actor->face_location(dest->x, dest->y);
	}
}

bool TimedRestGather::move_party() {
	bool moving = false; // moving or waiting
	const sint16 positions[3 * 3] = {
		7, 0, 4, // list of party members arranged by location
		3, -1, 2, // campfire is at positions[1][1]
		5, 1, 6
	};

	// check everyone in party because they might not be in the positions list
	for (sint32 a = 0; a < party->get_party_size(); a++) {
		for (int x = 0; x < 3; x++)
			for (int y = 0; y < 3; y++)
				if (positions[x + y * 3] == a) {
					Actor *actor = party->get_actor(a);
					MapCoord loc = actor->get_location();
					MapCoord actor_dest(dest->x + x - 1, dest->y + y - 1, loc.z);
					if (actor_dest == loc) {
						actor->face_location(dest->x, dest->y); // look at camp
						actor->delete_pathfinder();
					} else {
						moving = true; // still moving to circle
						if (actor->get_pathfinder() == 0)
							actor->pathfind_to(actor_dest.x, actor_dest.y);
						actor->set_moves_left(actor->get_dexterity());
						actor->update(); // ActorManager is paused
					}
					x = 3;
					y = 3;
					break; // break to first loop
				}
	}
	return moving;
}

TimedRest::TimedRest(uint8 hours, Actor *who_will_guard, Obj *campfire_obj)
	: TimedAdvance(hours, 80), party(Game::get_game()->get_party()),
	  scroll(Game::get_game()->get_scroll()), sleeping(0),
	  print_message(0) {
	lookout = who_will_guard;
	campfire = campfire_obj;
	number_that_had_food = 0;
}

/* This is the only place we know that the TimedAdvance has completed. */
TimedRest::~TimedRest() {
	//MapCoord loc = Game::get_game()->get_player()->get_actor()->get_location();
	assert(campfire != 0);
	
	campfire->frame_n = 0; // extinguish campfire

	bool can_heal = (Game::get_game()->get_clock()->get_rest_counter() == 0); //only heal once every 12 hours.

	for (int s = 0; s < party->get_party_size(); s++) {
		Actor *actor = party->get_actor(s);

		if (can_heal && actor->is_sleeping() && s < number_that_had_food) {
			//heal actors.
			uint8 hp_diff = actor->get_maxhp() - actor->get_hp();
			if (hp_diff > 0) {
				if (hp_diff == 1)
					hp_diff = 2;

				actor->set_hp(actor->get_hp() + NUVIE_RAND() % (hp_diff / 2) + hp_diff / 2);
				scroll->display_fmt_string("%s has healed.\n", actor->get_name());
			}

		}
		party->get_actor(s)->revert_worktype(); // "wake up"
	}

	if (can_heal)
		Game::get_game()->get_clock()->set_rest_counter(12); //don't heal by resting for another 12 hours.

	Game::get_game()->get_player()->set_mapwindow_centered(true);
	Game::get_game()->unpause_user();
	Game::get_game()->get_event()->endAction(true); // exit Rest mode
}

void TimedRest::timed(uint32 evtime) {
	if (sleeping == false) { // mealtime
		if (evtime - prev_evtime > 500) { // print the next message
			prev_evtime = evtime; // normally set by TimedAdvance::timed()

			if (print_message == 0)
				bard_play(); // Iolo plays a tune.
			else if (print_message <= party->get_party_size())
				eat(party->get_actor(print_message - 1)); // print each person's message
			else {
				sleeping = true; // finished eating
				sleep();
			}
			++print_message;
		}
	} else { // sleeping
		TimedAdvance::timed(evtime);
		for (int s = 0; s < party->get_party_size(); s++)
			party->get_actor(s)->update_time(); // checks status effects

		// FIXME: chance for random enemies to attack
	}
}

/* Check if party has any food, and consume it, allowing the actor to heal. */
void TimedRest::eat(Actor *actor) {
	Obj *food = actor->inventory_get_food(); // search actor's inventory first
	if (!food)
		food = party->get_food();

	if (food) {
		scroll->display_fmt_string("%s has food.\n", actor->get_name());
		Game::get_game()->get_usecode()->destroy_obj(food, 1);
		number_that_had_food++;
	} else
		scroll->display_fmt_string("%s has no food.\n", actor->get_name());
}

/* Look for a bard in the party and have them play a tune. */
void TimedRest::bard_play() {
	scroll->display_string("Mealtime!\n");
	for (int b = 0; b < party->get_party_size(); b++)
		if (party->get_actor(b)->get_obj_n() == OBJ_U6_MUSICIAN) {
			Actor *bard = party->get_actor(b);
			bard->morph(OBJ_U6_MUSICIAN_PLAYING);
			scroll->display_fmt_string("%s plays a tune.\n", bard->get_name());
			break;
		}
}

/* Start sleeping until the requested time. One person can stand guard. */
void TimedRest::sleep() {
	// FIXME: changing to SLEEP worktype should automatically do this
	for (int b = 0; b < party->get_party_size(); b++)
		if (party->get_actor(b)->get_obj_n() == OBJ_U6_MUSICIAN_PLAYING)
			party->get_actor(b)->morph(OBJ_U6_MUSICIAN);

	for (int s = 0; s < party->get_party_size(); s++) {
		Actor *actor = party->get_actor(s);
		if (actor == lookout) {
			actor->set_worktype(WORKTYPE_U6_LOOKOUT);
			scroll->display_fmt_string("\n%s stands guard while the party rests.\n", actor->get_name());
		} else {
			actor->set_worktype(WORKTYPE_U6_SLEEP);
		}
	}
}

} // End of namespace Nuvie
} // End of namespace Ultima
