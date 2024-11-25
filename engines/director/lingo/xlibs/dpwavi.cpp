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
#include "director/lingo/xlibs/dpwavi.h"

/**************************************************
 *
 * USED IN:
 * jman-win
 * hellcab-win
 *
 **************************************************/

/*
-- AVI External Factory. 02oct92 JT
--DPWAVI
X  +mStartup -- First time init
X  +mQuit  -- Major bye bye
XI     mNew qtPacket -- create a window
X      mDispose -- close and dispose window
XII    mVerb msg, qtPacker -- do something
 */

namespace Director {

const char *const DPWAVIXObj::xlibName = "DPWAVI";
const XlibFileDesc DPWAVIXObj::fileNames[] = {
	{ "DPWAVI",	nullptr },
	{ nullptr,	nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "startup",				DPWAVIXObj::m_startup,		 0, 0,	300 },
	{ "quit",				DPWAVIXObj::m_quit,		 0, 0,	300 },
	{ "new",				DPWAVIXObj::m_new,		 1, 1,	300 },
	{ "dispose",				DPWAVIXObj::m_dispose,		 0, 0,	300 },
	{ "verb",				DPWAVIXObj::m_verb,		 2, 2,	300 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static const BuiltinProto xlibBuiltins[] = {
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

DPWAVIXObject::DPWAVIXObject(ObjectType ObjectType) :Object<DPWAVIXObject>("DPWAVI") {
	_objType = ObjectType;
}

void DPWAVIXObj::open(ObjectType type, const Common::Path &path) {
    DPWAVIXObject::initMethods(xlibMethods);
    DPWAVIXObject *xobj = new DPWAVIXObject(type);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void DPWAVIXObj::close(ObjectType type) {
    DPWAVIXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void DPWAVIXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("DPWAVIXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(DPWAVIXObj::m_startup)
XOBJSTUBNR(DPWAVIXObj::m_quit)
XOBJSTUBNR(DPWAVIXObj::m_dispose)
XOBJSTUBNR(DPWAVIXObj::m_verb)

}
