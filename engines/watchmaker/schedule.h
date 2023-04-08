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
#include "watchmaker/message.h"

namespace Watchmaker {

void Event(EventClass classe, uint8 event, uint16 flags, int16 wparam1, int16 wparam2,
           uint8 bparam, void *p0, void *p1, void *p2);

void ProcessTheMessage(WGame &game);
void ReEvent();
void DeleteWaitingMsgs(uint16 flags);
void AddWaitingMsgs(uint16 flags);

class MessageSystem {
public:
	void init();
	void removeEvent(EventClass classe, uint8 event);
	void removeEvent_bparam(EventClass classe, uint8 event, uint8 bparam);
	void scheduler(); // TODO: Rename
	message GameMessage[MAX_MESSAGES];
	pqueue Game;
	// message *TheMessage; // TODO: Replace the global variable with this
	message idlemessage  = {EventClass::MC_IDLE, 0, MP_DEFAULT};
	uint8 SuperEventActivate;
private:
	void initQueue(pqueue *lq);
};

} // End of namespace Watchmaker

#endif // WATCHMAKER_SCHEDULE_H
