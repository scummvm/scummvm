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
#include "director/movie.h"
#include "director/score.h"
#include "director/window.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/v/voyagerxsound.h"

/**************************************************
 *
 * USED IN:
 * Puppet Motel
 *
 **************************************************/

/*
-- Voyager Company XSound External Factory. 27October1995
--XSound
I      mNew                --Creates a new instance of XSound
X      mDispose            --Disposes of XSound instance
X      init                --Initializes the XSound library
III    open, numchan, monostereo   --Opens the specified sound channel
X      close               --Stops playback and closes all channels
XI     bufsize, kbytes		--Sets buffer size to kbytes
IS     exists, name        --Determines if AIFF file exists on disk
II     status, chan        --Determines if the specified channel is busy
XS     path, pathname      --Sets path used when opening sound files
SS     duration, name      --Returns duration in secs for specified file
SI     curtime, chan       --Returns current file location in seconds
V      playfile, chan, name, tstart, tend  --Plays specified AIFF file
IIS    loadfile, chan, name -- preloads the specified AIFF file
XI     unloadfile, chan    -- unloads a previously loaded file
V      playsnd, chan, name, tstart, tend   --Plays specified WAVE file
V      extplayfile, chan, name, lstart, lend   --Plays specified looped AIFF
XI     stop, chan          --Stops playback on specified channel
XII    volume, chan, vol   --Sets volume for specified channel
XIII   leftrightvol, chan, lvol, rvol      --Sets volume of left and right
V      fade, chan, endvol, duration, autostop  --Fades channel over duration
XII    frequency, chan, percent            --Transposes pitch by percent
XII    pan, chan, panvalue --Sets the panning of a channel
ISS    startrecord, name, duration         --Records new sound file
X      stoprecord          --Stops any recording in process
XS     recordpath, path    --Sets default record path
 */

