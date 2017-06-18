/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "director/lingo/lingo.h"
#include "director/frame.h"
#include "director/sprite.h"

namespace Director {

struct EventHandlerType {
	LEvent handler;
	const char *name;
} static const eventHandlerDescs[] = {
	{ kEventPrepareMovie,		"prepareMovie" },
	{ kEventStartMovie,			"startMovie" },			//		D3?
	{ kEventStepMovie,			"stepMovie" },			//		D3?
	{ kEventStopMovie,			"stopMovie" },			//		D3?

	{ kEventNew,				"newSprite" },
	{ kEventBeginSprite,		"beginSprite" },
	{ kEventEndSprite,			"endSprite" },

	{ kEventEnterFrame, 		"enterFrame" },			//			D4
	{ kEventPrepareFrame, 		"prepareFrame" },
	{ kEventIdle,				"idle" },
	{ kEventStepFrame,			"stepFrame"},
	{ kEventExitFrame, 			"exitFrame" },			//			D4

	{ kEventActivateWindow,		"activateWindow" },
	{ kEventDeactivateWindow,	"deactivateWindow" },
	{ kEventMoveWindow,			"moveWindow" },
	{ kEventResizeWindow,		"resizeWindow" },
	{ kEventOpenWindow,			"openWindow" },
	{ kEventCloseWindow,		"closeWindow" },
	{ kEventStart,				"start" },

	{ kEventKeyUp,				"keyUp" },				//			D4
	{ kEventKeyDown,			"keyDown" },			// D2 w		D4 (as when from D2)
	{ kEventMouseUp,			"mouseUp" },			// D2 w	D3?
	{ kEventMouseDown,			"mouseDown" },			// D2 w	D3?
	{ kEventRightMouseDown,		"rightMouseDown" },
	{ kEventRightMouseUp,		"rightMouseUp" },
	{ kEventMouseEnter,			"mouseEnter" },
	{ kEventMouseLeave,			"mouseLeave" },
	{ kEventMouseUpOutSide,		"mouseUpOutSide" },
	{ kEventMouseWithin,		"mouseWithin" },

	{ kEventTimeout,			"timeout" },			// D2 as when

	{ kEventNone,				0 },
};

void Lingo::initEventHandlerTypes() {
	for (const EventHandlerType *t = &eventHandlerDescs[0]; t->handler != kEventNone; ++t) {
		_eventHandlerTypeIds[t->name] = t->handler;
		_eventHandlerTypes[t->handler] = t->name;
	}
}

ScriptType Lingo::event2script(LEvent ev) {
	if (_vm->getVersion() < 4) {
		switch (ev) {
		//case kEventStartMovie: // We are precompiling it now
		//	return kMovieScript;
		case kEventEnterFrame:
			return kFrameScript;
		default:
			return kNoneScript;
		}
	}

	return kNoneScript;
}

Symbol *Lingo::getHandler(Common::String &name) {
	if (!_eventHandlerTypeIds.contains(name)) {
		if (_builtins.contains(name))
			return _builtins[name];

		return NULL;
	}

	uint32 entityIndex = ENTITY_INDEX(_eventHandlerTypeIds[name], _currentEntityId);
	if (!_handlers.contains(entityIndex))
		return NULL;

	return _handlers[entityIndex];
}

void Lingo::primaryEventHandler(LEvent event) {
	/* When an event occurs the message [...] is first sent to a
	 * primary event handler: [... if exists it is executed] and the
	 * event is passed on to other objects unless you explicitly stop
	 * the message by including the dontPassEventCommand in the script
	 * [D4 docs page 77]
	 */
	debugC(3, kDebugLingoExec, "STUB: primary event handler (%s) not implemented", _eventHandlerTypes[event]);
	switch (event) {
	case kEventMouseDown:
	case kEventMouseUp:
	case kEventKeyUp:
	case kEventKeyDown:
	case kEventTimeout:
		// TODO
		break;
	default:
		/* N.B.: No primary event handlers for events other than
		 * keyup, keydown, mouseup, mousedown, timeout
		 * [see: www.columbia.edu/itc/visualarts/r4110/s2001/handouts
		 * /03_03_Event_Hierarchy.pdf]
		 */
		warning("primaryEventHandler() on event other than mouseDown, mouseUp, keyUp, keyDown, timeout");
	}
#ifdef DEBUG_DONTPASSEVENT
	// #define DEBUG_DONTPASSEVENT to simulate raising of the dontPassEvent flag
	g_lingo->dontPassEvent = true;
	debugC(3, kDebugLingoExec, "STUB: primaryEventHandler raising dontPassEvent");
#else
	debugC(3, kDebugLingoExec, "STUB: primaryEventHandler not raising dontPassEvent");
#endif
}

void Lingo::processInputEvent(LEvent event) {
	// Primary Event handler
	// Score Script
	// Script of Cast Member
	// Score Script
	// Movie Script
}

void Lingo::runMovieScript(LEvent event) {
	/* If more than one movie script handles the same message, Lingo
	 * searches the movie scripts according to their order in the cast
	 * window [p.81 of D4 docs]
	 */

	for (uint i = 0; i < _scripts[kMovieScript].size(); i++) {
		// processEvent(event,
		//			 kMovieScript,
		//			 ?);
		// TODO: How do know which script handles the message?
	}
	debugC(3, kDebugLingoExec, "STUB: processEvent(event, kMovieScript, ?)");
}

void Lingo::processFrameEvent(LEvent event) {
	/* [in D4] the enterFrame, exitFrame, idle and timeout messages
	 * are sent to a frame script and then a movie script.  If the
	 * current frame has no frame script when the event occurs, the
	 * message goes to movie scripts.
	 * [...]
	 * If more than one movie script handles the same message, Lingo
	 * searches the movie scripts according to their order in the cast
	 * window [p.81 of D4 docs]
	 */
	// TODO: Same for D2-3 or not?
	Score *score = _vm->getCurrentScore();

	if (event == kEventTimeout) {
		primaryEventHandler(event);
	}

	if (g_lingo->dontPassEvent) {
		g_lingo->dontPassEvent = false;
	} else {
		assert(score->_frames[score->getCurrentFrame()] != nullptr);
		if (!g_lingo->_scripts[kFrameScript].contains(kFrameScript)) {
			processEvent(event,
						 kFrameScript,
						 score->_frames[score->getCurrentFrame()]->_actionId);
		}
		runMovieScript(event);
	}
}

void Lingo::processGenericEvent(LEvent event) {
	// Primary Event handler
	// Movie Script
}

void Lingo::processEvent(LEvent event) {
	switch (event) {
		case kEventKeyUp:
		case kEventKeyDown:
		case kEventMouseUp:
		case kEventMouseDown:
			processInputEvent(event);
			break;

		case kEventEnterFrame:
		case kEventExitFrame:
			processFrameEvent(event);
			break;

		case kEventStartMovie:
		case kEventStopMovie:
		case kEventIdle:
		case kEventTimeout:
			processGenericEvent(event);
			break;

		default:
			warning("processEvent: Unhandled event %s", _eventHandlerTypes[event]);
	}
}

void Lingo::processEvent(LEvent event, ScriptType st, int entityId) {
	if (entityId < 0)
		return;

	debugC(9, kDebugEvents, "Lingo::processEvent(%s, %s, %d)", _eventHandlerTypes[event], scriptType2str(st), entityId);

	_currentEntityId = entityId;

	if (!_eventHandlerTypes.contains(event))
		error("processEvent: Unknown event %d for entity %d", event, entityId);

	if (_handlers.contains(ENTITY_INDEX(event, entityId))) {
		debugC(1, kDebugEvents, "Lingo::processEvent(%s, %s, %d), _eventHandler", _eventHandlerTypes[event], scriptType2str(st), entityId);
		call(_eventHandlerTypes[event], 0); // D4+ Events
	} else if (event == kEventNone && _scripts[st].contains(entityId)) {
		debugC(1, kDebugEvents, "Lingo::processEvent(%s, %s, %d), script", _eventHandlerTypes[event], scriptType2str(st), entityId);

		executeScript(st, entityId); // D3 list of scripts.
	} else {
		//debugC(3, kDebugLingoExec, "STUB: processEvent(%s) for %d", _eventHandlerTypes[event], entityId);
	}
}

} // End of namespace Director
