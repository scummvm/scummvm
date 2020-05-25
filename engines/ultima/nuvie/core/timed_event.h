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

#ifndef NUVIE_CORE_TIMED_EVENT_H
#define NUVIE_CORE_TIMED_EVENT_H

#include "ultima/shared/std/string.h"
#include "ultima/nuvie/misc/call_back.h"
#include "ultima/nuvie/core/obj_manager.h"

namespace Ultima {
namespace Nuvie {

class Actor;
class CallBack;
class Events;
class GameClock;
class MapCoord;
class MapWindow;
class MsgScroll;
class Party;
class TimedCallbackTarget;
class TimedEvent;

/* A queue for our events.
 */
class TimeQueue {
	Std::list<TimedEvent *> tq;
public:
	TimeQueue() : tq() { }
	~TimeQueue() {
		clear();
	}

	bool empty() const {
		return (tq.empty());
	}
	void clear();
	void add_timer(TimedEvent *tevent);
	void remove_timer(TimedEvent *tevent);
	TimedEvent *pop_timer();
	bool delete_timer(TimedEvent *tevent);

	bool call_timer(uint32 now); // activate
	void call_timers(uint32 now); // activate all
};


#define TIMER_IMMEDIATE true
#define TIMER_DELAYED false
#define TIMER_REALTIME true
#define TIMER_GAMETIME false

/* Events activated by a timer. Add to one of the Events time-queues to start.
 * (THERE IS ONLY ONE SET)  The timed() method is called on activation,
 * and the timer may be automatically deleted or repeated.
 */
class TimedEvent {
	friend class TimeQueue;
	friend class Events;
protected:
	TimeQueue *tq; // the TimeQueue; so we can add ourself
	uint32 delay, time; // timer delay, and next absolute time to activate
	sint8 repeat_count; // repeat how many times? (-1=infinite;0=stop)
	bool ignore_pause; // activates even if game is paused
	bool real_time; // time and delay is in milliseconds (false=game ticks/turns)
	bool tq_can_delete; // can TimeQueue delete this TimedEvent when done?
	bool defunct; // deleted; don't activate (use to stop timers from outside)

public:
	TimedEvent(uint32 reltime, bool immediate = TIMER_DELAYED, bool realtime = TIMER_REALTIME);
	virtual ~TimedEvent() { }
	virtual void timed(uint32 evtime) {
		DEBUG(0, LEVEL_ERROR, "TimedEvent: undefined timer method\n");
	}

protected:
	// stop repeating, remove from tq if it won't delete it
	// NOTE: potential for bug here, this doesn't prevent it from being called once more
	void stop() {
		repeat_count = 0;
		if (!tq_can_delete) dequeue();
	}
	// repeat once (or for requested count)
	void repeat(uint32 count = 1) {
		repeat_count = count;
	}

public:
	void queue(); // set tq, add to tq
	void dequeue(); // remove from tq, clear tq

	void set_time(); // set `time' from `delay'
	void stop_timer() {
		stop();
		defunct = true;
	}
};


/* Print to stdout. (timer test)
 */
class TimedMessage : public TimedEvent {
	Std::string msg;
public:
	TimedMessage(uint32 reltime, const char *m, bool repeating = false)
		: TimedEvent(reltime), msg(m) {
		repeat_count = repeating ? -1 : 0;
	}
	void timed(uint32 evtime) override {
		DEBUG(0, LEVEL_NOTIFICATION, "Activate! evtime=%d msg=\"%s\"\n", evtime, msg.c_str());
	}
};


/* Move the party to/from a dungeon or ladder or moongate. Characters off-screen
 * will teleport.
 */
class TimedPartyMove : public TimedEvent, public CallBack {
protected:
	MapWindow *map_window;
	Party *party; // the party
	MapCoord *dest; // destination, where all actors walk to and disappear
	MapCoord *target; // where they reappear at the new plane
	uint32 moves_left; // walk timeout
	Obj *moongate; // if using a moongate
	uint8 wait_for_effect; // waiting for a visual effect to complete if not 0
	Actor *actor_to_hide; // this actor has reached exit and should be hidden
	bool falling_in;

public:
	TimedPartyMove(MapCoord *d, MapCoord *t, uint32 step_delay = 500);
	TimedPartyMove(MapCoord *d, MapCoord *t, Obj *use_obj, uint32 step_delay = 500);
	TimedPartyMove(uint32 step_delay = 500);
	~TimedPartyMove() override;
	void init(MapCoord *d, MapCoord *t, Obj *use_obj);
	void timed(uint32 evtime) override;

