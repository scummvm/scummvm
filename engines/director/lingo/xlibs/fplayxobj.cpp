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
  * Spaceship Warlock Mac
  *
  *************************************/

#include "director/director.h"
#include "director/archive.h"
#include "director/sound.h"
#include "director/util.h"
#include "director/lingo/lingo.h"
#include "director/lingo/xlibs/fplayxobj.h"

#include "audio/audiostream.h"

namespace Director {

static BuiltinProto builtins[] = {
	{ "FPlay",			FPlayXObj::b_fplay,		-1,0, 200, CBLTIN },
	{ "SndInfo",		FPlayXObj::b_sndinfo,	-1,0, 200, FBLTIN },
	{ "SndList",		FPlayXObj::b_sndlist,	-1,0, 200, FBLTIN },
	{ "SndList",		FPlayXObj::b_sndlist,	-1,0, 200, FBLTIN },
	{ "Volume",			FPlayXObj::b_volume,	-1,0, 200, FBLTIN },
	{ "FileName",		FPlayXObj::b_filename,	-1,0, 200, FBLTIN },
	{ "InputLevel",		FPlayXObj::b_inputlevel,-1,0, 200, FBLTIN },
	{ "FSound",			FPlayXObj::b_fsound,	-1,0, 200, FBLTIN },
	{ 0, 0, 0, 0, 0, VOIDSYM }
};

void FPlayXObj::initialize(int type) {
	if (!g_lingo->_builtinCmds.contains("FPlay")) {
		g_lingo->initBuiltIns(builtins);
	} else {
		warning("FPlayXObj already initialized");
	}
}

void FPlayXObj::b_fplay(int nargs) {
	if (nargs == 0) {
		warning("FPlayXObj::b_fplay: requires at least one argument");
		return;
	}

	if (nargs > 2) {
		g_lingo->dropStack(nargs - 2);
		warning("FPlayXObj::b_fplay: unhandled %d arguments", nargs - 2);
	}

	Common::String mode;
	if (nargs == 2)
		mode = g_lingo->pop().asString();

	bool loop = false;
	if (!mode.empty()) {
		if (mode.equalsIgnoreCase("continuous"))
			loop = true;
		else
			warning("FPlayXObj::b_fplay: unhandled mode %s", mode.c_str());
	}

	Datum d = g_lingo->pop();
	DirectorSound *sound = g_director->getSoundManager();
	Common::String sndName = d.asString();

	if (sndName.equalsIgnoreCase("stop")) {
		sound->stopSound(1);
		return;
	}

	uint32 tag = MKTAG('s', 'n', 'd', ' ');
	uint id = 0xFFFF;
	Archive *archive = nullptr;

	// iterate opened ResFiles
	for (Common::HashMap<Common::String, Archive *, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo>::iterator it = g_director->_openResFiles.begin(); it != g_director->_openResFiles.end(); ++it) {
		id = it->_value->findResourceID(tag, sndName, true);
		if (id != 0xFFFF) {
			archive = it->_value;
			break;
		}
	}

	if (id == 0xFFFF) {
		warning("FPlayXObj::b_fplay: can not find sound %s", sndName.c_str());
		return;
	}

	Common::SeekableReadStreamEndian *sndData = archive->getResource(tag, id);
	if (sndData != nullptr) {
		SNDDecoder *ad = new SNDDecoder();
		ad->loadStream(*sndData);
		delete sndData;

		Audio::AudioStream *as;
		if (loop)
			as = ad->getLoopingAudioStream();
		else
			as = ad->getAudioStream();

		if (!as) {
			warning("FPlayXObj::b_fplay: failed to get audio stream");
			return;
		}
		sound->playStream(*as, 1);
		delete ad;
	}
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
	g_lingo->printSTUBWithArglist("b_fsound", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

} // End of namespace Director

