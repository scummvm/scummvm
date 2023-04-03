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
 * Standard Macromedia Director XObject
 *
 *************************************/

/*
 * mNew, portObject, baudRate, playerType  creates a
 *    new instance and returns error code
 *    portObject is an instance from SerialPort XObject.
 *    baudRate should be the same as machine setting.
 *        9600(default for Pioneer 8000),
 *        4800(preferred for Pioneer 2200, 4200),
 *        2400, and 1200.
 *    playerType:
 *        0 for Pioneer 2200, 4200 and 8000
 *        1 for Sony Laser Max 1200, 1500, and 2000
 *    Error codes:
 *        -1 : incorrect BaudRate.
 *        -2 : memory error.
 *        -3 : SerialPort Drivers would not open.
 *
 * mDispose  frees this instance from memory.
 * mName  returns my name.
 * mPlayer  returns the player.
 *
 * Note: all of the following methods will return either "OK" or an error message. Possible error messages include:
 *     "No Response" -- bad connection or wrong baud rate.
 *     "Not Ready"    -- disc ejected or motor stopped.
 *
 * mPlay  normal playback mode in the forward direction.
 * mPlayRev  playback mode in the reverse direction.
 *
 * mFastFwd  fast forward playback mode.
 *     3 times normal speed.
 * mFastRev  fast reverse playback mode.
 *     3 times normal speed.
 *
 * mSlowFwd  slow forward playback mode.
 *     1/5 times normal speed.
 * mSlowRev  slow reverse playback mode
 *     1/5 times normal speed.
 *
 * mStepFwd  step forward a single frame.
 * mStepRev  step reverse a single frame.
 *
 * mPlayJog, nFrame  step multiple frames either forward
 *     or reverse
 *
 * mPlaySpeed, rate  play at slower than normal speed.
 *     rate can be any of the following:
 *         30 is 1x; 15 is 1/2x; 10 is 1/3x; 5 is 1/6x
 *     Example: -10 is one third normal speed, backwards.
 *
 * mPlaySegment, start, end  play a segment of video disc.
 *     Start and end are frame numbers.
 *
 * mPause  set player to display freeze picture (STILL)
 *     When this method is called a second time, this will
 *     continue the mode prior to first call.
 * mStop  halts playback of videodisc.
 * mEject  opens disc compartment and ejects disc.
 *
 * mStopAtFrame, frameNum  set to stop at frameNum
 *
 * mSearchWait, frameNum  search for frameNum and
 *     returns "OK" when search is completed.
 *
 * mReadPos  return the current frame position
 * mShowDisplay, flag  enable/disable frame display
 *
 * mClear  clear all modes of player. Remove Stop markers
 *
 * mVideoControl, videoState  control squelch condition
 *     of video image.
 *
 * mAudioControl, audioState
 *     audioState is one of the following
 *         0 : Turn off both audio channels.
 *         1 : Turn on channel 1 only.
 *         2 : Turn on channel 2 only.
 *         3 : Turn on both audio channels.
 *
 * mStatus  return either "OK" or error message
 *     See the mDescribe for a full list of errors.
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/videodiscxobj.h"

namespace Director {

const char *VideodiscXObj::xlibName = "LaserDisc";
const char *VideodiscXObj::fileNames[] = {
	"Videodisc XObj",
	"LaserDisc",
	nullptr
};

static MethodProto xlibMethods[] = {
	{ "new",			VideodiscXObj::m_new,			 3, 3,	200 },	// D2
	{ "Name",			VideodiscXObj::m_name,			 0, 0,	200 },	// D2
	{ "Player",			VideodiscXObj::m_player,		 0, 0,	200 },	// D2
	{ "Play",			VideodiscXObj::m_play,			 0, 0,	200 },	// D2
	{ "PlayRev",		VideodiscXObj::m_playRev,		 0, 0,	200 },	// D2
	{ "FastFwd",		VideodiscXObj::m_fastFwd,		 0, 0,	200 },	// D2
	{ "FastRev",		VideodiscXObj::m_fastRev,		 0, 0,	200 },	// D2
	{ "SlowFwd",		VideodiscXObj::m_slowFwd,		 0, 0,	200 },	// D2
	{ "SlowRev",		VideodiscXObj::m_slowRev,		 0, 0,	200 },	// D2
	{ "StepFwd",		VideodiscXObj::m_stepFwd,		 0, 0,	200 },	// D2
	{ "StepRev",		VideodiscXObj::m_stepRev,		 0, 0,	200 },	// D2
	{ "PlayJog",		VideodiscXObj::m_playJog,		 1, 1,	200 },	// D2
	{ "PlaySpeed",		VideodiscXObj::m_playSpeed,		 1, 1,	200 },	// D2
	{ "PlaySegment",	VideodiscXObj::m_playSegment,	 2, 2,	200 },	// D2
	{ "Pause",			VideodiscXObj::m_pause,			 0, 0,	200 },	// D2
	{ "Stop",			VideodiscXObj::m_stop,			 0, 0,	200 },	// D2
	{ "Eject",			VideodiscXObj::m_eject,			 0, 0,	200 },	// D2
	{ "StopAtFrame",	VideodiscXObj::m_stopAtFrame,	 1, 1,	200 },	// D2
	{ "SearchWait",		VideodiscXObj::m_searchWait,	 1, 1,	200 },	// D2
	{ "ReadPos",		VideodiscXObj::m_readPos,		 0, 0,	200 },	// D2
	{ "ShowDisplay",	VideodiscXObj::m_showDisplay,	 0, 0,	200 },	// D2
	{ "Clear",			VideodiscXObj::m_clear,			 0, 0,	200 },	// D2
	{ "VideoControl",	VideodiscXObj::m_videoControl,	 1, 1,	200 },	// D2
	{ "AudioControl",	VideodiscXObj::m_audioControl,	 1, 1,	200 },	// D2
	{ "Status",			VideodiscXObj::m_status,		 0, 0,	200 },	// D2
	{ nullptr, nullptr, 0, 0, 0 }
};

void VideodiscXObj::open(int type) {
	if (type == kXObj) {
		VideodiscXObject::initMethods(xlibMethods);
		VideodiscXObject *xobj = new VideodiscXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void VideodiscXObj::close(int type) {
	if (type == kXObj) {
		VideodiscXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


VideodiscXObject::VideodiscXObject(ObjectType ObjectType) :Object<VideodiscXObject>("LaserDisc") {
	_objType = ObjectType;
}

void VideodiscXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("VideodiscXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

void VideodiscXObj::m_name(int nargs) {
	g_lingo->printSTUBWithArglist("VideodiscXObj::m_name", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void VideodiscXObj::m_player(int nargs) {
	g_lingo->printSTUBWithArglist("VideodiscXObj::m_player", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void VideodiscXObj::m_play(int nargs) {
	g_lingo->printSTUBWithArglist("VideodiscXObj::m_play", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void VideodiscXObj::m_playRev(int nargs) {
	g_lingo->printSTUBWithArglist("VideodiscXObj::m_playRev", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void VideodiscXObj::m_fastFwd(int nargs) {
	g_lingo->printSTUBWithArglist("VideodiscXObj::m_fastFwd", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void VideodiscXObj::m_fastRev(int nargs) {
	g_lingo->printSTUBWithArglist("VideodiscXObj::m_fastRev", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void VideodiscXObj::m_slowFwd(int nargs) {
	g_lingo->printSTUBWithArglist("VideodiscXObj::m_slowFwd", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void VideodiscXObj::m_slowRev(int nargs) {
	g_lingo->printSTUBWithArglist("VideodiscXObj::m_slowRev", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void VideodiscXObj::m_stepFwd(int nargs) {
	g_lingo->printSTUBWithArglist("VideodiscXObj::m_stepFwd", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void VideodiscXObj::m_stepRev(int nargs) {
	g_lingo->printSTUBWithArglist("VideodiscXObj::m_stepRev", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void VideodiscXObj::m_playJog(int nargs) {
	g_lingo->printSTUBWithArglist("VideodiscXObj::m_playJog", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void VideodiscXObj::m_playSpeed(int nargs) {
	g_lingo->printSTUBWithArglist("VideodiscXObj::m_playSpeed", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void VideodiscXObj::m_playSegment(int nargs) {
	g_lingo->printSTUBWithArglist("VideodiscXObj::m_playSegment", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void VideodiscXObj::m_pause(int nargs) {
	g_lingo->printSTUBWithArglist("VideodiscXObj::m_pause", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void VideodiscXObj::m_stop(int nargs) {
	g_lingo->printSTUBWithArglist("VideodiscXObj::m_stop", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void VideodiscXObj::m_eject(int nargs) {
	g_lingo->printSTUBWithArglist("VideodiscXObj::m_eject", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void VideodiscXObj::m_stopAtFrame(int nargs) {
	g_lingo->printSTUBWithArglist("VideodiscXObj::m_stopAtFrame", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void VideodiscXObj::m_searchWait(int nargs) {
	g_lingo->printSTUBWithArglist("VideodiscXObj::m_searchWait", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void VideodiscXObj::m_readPos(int nargs) {
	g_lingo->printSTUBWithArglist("VideodiscXObj::m_readPos", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void VideodiscXObj::m_showDisplay(int nargs) {
	g_lingo->printSTUBWithArglist("VideodiscXObj::m_showDisplay", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void VideodiscXObj::m_clear(int nargs) {
	g_lingo->printSTUBWithArglist("VideodiscXObj::m_clear", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void VideodiscXObj::m_videoControl(int nargs) {
	g_lingo->printSTUBWithArglist("VideodiscXObj::m_videoControl", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void VideodiscXObj::m_audioControl(int nargs) {
	g_lingo->printSTUBWithArglist("VideodiscXObj::m_audioControl", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void VideodiscXObj::m_status(int nargs) {
	g_lingo->printSTUBWithArglist("VideodiscXObj::m_status", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}


} // End of namespace Director
