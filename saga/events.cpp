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

// Event management module


#include "saga/saga.h"
#include "saga/gfx.h"

#include "saga/yslib.h"

#include "saga/animation.h"
#include "saga/console.h"
#include "saga/scene.h"
#include "saga/interface.h"
#include "saga/text.h"
#include "saga/palanim.h"
#include "saga/render.h"
#include "saga/game_mod.h"
#include "saga/sndres.h"
#include "saga/music.h"

#include "saga/events.h"

namespace Saga {

Events::Events(SagaEngine *vm) : _vm(vm), _initialized(false) {
	debug(0, "Initializing event subsystem...");
	_eventList = ys_dll_create();

	if (_eventList)
		_initialized = true;
}

Events::~Events(void) {
	debug(0, "Shutting down event subsystem...");
	freeList();
}

// Function to process event list once per frame. 
// First advances event times, then processes each event with the appropriate
//  handler depending on the type of event.
int Events::handleEvents(long msec) {
	YS_DL_NODE *walk_node;
	YS_DL_NODE *next_node;

	EVENT *event_p;

	long delta_time;
	int result;

	// Advance event times
	processEventTime(msec);

	// Process each event in list
	for (walk_node = ys_dll_head(_eventList); walk_node != NULL; walk_node = next_node) {
		event_p = (EVENT *)ys_dll_get_data(walk_node);

		// Save next event in case current event is handled and removed 
		next_node = ys_dll_next(walk_node);

		// Call the appropriate event handler for the specific event type
		switch (event_p->type) {

		case ONESHOT_EVENT:
			result = handleOneShot(event_p);
			break;

		case CONTINUOUS_EVENT:
			result = handleContinuous(event_p);
			break;

		case INTERVAL_EVENT:
			result = handleInterval(event_p);
			break;

		case IMMEDIATE_EVENT:
			result = handleImmediate(event_p);
			break;

		default:
			result = EVENT_INVALIDCODE;
			warning("Invalid event code encountered");
			break;
		}

		// Process the event appropriately based on result code from 
		// handler
		if ((result == EVENT_DELETE) || (result == EVENT_INVALIDCODE)) {
			// If there is no event chain, delete the base event.
			if (event_p->chain == NULL) {
				ys_dll_delete(walk_node);
			} else {
				// If there is an event chain present, move the next event 
				// in the chain up, adjust it by the previous delta time, 
				// and reprocess the event by adjusting next_node. */
				delta_time = event_p->time;

				ys_dll_replace(walk_node, event_p->chain, sizeof *event_p);

				event_p = (EVENT *)ys_dll_get_data(walk_node);
				event_p->time += delta_time;

				next_node = walk_node;
			}
		} else if (result == EVENT_BREAK) {
			break;
		}
	}

	return SUCCESS;
}

int Events::handleContinuous(EVENT *event) {
	double event_pc = 0.0; // Event completion percentage
	int event_done = 0;

	BUFFER_INFO buf_info;
	SCENE_BGINFO bg_info;
	SURFACE *back_buf;

	event_pc = ((double)event->duration - event->time) / event->duration;

	if (event_pc >= 1.0) {
		// Cap percentage to 100
		event_pc = 1.0;
		event_done = 1;
	}

	if (event_pc < 0.0) {
		// Event not signaled, skip it
		return EVENT_CONTINUE;
	} else if (!(event->code & SIGNALED)) {
		// Signal event
		event->code |= SIGNALED;
		event_pc = 0.0;
	}

	switch (event->code & EVENT_MASK) {
	case PAL_EVENT:
		switch (event->op) {
		case EVENT_BLACKTOPAL:
			back_buf = _vm->_gfx->getBackBuffer();
			_vm->_gfx->blackToPal(back_buf, (PALENTRY *)event->data, event_pc);
			break;

		case EVENT_PALTOBLACK:
			back_buf = _vm->_gfx->getBackBuffer();
			_vm->_gfx->palToBlack(back_buf, (PALENTRY *)event->data, event_pc);
			break;
		default:
			break;
		}
		break;
	case TRANSITION_EVENT:
		switch (event->op) {
		case EVENT_DISSOLVE:
			_vm->_render->getBufferInfo(&buf_info);
			_vm->_scene->getBGInfo(&bg_info);
			_vm->transitionDissolve(buf_info.bg_buf, buf_info.bg_buf_w, 
					buf_info.bg_buf_h, buf_info.bg_buf_w, bg_info.bg_buf, bg_info.bg_w, 
					bg_info.bg_h, bg_info.bg_p, 0, 0, 0, event_pc);
			break;
		case EVENT_DISSOLVE_BGMASK:
			// we dissolve it centered.
			// set flag of Dissolve to 1. It is a hack to simulate zero masking.
			int w, h;
			byte *mask_buf;
			size_t len;

			_vm->_render->getBufferInfo(&buf_info);
			_vm->_scene->getBGMaskInfo(&w, &h, &mask_buf, &len);
			_vm->transitionDissolve(buf_info.bg_buf, buf_info.bg_buf_w, 
					buf_info.bg_buf_h, buf_info.bg_buf_w, mask_buf, w, h, 0, 1, 
					(320 - w) / 2, (200 - h) / 2, event_pc);
			break;
		default:
			break;
		}
		break;
	case CONSOLE_EVENT:
		switch (event->op) {
		case EVENT_ACTIVATE:
			_vm->_console->dropConsole(event_pc);
			break;
		case EVENT_DEACTIVATE:
			_vm->_console->raiseConsole(event_pc);
			break;
		default:
			break;
		}
		break;
	default:
		break;

	}

	if (event_done) {
		return EVENT_DELETE;
	}

	return EVENT_CONTINUE;
}

int Events::handleImmediate(EVENT *event) {
	double event_pc = 0.0; // Event completion percentage
	bool event_done = false;

	SURFACE *back_buf;

	event_pc = ((double)event->duration - event->time) / event->duration;

	if (event_pc >= 1.0) {
		// Cap percentage to 100
		event_pc = 1.0;
		event_done = true;
	}

	if (event_pc < 0.0) {
		// Event not signaled, skip it
		return EVENT_BREAK;
	} else if (!(event->code & SIGNALED)) {
		// Signal event
		event->code |= SIGNALED;
		event_pc = 0.0;
	}

	switch (event->code & EVENT_MASK) {
	case PAL_EVENT:
		switch (event->op) {
		case EVENT_BLACKTOPAL:
			back_buf = _vm->_gfx->getBackBuffer();
			_vm->_gfx->blackToPal(back_buf, (PALENTRY *)event->data, event_pc);
			break;

		case EVENT_PALTOBLACK:
			back_buf = _vm->_gfx->getBackBuffer();
			_vm->_gfx->palToBlack(back_buf, (PALENTRY *)event->data, event_pc);
			break;
		default:
			break;
		}
		break;
	case SCRIPT_EVENT:
	case BG_EVENT:
	case INTERFACE_EVENT:
		handleOneShot(event);
		event_done = true;
		break;
	default:
		break;

	}

	if (event_done) {
		return EVENT_DELETE;
	}

	return EVENT_BREAK;
}

int Events::handleOneShot(EVENT *event) {
	SURFACE *back_buf;
	SCRIPT_THREAD *sthread;

	static SCENE_BGINFO bginfo;

	if (event->time > 0) {
		return EVENT_CONTINUE;
	}

	// Event has been signaled

	switch (event->code & EVENT_MASK) {
	case TEXT_EVENT:
		switch (event->op) {
		case EVENT_DISPLAY:
			_vm->textSetDisplay((TEXTLIST_ENTRY *)event->data, 1);
			break;
		case EVENT_REMOVE:
			{
			 SCENE_INFO scene_info;
				_vm->_scene->getInfo(&scene_info);
				_vm->textDeleteEntry(scene_info.text_list, (TEXTLIST_ENTRY *)event->data);
			}
			break;
		default:
			break;
		}

		break;
	case VOICE_EVENT:
		_vm->_sndRes->playVoice(event->param);
		break;
	case MUSIC_EVENT:
		_vm->_music->stop();
		if (event->op == EVENT_PLAY)
			_vm->_music->play(event->param, event->param2);
		break;
	case BG_EVENT:
		{
		 BUFFER_INFO rbuf_info;
			Point bg_pt;

			if (_vm->_scene->getMode() == SCENE_MODE_NORMAL) {

				back_buf = _vm->_gfx->getBackBuffer();

				_vm->_render->getBufferInfo(&rbuf_info);
				_vm->_scene->getBGInfo(&bginfo);

				bg_pt.x = bginfo.bg_x;
				bg_pt.y = bginfo.bg_y;

				bufToBuffer(rbuf_info.bg_buf, rbuf_info.bg_buf_w, rbuf_info.bg_buf_h,
								bginfo.bg_buf, bginfo.bg_w, bginfo.bg_h, NULL, &bg_pt);
				if (event->param == SET_PALETTE) {
					PALENTRY *pal_p;
					_vm->_scene->getBGPal(&pal_p);
					_vm->_gfx->setPalette(back_buf, pal_p);
				}
			}
		}
		break;
	case ANIM_EVENT:
		switch (event->op) {
		case EVENT_FRAME:
			_vm->_anim->play(event->param, event->time);
			break;
		case EVENT_SETFLAG:
			_vm->_anim->setFlag(event->param, event->param2);
			break;
		case EVENT_CLEARFLAG:
			_vm->_anim->clearFlag(event->param, event->param2);
			break;
		default:
			break;
		}
		break;
	case SCENE_EVENT:
		switch (event->op) {
		case EVENT_END:
			_vm->_scene->nextScene();
			return EVENT_BREAK;
			break;
		default:
			break;
		}
		break;
	case PALANIM_EVENT:
		switch (event->op) {
		case EVENT_CYCLESTART:
			_vm->_palanim->cycleStart();
			break;
		case EVENT_CYCLESTEP:
			_vm->_palanim->cycleStep(event->time);
			break;
		default:
			break;
		}
		break;
	case INTERFACE_EVENT:
		switch (event->op) {
		case EVENT_ACTIVATE:
			_vm->_interface->activate();
			break;
		default:
			break;
		}
		break;
	case SCRIPT_EVENT:
		debug(0, "Starting start script #%d", event->param);

		sthread = _vm->_script->SThreadCreate();
		if (sthread == NULL) {
			_vm->_console->print("Thread creation failed.");
			break;
		}

		sthread->threadVars[kVarAction] = TO_LE_16(event->param2);
		sthread->threadVars[kVarObject] = TO_LE_16(event->param3);
		sthread->threadVars[kVarWithObject] = TO_LE_16(event->param4);
		sthread->threadVars[kVarActor] = TO_LE_16(event->param5);

		_vm->_script->SThreadExecute(sthread, event->param);

		if (event->op == EVENT_BLOCKING)
			_vm->_script->SThreadCompleteThread();

		break;
	default:
		break;
	}

	return EVENT_DELETE;
}

int Events::handleInterval(EVENT *event) {
	return EVENT_DELETE;
}

// Schedules an event in the event list; returns a pointer to the scheduled
// event suitable for chaining if desired.
EVENT *Events::queue(EVENT *event) {
	YS_DL_NODE *new_node;
	EVENT *queued_event;

	event->chain = NULL;
	new_node = ys_dll_add_tail(_eventList, event, sizeof *event);

	if (new_node == NULL) {
		return NULL;
	}

	queued_event = (EVENT *)ys_dll_get_data(new_node);

	initializeEvent(queued_event);

	return queued_event;
}

// Places a 'add_event' on the end of an event chain given by 'head_event'
// (head_event may be in any position in the event chain)
EVENT *Events::chain(EVENT *head_event, EVENT *add_event) {
	EVENT *walk_event;
	EVENT *new_event;

	// Allocate space for new event
	new_event = (EVENT *)malloc(sizeof *new_event);
	if (new_event == NULL) {
		return NULL;
	}

	// Copy event data to new event
	*new_event = *add_event;

	// Walk to end of chain
	for (walk_event = head_event; walk_event->chain != NULL; walk_event = walk_event->chain) {
		continue;
	}

	// Place new event
	walk_event->chain = new_event;
	new_event->chain = NULL;
	initializeEvent(new_event);

	return new_event;
}

int Events::initializeEvent(EVENT *event) {
	switch (event->type) {
	case ONESHOT_EVENT:
		break;
	case CONTINUOUS_EVENT:
	case IMMEDIATE_EVENT:
		event->time += event->duration;
		break;
	case INTERVAL_EVENT:
		break;
	default:
		return FAILURE;
		break;
	}

	return SUCCESS;
}

int Events::clearList() {
	YS_DL_NODE *walk_node;
	YS_DL_NODE *next_node;
	EVENT *chain_walk;
	EVENT *next_chain;
	EVENT *event_p;

	// Walk down event list
	for (walk_node = ys_dll_head(_eventList); walk_node != NULL; walk_node = next_node) {
		next_node = ys_dll_next(walk_node);
		event_p = (EVENT *)ys_dll_get_data(walk_node);

		// Only remove events not marked NODESTROY (engine events)
		if (!(event_p->code & NODESTROY)) {
			// Remove any events chained off this one */
			for (chain_walk = event_p->chain; chain_walk != NULL; chain_walk = next_chain) {
				next_chain = chain_walk->chain;
				free(chain_walk);
			}
			ys_dll_delete(walk_node);
		}
	}

	return SUCCESS;
}

// Removes all events from the list (even NODESTROY)
int Events::freeList() {
	YS_DL_NODE *walk_node;
	YS_DL_NODE *next_node;
	EVENT *chain_walk;
	EVENT *next_chain;
	EVENT *event_p;

	// Walk down event list
	for (walk_node = ys_dll_head(_eventList); walk_node != NULL; walk_node = next_node) {
		event_p = (EVENT *)ys_dll_get_data(walk_node);
		// Remove any events chained off current node
		for (chain_walk = event_p->chain; chain_walk != NULL; chain_walk = next_chain) {
			next_chain = chain_walk->chain;
			free(chain_walk);
		}

		// Delete current node
		next_node = ys_dll_next(walk_node);
		ys_dll_delete(walk_node);
	}

	return SUCCESS;
}

// Walks down the event list, updating event times by 'msec'.
int Events::processEventTime(long msec) {
	YS_DL_NODE *walk_node;
	EVENT *event_p;
	uint16 event_count = 0;

	for (walk_node = ys_dll_head(_eventList); walk_node != NULL; walk_node = ys_dll_next(walk_node)) {
		event_p = (EVENT *)ys_dll_get_data(walk_node);
		event_p->time -= msec;
		event_count++;

		if (event_p->type == IMMEDIATE_EVENT)
			break;

		if (event_count > EVENT_WARNINGCOUNT) {
			warning("Event list exceeds %u", EVENT_WARNINGCOUNT);
		}
	}

	return SUCCESS;
}

} // End of namespace Saga
