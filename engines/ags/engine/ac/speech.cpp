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

#include "ags/engine/ac/speech.h"
#include "ags/engine/ac/asset_helper.h"
#include "ags/shared/ac/common.h"
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_audio.h"
#include "ags/engine/ac/global_display.h"
#include "ags/engine/ac/dynobj/cc_script_object.h"
#include "ags/engine/ac/dynobj/dynobj_manager.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/engine/ac/dynobj/script_overlay.h"
#include "ags/engine/ac/game_setup.h"
#include "ags/engine/ac/game_state.h"
#include "ags/shared/core/asset_manager.h"
#include "ags/engine/main/engine.h"
#include "ags/shared/util/directory.h"
#include "ags/shared/util/path.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

int user_to_internal_skip_speech(SkipSpeechStyle userval) {
	switch (userval) {
	case kSkipSpeechNone:
		return SKIP_NONE;
	case kSkipSpeechKeyMouseTime:
		return SKIP_AUTOTIMER | SKIP_KEYPRESS | SKIP_MOUSECLICK;
	case kSkipSpeechKeyTime:
		return SKIP_AUTOTIMER | SKIP_KEYPRESS;
	case kSkipSpeechTime:
		return SKIP_AUTOTIMER;
	case kSkipSpeechKeyMouse:
		return SKIP_KEYPRESS | SKIP_MOUSECLICK;
	case kSkipSpeechMouseTime:
		return SKIP_AUTOTIMER | SKIP_MOUSECLICK;
	case kSkipSpeechKey:
		return SKIP_KEYPRESS;
	case kSkipSpeechMouse:
		return SKIP_MOUSECLICK;
	default:
		quit("user_to_internal_skip_speech: unknown userval");
		return SKIP_NONE;
	}
}

SkipSpeechStyle internal_skip_speech_to_user(int internal_val) {
	if (internal_val & SKIP_AUTOTIMER) {
		internal_val &= ~SKIP_AUTOTIMER;
		if (internal_val == (SKIP_KEYPRESS | SKIP_MOUSECLICK)) {
			return kSkipSpeechKeyMouseTime;
		} else if (internal_val == SKIP_KEYPRESS) {
			return kSkipSpeechKeyTime;
		} else if (internal_val == SKIP_MOUSECLICK) {
			return kSkipSpeechMouseTime;
		}
		return kSkipSpeechTime;
	} else {
		if (internal_val == (SKIP_KEYPRESS | SKIP_MOUSECLICK)) {
			return kSkipSpeechKeyMouse;
		} else if (internal_val == SKIP_KEYPRESS) {
			return kSkipSpeechKey;
		} else if (internal_val == SKIP_MOUSECLICK) {
			return kSkipSpeechMouse;
		}
	}
	return kSkipSpeechNone;
}

