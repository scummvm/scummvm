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
#include "director/lingo/lingo-builtins.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/maniacbg.h"

/**************************************************
 *
 * USED IN:
 * Maniac Sports
 *
 **************************************************/

/*
-- ForeMost XObject. 6/23/94 greg yachuk
ForeMost
I      mNew        --Creates a new instance of the XObject
X      mDispose    --Disposes of XObject instance.
I      mIsForeMost --Is this Application foremost. 1=Yes, 0=No.
 */

namespace Director {

const char *const ManiacBgXObj::xlibName = "ForeMost";
const XlibFileDesc ManiacBgXObj::fileNames[] = {
	{ "maniacbg",	nullptr },
	{ nullptr,		nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				ManiacBgXObj::m_new,		 0, 0,	300 },
	{ "dispose",				ManiacBgXObj::m_dispose,		 0, 0,	300 },
	{ "isForeMost",				ManiacBgXObj::m_isForeMost,		 0, 0,	300 },
	{ nullptr, nullptr, 0, 0, 0 }
};

ManiacBgXObject::ManiacBgXObject(ObjectType ObjectType) :Object<ManiacBgXObject>("ForeMost") {
	_objType = ObjectType;
}

void ManiacBgXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		ManiacBgXObject::initMethods(xlibMethods);
		ManiacBgXObject *xobj = new ManiacBgXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void ManiacBgXObj::close(ObjectType type) {
	if (type == kXObj) {
		ManiacBgXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}

void ManiacBgXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("ManiacBgXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(ManiacBgXObj::m_dispose)

void ManiacBgXObj::m_isForeMost(int nargs) {
	// process events
	LB::b_updateStage(0);
	g_lingo->push(Datum(1));
	return;
}

}
