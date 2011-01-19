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

	virtual void decodeString(char *line) = 0;
	virtual void runScheduler() = 0;

	void freeActListArr();
	void initEventQueue();
	void insertActionList(uint16 actIndex);
	void loadActListArr(Common::File &in);
	void loadAlNewscrIndex(Common::File &in);
	void newScreen(int screenIndex);
	void processBonus(int bonusIndex);
	void processMaze(int x1, int x2, int y1, int y2);
	void restoreScreen(int screenIndex);
	void restoreEvents(Common::SeekableReadStream *f);
	void saveEvents(Common::WriteStream *f);
	void waitForRefresh(void);

protected:
	HugoEngine *_vm;

	uint16   _actListArrSize;
	uint16   _alNewscrIndex;

	event_t *_freeEvent;                                // Free list of event structures
	event_t *_headEvent;                                // Head of list (earliest time)
	event_t *_tailEvent;                                // Tail of list (latest time)
	event_t  _events[kMaxEvents];                       // Statically declare event structures

	act    **_actListArr;

	virtual const char *getCypher() = 0;

	virtual uint32 getTicks() = 0;

	virtual void delEventType(action_t actTypeDel) = 0;
	virtual void delQueue(event_t *curEvent) = 0;
	virtual void promptAction(act *action) = 0;

	event_t *doAction(event_t *curEvent);
	event_t *getQueue();

	uint32 getDosTicks(bool updateFl);
	uint32 getWinTicks();

	void insertAction(act *action);
};

class Scheduler_v1d : public Scheduler {
public:
	Scheduler_v1d(HugoEngine *vm);
	~Scheduler_v1d();

	virtual void decodeString(char *line);
	virtual void runScheduler();

protected:
	virtual const char *getCypher();

	virtual uint32 getTicks();

	virtual void delEventType(action_t actTypeDel);
	virtual void delQueue(event_t *curEvent);
	virtual void promptAction(act *action);
};

class Scheduler_v2d : public Scheduler_v1d {
public:
	Scheduler_v2d(HugoEngine *vm);
	virtual ~Scheduler_v2d();

	void decodeString(char *line);

protected:
	virtual const char *getCypher();
	
	void delEventType(action_t actTypeDel);
	void delQueue(event_t *curEvent);
	void promptAction(act *action);
};

class Scheduler_v3d : public Scheduler_v2d {
public:
	Scheduler_v3d(HugoEngine *vm);
	~Scheduler_v3d();

protected:
	const char *getCypher();
};

class Scheduler_v1w : public Scheduler_v3d {
public:
	Scheduler_v1w(HugoEngine *vm);
	~Scheduler_v1w();

	void runScheduler();

protected:
	uint32 getTicks();

};
} // End of namespace Hugo
#endif //HUGO_SCHEDULE_H
