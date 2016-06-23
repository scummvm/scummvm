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
#include "director/lingo/lingo-gr.h"

namespace Director {

Lingo *g_lingo;

struct EventHandlerType {
	LEvent handler;
	const char *name;
} static const eventHanlerDescs[] = {
	{ kEventPrepareMovie,		"prepareMovie" },
	{ kEventStartMovie,			"startMovie" },
	{ kEventStopMovie,			"stopMovie" },

	{ kEventNew,				"newSprite" },
	{ kEventBeginSprite,		"beginSprite" },
	{ kEventEndSprite,			"endSprite" },

	{ kEventEnterFrame, 		"enterFrame" },
	{ kEventPrepareFrame, 		"prepareFrame" },
	{ kEventIdle,				"idle" },
	{ kEventStepFrame,			"stepFrame"},
	{ kEventExitFrame, 			"exitFrame" },

	{ kEventActivateWindow,		"activateWindow" },
	{ kEventDeactivateWindow,	"deactivateWindow" },
	{ kEventMoveWindow,			"moveWindow" },
	{ kEventResizeWindow,		"resizeWindow" },
	{ kEventOpenWindow,			"openWindow" },
	{ kEventCloseWindow,		"closeWindow" },
	{ kEventStart,				"start" },

	{ kEventKeyUp,				"keyUp" },
	{ kEventKeyDown,			"keyDown" },
	{ kEventMouseUp,			"mouseUp" },
	{ kEventMouseDown,			"mouseDown" },
	{ kEventRightMouseDown,		"rightMouseDown" },
	{ kEventRightMouseUp,		"rightMouseUp" },
	{ kEventMouseEnter,			"mouseEnter" },
	{ kEventMouseLeave,			"mouseLeave" },
	{ kEventMouseUpOutSide,		"mouseUpOutSide" },
	{ kEventMouseWithin,		"mouseWithin" },

	{ kEventNone,				0 },
};

Symbol::Symbol() {
	name = NULL;
	type = VOID;
	u.str = NULL;
}

Lingo::Lingo(DirectorEngine *vm) : _vm(vm) {
	g_lingo = this;

	for (const EventHandlerType *t = &eventHanlerDescs[0]; t->handler != kEventNone; ++t)
		_eventHandlerTypes[t->handler] = t->name;

	_currentScript = 0;
	_pc = 0;
	_returning = false;
	_indef = false;

	warning("Lingo Inited");
}

Lingo::~Lingo() {
}

int Lingo::codeString(const char *str) {
	int numInsts = calcStringAlignment(str);

	// Where we copy the string over
	int pos = _currentScript->size();

	// Allocate needed space in script
	for (int i = 0; i < numInsts; i++)
		_currentScript->push_back(0);

	byte *dst = (byte *)&_currentScript->front() + pos * sizeof(inst);

	memcpy(dst, str, strlen(str) + 1);

	return _currentScript->size();
}

void Lingo::addCode(Common::String code, ScriptType type, uint16 id) {
	code += '\n';

	debug(2, "Add code \"%s\" for type %d with id %d", code.c_str(), type, id);

	if (_scripts[type].contains(id)) {
		delete _scripts[type][id];
	}

	_currentScript = new ScriptData;
	_scripts[type][id] = _currentScript;

	parse(code.c_str());

	code1(STOP);

	Common::hexdump((byte *)&_currentScript->front(), _currentScript->size() * sizeof(inst));
}

void Lingo::executeScript(ScriptType type, uint16 id) {
	if (!_scripts[type].contains(id)) {
		warning("Request to execute non-existant script type %d id %d", type, id);
		return;
	}

	_currentScript = _scripts[type][id];
	_pc = 0;
	_returning = false;

	execute(_pc);
}

void Lingo::processEvent(LEvent event, int entityId) {
	if (!_eventHandlerTypes.contains(event))
		error("processEvent: Unknown event %d for entity %d", event, entityId);

	debug(2, "processEvent(%s) for %d", _eventHandlerTypes[event], entityId);
}

} // End of namespace Director
