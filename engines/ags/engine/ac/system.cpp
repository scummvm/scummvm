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
#include "ags/engine/ac/draw.h"
#include "ags/engine/ac/dynobj/cc_audio_channel.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_debug.h"
#include "ags/engine/ac/global_translation.h"
#include "ags/engine/ac/mouse.h"
#include "ags/engine/ac/string.h"
#include "ags/engine/ac/system.h"
#include "ags/engine/ac/dynobj/script_system.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/gfx/graphics_driver.h"
#include "ags/engine/main/config.h"
#include "ags/engine/main/graphics_mode.h"
#include "ags/engine/main/engine.h"
#include "ags/engine/main/main.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/shared/util/string_compat.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/engine/ac/dynobj/script_string.h"
#include "ags/globals.h"
#include "ags/events.h"

namespace AGS3 {

using namespace AGS::Shared;

bool System_HasInputFocus() {
	return !_G(switched_away);
}

int System_GetColorDepth() {
	return _GP(scsystem).coldepth;
}

int System_GetOS() {
	return _GP(scsystem).os;
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
	return _GP(game).GetGameRes().Width;
}

int System_GetScreenHeight() {
	return _GP(game).GetGameRes().Height;
}

int System_GetViewportHeight() {
	return game_to_data_coord(_GP(play).GetMainViewport().GetHeight());
}

int System_GetViewportWidth() {
	return game_to_data_coord(_GP(play).GetMainViewport().GetWidth());
}

const char *System_GetVersion() {
	return CreateNewScriptString(_G(EngineVersion).LongString.GetCStr());
}

int System_GetHardwareAcceleration() {
	return _G(gfxDriver)->HasAcceleratedTransform() ? 1 : 0;
}

int System_GetNumLock() {
	return (::AGS::g_events->getModifierFlags() & Common::KBD_NUM) ? 1 : 0;
}

int System_GetCapsLock() {
	return (::AGS::g_events->getModifierFlags() & Common::KBD_CAPS) ? 1 : 0;
}

int System_GetScrollLock() {
	return (::AGS::g_events->getModifierFlags() & Common::KBD_SCRL) ? 1 : 0;
}

int System_GetVsync() {
	return _GP(scsystem).vsync;
}

void System_SetVsync(int newValue) {
	if (ags_stricmp(_G(gfxDriver)->GetDriverID(), "D3D9") != 0)
		_GP(scsystem).vsync = newValue;
}

int System_GetWindowed() {
	return _GP(scsystem).windowed;
}

void System_SetWindowed(int windowed) {
	if (windowed != _GP(scsystem).windowed)
		engine_try_switch_windowed_gfxmode();
}

int System_GetSupportsGammaControl() {
	return _G(gfxDriver)->SupportsGammaControl();
}

int System_GetGamma() {
	return _GP(play).gamma_adjustment;
}

void System_SetGamma(int newValue) {
	if ((newValue < 0) || (newValue > 200))
		quitprintf("!System.Gamma: value must be between 0-200 (not %d)", newValue);

	if (_GP(play).gamma_adjustment != newValue) {
		debug_script_log("Gamma control set to %d", newValue);
		_GP(play).gamma_adjustment = newValue;

		if (_G(gfxDriver)->SupportsGammaControl())
			_G(gfxDriver)->SetGamma(newValue);
	}
}

int System_GetAudioChannelCount() {
	return MAX_SOUND_CHANNELS;
}

ScriptAudioChannel *System_GetAudioChannels(int index) {
	if ((index < 0) || (index >= MAX_SOUND_CHANNELS))
		quit("!System.AudioChannels: invalid sound channel index");

	return &_G(scrAudioChannel)[index];
}

int System_GetVolume() {
	return _GP(play).digital_master_volume;
}

void System_SetVolume(int newvol) {
	if ((newvol < 0) || (newvol > 100))
		quit("!System.Volume: invalid volume - must be from 0-100");

	_GP(play).digital_master_volume = newvol;
#if !AGS_PLATFORM_SCUMMVM
	auto newvol_f = static_cast<float>(newvol) / 100.0;
	audio_core_set_master_volume(newvol_f);
#endif
}

const char *System_GetRuntimeInfo() {
	String runtimeInfo = GetRuntimeInfo();

	return CreateNewScriptString(runtimeInfo.GetCStr());
}

int System_GetRenderAtScreenResolution() {
	return _GP(usetup).RenderAtScreenRes;
}

void System_SetRenderAtScreenResolution(int enable) {
	_GP(usetup).RenderAtScreenRes = enable != 0;
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

RuntimeScriptValue Sc_System_GetAudioChannelCount(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(System_GetAudioChannelCount);
}

// ScriptAudioChannel* (int index)
RuntimeScriptValue Sc_System_GetAudioChannels(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJ_PINT(ScriptAudioChannel, _GP(ccDynamicAudio), System_GetAudioChannels);
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
	API_CONST_SCALL_OBJ(const char, _GP(myScriptStringImpl), System_GetVersion);
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
	API_CONST_SCALL_OBJ(const char, _GP(myScriptStringImpl), System_GetRuntimeInfo);
}

RuntimeScriptValue Sc_System_GetRenderAtScreenResolution(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(System_GetRenderAtScreenResolution);
}

RuntimeScriptValue Sc_System_SetRenderAtScreenResolution(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(System_SetRenderAtScreenResolution);
}

RuntimeScriptValue Sc_System_SaveConfigToFile(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID(save_config_file);
}

RuntimeScriptValue Sc_System_Log(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_SCRIPT_SPRINTF_PURE(Sc_System_Log, 2);
	Debug::Printf(kDbgGroup_Script, (MessageType)params[0].IValue, String::Wrapper(scsf_buffer));
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

	ccAddExternalStaticFunction("System::SaveConfigToFile", Sc_System_SaveConfigToFile);
	ccAddExternalStaticFunction("System::Log^102", Sc_System_Log);
}

} // namespace AGS3
