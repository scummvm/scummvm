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
 
    Event management module

 Notes: 
*/

#include "saga.h"
#include "reinherit.h"

#include "yslib.h"

/*
 * Uses the following modules:
\*--------------------------------------------------------------------------*/
#include "animation_mod.h"
#include "console_mod.h"
#include "scene_mod.h"
#include "gfx_mod.h"
#include "interface_mod.h"
#include "text_mod.h"
#include "palanim_mod.h"
#include "render_mod.h"
#include "game_mod.h"
#include "sndres.h"
#include "music.h"

/*
 * Begin module
\*--------------------------------------------------------------------------*/
#include "events_mod.h"
#include "events.h"

namespace Saga {

static YS_DL_LIST *EventList;

int EVENT_Init(void)
{
	R_printf(R_STDOUT, "Initializing event subsystem...\n");

	EventList = ys_dll_create();

	return (EventList != NULL) ? R_SUCCESS : R_FAILURE;
}

int EVENT_Shutdown(void)
{
	R_printf(R_STDOUT, "Shutting down event subsystem...\n");

	EVENT_FreeList();

	return R_SUCCESS;
}

int EVENT_HandleEvents(long msec)
/*--------------------------------------------------------------------------*\
 * Function to process event list once per frame. 
 * First advances event times, then processes each event with the appropriate
 *  handler depending on the type of event.
\*--------------------------------------------------------------------------*/
{
	YS_DL_NODE *walk_node;
	YS_DL_NODE *next_node;

	R_EVENT *event_p;

	long delta_time;
	int result;

	/* Advance event times
	 * \*------------------------------------------------------------- */
	ProcessEventTime(msec);

	/* Process each event in list
	 * \*------------------------------------------------------------- */
	for (walk_node = ys_dll_head(EventList);
	    walk_node != NULL; walk_node = next_node) {

		event_p = (R_EVENT *)ys_dll_get_data(walk_node);

		/* Save next event in case current event is handled and removed */
		next_node = ys_dll_next(walk_node);

		/* Call the appropriate event handler for the specific event type */
		switch (event_p->type) {

		case R_ONESHOT_EVENT:
			result = HandleOneShot(event_p);
			break;

		case R_CONTINUOUS_EVENT:
			result = HandleContinuous(event_p);
			break;

		case R_INTERVAL_EVENT:
			result = HandleInterval(event_p);
			break;

		default:
			result = R_EVENT_INVALIDCODE;
			R_printf(R_STDERR,
			    "Invalid event code encountered.\n");
			break;
		}

		/* Process the event appropriately based on result code from 
		 * handler */
		if ((result == R_EVENT_DELETE) ||
		    (result == R_EVENT_INVALIDCODE)) {

			/* If there is no event chain, delete the base event. */
			if (event_p->chain == NULL) {
				ys_dll_delete(walk_node);
			} else {
				/* If there is an event chain present, move the next event 
				 * in the chain up, adjust it by the previous delta time, 
				 * and reprocess the event by adjusting next_node. */
				delta_time = event_p->time;

				ys_dll_replace(walk_node, event_p->chain,
				    sizeof *event_p);

				event_p = (R_EVENT *)ys_dll_get_data(walk_node);
				event_p->time += delta_time;

				next_node = walk_node;
			}
		} else if (result == R_EVENT_BREAK) {

			break;
		}

	}			/* end for () */

	return R_SUCCESS;
}

int HandleContinuous(R_EVENT * event)
{

	double event_pc = 0.0;	/* Event completion percentage */
	int event_done = 0;

	R_BUFFER_INFO buf_info;
	SCENE_BGINFO bg_info;
	R_SURFACE *back_buf;

	event_pc = ((double)event->duration - event->time) / event->duration;

	if (event_pc >= 1.0) {
		/* Cap percentage to 100 */
		event_pc = 1.0;
		event_done = 1;
	}

	if (event_pc < 0.0) {
		/* Event not signaled, skip it */
		return R_EVENT_CONTINUE;
	} else if (!(event->code & R_SIGNALED)) {
		/* Signal event */
		event->code |= R_SIGNALED;
		event_pc = 0.0;
	}

	switch (event->code & R_EVENT_MASK) {

	case R_PAL_EVENT:

		switch (event->op) {

		case EVENT_BLACKTOPAL:

			back_buf = SYSGFX_GetBackBuffer();

			SYSGFX_BlackToPal(back_buf, (PALENTRY *)event->data, event_pc);
			break;

		case EVENT_PALTOBLACK:

			back_buf = SYSGFX_GetBackBuffer();

			SYSGFX_PalToBlack(back_buf, (PALENTRY *)event->data, event_pc);
			break;

		default:
			break;
		}		/* end switch() */

		break;

	case R_TRANSITION_EVENT:

		switch (event->op) {

		case EVENT_DISSOLVE:

			RENDER_GetBufferInfo(&buf_info);
			SCENE_GetBGInfo(&bg_info);

			TRANSITION_Dissolve(buf_info.r_bg_buf,
			    buf_info.r_bg_buf_w,
			    buf_info.r_bg_buf_h,
			    buf_info.r_bg_buf_w,
			    bg_info.bg_buf, bg_info.bg_p, 0, event_pc);

			break;

		default:
			break;
		}

		break;

	case R_CONSOLE_EVENT:

		switch (event->op) {

		case EVENT_ACTIVATE:

			CON_DropConsole(event_pc);
			break;

		case EVENT_DEACTIVATE:

			CON_RaiseConsole(event_pc);
			break;

		default:
			break;
		}

		break;

	default:
		break;

	}			/* end switch( event->event_code ) */

	if (event_done) {
		return R_EVENT_DELETE;
	}

	return R_EVENT_CONTINUE;
}

static int HandleOneShot(R_EVENT * event)
{

	R_SURFACE *back_buf;

	static SCENE_BGINFO bginfo;

	if (event->time > 0) {
		return R_EVENT_CONTINUE;
	}

	/* Event has been signaled */

	switch (event->code & R_EVENT_MASK) {

	case R_TEXT_EVENT:

		switch (event->op) {

		case EVENT_DISPLAY:

			TEXT_SetDisplay((R_TEXTLIST_ENTRY *)event->data, 1);
			break;

		case EVENT_REMOVE:
			{
				R_SCENE_INFO scene_info;

				SCENE_GetInfo(&scene_info);

				TEXT_DeleteEntry(scene_info.text_list,
								 (R_TEXTLIST_ENTRY *)event->data);
			}
			break;

		default:
			break;
		}

		break;

	case R_VOICE_EVENT:

		_vm->_sndRes->playVoice(event->param);
		break;

	case R_MUSIC_EVENT:

		_vm->_music->play(event->param, event->param2);
		break;

	case R_BG_EVENT:
		{
			R_BUFFER_INFO rbuf_info;
			R_POINT bg_pt;

			if (SCENE_GetMode() == R_SCENE_MODE_NORMAL) {

				back_buf = SYSGFX_GetBackBuffer();

				RENDER_GetBufferInfo(&rbuf_info);
				SCENE_GetBGInfo(&bginfo);

				bg_pt.x = bginfo.bg_x;
				bg_pt.y = bginfo.bg_y;

				GFX_BufToBuffer(rbuf_info.r_bg_buf,
				    rbuf_info.r_bg_buf_w,
				    rbuf_info.r_bg_buf_h,
				    bginfo.bg_buf,
				    bginfo.bg_w, bginfo.bg_h, NULL, &bg_pt);

				if (event->param == SET_PALETTE) {

					PALENTRY *pal_p;

					SCENE_GetBGPal(&pal_p);
					SYSGFX_SetPalette(back_buf, pal_p);
				}
			}
		}
		break;

	case R_ANIM_EVENT:

		switch (event->op) {

		case EVENT_FRAME:

			ANIM_Play(event->param, event->time);
			break;

		default:
			break;
		}

		break;

	case R_SCENE_EVENT:

		switch (event->op) {

		case EVENT_END:

			SCENE_Next();

			return R_EVENT_BREAK;
			break;

		default:
			break;
		}

		break;

	case R_PALANIM_EVENT:

		switch (event->op) {

		case EVENT_CYCLESTART:

			PALANIM_CycleStart();
			break;

		case EVENT_CYCLESTEP:

			PALANIM_CycleStep(event->time);
			break;

		default:
			break;
		}
		break;

	case R_INTERFACE_EVENT:

		switch (event->op) {

		case EVENT_ACTIVATE:

			INTERFACE_Activate();
			break;

		default:
			break;
		}

		break;

	default:
		break;

	}			/* end switch( event->code ) */

	return R_EVENT_DELETE;
}

static int HandleInterval(R_EVENT * event)
{
	YS_IGNORE_PARAM(event);

	return R_EVENT_DELETE;
}

R_EVENT *EVENT_Queue(R_EVENT * event)
/*--------------------------------------------------------------------------*\
 * Schedules an event in the event list; returns a pointer to the scheduled
 * event suitable for chaining if desired.
\*--------------------------------------------------------------------------*/
{
	YS_DL_NODE *new_node;
	R_EVENT *queued_event;

	event->chain = NULL;

	new_node = ys_dll_add_tail(EventList, event, sizeof *event);

	if (new_node == NULL) {
		return NULL;
	}

	queued_event = (R_EVENT *)ys_dll_get_data(new_node);

	InitializeEvent(queued_event);

	return queued_event;
}

R_EVENT *EVENT_Chain(R_EVENT * head_event, R_EVENT * add_event)
/*--------------------------------------------------------------------------*\
 * Places a 'add_event' on the end of an event chain given by 'head_event'
 * (head_event may be in any position in the event chain)
\*--------------------------------------------------------------------------*/
{
	R_EVENT *walk_event;
	R_EVENT *new_event;

	/* Allocate space for new event */
	new_event = (R_EVENT *)malloc(sizeof *new_event);
	if (new_event == NULL) {
		return NULL;
	}

	/* Copy event data to new event */
	*new_event = *add_event;

	/* Walk to end of chain */
	for (walk_event = head_event;
	    walk_event->chain != NULL; walk_event = walk_event->chain) {

		continue;
	}

	/* Place new event */
	walk_event->chain = new_event;
	new_event->chain = NULL;

	InitializeEvent(new_event);

	return new_event;
}

static int InitializeEvent(R_EVENT * event)
{

	switch (event->type) {

	case R_ONESHOT_EVENT:
		break;

	case R_CONTINUOUS_EVENT:
		event->time += event->duration;
		break;

	case R_INTERVAL_EVENT:
		break;

	default:
		return R_FAILURE;
		break;
	}

	return R_SUCCESS;
}

int EVENT_ClearList(void)
/*--------------------------------------------------------------------------*\
 * Removes all events from the list except NODESTROY (engine) events
\*--------------------------------------------------------------------------*/
{
	YS_DL_NODE *walk_node;
	YS_DL_NODE *next_node;

	R_EVENT *chain_walk;
	R_EVENT *next_chain;

	R_EVENT *event_p;

	/* Walk down event list */
	for (walk_node = ys_dll_head(EventList);
	    walk_node != NULL; walk_node = next_node) {

		next_node = ys_dll_next(walk_node);
		event_p = (R_EVENT *)ys_dll_get_data(walk_node);

		/* Only remove events not marked R_NODESTROY (engine events) */
		if (!(event_p->code & R_NODESTROY)) {

			/* Remove any events chained off this one */
			for (chain_walk = event_p->chain;
			    chain_walk != NULL; chain_walk = next_chain) {

				next_chain = chain_walk->chain;
				free(chain_walk);
			}

			ys_dll_delete(walk_node);
		}
	}

	return R_SUCCESS;
}

int EVENT_FreeList(void)
/*--------------------------------------------------------------------------*\
 * Removes all events from the list (even R_NODESTROY)
\*--------------------------------------------------------------------------*/
{
	YS_DL_NODE *walk_node;
	YS_DL_NODE *next_node;

	R_EVENT *chain_walk;
	R_EVENT *next_chain;

	R_EVENT *event_p;

	/* Walk down event list */
	for (walk_node = ys_dll_head(EventList);
	    walk_node != NULL; walk_node = next_node) {

		event_p = (R_EVENT *)ys_dll_get_data(walk_node);

		/* Remove any events chained off current node */
		for (chain_walk = event_p->chain;
		    chain_walk != NULL; chain_walk = next_chain) {

			next_chain = chain_walk->chain;
			free(chain_walk);
		}

		/* Delete current node */
		next_node = ys_dll_next(walk_node);
		ys_dll_delete(walk_node);
	}

	return R_SUCCESS;
}

static int ProcessEventTime(long msec)
/*--------------------------------------------------------------------------*\
 * Walks down the event list, updating event times by 'msec'.
\*--------------------------------------------------------------------------*/
{
	YS_DL_NODE *walk_node;
	R_EVENT *event_p;

	uint16 event_count = 0;

	for (walk_node = ys_dll_head(EventList);
	    walk_node != NULL; walk_node = ys_dll_next(walk_node)) {

		event_p = (R_EVENT *)ys_dll_get_data(walk_node);
		event_p->time -= msec;

		event_count++;

		if (event_count > R_EVENT_WARNINGCOUNT) {
			R_printf(R_STDERR,
			    "WARNING: Event list exceeds %u.\n", R_EVENT_WARNINGCOUNT);
		}
	}

	return R_SUCCESS;
}

} // End of namespace Saga
