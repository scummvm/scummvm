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
/*

 Description:   
 
    Event module public header

 Notes: 
*/

#ifndef SAGA_EVENTS_MOD_H
#define SAGA_EVENTS_MOD_H

namespace Saga {

enum R_EVENT_TYPES {

	R_ONESHOT_EVENT,
	R_CONTINUOUS_EVENT,
	R_INTERVAL_EVENT
};

enum R_EVENT_FLAGS {

	R_SIGNALED = 0x8000,
	R_NODESTROY = 0x4000
};

enum R_EVENT_CODES {

	R_BG_EVENT = 1,
	R_ANIM_EVENT,
	R_MUSIC_EVENT,
	R_VOICE_EVENT,
	R_SOUND_EVENT,
	R_SCENE_EVENT,
	R_TEXT_EVENT,
	R_PAL_EVENT,
	R_PALANIM_EVENT,
	R_TRANSITION_EVENT,
	R_INTERFACE_EVENT,
	R_CONSOLE_EVENT,
	R_ACTOR_EVENT
};

enum R_EVENT_OPS {

	/* INSTANTANEOUS events
	 * \*------------------------------------------------------------- */
	/* BG events */
	EVENT_DISPLAY = 1,
	/* ANIM events */
	EVENT_FRAME = 1,
	/* MUISC & SOUND events */
	EVENT_PLAY = 1,
	EVENT_STOP = 2,
	/* SCENE events */
	EVENT_END = 2,
	/* TEXT events */
	EVENT_HIDE = 2,
	EVENT_REMOVE = 3,
	/* PALANIM events */
	EVENT_CYCLESTART = 1,
	EVENT_CYCLESTEP = 2,
	/* INTERFACE events */
	EVENT_ACTIVATE = 1,
	EVENT_DEACTIVATE,
	/* ACTOR events */
	EVENT_MOVE = 1,

	/* CONTINUOUS events
	 * \*------------------------------------------------------------- */
	/* PALETTE events */
	EVENT_PALTOBLACK = 1,
	EVENT_BLACKTOPAL = 2,
	/* TRANSITION events */
	EVENT_DISSOLVE = 1
};

enum R_EVENT_PARAMS {

	NO_SET_PALETTE,
	SET_PALETTE
};

struct R_EVENT {

	unsigned int type;
	unsigned int code;	/* Event operation category & flags */
	int op;			/* Event operation */

	long param;		/* Optional event parameter */
	long param2;

	void *data;		/* Optional event data */

	long time;		/* Elapsed time until event */
	long duration;		/* Duration of event */
	long d_reserved;

	R_EVENT *chain;	/* Event chain 
					 * (For consecutive events) */

	R_EVENT() { memset(this, 0, sizeof(*this)); }

};

int EVENT_Init(void);

int EVENT_Shutdown(void);

int EVENT_HandleEvents(long msec);

int EVENT_ClearList(void);

int EVENT_FreeList(void);

R_EVENT *EVENT_Queue(R_EVENT * event);

R_EVENT *EVENT_Chain(R_EVENT * head_event, R_EVENT * add_event);

} // End of namespace Saga

#endif				/* R_EVENTS_MOD_H */
