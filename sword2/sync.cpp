/* Copyright (C) 1994-2003 Revolution Software Ltd
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

#include "stdafx.h"
#include "debug.h"
#include "defs.h"
#include "interpreter.h"
#include "sync.h"

typedef	struct {
	uint32 id;
	uint32 sync;
} _sync_unit;	// haaaaaaaa

// there wont be many will there. probably 2 at most i reckon
#define	MAX_syncs 10

_sync_unit sync_list[MAX_syncs];

void Init_sync_system(void) {		// Tony27Nov96
	// set list to 0's
	for (int j = 0; j < MAX_syncs; j++)
		sync_list[j].id = 0;
}

int32 FN_send_sync(int32 *params) {	// Tony27Nov96
	//param	0 sync's recipient
	//param	1 sync value

	for (int i = 0; i < MAX_syncs; i++) {
		if (sync_list[i].id == 0) {
			// Zdebug(" %d sending sync %d to %d", ID, params[1], params[0]);
			sync_list[i].id = params[0];
			sync_list[i].sync = params[1];
			return IR_CONT;
		}
	}

	// The original code didn't even check for this condition, so maybe
	// it should be a fatal error?

	warning("No free sync slot");
	return IR_CONT;
}

void Clear_syncs(uint32	id) {		// Tony27Nov96
	// clear any syncs registered for this id
	// call this just after the id has been processed
	// there could in theory be more than one sync waiting for us so
	// clear the lot

	for (int i = 0; i < MAX_syncs; i++) {
		if (sync_list[i].id == id) {
			// Zdebug("removing sync %d for %d", i, id);
			sync_list[i].id = 0;
		}
	}
}

uint32 Get_sync(void) {			// Tony27Nov96
	// check for a sync waiting for this character
	// - called from system code eg. from inside FN_anim(), to see if
	// animation to be quit

	for (int i = 0; i < MAX_syncs; i++) {
		if (sync_list[i].id == ID) {
			// means sync found   Tony12July97
			return 1;
		}
	}

	// no sync found
	return 0;
}

int32 FN_get_sync(int32 *params) {	// Tony27Nov96
	// check for a sync waiting for this character
	// - called from script
	// params     none

	for (int i = 0; i < MAX_syncs; i++) {
		if (sync_list[i].id == ID) {
			// return sync value waiting
			RESULT = sync_list[i].sync;
			return IR_CONT;
		}
	}

	// no sync found
	RESULT = 0;
	return IR_CONT;
}

int32 FN_wait_sync(int32 *params) {	// Tony27Nov96
	// keep calling until a sync recieved
	// params     none

	// Zdebug("%d waits", ID);

	for (int i = 0; i < MAX_syncs; i++) {
		if (sync_list[i].id == ID) {
			// return sync value waiting
			//Zdebug(" go");
			RESULT = sync_list[i].sync;
			return IR_CONT;
		}
	}

	// back again next cycle
	return IR_REPEAT;
}
