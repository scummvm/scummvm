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
 * The Daedalus Encounter
 *
 *************************************/
/* -! Category compactDisc
* -! Title AppleCD SC
* -! Protocol Ortho-Play 1.6.1
* --© 1990,1991 MacroMind, Inc. Alan McNeil
* ------------------------------------------------------
* --AppleCD SC, version 2.5.6
* --Mar  7 1994
* --including AppleCD SC Plus volume control
* ------------------------------------------------------
* X  mNew
* X  mDispose
* --
* I  mGetMaxNodes
* SI  mGetNodeTitle NodeNum
* II  mSelectNode NodeNum
* --
* I  mService
* I  mGetValue
* I  mCancel
* S  mExplain
* I  mIdle
* --
* X  mReadStatus
* X  mReadPos
* XI mSearchTo position
* X  mPlay
* X  mStill
* X  mStop
* X  mScanForward
* X  mScanReverse
* X  mEject
* --
* I  mGetFirstTrack
* I  mGetLastTrack
* II  mGetFirstFrame tracknum
* II  mGetLastFrame tracknum
* I  mGetTrack
* XII mAudioEnable number number
* I  mGetFrameResolution
* --
* II mSetInPoint frame
* II mSetOutPoint frame
* II mSetDuration frames
* --
* X  mPlayCue
* X  mPlaySegment
* --
* S    mTitle
* SI   mTrackName tracknum
* IS   mSetTitle string
* IIS  mSetTrackName tracknum string
* II   mGetTrackType tracknum
* III  mSetVolume leftVolume rightVolume
* II   mGetVolume leftFlag
* --
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/applecdxobj.h"


namespace Director {

const char *AppleCDXObj::xlibName = "AppleCD";
const char *AppleCDXObj::fileNames[] = {
	"AppleCD",
	0
};

static MethodProto xlibMethods[] = {
	{ "new",					AppleCDXObj::m_new,			0,	0,	400 },	// D4
	{ "Service",				AppleCDXObj::m_service,		0,	0,	400 },	// D4
	{ "ReadStatus",				AppleCDXObj::m_readStatus,	0,	0,	400 },	// D4
	{ "Eject",    				AppleCDXObj::m_eject,		0,	0,	400 },	// D4
    { nullptr, nullptr, 0, 0, 0 }
};

void AppleCDXObj::open(int type) {
	if (type == kXObj) {
		AppleCDXObject::initMethods(xlibMethods);
		AppleCDXObject *xobj = new AppleCDXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void AppleCDXObj::close(int type) {
	if (type == kXObj) {
		AppleCDXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


AppleCDXObject::AppleCDXObject(ObjectType ObjectType) :Object<AppleCDXObject>("AppleCDXObj") {
	_objType = ObjectType;
}

void AppleCDXObj::m_new(int nargs) {
	g_lingo->push(g_lingo->_currentMe);
}

void AppleCDXObj::m_service(int nargs) {
	g_lingo->push(Datum(0));
}

void AppleCDXObj::m_readStatus(int nargs) {
}

void AppleCDXObj::m_eject(int nargs) {
}

} // End of namespace Director
