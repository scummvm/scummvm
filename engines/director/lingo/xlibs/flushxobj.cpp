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

/* FlushXObj is a Mac only XObject to call the underlying FlushEvents function
 * from the Macintosh Toolbox. Its purpose is to flush, i.e. remove, events
 * that happened while loading code.
 *
 * Implemented as a no-op, there's no need to flush events because:
 * - ScummVM handles them and
 * - computers were slower and queued events when the program was loading.
 *
 * More information about the Toolbox and the flush events can be found here:
 * https://en.wikipedia.org/wiki/Macintosh_Toolbox
 * https://developer.apple.com/legacy/library/documentation/mac/pdf/MacintoshToolboxEssentials.pdf
 *
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/flushxobj.h"

/*************************************
 *
 * USED IN:
 * majestic-mac
 *
 *************************************/

/*
 *  8-- FlushEvents: an XObject to call toolbox FlushEvents().-- By Scott Kelley , sakelley@ucsd.edu, 6/2/93?-- Copyright
 *  1993 The Regents of the University of California;-- Freely distributable. No warranties. Ask before selling!
 * --5--  mNew creates a new instance of the Flush XObject.@--  Note that the object contains internal variables which allow>--  a unique masking operation to be built for each object, or:--  mFlushEvents can pass masks to FlushEvents() directly.
 * I   mNew
 * --?--  mFlushEvents is identical to the toolbox call FlushEvents()%XII mFlushEvents, eventMask, stopMask
 * --?--  mClearMask clears the object's internal masks. Note that if<--  you never call this, the object starts with a default of9--  flushing everything (i.e. FlushEvent(everyEvent,0)  )
 * X   mClearMask
 * --<--  mAddToMask adds the specified event mask to the object's8--  internal masks (i.e. OR's it with the existing mask)!XII mAddToMask eventMask,stopMask
 * --6--  mFlush calls FlushEvents() with the internal masks
 * X   mFlush
 * --"--  mDispose gets rid of an object2X   mDispose       -- dispose of an instance of u * s
 */

namespace Director {

const char *const FlushXObj::xlibNames[] = {
	"FlushXObj",
	"Johnny",
	nullptr,
};
const XlibFileDesc FlushXObj::fileNames[] = {
	{ "FlushEvents",	nullptr },
	{ "FlushXObj",		nullptr },
	{ "Johnny",			nullptr },
	{ "Johnny.XObj",	nullptr },
	{ "Toilet",			nullptr },
	{ nullptr,			nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				FlushXObj::m_new,				 0, 0,	300 },	// D3
	{ "AddToMask",			FlushXObj::m_addToMask,			 2, 2,	400 },	// D4
	{ "ClearMask",			FlushXObj::m_clearMask,			 0, 0,	400 },	// D4
	{ "Flush",				FlushXObj::m_flush,				 0, 0,  300 },	// D3
	{ "FlushEvents",		FlushXObj::m_flushEvents,		 2, 2,  400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void FlushXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		FlushXObject::initMethods(xlibMethods);
		FlushXObject *xobj = new FlushXObject(kXObj);
		for (uint i = 0; xlibNames[i]; i++) {
			g_lingo->exposeXObject(xlibNames[i], xobj);
		}
	}
}

void FlushXObj::close(ObjectType type) {
	if (type == kXObj) {
		FlushXObject::cleanupMethods();
		for (uint i = 0; xlibNames[i]; i++) {
			g_lingo->_globalvars[xlibNames[i]] = Datum();
		}
	}
}


FlushXObject::FlushXObject(ObjectType ObjectType) :Object<FlushXObject>("FlushXObj") {
	_objType = ObjectType;
}

void FlushXObj::m_new(int nargs) {
	g_lingo->push(g_lingo->_state->me);
}

void FlushXObj::m_clearMask(int nargs) {
	debug(5, "FlushXobj::m_clearMask: no-op");
}

void FlushXObj::m_addToMask(int nargs) {
	g_lingo->pop();
	g_lingo->pop();

	debug(5, "FlushXobj::m_addToMask: no-op");
}

void FlushXObj::m_flush(int nargs) {
	debug(5, "FlushXobj::m_flush: no-op");
}

void FlushXObj::m_flushEvents(int nargs) {
	g_lingo->pop();
	g_lingo->pop();
	debug(5, "FlushXobj::m_flush: no-op");
}

} // End of namespace Director
