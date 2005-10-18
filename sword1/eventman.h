/* ScummVM - Scumm Interpreter
 * Copyright (C) 2003-2005 The ScummVM project
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * $Header$
 *
 */

#ifndef BSEVENTMAN_H
#define BSEVENTMAN_H

#include "sword1/object.h"

namespace Sword1 {

#define TOTAL_EVENT_SLOTS 20

struct GlobalEvent {
	int32 eventNumber;
	int32 delay;
};

class EventManager {
public:
	EventManager(void);
	void serviceGlobalEventList(void);
	void checkForEvent(Object *compact);
	int fnCheckForEvent(Object *cpt, int32 id, int32 pause);
	void fnIssueEvent(Object *compact, int32 id, int32 event, int32 delay);
	bool eventValid(int32 event);
private:
	GlobalEvent _eventPendingList[TOTAL_EVENT_SLOTS];
};

} // End of namespace Sword1

#endif // BSEVENTMAN_H
