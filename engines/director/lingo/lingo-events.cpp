/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "director/director.h"
#include "director/debugger.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-code.h"
#include "director/lingo/lingo-object.h"
#include "director/cast.h"
#include "director/castmember/castmember.h"
#include "director/channel.h"
#include "director/frame.h"
#include "director/movie.h"
#include "director/score.h"
#include "director/sprite.h"
#include "director/types.h"
#include "director/window.h"

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

	{ kEventNone, nullptr }
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
		case kEventExitFrame:
			return kScoreScript;
		default:
			return kNoneScript;
		}
	}

	return kNoneScript;
}

void Movie::setPrimaryEventHandler(LEvent event, const Common::String &code) {
	debugC(3, kDebugLingoExec, "setting primary event handler (%s)", _lingo->_eventHandlerTypes[event]);
	LingoArchive *mainArchive = getMainLingoArch();
	mainArchive->primaryEventHandlers[event] = code;
	mainArchive->replaceCode(code, kEventScript, event);
}

void Movie::resolveScriptEvent(LingoEvent &event) {
	// Resolve the script details of an event.
	// This must be done at execution time, as it relies on
	// e.g. the current frame, the current arrangement of sprites...
	uint16 spriteId = 0;
	if (event.mousePos != Common::Point(-1, -1)) {
		// Fetch the sprite underneath the mouse cursor.

		// D3 doesn't have both mouse up and down.
		// But we still want to know if the mouse is down for press effects.
		// Since we don't have mouse up and down before D3, then we use ActiveSprite
		if (g_director->getVersion() < 400)
			spriteId = _score->getActiveSpriteIDFromPos(event.mousePos);
		else
			spriteId = _score->getMouseSpriteIDFromPos(event.mousePos);
		_currentActiveSpriteId = _score->getActiveSpriteIDFromPos(event.mousePos); // the clickOn
		_currentMouseSpriteId = _score->getMouseSpriteIDFromPos(event.mousePos);
	}
	event.channelId = spriteId;

	// mouseDown/mouseUp events will have one of each of the source types queued.
	// run these steps at the very beginning (i.e. before the first source type).
	if (event.eventHandlerSourceType == kPrimaryHandler) {
		if (event.event == kEventMouseDown) {
			if (!spriteId && _isBeepOn) {
				g_lingo->func_beep(1);
			}

			if (spriteId > 0) {
				if (_score->_channels[spriteId]->_sprite->shouldHilite()) {
					_currentHiliteChannelId = spriteId;
					g_director->_wm->_hilitingWidget = true;
					g_director->getCurrentWindow()->setDirty(true);
					g_director->getCurrentWindow()->addDirtyRect(_score->_channels[_currentHiliteChannelId]->getBbox());
				}

				CastMember *cast = getCastMember(_score->_channels[spriteId]->_sprite->_castId);
				if (cast && cast->_type == kCastButton)
					_mouseDownWasInButton = true;

				if (_score->_channels[spriteId]->_sprite->_moveable) {
					_draggingSpriteOffset = _score->_channels[spriteId]->getPosition() - event.mousePos;
					_currentDraggedChannel = _score->_channels[spriteId];
				}

				// In the case of clicking the mouse, it is possible for a mouseDown action to
				// change the cast member underneath. on mouseUp should always load the cast
				// script for the original cast member, not the new one.
				_currentMouseDownCastID = _score->_channels[spriteId]->_sprite->_castId;

			} else {
				_currentHiliteChannelId = 0;
				_mouseDownWasInButton = false;
				_draggingSpriteOffset = Common::Point(0, 0);
				_currentDraggedChannel = nullptr;
				_currentMouseDownCastID = CastMemberID();
			}

		} else if (event.event == kEventMouseUp) {
			if (_currentHiliteChannelId && _score->_channels[_currentHiliteChannelId]) {
				g_director->getCurrentWindow()->setDirty(true);
				g_director->getCurrentWindow()->addDirtyRect(_score->_channels[_currentHiliteChannelId]->getBbox());
			}
			g_director->_wm->_hilitingWidget = false;

			_currentDraggedChannel = nullptr;

			// If this is a button cast member, and the last mouse down event was in a button
			// (any button), flip this button's hilite flag.
			// Now you might think, "Wait, we don't flip this flag in the mouseDown event.
			// And why any button??? This doesn't make any sense."
			// No, it doesn't make sense, but it's what Director does.
			if (_mouseDownWasInButton && spriteId) {
				CastMember *cast = getCastMember(_score->_channels[spriteId]->_sprite->_castId);
				if (cast && cast->_type == kCastButton)
					cast->_hilite = !cast->_hilite;
			}
			_currentHiliteChannelId = 0;
			_mouseDownWasInButton = false;
			g_director->loadSlowdownCooloff();
		}
	}

	switch (event.eventHandlerSourceType) {
	case kPrimaryHandler:
		// Run the primary event handler.
		// Note that this isn't a "real" cast member ID, it's just the enum
		// of the type of event, so it can be crammed into the script context
		// index. kEventScript is a script type reserved for the primary event
		// handlers (e.g. the mouseDownScript, the mouseUpScript), so there will
		// be no collision with script cast members like ScoreScripts.
		{
			CastMemberID scriptId(event.event, DEFAULT_CAST_LIB);
			if (getScriptContext(kEventScript, scriptId)) {
				event.event = kEventGeneric;
				event.scriptType = kEventScript;
				event.scriptId = scriptId;
			}
		}
		break;
	/* When the mouseDown or mouseUp occurs over a sprite, the message
	 * goes first to the sprite script, then to the script of the cast
	 * member, to the frame script and finally to the movie scripts.
	 *
	 * When the mouseDown or mouseUp doesn't occur over a sprite, the
	 * message goes to the frame script and then to the movie script.
	 *
	 * When more than one movie script [...]
	 * [D4 docs] */
	case kSpriteHandler:
		{
			if (!spriteId)
				return;
			Frame *currentFrame = _score->_currentFrame;
			assert(currentFrame != nullptr);
			Sprite *sprite = _score->getSpriteById(spriteId);

			// Sprite (score) script
			if (sprite && sprite->_scriptId.member) {
				ScriptContext *script = getScriptContext(kScoreScript, sprite->_scriptId);
				if (script) {
					if (script->_eventHandlers.contains(event.event)) {
						// D4-style event handler
						event.scriptType = kScoreScript;
						event.scriptId = sprite->_scriptId;
					} else if (script->_eventHandlers.contains(kEventGeneric)) {
						// D3-style sprite script, not contained in a handler
						// If sprite is immediate, its script is run on mouseDown, otherwise on mouseUp
						if ((event.event == kEventMouseDown && sprite->_immediate) || (event.event == kEventMouseUp && !sprite->_immediate)) {
							event.event = kEventGeneric;
							event.scriptType = kScoreScript;
							event.scriptId = sprite->_scriptId;
						}
						return; // FIXME: Do not execute the cast script if there is a D3-style sprite script
					}
				}
			}
		}
		break;
	case kCastHandler:
		{
			// Cast script
			// A strange quirk; if we're in a mouseDown event, Director will test
			// at runtime to find out whatever is under the mouse and use that.
			// If we're in a mouseUp event, Director will use whatever was
			// discovered -at the very beginning- of the mouseDown event chain.
			// This means e.g. the cast member can be swapped out from underneath in
			// the mouseDown sprite script and the event passed down, which
			// will mean the old cast member cast script does not get a mouseDown
			// call, but it -does- get a mouseUp call.
			// A bit unhinged, but we have a test that proves Director does this,
			// so we have to do it too.
			CastMemberID targetCast = _currentMouseDownCastID;
			if (event.event == kEventMouseDown) {
				if (!spriteId)
					return;
				Sprite *sprite = _score->getSpriteById(spriteId);
				targetCast = sprite->_castId;
			}

			ScriptContext *script = getScriptContext(kCastScript, targetCast);
			if (script && script->_eventHandlers.contains(event.event)) {
				event.scriptType = kCastScript;
				event.scriptId = targetCast;
			}
		}
		break;
	case kFrameHandler:
		{
			/* [in D4] the enterFrame, exitFrame, idle and timeout messages
			 * are sent to a frame script and then a movie script.	If the
			 * current frame has no frame script when the event occurs, the
			 * message goes to movie scripts.
			 * [p.81 of D4 docs]
			 */

			if (_score->_currentFrame == nullptr)
				return;

			CastMemberID scriptId = _score->_currentFrame->_mainChannels.actionId;
			if (!scriptId.member)
				return;

			ScriptContext *script = getScriptContext(kScoreScript, scriptId);
			if (!script)
				return;

			if (script->_eventHandlers.contains(event.event)) {
				event.scriptType = kScoreScript;
				event.scriptId = scriptId;
				return;
			}

			// Scopeless statements (ie one lined lingo commands) are executed at exitFrame
			// A score script can have both scopeless and scoped lingo. (eg. porting from D3.1 to D4)
			// In the event of both being specified in the ScoreScript, the scopeless handler is ignored.

			if (event.event == kEventExitFrame && script->_eventHandlers.contains(kEventGeneric) &&
				!(script->_eventHandlers.contains(kEventExitFrame) || script->_eventHandlers.contains(kEventEnterFrame))) {
				event.event = kEventGeneric;
				event.scriptType = kScoreScript;
				event.scriptId = scriptId;
			}
		}
		break;
	case kMovieHandler:
		{
			/* If more than one movie script handles the same message, Lingo
			 * searches the movie scripts according to their order in the cast
			 * window [p.81 of D4 docs]
			 */

			// FIXME: shared cast movie scripts could come before main movie ones
			// Movie scripts are fixed, so it's fine to look them up in advance.
			for (auto &cast : _casts) {
				LingoArchive *archive = cast._value->_lingoArchive;
				for (auto &it : archive->scriptContexts[kMovieScript]) {
					if (it._value->_eventHandlers.contains(event.event)) {
						event.scriptType = kMovieScript;
						event.scriptId = CastMemberID(it._key, cast._key);
						return;
					}
				}
			}
			LingoArchive *sharedArchive = getSharedLingoArch();
			if (sharedArchive) {
				for (auto &it : sharedArchive->scriptContexts[kMovieScript]) {
					if (it._value->_eventHandlers.contains(event.event)) {
						event.scriptType = kMovieScript;
						event.scriptId = CastMemberID(it._key, DEFAULT_CAST_LIB);
						return;
					}
				}
			}
		}
		break;
	default:
		break;
	}
}

