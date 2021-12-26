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

/*
 * Macromedia provides several XObjects for controlling some common devices that
 * play video and audio source material:
 *  - The Sony videodisc and Pioneer videodisc XObjects control many models
 *    manufactured by two popular brands of videodisc players.
 *  - The VISCA XObject controls a wide variety ofvideotape recorders that use
 *    the VISCA protocol.
 *  - The AppleCD XObject controls the CD audio capabilities ofthe AppleCD SC.
 *
 *  Reference: Director 4 Using Lingo, pages 325-339
 */

#include "director/director.h"
#include "director/lingo/lingo.h"
#include "director/lingo/lingo-object.h"
#include "director/lingo/xlibs/orthoplayxobj.h"


namespace Director {

const char *OrthoPlayXObj::xlibName = "OrthoPlayXObj";
const char *OrthoPlayXObj::fileNames[] = {
	"OrthoPlay XObj",
	nullptr
};

static MethodProto xlibMethods[] = {
	{ "new",				OrthoPlayXObj::m_new,				 0, 0,	200 },	// D2
	{ "SetSerialPort",		OrthoPlayXObj::m_setSerialPort,		 1, 1,	200 },	// D2
	{ "SetInitViaDlog",		OrthoPlayXObj::m_setInitViaDlog,	 1, 1,	200 },	// D2
	{ "GetInitInfo",		OrthoPlayXObj::m_getInitInfo,		 0, 0,  200 },	// D2
	{ "SetInitInfo",		OrthoPlayXObj::m_setInitInfo,		 1, 1,  200 },	// D2
	{ "GetMaxDevices",		OrthoPlayXObj::m_getMaxDevices,		 0, 0,  200 },	// D2
	{ "GetDeviceTitle",		OrthoPlayXObj::m_getDeviceTitle,	 1, 1,  200 },	// D2
	{ "SetDevice",			OrthoPlayXObj::m_setDevice,			 1, 1,  200 },	// D2
	{ "SelectDevice",		OrthoPlayXObj::m_selectDevice,		 1, 1,  200 },	// D2
	{ "GetDevice",			OrthoPlayXObj::m_getDevice,			 0, 0,  200 },	// D2
	{ "Service",			OrthoPlayXObj::m_service,			 0, 0,  200 },	// D2
	{ "GetValue",			OrthoPlayXObj::m_getValue,			 0, 0,  200 },	// D2
	{ "Cancel",				OrthoPlayXObj::m_cancel,			 0, 0,  200 },	// D2
	{ "Explain",			OrthoPlayXObj::m_explain,			 0, 0,  200 },	// D2
	{ "Idle",				OrthoPlayXObj::m_idle,				 0, 0,  200 },	// D2
	{ "ReadStatus",			OrthoPlayXObj::m_readStatus,		 0, 0,  200 },	// D2
	{ "ReadPos",			OrthoPlayXObj::m_readPos,			 0, 0,  200 },	// D2
	{ "SearchTo",			OrthoPlayXObj::m_searchTo,			 1, 1,  200 },	// D2
	{ "Play",				OrthoPlayXObj::m_play,				 0, 0,  200 },	// D2
	{ "Still",				OrthoPlayXObj::m_still,				 0, 0,  200 },	// D2
	{ "Stop",				OrthoPlayXObj::m_stop,				 0, 0,  200 },	// D2
	{ "ScanForward",		OrthoPlayXObj::m_scanForward,		 0, 0,  200 },	// D2
	{ "ScanReverse",		OrthoPlayXObj::m_scanReverse,		 0, 0,  200 },	// D2
	{ "PlayReverse",		OrthoPlayXObj::m_playReverse,		 0, 0,  200 },	// D2
	{ "FastForward",		OrthoPlayXObj::m_fastForward,		 0, 0,  200 },	// D2
	{ "Rewind",				OrthoPlayXObj::m_rewind,			 0, 0,  200 },	// D2
	{ "StepForward",		OrthoPlayXObj::m_stepForward,		 0, 0,  200 },	// D2
	{ "StepReverse",		OrthoPlayXObj::m_stepReverse,		 0, 0,  200 },	// D2
	{ "Shuttle",			OrthoPlayXObj::m_shuttle,			 1, 1,  200 },	// D2
	{ "Record",				OrthoPlayXObj::m_record,			 0, 0,  200 },	// D2
	{ "Eject",				OrthoPlayXObj::m_eject,				 0, 0,  200 },	// D2
	{ "PrepareMedium",		OrthoPlayXObj::m_prepareMedium,		 0, 0,  200 },	// D2
	{ "GetFirstTrack",		OrthoPlayXObj::m_getFirstTrack,		 0, 0,  200 },	// D2
	{ "GetLastTrack",		OrthoPlayXObj::m_getLastTrack,		 0, 0,  200 },	// D2
	{ "GetFirstFrame",		OrthoPlayXObj::m_getFirstFrame,		 1, 1,  200 },	// D2
	{ "GetLastFrame",		OrthoPlayXObj::m_getLastFrame,		 1, 1,  200 },	// D2
	{ "GetTrack",			OrthoPlayXObj::m_getTrack,			 0, 0,  200 },	// D2
	{ "ResetCounter",		OrthoPlayXObj::m_resetCounter,		 0, 0,  200 },	// D2
	{ "AudioEnable",		OrthoPlayXObj::m_audioEnable,		 2, 2,  200 },	// D2
	{ "AudioMute",			OrthoPlayXObj::m_audioMute,			 2, 2,  200 },	// D2
	{ "VideoEnable",		OrthoPlayXObj::m_videoEnable,		 2, 2,  200 },	// D2
	{ "ShowFrame",			OrthoPlayXObj::m_showFrame,			 1, 1,  200 },	// D2
	{ "GetFrameResolution",	OrthoPlayXObj::m_getFrameResolution, 0, 0,  200 },	// D2
	{ "SetFrameResolution",	OrthoPlayXObj::m_setFrameResolution, 1, 1,  200 },	// D2
	{ "HasDropFrames",		OrthoPlayXObj::m_hasDropFrames,		 0, 0,  200 },	// D2
	{ "SendRaw",			OrthoPlayXObj::m_sendRaw,			 1, 1,  200 },	// D2
	{ "ReadRaw",			OrthoPlayXObj::m_readRaw,			 0, 0,  200 },	// D2
	{ "SetInPoint",			OrthoPlayXObj::m_setInPoint,		 1, 1,  200 },	// D2
	{ "SetOutPoint",		OrthoPlayXObj::m_setOutPoint,		 1, 1,  200 },	// D2
	{ "SetDuration",		OrthoPlayXObj::m_setDuration,		 1, 1,  200 },	// D2
	{ "GetMinDuration",		OrthoPlayXObj::m_getMinDuration,	 0, 0,  200 },	// D2
	{ "SetPreroll",			OrthoPlayXObj::m_setPreroll,		 1, 1,  200 },	// D2
	{ "GetPreroll",			OrthoPlayXObj::m_getPreroll,		 0, 0,  200 },	// D2
	{ "SetPostroll",		OrthoPlayXObj::m_setPostroll,		 1, 1,  200 },	// D2
	{ "GetPostroll",		OrthoPlayXObj::m_getPostroll,		 0, 0,  200 },	// D2
	{ "SetFieldDominance",	OrthoPlayXObj::m_setFieldDominance,	 1, 1,  200 },	// D2
	{ "PlayCue",			OrthoPlayXObj::m_playCue,			 0, 0,  200 },	// D2
	{ "PlaySegment",		OrthoPlayXObj::m_playSegment,		 0, 0,  200 },	// D2
	{ "RecordCue",			OrthoPlayXObj::m_recordCue,			 0, 0,  200 },	// D2
	{ "RecordSegment",		OrthoPlayXObj::m_recordSegment,		 0, 0,  200 },	// D2
	{ "RecordVideoEnable",	OrthoPlayXObj::m_recordVideoEnable,	 1, 1,  200 },	// D2
	{ "RecordAudioEnable",	OrthoPlayXObj::m_recordAudioEnable,	 2, 2,  200 },	// D2
	{ "AssembleRecord",		OrthoPlayXObj::m_assembleRecord,	 0, 0,  200 },	// D2
	{ "PreviewRecord",		OrthoPlayXObj::m_previewRecord,		 0, 0,  200 },	// D2
	{ "GotoInPoint",		OrthoPlayXObj::m_gotoInPoint,		 0, 0,  200 },	// D2
	{ "GotoOutPoint",		OrthoPlayXObj::m_gotoOutPoint,		 0, 0,  200 },	// D2
	{ "GotoPrerollPoint",	OrthoPlayXObj::m_gotoPrerollPoint,	 0, 0,  200 },	// D2
	{ "GotoPostrollPoint",	OrthoPlayXObj::m_gotoPostrollPoint,	 0, 0,  200 },	// D2
	{ nullptr, nullptr, 0, 0, 0 }
};

void OrthoPlayXObj::open(int type) {
	if (type == kXObj) {
		OrthoPlayXObject::initMethods(xlibMethods);
		OrthoPlayXObject *xobj = new OrthoPlayXObject(kXObj);
		g_lingo->_globalvars[xlibName] = xobj;
	}
}

void OrthoPlayXObj::close(int type) {
	if (type == kXObj) {
		OrthoPlayXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}


OrthoPlayXObject::OrthoPlayXObject(ObjectType ObjectType) :Object<OrthoPlayXObject>("OrthoPlayXObj") {
	_objType = ObjectType;
}

void OrthoPlayXObj::m_new(int nargs) {
	g_lingo->push(g_lingo->_currentMe);
}

void OrthoPlayXObj::m_setSerialPort(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_setSerialPort", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_setInitViaDlog(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_setInitViaDlog", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_getInitInfo(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_getInitInfo", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_setInitInfo(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_setInitInfo", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_getMaxDevices(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_getMaxDevices", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_getDeviceTitle(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_getDeviceTitle", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_setDevice(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_setDevice", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_selectDevice(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_selectDevice", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_getDevice(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_getDevice", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_service(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_service", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_getValue(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_getValue", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_cancel(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_cancel", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_explain(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_explain", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_idle(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_idle", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_readStatus(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_readStatus", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_readPos(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_readPos", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_searchTo(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_searchTo", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_play(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_play", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_still(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_still", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_stop(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_stop", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_scanForward(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_scanForward", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_scanReverse(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_scanReverse", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_playReverse(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_playReverse", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_fastForward(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_fastForward", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_rewind(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_rewind", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_stepForward(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_stepForward", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_stepReverse(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_stepReverse", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_shuttle(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_shuttle", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_record(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_record", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_eject(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_eject", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_prepareMedium(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_prepareMedium", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_getFirstTrack(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_getFirstTrack", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_getLastTrack(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_getLastTrack", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_getFirstFrame(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_getFirstFrame", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_getLastFrame(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_getLastFrame", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_getTrack(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_getTrack", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_resetCounter(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_resetCounter", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_audioEnable(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_audioEnable", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_audioMute(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_audioMute", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_videoEnable(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_videoEnable", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_showFrame(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_showFrame", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_getFrameResolution(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_getFrameResolution", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_setFrameResolution(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_setFrameResolution", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_hasDropFrames(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_hasDropFrames", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_sendRaw(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_sendRaw", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_readRaw(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_readRaw", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_setInPoint(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_setInPoint", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_setOutPoint(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_setOutPoint", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_setDuration(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_setDuration", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_getMinDuration(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_getMinDuration", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_setPreroll(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_setPreroll", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_getPreroll(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_getPreroll", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_setPostroll(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_setPostroll", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_getPostroll(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_getPostroll", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_setFieldDominance(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_setFieldDominance", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_playCue(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_playCue", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_playSegment(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_playSegment", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_recordCue(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_recordCue", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_recordSegment(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_recordSegment", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_recordVideoEnable(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_recordVideoEnable", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_recordAudioEnable(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_recordAudioEnable", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_assembleRecord(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_assembleRecord", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_previewRecord(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_previewRecord", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(Datum());
}

void OrthoPlayXObj::m_gotoInPoint(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_gotoInPoint", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_gotoOutPoint(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_gotoOutPoint", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_gotoPrerollPoint(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_gotoPrerollPoint", nargs);
	g_lingo->dropStack(nargs);
}

void OrthoPlayXObj::m_gotoPostrollPoint(int nargs) {
	g_lingo->printSTUBWithArglist("OrthoPlayXObj::m_gotoPostrollPoint", nargs);
	g_lingo->dropStack(nargs);
}



} // End of namespace Director
