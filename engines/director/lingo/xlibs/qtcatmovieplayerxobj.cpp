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
#include "director/lingo/xlibs/qtcatmovieplayerxobj.h"

/**************************************************
 *
 * USED IN:
 * Jewels of the Oracle - Win
 * Jewels of the Oracle - Mac
 *
 **************************************************/

/*
-- CatPlayr External Factory. 16/03/95 - AAF - Changed to play concatenated movies
CatPlayr
IS         mNew, fileName --Creates a new instance of the XObject
X          mDispose --Disposes of XObject instance
IIIIIIIIIS mPlay, movieOffset, startTime, duration, interruptable, horizontal, vertical, hideCursor, everyFrame, siblingWindowName --Plays QT


-- QTCatMoviePlayer, QuickTime Player XObject - js 95.05.01
IS         mNew, fileName, -- Open movie and place at h,v
X          mDispose -- Dispose of movie and close file
XIIIIIIIIS mPlay, filesOffset, startTime, duration, interruptable, h, v, hideCursor, playEveryFrame, windowName --  Play movie from start to end

 */

namespace Director {

const char *const QTCatMoviePlayerXObj::xlibName = "CatPlayr";
const XlibFileDesc QTCatMoviePlayerXObj::fileNames[] = {
	{ "CATPLAYR",				nullptr }, // Jewels of the Oracle - Win
	{ "QTCatMoviePlayer.XObj",	nullptr }, // Jewels of the Oracle - Mac
	{ nullptr,					nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",				QTCatMoviePlayerXObj::m_new,		 1, 1,	400 },
	{ "dispose",			QTCatMoviePlayerXObj::m_dispose,	 0, 0,	400 },
	{ "play",				QTCatMoviePlayerXObj::m_play,		 9, 9,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

QTCatMoviePlayerXObject::QTCatMoviePlayerXObject(ObjectType ObjectType) :Object<QTCatMoviePlayerXObject>("CatPlayr") {
	_objType = ObjectType;
}

void QTCatMoviePlayerXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		QTCatMoviePlayerXObject::initMethods(xlibMethods);
		QTCatMoviePlayerXObject *xobj = new QTCatMoviePlayerXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void QTCatMoviePlayerXObj::close(ObjectType type) {
	if (type == kXObj) {
		QTCatMoviePlayerXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}

void QTCatMoviePlayerXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("QTCatMoviePlayerXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBNR(QTCatMoviePlayerXObj::m_dispose)
XOBJSTUB(QTCatMoviePlayerXObj::m_play, 0)

}
