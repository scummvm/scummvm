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
#include "director/lingo/xlibs/mmovie.h"

/**************************************************
 *
 * USED IN:
 * Virtual Nightclub
 *
 **************************************************/

/*
Multi Movie XObject by Mediamation Ltd. Copyright © Trip Media Ltd 1995-1996.
--MMovie
I       mNew
X       mDispose
IS      mOpenMMovie, fileName
II      mCloseMMovie, movieIndex
ISSSSS  mPlaySegment, segmentName, restoreOpt, abortOpt, purgeOpt, asyncOpt
ISSSSS  mPlaySegLoop, segmentName, restoreOpt, abortOpt, purgeOpt, asyncOpt
I       mIdleSegment
I       mStopSegment
IS      mSeekSegment, segmentName
II      mSetSegmentTime, segTime
IIIII   mSetDisplayBounds, left, top, right, bottom
II      mGetMovieNormalWidth, movieIndex
II      mGetMovieNormalHeight, movieIndex
II      mGetSegCount, movieIndex
SII     mGetSegName, movieIndex, segmentIndex
I       mGetMovieRate
II      mSetMovieRate, ratePercent
I       mFlushEvents
IIIII   mInvalidateRect, left, top, right, bottom
SSI     mReadFile, fileName, scramble
SSSI    mWriteFile, fileName, data, scramble
ISS     mCopyFile, sourceFileName, destFileName
I       mCopyFileCont
IS      mFreeSpace, driveLetter
IS      mDeleteFile, fileName
S       mVolList
 */

namespace Director {

const char *MMovieXObj::xlibName = "MMovie";
const char *MMovieXObj::fileNames[] = {
	"MMovie",
	nullptr
};

static MethodProto xlibMethods[] = {
	{ "Movie",				MMovieXObj::m_Movie,		 4, 4,	400 },
	{ "new",				MMovieXObj::m_new,		 0, 0,	400 },
	{ "dispose",				MMovieXObj::m_dispose,		 0, 0,	400 },
	{ "openMMovie",				MMovieXObj::m_openMMovie,		 1, 1,	400 },
	{ "closeMMovie",				MMovieXObj::m_closeMMovie,		 1, 1,	400 },
	{ "playSegment",				MMovieXObj::m_playSegment,		 5, 5,	400 },
	{ "playSegLoop",				MMovieXObj::m_playSegLoop,		 5, 5,	400 },
	{ "idleSegment",				MMovieXObj::m_idleSegment,		 0, 0,	400 },
	{ "stopSegment",				MMovieXObj::m_stopSegment,		 0, 0,	400 },
	{ "seekSegment",				MMovieXObj::m_seekSegment,		 1, 1,	400 },
	{ "setSegmentTime",				MMovieXObj::m_setSegmentTime,		 1, 1,	400 },
	{ "setDisplayBounds",				MMovieXObj::m_setDisplayBounds,		 4, 4,	400 },
	{ "getMovieNormalWidth",				MMovieXObj::m_getMovieNormalWidth,		 1, 1,	400 },
	{ "getMovieNormalHeight",				MMovieXObj::m_getMovieNormalHeight,		 1, 1,	400 },
	{ "getSegCount",				MMovieXObj::m_getSegCount,		 1, 1,	400 },
	{ "getSegName",				MMovieXObj::m_getSegName,		 2, 2,	400 },
	{ "getMovieRate",				MMovieXObj::m_getMovieRate,		 0, 0,	400 },
	{ "setMovieRate",				MMovieXObj::m_setMovieRate,		 1, 1,	400 },
	{ "flushEvents",				MMovieXObj::m_flushEvents,		 0, 0,	400 },
	{ "invalidateRect",				MMovieXObj::m_invalidateRect,		 4, 4,	400 },
	{ "readFile",				MMovieXObj::m_readFile,		 2, 2,	400 },
	{ "writeFile",				MMovieXObj::m_writeFile,		 3, 3,	400 },
	{ "copyFile",				MMovieXObj::m_copyFile,		 2, 2,	400 },
	{ "copyFileCont",				MMovieXObj::m_copyFileCont,		 0, 0,	400 },
	{ "freeSpace",				MMovieXObj::m_freeSpace,		 1, 1,	400 },
	{ "deleteFile",				MMovieXObj::m_deleteFile,		 1, 1,	400 },
	{ "volList",				MMovieXObj::m_volList,		 0, 0,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

MMovieXObject::MMovieXObject(ObjectType ObjectType) :Object<MMovieXObject>("MMovie") {
	_objType = ObjectType;
}

void MMovieXObj::open(ObjectType type, const Common::Path &path) {
    MMovieXObject::initMethods(xlibMethods);
    MMovieXObject *xobj = new MMovieXObject(type);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void MMovieXObj::close(ObjectType type) {
    MMovieXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void MMovieXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("MMovieXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(MMovieXObj::m_Movie, 0)
XOBJSTUBNR(MMovieXObj::m_dispose)
XOBJSTUB(MMovieXObj::m_openMMovie, 0)
XOBJSTUB(MMovieXObj::m_closeMMovie, 0)
XOBJSTUB(MMovieXObj::m_playSegment, 0)
XOBJSTUB(MMovieXObj::m_playSegLoop, 0)
XOBJSTUB(MMovieXObj::m_idleSegment, 0)
XOBJSTUB(MMovieXObj::m_stopSegment, 0)
XOBJSTUB(MMovieXObj::m_seekSegment, 0)
XOBJSTUB(MMovieXObj::m_setSegmentTime, 0)
XOBJSTUB(MMovieXObj::m_setDisplayBounds, 0)
XOBJSTUB(MMovieXObj::m_getMovieNormalWidth, 0)
XOBJSTUB(MMovieXObj::m_getMovieNormalHeight, 0)
XOBJSTUB(MMovieXObj::m_getSegCount, 0)
XOBJSTUB(MMovieXObj::m_getSegName, "")
XOBJSTUB(MMovieXObj::m_getMovieRate, 0)
XOBJSTUB(MMovieXObj::m_setMovieRate, 0)
XOBJSTUB(MMovieXObj::m_flushEvents, 0)
XOBJSTUB(MMovieXObj::m_invalidateRect, 0)
XOBJSTUB(MMovieXObj::m_readFile, "")
XOBJSTUB(MMovieXObj::m_writeFile, "")
XOBJSTUB(MMovieXObj::m_copyFile, 0)
XOBJSTUB(MMovieXObj::m_copyFileCont, 0)
XOBJSTUB(MMovieXObj::m_freeSpace, 0)
XOBJSTUB(MMovieXObj::m_deleteFile, 0)
XOBJSTUB(MMovieXObj::m_volList, "")

}