namespace Director {

const char *VoyagerXSoundXObj::xlibName = "XSound";
const XlibFileDesc VoyagerXSoundXObj::fileNames[] = {
	{ "XSound",   "puppetmotel" },
	{ "XSound32",   "puppetmotel" },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				VoyagerXSoundXObj::m_new,		 0, 0,	400 },
	{ "dispose",				VoyagerXSoundXObj::m_dispose,		 0, 0,	400 },
	{ "init",				VoyagerXSoundXObj::m_init,		 0, 0,	400 },
	{ "open",				VoyagerXSoundXObj::m_open,		 2, 2,	400 },
	{ "close",				VoyagerXSoundXObj::m_close,		 0, 0,	400 },
	{ "bufsize",				VoyagerXSoundXObj::m_bufsize,		 1, 1,	400 },
	{ "exists",				VoyagerXSoundXObj::m_exists,		 1, 1,	400 },
	{ "status",				VoyagerXSoundXObj::m_status,		 1, 1,	400 },
	{ "path",				VoyagerXSoundXObj::m_path,		 1, 1,	400 },
	{ "duration",				VoyagerXSoundXObj::m_duration,		 1, 1,	400 },
	{ "curtime",				VoyagerXSoundXObj::m_curtime,		 1, 1,	400 },
	{ "playfile",				VoyagerXSoundXObj::m_playfile,		 0, 0,	400 },
	{ "loadfile",				VoyagerXSoundXObj::m_loadfile,		 2, 2,	400 },
	{ "unloadfile",				VoyagerXSoundXObj::m_unloadfile,		 1, 1,	400 },
	{ "playsnd",				VoyagerXSoundXObj::m_playsnd,		 0, 0,	400 },
	{ "extplayfile",				VoyagerXSoundXObj::m_extplayfile,		 0, 0,	400 },
	{ "stop",				VoyagerXSoundXObj::m_stop,		 1, 1,	400 },
	{ "volume",				VoyagerXSoundXObj::m_volume,		 2, 2,	400 },
	{ "leftrightvol",				VoyagerXSoundXObj::m_leftrightvol,		 3, 3,	400 },
	{ "fade",				VoyagerXSoundXObj::m_fade,		 0, 0,	400 },
	{ "frequency",				VoyagerXSoundXObj::m_frequency,		 2, 2,	400 },
	{ "pan",				VoyagerXSoundXObj::m_pan,		 2, 2,	400 },
	{ "startrecord",				VoyagerXSoundXObj::m_startrecord,		 2, 2,	400 },
	{ "stoprecord",				VoyagerXSoundXObj::m_stoprecord,		 0, 0,	400 },
	{ "recordpath",				VoyagerXSoundXObj::m_recordpath,		 1, 1,	400 },
	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {

	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

VoyagerXSoundXObject::VoyagerXSoundXObject(ObjectType ObjectType) :Object<VoyagerXSoundXObject>("XSound") {
	_objType = ObjectType;
	_soundManager = g_director->getCurrentWindow()->getSoundManager();

}

VoyagerXSoundXObject::~VoyagerXSoundXObject() {
	close();
}

int VoyagerXSoundXObject::open(int numChan, int monoStereo) {
	if (!_channels.contains(numChan)) {
		_channels[numChan] = new VoyagerChannel();
		_channels[numChan]->channelID = numChan + 1000;
	}
	return 1;
}

void VoyagerXSoundXObject::close() {
	for (auto &it : _channels)
		delete it._value;
	_channels.clear();
}

int VoyagerXSoundXObject::status(int chan) {
	if (_channels.contains(chan)) {
		return _soundManager->isChannelActive(_channels[chan]->channelID) ? 1 : 0;
	}
	return 0;
}

int VoyagerXSoundXObject::playfile(int chan, Common::String &path, int tstart, int tend) {
	if (!_channels.contains(chan)) {
		open(chan, 2);
	}
	_soundManager->playFile(path, _channels[chan]->channelID);
	return 1;
}

int VoyagerXSoundXObject::fade(int chan, int endvol, int duration, bool autostop) {
	if (!_channels.contains(chan)) {
		return 0;
	}
	int channelID = _channels[chan]->channelID;
	_soundManager->registerFade(channelID, _soundManager->getChannelVolume(channelID), endvol, duration*60, autostop);
	Window *window = g_director->getCurrentWindow();
	Score *score = window->getCurrentMovie()->getScore();
	score->_activeFade = true;
	return 1;
}

void VoyagerXSoundXObject::stop(int chan) {
	if (!_channels.contains(chan)) {
		return;
	}
	int channelID = _channels[chan]->channelID;
	_soundManager->stopSound(channelID);
}

void VoyagerXSoundXObject::volume(int chan, int vol) {
	if (!_channels.contains(chan)) {
		return;
	}
	int channelID = _channels[chan]->channelID;
	_soundManager->setChannelVolume(channelID, vol);
}

void VoyagerXSoundXObject::leftrightvol(int chan, uint8 lvol, uint8 rvol) {
	if (!_channels.contains(chan)) {
		return;
	}
	int channelID = _channels[chan]->channelID;
	_soundManager->setChannelFaderL(channelID, lvol);
	_soundManager->setChannelFaderR(channelID, lvol);
}

void VoyagerXSoundXObject::frequency(int chan, int percent) {
	if (!_channels.contains(chan)) {
		return;
	}
	int channelID = _channels[chan]->channelID;
	_soundManager->setChannelPitchShift(channelID, percent);
}

void VoyagerXSoundXObject::pan(int chan, int percent) {
	if (!_channels.contains(chan)) {
		return;
	}
	percent = MAX(MIN(100, percent), -100);
	int channelID = _channels[chan]->channelID;
	_soundManager->setChannelBalance(channelID, (int8)percent);
}


void VoyagerXSoundXObj::open(ObjectType type, const Common::Path &path) {
    VoyagerXSoundXObject::initMethods(xlibMethods);
    VoyagerXSoundXObject *xobj = new VoyagerXSoundXObject(type);
    if (type == kXtraObj)
        g_lingo->_openXtras.push_back(xlibName);
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void VoyagerXSoundXObj::close(ObjectType type) {
    VoyagerXSoundXObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();
}

void VoyagerXSoundXObj::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("VoyagerXSoundXObj::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}


// For some reason the game code calls all of these with ARGC, so always return something
XOBJSTUBNR(VoyagerXSoundXObj::m_dispose)
XOBJSTUBNR(VoyagerXSoundXObj::m_init)

void VoyagerXSoundXObj::m_open(int nargs) {
	g_lingo->printSTUBWithArglist("VoyagerXSoundXObj::m_open", nargs);
	ARGNUMCHECK(2);
	VoyagerXSoundXObject *me = static_cast<VoyagerXSoundXObject *>(g_lingo->_state->me.u.obj);
	Datum monoStereo = g_lingo->pop();
	Datum numChan = g_lingo->pop();
	int result = me->open(numChan.asInt(), monoStereo.asInt());
	g_lingo->push(result);
}

void VoyagerXSoundXObj::m_close(int nargs) {
	g_lingo->printSTUBWithArglist("VoyagerXSoundXObj::m_close", nargs);
	ARGNUMCHECK(0);
	VoyagerXSoundXObject *me = static_cast<VoyagerXSoundXObject *>(g_lingo->_state->me.u.obj);
	me->close();
	g_lingo->push(0);
}

XOBJSTUB(VoyagerXSoundXObj::m_bufsize, 0)
XOBJSTUB(VoyagerXSoundXObj::m_exists, 0)

void VoyagerXSoundXObj::m_status(int nargs) {
	g_lingo->printSTUBWithArglist("VoyagerXSoundXObj::m_status", nargs);
	VoyagerXSoundXObject *me = static_cast<VoyagerXSoundXObject *>(g_lingo->_state->me.u.obj);
	ARGNUMCHECK(1);
	Datum chan = g_lingo->pop();
	g_lingo->push(me->status(chan.asInt()));
}

XOBJSTUB(VoyagerXSoundXObj::m_path, 0)
XOBJSTUB(VoyagerXSoundXObj::m_duration, "")
XOBJSTUB(VoyagerXSoundXObj::m_curtime, "")

void VoyagerXSoundXObj::m_playfile(int nargs) {
	g_lingo->printSTUBWithArglist("VoyagerXSoundXObj::m_playfile", nargs);
	VoyagerXSoundXObject *me = static_cast<VoyagerXSoundXObject *>(g_lingo->_state->me.u.obj);
	if (nargs < 2) {
		warning("VoyagerXSoundXObj::m_playfile: expected at least 2 args");
		g_lingo->dropStack(nargs);
		g_lingo->push(0);
		return;
	} else if (nargs > 4) {
		g_lingo->dropStack(nargs - 4);
		nargs = 4;
	}
	Datum tend(-1);
	if (nargs == 4) {
		tend = g_lingo->pop();
		nargs--;
	}
	Datum tstart(-1);
	if (nargs == 3) {
		tstart = g_lingo->pop();
		nargs--;
	}
	Common::String path = g_lingo->pop().asString();
	Datum chan = g_lingo->pop();
	int result = me->playfile(chan.asInt(), path, tstart.asInt(), tend.asInt());
	g_lingo->push(result);
}

XOBJSTUB(VoyagerXSoundXObj::m_loadfile, 0)
XOBJSTUB(VoyagerXSoundXObj::m_unloadfile, 0)

void VoyagerXSoundXObj::m_playsnd(int nargs) {
	m_playfile(nargs);
}

XOBJSTUB(VoyagerXSoundXObj::m_extplayfile, 0)

void VoyagerXSoundXObj::m_stop(int nargs) {
	g_lingo->printSTUBWithArglist("VoyagerXSoundXObj::m_stop", nargs);
	VoyagerXSoundXObject *me = static_cast<VoyagerXSoundXObject *>(g_lingo->_state->me.u.obj);
	ARGNUMCHECK(1);
	int chan = g_lingo->pop().asInt();
	me->stop(chan);
	g_lingo->push(Datum(1));
}

void VoyagerXSoundXObj::m_volume(int nargs) {
	g_lingo->printSTUBWithArglist("VoyagerXSoundXObj::m_volume", nargs);
	VoyagerXSoundXObject *me = static_cast<VoyagerXSoundXObject *>(g_lingo->_state->me.u.obj);
	ARGNUMCHECK(2);
	int vol = g_lingo->pop().asInt();
	int chan = g_lingo->pop().asInt();
	me->volume(chan, vol);
	g_lingo->push(Datum(1));
}

void VoyagerXSoundXObj::m_leftrightvol(int nargs) {
	g_lingo->printSTUBWithArglist("VoyagerXSoundXObj::m_pan", nargs);
	VoyagerXSoundXObject *me = static_cast<VoyagerXSoundXObject *>(g_lingo->_state->me.u.obj);
	ARGNUMCHECK(3);
	int rvol = g_lingo->pop().asInt();
	int lvol = g_lingo->pop().asInt();
	int chan = g_lingo->pop().asInt();
	me->leftrightvol(chan, (uint8)lvol, (uint8)rvol);
	g_lingo->push(Datum(1));
}

void VoyagerXSoundXObj::m_fade(int nargs) {
	g_lingo->printSTUBWithArglist("VoyagerXSoundXObj::m_fade", nargs);
	VoyagerXSoundXObject *me = static_cast<VoyagerXSoundXObject *>(g_lingo->_state->me.u.obj);
	if (nargs < 2) {
		warning("VoyagerXSoundXObj::m_fade: expected at least 2 args");
		g_lingo->dropStack(nargs);
		g_lingo->push(Datum());
		return;
	}
	if (nargs > 4) {
		warning("VoyagerXSoundXObj: dropping %d extra args", nargs - 4);
		g_lingo->dropStack(nargs - 4);
		nargs = 4;
	}
	bool autoStop = false;
	int duration = 0;
	if (nargs == 4) {
		autoStop = (bool)g_lingo->pop().asInt();
		nargs--;
	}
	if (nargs == 3) {
		duration = g_lingo->pop().asInt();
		nargs--;
	}
	int endVol = g_lingo->pop().asInt();
	int chan = g_lingo->pop().asInt();

	g_lingo->push(Datum(me->fade(chan, endVol, duration, autoStop)));
}

void VoyagerXSoundXObj::m_frequency(int nargs) {
	g_lingo->printSTUBWithArglist("VoyagerXSoundXObj::m_frequency", nargs);
	VoyagerXSoundXObject *me = static_cast<VoyagerXSoundXObject *>(g_lingo->_state->me.u.obj);
	ARGNUMCHECK(2);
	int percent = g_lingo->pop().asInt();
	int chan = g_lingo->pop().asInt();
	me->frequency(chan, percent);
	g_lingo->push(Datum(1));
}


void VoyagerXSoundXObj::m_pan(int nargs) {
	g_lingo->printSTUBWithArglist("VoyagerXSoundXObj::m_pan", nargs);
	VoyagerXSoundXObject *me = static_cast<VoyagerXSoundXObject *>(g_lingo->_state->me.u.obj);
	ARGNUMCHECK(2);
	int percent = g_lingo->pop().asInt();
	int chan = g_lingo->pop().asInt();
	me->pan(chan, percent);
	g_lingo->push(Datum(1));
}


XOBJSTUB(VoyagerXSoundXObj::m_startrecord, 0)
XOBJSTUB(VoyagerXSoundXObj::m_stoprecord, 0)
XOBJSTUB(VoyagerXSoundXObj::m_recordpath, 0)

}
