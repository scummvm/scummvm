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
	{ "new",		BatQT::m_new,			 0, 0,	400 },	// D4
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


BatQTXObject::BatQTXObject(ObjectType ObjectType) : Object<BatQTXObject>("FindSys") {
	_objType = ObjectType;
}

void BatQT::m_new(int nargs) {
	g_lingo->push(g_lingo->_state->me);
}

} // End of namespace Director
