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
#include "director/lingo/xtras/b/bkmixer.h"

/**************************************************
 *
 * USED IN:
 * Nature Virtual Serengeti
 *
 **************************************************/

/*
-- xtra bkMixer
new object me -- (c)1998 Burak KALAYCI (burakk@usa.net)
* bkMixerVersion 
* bkGetMasterVolume 
* bkGetCDVolume 
* bkGetWaveVolume 
* bkGetMidiVolume 
* bkGetLineInVolume 
* bkGetMicVolume 
* bkGetSpeakerVolume 
* bkSetMasterVolume integer
* bkSetCDVolume integer
* bkSetWaveVolume integer
* bkSetMidiVolume integer
* bkSetLineInVolume integer
* bkSetMicVolume integer
* bkSetSpeakerVolume integer
* bkIsMasterMute 
* bkIsCDMute 
* bkIsWaveMute 
* bkIsMidiMute 
* bkIsLineInMute 
* bkIsMicMute 
* bkIsSpeakerMute 
* bkMuteMaster integer
* bkMuteCD integer
* bkMuteWave integer
* bkMuteMidi integer
* bkMuteLineIn integer
* bkMuteMic integer
* bkMuteSpeaker integer
* bkDetectWaveOut
* bkGetWaveOutName
* bkDetectMixer
* bkGetMixerName
* bkGetMasterVolumeEx integer
* bkGetCDVolumeEx integer
* bkGetWaveVolumeEx integer
* bkGetMidiVolumeEx integer
* bkGetLineInVolumeEx integer
* bkGetMicVolumeEx integer
* bkGetSpeakerVolumeEx integer
* bkSetMasterVolumeEx integer,integer
* bkSetCDVolumeEx integer,integer
* bkSetWaveVolumeEx integer,integer
* bkSetMidiVolumeEx integer,integer
* bkSetLineInVolumeEx integer,integer
* bkSetMicVolumeEx integer,integer
* bkSetSpeakerVolumeEx integer,integer
"
 */

