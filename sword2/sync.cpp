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
#include "sword2/logic.h"

namespace Sword2 {

int32 Logic::fnSendSync(int32 *params) {
	// params:	0 sync's recipient
	//		1 sync value

	for (int i = 0; i < MAX_syncs; i++) {
		if (_syncList[i].id == 0) {
			debug(5, "%d sends sync %d to %d", _scriptVars[ID], params[1], params[0]);
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

/**
 * Clear any syncs registered for this id. Call this just after the id has been
 * processed. Theoretically there could be more than one sync waiting for us,
 * so clear the lot.
 */

void Logic::clearSyncs(uint32 id) {
	for (int i = 0; i < MAX_syncs; i++) {
		if (_syncList[i].id == id) {
			debug(5, "removing sync %d for %d", i, id);
			_syncList[i].id = 0;
		}
	}
}

/**
 * Check for a sync waiting for this character. Called from fnAnim() to see if
 * animation is to be finished. Returns an index into _syncList[], or -1.
 */

int Logic::getSync(void) {
	for (int i = 0; i < MAX_syncs; i++) {
		if (_syncList[i].id == _scriptVars[ID])
			return i;
	}

	return -1;
}

/**
 * Like getSync(), but called from scripts. Sets the RESULT variable to
 * the sync value, or 0 if none is found.
 */

int32 Logic::fnGetSync(int32 *params) {
	// params:	none

	int slot = getSync();

	_scriptVars[RESULT] = (slot != -1) ? _syncList[slot].sync : 0;
	return IR_CONT;
}

/**
 * Wait for sync to happen. Sets the RESULT variable to the sync value, once
 * it has been found.
 */

int32 Logic::fnWaitSync(int32 *params) {
	// params:	none

	debug(6, "fnWaitSync: %d waits", _scriptVars[ID]);

	int slot = getSync();

	if (slot == -1)
		return IR_REPEAT;

	debug(5, "fnWaitSync: %d got sync %d", _scriptVars[ID], _syncList[slot].sync);
	_scriptVars[RESULT] = _syncList[slot].sync;
	return IR_CONT;
}

} // End of namespace Sword2
