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
 * Standard Macromedia XObject
 *
 *************************************/

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
#include "director/lingo/lingo-utils.h"
#include "director/lingo/xlibs/orthoplayxobj.h"


namespace Director {

const char *const OrthoPlayXObj::xlibName = "OrthoPlayXObj";
const XlibFileDesc OrthoPlayXObj::fileNames[] = {
	{ "OrthoPlay XObj",	nullptr },
	{ nullptr,			nullptr },
};

static const MethodProto xlibMethods[] = {
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

void OrthoPlayXObj::open(ObjectType type, const Common::Path &path) {
	if (type == kXObj) {
		OrthoPlayXObject::initMethods(xlibMethods);
		OrthoPlayXObject *xobj = new OrthoPlayXObject(kXObj);
		g_lingo->exposeXObject(xlibName, xobj);
	}
}

void OrthoPlayXObj::close(ObjectType type) {
	if (type == kXObj) {
		OrthoPlayXObject::cleanupMethods();
		g_lingo->_globalvars[xlibName] = Datum();
	}
}

OrthoPlayXObject::OrthoPlayXObject(ObjectType ObjectType) :Object<OrthoPlayXObject>("OrthoPlayXObj") {
	_objType = ObjectType;
}

void OrthoPlayXObj::m_new(int nargs) {
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUBV(OrthoPlayXObj::m_setSerialPort)
XOBJSTUBV(OrthoPlayXObj::m_setInitViaDlog)
XOBJSTUBV(OrthoPlayXObj::m_getInitInfo)
XOBJSTUBV(OrthoPlayXObj::m_setInitInfo)
XOBJSTUBV(OrthoPlayXObj::m_getMaxDevices)
XOBJSTUBV(OrthoPlayXObj::m_getDeviceTitle)
XOBJSTUBV(OrthoPlayXObj::m_setDevice)
XOBJSTUBV(OrthoPlayXObj::m_selectDevice)
XOBJSTUBV(OrthoPlayXObj::m_getDevice)
XOBJSTUBV(OrthoPlayXObj::m_service)
XOBJSTUBV(OrthoPlayXObj::m_getValue)
XOBJSTUBV(OrthoPlayXObj::m_cancel)
XOBJSTUBV(OrthoPlayXObj::m_explain)
XOBJSTUBV(OrthoPlayXObj::m_idle)
XOBJSTUBNR(OrthoPlayXObj::m_readStatus)
XOBJSTUBNR(OrthoPlayXObj::m_readPos)
XOBJSTUBNR(OrthoPlayXObj::m_searchTo)
XOBJSTUBNR(OrthoPlayXObj::m_play)
XOBJSTUBNR(OrthoPlayXObj::m_still)
XOBJSTUBNR(OrthoPlayXObj::m_stop)
XOBJSTUBNR(OrthoPlayXObj::m_scanForward)
XOBJSTUBNR(OrthoPlayXObj::m_scanReverse)
XOBJSTUBNR(OrthoPlayXObj::m_playReverse)
XOBJSTUBNR(OrthoPlayXObj::m_fastForward)
XOBJSTUBNR(OrthoPlayXObj::m_rewind)
XOBJSTUBNR(OrthoPlayXObj::m_stepForward)
XOBJSTUBNR(OrthoPlayXObj::m_stepReverse)
XOBJSTUBNR(OrthoPlayXObj::m_shuttle)
XOBJSTUBNR(OrthoPlayXObj::m_record)
XOBJSTUBNR(OrthoPlayXObj::m_eject)
XOBJSTUBNR(OrthoPlayXObj::m_prepareMedium)
XOBJSTUBV(OrthoPlayXObj::m_getFirstTrack)
XOBJSTUBV(OrthoPlayXObj::m_getLastTrack)
XOBJSTUBV(OrthoPlayXObj::m_getFirstFrame)
XOBJSTUBV(OrthoPlayXObj::m_getLastFrame)
XOBJSTUBV(OrthoPlayXObj::m_getTrack)
XOBJSTUBNR(OrthoPlayXObj::m_resetCounter)
XOBJSTUBNR(OrthoPlayXObj::m_audioEnable)
XOBJSTUBNR(OrthoPlayXObj::m_audioMute)
XOBJSTUBNR(OrthoPlayXObj::m_videoEnable)
XOBJSTUBNR(OrthoPlayXObj::m_showFrame)
XOBJSTUBV(OrthoPlayXObj::m_getFrameResolution)
XOBJSTUBV(OrthoPlayXObj::m_setFrameResolution)
XOBJSTUBV(OrthoPlayXObj::m_hasDropFrames)
XOBJSTUBNR(OrthoPlayXObj::m_sendRaw)
XOBJSTUBV(OrthoPlayXObj::m_readRaw)
XOBJSTUBV(OrthoPlayXObj::m_setInPoint)
XOBJSTUBV(OrthoPlayXObj::m_setOutPoint)
XOBJSTUBV(OrthoPlayXObj::m_setDuration)
XOBJSTUBV(OrthoPlayXObj::m_getMinDuration)
XOBJSTUBV(OrthoPlayXObj::m_setPreroll)
XOBJSTUBV(OrthoPlayXObj::m_getPreroll)
XOBJSTUBV(OrthoPlayXObj::m_setPostroll)
XOBJSTUBV(OrthoPlayXObj::m_getPostroll)
XOBJSTUBV(OrthoPlayXObj::m_setFieldDominance)
XOBJSTUBNR(OrthoPlayXObj::m_playCue)
XOBJSTUBNR(OrthoPlayXObj::m_playSegment)
XOBJSTUBNR(OrthoPlayXObj::m_recordCue)
XOBJSTUBNR(OrthoPlayXObj::m_recordSegment)
XOBJSTUBNR(OrthoPlayXObj::m_recordVideoEnable)
XOBJSTUBNR(OrthoPlayXObj::m_recordAudioEnable)
XOBJSTUBV(OrthoPlayXObj::m_assembleRecord)
XOBJSTUBV(OrthoPlayXObj::m_previewRecord)
XOBJSTUBNR(OrthoPlayXObj::m_gotoInPoint)
XOBJSTUBNR(OrthoPlayXObj::m_gotoOutPoint)
XOBJSTUBNR(OrthoPlayXObj::m_gotoPrerollPoint)
XOBJSTUBNR(OrthoPlayXObj::m_gotoPostrollPoint)

} // End of namespace Director