void Movie::queueEvent(Common::Queue<LingoEvent> &queue, LEvent event, int targetId, Common::Point pos) {
	int eventId = _nextEventId++;
	if (_nextEventId < 0)
		_nextEventId = 0;

	int oldQueueSize = queue.size();

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
		{
			// Queue a call to the the primary event handler.
			// As per above, by default this will pass through to any subsequent handlers,
			// unless the script calls "dontPassEvent".
			queue.push(LingoEvent(event, eventId, kPrimaryHandler, true, pos));
		}
		break;
	case kEventMenuCallback:
		{
			CastMemberID scriptID = CastMemberID(targetId, DEFAULT_CAST_LIB);
			if (getScriptContext(kEventScript, scriptID)) {
				queue.push(LingoEvent(kEventGeneric, eventId, kEventScript, true, scriptID, pos));
			}
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
			queue.push(LingoEvent(event, eventId, kSpriteHandler, false, pos));
			queue.push(LingoEvent(event, eventId, kCastHandler, false, pos));
			break;

		case kEventExitFrame:
			queue.push(LingoEvent(event, eventId, kFrameHandler, false, pos));
			break;

		case kEventIdle:
		case kEventStartUp:
		case kEventStartMovie:
		case kEventStepMovie:
		case kEventStopMovie:
			queue.push(LingoEvent(event, eventId, kMovieHandler, false, pos));
			break;

		// no-op; only handled by the primary event handler above
		// empty case avoids them generating logs from the default
		// unhandled event case below.
		case kEventKeyUp:
		case kEventKeyDown:
		case kEventTimeout:
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
			queue.push(LingoEvent(event, eventId, kSpriteHandler, false, pos));
			queue.push(LingoEvent(event, eventId, kCastHandler, false, pos));
			// fall through

		case kEventIdle:
		case kEventEnterFrame:
		case kEventExitFrame:
		case kEventTimeout:
			queue.push(LingoEvent(event, eventId, kFrameHandler, false, pos));
			// fall through

		case kEventStartUp:
		case kEventStartMovie:
		case kEventStepMovie:
		case kEventStopMovie:
		case kEventPrepareMovie:
			queue.push(LingoEvent(event, eventId, kMovieHandler, false, pos));
			break;

		default:
			warning("registerEvent: Unhandled event %s", _lingo->_eventHandlerTypes[event]);
		}
	}

	if (oldQueueSize == queue.size()) {
		debugC(9, kDebugEvents, "Lingo::queueEvent(%s): no event handler", _lingo->_eventHandlerTypes[event]);
	}
}

