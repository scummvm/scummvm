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
#include "director/lingo/xtras/a/atixtrl.h"

/**************************************************
 *
 * USED IN:
 * [insert game here]
 *
 **************************************************/

/*
-- xtra ATIXtrl
-- ATIXtrl Xtra, for controlling video preview and capture on
-- the ATI All-In-Wonder video card with Director 5 & later.
-- Requires DVA16.DLL and DVA32.DLL in the same folder or in Windows\System,
-- plus installed ATI video drivers. Windows 95/98 only. (Requires MoaFile.x32
-- in Xtras folder when used with Director 5.)
-- 
-- Copyright © 1997-1999, Glenn M. Picher, Dirigo Multimedia
-- gpicher@maine.com, http://www.maine.com/shops/gpicher
-- (207)767-8015
-- 
-- Demo evaluation version 1.0.2.21, April 23rd, 1999.
-- *Not* licensed for distribution!
-- 
new object me
-- Note: all handlers are global. No need to create a new object!
--
* atixLoadDriver
-- Establishes connection with ATI drivers.
-- Must be used before any other method; must call
-- atixUnloadDriver when done with the driver!
* atixUnloadDriver
-- Disconnects from ATI drivers.
* atixGetSource
-- Returns currently selected connector; either
-- 'tuner', 'composite', or 'svideo'.
* atixSetSource string connector
-- Sets the input connector;
-- supply 'tuner', 'composite', or 'svideo'.
* atixLowestChannel
* atixHighestChannel
-- Tells you the lowest and highest channels to which the tuner may be tuned
-- with atixTuneToChannel(), considering NTSC/PAL and ANTENNA/CABLE settings.
* atixTuneToChannel integer channelNumber
-- Tune to a particular fixed channel number.
* atixTunerLocked
-- Verifies that a valid signal is being tuned on the last tuned channel.
-- (You may need a few seconds' delay after using atixTuneToChannel().)
* atixSetBrightness integer brightValue
* atixSetContrast integer contrastValue
* atixSetColor integer colorValue
-- Sets the brightness, contrast, and color intensity respectively,
-- in the range 0 to 200 percent of normal.
* atixSetTint integer tintValue
-- Sets the tint value, in the range -180 to 180 degrees from normal.
* atixSetHAdjust integer adjustValue
* atixSetVAdjust integer adjustValue
-- Adjusts video placement; supply -50 to 50. Not all cards support this.
* atixSetTracking integer trackingSettting
-- Adjust tracking; supply 0, 1 or 2. Not all cards support this.
* atixSetSharpness integer sharpValue
-- Adjusts sharpness; supply 0 (smooth) or 1 (sharp). Not all cards support this.
* atixSetColorTemp integer tempValue
-- Adjust color temp; supply 0 (cool) or 1 (warm).
* atixSetQuality integer qualityValue
-- Sets quality-vs-speed tradeoff. Supply 0 (speed) or 1 (quality). Must have
-- already prepared a preview, overlay, or capture. May not have any effect
-- with some drivers.
* atixPreparePreview
-- Show preview display, without transparency. See also atixSetMute().
* atixPrepareOverlay
-- Show preview display, with transparency. See also atixSetMute(). Video
-- will show through any pure magenta pixels within the onscreen area.
-- Use atixSetPreviewRect() first to specify window location and size (or
-- a default rectangle 0,0 to 640,480 will be used, aligned with the top
-- left of the stage).
* atixPrepareCapture
-- Prepares for capture. No onscreen display. Only a preview, overlay or capture
-- can be prepared at any one time-- not more than one. Preparing one automatically
-- unprepares any others, if necessary.
* atixSetCropping integer l, integer t, integer r, integer b
-- Sets cropping area within 640x480 hardware capture area to be saved
-- in future captures; also controls cropping in preview windows (the
-- preview will be scaled up to fit the preview window size). Generally,
-- the preview window size should match the cropping size if you want
-- previews to exactly match captures. The supplied dimensions must
-- be within 640x480, and no smaller than 4x4. Use this *after*
-- atixPrepareCapture(), or *before* the preview prepare methods.
-- (NOTE: This is *not* implemented for previews in this version of the
-- Xtra, but it *does* work for captures. This appears to be due to an ATI
-- bug, a workaround for which *may* be available in a future version of
-- this Xtra. Use a preview overlay, and magenta bitmap positioned to match the
-- crop area, within a 640x480 preview, for preview positioning, as a
-- workaround for this problem, using this version of the Xtra.
* atixSetPreviewRect integer l, integer t, integer r, integer b
-- Sets onscreen position and size of preview or overlay area. Coordinates
-- are relative to the top left of the stage. Use this *before*
-- atixPreparePreview() or atixPrepareOverlay() to set an initial
-- position; you can also move the window if it's already displayed.
* atixUnprepare
-- Ends preview sequence, or capture readiness.
* atixCaptureFrame any whereToPut
-- Captures one video field either to a
-- .BMP file (if a string is provided) or into an existing bitmap
-- cast member (if a member reference is provided). Include the full
-- path, and '.BMP', in the filename string, if saving to a BMP file.
-- BMP files are saved as 24 bit; members are saved with whatever bit
-- depth the member currently uses. Only saves the area within the
-- rectangle set by atixSetCropping().
* atixConfigureDialog
-- Shows a configuration dialog. Preview, overlay or capture must already
-- have been prepared. Driver may or may not support this dialog.
* atixDriverInfoDialog
-- Shows a driver information dialog, which can help identifying driver version
-- numbers and technical support issues.
* atixSetVolumes integer leftVolume, integer rightVolume
-- Sets tuner volumes, 0-1000. If stereo isn't supported, the two values are averaged.
-- See also atixSetMute().
* atixGetLeftVolume
* atixGetRightVolume
-- Returns current tuner volume settings, 0-1000. If stereo isn't supported, both
-- return the mono setting.
* atixCanUseStereo
-- Returns TRUE or FALSE based on whether the driver supports true stereo volumes.
* atixSetSoundMix integer mixRate
-- Sets sound mix between 0 (all passthrough) and 1000 (all tuner). Intermediate
-- numbers mix the two sources. Not all drivers may support this method. See also
-- atixSetMute().
* atixCanMixSound
-- Returns TRUE or FALSE based on the driver's sound mixing capabilities.
* atixSetSoundSwitch integer switchSetting
-- Sets sound switch to 0 (audio passthrough) or 1 (tuner). Not all drivers may
-- support this method. See also atixSetMute().
* atixCanSwitchSound
-- Returns TRUE or FALSE based on the driver's sound switching capabilities.
* atixSetMute integer muteSetting
-- Sets audio muting (1 to mute, 0 to unmute). Normally you'll unmute the sound,
-- set the sound switch to tuner, and turn up the volume, when you display a tuner
-- input video window. Previous versions of this Xtra lacking any audio methods did
-- these three things automatically when preparing a tuner preview or overlay.
* atixGetAudioDrivers
-- Gets a string of all audio drivers, one per line. Usually there's only one.
* atixUseAudioDriver integer driverNum
-- Sets which audio driver to use. Usually there's only one. Default is 1.
* atixGetAudioConnectors
-- Gets a string of all audio connectors for the current audio driver, one per line.
* atixUseAudioConnector integer connectorNum
-- Specifies which audio connector to use with the next two audio methods.
* atixGetConnectorRecordVolume
-- Returns a record volume level for the current audio connector, 0 to 100.
* atixSetConnectorRecordVolume integer newVolume
-- Sets a volume level for the current audio connector, 0 to 100.
"
 */