bool init_voicepak(const String &name) {
	if (_GP(usetup).no_speech_pack) return false; // voice-over disabled

	String speech_file = name.IsEmpty() ? "speech.vox" : String::FromFormat("sp_%s.vox", name.GetCStr());
	if (_GP(ResPaths).SpeechPak.Name.CompareNoCase(speech_file) == 0)
		return true; // same pak already assigned

	// First remove existing voice packs
	_GP(ResPaths).VoiceAvail = false;
	// FIXME: don't remove the default speech.vox when changing pak, as this causes a crash in Beyond the Edge of Owlsgard
	// Duplicate checks are already present so this shouldn't cause problems but still, it should be looked into
	if (_GP(ResPaths).SpeechPak.Name.CompareNoCase("speech.vox") != 0)
		_GP(AssetMgr)->RemoveLibrary(_GP(ResPaths).SpeechPak.Path);
	_GP(AssetMgr)->RemoveLibrary(_GP(ResPaths).VoiceDirSub);

	// Now check for the new packs and add if they exist
	String speech_filepath = find_assetlib(speech_file);
	if (!speech_filepath.IsEmpty()) {
		Debug::Printf(kDbgMsg_Info, "Voice pack found: %s", speech_file.GetCStr());
		_GP(ResPaths).VoiceAvail = true;
	} else {
		Debug::Printf(kDbgMsg_Info, "Was not able to init voice pack '%s': file not found or of unknown format.",
			speech_file.GetCStr());
	}

	String speech_subdir = "";
	if (!_GP(ResPaths).VoiceDir2.IsEmpty() &&
			!_GP(ResPaths).VoiceDir2.IsEmpty() && Path::ComparePaths(_GP(ResPaths).DataDir, _GP(ResPaths).VoiceDir2) != 0) {
		// If we have custom voice directory set, we will enable voice-over even if speech.vox does not exist
		speech_subdir = name.IsEmpty() ? _GP(ResPaths).VoiceDir2 : Path::ConcatPaths(_GP(ResPaths).VoiceDir2, name);
		if (File::IsDirectory(speech_subdir) && !FindFile::OpenFiles(speech_subdir).AtEnd()) {
			Debug::Printf(kDbgMsg_Info, "Optional voice directory is defined: %s", speech_subdir.GetCStr());
			_GP(ResPaths).VoiceAvail = true;
		}
	}

	// Save new resource locations and register asset libraries
	_G(VoicePakName) = name;
	_G(VoiceAssetPath) = name.IsEmpty() ? "" : String::FromFormat("%s/", name.GetCStr());
	_GP(ResPaths).SpeechPak.Name = speech_file;
	_GP(ResPaths).SpeechPak.Path = speech_filepath;
	_GP(ResPaths).VoiceDirSub = speech_subdir;
	_GP(AssetMgr)->AddLibrary(_GP(ResPaths).VoiceDirSub, "voice");
	_GP(AssetMgr)->AddLibrary(_GP(ResPaths).SpeechPak.Path, "voice");
	return _GP(ResPaths).VoiceAvail;
}

String get_voicepak_name() {
	return _G(VoicePakName);
}

String get_voice_assetpath() {
	return _G(VoiceAssetPath);
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

ScriptOverlay *Speech_GetTextOverlay() {
	return const_cast<ScriptOverlay*>((const ScriptOverlay*)ccGetObjectAddressFromHandle(_GP(play).speech_text_schandle));
}

ScriptOverlay *Speech_GetPortraitOverlay() {
	return const_cast<ScriptOverlay*>((const ScriptOverlay*)ccGetObjectAddressFromHandle(_GP(play).speech_face_schandle));
}

int Speech_GetAnimationStopTimeMargin() {
	return _GP(play).close_mouth_speech_time;
}

void Speech_SetAnimationStopTimeMargin(int time) {
	_GP(play).close_mouth_speech_time = time;
}

int Speech_GetCustomPortraitPlacement() {
	return _GP(play).speech_portrait_placement;
}

void Speech_SetCustomPortraitPlacement(int placement) {
	_GP(play).speech_portrait_placement = placement;
}

int Speech_GetDisplayPostTimeMs() {
	return _GP(play).speech_display_post_time_ms;
}

void Speech_SetDisplayPostTimeMs(int time_ms) {
	_GP(play).speech_display_post_time_ms = time_ms;
}

int Speech_GetGlobalSpeechAnimationDelay() {
	return _GP(play).talkanim_speed;
}

void Speech_SetGlobalSpeechAnimationDelay(int delay) {
	if (_GP(game).options[OPT_GLOBALTALKANIMSPD] == 0) {
		debug_script_warn("Speech.GlobalSpeechAnimationDelay cannot be set when global speech animation speed is not enabled; set Speech.UseGlobalSpeechAnimationDelay first!");
		return;
	}
	_GP(play).talkanim_speed = delay;
}

int Speech_GetPortraitXOffset() {
	return _GP(play).speech_portrait_x;
}

void Speech_SetPortraitXOffset(int x) {
	_GP(play).speech_portrait_x = x;
}

int Speech_GetPortraitY() {
	return _GP(play).speech_portrait_y;
}

void Speech_SetPortraitY(int y) {
	_GP(play).speech_portrait_y = y;
}

int Speech_GetStyle() {
	return _GP(game).options[OPT_SPEECHTYPE];
}

int Speech_GetSkipKey() {
	return _GP(play).skip_speech_specific_key;
}

void Speech_SetSkipKey(int key) {
	_GP(play).skip_speech_specific_key = key;
}

int Speech_GetTextAlignment() {
	return _GP(play).speech_text_align;
}

void Speech_SetTextAlignment_Old(int alignment) {
	_GP(play).speech_text_align = ReadScriptAlignment(alignment);
}

void Speech_SetTextAlignment(int alignment) {
	_GP(play).speech_text_align = (HorAlignment)alignment;
}

int Speech_GetUseGlobalSpeechAnimationDelay() {
	return _GP(game).options[OPT_GLOBALTALKANIMSPD];
}

void Speech_SetUseGlobalSpeechAnimationDelay(int delay) {
	_GP(game).options[OPT_GLOBALTALKANIMSPD] = delay;
}

//-----------------------------------------------------------------------------

RuntimeScriptValue Sc_Speech_GetAnimationStopTimeMargin(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Speech_GetAnimationStopTimeMargin);
}

