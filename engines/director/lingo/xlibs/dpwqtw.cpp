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
* jman-win
*
*************************************/

/**
 *  -- QuickTime for Windows Player External Factory. 02oct92 JT
 * DPWQTW
 * X  +mStartup -- First time init
 * X  +mQuit  -- Major bye bye
 * XI     mNew qtPacket -- create a window
 * X      mDispose -- close and dispose window
 * XII    mVerb msg, qtPacker -- do something
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/dpwqtw.h"


namespace Director {

// The name is different from the obj filename.
const char *DPwQTw::xlibName = "dpwqtw";
const char *DPwQTw::fileNames[] = {
	"dpwqtw",
	nullptr
};

static MethodProto xlibMethods[] = {
	{ "new",		DPwQTw::m_new,			 0, 1,	400 },	// D4
	{ "startup",	DPwQTw::m_startup,		 0, 0,	400 },	// D4
	{ "quit",		DPwQTw::m_quit,			 0, 0,	400 },	// D4
	{ "verb",		DPwQTw::m_verb,			 2, 2,	400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void DPwQTw::open(int type) {
	if (type == kXObj) {
		DPwQTwXObject::initMethods(xlibMethods);
		DPwQTwXObject *xobj = new DPwQTwXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void DPwQTw::close(int type) {
	if (type == kXObj) {
		DPwQTwXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


DPwQTwXObject::DPwQTwXObject(ObjectType ObjectType) : Object<DPwQTwXObject>("dpwqtw") {
	_objType = ObjectType;
}

void DPwQTw::m_new(int nargs) {
	g_lingo->push(g_lingo->_state->me);
}

void DPwQTw::m_startup(int nargs) {
	// no op
}

void DPwQTw::m_quit(int nargs) {
	// no op
}

void DPwQTw::m_verb(int nargs) {
	g_lingo->printSTUBWithArglist("DPwQTw::m_verb", nargs);
}

} // End of namespace Director
