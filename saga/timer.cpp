/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004 The ScummVM project
 *
 * The ReInherit Engine is (C)2000-2003 by Daniel Balsom.
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Header$
 *
 */

#include "saga.h"
#include "timer.h"

// FIXME: replace calls to this with direct OSystem calls

namespace Saga {

struct R_SYSTIMER {
	int t_running;
	unsigned long t_interval;
	void *t_param;
};

struct R_SYSTIMER_DATA {
	int initialized;
	uint32 t_start_ticks;
	uint32 t_current_ticks;
	uint32 t_previous_ticks;
};

static R_SYSTIMER_DATA R_TimerData;

int SYSTIMER_InitMSCounter() {
	if (R_TimerData.initialized) {
		return R_FAILURE;
	}

	R_TimerData.t_previous_ticks = g_system->get_msecs();
	R_TimerData.initialized = 1;

	return R_SUCCESS;
}

unsigned long SYSTIMER_ReadMSCounter() {
	uint32 ms_elapsed = 0;

	if (!R_TimerData.initialized) {
		return 0;
	}

	R_TimerData.t_current_ticks = g_system->get_msecs();

	if (R_TimerData.t_current_ticks < R_TimerData.t_previous_ticks) {
		// Timer has rolled over after 49 days
	} else {
		ms_elapsed = R_TimerData.t_current_ticks - R_TimerData.t_previous_ticks;
		R_TimerData.t_previous_ticks = R_TimerData.t_current_ticks;
	}

	return ms_elapsed;
}

int SYSTIMER_ResetMSCounter() {
	if (!R_TimerData.initialized) {
		return R_FAILURE;
	}

	R_TimerData.t_previous_ticks = g_system->get_msecs();

	return R_SUCCESS;
}

int SYSTIMER_Sleep(uint16 msec) {
	g_system->delay_msecs(msec);

	return R_SUCCESS;
}


} // End of namespace Saga
