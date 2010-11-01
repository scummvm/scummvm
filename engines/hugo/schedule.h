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

#include "common/file.h"

namespace Hugo {

#define SIGN(X)       ((X < 0) ? -1 : 1)
#define kMaxEvents     50                           // Max events in event queue

struct event_t {
	act            *action;                         // Ptr to action to perform
	bool            localActionFl;                  // true if action is only for this screen
	uint32          time;                           // (absolute) time to perform action
	struct event_t *prevEvent;                      // Chain to previous event
	struct event_t *nextEvent;                      // Chain to next event
};

class Scheduler {
public:
	Scheduler(HugoEngine *vm);
	virtual ~Scheduler();

	virtual void insertAction(act *action) = 0;
	virtual void restoreEvents(Common::SeekableReadStream *f) = 0;
	virtual void runScheduler() = 0;
	virtual void saveEvents(Common::WriteStream *f) = 0;

	void   decodeString(char *line);
	void   freeActListArr();
	void   initEventQueue();
	void   insertActionList(uint16 actIndex);
	void   loadActListArr(Common::File &in);
	void   loadAlNewscrIndex(Common::File &in);
	void   newScreen(int screenIndex);
	void   processBonus(int bonusIndex);
	void   processMaze();
	void   restoreScreen(int screenIndex);
	void   waitForRefresh(void);

	uint32 getWinTicks();
	uint32 getDosTicks(bool updateFl);

	act    **_actListArr;
	uint16   _actListArrSize;
	uint16   _alNewscrIndex;

protected:
	HugoEngine *_vm;

	enum seqTextSchedule {
		kSsNoBackground = 0,
		kSsBadSaveGame  = 1
	};

	event_t *_freeEvent;                                // Free list of event structures
	event_t *_headEvent;                                // Head of list (earliest time)
	event_t *_tailEvent;                                // Tail of list (latest time)
	event_t  _events[kMaxEvents];                       // Statically declare event structures

	virtual const char *getCypher() = 0;
	virtual void delQueue(event_t *curEvent) = 0;
	virtual event_t *doAction(event_t *curEvent) = 0;

	event_t *getQueue();
};

class Scheduler_v1d : public Scheduler {
public:
	Scheduler_v1d(HugoEngine *vm);
	~Scheduler_v1d();

	virtual const char *getCypher();
	virtual void insertAction(act *action);
	virtual void restoreEvents(Common::SeekableReadStream *f);
	virtual void saveEvents(Common::WriteStream *f);
	virtual void runScheduler();
protected:
	virtual void delQueue(event_t *curEvent);
	virtual event_t *doAction(event_t *curEvent);
};

class Scheduler_v2d : public Scheduler_v1d {
public:
	Scheduler_v2d(HugoEngine *vm);
	virtual ~Scheduler_v2d();

	virtual void insertAction(act *action);
protected:
	virtual void delQueue(event_t *curEvent);
	virtual event_t *doAction(event_t *curEvent);
};

class Scheduler_v3d : public Scheduler_v2d {
public:
	Scheduler_v3d(HugoEngine *vm);
	~Scheduler_v3d();

	const char *getCypher();
protected:
	virtual event_t *doAction(event_t *curEvent);

};

class Scheduler_v1w : public Scheduler_v3d {
public:
	Scheduler_v1w(HugoEngine *vm);
	~Scheduler_v1w();

	virtual event_t *doAction(event_t *curEvent);
	void insertAction(act *action);
	void restoreEvents(Common::SeekableReadStream *f);
	void runScheduler();
	void saveEvents(Common::WriteStream *f);
};
} // End of namespace Hugo
#endif //HUGO_SCHEDULE_H
