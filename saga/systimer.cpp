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
#include "reinherit.h"

#include <SDL.h>

/*
 * Begin module
\*--------------------------------------------------------------------------*/
#include "systimer.h"

namespace Saga {

struct R_SYSTIMER {

	int t_running;

	unsigned long t_interval;
	void *t_param;

	R_SYSTIMER_CALLBACK t_callback_f;
	SDL_TimerID t_sdl_timerid;
};

struct R_SYSTIMER_DATA {

   int initialized;

   Uint32 t_start_ticks;

   Uint32 t_current_ticks;
   Uint32 t_previous_ticks;

};

static R_SYSTIMER_DATA R_TimerData;

static Uint32         SYSTIMER_Callback(Uint32 interval, void *param);

int SYSTIMER_InitMSCounter(void)
{

	if (R_TimerData.initialized) {
		return R_FAILURE;
	}

	R_TimerData.t_previous_ticks = SDL_GetTicks();

	R_TimerData.initialized = 1;

	return R_SUCCESS;
}

unsigned long SYSTIMER_ReadMSCounter(void)
{

	Uint32 ms_elapsed = 0;

	if (!R_TimerData.initialized) {
		return 0;
	}

	R_TimerData.t_current_ticks = SDL_GetTicks();

	if (R_TimerData.t_current_ticks < R_TimerData.t_previous_ticks) {
		/* Timer has rolled over after 49 days... */
	} else {
		ms_elapsed = R_TimerData.t_current_ticks -
		    R_TimerData.t_previous_ticks;

		R_TimerData.t_previous_ticks = R_TimerData.t_current_ticks;
	}

	return ms_elapsed;
}

int SYSTIMER_ResetMSCounter(void)
{

	if (!R_TimerData.initialized) {
		return R_FAILURE;
	}

	R_TimerData.t_previous_ticks = SDL_GetTicks();

	return R_SUCCESS;
}

int SYSTIMER_Sleep(uint16 msec)
{
	SDL_Delay(msec);

	return R_SUCCESS;
}

int
SYSTIMER_CreateTimer(R_SYSTIMER ** timer,
    unsigned long interval, void *param, R_SYSTIMER_CALLBACK callback)
{
	R_SYSTIMER *new_timer = (R_SYSTIMER *)malloc(sizeof *new_timer);
	if (new_timer == NULL) {
		return R_MEM;
	}

	new_timer->t_interval = interval;
	new_timer->t_param = param;
	new_timer->t_callback_f = callback;

	*timer = new_timer;

	new_timer->t_sdl_timerid = SDL_AddTimer(interval,
	    SYSTIMER_Callback, new_timer);

	if (new_timer->t_sdl_timerid == NULL) {
		free(new_timer);
		*timer = NULL;

		return R_FAILURE;
	}

	return R_SUCCESS;
}

int SYSTIMER_DestroyTimer(R_SYSTIMER * timer)
{
	if (timer == NULL) {
		return R_FAILURE;
	}

	timer->t_running = 0;

	SDL_RemoveTimer(timer->t_sdl_timerid);

	free(timer);

	return R_SUCCESS;
}

Uint32 SYSTIMER_Callback(Uint32 interval, void *param)
{
	R_SYSTIMER *timer_p = (R_SYSTIMER *)param;

	timer_p->t_callback_f(timer_p->t_interval, timer_p->t_param);

	return timer_p->t_interval;
}

} // End of namespace Saga
