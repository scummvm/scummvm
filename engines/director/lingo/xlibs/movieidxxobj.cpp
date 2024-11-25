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
#include "director/lingo/xlibs/movieidxxobj.h"

/**************************************************
 *
 * USED IN:
 * Jewels of the Oracle - Win
 * Jewels of the Oracle - Mac
 *
 **************************************************/

/*
-- MovieIdx External Factory. 16/03/95 - AAF - New
MovieIdx
I     mNew --Creates a new instance of the XObject
X     mDispose --Disposes of XObject instance
SS    mMovieInfo, movieName --Returns movie information for a given name

-- MovieIdx, Movie Index XObject - aep 95.05.01
I mNew --  read map given file name
X mDispose --  dispose of map
SS mMovieInfo -- return info given name
 */

namespace Director {

const char *const MovieIdxXObj::xlibName = "MovieIdx";
const XlibFileDesc MovieIdxXObj::fileNames[] = {
	{ "MovieIdx",		nullptr }, // Jewels of the Oracle - Win
	{ "MovieIdx.XObj",	nullptr }, // Jewels of the Oracle - Mac
	{ nullptr,			nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				MovieIdxXObj::m_new,		 0, 0,	400 },
	{ "dispose",				MovieIdxXObj::m_dispose,		 0, 0,	400 },
	{ "movieInfo",				MovieIdxXObj::m_movieInfo,		 1, 1,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

MovieIdxXObject::MovieIdxXObject(ObjectType ObjectType) :Object<MovieIdxXObject>("MovieIdx") {
	_objType = ObjectType;
}

void MovieIdxXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		MovieIdxXObject::initMethods(xlibMethods);
		MovieIdxXObject *xobj = new MovieIdxXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void MovieIdxXObj::close(ObjectType type) {
	if (type == kXObj) {
		MovieIdxXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}

void MovieIdxXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("MovieIdxXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(MovieIdxXObj::m_dispose)
XOBJSTUB(MovieIdxXObj::m_movieInfo, "")

}
