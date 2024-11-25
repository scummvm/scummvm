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
 * The Apartment 2.0
 * Cellofania
 *
 *************************************/

/*
 * --AppleAudioCD, CDAudio, 1.0, 4/3/90
 * --
 * --© 1989, 1990 MacroMind, Inc.
 * -- by Jeff Tanner
 * --
 * ------------------------------------------------------
 * ------------------------------------------------------
 * --  An Apple CD SC Player must be mounted
 * --      in order for this XObject to operate properly.
 * --      The easiest way to check for mounting is to
 * --      check the desktop for the CD icon.
 * ------------------------------------------------------
 * ------------------------------------------------------
 * -- This XObject recognizes only the first player in the SCSI chain .
 * ------------------------------------------------------
 * ------------------------------------------------------
 * --=METHODS=--
 * X   mNew --Creates a new instance of the XObject.
 * X   mDispose --Disposes of the instance.
 * S   mName --Returns name of the XObject.
 * ------------------------------------------------------
 * ------------------------------------------------------
 * -- PLAY CD METHODS:
 * S   mPlay --Plays current track from the beginning.
 * SI mPlayTrack, trackNum --Plays the specified track from the beginning.
 * SS   mPlayName, trackName --Plays by track name.
 * --      Note: The full pathname is unnecessary.
 * --      The track names for tracks 1 - 9 are
 * --          "TRACK<space><space>trackNum" and for tracks 10 to 99 are
 * --          "TRACK<space>trackNum".
 * SIII mPlayAbsTime, minute, second, frame --Starts play at absolute time position on current CD-ROM.
 * --
 * SIIIIII mPlaySegment, startMin, startSec, startFrm, stopMin, stopSec, stopFrm
 * --          < startMin, startSec, startFrm > - Start time
 * --          < stopMin, stopSec, stopFrm > - Stop time
 * SII  mAskPlay, leftDialog, topDialog        --With a file dialog box,
 * --                                  selects an Audio track to play.
 * --          < leftDialog, topDialog > - Where to place the file dialog box
 * ------------------------------------------------------
 * S   mStepFwd --Steps forward one track and Plays.
 * S   mStepBwd --Steps back one track and Plays.
 * --
 * S    mPause --Pauses the player.
 * --      When this method is called a second time,
 * --      the player will continue in normal play mode.
 * S    mContinue --Continues the mode prior to calling mPause.
 * --
 * S    mStop                              --Stops play.
 * SI   mStopTrack, trackNum               --Stops when the selected track finishes playing.
 * SIII mStopAbsTime, minute, second, frame --Stops play at a specified absolute time position.
 * S    mRemoveStop -- Removes stop conditions.
 * --          Stop conditions are set with these methods:
 * --                  mPlaySegment
 * --                  mStopTrack
 * --                  mStopAbsTime
 * --
 * S    mEject --Ejects CD-ROM from drive.
 * --
 * ------------------------------------------------------
 * ------------------------------------------------------
 * -- STATUS METHODS:
 * S    mStatus --Returns status of Audio CD player.
 * --      Returns message strings:
 * --          Audio play in progress
 * --          Audio pause in operation
 * --          Audio muting on
 * --          Audio play operation completed
 * --          Error occurred during audio play
 * --          Not currently playing
 * --
 * S    mPlayMode      --Returns a play mode from audio track.
 * --      The play mode describes how to play the audio track.
 * --      Returns message strings:
 * --          Muting on (no audio)
 * --          Right channel through right channel only
 * --          Left channel through right channel only
 * --          Left and right channels through right channel only
 * --          Right channel through left channel only
 * --          Right channel through left and right channel
 * --          Right channel through left channel,
 * --              Left channel through right channel
 * --          Right channel through left channel,
 * --              Left and right channels through right channel
 * --          Left channel through left channel only
 * --          Left channel through left channel,
 * --              Right channel through right channel (Stereo)
 * --          Left channel through left and right channel
 * --          Left channel through left channel,
 * --              Left and right channels through right channel
 * --          Left and right channels through left channel only
 * --          Left and right channels through left channel,
 * --              Right channel through right channel
 * --          Left and right channels through left channel,
 * --              Left channel through right channel
 * --          Left and right channels through
 * --              both left channel and right channel (Mono)
 * --
 * S    mCurrentFormat         --Returns the format of the current track.
 * --      Returns message strings:
 * --          2 audio channels without preemphasis
 * --          2 audio channels with preemphasis
 * --          4 audio channels without preemphasis
 * --          4 audio channels with preemphasis
 * --          Data track
 * --
 * ------------------------------------------------------
 * ------------------------------------------------------
 * --
 * I    mCurrentTrack --Returns number of the current track.
 * S    mCurrentTime  --Returns the current absolute time (min:sec:frm).
 * --
 * I    mFirstTrack  -- Returns first track number on current CD-ROM.
 * I    mLastTrack  -- Returns last track number on current CD-ROM.
 * S    mTotalTime -- Returns total time on current CD-ROM (min:sec:frm)
 * ------------------------------------------------------
 * ------------------------------------------------------
 * -- SCANNING METHODS:
 * -- Starting at a specific time:
 * --   min, sec, and frm parameters are to indicate
 * --      the absolute time to start scan.
 * --   monitorP - if true, it will stop scan moment mouse
 * --      is released, and continue playing at current position.
 * --      However, this will inhibit all other events.
 * --      Otherwise use mStopScan method.
 * SIIII mScanFwd min, sec, frm, monitorP -- Fast forward scan
 * SIIII mScanBwd min, sec, frm, monitorP -- Fast reverse scan
 * --
 * S   mStopScan --Stops scan and continues playing at current position.
 * --
 * --  End description of AppleAudioCD XObject methods.
 * ------------------------------------------------------
 * ------------------------------------------------------
 */

