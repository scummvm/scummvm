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
 * Night Light (Mac)
 *
 *************************************/
/*
-- 		PrefPath XObject
I			mNew
X			mDispose
SS			mPrefPath
*/

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/prefpath.h"


namespace Director {

const char *PrefPath::xlibName = "PrefPath";
const char *PrefPath::fileNames[] = {
	"PrefPath",
	0
};

static MethodProto xlibMethods[] = {
	{ "PrefPath", PrefPath::m_prefpath, 1, 1, 400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void PrefPath::open(int type) {
	if (type == kXObj) {
		PrefPathObject::initMethods(xlibMethods);
		PrefPathObject *xobj = new PrefPathObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void PrefPath::close(int type) {
	if (type == kXObj) {
		PrefPathObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


PrefPathObject::PrefPathObject(ObjectType ObjectType) :Object<PrefPathObject>("PrefPath") {
	_objType = ObjectType;
}

void PrefPath::m_new(int nargs) {
	g_lingo->push(g_lingo->_state->me);
}

void PrefPath::m_prefpath(int nargs) {
	// Returns 0 if the Preferences File cannot be read
	g_lingo->printSTUBWithArglist("PrefPath", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(1));
}

} // End of namespace Director
