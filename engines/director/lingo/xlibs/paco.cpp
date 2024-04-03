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
#include "director/lingo/xlibs/paco.h"

/**************************************************
 *
 * USED IN:
 * hellcab-win
 *
 **************************************************/

/*
-- PACow External Factory. 15Jul93 JMU
--PACo
SS     mNew, command      --Creates a new instance of the XObject
X      mDispose            --Disposes of XObject instance
SSS    mPACo, commands, results    --Plays Paco movies
 */

namespace Director {

const char *PACoXObj::xlibName = "PACo";
const char *PACoXObj::fileNames[] = {
	"PACO",
	nullptr
};

static MethodProto xlibMethods[] = {
	{ "new",				PACoXObj::m_new,		 1, 1,	300 },
	{ "dispose",				PACoXObj::m_dispose,		 0, 0,	300 },
	{ "pACo",				PACoXObj::m_pACo,		 2, 2,	300 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

PACoXObject::PACoXObject(ObjectType ObjectType) :Object<PACoXObject>("PACoXObj") {
	_objType = ObjectType;
}

void PACoXObj::open(ObjectType type, const Common::Path &path) {
    PACoXObject::initMethods(xlibMethods);
    PACoXObject *xobj = new PACoXObject(type);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void PACoXObj::close(ObjectType type) {
    PACoXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void PACoXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("PACoXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(PACoXObj::m_dispose)
XOBJSTUB(PACoXObj::m_pACo, "")

}
