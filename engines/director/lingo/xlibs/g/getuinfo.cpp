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

#include "common/system.h"

#include "director/director.h"
#include "director/movie.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/g/getuinfo.h"

/**************************************************
 *
 * USED IN:
 * Elroy Hits the Pavement
 *
 **************************************************/

/*
-- Penworks XObject Utilities
--GetUInfo
I      mNew                -- Creates a new instance of the XObject
X      mDispose            -- Disposes of XObject instance
I      mLButtonDown        -- Returns TRUE if LEFT mouse button is down
I      mRButtonDown        -- Returns TRUE if RIGHT mouse button is down
I      mKeyPressed         -- Returns the virtual keycode if a key is pressed
 */

namespace Director {

const char *GetUInfoXObj::xlibName = "GetUInfo";
const XlibFileDesc GetUInfoXObj::fileNames[] = {
	{ "GETUINFO",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				GetUInfoXObj::m_new,		 0, 0,	400 },
	{ "dispose",				GetUInfoXObj::m_dispose,		 0, 0,	400 },
	{ "lButtonDown",				GetUInfoXObj::m_lButtonDown,		 0, 0,	400 },
	{ "rButtonDown",				GetUInfoXObj::m_rButtonDown,		 0, 0,	400 },
	{ "keyPressed",				GetUInfoXObj::m_keyPressed,		 0, 0,	400 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

GetUInfoXObject::GetUInfoXObject(ObjectType ObjectType) :Object<GetUInfoXObject>("GetUInfo") {
	_objType = ObjectType;
}

void GetUInfoXObj::open(ObjectType type, const Common::Path &path) {
    GetUInfoXObject::initMethods(xlibMethods);
    GetUInfoXObject *xobj = new GetUInfoXObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void GetUInfoXObj::close(ObjectType type) {
    GetUInfoXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void GetUInfoXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("GetUInfoXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

void GetUInfoXObj::m_dispose(int nargs) {
	g_lingo->_state->me.u.obj->dispose();
	g_lingo->push(g_lingo->_state->me);
}

void GetUInfoXObj::m_lButtonDown(int nargs) {
	g_lingo->dropStack(nargs);
	int state = g_system->getEventManager()->getButtonState() & (1 << Common::MOUSE_BUTTON_LEFT) ? 1 : 0;
	g_lingo->push(Datum(state));
}

void GetUInfoXObj::m_rButtonDown(int nargs) {
	g_lingo->dropStack(nargs);
	int state = g_system->getEventManager()->getButtonState() & (1 << Common::MOUSE_BUTTON_RIGHT) ? 1 : 0;
	g_lingo->push(Datum(state));
}

void GetUInfoXObj::m_keyPressed(int nargs) {
	g_lingo->dropStack(nargs);
	Movie *movie = g_director->getCurrentMovie();
	g_lingo->push(Datum(movie ? movie->_keyCode : 0));
}

}
