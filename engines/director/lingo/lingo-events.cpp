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

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-object.h"
#include "director/cast.h"
#include "director/movie.h"
#include "director/frame.h"
#include "director/score.h"
#include "director/sprite.h"

namespace Director {

struct EventHandlerType {
	LEvent handler;
	const char *name;
} static const eventHandlerDescs[] = {
	{ kEventPrepareMovie,		"prepareMovie" },
	{ kEventStartMovie,			"startMovie" },			//		D3
	{ kEventStepMovie,			"stepMovie" },			//		D3
	{ kEventStopMovie,			"stopMovie" },			//		D3

	{ kEventNew,				"newSprite" },
	{ kEventBeginSprite,		"beginSprite" },
	{ kEventEndSprite,			"endSprite" },

	{ kEventEnterFrame,			"enterFrame" },			//			D4
	{ kEventPrepareFrame,		"prepareFrame" },
	{ kEventIdle,				"idle" },				//		D3
	{ kEventStepFrame,			"stepFrame"},
	{ kEventExitFrame,			"exitFrame" },			//			D4

	{ kEventActivateWindow,		"activateWindow" },
	{ kEventDeactivateWindow,	"deactivateWindow" },
	{ kEventMoveWindow,			"moveWindow" },
	{ kEventResizeWindow,		"resizeWindow" },
	{ kEventOpenWindow,			"openWindow" },
	{ kEventCloseWindow,		"closeWindow" },

	{ kEventKeyUp,				"keyUp" },				//			D4
	{ kEventKeyDown,			"keyDown" },			// D2 w		D4 (as when from D2)
	{ kEventMouseUp,			"mouseUp" },			// D2 w	D3
	{ kEventMouseDown,			"mouseDown" },			// D2 w	D3
	{ kEventRightMouseDown,		"rightMouseDown" },
	{ kEventRightMouseUp,		"rightMouseUp" },
	{ kEventMouseEnter,			"mouseEnter" },
	{ kEventMouseLeave,			"mouseLeave" },
	{ kEventMouseUpOutSide,		"mouseUpOutSide" },
	{ kEventMouseWithin,		"mouseWithin" },

	{ kEventTimeout,			"timeout" },			// D2 as when

	{ kEventStartUp,			"startUp" },

	{ kEventGeneric,			"scummvm_generic" },

	{ kEventNone, 0 }
};

void Lingo::initEventHandlerTypes() {
	for (const EventHandlerType *t = &eventHandlerDescs[0]; t->handler != kEventNone; ++t) {
		_eventHandlerTypeIds[t->name] = t->handler;
		_eventHandlerTypes[t->handler] = t->name;
	}
	_eventHandlerTypes[kEventNone] = 0;
}

ScriptType Lingo::event2script(LEvent ev) {
	if (_vm->getVersion() < 400) {
		switch (ev) {
		//case kEventStartMovie: // We are precompiling it now
		//	return kMovieScript;
		case kEventEnterFrame:
			return kScoreScript;
		default:
			return kNoneScript;
		}
	}

	return kNoneScript;
}

int Movie::getEventCount() {
	return _eventQueue.size();
}

void Movie::setPrimaryEventHandler(LEvent event, const Common::String &code) {
	debugC(3, kDebugLingoExec, "setting primary event handler (%s)", _lingo->_eventHandlerTypes[event]);
	LingoArchive *mainArchive = getMainLingoArch();
	mainArchive->primaryEventHandlers[event] = code;
	mainArchive->addCode(code.c_str(), kEventScript, event);
}

void Movie::queueSpriteEvent(LEvent event, int eventId, int spriteId) {
	/* When the mouseDown or mouseUp occurs over a sprite, the message
	 * goes first to the sprite script, then to the script of the cast
	 * member, to the frame script and finally to the movie scripts.
	 *
	 * When the mouseDown or mouseUp doesn't occur over a sprite, the
	 * message goes to the frame script and then to the movie script.
	 *
	 * When more than one movie script [...]
	 * [D4 docs] */

	Frame *currentFrame = _score->_frames[_score->getCurrentFrame()];
	assert(currentFrame != nullptr);
	Sprite *sprite = _score->getSpriteById(spriteId);

	// Sprite (score) script
	if (sprite->_scriptId) {
		ScriptContext *script = getScriptContext(kScoreScript, sprite->_scriptId);
		if (script) {
			// In D3 the event lingo is not contained in a handler
			// If sprite is immediate, its script is run on mouseDown, otherwise on mouseUp
			if (((event == kEventMouseDown && sprite->_immediate) || (event == kEventMouseUp && !sprite->_immediate))
					&& script->_eventHandlers.contains(kEventGeneric)) {
				_eventQueue.push(LingoEvent(kEventGeneric, eventId, kScoreScript, sprite->_scriptId, false, spriteId));
			} else if (script->_eventHandlers.contains(event)) {
				_eventQueue.push(LingoEvent(event, eventId, kScoreScript, sprite->_scriptId, false, spriteId));
			}
		}
	}

	// Cast script
	ScriptContext *script = getScriptContext(kCastScript, sprite->_castId);
	if (script && script->_eventHandlers.contains(event)) {
		_eventQueue.push(LingoEvent(event, eventId, kCastScript, sprite->_castId, false, spriteId));
	}
}

void Movie::queueFrameEvent(LEvent event, int eventId) {
	/* [in D4] the enterFrame, exitFrame, idle and timeout messages
	 * are sent to a frame script and then a movie script.	If the
	 * current frame has no frame script when the event occurs, the
	 * message goes to movie scripts.
	 * [p.81 of D4 docs]
	 */

	// if (event == kEventPrepareFrame || event == kEventIdle) {
	// 	entity = score->getCurrentFrame();
	// } else {

	assert(_score->_frames[_score->getCurrentFrame()] != nullptr);
	int scriptId = _score->_frames[_score->getCurrentFrame()]->_actionId;
	if (!scriptId)
		return;

	ScriptContext *script = getScriptContext(kScoreScript, scriptId);
	if (!script)
		return;

	if (event == kEventEnterFrame && script->_eventHandlers.contains(kEventGeneric)) {
		_eventQueue.push(LingoEvent(kEventGeneric, eventId, kScoreScript, scriptId, false));
	} else if (script->_eventHandlers.contains(event)) {
		_eventQueue.push(LingoEvent(event, eventId, kScoreScript, scriptId, false));
	}
}

void Movie::queueMovieEvent(LEvent event, int eventId) {
	/* If more than one movie script handles the same message, Lingo
	 * searches the movie scripts according to their order in the cast
	 * window [p.81 of D4 docs]
	 */

	// FIXME: shared cast movie scripts could come before main movie ones
	LingoArchive *mainArchive = getMainLingoArch();
	for (ScriptContextHash::iterator it = mainArchive->scriptContexts[kMovieScript].begin();
			it != mainArchive->scriptContexts[kMovieScript].end(); ++it) {
		if (it->_value->_eventHandlers.contains(event)) {
			_eventQueue.push(LingoEvent(event, eventId, kMovieScript, it->_key, false));
			return;
		}
	}
	LingoArchive *sharedArchive = getSharedLingoArch();
	if (sharedArchive) {
		for (ScriptContextHash::iterator it = sharedArchive->scriptContexts[kMovieScript].begin();
				it != sharedArchive->scriptContexts[kMovieScript].end(); ++it) {
			if (it->_value->_eventHandlers.contains(event)) {
				_eventQueue.push(LingoEvent(event, eventId, kMovieScript, it->_key, false));
				return;
			}
		}
	}
}

void Movie::registerEvent(LEvent event, int targetId) {
	int eventId = _nextEventId++;
	if (_nextEventId < 0)
		_nextEventId = 0;

	int oldQueueSize = _eventQueue.size();

	/* When an event occurs the message [...] is first sent to a
	 * primary event handler: [... if exists it is executed] and the
	 * event is passed on to other objects unless you explicitly stop
	 * the message by including the dontPassEvent command in the script
	 * [D4 docs page 77]
	 */
	/* N.B.: No primary event handlers for events other than
	 * keyup, keydown, mouseup, mousedown, timeout
	 * [see: www.columbia.edu/itc/visualarts/r4110/s2001/handouts
	 * /03_03_Event_Hierarchy.pdf]
	 */
	switch (event) {
	case kEventMouseDown:
	case kEventMouseUp:
	case kEventKeyUp:
	case kEventKeyDown:
	case kEventTimeout:
		if (getScriptContext(kEventScript, event)) {
			_eventQueue.push(LingoEvent(kEventGeneric, eventId, kEventScript, event, true));
		}
		break;
	case kEventMenuCallback:
		if (getScriptContext(kEventScript, targetId)) {
			_eventQueue.push(LingoEvent(kEventGeneric, eventId, kEventScript, targetId, true));
		}
		break;
	default:
		break;
	}

	if (_vm->getVersion() < 400) {
		// In D2-3, specific objects handle each event, with no passing
		switch(event) {
		case kEventMouseUp:
		case kEventMouseDown:
			if (targetId) {
				queueSpriteEvent(event, eventId, targetId);
			}
			break;

		case kEventEnterFrame:
			queueFrameEvent(event, eventId);
			break;
		
		case kEventIdle:
		case kEventStartUp:
		case kEventStartMovie:
		case kEventStepMovie:
		case kEventStopMovie:
			queueMovieEvent(event, eventId);
			break;

		default:
			warning("registerEvent: Unhandled event %s", _lingo->_eventHandlerTypes[event]);	
		}
	} else {
		/* In D4+, queue any objects that responds to this event, in order of precedence.
		 *   (Sprite -> Cast Member -> Frame -> Movie)
		 * Once one of these objects handles the event, any event handlers queued
		 * for the same event will be ignored unless the pass command was called.
		 */
		switch (event) {
		case kEventKeyUp:
		case kEventKeyDown:
		case kEventMouseUp:
		case kEventMouseDown:
		case kEventBeginSprite:
			if (targetId) {
				queueSpriteEvent(event, eventId, targetId);
			}
			// fall through

		case kEventIdle:
		case kEventEnterFrame:
		case kEventExitFrame:
			queueFrameEvent(event, eventId);
			// fall through

		case kEventStartUp:
		case kEventStartMovie:
		case kEventStopMovie:
		case kEventTimeout:
		case kEventPrepareMovie:
			queueMovieEvent(event, eventId);
			break;

		default:
			warning("registerEvent: Unhandled event %s", _lingo->_eventHandlerTypes[event]);
		}
	}

	if (oldQueueSize == _eventQueue.size()) {
		debugC(9, kDebugEvents, "Lingo::registerEvent(%s): no event handler", _lingo->_eventHandlerTypes[event]);
	}
}

void Movie::processEvent(LEvent event, int targetId) {
	registerEvent(event, targetId);
	_vm->setCurrentMovie(this);
	_lingo->processEvents();
}

void Lingo::processEvents() {
	int lastEventId = -1;
	Movie *movie = _vm->getCurrentMovie();
	Score *sc = movie->getScore();

	if (_vm->getVersion() >= 300 && sc->getCurrentFrame() > 0 && sc->_playState != kPlayStopped && movie->_eventQueue.empty())
		movie->registerEvent(kEventIdle);

	while (!movie->_eventQueue.empty()) {
		LingoEvent el = movie->_eventQueue.pop();

		if (sc->_playState == kPlayStopped && el.event != kEventStopMovie)
			continue;

		if (lastEventId == el.eventId && !_passEvent)
			continue;

		_passEvent = el.passByDefault;
		processEvent(el.event, el.scriptType, el.scriptId, el.channelId);
		lastEventId = el.eventId;
	}
}

void Lingo::processEvent(LEvent event, ScriptType st, int scriptId, int channelId) {
	_currentChannelId = channelId;

	if (!_eventHandlerTypes.contains(event))
		error("processEvent: Unknown event %d", event);

	ScriptContext *script = g_director->getCurrentMovie()->getScriptContext(st, scriptId);

	if (script && script->_eventHandlers.contains(event)) {
		debugC(1, kDebugEvents, "Lingo::processEvent(%s, %s, %d): executing event handler", _eventHandlerTypes[event], scriptType2str(st), scriptId);
		LC::call(script->_eventHandlers[event], 0, false);
		execute(_pc);
	} else {
		debugC(9, kDebugEvents, "Lingo::processEvent(%s, %s, %d): no handler", _eventHandlerTypes[event], scriptType2str(st), scriptId);
	}
}

} // End of namespace Director
