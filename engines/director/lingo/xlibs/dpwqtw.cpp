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
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/dpwqtw.h"

/**************************************************
 *
 * USED IN:
 * jman-win
 * hellcab-win
 *
 **************************************************/

/*
-- QuickTime for Windows Player External Factory. 02oct92 JT
--DPWQTW
X  +mStartup -- First time init
X  +mQuit  -- Major bye bye
XI     mNew qtPacket -- create a window
X      mDispose -- close and dispose window
XII    mVerb msg, qtPacker -- do something
 */

namespace Director {

const char *const DPWQTWXObj::xlibName = "DPWQTW";
const XlibFileDesc DPWQTWXObj::fileNames[] = {
	{ "DPWQTW",	nullptr },
	{ nullptr,	nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "startup",				DPWQTWXObj::m_startup,		 0, 0,	300 },
	{ "quit",				DPWQTWXObj::m_quit,		 0, 0,	300 },
	{ "new",				DPWQTWXObj::m_new,		 1, 1,	300 },
	{ "dispose",				DPWQTWXObj::m_dispose,		 0, 0,	300 },
	{ "verb",				DPWQTWXObj::m_verb,		 2, 2,	300 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static const BuiltinProto xlibBuiltins[] = {
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

DPWQTWXObject::DPWQTWXObject(ObjectType ObjectType) :Object<DPWQTWXObject>("DPWQTW") {
	_objType = ObjectType;
}

void DPWQTWXObj::open(ObjectType type, const Common::Path &path) {
    DPWQTWXObject::initMethods(xlibMethods);
    DPWQTWXObject *xobj = new DPWQTWXObject(type);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void DPWQTWXObj::close(ObjectType type) {
    DPWQTWXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void DPWQTWXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("DPWQTWXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(DPWQTWXObj::m_startup)
XOBJSTUBNR(DPWQTWXObj::m_quit)
XOBJSTUBNR(DPWQTWXObj::m_dispose)
XOBJSTUBNR(DPWQTWXObj::m_verb)

}