namespace Director {

const char *AtixtrlXtra::xlibName = "Atixtrl";
const XlibFileDesc AtixtrlXtra::fileNames[] = {
	{ "atixtrl",   nullptr },
	{ nullptr,        nullptr },
};

static MethodProto xlibMethods[] = {
	{ "new",				AtixtrlXtra::m_new,		 0, 0,	500 },
	{ """,				AtixtrlXtra::m_",		 -1, -1,	500 },

	{ nullptr, nullptr, 0, 0, 0 }
};

static BuiltinProto xlibBuiltins[] = {
	{ "atixLoadDriver", AtixtrlXtra::m_atixLoadDriver, 0, 0, 500, HBLTIN },
	{ "atixUnloadDriver", AtixtrlXtra::m_atixUnloadDriver, 0, 0, 500, HBLTIN },
	{ "atixGetSource", AtixtrlXtra::m_atixGetSource, 0, 0, 500, HBLTIN },
	{ "atixSetSource", AtixtrlXtra::m_atixSetSource, 1, 1, 500, HBLTIN },
	{ "atixLowestChannel", AtixtrlXtra::m_atixLowestChannel, 0, 0, 500, HBLTIN },
	{ "atixHighestChannel", AtixtrlXtra::m_atixHighestChannel, 0, 0, 500, HBLTIN },
	{ "atixTuneToChannel", AtixtrlXtra::m_atixTuneToChannel, 1, 1, 500, HBLTIN },
	{ "atixTunerLocked", AtixtrlXtra::m_atixTunerLocked, 0, 0, 500, HBLTIN },
	{ "atixSetBrightness", AtixtrlXtra::m_atixSetBrightness, 1, 1, 500, HBLTIN },
	{ "atixSetContrast", AtixtrlXtra::m_atixSetContrast, 1, 1, 500, HBLTIN },
	{ "atixSetColor", AtixtrlXtra::m_atixSetColor, 1, 1, 500, HBLTIN },
	{ "atixSetTint", AtixtrlXtra::m_atixSetTint, 1, 1, 500, HBLTIN },
	{ "atixSetHAdjust", AtixtrlXtra::m_atixSetHAdjust, 1, 1, 500, HBLTIN },
	{ "atixSetVAdjust", AtixtrlXtra::m_atixSetVAdjust, 1, 1, 500, HBLTIN },
	{ "atixSetTracking", AtixtrlXtra::m_atixSetTracking, 1, 1, 500, HBLTIN },
	{ "atixSetSharpness", AtixtrlXtra::m_atixSetSharpness, 1, 1, 500, HBLTIN },
	{ "atixSetColorTemp", AtixtrlXtra::m_atixSetColorTemp, 1, 1, 500, HBLTIN },
	{ "atixSetQuality", AtixtrlXtra::m_atixSetQuality, 1, 1, 500, HBLTIN },
	{ "atixPreparePreview", AtixtrlXtra::m_atixPreparePreview, 0, 0, 500, HBLTIN },
	{ "atixPrepareOverlay", AtixtrlXtra::m_atixPrepareOverlay, 0, 0, 500, HBLTIN },
	{ "atixPrepareCapture", AtixtrlXtra::m_atixPrepareCapture, 0, 0, 500, HBLTIN },
	{ "atixSetCropping", AtixtrlXtra::m_atixSetCropping, 4, 4, 500, HBLTIN },
	{ "atixSetPreviewRect", AtixtrlXtra::m_atixSetPreviewRect, 4, 4, 500, HBLTIN },
	{ "atixUnprepare", AtixtrlXtra::m_atixUnprepare, 0, 0, 500, HBLTIN },
	{ "atixCaptureFrame", AtixtrlXtra::m_atixCaptureFrame, 1, 1, 500, HBLTIN },
	{ "atixConfigureDialog", AtixtrlXtra::m_atixConfigureDialog, 0, 0, 500, HBLTIN },
	{ "atixDriverInfoDialog", AtixtrlXtra::m_atixDriverInfoDialog, 0, 0, 500, HBLTIN },
	{ "atixSetVolumes", AtixtrlXtra::m_atixSetVolumes, 2, 2, 500, HBLTIN },
	{ "atixGetLeftVolume", AtixtrlXtra::m_atixGetLeftVolume, 0, 0, 500, HBLTIN },
	{ "atixGetRightVolume", AtixtrlXtra::m_atixGetRightVolume, 0, 0, 500, HBLTIN },
	{ "atixCanUseStereo", AtixtrlXtra::m_atixCanUseStereo, 0, 0, 500, HBLTIN },
	{ "atixSetSoundMix", AtixtrlXtra::m_atixSetSoundMix, 1, 1, 500, HBLTIN },
	{ "atixCanMixSound", AtixtrlXtra::m_atixCanMixSound, 0, 0, 500, HBLTIN },
	{ "atixSetSoundSwitch", AtixtrlXtra::m_atixSetSoundSwitch, 1, 1, 500, HBLTIN },
	{ "atixCanSwitchSound", AtixtrlXtra::m_atixCanSwitchSound, 0, 0, 500, HBLTIN },
	{ "atixSetMute", AtixtrlXtra::m_atixSetMute, 1, 1, 500, HBLTIN },
	{ "atixGetAudioDrivers", AtixtrlXtra::m_atixGetAudioDrivers, 0, 0, 500, HBLTIN },
	{ "atixUseAudioDriver", AtixtrlXtra::m_atixUseAudioDriver, 1, 1, 500, HBLTIN },
	{ "atixGetAudioConnectors", AtixtrlXtra::m_atixGetAudioConnectors, 0, 0, 500, HBLTIN },
	{ "atixUseAudioConnector", AtixtrlXtra::m_atixUseAudioConnector, 1, 1, 500, HBLTIN },
	{ "atixGetConnectorRecordVolume", AtixtrlXtra::m_atixGetConnectorRecordVolume, 0, 0, 500, HBLTIN },
	{ "atixSetConnectorRecordVolume", AtixtrlXtra::m_atixSetConnectorRecordVolume, 1, 1, 500, HBLTIN },
	{ nullptr, nullptr, 0, 0, 0, VOIDSYM }
};

AtixtrlXtraObject::AtixtrlXtraObject(ObjectType ObjectType) :Object<AtixtrlXtraObject>("Atixtrl") {
	_objType = ObjectType;
}

bool AtixtrlXtraObject::hasProp(const Common::String &propName) {
	return (propName == "name");
}

Datum AtixtrlXtraObject::getProp(const Common::String &propName) {
	if (propName == "name")
		return Datum(AtixtrlXtra::xlibName);
	warning("AtixtrlXtra::getProp: unknown property '%s'", propName.c_str());
	return Datum();
}

void AtixtrlXtra::open(ObjectType type, const Common::Path &path) {
    AtixtrlXtraObject::initMethods(xlibMethods);
    AtixtrlXtraObject *xobj = new AtixtrlXtraObject(type);
    if (type == kXtraObj) {
        g_lingo->_openXtras.push_back(xlibName);
		g_lingo->_openXtraObjects.push_back(xobj);
	}
    g_lingo->exposeXObject(xlibName, xobj);
    g_lingo->initBuiltIns(xlibBuiltins);
}

void AtixtrlXtra::close(ObjectType type) {
    AtixtrlXtraObject::cleanupMethods();
    g_lingo->_globalvars[xlibName] = Datum();

}

void AtixtrlXtra::m_new(int nargs) {
	g_lingo->printSTUBWithArglist("AtixtrlXtra::m_new", nargs);
	g_lingo->dropStack(nargs);
	g_lingo->push(g_lingo->_state->me);
}

XOBJSTUB(AtixtrlXtra::m_atixLoadDriver, 0)
XOBJSTUB(AtixtrlXtra::m_atixUnloadDriver, 0)
XOBJSTUB(AtixtrlXtra::m_atixGetSource, 0)
XOBJSTUB(AtixtrlXtra::m_atixSetSource, 0)
XOBJSTUB(AtixtrlXtra::m_atixLowestChannel, 0)
XOBJSTUB(AtixtrlXtra::m_atixHighestChannel, 0)
XOBJSTUB(AtixtrlXtra::m_atixTuneToChannel, 0)
XOBJSTUB(AtixtrlXtra::m_atixTunerLocked, 0)
XOBJSTUB(AtixtrlXtra::m_atixSetBrightness, 0)
XOBJSTUB(AtixtrlXtra::m_atixSetContrast, 0)
XOBJSTUB(AtixtrlXtra::m_atixSetColor, 0)
XOBJSTUB(AtixtrlXtra::m_atixSetTint, 0)
XOBJSTUB(AtixtrlXtra::m_atixSetHAdjust, 0)
XOBJSTUB(AtixtrlXtra::m_atixSetVAdjust, 0)
XOBJSTUB(AtixtrlXtra::m_atixSetTracking, 0)
XOBJSTUB(AtixtrlXtra::m_atixSetSharpness, 0)
XOBJSTUB(AtixtrlXtra::m_atixSetColorTemp, 0)
XOBJSTUB(AtixtrlXtra::m_atixSetQuality, 0)
XOBJSTUB(AtixtrlXtra::m_atixPreparePreview, 0)
XOBJSTUB(AtixtrlXtra::m_atixPrepareOverlay, 0)
XOBJSTUB(AtixtrlXtra::m_atixPrepareCapture, 0)
XOBJSTUB(AtixtrlXtra::m_atixSetCropping, 0)
XOBJSTUB(AtixtrlXtra::m_atixSetPreviewRect, 0)
XOBJSTUB(AtixtrlXtra::m_atixUnprepare, 0)
XOBJSTUB(AtixtrlXtra::m_atixCaptureFrame, 0)
XOBJSTUB(AtixtrlXtra::m_atixConfigureDialog, 0)
XOBJSTUB(AtixtrlXtra::m_atixDriverInfoDialog, 0)
XOBJSTUB(AtixtrlXtra::m_atixSetVolumes, 0)
XOBJSTUB(AtixtrlXtra::m_atixGetLeftVolume, 0)
XOBJSTUB(AtixtrlXtra::m_atixGetRightVolume, 0)
XOBJSTUB(AtixtrlXtra::m_atixCanUseStereo, 0)
XOBJSTUB(AtixtrlXtra::m_atixSetSoundMix, 0)
XOBJSTUB(AtixtrlXtra::m_atixCanMixSound, 0)
XOBJSTUB(AtixtrlXtra::m_atixSetSoundSwitch, 0)
XOBJSTUB(AtixtrlXtra::m_atixCanSwitchSound, 0)
XOBJSTUB(AtixtrlXtra::m_atixSetMute, 0)
XOBJSTUB(AtixtrlXtra::m_atixGetAudioDrivers, 0)
XOBJSTUB(AtixtrlXtra::m_atixUseAudioDriver, 0)
XOBJSTUB(AtixtrlXtra::m_atixGetAudioConnectors, 0)
XOBJSTUB(AtixtrlXtra::m_atixUseAudioConnector, 0)
XOBJSTUB(AtixtrlXtra::m_atixGetConnectorRecordVolume, 0)
XOBJSTUB(AtixtrlXtra::m_atixSetConnectorRecordVolume, 0)
XOBJSTUB(AtixtrlXtra::m_", 0)

}
