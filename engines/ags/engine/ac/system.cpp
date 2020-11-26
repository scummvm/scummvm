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

#include "ags/shared/ac/common.h"
#include "ags/shared/ac/draw.h"
#include "ags/shared/ac/gamesetup.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/shared/ac/gamestate.h"
#include "ags/shared/ac/mouse.h"
#include "ags/shared/ac/string.h"
#include "ags/shared/ac/system.h"
#include "ags/shared/ac/dynobj/scriptsystem.h"
#include "ags/shared/debugging/debug_log.h"
#include "ags/shared/debugging/out.h"
#include "ags/shared/main/engine.h"
#include "ags/shared/main/main.h"
#include "ags/shared/gfx/graphicsdriver.h"
#include "ags/shared/ac/dynobj/cc_audiochannel.h"
#include "ags/shared/main/graphics_mode.h"
#include "ags/shared/ac/global_debug.h"
#include "ags/shared/ac/global_translation.h"
#include "ags/shared/media/audio/audio_system.h"
#include "ags/shared/util/string_compat.h"

#include "ags/shared/debugging/out.h"
#include "ags/shared/script/script_api.h"
#include "ags/shared/script/script_runtime.h"
#include "ags/shared/ac/dynobj/scriptstring.h"

namespace AGS3 {

using namespace AGS::Shared;
using namespace AGS::Engine;

extern GameSetupStruct game;
extern GameSetup usetup;
extern GameState play;
extern ScriptAudioChannel scrAudioChannel[MAX_SOUND_CHANNELS + 1];
extern ScriptSystem scsystem;
extern IGraphicsDriver *gfxDriver;
extern CCAudioChannel ccDynamicAudio;
extern volatile bool switched_away;

bool System_HasInputFocus() {
	return !switched_away;
}

int System_GetColorDepth() {
	return scsystem.coldepth;
}

int System_GetOS() {
	return scsystem.os;
}

// [IKM] 2014-09-21
// IMPORTANT NOTE on System.ScreenWidth and System.ScreenHeight:
// It appears that in AGS these properties were not defining actual window size
// in pixels, but rather game frame size, which could include black borders,
// in 'native' (unscaled) pixels. This was due the specifics of how graphics
// modes were implemented in previous versions.
//
// Quote from the old manual:
// "Returns the actual screen width that the game is running at. If a graphic
//  filter is in use, the resolution returned will be that before any
//  stretching by the filter has been applied. If widescreen side borders are
//  enabled, the screen width reported will include the size of these borders."
//
// The key words are "the resolution returned will be that BEFORE any
// stretching by the filter has been applied".
//
// Since now the letterbox and pillarbox borders are handled by graphics
// renderer and are not part of the game anymore, these properties should
// return strictly native game size. This is required for backwards
// compatibility.
//
int System_GetScreenWidth() {
	return game.GetGameRes().Width;
}

int System_GetScreenHeight() {
	return game.GetGameRes().Height;
}

int System_GetViewportHeight() {
	return game_to_data_coord(play.GetMainViewport().GetHeight());
}

int System_GetViewportWidth() {
	return game_to_data_coord(play.GetMainViewport().GetWidth());
}

const char *System_GetVersion() {
	return CreateNewScriptString(EngineVersion.LongString);
}

int System_GetHardwareAcceleration() {
	return gfxDriver->HasAcceleratedTransform() ? 1 : 0;
}

int System_GetNumLock() {
	return (key_shifts & KB_NUMLOCK_FLAG) ? 1 : 0;
}

int System_GetCapsLock() {
	return (key_shifts & KB_CAPSLOCK_FLAG) ? 1 : 0;
}

int System_GetScrollLock() {
	return (key_shifts & KB_SCROLOCK_FLAG) ? 1 : 0;
}

void System_SetNumLock(int newValue) {
	// doesn't work ... maybe allegro doesn't implement this on windows
	int ledState = key_shifts & (KB_SCROLOCK_FLAG | KB_CAPSLOCK_FLAG);
	if (newValue) {
		ledState |= KB_NUMLOCK_FLAG;
	}
	set_leds(ledState);
}

int System_GetVsync() {
	return scsystem.vsync;
}

void System_SetVsync(int newValue) {
	if (ags_stricmp(gfxDriver->GetDriverID(), "D3D9") != 0)
		scsystem.vsync = newValue;
}

int System_GetWindowed() {
	return scsystem.windowed;
}

void System_SetWindowed(int windowed) {
	if (windowed != scsystem.windowed)
		engine_try_switch_windowed_gfxmode();
}

int System_GetSupportsGammaControl() {
	return gfxDriver->SupportsGammaControl();
}

int System_GetGamma() {
	return play.gamma_adjustment;
}

void System_SetGamma(int newValue) {
	if ((newValue < 0) || (newValue > 200))
		quitprintf("!System.Gamma: value must be between 0-200 (not %d)", newValue);

	if (play.gamma_adjustment != newValue) {
		debug_script_log("Gamma control set to %d", newValue);
		play.gamma_adjustment = newValue;

		if (gfxDriver->SupportsGammaControl())
			gfxDriver->SetGamma(newValue);
	}
}

int System_GetAudioChannelCount() {
	return MAX_SOUND_CHANNELS;
}

ScriptAudioChannel *System_GetAudioChannels(int index) {
	if ((index < 0) || (index >= MAX_SOUND_CHANNELS))
		quit("!System.AudioChannels: invalid sound channel index");

	return &scrAudioChannel[index];
}

int System_GetVolume() {
	return play.digital_master_volume;
}

void System_SetVolume(int newvol) {
	if ((newvol < 0) || (newvol > 100))
		quit("!System.Volume: invalid volume - must be from 0-100");

	if (newvol == play.digital_master_volume)
		return;

	play.digital_master_volume = newvol;
	set_volume((newvol * 255) / 100, (newvol * 255) / 100);

	// allegro's set_volume can lose the volumes of all the channels
	// if it was previously set low; so restore them
	AudioChannelsLock lock;
	for (int i = 0; i <= MAX_SOUND_CHANNELS; i++) {
		auto *ch = lock.GetChannelIfPlaying(i);
		if (ch)
			ch->adjust_volume();
	}
}

const char *System_GetRuntimeInfo() {
	String runtimeInfo = GetRuntimeInfo();

	return CreateNewScriptString(runtimeInfo.GetCStr());
}

int System_GetRenderAtScreenResolution() {
	return usetup.RenderAtScreenRes;
}

void System_SetRenderAtScreenResolution(int enable) {
	usetup.RenderAtScreenRes = enable != 0;
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

extern ScriptString myScriptStringImpl;

// int ()
RuntimeScriptValue Sc_System_GetAudioChannelCount(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(System_GetAudioChannelCount);
}

// ScriptAudioChannel* (int index)
RuntimeScriptValue Sc_System_GetAudioChannels(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJ_PINT(ScriptAudioChannel, ccDynamicAudio, System_GetAudioChannels);
}

// int ()
RuntimeScriptValue Sc_System_GetCapsLock(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(System_GetCapsLock);
}

// int ()
RuntimeScriptValue Sc_System_GetColorDepth(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(System_GetColorDepth);
}

// int ()
RuntimeScriptValue Sc_System_GetGamma(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(System_GetGamma);
}

// void (int newValue)
RuntimeScriptValue Sc_System_SetGamma(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(System_SetGamma);
}

// int ()
RuntimeScriptValue Sc_System_GetHardwareAcceleration(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(System_GetHardwareAcceleration);
}

RuntimeScriptValue Sc_System_GetHasInputFocus(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_BOOL(System_HasInputFocus);
}

// int ()
RuntimeScriptValue Sc_System_GetNumLock(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(System_GetNumLock);
}

// void (int newValue)
RuntimeScriptValue Sc_System_SetNumLock(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(System_SetNumLock);
}

// int ()
RuntimeScriptValue Sc_System_GetOS(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(System_GetOS);
}

// int ()
RuntimeScriptValue Sc_System_GetScreenHeight(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(System_GetScreenHeight);
}

// int ()
RuntimeScriptValue Sc_System_GetScreenWidth(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(System_GetScreenWidth);
}

// int ()
RuntimeScriptValue Sc_System_GetScrollLock(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(System_GetScrollLock);
}

// int ()
RuntimeScriptValue Sc_System_GetSupportsGammaControl(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(System_GetSupportsGammaControl);
}

// const char *()
RuntimeScriptValue Sc_System_GetVersion(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJ(const char, myScriptStringImpl, System_GetVersion);
}

// int ()
RuntimeScriptValue Sc_System_GetViewportHeight(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(System_GetViewportHeight);
}

// int ()
RuntimeScriptValue Sc_System_GetViewportWidth(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(System_GetViewportWidth);
}

// int ()
RuntimeScriptValue Sc_System_GetVolume(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(System_GetVolume);
}

// void (int newvol)
RuntimeScriptValue Sc_System_SetVolume(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(System_SetVolume);
}

// int ()
RuntimeScriptValue Sc_System_GetVsync(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(System_GetVsync);
}

// void (int newValue)
RuntimeScriptValue Sc_System_SetVsync(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(System_SetVsync);
}

RuntimeScriptValue Sc_System_GetWindowed(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(System_GetWindowed);
}

RuntimeScriptValue Sc_System_SetWindowed(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(System_SetWindowed);
}

// const char *()
RuntimeScriptValue Sc_System_GetRuntimeInfo(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJ(const char, myScriptStringImpl, System_GetRuntimeInfo);
}

RuntimeScriptValue Sc_System_GetRenderAtScreenResolution(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(System_GetRenderAtScreenResolution);
}

RuntimeScriptValue Sc_System_SetRenderAtScreenResolution(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(System_SetRenderAtScreenResolution);
}

RuntimeScriptValue Sc_System_Log(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_SCRIPT_SPRINTF(Sc_System_Log, 2);
	Debug::Printf(kDbgGroup_Script, (MessageType)params[0].IValue, "%s", scsf_buffer);
	return RuntimeScriptValue((int32_t)0);
}



void RegisterSystemAPI() {
	ccAddExternalStaticFunction("System::get_AudioChannelCount", Sc_System_GetAudioChannelCount);
	ccAddExternalStaticFunction("System::geti_AudioChannels", Sc_System_GetAudioChannels);
	ccAddExternalStaticFunction("System::get_CapsLock", Sc_System_GetCapsLock);
	ccAddExternalStaticFunction("System::get_ColorDepth", Sc_System_GetColorDepth);
	ccAddExternalStaticFunction("System::get_Gamma", Sc_System_GetGamma);
	ccAddExternalStaticFunction("System::set_Gamma", Sc_System_SetGamma);
	ccAddExternalStaticFunction("System::get_HardwareAcceleration", Sc_System_GetHardwareAcceleration);
	ccAddExternalStaticFunction("System::get_HasInputFocus", Sc_System_GetHasInputFocus);
	ccAddExternalStaticFunction("System::get_NumLock", Sc_System_GetNumLock);
	ccAddExternalStaticFunction("System::set_NumLock", Sc_System_SetNumLock);
	ccAddExternalStaticFunction("System::get_OperatingSystem", Sc_System_GetOS);
	ccAddExternalStaticFunction("System::get_RenderAtScreenResolution", Sc_System_GetRenderAtScreenResolution);
	ccAddExternalStaticFunction("System::set_RenderAtScreenResolution", Sc_System_SetRenderAtScreenResolution);
	ccAddExternalStaticFunction("System::get_RuntimeInfo", Sc_System_GetRuntimeInfo);
	ccAddExternalStaticFunction("System::get_ScreenHeight", Sc_System_GetScreenHeight);
	ccAddExternalStaticFunction("System::get_ScreenWidth", Sc_System_GetScreenWidth);
	ccAddExternalStaticFunction("System::get_ScrollLock", Sc_System_GetScrollLock);
	ccAddExternalStaticFunction("System::get_SupportsGammaControl", Sc_System_GetSupportsGammaControl);
	ccAddExternalStaticFunction("System::get_Version", Sc_System_GetVersion);
	ccAddExternalStaticFunction("SystemInfo::get_Version", Sc_System_GetVersion);
	ccAddExternalStaticFunction("System::get_ViewportHeight", Sc_System_GetViewportHeight);
	ccAddExternalStaticFunction("System::get_ViewportWidth", Sc_System_GetViewportWidth);
	ccAddExternalStaticFunction("System::get_Volume", Sc_System_GetVolume);
	ccAddExternalStaticFunction("System::set_Volume", Sc_System_SetVolume);
	ccAddExternalStaticFunction("System::get_VSync", Sc_System_GetVsync);
	ccAddExternalStaticFunction("System::set_VSync", Sc_System_SetVsync);
	ccAddExternalStaticFunction("System::get_Windowed", Sc_System_GetWindowed);
	ccAddExternalStaticFunction("System::set_Windowed", Sc_System_SetWindowed);
	ccAddExternalStaticFunction("System::Log^102", Sc_System_Log);

	/* ----------------------- Registering unsafe exports for plugins -----------------------*/

	ccAddExternalFunctionForPlugin("System::get_AudioChannelCount", (void *)System_GetAudioChannelCount);
	ccAddExternalFunctionForPlugin("System::geti_AudioChannels", (void *)System_GetAudioChannels);
	ccAddExternalFunctionForPlugin("System::get_CapsLock", (void *)System_GetCapsLock);
	ccAddExternalFunctionForPlugin("System::get_ColorDepth", (void *)System_GetColorDepth);
	ccAddExternalFunctionForPlugin("System::get_Gamma", (void *)System_GetGamma);
	ccAddExternalFunctionForPlugin("System::set_Gamma", (void *)System_SetGamma);
	ccAddExternalFunctionForPlugin("System::get_HardwareAcceleration", (void *)System_GetHardwareAcceleration);
	ccAddExternalFunctionForPlugin("System::get_NumLock", (void *)System_GetNumLock);
	ccAddExternalFunctionForPlugin("System::set_NumLock", (void *)System_SetNumLock);
	ccAddExternalFunctionForPlugin("System::get_OperatingSystem", (void *)System_GetOS);
	ccAddExternalFunctionForPlugin("System::get_RuntimeInfo", (void *)System_GetRuntimeInfo);
	ccAddExternalFunctionForPlugin("System::get_ScreenHeight", (void *)System_GetScreenHeight);
	ccAddExternalFunctionForPlugin("System::get_ScreenWidth", (void *)System_GetScreenWidth);
	ccAddExternalFunctionForPlugin("System::get_ScrollLock", (void *)System_GetScrollLock);
	ccAddExternalFunctionForPlugin("System::get_SupportsGammaControl", (void *)System_GetSupportsGammaControl);
	ccAddExternalFunctionForPlugin("System::get_Version", (void *)System_GetVersion);
	ccAddExternalFunctionForPlugin("SystemInfo::get_Version", (void *)System_GetVersion);
	ccAddExternalFunctionForPlugin("System::get_ViewportHeight", (void *)System_GetViewportHeight);
	ccAddExternalFunctionForPlugin("System::get_ViewportWidth", (void *)System_GetViewportWidth);
	ccAddExternalFunctionForPlugin("System::get_Volume", (void *)System_GetVolume);
	ccAddExternalFunctionForPlugin("System::set_Volume", (void *)System_SetVolume);
	ccAddExternalFunctionForPlugin("System::get_VSync", (void *)System_GetVsync);
	ccAddExternalFunctionForPlugin("System::set_VSync", (void *)System_SetVsync);
	ccAddExternalFunctionForPlugin("System::get_Windowed", (void *)System_GetWindowed);
}

} // namespace AGS3
