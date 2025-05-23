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
#include "director/lingo/xlibs/voyagerxsound.h"

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
XOBJSTUB(VoyagerXSoundXObj::m_open, 0)
XOBJSTUB(VoyagerXSoundXObj::m_close, 0)
XOBJSTUB(VoyagerXSoundXObj::m_bufsize, 0)
XOBJSTUB(VoyagerXSoundXObj::m_exists, 0)
XOBJSTUB(VoyagerXSoundXObj::m_status, 0)
XOBJSTUB(VoyagerXSoundXObj::m_path, 0)
XOBJSTUB(VoyagerXSoundXObj::m_duration, "")
XOBJSTUB(VoyagerXSoundXObj::m_curtime, "")
XOBJSTUB(VoyagerXSoundXObj::m_playfile, 0)
XOBJSTUB(VoyagerXSoundXObj::m_loadfile, 0)
XOBJSTUB(VoyagerXSoundXObj::m_unloadfile, 0)
XOBJSTUB(VoyagerXSoundXObj::m_playsnd, 0)
XOBJSTUB(VoyagerXSoundXObj::m_extplayfile, 0)
XOBJSTUB(VoyagerXSoundXObj::m_stop, 0)
XOBJSTUB(VoyagerXSoundXObj::m_volume, 0)
XOBJSTUB(VoyagerXSoundXObj::m_leftrightvol, 0)
XOBJSTUB(VoyagerXSoundXObj::m_fade, 0)
XOBJSTUB(VoyagerXSoundXObj::m_frequency, 0)
XOBJSTUB(VoyagerXSoundXObj::m_pan, 0)
XOBJSTUB(VoyagerXSoundXObj::m_startrecord, 0)
XOBJSTUB(VoyagerXSoundXObj::m_stoprecord, 0)
XOBJSTUB(VoyagerXSoundXObj::m_recordpath, 0)

}
