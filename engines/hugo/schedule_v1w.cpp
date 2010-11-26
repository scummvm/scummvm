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

// This module contains all the scheduling and timing stuff

#include "common/system.h"

#include "hugo/game.h"
#include "hugo/hugo.h"
#include "hugo/schedule.h"
#include "hugo/global.h"
#include "hugo/file.h"
#include "hugo/display.h"
#include "hugo/parser.h"
#include "hugo/util.h"
#include "hugo/sound.h"
#include "hugo/object.h"

namespace Hugo {

Scheduler_v1w::Scheduler_v1w(HugoEngine *vm) : Scheduler_v3d(vm) {
}

Scheduler_v1w::~Scheduler_v1w() {
}

uint32 Scheduler_v1w::getTicks() {
	return getWinTicks();
}

/**
* This is the scheduler which runs every tick.  It examines the event queue
* for any events whose time has come.  It dequeues these events and performs
* the action associated with the event, returning it to the free queue
*/
void Scheduler_v1w::runScheduler() {
	debugC(6, kDebugSchedule, "runScheduler");

	uint32 ticker = getTicks();                     // The time now, in ticks
	event_t *curEvent = _headEvent;                 // The earliest event

	while (curEvent && (curEvent->time <= ticker))  // While mature events found
		curEvent = doAction(curEvent);              // Perform the action (returns next_p)

	_vm->getGameStatus().tick++;                    // Accessed elsewhere via getTicks()
}
} // End of namespace Hugo