void Movie::queueInputEvent(LEvent event, int targetId, Common::Point pos) {
	queueEvent(_inputEventQueue, event, targetId, pos);
}

void Movie::processEvent(LEvent event, int targetId) {
	Common::Queue<LingoEvent> queue;
	queueEvent(queue, event, targetId);
	_vm->setCurrentWindow(this->getWindow());
	_lingo->processEvents(queue, false);
}

void Lingo::processEvents(Common::Queue<LingoEvent> &queue, bool isInputEvent) {
	if (isInputEvent && _currentInputEvent.type != VOIDSYM) {
		// only one input event should be in flight at a time.
		return;
	}
	int lastEventId = -1;
	Movie *movie = _vm->getCurrentMovie();
	Score *sc = movie->getScore();

	while (!queue.empty()) {
		LingoEvent el = queue.pop();

		if (sc->_playState == kPlayStopped && el.event != kEventStopMovie)
			continue;

		// fetch the sprite ID, script ID to call, etc if not present.
		movie->resolveScriptEvent(el);

		if (el.scriptType == kNoneScript) {
			debugC(9, kDebugEvents, "Lingo::processEvents: no matching script for event (%s, %s, %s, %d), continuing",
				_eventHandlerTypes[el.event], scriptType2str(el.scriptType), el.scriptId.asString().c_str(), el.channelId
			);
			continue;
		}

		if (lastEventId == el.eventId && !_passEvent) {
			debugC(5, kDebugEvents, "Lingo::processEvents: swallowed event (%s, %s, %s, %d) because _passEvent was false",
				_eventHandlerTypes[el.event], scriptType2str(el.scriptType), el.scriptId.asString().c_str(), el.channelId
			);
			continue;
		}

		_passEvent = el.passByDefault;

		debugC(5, kDebugEvents, "Lingo::processEvents: starting event script (%s, %s, %s, %d)",
			_eventHandlerTypes[el.event], scriptType2str(el.scriptType), el.scriptId.asString().c_str(), el.channelId
		);
		bool completed = processEvent(el.event, el.scriptType, el.scriptId, el.channelId);

		if (isInputEvent && !completed) {
			debugC(5, kDebugEvents, "Lingo::processEvents: context frozen on an input event, stopping");
			LingoState *state = g_director->getCurrentWindow()->getLastFrozenLingoState();
			if (state) {
				_currentInputEvent = state->callstack.front()->sp;
			}
			break;
		}
		lastEventId = el.eventId;
	}
}

bool Lingo::processEvent(LEvent event, ScriptType st, CastMemberID scriptId, int channelId) {
	_currentChannelId = channelId;

	if (!_eventHandlerTypes.contains(event))
		error("processEvent: Unknown event %d", event);

	ScriptContext *script = g_director->getCurrentMovie()->getScriptContext(st, scriptId);

	if (script && script->_eventHandlers.contains(event)) {
		debugC(1, kDebugEvents, "Lingo::processEvent(%s, %s, %s): executing event handler", _eventHandlerTypes[event], scriptType2str(st), scriptId.asString().c_str());
		g_debugger->eventHook(event);
		LC::call(script->_eventHandlers[event], 0, false);
		return execute();
	} else {
		debugC(9, kDebugEvents, "Lingo::processEvent(%s, %s, %s): no handler", _eventHandlerTypes[event], scriptType2str(st), scriptId.asString().c_str());
	}
	return true;
}

} // End of namespace Director
