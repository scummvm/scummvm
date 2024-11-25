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
 * Spaceship Warlock (Mac)
 *
 *************************************/

/*
 * Version 2.0.3  Copyright © 1988-90 Farallon Computing, Inc.
 */

#include "director/director.h"
#include "director/sound.h"
#include "director/window.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/fplayxobj.h"

#include "audio/audiostream.h"

namespace Director {

const char *const FPlayXObj::xlibName = "FPlay";
const XlibFileDesc FPlayXObj::fileNames[] = {
	{ "FPlayXObj",	nullptr },
	{ "FPlay",		nullptr },
	{ nullptr,		nullptr },
};

static const BuiltinProto builtins[] = {
	{ "FPlay",			FPlayXObj::b_fplay,		   -1,0, 200, CBLTIN },	// XCMD
	{ "CopySnd",		FPlayXObj::b_copysnd,	   -1,0, 200, CBLTIN },	// XCMD
	{ "EraseSnd",		FPlayXObj::b_erasesnd,	   -1,0, 200, CBLTIN },	// XCMD
	{ "PasteSnd",		FPlayXObj::b_pastesnd,	   -1,0, 200, CBLTIN },	// XCMD
	{ "RenameSnd",		FPlayXObj::b_renamesnd,	   -1,0, 200, CBLTIN },	// XCMD
	{ "DuplicateSnd",	FPlayXObj::b_duplicatesnd, -1,0, 200, CBLTIN },	// XCMD
	{ "SndInfo",		FPlayXObj::b_sndinfo,	   -1,0, 200, FBLTIN },	// XFCN
	{ "SndList",		FPlayXObj::b_sndlist,	   -1,0, 200, FBLTIN },	// XFCN
	{ "Volume",			FPlayXObj::b_volume,	   -1,0, 200, FBLTIN },	// XFCN
	{ "FileName",		FPlayXObj::b_filename,	   -1,0, 200, FBLTIN },	// XFCN
	{ "InputLevel",		FPlayXObj::b_inputlevel,   -1,0, 200, FBLTIN },	// XFCN
	{ "FSound",			FPlayXObj::b_fsound,	    0,0, 200, FBLTIN },	// XFCN
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

void FPlayXObj::open(ObjectType type, const Common::Path &path) {
	g_lingo->initBuiltIns(builtins);
}

void FPlayXObj::close(ObjectType type) {
	g_lingo->cleanupBuiltIns(builtins);
}

void FPlayXObj::b_fplay(int nargs) {
	if (nargs == 0) {
		warning("FPlayXObj::b_fplay: requires at least one argument");
		return;
	}

	Common::Array<Common::String> arr(nargs);
	for (int i = nargs - 1; i >= 0; i--) {
		arr[i] = g_lingo->pop().asString();
	}

	DirectorSound *sound = g_director->getCurrentWindow()->getSoundManager();
	sound->playFPlaySound(arr);
}

XOBJSTUBV(FPlayXObj::b_copysnd)
XOBJSTUBV(FPlayXObj::b_erasesnd)
XOBJSTUBV(FPlayXObj::b_pastesnd)
XOBJSTUBV(FPlayXObj::b_renamesnd)
XOBJSTUBV(FPlayXObj::b_duplicatesnd)
XOBJSTUBV(FPlayXObj::b_sndinfo)
XOBJSTUBV(FPlayXObj::b_sndlist)
XOBJSTUBV(FPlayXObj::b_volume)
XOBJSTUBV(FPlayXObj::b_filename)
XOBJSTUBV(FPlayXObj::b_inputlevel)

void FPlayXObj::b_fsound(int nargs) {
	if (nargs != 0) {
		warning("FPlayXObj::b_fsound: unhandled arguments");
		g_lingo->dropStack(nargs);
	}

	DirectorSound *sound = g_director->getCurrentWindow()->getSoundManager();
	if (sound->isChannelActive(1)) {
		g_lingo->push(Datum(sound->getCurrentSound()));
	} else {
		g_lingo->push(Datum("done"));
	}
}

} // End of namespace Director
