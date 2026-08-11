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
#include "director/lingo/xlibs/p/playsoundmoviexobj.h"

/**************************************************
 *
 * USED IN:
 * wttf
 *
 **************************************************/

/*
--		SaveNRestore XObj v 1.0d2 (c) 1995 Samizdat Productions. All Rights Reserved.
--		written by Christopher P. Kelly
I		mNew
IS		mMovieOpen fullPathName
I		mMovieIdle
II		mMovieSetVolume newVolume
I		mMovieStop
 */

namespace Director {

const char *PlaySoundMovieXObj::xlibName = "PlaySoundMovieXObj";
const XlibFileDesc PlaySoundMovieXObj::fileNames[] = {
	{ "PlaySoundMovieXObj",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				PlaySoundMovieXObj::m_new,		 0, 0,	400 },
	{ "movieOpen",				PlaySoundMovieXObj::m_movieOpen,		 1, 1,	400 },
	{ "movieIdle",				PlaySoundMovieXObj::m_movieIdle,		 0, 0,	400 },
	{ "movieSetVolume",				PlaySoundMovieXObj::m_movieSetVolume,		 1, 1,	400 },
	{ "movieStop",				PlaySoundMovieXObj::m_movieStop,		 0, 0,	400 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

PlaySoundMovieXObject::PlaySoundMovieXObject(ObjectType ObjectType) :Object<PlaySoundMovieXObject>("PlaySoundMovieXObj") {
	_objType = ObjectType;
}

void PlaySoundMovieXObj::open(ObjectType type, const Common::Path &path) {
    PlaySoundMovieXObject::initMethods(xlibMethods);
    PlaySoundMovieXObject *xobj = new PlaySoundMovieXObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void PlaySoundMovieXObj::close(ObjectType type) {
    PlaySoundMovieXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void PlaySoundMovieXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("PlaySoundMovieXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(PlaySoundMovieXObj::m_movieOpen, 0)
XOBJSTUB(PlaySoundMovieXObj::m_movieIdle, 0)
XOBJSTUB(PlaySoundMovieXObj::m_movieSetVolume, 0)
XOBJSTUB(PlaySoundMovieXObj::m_movieStop, 0)

}