RuntimeScriptValue Sc_Speech_SetAnimationStopTimeMargin(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(Speech_SetAnimationStopTimeMargin);
}

RuntimeScriptValue Sc_Speech_GetCustomPortraitPlacement(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Speech_GetCustomPortraitPlacement);
}

RuntimeScriptValue Sc_Speech_SetCustomPortraitPlacement(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(Speech_SetCustomPortraitPlacement);
}

RuntimeScriptValue Sc_Speech_GetDisplayPostTimeMs(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Speech_GetDisplayPostTimeMs);
}

RuntimeScriptValue Sc_Speech_SetDisplayPostTimeMs(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(Speech_SetDisplayPostTimeMs);
}

RuntimeScriptValue Sc_Speech_GetGlobalSpeechAnimationDelay(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Speech_GetGlobalSpeechAnimationDelay);
}

RuntimeScriptValue Sc_Speech_SetGlobalSpeechAnimationDelay(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(Speech_SetGlobalSpeechAnimationDelay);
}

RuntimeScriptValue Sc_Speech_GetPortraitXOffset(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Speech_GetPortraitXOffset);
}

RuntimeScriptValue Sc_Speech_SetPortraitXOffset(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(Speech_SetPortraitXOffset);
}

RuntimeScriptValue Sc_Speech_GetPortraitY(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Speech_GetPortraitY);
}

RuntimeScriptValue Sc_Speech_SetPortraitY(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(Speech_SetPortraitY);
}

RuntimeScriptValue Sc_Speech_GetStyle(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Speech_GetStyle);
}

extern RuntimeScriptValue Sc_SetSpeechStyle(const RuntimeScriptValue *params, int32_t param_count);

RuntimeScriptValue Sc_Speech_GetSkipKey(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Speech_GetSkipKey);
}

RuntimeScriptValue Sc_Speech_SetSkipKey(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(Speech_SetSkipKey);
}

RuntimeScriptValue Sc_Speech_GetSkipStyle(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(GetSkipSpeech);
}

extern RuntimeScriptValue Sc_SetSkipSpeech(const RuntimeScriptValue *params, int32_t param_count);

RuntimeScriptValue Sc_Speech_GetTextAlignment(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Speech_GetTextAlignment);
}

RuntimeScriptValue Sc_Speech_SetTextAlignment_Old(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(Speech_SetTextAlignment_Old);
}

RuntimeScriptValue Sc_Speech_SetTextAlignment(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(Speech_SetTextAlignment);
}

RuntimeScriptValue Sc_Speech_GetUseGlobalSpeechAnimationDelay(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(Speech_GetUseGlobalSpeechAnimationDelay);
}

RuntimeScriptValue Sc_Speech_SetUseGlobalSpeechAnimationDelay(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_VOID_PINT(Speech_SetUseGlobalSpeechAnimationDelay);
}

RuntimeScriptValue Sc_Speech_GetVoiceMode(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(GetVoiceMode);
}

