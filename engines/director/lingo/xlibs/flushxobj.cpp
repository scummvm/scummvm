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


namespace Director {

static const char *xlibName = "FlushXObj";

static MethodProto xlibMethods[] = {
	{ "new",				FlushXObj::m_new,				 0, 0,	400 },	// D4
	{ "AddToMask",			FlushXObj::m_addToMask,			 2, 2,	400 },	// D4
	{ "ClearMask",			FlushXObj::m_clearMask,			 0, 0,	400 },	// D4
	{ "Flush",				FlushXObj::m_flush,				 0, 0,  400 },	// D4
	{ "FlushEvents",		FlushXObj::m_flushEvents,		 2, 2,  400 },	// D4
	{ 0, 0, 0, 0, 0 }
};

void FlushXObj::initialize(int type) {
	FlushXObject::initMethods(xlibMethods);
	if (type & kXObj) {
		if (!g_lingo->_globalvars.contains(xlibName)) {
			FlushXObject *xobj = new FlushXObject(kXObj);
			g_lingo->_globalvars[xlibName] = xobj;
		} else {
			warning("FlushXObject already initialized");
		}
	}
}


FlushXObject::FlushXObject(ObjectType ObjectType) :Object<FlushXObject>("FlushXObj") {
	_objType = ObjectType;
}

void FlushXObj::m_new(int nargs) {
	g_lingo->push(g_lingo->_currentMe);
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
