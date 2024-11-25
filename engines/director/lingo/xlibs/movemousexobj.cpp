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
#include "director/lingo/xlibs/movemousexobj.h"

/*************************************
 *
 * USED IN:
 * MediaBook Sampler CD/XObject Studio
 *
 *************************************/

/*
 * MoveMouse 0.1 - (c) Andrew Green 1993 for Mac
 */


namespace Director {

const char *const MoveMouseXObj::xlibName = "MoveMouse";
const XlibFileDesc MoveMouseXObj::fileNames[] = {
	{ "MoveMouse",		nullptr },
	{ "MoveMouse.XObj",	nullptr },
	{ "MOVEWIN",		nullptr },
	{ nullptr,			nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",					MoveMouseXObj::m_new,			 0, 0,	400 },	// D4
	{ "setMouseLoc",			MoveMouseXObj::m_setMouseLoc,	 2, 2,	400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

MoveMouseXObject::MoveMouseXObject(ObjectType ObjectType) :Object<MoveMouseXObject>("MoveMouse") {
	_objType = ObjectType;
}

void MoveMouseXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		MoveMouseXObject::initMethods(xlibMethods);
		MoveMouseXObject *xobj = new MoveMouseXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void MoveMouseXObj::close(ObjectType type) {
	if (type == kXObj) {
		MoveMouseXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}

void MoveMouseXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("MoveMouseXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

void MoveMouseXObj::m_setMouseLoc(int nargs) {
	if (nargs != 2) {
		warning("MoveMouseXObj::m_setMouseLoc: expected 2 arguments");
		g_lingo->dropStack(nargs);
		return;
	}
	int y = g_lingo->pop().asInt();
	int x = g_lingo->pop().asInt();
	g_system->warpMouse(x, y);
}

}