RuntimeScriptValue Sc_Speech_GetTextOverlay(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO(ScriptOverlay, Speech_GetTextOverlay);
}

RuntimeScriptValue Sc_Speech_GetPortraitOverlay(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_OBJAUTO(ScriptOverlay, Speech_GetPortraitOverlay);
}

extern RuntimeScriptValue Sc_SetVoiceMode(const RuntimeScriptValue *params, int32_t param_count);

void RegisterSpeechAPI(ScriptAPIVersion base_api, ScriptAPIVersion /*compat_api*/) {
	ScFnRegister speech_api[] = {
		{"Speech::get_AnimationStopTimeMargin", API_FN_PAIR(Speech_GetAnimationStopTimeMargin)},
		{"Speech::set_AnimationStopTimeMargin", API_FN_PAIR(Speech_SetAnimationStopTimeMargin)},
		{"Speech::get_CustomPortraitPlacement", API_FN_PAIR(Speech_GetCustomPortraitPlacement)},
		{"Speech::set_CustomPortraitPlacement", API_FN_PAIR(Speech_SetCustomPortraitPlacement)},
		{"Speech::get_DisplayPostTimeMs", API_FN_PAIR(Speech_GetDisplayPostTimeMs)},
		{"Speech::set_DisplayPostTimeMs", API_FN_PAIR(Speech_SetDisplayPostTimeMs)},
		{"Speech::get_GlobalSpeechAnimationDelay", API_FN_PAIR(Speech_GetGlobalSpeechAnimationDelay)},
		{"Speech::set_GlobalSpeechAnimationDelay", API_FN_PAIR(Speech_SetGlobalSpeechAnimationDelay)},
		{"Speech::get_PortraitOverlay", API_FN_PAIR(Speech_GetPortraitOverlay)},
		{"Speech::get_PortraitXOffset", API_FN_PAIR(Speech_GetPortraitXOffset)},
		{"Speech::set_PortraitXOffset", API_FN_PAIR(Speech_SetPortraitXOffset)},
		{"Speech::get_PortraitY", API_FN_PAIR(Speech_GetPortraitY)},
		{"Speech::set_PortraitY", API_FN_PAIR(Speech_SetPortraitY)},
		{"Speech::get_SkipKey", API_FN_PAIR(Speech_GetSkipKey)},
		{"Speech::set_SkipKey", API_FN_PAIR(Speech_SetSkipKey)},
		{"Speech::get_SkipStyle", Sc_Speech_GetSkipStyle},
		{"Speech::set_SkipStyle", API_FN_PAIR(SetSkipSpeech)},
		{"Speech::get_Style", API_FN_PAIR(Speech_GetStyle)},
		{"Speech::set_Style", API_FN_PAIR(SetSpeechStyle)},
		{"Speech::get_TextAlignment", API_FN_PAIR(Speech_GetTextAlignment)},
		{"Speech::get_TextOverlay", API_FN_PAIR(Speech_GetTextOverlay)},
		{"Speech::get_UseGlobalSpeechAnimationDelay", API_FN_PAIR(Speech_GetUseGlobalSpeechAnimationDelay)},
		{"Speech::set_UseGlobalSpeechAnimationDelay", API_FN_PAIR(Speech_SetUseGlobalSpeechAnimationDelay)},
		{"Speech::get_VoiceMode", Sc_Speech_GetVoiceMode},
		{"Speech::set_VoiceMode", API_FN_PAIR(SetVoiceMode)},
	};

	ccAddExternalFunctions361(speech_api);

	// Few functions have to be selected based on API level
	if (base_api < kScriptAPI_v350)
		ccAddExternalStaticFunction361("Speech::set_TextAlignment", API_FN_PAIR(Speech_SetTextAlignment_Old));
	else
		ccAddExternalStaticFunction361("Speech::set_TextAlignment", API_FN_PAIR(Speech_SetTextAlignment));

	/* -- Don't register more unsafe plugin symbols until new plugin interface is designed --*/
}

} // namespace AGS3
