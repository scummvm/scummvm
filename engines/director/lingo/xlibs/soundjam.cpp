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
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/soundjam.h"

namespace Director {

static const char *xlibName = "SoundJam";

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

void SoundJam::initialize(int type) {
	SoundJamObject::initMethods(xlibMethods);
	if (type & kXObj) {
		if (!g_lingo->_globalvars.contains(xlibName)) {
			SoundJamObject *xobj = new SoundJamObject(kXObj);
			g_lingo->_globalvars[xlibName] = xobj;
		} else {
			warning("SoundJam already initialized");
		}
	}
}

SoundJamObject::SoundJamObject(ObjectType objType) : Object<SoundJamObject>("SoundJam") {
	_objType = objType;
}

void SoundJam::m_new(int nargs) {
	/* Datum numberOfChannels = */ g_lingo->pop();

	// Meet MediaBand seems to have fully working fallbacks to
	// standard Lingo builtins, which it uses when SoundJam
	// fails to initialize. So let's fail to initialize it...

	g_lingo->push(-20023); // Indicates this version of Director does not support SoundJam

	// If we discover that the standard builtins don't replicate
	// everything SoundJam is used for, then we'll have to properly
	// implement this.
}

void SoundJam::m_defineFileSound(int nargs) {
	g_lingo->printSTUBWithArglist("SoundJam::m_defineFileSound", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void SoundJam::m_defineCastSound(int nargs) {
	g_lingo->printSTUBWithArglist("SoundJam::m_defineCastSound", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void SoundJam::m_undefineSound(int nargs) {
	g_lingo->printSTUBWithArglist("SoundJam::m_undefineSound", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
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
	g_lingo->printSTUBWithArglist("SoundJam::m_switchNew", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
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
	g_lingo->printSTUBWithArglist("SoundJam::m_stop", nargs);
	g_lingo->dropStack(nargs);
}

} // End of namespace Director

