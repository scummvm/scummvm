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
 * Classical Cats
 * The Daedalus Encounter
 * Refixion II
 *
 *************************************/

/*
   -! Category compactDisc
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

#include "backends/audiocd/audiocd.h"
#include "common/file.h"
#include "common/formats/cue.h"
#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/applecdxobj.h"


namespace Director {

const char *const AppleCDXObj::xlibName = "AppleCD";
const XlibFileDesc AppleCDXObj::fileNames[] = {
	{ "AppleCD",		nullptr },
	{ "AppleCD XObj",	nullptr },
	{ nullptr,			nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",					AppleCDXObj::m_new,			0,	0,	300 },	// D3
	{ "dispose",				AppleCDXObj::m_dispose,			0,	0,	300 },	// D3
	{ "Service",				AppleCDXObj::m_service,		0,	0,	300 },	// D4
	{ "Still",				AppleCDXObj::m_still,		0,	0,	300 },	// D3
	{ "ReadStatus",				AppleCDXObj::m_readStatus,	0,	0,	300 },	// D3
	{ "GetValue",				AppleCDXObj::m_getValue,	0,	0,	300 },	// D3
	{ "Eject",    				AppleCDXObj::m_eject,		0,	0,	300 },	// D3
	{ "GetFirstTrack",    				AppleCDXObj::m_getFirstTrack,		0,	0,	300 },	// D3
	{ "GetLastTrack",    				AppleCDXObj::m_getLastTrack,		0,	0,	300 },	// D3
	{ "GetFirstFrame",    				AppleCDXObj::m_getFirstFrame,		1,	1,	300 },	// D3
	{ "GetLastFrame",    				AppleCDXObj::m_getLastFrame,		1,	1,	300 },	// D3
	{ "SetInPoint",    				AppleCDXObj::m_setInPoint,		1,	1,	300 },	// D3
	{ "SetOutPoint",    				AppleCDXObj::m_setOutPoint,		1,	1,	300 },	// D3
	{ "PlayCue",    				AppleCDXObj::m_playCue,		0,	0,	300 },	// D3
	{ "PlaySegment",    				AppleCDXObj::m_playSegment,		0,	0,	300 },	// D3
	{ "ReadPos",    				AppleCDXObj::m_readPos,		0,	0,	300 },	// D3
    { nullptr, nullptr, 0, 0, 0 }
};

void AppleCDXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		AppleCDXObject::initMethods(xlibMethods);
		AppleCDXObject *xobj = new AppleCDXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void AppleCDXObj::close(ObjectType type) {
	if (type == kXObj) {
		AppleCDXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


AppleCDXObject::AppleCDXObject(ObjectType ObjectType) :Object<AppleCDXObject>("AppleCD") {
	_objType = ObjectType;
	_inpoint = 0;
	_outpoint = 0;

	Common::File cuefile;
	if (cuefile.open("disc.cue")) {
		Common::String cuestring = cuefile.readString(0, cuefile.size());

		_cue = Common::SharedPtr<Common::CueSheet>(new Common::CueSheet(cuestring.c_str()));
	}
}

void AppleCDXObj::m_new(int nargs) {
	g_lingo->push(g_lingo->_state->me);
}

void AppleCDXObj::m_dispose(int nargs) {
	g_director->_system->getAudioCDManager()->stop();
}

void AppleCDXObj::m_still(int nargs) {
	g_director->_system->getAudioCDManager()->stop();
}

// Not implemented yet; needs to be able to return appropriate values
// for playing/paused.
void AppleCDXObj::m_service(int nargs) {
	g_lingo->push(Datum(0));
}

void AppleCDXObj::m_readStatus(int nargs) {
}

void AppleCDXObj::m_getValue(int nargs) {
	AppleCDXObject *me = static_cast<AppleCDXObject *>(g_lingo->_state->me.u.obj);

	g_lingo->push(Datum(me->_returnValue));
}

void AppleCDXObj::m_setInPoint(int nargs) {
	AppleCDXObject *me = static_cast<AppleCDXObject *>(g_lingo->_state->me.u.obj);

	int inpoint = g_lingo->pop().asInt();
	debug(5, "AppleCDXObj::setInPoint: %i", inpoint);
	me->_inpoint = inpoint;
}

void AppleCDXObj::m_setOutPoint(int nargs) {
	AppleCDXObject *me = static_cast<AppleCDXObject *>(g_lingo->_state->me.u.obj);

	int outpoint = g_lingo->pop().asInt();
	debug(5, "AppleCDXObj::setOutPoint: %i", outpoint);
	me->_outpoint = outpoint;
}

void AppleCDXObj::m_playCue(int nargs) {
	// Essentially a noop for us; this asks the drive to seek to that point,
	// then poll until it does. We don't have seek times, so we'll
	// simply noop.
}

void AppleCDXObj::m_playSegment(int nargs) {
	// Performs playback at the pre-specified absolute point on the disc,
	// using the values from setInPoint and setOutPoint
	AppleCDXObject *me = static_cast<AppleCDXObject *>(g_lingo->_state->me.u.obj);

	g_director->_system->getAudioCDManager()->playAbsolute(me->_inpoint, -1, 0, 0);
}

void AppleCDXObj::m_readPos(int nargs) {
	AppleCDXObject *me = static_cast<AppleCDXObject *>(g_lingo->_state->me.u.obj);

	AudioCDManager::Status status = g_director->_system->getAudioCDManager()->getStatus();
	if (me->_cue) {
		// ScummVM currently doesn't support specifying the exact frame, so we pretend we're at the first frame of the song
		Common::CueSheet::CueTrack *track = me->_cue->getTrack(status.track);
		if (track != nullptr) {
			me->_returnValue = track->indices[0];
		}
	}
}

void AppleCDXObj::m_getFirstTrack(int nargs) {
	AppleCDXObject *me = static_cast<AppleCDXObject *>(g_lingo->_state->me.u.obj);

	if (me->_cue) {
		Common::Array<Common::CueSheet::CueTrack> tracks = me->_cue->tracks();
		// If we have a set of tracks parsed, return the first track's number
		int index;
		if (tracks.size() >= 1) {
			index = tracks[0].number;
		} else {
			index = 1;
		}
		debug(5, "AppleCDXObj::m_getFirstTrack: returning %i", index);
		g_lingo->push(Datum(index));
	} else {
		// If we don't have a TOC, just assume the first track is 1
		debug(5, "AppleCDXObj::m_getFirstTrack: returning default");
		g_lingo->push(Datum(1));
	}
}

void AppleCDXObj::m_getLastTrack(int nargs) {
	AppleCDXObject *me = static_cast<AppleCDXObject *>(g_lingo->_state->me.u.obj);

	if (me->_cue) {
		Common::Array<Common::CueSheet::CueTrack> tracks = me->_cue->tracks();
		// If we have a set of tracks parsed, return the final track's number
		int index;
		if (tracks.size() >= 1) {
			index = tracks.back().number;
		} else {
			index = 1;
		}
		debug(5, "AppleCDXObj::m_getLastTrack: returning %i", index);
	} else {
		// If we don't have a TOC, just assume the last track is 1
		debug(5, "AppleCDXObj::m_getLastTrack: returning default");
		g_lingo->push(Datum(1));
	}
}

void AppleCDXObj::m_getFirstFrame(int nargs) {
	AppleCDXObject *me = static_cast<AppleCDXObject *>(g_lingo->_state->me.u.obj);
	int trackNum = g_lingo->pop().asInt();

	if (me->_cue) {
		Common::CueSheet::CueTrack *track = me->_cue->getTrack(trackNum);

		// We use index 1 here because index 0 is typically the
		// pregap, and we don't want to describe the first sector of the
		// pregap as being the first sector of the track. Most discs
		// will have only two indices, and the second index is where the
		// actual track begins.
		int index = track->indices.size() > 1 ? track->indices[1] : track->indices[0];
		debug(5, "AppleCDXObj::m_getFirstFrame(%i): returning %i", trackNum, index);
		g_lingo->push(Datum(index));
	} else {
		// If we don't have a TOC, just provide a stub value
		debug(5, "AppleCDXObj::m_getFirstFrame(%i): returning default", trackNum);
		g_lingo->push(Datum(0));
	}
}

// Currently calculated based on the start of the next track, since
// cuesheets don't contain the duration of a song.
void AppleCDXObj::m_getLastFrame(int nargs) {
	AppleCDXObject *me = static_cast<AppleCDXObject *>(g_lingo->_state->me.u.obj);
	int trackNum = g_lingo->pop().asInt();

	if (me->_cue) {
		// We look for the pregap of the next track, if there is one.
		// TODO opening the actual audio track and getting its length
		// in sectors would produce a more accurate result for the final track
		Common::CueSheet::CueTrack *track = me->_cue->getTrack(trackNum + 1);
		int index;
		if (track) {
			// Don't use the pregap if there is no pregap
			index = track->indices[0] == -1 ? track->indices[1] : track->indices[0];
		} else {
			debug(5, "AppleCDXObj::m_getLastFrame(%i): no track at trackNum %i, setting index to 0", trackNum, trackNum + 1);
			index = 0;
		}
		debug(5, "AppleCDXObj::m_getLastFrame(%i): returning %i", trackNum, index);
		g_lingo->push(Datum(index));
	} else {
		// If we don't have a TOC, just provide a stub value
		debug(5, "AppleCDXObj::m_getLastFrame(%i): returning default", trackNum);
		g_lingo->push(Datum(0));
	}
}

void AppleCDXObj::m_eject(int nargs) {
    debug(5, "AppleCDXObj::eject: Ejecting CD");
}

} // End of namespace Director
