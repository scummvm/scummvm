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
#ifndef SAGA_SYSTIMER_H__
#define SAGA_SYSTIMER_H__

namespace Saga {

typedef struct R_SYSTIMER_DATA_tag {

	int initialized;

	Uint32 t_start_ticks;

	Uint32 t_current_ticks;
	Uint32 t_previous_ticks;

} R_SYSTIMER_DATA;

struct R_SYSTIMER_tag {

	int t_running;

	unsigned long t_interval;
	void *t_param;

	R_SYSTIMER_CALLBACK t_callback_f;
	SDL_TimerID t_sdl_timerid;
};

Uint32         SYSTIMER_Callback(Uint32 interval, void *param);

} // End of namespace Saga

#endif				/* SAGA_SYSTIMER_H__ */
