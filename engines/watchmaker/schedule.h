/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#ifndef WATCHMAKER_SCHEDULE_H
#define WATCHMAKER_SCHEDULE_H

#include "watchmaker/types.h"
#include "watchmaker/struct.h"
#include "watchmaker/game.h"
#include "watchmaker/message.h"

namespace Watchmaker {

void Event(EventClass classe, uint8 event, uint16 flags, int16 wparam1, int16 wparam2,
		   uint8 bparam, void *p0, void *p1, void *p2);

void InitMessageSystem();
void Scheduler();
void ProcessTheMessage(WGame &game);
void ReEvent();
void DeleteWaitingMsgs(uint16 flags);
void AddWaitingMsgs(uint16 flags);
void RemoveEvent(pqueue *lq, EventClass classe, uint8 event);
void RemoveEvent_bparam(pqueue *lq, EventClass classe, uint8 event, uint8 bparam);

} // End of namespace Watchmaker

#endif // WATCHMAKER_SCHEDULE_H
