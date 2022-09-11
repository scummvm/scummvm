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

/*
 * mNew	 creates a new object instance for a single
 *     CD-ROM player. if the Apple Audio CD Player is not
 *     mounted the instance will not be created.
 *
 * mDispose  disposes of the current instance.
 *     This should always be called when you are finished
 *     using the XObject.
 *
 * mName  returns the name of the XObj.
 *
 * mPlay  plays the  current track from the beginning.
 *
 * mPlayTrack, trackNum  plays the specified track number
 *     from its beginning.
 *
 * mPlayName, trackName  plays the specified track name.
 *     (Note: volume name is ignored.)
 *
 * mPlayAbsTime, minute, second, frame  plays starting at
 *     the specified absolute time on current CD-ROM.
 *
 * mPlaySegment, startMin, startSec, startFrm, stopMin, stopSec, stopFrm  plays starting at the specified absolute
 *     time, and stops at the specified stopping point.
 *
 * mAskPlay, leftDialogPosition, topDialogPosition  opens
 *     a standard file dialog for the user to select a track.
 *
 * mStepFwd  steps forward one track and plays.
 *
 * mStepBwd  steps back one track and plays.
 *
 * mPause  pauses the player. When this method is called a
 *     second time, the player will continue the mode prior to
 *     first call; just like calling mContinue.
 *
 * mContinue  continues the mode prior to calling mPause.
 *
 * mStop  stops playing.
 *
 * mStopTrack, trackNum  stops when specified track
 *     finishes playing.
 *
 * mStopAbsTime, minute, second, frame  stops play at
 *     absolute time position.
 *
 * mRemoveStop  removes the conditions which are set
 *     with: mPlaySegment, mStopTrack and mStopAbsTime.
 *
 * mEject  ejects the CD from the drive.
 *
 * mStatus  returns the status of the CD player. See the
 *     mDescribe for a list of messages which are returned.
 *
 * mPlayMode  returns the play mode from an audio track.
 *     The play mode describes how to play the audio track.
 *     See the mDescribe for a list of messages.
 *
 * mCurrentFormat  returns the format of the current track
 *     See the mDescribe for a list of messages.
 *
 * mCurrentTrack  returns the current track.
 *
 * mCurrentTime  returns the current time (min:sec:frame).
 *
 * mFirstTrack  returns the first track number on current CD
 *
 * mLastTrack  returns the last track number on current CD
 *
 * mTotalTime  returns the total time on current CD in
 *     minutes:seconds:frames.
 *
 * mScanFwd min, sec, frm, monitorP  scans forward.
 * mScanBwd min, sec, frm, monitorP  scans backwards.
 *   min, sec, and frm are the absolute time to start scan.
 *   monitorP — if true, will stop scan when mouse is
 *   released, and continue playing at current position.
 *   However, this will inhibit all other events.
 *   Otherwise use the mStopScan method.
 *
 * mStopScan  stops scan and continues playing at current
 *     position.
 */

#include "backends/audiocd/audiocd.h"
#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/cdromxobj.h"

