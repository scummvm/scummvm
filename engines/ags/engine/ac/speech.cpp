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
#include "ags/engine/ac/dynobj/cc_dynamic_object.h"
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
	_GP(play).voice_avail = false;
	// FIXME: don't remove the default speech.vox when changing pak, as this causes a crash in Beyond the Edge of Owlsgard
	// Duplicate checks are already present so this shouldn't cause problems but still, it should be looked into
	if (_GP(ResPaths).SpeechPak.Name.CompareNoCase("speech.vox") != 0)
		_GP(AssetMgr)->RemoveLibrary(_GP(ResPaths).SpeechPak.Path);
	_GP(AssetMgr)->RemoveLibrary(_GP(ResPaths).VoiceDirSub);

	// Now check for the new packs and add if they exist
	String speech_filepath = find_assetlib(speech_file);
	if (!speech_filepath.IsEmpty()) {
		Debug::Printf(kDbgMsg_Info, "Voice pack found: %s", speech_file.GetCStr());
		_GP(play).voice_avail = true;
	} else {
		Debug::Printf(kDbgMsg_Error, "Unable to init voice pack '%s', file not found or of unknown format.",
			speech_file.GetCStr());
	}

	String speech_subdir = "";
	if (!_GP(ResPaths).VoiceDir2.IsEmpty() &&
			!_GP(ResPaths).VoiceDir2.IsEmpty() && Path::ComparePaths(_GP(ResPaths).DataDir, _GP(ResPaths).VoiceDir2) != 0) {
		// If we have custom voice directory set, we will enable voice-over even if speech.vox does not exist
		speech_subdir = name.IsEmpty() ? _GP(ResPaths).VoiceDir2 : Path::ConcatPaths(_GP(ResPaths).VoiceDir2, name);
		if (File::IsDirectory(speech_subdir) && !FindFile::OpenFiles(speech_subdir).AtEnd()) {
			Debug::Printf(kDbgMsg_Info, "Optional voice directory is defined: %s", speech_subdir.GetCStr());
			_GP(play).voice_avail = true;
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
	return _GP(play).voice_avail;
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

RuntimeScriptValue Sc_Speech_GetAnimationStopTimeMargin(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARGET_INT(_GP(play).close_mouth_speech_time);
}

RuntimeScriptValue Sc_Speech_SetAnimationStopTimeMargin(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARSET_PINT(_GP(play).close_mouth_speech_time);
}

RuntimeScriptValue Sc_Speech_GetCustomPortraitPlacement(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARGET_INT(_GP(play).speech_portrait_placement);
}

RuntimeScriptValue Sc_Speech_SetCustomPortraitPlacement(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARSET_PINT(_GP(play).speech_portrait_placement);
}

RuntimeScriptValue Sc_Speech_GetDisplayPostTimeMs(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARGET_INT(_GP(play).speech_display_post_time_ms);
}

RuntimeScriptValue Sc_Speech_SetDisplayPostTimeMs(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARSET_PINT(_GP(play).speech_display_post_time_ms);
}

RuntimeScriptValue Sc_Speech_GetGlobalSpeechAnimationDelay(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARGET_INT(_GP(play).talkanim_speed);
}

RuntimeScriptValue Sc_Speech_SetGlobalSpeechAnimationDelay(const RuntimeScriptValue *params, int32_t param_count) {
	if (_GP(game).options[OPT_GLOBALTALKANIMSPD] == 0) {
		debug_script_warn("Speech.GlobalSpeechAnimationDelay cannot be set when global speech animation speed is not enabled; set Speech.UseGlobalSpeechAnimationDelay first!");
		return RuntimeScriptValue();
	}
	API_VARSET_PINT(_GP(play).talkanim_speed);
}

RuntimeScriptValue Sc_Speech_GetPortraitXOffset(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARGET_INT(_GP(play).speech_portrait_x);
}

RuntimeScriptValue Sc_Speech_SetPortraitXOffset(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARSET_PINT(_GP(play).speech_portrait_x);
}

RuntimeScriptValue Sc_Speech_GetPortraitY(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARGET_INT(_GP(play).speech_portrait_y);
}

RuntimeScriptValue Sc_Speech_SetPortraitY(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARSET_PINT(_GP(play).speech_portrait_y);
}

RuntimeScriptValue Sc_Speech_GetStyle(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARGET_INT(_GP(game).options[OPT_SPEECHTYPE]);
}

extern RuntimeScriptValue Sc_SetSpeechStyle(const RuntimeScriptValue *params, int32_t param_count);

RuntimeScriptValue Sc_Speech_GetSkipKey(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARGET_INT(_GP(play).skip_speech_specific_key);
}

RuntimeScriptValue Sc_Speech_SetSkipKey(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARSET_PINT(_GP(play).skip_speech_specific_key);
}

RuntimeScriptValue Sc_Speech_GetSkipStyle(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(GetSkipSpeech);
}

extern RuntimeScriptValue Sc_SetSkipSpeech(const RuntimeScriptValue *params, int32_t param_count);

RuntimeScriptValue Sc_Speech_GetTextAlignment(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARGET_INT(_GP(play).speech_text_align);
}

RuntimeScriptValue Sc_Speech_SetTextAlignment_Old(const RuntimeScriptValue *params, int32_t param_count) {
	ASSERT_VARIABLE_VALUE(_GP(play).speech_text_align);
	_GP(play).speech_text_align = ReadScriptAlignment(params[0].IValue);
	return RuntimeScriptValue();
}

RuntimeScriptValue Sc_Speech_SetTextAlignment(const RuntimeScriptValue *params, int32_t param_count) {
	ASSERT_VARIABLE_VALUE(_GP(play).speech_text_align);
	_GP(play).speech_text_align = (HorAlignment)params[0].IValue;
	return RuntimeScriptValue();
}

RuntimeScriptValue Sc_Speech_GetUseGlobalSpeechAnimationDelay(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARGET_INT(_GP(game).options[OPT_GLOBALTALKANIMSPD]);
}

RuntimeScriptValue Sc_Speech_SetUseGlobalSpeechAnimationDelay(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARSET_PINT(_GP(game).options[OPT_GLOBALTALKANIMSPD]);
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
	ccAddExternalStaticFunction("Speech::get_AnimationStopTimeMargin", Sc_Speech_GetAnimationStopTimeMargin);
	ccAddExternalStaticFunction("Speech::set_AnimationStopTimeMargin", Sc_Speech_SetAnimationStopTimeMargin);
	ccAddExternalStaticFunction("Speech::get_CustomPortraitPlacement", Sc_Speech_GetCustomPortraitPlacement);
	ccAddExternalStaticFunction("Speech::set_CustomPortraitPlacement", Sc_Speech_SetCustomPortraitPlacement);
	ccAddExternalStaticFunction("Speech::get_DisplayPostTimeMs", Sc_Speech_GetDisplayPostTimeMs);
	ccAddExternalStaticFunction("Speech::set_DisplayPostTimeMs", Sc_Speech_SetDisplayPostTimeMs);
	ccAddExternalStaticFunction("Speech::get_GlobalSpeechAnimationDelay", Sc_Speech_GetGlobalSpeechAnimationDelay);
	ccAddExternalStaticFunction("Speech::set_GlobalSpeechAnimationDelay", Sc_Speech_SetGlobalSpeechAnimationDelay);
	ccAddExternalStaticFunction("Speech::get_PortraitOverlay", Sc_Speech_GetPortraitOverlay);
	ccAddExternalStaticFunction("Speech::get_PortraitXOffset", Sc_Speech_GetPortraitXOffset);
	ccAddExternalStaticFunction("Speech::set_PortraitXOffset", Sc_Speech_SetPortraitXOffset);
	ccAddExternalStaticFunction("Speech::get_PortraitY", Sc_Speech_GetPortraitY);
	ccAddExternalStaticFunction("Speech::set_PortraitY", Sc_Speech_SetPortraitY);
	ccAddExternalStaticFunction("Speech::get_SkipKey", Sc_Speech_GetSkipKey);
	ccAddExternalStaticFunction("Speech::set_SkipKey", Sc_Speech_SetSkipKey);
	ccAddExternalStaticFunction("Speech::get_SkipStyle", Sc_Speech_GetSkipStyle);
	ccAddExternalStaticFunction("Speech::set_SkipStyle", Sc_SetSkipSpeech);
	ccAddExternalStaticFunction("Speech::get_Style", Sc_Speech_GetStyle);
	ccAddExternalStaticFunction("Speech::set_Style", Sc_SetSpeechStyle);
	ccAddExternalStaticFunction("Speech::get_TextAlignment", Sc_Speech_GetTextAlignment);
	if (base_api < kScriptAPI_v350)
		ccAddExternalStaticFunction("Speech::set_TextAlignment", Sc_Speech_SetTextAlignment_Old);
	else
		ccAddExternalStaticFunction("Speech::set_TextAlignment", Sc_Speech_SetTextAlignment);
	ccAddExternalStaticFunction("Speech::get_TextOverlay", Sc_Speech_GetTextOverlay);
	ccAddExternalStaticFunction("Speech::get_UseGlobalSpeechAnimationDelay", Sc_Speech_GetUseGlobalSpeechAnimationDelay);
	ccAddExternalStaticFunction("Speech::set_UseGlobalSpeechAnimationDelay", Sc_Speech_SetUseGlobalSpeechAnimationDelay);
	ccAddExternalStaticFunction("Speech::get_VoiceMode", Sc_Speech_GetVoiceMode);
	ccAddExternalStaticFunction("Speech::set_VoiceMode", Sc_SetVoiceMode);

	/* -- Don't register more unsafe plugin symbols until new plugin interface is designed --*/
}

} // namespace AGS3
