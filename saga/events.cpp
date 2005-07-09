/* ScummVM - Scumm Interpreter
 * Copyright (C) 2004-2005 The ScummVM project
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


#include "saga/animation.h"
#include "saga/console.h"
#include "saga/scene.h"
#include "saga/interface.h"
#include "saga/text.h"
#include "saga/palanim.h"
#include "saga/render.h"
#include "saga/sndres.h"
#include "saga/music.h"
#include "saga/actor.h"

#include "saga/events.h"

namespace Saga {

Events::Events(SagaEngine *vm) : _vm(vm), _initialized(false) {
	debug(8, "Initializing event subsystem...");
	_initialized = true;
}

Events::~Events(void) {
	debug(8, "Shutting down event subsystem...");
	freeList();
}

// Function to process event list once per frame. 
// First advances event times, then processes each event with the appropriate
//  handler depending on the type of event.
int Events::handleEvents(long msec) {
	EVENT *event_p;

	long delta_time;
	int result;

	// Advance event times
	processEventTime(msec);

	// Process each event in list
	for (EventList::iterator eventi = _eventList.begin(); eventi != _eventList.end(); ++eventi) {
		event_p = (EVENT *)eventi.operator->();

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
				eventi=_eventList.eraseAndPrev(eventi);
			} else {
				// If there is an event chain present, move the next event 
				// in the chain up, adjust it by the previous delta time, 
				// and reprocess the event  */
				delta_time = event_p->time;
				EVENT *from_chain=event_p->chain;
				memcpy(event_p, from_chain,sizeof(*event_p));
				free(from_chain);  

				event_p->time += delta_time;
				--eventi;
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

	Surface *backGroundSurface;
	BGInfo bgInfo;

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
			_vm->_gfx->blackToPal((PalEntry *)event->data, event_pc);
			break;

		case EVENT_PALTOBLACK:
			_vm->_gfx->palToBlack((PalEntry *)event->data, event_pc);
			break;
		default:
			break;
		}
		break;
	case TRANSITION_EVENT:
		switch (event->op) {
		case EVENT_DISSOLVE:
			backGroundSurface = _vm->_render->getBackGroundSurface();
			_vm->_scene->getBGInfo(bgInfo);
			_vm->transitionDissolve((byte*)backGroundSurface->pixels, backGroundSurface->w, 
				backGroundSurface->h, bgInfo.buffer, bgInfo.bounds.width(), bgInfo.bounds.height(), 0, 0, 0, event_pc);
			break;
		case EVENT_DISSOLVE_BGMASK:
			// we dissolve it centered.
			// set flag of Dissolve to 1. It is a hack to simulate zero masking.
			int w, h;
			byte *mask_buf;
			size_t len;

			backGroundSurface = _vm->_render->getBackGroundSurface();
			_vm->_scene->getBGMaskInfo(w, h, mask_buf, len);
			_vm->transitionDissolve((byte*)backGroundSurface->pixels, backGroundSurface->w, 
					backGroundSurface->h, mask_buf, w, h, 1, 
					(_vm->getDisplayWidth() - w) / 2, (_vm->getDisplayHeight() - h) / 2, event_pc);
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
			_vm->_gfx->blackToPal((PalEntry *)event->data, event_pc);
			break;

		case EVENT_PALTOBLACK:
			_vm->_gfx->palToBlack((PalEntry *)event->data, event_pc);
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
	Surface *backBuffer;
	ScriptThread *sthread;
	Rect rect;


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
				_vm->textDeleteEntry(_vm->_scene->_textList, (TEXTLIST_ENTRY *)event->data);
			}
			break;
		default:
			break;
		}

		break;
	case SOUND_EVENT:
		_vm->_sound->stopSound();
		if (event->op == EVENT_PLAY)
			_vm->_sndRes->playSound(event->param, event->param2, event->param3 != 0);
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
			Surface *backGroundSurface;
			BGInfo bgInfo;

			if (!(_vm->_scene->getFlags() & kSceneFlagISO)) {

				backBuffer = _vm->_gfx->getBackBuffer();
				backGroundSurface = _vm->_render->getBackGroundSurface();
				_vm->_scene->getBGInfo(bgInfo);

				backGroundSurface->blit(bgInfo.bounds, bgInfo.buffer);

				// If it is inset scene then draw black border
				if (bgInfo.bounds.width() < _vm->getDisplayWidth() || bgInfo.bounds.height() < _vm->getSceneHeight()) {
					Common::Rect rect1(2, bgInfo.bounds.height() + 4);
					Common::Rect rect2(bgInfo.bounds.width() + 4, 2);
					Common::Rect rect3(2, bgInfo.bounds.height() + 4);
					Common::Rect rect4(bgInfo.bounds.width() + 4, 2);
					rect1.moveTo(bgInfo.bounds.left - 2, bgInfo.bounds.top - 2);
					rect2.moveTo(bgInfo.bounds.left - 2, bgInfo.bounds.top - 2);
					rect3.moveTo(bgInfo.bounds.right, bgInfo.bounds.top - 2);
					rect4.moveTo(bgInfo.bounds.left - 2, bgInfo.bounds.bottom);

					backGroundSurface->drawRect(rect1, kITEColorBlack);
					backGroundSurface->drawRect(rect2, kITEColorBlack);
					backGroundSurface->drawRect(rect3, kITEColorBlack);
					backGroundSurface->drawRect(rect4, kITEColorBlack);
				}

				if (event->param == SET_PALETTE) {
					PalEntry *pal_p;
					_vm->_scene->getBGPal(&pal_p);
					_vm->_gfx->setPalette(pal_p);
				}
			}
		}
		break;
	case ANIM_EVENT:
		switch (event->op) {
		case EVENT_PLAY:
			_vm->_anim->play(event->param, event->time, true);
			break;
		case EVENT_STOP:
			_vm->_anim->stop(event->param);
			break;
		case EVENT_FRAME:
			_vm->_anim->play(event->param, event->time, false);
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
		case EVENT_DEACTIVATE:
			_vm->_interface->deactivate();
			break;
		case EVENT_SET_STATUS:
			_vm->_interface->setStatusText((const char*)event->data);
			_vm->_interface->drawStatusBar();
			break;
		case EVENT_CLEAR_STATUS:
			_vm->_interface->setStatusText("");
			_vm->_interface->drawStatusBar();
			break;
		case EVENT_SET_FADE_MODE:
			_vm->_interface->setFadeMode(event->param);
			break;
		default:
			break;
		}
		break;
	case SCRIPT_EVENT:
		switch (event->op) {
		case EVENT_EXEC_BLOCKING:
		case EVENT_EXEC_NONBLOCKING:
			debug(6, "Exec module number %d script entry number %d", event->param, event->param2);
		
			sthread = _vm->_script->createThread(event->param, event->param2);
			if (sthread == NULL) {
				_vm->_console->DebugPrintf("Thread creation failed.\n");
				break;
			}

			sthread->_threadVars[kThreadVarAction] = TO_LE_16(event->param3);
			sthread->_threadVars[kThreadVarObject] = TO_LE_16(event->param4);
			sthread->_threadVars[kThreadVarWithObject] = TO_LE_16(event->param5);
			sthread->_threadVars[kThreadVarActor] = TO_LE_16(event->param6);

			if (event->op == EVENT_EXEC_BLOCKING)
				_vm->_script->completeThread();

			break;
		case EVENT_THREAD_WAKE:
			_vm->_script->wakeUpThreads(event->param);
			break;
		}
		break;
	case CURSOR_EVENT:
		switch (event->op) {
		case EVENT_SHOW:
			_vm->_gfx->showCursor(true);
			break;
		case EVENT_HIDE:
			_vm->_gfx->showCursor(false);
			break;
		default:
			break;
		}
		break;
	case GRAPHICS_EVENT:
		switch (event->op) {
		case EVENT_FILL_RECT:
			rect.top = event->param2;
			rect.bottom = event->param3;
			rect.left = event->param4;
			rect.right = event->param5;
			((Surface *)event->data)->drawRect(rect, event->param);
			break;
		case EVENT_SETFLAG:
			_vm->_render->setFlag(event->param);
			break;
		case EVENT_CLEARFLAG:
			_vm->_render->clearFlag(event->param);
			break;
		default:
			break;
		}
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
	EVENT *queuedEvent;

	queuedEvent = _eventList.pushBack(*event).operator->();
	initializeEvent(queuedEvent);

	return queuedEvent;
}