namespace Director {

const char *BkmixerXtra::xlibName = "Bkmixer";
const XlibFileDesc BkmixerXtra::fileNames[] = {
	{ "bkmixer",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				BkmixerXtra::m_new,		 0, 0,	500 },
	{ """,				BkmixerXtra::m_",		 -1, -1,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "bkMixerVersion", BkmixerXtra::m_bkMixerVersion, 0, 0, 500, HBLTIN },
	{ "bkGetMasterVolume", BkmixerXtra::m_bkGetMasterVolume, 0, 0, 500, HBLTIN },
	{ "bkGetCDVolume", BkmixerXtra::m_bkGetCDVolume, 0, 0, 500, HBLTIN },
	{ "bkGetWaveVolume", BkmixerXtra::m_bkGetWaveVolume, 0, 0, 500, HBLTIN },
	{ "bkGetMidiVolume", BkmixerXtra::m_bkGetMidiVolume, 0, 0, 500, HBLTIN },
	{ "bkGetLineInVolume", BkmixerXtra::m_bkGetLineInVolume, 0, 0, 500, HBLTIN },
	{ "bkGetMicVolume", BkmixerXtra::m_bkGetMicVolume, 0, 0, 500, HBLTIN },
	{ "bkGetSpeakerVolume", BkmixerXtra::m_bkGetSpeakerVolume, 0, 0, 500, HBLTIN },
	{ "bkSetMasterVolume", BkmixerXtra::m_bkSetMasterVolume, 1, 1, 500, HBLTIN },
	{ "bkSetCDVolume", BkmixerXtra::m_bkSetCDVolume, 1, 1, 500, HBLTIN },
	{ "bkSetWaveVolume", BkmixerXtra::m_bkSetWaveVolume, 1, 1, 500, HBLTIN },
	{ "bkSetMidiVolume", BkmixerXtra::m_bkSetMidiVolume, 1, 1, 500, HBLTIN },
	{ "bkSetLineInVolume", BkmixerXtra::m_bkSetLineInVolume, 1, 1, 500, HBLTIN },
	{ "bkSetMicVolume", BkmixerXtra::m_bkSetMicVolume, 1, 1, 500, HBLTIN },
	{ "bkSetSpeakerVolume", BkmixerXtra::m_bkSetSpeakerVolume, 1, 1, 500, HBLTIN },
	{ "bkIsMasterMute", BkmixerXtra::m_bkIsMasterMute, 0, 0, 500, HBLTIN },
	{ "bkIsCDMute", BkmixerXtra::m_bkIsCDMute, 0, 0, 500, HBLTIN },
	{ "bkIsWaveMute", BkmixerXtra::m_bkIsWaveMute, 0, 0, 500, HBLTIN },
	{ "bkIsMidiMute", BkmixerXtra::m_bkIsMidiMute, 0, 0, 500, HBLTIN },
	{ "bkIsLineInMute", BkmixerXtra::m_bkIsLineInMute, 0, 0, 500, HBLTIN },
	{ "bkIsMicMute", BkmixerXtra::m_bkIsMicMute, 0, 0, 500, HBLTIN },
	{ "bkIsSpeakerMute", BkmixerXtra::m_bkIsSpeakerMute, 0, 0, 500, HBLTIN },
	{ "bkMuteMaster", BkmixerXtra::m_bkMuteMaster, 1, 1, 500, HBLTIN },
	{ "bkMuteCD", BkmixerXtra::m_bkMuteCD, 1, 1, 500, HBLTIN },
	{ "bkMuteWave", BkmixerXtra::m_bkMuteWave, 1, 1, 500, HBLTIN },
	{ "bkMuteMidi", BkmixerXtra::m_bkMuteMidi, 1, 1, 500, HBLTIN },
	{ "bkMuteLineIn", BkmixerXtra::m_bkMuteLineIn, 1, 1, 500, HBLTIN },
	{ "bkMuteMic", BkmixerXtra::m_bkMuteMic, 1, 1, 500, HBLTIN },
	{ "bkMuteSpeaker", BkmixerXtra::m_bkMuteSpeaker, 1, 1, 500, HBLTIN },
	{ "bkDetectWaveOut", BkmixerXtra::m_bkDetectWaveOut, 0, 0, 500, HBLTIN },
	{ "bkGetWaveOutName", BkmixerXtra::m_bkGetWaveOutName, 0, 0, 500, HBLTIN },
	{ "bkDetectMixer", BkmixerXtra::m_bkDetectMixer, 0, 0, 500, HBLTIN },
	{ "bkGetMixerName", BkmixerXtra::m_bkGetMixerName, 0, 0, 500, HBLTIN },
	{ "bkGetMasterVolumeEx", BkmixerXtra::m_bkGetMasterVolumeEx, 1, 1, 500, HBLTIN },
	{ "bkGetCDVolumeEx", BkmixerXtra::m_bkGetCDVolumeEx, 1, 1, 500, HBLTIN },
	{ "bkGetWaveVolumeEx", BkmixerXtra::m_bkGetWaveVolumeEx, 1, 1, 500, HBLTIN },
	{ "bkGetMidiVolumeEx", BkmixerXtra::m_bkGetMidiVolumeEx, 1, 1, 500, HBLTIN },
	{ "bkGetLineInVolumeEx", BkmixerXtra::m_bkGetLineInVolumeEx, 1, 1, 500, HBLTIN },
	{ "bkGetMicVolumeEx", BkmixerXtra::m_bkGetMicVolumeEx, 1, 1, 500, HBLTIN },
	{ "bkGetSpeakerVolumeEx", BkmixerXtra::m_bkGetSpeakerVolumeEx, 1, 1, 500, HBLTIN },
	{ "bkSetMasterVolumeEx", BkmixerXtra::m_bkSetMasterVolumeEx, 2, 2, 500, HBLTIN },
	{ "bkSetCDVolumeEx", BkmixerXtra::m_bkSetCDVolumeEx, 2, 2, 500, HBLTIN },
	{ "bkSetWaveVolumeEx", BkmixerXtra::m_bkSetWaveVolumeEx, 2, 2, 500, HBLTIN },
	{ "bkSetMidiVolumeEx", BkmixerXtra::m_bkSetMidiVolumeEx, 2, 2, 500, HBLTIN },
	{ "bkSetLineInVolumeEx", BkmixerXtra::m_bkSetLineInVolumeEx, 2, 2, 500, HBLTIN },
	{ "bkSetMicVolumeEx", BkmixerXtra::m_bkSetMicVolumeEx, 2, 2, 500, HBLTIN },
	{ "bkSetSpeakerVolumeEx", BkmixerXtra::m_bkSetSpeakerVolumeEx, 2, 2, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

BkmixerXtraObject::BkmixerXtraObject(ObjectType ObjectType) :Object<BkmixerXtraObject>("Bkmixer") {
	_objType = ObjectType;
}

bool BkmixerXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum BkmixerXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(BkmixerXtra::xlibName);
	warning("BkmixerXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void BkmixerXtra::open(ObjectType type, const Common::Path &path) {
    BkmixerXtraObject::initMethods(xlibMethods);
    BkmixerXtraObject *xobj = new BkmixerXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void BkmixerXtra::close(ObjectType type) {
    BkmixerXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void BkmixerXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("BkmixerXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(BkmixerXtra::m_bkMixerVersion, 0)
XOBJSTUB(BkmixerXtra::m_bkGetMasterVolume, 0)
XOBJSTUB(BkmixerXtra::m_bkGetCDVolume, 0)
XOBJSTUB(BkmixerXtra::m_bkGetWaveVolume, 0)
XOBJSTUB(BkmixerXtra::m_bkGetMidiVolume, 0)
XOBJSTUB(BkmixerXtra::m_bkGetLineInVolume, 0)
XOBJSTUB(BkmixerXtra::m_bkGetMicVolume, 0)
XOBJSTUB(BkmixerXtra::m_bkGetSpeakerVolume, 0)
XOBJSTUB(BkmixerXtra::m_bkSetMasterVolume, 0)
XOBJSTUB(BkmixerXtra::m_bkSetCDVolume, 0)
XOBJSTUB(BkmixerXtra::m_bkSetWaveVolume, 0)
XOBJSTUB(BkmixerXtra::m_bkSetMidiVolume, 0)
XOBJSTUB(BkmixerXtra::m_bkSetLineInVolume, 0)
XOBJSTUB(BkmixerXtra::m_bkSetMicVolume, 0)
XOBJSTUB(BkmixerXtra::m_bkSetSpeakerVolume, 0)
XOBJSTUB(BkmixerXtra::m_bkIsMasterMute, 0)
XOBJSTUB(BkmixerXtra::m_bkIsCDMute, 0)
XOBJSTUB(BkmixerXtra::m_bkIsWaveMute, 0)
XOBJSTUB(BkmixerXtra::m_bkIsMidiMute, 0)
XOBJSTUB(BkmixerXtra::m_bkIsLineInMute, 0)
XOBJSTUB(BkmixerXtra::m_bkIsMicMute, 0)
XOBJSTUB(BkmixerXtra::m_bkIsSpeakerMute, 0)
XOBJSTUB(BkmixerXtra::m_bkMuteMaster, 0)
XOBJSTUB(BkmixerXtra::m_bkMuteCD, 0)
XOBJSTUB(BkmixerXtra::m_bkMuteWave, 0)
XOBJSTUB(BkmixerXtra::m_bkMuteMidi, 0)
XOBJSTUB(BkmixerXtra::m_bkMuteLineIn, 0)
XOBJSTUB(BkmixerXtra::m_bkMuteMic, 0)
XOBJSTUB(BkmixerXtra::m_bkMuteSpeaker, 0)
XOBJSTUB(BkmixerXtra::m_bkDetectWaveOut, 0)
XOBJSTUB(BkmixerXtra::m_bkGetWaveOutName, 0)
XOBJSTUB(BkmixerXtra::m_bkDetectMixer, 0)
XOBJSTUB(BkmixerXtra::m_bkGetMixerName, 0)
XOBJSTUB(BkmixerXtra::m_bkGetMasterVolumeEx, 0)
XOBJSTUB(BkmixerXtra::m_bkGetCDVolumeEx, 0)
XOBJSTUB(BkmixerXtra::m_bkGetWaveVolumeEx, 0)
XOBJSTUB(BkmixerXtra::m_bkGetMidiVolumeEx, 0)
XOBJSTUB(BkmixerXtra::m_bkGetLineInVolumeEx, 0)
XOBJSTUB(BkmixerXtra::m_bkGetMicVolumeEx, 0)
XOBJSTUB(BkmixerXtra::m_bkGetSpeakerVolumeEx, 0)
XOBJSTUB(BkmixerXtra::m_bkSetMasterVolumeEx, 0)
XOBJSTUB(BkmixerXtra::m_bkSetCDVolumeEx, 0)
XOBJSTUB(BkmixerXtra::m_bkSetWaveVolumeEx, 0)
XOBJSTUB(BkmixerXtra::m_bkSetMidiVolumeEx, 0)
XOBJSTUB(BkmixerXtra::m_bkSetLineInVolumeEx, 0)
XOBJSTUB(BkmixerXtra::m_bkSetMicVolumeEx, 0)
XOBJSTUB(BkmixerXtra::m_bkSetSpeakerVolumeEx, 0)
XOBJSTUB(BkmixerXtra::m_", 0)

}