namespace Director {

const char *CDROMXObj::xlibName = "AppleAudioCD";
const char *CDROMXObj::fileNames[] = {
	"CD-ROM XObj",
	"AppleAudioCD",
	nullptr
};

static MethodProto xlibMethods[] = {
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

void CDROMXObj::open(int type) {
	if (type == kXObj) {
		CDROMXObject::initMethods(xlibMethods);
		CDROMXObject *xobj = new CDROMXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void CDROMXObj::close(int type) {
	if (type == kXObj) {
		CDROMXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
        g_director->_system->getAudioCDManager()->close();
	}
}


CDROMXObject::CDROMXObject(ObjectType ObjectType) :Object<CDROMXObject>("AppleAudioCD") {
	_objType = ObjectType;
}

void CDROMXObj::m_new(int nargs) {
    g_director->_system->getAudioCDManager()->open();
	g_lingo->printSTUBWithArglist("CDROMXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_currentMe);
}

// Returns the name of the XObj
void CDROMXObj::m_name(int nargs) {
	g_lingo->push(Datum("AppleAudioCD"));
}

void CDROMXObj::m_play(int nargs) {
    // This is a request to play the current track from the start,
    // which we can't do if there's no track information.
    if (g_director->_cdda_status.track == 0)
        return;

    g_director->_system->getAudioCDManager()->play(g_director->_cdda_status.track, -1, 0, 0);
    g_director->_cdda_status = g_director->_system->getAudioCDManager()->getStatus();
}

void CDROMXObj::m_playTrack(int nargs) {
    int track = g_lingo->pop().asInt();
    g_director->_system->getAudioCDManager()->play(track, -1, 0, 0);
    g_director->_cdda_status = g_director->_system->getAudioCDManager()->getStatus();
}

// Name format is "TRACK NN", with one-digit tracks padded with a leading space
void CDROMXObj::m_playName(int nargs) {
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

    g_director->_system->getAudioCDManager()->play(trackNumI, -1, 0, 0);
    g_director->_cdda_status = g_director->_system->getAudioCDManager()->getStatus();
}

void CDROMXObj::m_playAbsTime(int nargs) {
	Datum min = g_lingo->pop();
    Datum sec = g_lingo->pop();
    Datum frac = g_lingo->pop();
    // Can't implement this without implementing a full CD TOC, since
    // it doesn't interact with songs at the "track" level.
    debug(5, "STUB: CDROMXObj::m_playAbsTime Request to play starting at %i:%i.%i", min.asInt(), sec.asInt(), frac.asInt());
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

void CDROMXObj::m_askPlay(int nargs) {
	g_lingo->printSTUBWithArglist("CDROMXObj::m_askPlay", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void CDROMXObj::m_stepFwd(int nargs) {
    g_director->_system->getAudioCDManager()->play(g_director->_cdda_status.track + 1, -1, 0, 0);
    g_director->_cdda_status = g_director->_system->getAudioCDManager()->getStatus();
}

void CDROMXObj::m_stepBwd(int nargs) {
    int track = g_director->_cdda_status.track - 1;
    if (track < 1)
        track = 1;

    g_director->_system->getAudioCDManager()->play(track, -1, 0, 0);
    g_director->_cdda_status = g_director->_system->getAudioCDManager()->getStatus();
}

void CDROMXObj::m_pause(int nargs) {
    // Leaves a trace of the current position so we can resume from it
    g_director->_cdda_status = g_director->_system->getAudioCDManager()->getStatus();
    g_director->_cdda_status.playing = false;
    g_director->_system->getAudioCDManager()->stop();
}

void CDROMXObj::m_continue(int nargs) {
    // Can only resume if there's data to resume from
    if (g_director->_cdda_status.track == 0)
        return;

    g_director->_system->getAudioCDManager()->play(g_director->_cdda_status.track, -1, g_director->_cdda_status.start, 0);
    g_director->_cdda_status = g_director->_system->getAudioCDManager()->getStatus();
}

void CDROMXObj::m_stop(int nargs) {
    g_director->_system->getAudioCDManager()->stop();
    g_director->_cdda_status = g_director->_system->getAudioCDManager()->getStatus();
}

void CDROMXObj::m_stopTrack(int nargs) {
    Datum track = g_lingo->pop();
    AudioCDManager::Status status = g_director->_system->getAudioCDManager()->getStatus();

    if (!status.playing)
        return;

    // stopTrack isn't "stop now", but "stop after this track".
    // This play command ensures we continue from here and end with this
    // track, regardless of previous commands.
    g_director->_system->getAudioCDManager()->play(status.track, 1, status.start, status.start + status.duration);
    g_director->_cdda_status = g_director->_system->getAudioCDManager()->getStatus();
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
    Datum track = g_lingo->pop();
    AudioCDManager::Status status = g_director->_system->getAudioCDManager()->getStatus();

    if (!status.playing)
        return;

    g_director->_system->getAudioCDManager()->play(status.track, -1, status.start, status.start + status.duration);
    g_director->_cdda_status = g_director->_system->getAudioCDManager()->getStatus();
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
	g_lingo->push(Datum(g_director->_cdda_status.track));
}

void CDROMXObj::m_currentTime(int nargs) {
	g_lingo->printSTUBWithArglist("CDROMXObj::m_currentTime", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

// The next few methods depend on full TOC implementation, so they
// can't be implemented right now.
void CDROMXObj::m_firstTrack(int nargs) {
	g_lingo->printSTUBWithArglist("CDROMXObj::m_firstTrack", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void CDROMXObj::m_lastTrack(int nargs) {
	g_lingo->printSTUBWithArglist("CDROMXObj::m_lastTrack", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void CDROMXObj::m_totalTime(int nargs) {
	g_lingo->printSTUBWithArglist("CDROMXObj::m_totalTime", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

// The scan methods depend on absolute timing, so they also require
// a full TOC.
void CDROMXObj::m_scanFwd(int nargs) {
	g_lingo->printSTUBWithArglist("CDROMXObj::m_scanFwd", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void CDROMXObj::m_scanBwd(int nargs) {
	g_lingo->printSTUBWithArglist("CDROMXObj::m_scanBwd", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void CDROMXObj::m_stopScan(int nargs) {
	g_lingo->printSTUBWithArglist("CDROMXObj::m_stopScan", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}


} // End of namespace Director
