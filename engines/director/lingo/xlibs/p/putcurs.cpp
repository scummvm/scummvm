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
#include "director/lingo/xlibs/p/putcurs.h"

/**************************************************
 *
 * USED IN:
 * Die tolle Spiele-Box (Tivola)
 * Masters of the Elements
 *
 **************************************************/

/*
--Put Cursor XObject. 31 ago 95. Mauricio Piacentini Tabuleiro da Baiana Multimedia
--putCurs
I      mNew                --Creates a new instance of the XObject
X      mDispose            --Disposes of XObject instance
III    mSet, Xpos, Ypos    --Set the cursor to a new position
 */

namespace Director {

const char *PutcursXObj::xlibName = "Putcurs";
const XlibFileDesc PutcursXObj::fileNames[] = {
	{ "PUTCURS", nullptr },
	{ nullptr,   nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",		PutcursXObj::m_new,		0, 0,	400 },
	{ "dispose",	PutcursXObj::m_dispose,	0, 0,	400 },
	{ "set",		PutcursXObj::m_set,		2, 2,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

PutcursXObject::PutcursXObject(ObjectType ObjectType) :Object<PutcursXObject>("Putcurs") {
	_objType = ObjectType;
}

void PutcursXObj::open(ObjectType type, const Common::Path &path) {
	PutcursXObject::initMethods(xlibMethods);
    PutcursXObject *xobj = new PutcursXObject(kXObj);
    g_lingo->exposeXObject(xlibName, xobj);
}

void PutcursXObj::close(ObjectType type) {
    PutcursXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void PutcursXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("PutcursXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

void PutcursXObj::m_set(int nargs) {
	if (nargs != 2) {
		warning("PutcursXObj::m_set: expected 2 arguments, got %d", nargs);
		g_lingo->dropStack(nargs);
		return;
	}
	int y = g_lingo->pop().asInt();
	int x = g_lingo->pop().asInt();
	g_system->warpMouse(x, y);
}

XOBJSTUBNR(PutcursXObj::m_dispose)

}
