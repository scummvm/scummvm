/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

/*************************************
 *
 * USED IN:
 * Meet MediaBand
 *
 *************************************/

/*
	-- SoundJam Copyright © Canter Technology 1995
	SoundJam
	II   mNew, numberOfChannels
	ISI  mDefineFileSound, fullPathName, numberOfBeats
	III  mDefineCastSound, castMemberNumber, numberOfBeats
	II   mUndefineSound, soundID
	III  mReadSome, soundID, byteCount
	II   mStartSound, soundID
	II   mSwitchNew, soundID
	II   mSwitchParallel, soundID
	I    mHasSwitchHappened
	X    mToggleMute
	X    mStop
	X    mDispose
*/

#include "director/director.h"
#include "director/window.h"
#include "director/sound.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/soundjam.h"

namespace Director {

const char *SoundJam::xlibName = "SoundJam";
const char *SoundJam::fileNames[] = {
	"SoundJam",
	0
};

const int kJamChannel = 3;

static MethodProto xlibMethods[] = {
	{ "new",				SoundJam::m_new,			 1, 1,	400 },
	{ "defineFileSound",	SoundJam::m_defineFileSound, 2, 2,	400 },
	{ "defineCastSound",	SoundJam::m_defineCastSound, 2, 2,	400 },
	{ "undefineSound",		SoundJam::m_undefineSound,	 1, 1,	400 },
	{ "readSome",			SoundJam::m_readSome,		 2, 2,	400 },
	{ "startSound",			SoundJam::m_startSound,		 1, 1,	400 },
	{ "switchNew",			SoundJam::m_switchNew,		 1, 1,	400 },
	{ "switchParallel",		SoundJam::m_switchParallel,	 1, 1,	400 },
	{ "hasSwitchHappened",	SoundJam::m_hasSwitchHappened, 0, 0, 400 },
	{ "toggleMute",			SoundJam::m_toggleMute,		 0, 0,	400 },
	{ "stop",				SoundJam::m_stop,			 0, 0,	400 },
	{ 0, 0, 0, 0, 0 }
};

void SoundJam::open(int type) {
	if (type == kXObj) {
		SoundJamObject::initMethods(xlibMethods);
		SoundJamObject *xobj = new SoundJamObject(kXObj);
		g_lingo->_globalvars[xlibName] = xobj;
	}
}

void SoundJam::close(int type) {
	if (type == kXObj) {
		SoundJamObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}

SoundJamObject::SoundJamObject(ObjectType objType) : Object<SoundJamObject>("SoundJam") {
	_objType = objType;
}

void SoundJam::m_new(int nargs) {
	int numberOfChannels = g_lingo->pop().asInt();

	if (numberOfChannels != 1) {
		warning("SoundJam::m_new: Expected numberOfChannels = 1, got %d", numberOfChannels);
		g_lingo->push(Datum());
		return;
	}

	g_lingo->push(g_lingo->_currentMe);
}

void SoundJam::m_defineFileSound(int nargs) {
	g_lingo->printSTUBWithArglist("SoundJam::m_defineFileSound", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void SoundJam::m_defineCastSound(int nargs) {
	SoundJamObject *me = static_cast<SoundJamObject *>(g_lingo->_currentMe.u.obj);

	/* Datum numberOfBeats = */ g_lingo->pop();
	CastMemberID castMemberNumber = g_lingo->pop().asMemberID();

	int soundID = 0;
	while (me->_soundMap.contains(soundID))
		soundID++;

	me->_soundMap[soundID] = castMemberNumber;

	g_lingo->push(soundID);
}

void SoundJam::m_undefineSound(int nargs) {
	SoundJamObject *me = static_cast<SoundJamObject *>(g_lingo->_currentMe.u.obj);
	int soundID = g_lingo->pop().asInt();

	if (soundID < 0) {
		g_lingo->push(0); // success
		return;
	}

	if (!me->_soundMap.contains(soundID)) {
		warning("SoundJam::m_undefineSound: Sound %d is not defined", soundID);
		g_lingo->push(-1); // error
		return;
	}

	me->_soundMap.erase(soundID);
	g_lingo->push(0); // success
}

void SoundJam::m_readSome(int nargs) {
	g_lingo->printSTUBWithArglist("SoundJam::m_readSome", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void SoundJam::m_startSound(int nargs) {
	g_lingo->printSTUBWithArglist("SoundJam::m_startSound", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void SoundJam::m_switchNew(int nargs) {
	SoundJamObject *me = static_cast<SoundJamObject *>(g_lingo->_currentMe.u.obj);
	int soundID = g_lingo->pop().asInt();

	if (!me->_soundMap.contains(soundID)) {
		warning("SoundJam::m_switchNew: Sound %d is not defined", soundID);
		g_lingo->push(-1); // error
		return;
	}

	DirectorSound *sound = g_director->getCurrentWindow()->getSoundManager();
	sound->setPuppetSound(me->_soundMap[soundID], kJamChannel);
	sound->playPuppetSound(kJamChannel);
	g_lingo->push(0); // success
}

void SoundJam::m_switchParallel(int nargs) {
	g_lingo->printSTUBWithArglist("SoundJam::m_switchParallel", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void SoundJam::m_hasSwitchHappened(int nargs) {
	g_lingo->printSTUBWithArglist("SoundJam::m_hasSwitchHappened", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void SoundJam::m_toggleMute(int nargs) {
	g_lingo->printSTUBWithArglist("SoundJam::m_toggleMute", nargs);
	g_lingo->dropStack(nargs);
}

void SoundJam::m_stop(int nargs) {
	DirectorSound *sound = g_director->getCurrentWindow()->getSoundManager();
	sound->setPuppetSound(SoundID(), kJamChannel);
	sound->playPuppetSound(kJamChannel);
}

} // End of namespace Director