	uint16 callback(uint16 msg, CallBack *caller, void *data = NULL) override;

protected:
	bool move_party();
	bool fall_in();
	void hide_actor(Actor *person);
	void change_location();
};


/* Move the party into a vehicle and start it when everyone is there.
 */
class TimedPartyMoveToVehicle : public TimedPartyMove {
	Obj *ship_obj; // vehicle center
public:
	TimedPartyMoveToVehicle(MapCoord *d, Obj *obj, uint32 step_delay = 125);
	void timed(uint32 evtime) override;
};


#if 0
class TimedRTC : public TimedEvent {
public:
	TimedRTC() : TimedEvent(1000) {
		repeat_count = -1;
	}
	void timed(uint32 evtime) {
//        Game::get_game()->get_player()->pass();
	}
};
#endif


//FIXME: It isnt container search. Its a msgscroll effect to print one line at a time.
/* Dump one item at a time out of a container, and print it's name to MsgScroll.
 */
class TimedContainerSearch : public TimedEvent {
	MsgScroll *scroll;
	UseCode *uc;
	ObjManager *om;
	Obj *container_obj;
	Obj *prev_obj; // removed from container
public:
	TimedContainerSearch(Obj *obj);
	void timed(uint32 evtime) override;
};


/* Send timer message to callback target after `wait_time' is up, passing it
 * some target-defined data.
 *  new TimedCallback(PowderKeg, (void *)my_powderkeg_data, time_to_explode);
 */
class TimedCallback : public TimedEvent, public CallBack {
public:
	TimedCallback(CallBack *t, void *d, uint32 wait_time,
	              bool repeat = false);
	~TimedCallback() override {  }
	void timed(uint32 evtime) override;
	void clear_target() {
		set_target(NULL);
	}
};


class GameTimedCallback : public TimedCallback {
public:
	GameTimedCallback(CallBack *t, void *d, uint32 wait_time, bool repeat = false);
	~GameTimedCallback() override {  }
};


/* Advance gameclock up to 24hours from start time. The callback is used every
 * hour from the start time, up to and including the stop time.
 */
class TimedAdvance : public TimedCallback {
	GameClock *_clock;
	uint16 advance; // minutes requested
	uint8 minutes_this_hour;
protected:
	uint16 minutes; // minutes advanced
	uint16 rate; // rate is minutes-per-second
	uint32 prev_evtime; // last time the timer was called

public:
	TimedAdvance(uint8 hours, uint16 r = 60);
	TimedAdvance(Std::string timestring, uint16 r = 60); // "HH:MM"
	~TimedAdvance() override { }

	void init(uint16 min, uint16 r); // start time advance

	void timed(uint32 evtime) override;
	bool time_passed(); // returns true if stop time has passed
	void get_time_from_string(uint8 &hour, uint8 &minute, Std::string timestring);
};


/* Camping in the wilderness. Move everyone into a circle and place a campfire
 * in the center.
 */
class TimedRestGather : public TimedPartyMove {
public:
	TimedRestGather(uint16 x, uint16 y);

	void timed(uint32 evtime) override;

protected:
	bool move_party();
	void check_campfire();
};

/* Camping in the wilderness. Do a TimedAdvance until the requested time. The
 * camp can be broken by nearby foes.
 */
class TimedRest : public TimedAdvance {
	Party *party;
	MsgScroll *scroll;
	Actor *lookout;
	bool sleeping; // false: mealtime, true: sleeping
	uint8 print_message; // which message is to be printed next
	Obj *campfire;
	uint8 number_that_had_food;
public:
	TimedRest(uint8 hours, Actor *lookout, Obj *campfire_obj);
	~TimedRest() override;

	void timed(uint32 evtime) override;
	void eat(Actor *actor);
	void bard_play();
	void sleep();
};

} // End of namespace Nuvie
} // End of namespace Ultima

#endif
