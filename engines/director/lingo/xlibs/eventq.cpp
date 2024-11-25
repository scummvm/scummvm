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

/*************************************
*
* USED IN:
* Meet Mediaband
*
*************************************/

/*
 * -- EventQ XObject. 7/12/94 greg yachuk
 * I      mNew          --Creates a new instance of the XObject
 * X      mDispose      --Disposes of XObject instance.
 * X      mBufferEvents --Begins buffering messages.
 * X      mFlushEvents  --Ends buffering. Flushes all queued messages.
 * X      mPostEvents   --Ends buffering. Posts all queued messages.
 * I      mBufferStatus --1 => Buffering in effect. 0 => No buffering.
 * SSI    mGetNextEvent --Gets the next event of a certain type.
 * --                   -- param1: mouseDown or keyDown.
 * --                   -- param2: 1 => remove found message.
 * --                   --         0 => leave found message in queue.
*/

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/eventq.h"


namespace Director {

const char *const EventQXObj::xlibNames[] = {
	"EventQ",
	nullptr
};

const XlibFileDesc EventQXObj::fileNames[] = {
	{ "EventQ",	nullptr },
	{ nullptr,	nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",               EventQXObj::m_new,              0, 0,  400 },  // D4
	{ "Dispose",           EventQXObj::m_dispose,          0, 0,  400 },  // D4
	{ "BufferEvents",      EventQXObj::m_bufferEvents,     0, 0,  400 },  // D4
	{ "FlushEvents",       EventQXObj::m_flushEvents,      0, 0,  400 },  // D4
    { "PostEvents",        EventQXObj::m_postEvents,       0, 0,  400 },  // D4
	{ "BufferStatus",      EventQXObj::m_bufferStatus,     0, 0,  400 },  // D4
	{ "GetNextEvent",      EventQXObj::m_getNextEvent,     2, 2,  400 },  // D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void EventQXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		EventQXObject::initMethods(xlibMethods);
		EventQXObject *xobj = new EventQXObject(kXObj);
		for (uint i = 0; xlibNames[i]; i++) {
			g_lingo->exposeXObject(xlibNames[i], xobj);
		}
	}
}

void EventQXObj::close(ObjectType type) {
	if (type == kXObj) {
		EventQXObject::cleanupMethods();
		for (uint i = 0; xlibNames[i]; i++) {
			g_lingo->_globalvars[xlibNames[i]] = Datum();
		}
	}
}

EventQXObject::EventQXObject(ObjectType ObjectType) : Object<EventQXObject>("EventQ") {
	_objType = ObjectType;
}

void EventQXObj::m_new(int nargs) {
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(EventQXObj::m_dispose, 0)
XOBJSTUB(EventQXObj::m_bufferEvents, 0)
XOBJSTUB(EventQXObj::m_flushEvents, 0)
XOBJSTUB(EventQXObj::m_postEvents, 0)
XOBJSTUB(EventQXObj::m_bufferStatus, 0)
XOBJSTUB(EventQXObj::m_getNextEvent, 0)

} // End of namespace Director
