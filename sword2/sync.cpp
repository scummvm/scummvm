/* Copyright (C) 1994-2004 Revolution Software Ltd
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 */

#include "common/stdafx.h"
#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/interpreter.h"

namespace Sword2 {

int32 Logic::fnSendSync(int32 *params) {
	// params:	0 sync's recipient
	//		1 sync value

	for (int i = 0; i < MAX_syncs; i++) {
		if (_syncList[i].id == 0) {
			debug(5, " %d sending sync %d to %d", ID, params[1], params[0]);
			_syncList[i].id = params[0];
			_syncList[i].sync = params[1];
			return IR_CONT;
		}
	}

	// The original code didn't even check for this condition, so maybe
	// it should be a fatal error?

	warning("No free sync slot");
	return IR_CONT;
}

void Logic::clearSyncs(uint32 id) {
	// clear any syncs registered for this id
	// call this just after the id has been processed
	// there could in theory be more than one sync waiting for us so
	// clear the lot

	for (int i = 0; i < MAX_syncs; i++) {
		if (_syncList[i].id == id) {
			debug(5, "removing sync %d for %d", i, id);
			_syncList[i].id = 0;
		}
	}
}

bool Logic::getSync(void) {
	// check for a sync waiting for this character
	// - called from system code eg. from inside fnAnim(), to see if
	// animation to be quit

	for (int i = 0; i < MAX_syncs; i++) {
		if (_syncList[i].id == ID) {
			// means sync found
			return true;
		}
	}

	// no sync found
	return false;
}

int32 Logic::fnGetSync(int32 *params) {
	// check for a sync waiting for this character
	// - called from script

	// params:	none

	for (int i = 0; i < MAX_syncs; i++) {
		if (_syncList[i].id == ID) {
			// return sync value waiting
			RESULT = _syncList[i].sync;
			return IR_CONT;
		}
	}

	// no sync found
	RESULT = 0;
	return IR_CONT;
}

int32 Logic::fnWaitSync(int32 *params) {
	// keep calling until a sync received

	// params:	none

	debug(5, "fnWaitSync: %d waits", ID);

	for (int i = 0; i < MAX_syncs; i++) {
		if (_syncList[i].id == ID) {
			// return sync value waiting
			debug(5, "fnWaitSync: go");
			RESULT = _syncList[i].sync;
			return IR_CONT;
		}
	}

	// back again next cycle
	return IR_REPEAT;
}

} // End of namespace Sword2