#include "backends/audiocd/audiocd.h"
#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/cdromxobj.h"

namespace Director {

const char *const CDROMXObj::xlibName = "AppleAudioCD";
const XlibFileDesc CDROMXObj::fileNames[] = {
	{ "CD-ROM XObj",	nullptr },
	{ "AppleAudioCD",	nullptr },
	{ nullptr,			nullptr },
};

static const MethodProto xlibMethods[] = {
	{ "new",			CDROMXObj::m_new,			 0, 0,	200 },	// D2
	{ "Name",			CDROMXObj::m_name,		 	 0, 0,	200 },	// D2
	{ "Play",			CDROMXObj::m_play,		 	 0, 0,	200 },	// D2
	{ "PlayTrack",		CDROMXObj::m_playTrack,	 	 1, 1,	200 },	// D2
	{ "PlayName",		CDROMXObj::m_playName,	 	 1, 1,  200 },	// D2
	{ "PlayAbsTime",	CDROMXObj::m_playAbsTime,	 3, 3,  200 },	// D2
	{ "PlaySegment",	CDROMXObj::m_playSegment,	 6, 6,  200 },	// D2
	{ "AskPlay",		CDROMXObj::m_askPlay,		 2, 2,  200 },	// D2
	{ "StepFwd",		CDROMXObj::m_stepFwd,		 0, 0,  200 },	// D2
	{ "StepBwd",		CDROMXObj::m_stepBwd,		 0, 0,  200 },	// D2
	{ "Pause",			CDROMXObj::m_pause,			 0, 0,  200 },	// D2
	{ "Continue",		CDROMXObj::m_continue,		 0, 0,  200 },	// D2
	{ "Stop",			CDROMXObj::m_stop,			 0, 0,  200 },	// D2
	{ "StopTrack",		CDROMXObj::m_stopTrack,		 1, 1,  200 },	// D2
	{ "StopAbsTime",	CDROMXObj::m_stopAbsTime,	 3, 3,  200 },	// D2
	{ "RemoveStop",		CDROMXObj::m_removeStop,	 0, 0,  200 },	// D2
	{ "Eject",			CDROMXObj::m_eject,			 0, 0,  200 },	// D2
	{ "Status",			CDROMXObj::m_status,		 0, 0,  200 },	// D2
	{ "PlayMode",		CDROMXObj::m_playMode,		 0, 0,  200 },	// D2
	{ "CurrentFormat",	CDROMXObj::m_currentFormat,  0, 0,  200 },	// D2
	{ "CurrentTrack",	CDROMXObj::m_currentTrack,	 0, 0,  200 },	// D2
	{ "CurrentTime",	CDROMXObj::m_currentTime,	 0, 0,  200 },	// D2
	{ "FirstTrack",		CDROMXObj::m_firstTrack,	 0, 0,  200 },	// D2
	{ "LastTrack",		CDROMXObj::m_lastTrack,		 0, 0,  200 },	// D2
	{ "TotalTime",		CDROMXObj::m_totalTime,		 0, 0,  200 },	// D2
	{ "ScanFwd",		CDROMXObj::m_scanFwd,		 4, 4,  200 },	// D2
	{ "ScanBwd",		CDROMXObj::m_scanBwd,		 4, 4,  200 },	// D2
	{ "StopScan",		CDROMXObj::m_stopScan,		 0, 0,  200 },	// D2
	{ nullptr, nullptr, 0, 0, 0 }
};

void CDROMXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		CDROMXObject::initMethods(xlibMethods);
		CDROMXObject *xobj = new CDROMXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void CDROMXObj::close(ObjectType type) {
	if (type == kXObj) {
		CDROMXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
		g_director->_system->getAudioCDManager()->close();
	}
}


CDROMXObject::CDROMXObject(ObjectType ObjectType) :Object<CDROMXObject>("AppleAudioCD") {
	_objType = ObjectType;
	// Initialize _cdda_status
	_cdda_status.playing = false;
	_cdda_status.track = 0;
	_cdda_status.start = 0;
	_cdda_status.duration = 0;
	_cdda_status.numLoops = 0;
	_cdda_status.volume = Audio::Mixer::kMaxChannelVolume;
	_cdda_status.balance = 0;
}

void CDROMXObj::m_new(int nargs) {
	g_director->_system->getAudioCDManager()->open();
	g_lingo->printSTUBWithArglist("CDROMXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

// Returns the name of the XObj
void CDROMXObj::m_name(int nargs) {
	g_lingo->push(Datum("AppleAudioCD"));
}

void CDROMXObj::m_play(int nargs) {
	CDROMXObject *me = static_cast<CDROMXObject *>(g_lingo->_state->me.u.obj);

	// This is a request to play the current track from the start,
	// which we can't do if there's no track information.
	if (me->_cdda_status.track == 0)
		return;

	g_director->_system->getAudioCDManager()->play(me->_cdda_status.track, -1, 0, 0);
	me->_cdda_status = g_director->_system->getAudioCDManager()->getStatus();
}

void CDROMXObj::m_playTrack(int nargs) {
	CDROMXObject *me = static_cast<CDROMXObject *>(g_lingo->_state->me.u.obj);

	int track = g_lingo->pop().asInt();
	g_director->_system->getAudioCDManager()->play(track - 1, -1, 0, 0);
	me->_cdda_status = g_director->_system->getAudioCDManager()->getStatus();
}

// Name format is "TRACK NN", with one-digit tracks padded with a leading space
void CDROMXObj::m_playName(int nargs) {
	CDROMXObject *me = static_cast<CDROMXObject *>(g_lingo->_state->me.u.obj);

	Common::String track = g_lingo->pop().asString();
	if (track.size() < 8) {
		warning("CDROMXObj::m_playName: specified name has an invalid format (provided string was %s)", track.c_str());
		return;
	}
	Common::String trackNum = track.substr(6, 2);
	// Remove the leading string as needed
	if (trackNum.substr(0, 1) == " ")
		trackNum = trackNum.substr(1, 1);

	int trackNumI = atoi(trackNum.c_str());
	if (trackNumI < 1) {
		warning("CDROMXObj::m_playName: track number failed to parse (provided string was %s)", track.c_str());
	}

	g_director->_system->getAudioCDManager()->play(trackNumI - 1, -1, 0, 0);
	me->_cdda_status = g_director->_system->getAudioCDManager()->getStatus();
}

void CDROMXObj::m_playAbsTime(int nargs) {
	CDROMXObject *me = static_cast<CDROMXObject *>(g_lingo->_state->me.u.obj);

	Datum min = g_lingo->pop();
	Datum sec = g_lingo->pop();
	Datum frac = g_lingo->pop();

	int startFrame = (min.asInt() * 60 * 75) + (sec.asInt() * 75) + frac.asInt();
	debug(5, "CDROMXObj::m_playAbsTime: playing at frame %i", startFrame);
	g_director->_system->getAudioCDManager()->playAbsolute(startFrame, -1, 0);
	me->_cdda_status = g_director->_system->getAudioCDManager()->getStatus();

	g_lingo->push(Datum());
}

void CDROMXObj::m_playSegment(int nargs) {
	Datum startMin = g_lingo->pop();
	Datum startSec = g_lingo->pop();
	Datum startFrac = g_lingo->pop();
	Datum endMin = g_lingo->pop();
	Datum endSec = g_lingo->pop();
	Datum endFrac = g_lingo->pop();
	// Can't implement this without implementing a full CD TOC, since
	// it doesn't interact with songs at the "track" level.
	debug(5, "STUB: CDROMXObj::m_playSegment Request to play starting at %i:%i.%i and ending at %i:%i.%i", startMin.asInt(), startSec.asInt(), startFrac.asInt(), endMin.asInt(), endSec.asInt(), endFrac.asInt());
	g_lingo->push(Datum());
}

XOBJSTUBV(CDROMXObj::m_askPlay)

void CDROMXObj::m_stepFwd(int nargs) {
	CDROMXObject *me = static_cast<CDROMXObject *>(g_lingo->_state->me.u.obj);

	g_director->_system->getAudioCDManager()->play(me->_cdda_status.track + 1, -1, 0, 0);
	me->_cdda_status = g_director->_system->getAudioCDManager()->getStatus();
}

void CDROMXObj::m_stepBwd(int nargs) {
	CDROMXObject *me = static_cast<CDROMXObject *>(g_lingo->_state->me.u.obj);

	int track = me->_cdda_status.track - 1;
	if (track < 1)
		track = 1;

	g_director->_system->getAudioCDManager()->play(track, -1, 0, 0);
	me->_cdda_status = g_director->_system->getAudioCDManager()->getStatus();
}

void CDROMXObj::m_pause(int nargs) {
	CDROMXObject *me = static_cast<CDROMXObject *>(g_lingo->_state->me.u.obj);

	// Leaves a trace of the current position so we can resume from it
	me->_cdda_status = g_director->_system->getAudioCDManager()->getStatus();
	me->_cdda_status.playing = false;
	g_director->_system->getAudioCDManager()->stop();
}

void CDROMXObj::m_continue(int nargs) {
	CDROMXObject *me = static_cast<CDROMXObject *>(g_lingo->_state->me.u.obj);

	// Can only resume if there's data to resume from
	if (me->_cdda_status.track == 0)
		return;

	g_director->_system->getAudioCDManager()->play(me->_cdda_status.track, -1, me->_cdda_status.start, 0);
	me->_cdda_status = g_director->_system->getAudioCDManager()->getStatus();
}

void CDROMXObj::m_stop(int nargs) {
	CDROMXObject *me = static_cast<CDROMXObject *>(g_lingo->_state->me.u.obj);

	g_director->_system->getAudioCDManager()->stop();
	me->_cdda_status = g_director->_system->getAudioCDManager()->getStatus();
}

void CDROMXObj::m_stopTrack(int nargs) {
	CDROMXObject *me = static_cast<CDROMXObject *>(g_lingo->_state->me.u.obj);

	Datum track = g_lingo->pop();
	AudioCDManager::Status status = g_director->_system->getAudioCDManager()->getStatus();

	if (!status.playing)
		return;

	// stopTrack isn't "stop now", but "stop after this track".
	// This play command ensures we continue from here and end with this
	// track, regardless of previous commands.
	g_director->_system->getAudioCDManager()->play(status.track, 1, status.start, status.start + status.duration);
	me->_cdda_status = g_director->_system->getAudioCDManager()->getStatus();
}

void CDROMXObj::m_stopAbsTime(int nargs) {
	Datum min = g_lingo->pop();
	Datum sec = g_lingo->pop();
	Datum frac = g_lingo->pop();
	// Can't implement this without implementing a full CD TOC, since
	// it doesn't interact with songs at the "track" level.
	debug(5, "STUB: CDROMXObj::m_stopAbsTime Request to play starting at %i:%i.%i", min.asInt(), sec.asInt(), frac.asInt());
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void CDROMXObj::m_removeStop(int nargs) {
	CDROMXObject *me = static_cast<CDROMXObject *>(g_lingo->_state->me.u.obj);

	Datum track = g_lingo->pop();
	AudioCDManager::Status status = g_director->_system->getAudioCDManager()->getStatus();

	if (!status.playing)
		return;

	g_director->_system->getAudioCDManager()->play(status.track, -1, status.start, status.start + status.duration);
	me->_cdda_status = g_director->_system->getAudioCDManager()->getStatus();
}

void CDROMXObj::m_eject(int nargs) {
	warning("If you had had a CD drive, it would have ejected just now.");
}

// Valid strings are:
// "Audio play in progress"
// "Audio pause in operation"
// "Audio muting on"
// "Audio play operation completed"
// "Error occurred during audio play"
// "Not currently playing"
void CDROMXObj::m_status(int nargs) {
	// A fuller implementation could also track data to return the
	// "pause" and "completed" states.
	if (g_director->_system->getAudioCDManager()->isPlaying())
		g_lingo->push(Datum("Audio play in progress"));
	else
		g_lingo->push(Datum("Not currently playing"));
}

// Valid strings are:
// "Muting on (no audio)"
// "Right channel through right channel only"
// "Left channel through right channel only"
// "Left and right channels through right channel only"
// "Right channel through left channel only"
// "Right channel through left and right channel"
// "Right channel through left channel"
// "Left channel through right channel"
// "Right channel through left channel"
// "Left and right channels through right channel"
// "Left channel through left channel only"
// "Left channel through left channel"
// "Right channel through right channel (Stereo)"
// "Left channel through left and right channel"
// "Left channel through left channel"
// "Left and right channels through right channel"
// "Left and right channels through left channel only"
// "Left and right channels through left channel"
// "Left and right channels through left channel"
// "Right channel through right channel"
// "Left and right channels through left channel"
// "Left channel through right channel"
// "Left and right channels through"
// "both left channel and right channel (Mono)"
void CDROMXObj::m_playMode(int nargs) {
	// For now, nothing to change modes is implemented, so just return
	// a default
	g_lingo->push(Datum("Right channel through right channel (Stereo)"));
}

// Valid strings are:
// "audio channels without preemphasis"
// "audio channels with preemphasis"
void CDROMXObj::m_currentFormat(int nargs) {
	// Preemphasis not implemented, so just return this
	g_lingo->push(Datum("audio channels without preemphasis"));
}

void CDROMXObj::m_currentTrack(int nargs) {
	CDROMXObject *me = static_cast<CDROMXObject *>(g_lingo->_state->me.u.obj);

	g_lingo->push(Datum(me->_cdda_status.track));
}

XOBJSTUBV(CDROMXObj::m_currentTime)

// The next few methods depend on full TOC implementation, so they
// can't be implemented right now.
XOBJSTUBV(CDROMXObj::m_firstTrack)
XOBJSTUBV(CDROMXObj::m_lastTrack)
XOBJSTUBV(CDROMXObj::m_totalTime)

// The scan methods depend on absolute timing, so they also require
// a full TOC.
XOBJSTUBV(CDROMXObj::m_scanFwd)
XOBJSTUBV(CDROMXObj::m_scanBwd)
XOBJSTUBV(CDROMXObj::m_stopScan)

} // End of namespace Director