// Places a 'add_event' on the end of an event chain given by 'head_event'
// (head_event may be in any position in the event chain)
EVENT *Events::chain(EVENT *headEvent, EVENT *addEvent) {
	if (headEvent == NULL) {
		return queue(addEvent);
	}

	EVENT *walkEvent;
	for (walkEvent = headEvent; walkEvent->chain != NULL; walkEvent = walkEvent->chain) {
		continue;
	}

	walkEvent->chain = (EVENT *)malloc(sizeof(*walkEvent->chain));
	*walkEvent->chain = *addEvent;
	initializeEvent(walkEvent->chain);

	return walkEvent->chain;
}

int Events::initializeEvent(EVENT *event) {
	event->chain = NULL;
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
	EVENT *chain_walk;
	EVENT *next_chain;
	EVENT *event_p;

	// Walk down event list
	for (EventList::iterator eventi = _eventList.begin(); eventi != _eventList.end(); ++eventi) {
		event_p = (EVENT *)eventi.operator->();

		// Only remove events not marked NODESTROY (engine events)
		if (!(event_p->code & NODESTROY)) {
			// Remove any events chained off this one */
			for (chain_walk = event_p->chain; chain_walk != NULL; chain_walk = next_chain) {
				next_chain = chain_walk->chain;
				free(chain_walk);
			}
			eventi=_eventList.eraseAndPrev(eventi);
		}
	}

	return SUCCESS;
}

// Removes all events from the list (even NODESTROY)
int Events::freeList() {
	EVENT *chain_walk;
	EVENT *next_chain;
	EVENT *event_p;

	// Walk down event list
	EventList::iterator eventi = _eventList.begin();
	while (eventi != _eventList.end()) {
		event_p = (EVENT *)eventi.operator->();

		// Remove any events chained off this one */
		for (chain_walk = event_p->chain; chain_walk != NULL; chain_walk = next_chain) {
			next_chain = chain_walk->chain;
			free(chain_walk);
		}
		eventi=_eventList.erase(eventi);
	}

	return SUCCESS;
}

// Walks down the event list, updating event times by 'msec'.
int Events::processEventTime(long msec) {
	EVENT *event_p;
	uint16 event_count = 0;

	for (EventList::iterator eventi = _eventList.begin(); eventi != _eventList.end(); ++eventi) {
		event_p = (EVENT *)eventi.operator->();

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
