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
#include "ags/engine/ac/runtime_defines.h"
#include "ags/engine/ac/speech.h"
#include "ags/engine/debugging/debug_log.h"

#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/engine/ac/gamestate.h"
#include "ags/engine/ac/global_audio.h"
#include "ags/engine/ac/global_display.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"

namespace AGS3 {

int user_to_internal_skip_speech(SkipSpeechStyle userval) {
	switch (userval) {
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
		return 0;
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
	return kSkipSpeechUndefined;
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

extern GameSetupStruct game;
extern GameState play;

RuntimeScriptValue Sc_Speech_GetAnimationStopTimeMargin(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARGET_INT(play.close_mouth_speech_time);
}

RuntimeScriptValue Sc_Speech_SetAnimationStopTimeMargin(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARSET_PINT(play.close_mouth_speech_time);
}

RuntimeScriptValue Sc_Speech_GetCustomPortraitPlacement(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARGET_INT(play.speech_portrait_placement);
}

RuntimeScriptValue Sc_Speech_SetCustomPortraitPlacement(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARSET_PINT(play.speech_portrait_placement);
}

RuntimeScriptValue Sc_Speech_GetDisplayPostTimeMs(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARGET_INT(play.speech_display_post_time_ms);
}

RuntimeScriptValue Sc_Speech_SetDisplayPostTimeMs(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARSET_PINT(play.speech_display_post_time_ms);
}

RuntimeScriptValue Sc_Speech_GetGlobalSpeechAnimationDelay(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARGET_INT(play.talkanim_speed);
}

RuntimeScriptValue Sc_Speech_SetGlobalSpeechAnimationDelay(const RuntimeScriptValue *params, int32_t param_count) {
	if (game.options[OPT_GLOBALTALKANIMSPD] == 0) {
		debug_script_warn("Speech.GlobalSpeechAnimationDelay cannot be set when global speech animation speed is not enabled; set Speech.UseGlobalSpeechAnimationDelay first!");
		return RuntimeScriptValue();
	}
	API_VARSET_PINT(play.talkanim_speed);
}

RuntimeScriptValue Sc_Speech_GetPortraitXOffset(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARGET_INT(play.speech_portrait_x);
}

RuntimeScriptValue Sc_Speech_SetPortraitXOffset(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARSET_PINT(play.speech_portrait_x);
}

RuntimeScriptValue Sc_Speech_GetPortraitY(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARGET_INT(play.speech_portrait_y);
}

RuntimeScriptValue Sc_Speech_SetPortraitY(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARSET_PINT(play.speech_portrait_y);
}

RuntimeScriptValue Sc_Speech_GetStyle(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARGET_INT(game.options[OPT_SPEECHTYPE]);
}

extern RuntimeScriptValue Sc_SetSpeechStyle(const RuntimeScriptValue *params, int32_t param_count);

RuntimeScriptValue Sc_Speech_GetSkipKey(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARGET_INT(play.skip_speech_specific_key);
}

RuntimeScriptValue Sc_Speech_SetSkipKey(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARSET_PINT(play.skip_speech_specific_key);
}

RuntimeScriptValue Sc_Speech_GetSkipStyle(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(GetSkipSpeech);
}

extern RuntimeScriptValue Sc_SetSkipSpeech(const RuntimeScriptValue *params, int32_t param_count);

RuntimeScriptValue Sc_Speech_GetTextAlignment(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARGET_INT(play.speech_text_align);
}

RuntimeScriptValue Sc_Speech_SetTextAlignment_Old(const RuntimeScriptValue *params, int32_t param_count) {
	ASSERT_VARIABLE_VALUE(play.speech_text_align);
	play.speech_text_align = ReadScriptAlignment(params[0].IValue);
	return RuntimeScriptValue();
}

RuntimeScriptValue Sc_Speech_SetTextAlignment(const RuntimeScriptValue *params, int32_t param_count) {
	ASSERT_VARIABLE_VALUE(play.speech_text_align);
	play.speech_text_align = (HorAlignment)params[0].IValue;
	return RuntimeScriptValue();
}

RuntimeScriptValue Sc_Speech_GetUseGlobalSpeechAnimationDelay(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARGET_INT(game.options[OPT_GLOBALTALKANIMSPD]);
}

RuntimeScriptValue Sc_Speech_SetUseGlobalSpeechAnimationDelay(const RuntimeScriptValue *params, int32_t param_count) {
	API_VARSET_PINT(game.options[OPT_GLOBALTALKANIMSPD]);
}

RuntimeScriptValue Sc_Speech_GetVoiceMode(const RuntimeScriptValue *params, int32_t param_count) {
	API_SCALL_INT(GetVoiceMode);
}

extern RuntimeScriptValue Sc_SetVoiceMode(const RuntimeScriptValue *params, int32_t param_count);

void RegisterSpeechAPI(ScriptAPIVersion base_api, ScriptAPIVersion compat_api) {
	ccAddExternalStaticFunction("Speech::get_AnimationStopTimeMargin", Sc_Speech_GetAnimationStopTimeMargin);
	ccAddExternalStaticFunction("Speech::set_AnimationStopTimeMargin", Sc_Speech_SetAnimationStopTimeMargin);
	ccAddExternalStaticFunction("Speech::get_CustomPortraitPlacement", Sc_Speech_GetCustomPortraitPlacement);
	ccAddExternalStaticFunction("Speech::set_CustomPortraitPlacement", Sc_Speech_SetCustomPortraitPlacement);
	ccAddExternalStaticFunction("Speech::get_DisplayPostTimeMs", Sc_Speech_GetDisplayPostTimeMs);
	ccAddExternalStaticFunction("Speech::set_DisplayPostTimeMs", Sc_Speech_SetDisplayPostTimeMs);
	ccAddExternalStaticFunction("Speech::get_GlobalSpeechAnimationDelay", Sc_Speech_GetGlobalSpeechAnimationDelay);
	ccAddExternalStaticFunction("Speech::set_GlobalSpeechAnimationDelay", Sc_Speech_SetGlobalSpeechAnimationDelay);
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
	ccAddExternalStaticFunction("Speech::get_UseGlobalSpeechAnimationDelay", Sc_Speech_GetUseGlobalSpeechAnimationDelay);
	ccAddExternalStaticFunction("Speech::set_UseGlobalSpeechAnimationDelay", Sc_Speech_SetUseGlobalSpeechAnimationDelay);
	ccAddExternalStaticFunction("Speech::get_VoiceMode", Sc_Speech_GetVoiceMode);
	ccAddExternalStaticFunction("Speech::set_VoiceMode", Sc_SetVoiceMode);

	/* -- Don't register more unsafe plugin symbols until new plugin interface is designed --*/
}

} // namespace AGS3
