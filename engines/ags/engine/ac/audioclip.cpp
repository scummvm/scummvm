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

#include "ags/engine/ac/asset_helper.h"
#include "ags/engine/ac/audioclip.h"
#include "ags/engine/ac/audiochannel.h"
#include "ags/shared/ac/gamesetupstruct.h"
#include "ags/engine/script/runtimescriptvalue.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/engine/ac/dynobj/cc_audiochannel.h"
#include "ags/engine/media/audio/audio_system.h"

namespace AGS3 {

extern GameSetupStruct game;
extern ScriptAudioChannel scrAudioChannel[MAX_SOUND_CHANNELS + 1];
extern CCAudioChannel ccDynamicAudio;

int AudioClip_GetID(ScriptAudioClip *clip) {
	return clip->id;
}

int AudioClip_GetFileType(ScriptAudioClip *clip) {
	return clip->fileType;
}

int AudioClip_GetType(ScriptAudioClip *clip) {
	return clip->type;
}
int AudioClip_GetIsAvailable(ScriptAudioClip *clip) {
	return DoesAssetExistInLib(get_audio_clip_assetpath(clip->bundlingType, clip->fileName)) ? 1 : 0;
}

void AudioClip_Stop(ScriptAudioClip *clip) {
	AudioChannelsLock lock;
	for (int i = 0; i < MAX_SOUND_CHANNELS; i++) {
		auto *ch = lock.GetChannelIfPlaying(i);
		if ((ch != nullptr) && (ch->_sourceClip == clip)) {
			AudioChannel_Stop(&scrAudioChannel[i]);
		}
	}
}

ScriptAudioChannel *AudioClip_Play(ScriptAudioClip *clip, int priority, int repeat) {
	ScriptAudioChannel *sc_ch = play_audio_clip(clip, priority, repeat, 0, false);
	return sc_ch;
}

ScriptAudioChannel *AudioClip_PlayFrom(ScriptAudioClip *clip, int position, int priority, int repeat) {
	ScriptAudioChannel *sc_ch = play_audio_clip(clip, priority, repeat, position, false);
	return sc_ch;
}

ScriptAudioChannel *AudioClip_PlayQueued(ScriptAudioClip *clip, int priority, int repeat) {
	ScriptAudioChannel *sc_ch = play_audio_clip(clip, priority, repeat, 0, true);
	return sc_ch;
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"

RuntimeScriptValue Sc_AudioClip_GetID(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptAudioClip, AudioClip_GetID);
}

// int | ScriptAudioClip *clip
RuntimeScriptValue Sc_AudioClip_GetFileType(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptAudioClip, AudioClip_GetFileType);
}

// int | ScriptAudioClip *clip
RuntimeScriptValue Sc_AudioClip_GetType(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptAudioClip, AudioClip_GetType);
}

// int | ScriptAudioClip *clip
RuntimeScriptValue Sc_AudioClip_GetIsAvailable(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptAudioClip, AudioClip_GetIsAvailable);
}

// void | ScriptAudioClip *clip
RuntimeScriptValue Sc_AudioClip_Stop(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(ScriptAudioClip, AudioClip_Stop);
}

// ScriptAudioChannel* | ScriptAudioClip *clip, int priority, int repeat
RuntimeScriptValue Sc_AudioClip_Play(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ_PINT2(ScriptAudioClip, ScriptAudioChannel, ccDynamicAudio, AudioClip_Play);
}

// ScriptAudioChannel* | ScriptAudioClip *clip, int position, int priority, int repeat
RuntimeScriptValue Sc_AudioClip_PlayFrom(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ_PINT3(ScriptAudioClip, ScriptAudioChannel, ccDynamicAudio, AudioClip_PlayFrom);
}

// ScriptAudioChannel* | ScriptAudioClip *clip, int priority, int repeat
RuntimeScriptValue Sc_AudioClip_PlayQueued(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ_PINT2(ScriptAudioClip, ScriptAudioChannel, ccDynamicAudio, AudioClip_PlayQueued);
}

void RegisterAudioClipAPI() {
	ccAddExternalObjectFunction("AudioClip::Play^2", Sc_AudioClip_Play);
	ccAddExternalObjectFunction("AudioClip::PlayFrom^3", Sc_AudioClip_PlayFrom);
	ccAddExternalObjectFunction("AudioClip::PlayQueued^2", Sc_AudioClip_PlayQueued);
	ccAddExternalObjectFunction("AudioClip::Stop^0", Sc_AudioClip_Stop);
	ccAddExternalObjectFunction("AudioClip::get_ID", Sc_AudioClip_GetID);
	ccAddExternalObjectFunction("AudioClip::get_FileType", Sc_AudioClip_GetFileType);
	ccAddExternalObjectFunction("AudioClip::get_IsAvailable", Sc_AudioClip_GetIsAvailable);
	ccAddExternalObjectFunction("AudioClip::get_Type", Sc_AudioClip_GetType);

	/* ----------------------- Registering unsafe exports for plugins -----------------------*/

	ccAddExternalFunctionForPlugin("AudioClip::Play^2", (void *)AudioClip_Play);
	ccAddExternalFunctionForPlugin("AudioClip::PlayFrom^3", (void *)AudioClip_PlayFrom);
	ccAddExternalFunctionForPlugin("AudioClip::PlayQueued^2", (void *)AudioClip_PlayQueued);
	ccAddExternalFunctionForPlugin("AudioClip::Stop^0", (void *)AudioClip_Stop);
	ccAddExternalFunctionForPlugin("AudioClip::get_FileType", (void *)AudioClip_GetFileType);
	ccAddExternalFunctionForPlugin("AudioClip::get_IsAvailable", (void *)AudioClip_GetIsAvailable);
	ccAddExternalFunctionForPlugin("AudioClip::get_Type", (void *)AudioClip_GetType);
}

} // namespace AGS3
