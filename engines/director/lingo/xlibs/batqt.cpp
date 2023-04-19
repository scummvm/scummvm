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
 * teamxtreme2-win
 *
 *************************************/

/*
 * -- BatQt quicktime factory. 9Aug94 RNB
 * BatQt
 * I      mNew                    --Creates a new instance of the XObject
 * X      mDispose                --Disposes of XObject instance
 * S      mName                   --Returns the XObject name
 * I      mStatus                 --Returns an integer status code
 * SI     mError, code            --Returns an error string
 * S      mLastError              --Returns last error string
 * ISI    mOpen                   --Opens the specified movie
 * IIIS   mPlay                   --Play the movie, after setting parms
 * I      mStop                   --Stop the movie
 * S      mGetTimeRange           --Gets the current time range
 * S      mGetMovieBox            --Gets the current bounds box of the movie
 * I      mGetTime                --Gets the current time of the movie
 * SI     mSetTime                --Sets the current time of the movie
 * SI     mSetVolume              --Sets the volume of the movie
 * I      mLength                 --Gets the length of the movie
 * IIIII  mSetMovieBox            --Sets the bounding box of the movie
 * III    mSetTimeRange           -- Sets the active segment of the movie
 * II     mAddCallback            -- Adds a callback for the movie
 * II     mRemoveCallback         -- Removes a callback for the movie
 * I      mResetCallbacks         -- Resets the sent status of the callbacks
 * XS     mSetBatch               -- Applies a set of batch commands
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/batqt.h"


namespace Director {

// The name is different from the obj filename.
const char *BatQT::xlibName = "batQT";
const char *BatQT::fileNames[] = {
	"batQT",
	nullptr
};

static MethodProto xlibMethods[] = {
	{ "new",			BatQT::m_new,			0, 0,	400 },	// D4
	{ "dispose",		BatQT::m_dispose,		1, 1,	400 },	// D4
	{ "name",			BatQT::m_name,			0, 0,	400 },	// D4
	{ "status",			BatQT::m_status,		0, 0,	400 },	// D4
	{ "error",			BatQT::m_error,			1, 1,	400 },	// D4
	{ "lastError",		BatQT::m_lastError,		0, 0,	400 },	// D4
	{ "open",			BatQT::m_open,			2, 2,	400 },	// D4
	{ "play",			BatQT::m_play,			3, 3,	400 },	// D4
	{ "stop",			BatQT::m_stop,			0, 0,	400 },  // D4
	{ "getTimeRange",	BatQT::m_getTimeRange,	0, 0,	400 },  // D4
	{ "getMovieBox",	BatQT::m_getMovieBox,	0, 0,	400 },  // D4
	{ "getTime",		BatQT::m_getTime,		0, 0,	400 },  // D4
	{ "setTime",		BatQT::m_setTime,		1, 1,	400 },  // D4
	{ "setVolume",		BatQT::m_setVolume,		1, 1,	400 },  // D4
	{ "length",			BatQT::m_length,		0, 0,	400 },  // D4
	{ "setMovieBox",	BatQT::m_setMovieBox,	4, 4,	400 },  // D4
	{ "setTimeRange",	BatQT::m_setTimeRange,	2, 2,	400 },  // D4
	{ "addCallback",	BatQT::m_addCallback,	1, 1,	400 },  // D4
	{ "removeCallback",	BatQT::m_removeCallback,1, 1,	400 },  // D4
	{ "resetCallbacks",	BatQT::m_resetCallbacks,0, 0,	400 },  // D4
	{ "setBatch",		BatQT::m_setBatch,		1, 1,	400 },  // D4
	{ nullptr, nullptr, 0, 0, 0 }
};

void BatQT::open(int type) {
	if (type == kXObj) {
		BatQTXObject::initMethods(xlibMethods);
		BatQTXObject *xobj = new BatQTXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void BatQT::close(int type) {
	if (type == kXObj) {
		BatQTXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


BatQTXObject::BatQTXObject(ObjectType ObjectType) : Object<BatQTXObject>("BatQt") {
	_objType = ObjectType;
}

void BatQT::m_new(int nargs) {
	g_lingo->push(g_lingo->_state->me);
}

void BatQT::m_dispose(int nargs) {
	g_lingo->printSTUBWithArglist("BatQT::m_dispose", nargs);
	g_lingo->dropStack(nargs);
}

void BatQT::m_name(int nargs) {
	g_lingo->printSTUBWithArglist("BatQT::m_name", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(""));
}

void BatQT::m_status(int nargs) {
	g_lingo->printSTUBWithArglist("BatQT::m_status", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void BatQT::m_error(int nargs) {
	g_lingo->printSTUBWithArglist("BatQT::m_error", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(""));
}

void BatQT::m_lastError(int nargs) {
	g_lingo->printSTUBWithArglist("BatQT::m_lastError", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(""));
}

void BatQT::m_open(int nargs) {
	g_lingo->printSTUBWithArglist("BatQT::m_open", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void BatQT::m_play(int nargs) {
	g_lingo->printSTUBWithArglist("BatQT::m_play", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void BatQT::m_stop(int nargs) {
	g_lingo->printSTUBWithArglist("BatQT::m_stop", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void BatQT::m_getTimeRange(int nargs) {
	g_lingo->printSTUBWithArglist("BatQT::m_getTimeRange", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(""));
}

void BatQT::m_getMovieBox(int nargs) {
	g_lingo->printSTUBWithArglist("BatQT::m_getMovieBox", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum("0,0,320,240"));
}

void BatQT::m_getTime(int nargs) {
	g_lingo->printSTUBWithArglist("BatQT::m_getTime", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void BatQT::m_setTime(int nargs) {
	g_lingo->printSTUBWithArglist("BatQT::m_setTime", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(""));
}

void BatQT::m_setVolume(int nargs) {
	g_lingo->printSTUBWithArglist("BatQT::m_setVolume", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(""));
}

void BatQT::m_length(int nargs) {
	g_lingo->printSTUBWithArglist("BatQT::m_length", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void BatQT::m_setMovieBox(int nargs) {
	g_lingo->printSTUBWithArglist("BatQT::m_setMovieBox", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void BatQT::m_setTimeRange(int nargs) {
	g_lingo->printSTUBWithArglist("BatQT::m_setTimeRange", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void BatQT::m_addCallback(int nargs) {
	g_lingo->printSTUBWithArglist("BatQT::m_addCallback", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void BatQT::m_removeCallback(int nargs) {
	g_lingo->printSTUBWithArglist("BatQT::m_removeCallback", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void BatQT::m_resetCallbacks(int nargs) {
	g_lingo->printSTUBWithArglist("BatQT::m_resetCallbacks", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum(0));
}

void BatQT::m_setBatch(int nargs) {
	g_lingo->printSTUBWithArglist("BatQT::m_setBatch", nargs);
	g_lingo->dropStack(nargs);
}


} // End of namespace Director
