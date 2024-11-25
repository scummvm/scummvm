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
#include "director/lingo/xlibs/mmaskxobj.h"

/**************************************************
 *
 * USED IN:
 * Jewels of the Oracle - Mac
 *
 **************************************************/

/*
-- MMEX Mask
I	mNew
II	mMask, state
 */

namespace Director {

const char *const MMaskXObj::xlibName = "mmaskxobj";
const XlibFileDesc MMaskXObj::fileNames[] = {
	{ "MMASK.XOB",	nullptr },
	{ nullptr,		nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",			MMaskXObj::m_new,		0, 0,	400 },
	{ "dispose",		MMaskXObj::m_dispose,	0, 0,	400 },
	{ "mask",			MMaskXObj::m_mask,	 	1, 1,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

MMaskXObject::MMaskXObject(ObjectType ObjectType) :Object<MMaskXObject>("MMaskXObj") {
	_objType = ObjectType;
}

void MMaskXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		MMaskXObject::initMethods(xlibMethods);
		MMaskXObject *xobj = new MMaskXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void MMaskXObj::close(ObjectType type) {
	if (type == kXObj) {
		MMaskXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}

void MMaskXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("MMaskXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(MMaskXObj::m_dispose)
XOBJSTUBNR(MMaskXObj::m_mask)

}
