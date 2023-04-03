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

/*
 *  -- AVI External Factory. 02oct92 JT
 * DPWAVI
 * X  +mStartup -- First time init
 * X  +mQuit  -- Major bye bye
 * XI     mNew qtPacket -- create a window
 * X      mDispose -- close and dispose window
 * XII    mVerb msg, qtPacker -- do something
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/dpwavi.h"


namespace Director {

// The name is different from the obj filename.
const char *DPwAVI::xlibName = "dpwavi";
const char *DPwAVI::fileNames[] = {
	"dpwavi",
	nullptr
};

static MethodProto xlibMethods[] = {
	{ "new",		DPwAVI::m_new,			 0, 1,	400 },	// D4
	{ "startup",	DPwAVI::m_startup,		 0, 0,	400 },	// D4
	{ "quit",		DPwAVI::m_quit,			 0, 0,	400 },	// D4
	{ "verb",		DPwAVI::m_verb,			 2, 2,	400 },	// D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void DPwAVI::open(int type) {
	if (type == kXObj) {
		DPwAVIXObject::initMethods(xlibMethods);
		DPwAVIXObject *xobj = new DPwAVIXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void DPwAVI::close(int type) {
	if (type == kXObj) {
		DPwAVIXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


DPwAVIXObject::DPwAVIXObject(ObjectType ObjectType) : Object<DPwAVIXObject>("dpwavi") {
	_objType = ObjectType;
}

void DPwAVI::m_new(int nargs) {
	g_lingo->push(g_lingo->_state->me);
}

void DPwAVI::m_startup(int nargs) {
	// no op
}

void DPwAVI::m_quit(int nargs) {
	// no op
}

void DPwAVI::m_verb(int nargs) {
	g_lingo->printSTUBWithArglist("DPwAVI::m_verb", nargs);
}

} // End of namespace Director
