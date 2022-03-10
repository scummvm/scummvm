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
 * Spaceship Warlock Mac
 *
 *************************************/

#include "director/director.h"
#include "director/sound.h"
#include "director/window.h"
#include "director/lingo/lingo.h"
#include "director/lingo/xlibs/fplayxobj.h"

#include "audio/audiostream.h"

namespace Director {

const char *FPlayXObj::xlibName = "FPlay";
const char *FPlayXObj::fileNames[] = {
	"FPlayXObj",
	"FPlay",
	nullptr
};

static BuiltinProto builtins[] = {
	{ "FPlay",			FPlayXObj::b_fplay,		-1,0, 200, CBLTIN },
	{ "SndInfo",		FPlayXObj::b_sndinfo,	-1,0, 200, FBLTIN },
	{ "SndList",		FPlayXObj::b_sndlist,	-1,0, 200, FBLTIN },
	{ "SndList",		FPlayXObj::b_sndlist,	-1,0, 200, FBLTIN },
	{ "Volume",			FPlayXObj::b_volume,	-1,0, 200, FBLTIN },
	{ "FileName",		FPlayXObj::b_filename,	-1,0, 200, FBLTIN },
	{ "InputLevel",		FPlayXObj::b_inputlevel,-1,0, 200, FBLTIN },
	{ "FSound",			FPlayXObj::b_fsound,	 0,0, 200, FBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

void FPlayXObj::open(int type) {
	g_lingo->initBuiltIns(builtins);
}

void FPlayXObj::close(int type) {
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

void FPlayXObj::b_sndinfo(int nargs) {
	g_lingo->printSTUBWithArglist("b_sndinfo", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void FPlayXObj::b_sndlist(int nargs) {
	g_lingo->printSTUBWithArglist("b_sndlist", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void FPlayXObj::b_volume(int nargs) {
	g_lingo->printSTUBWithArglist("b_volume", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void FPlayXObj::b_filename(int nargs) {
	g_lingo->printSTUBWithArglist("b_filename", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void FPlayXObj::b_inputlevel(int nargs) {
	g_lingo->printSTUBWithArglist("b_inputlevel", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

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

