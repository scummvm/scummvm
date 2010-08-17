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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $URL$
 * $Id$
 *
 */

/*
 * This code is based on original Hugo Trilogy source code
 *
 * Copyright (c) 1989-1995 David P. Gray
 *
 */

#ifndef HUGO_SCHEDULE_H
#define HUGO_SCHEDULE_H

namespace Hugo {

#define kMaxEvents     50                           /* Max events in event queue */

struct event_t {
	act            *action;                         /* Ptr to action to perform */
	bool            localActionFl;                  /* TRUE if action is only for this screen */
	uint32          time;                           /* (absolute) time to perform action */
	struct event_t *prevEvent;                      /* Chain to previous event */
	struct event_t *nextEvent;                      /* Chain to next event */
};

class Scheduler {
public:
	Scheduler(HugoEngine &vm);

	void   initEventQueue();
	void   insertAction(act *action);
	void   insertActionList(uint16 actIndex);
	void   decodeString(char *line);
	void   runScheduler();
	uint32 getTicks();
	void   processBonus(int bonusIndex);
	void   newScreen(int screenIndex);
	void   restoreEvents(Common::SeekableReadStream *f);
	void   saveEvents(Common::WriteStream *f);
	void   restoreScreen(int screenIndex);
	void   swapImages(int objNumb1, int objNumb2);

private:
	enum seqTextSchedule {
		kSsNoBackground = 0,
		kSsBadSaveGame  = 1
	};

	HugoEngine &_vm;

	event_t _events[kMaxEvents];                        /* Statically declare event structures */

	event_t *_freeEvent;                                /* Free list of event structures */
	event_t *_headEvent;                                /* Head of list (earliest time) */
	event_t *_tailEvent;                                /* Tail of list (latest time) */

	event_t *getQueue();
	void     delQueue(event_t *curEvent);
	event_t *doAction(event_t *curEvent);
};

} // end of namespace Hugo
#endif //HUGO_SCHEDULE_H
