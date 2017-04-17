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

void Lingo::processInputEvent(LEvent event) {
	// Primary Event handler
	// Score Script
	// Script of Cast Member
	// Score Script
	// Movie Script
}

void Lingo::processFrameEvent(LEvent event) {
	// Primary Event handler
	// Score Script
	// Movie Script
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
