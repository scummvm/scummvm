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

#include "ags/engine/media/audio/audio_system.h"
#include "ags/engine/ac/asset_helper.h"
#include "ags/engine/ac/audio_clip.h"
#include "ags/engine/ac/audio_channel.h"
#include "ags/shared/ac/common.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/shared/core/asset_manager.h"
#include "ags/engine/ac/dynobj/cc_audio_channel.h"
#include "ags/engine/script/runtime_script_value.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

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
	return _GP(AssetMgr)->DoesAssetExist(get_audio_clip_assetpath(clip->bundlingType, clip->fileName)) ? 1 : 0;
}

void AudioClip_Stop(ScriptAudioClip *clip) {
	for (int i = NUM_SPEECH_CHANS; i < _GP(game).numGameChannels; i++) {
		auto *ch = AudioChans::GetChannelIfPlaying(i);
		if ((ch != nullptr) && (ch->_sourceClipID == clip->id)) {
			AudioChannel_Stop(&_G(scrAudioChannel)[i]);
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

ScriptAudioChannel *AudioClip_PlayOnChannel(ScriptAudioClip *clip, int chan, int priority, int repeat) {
	if (chan < NUM_SPEECH_CHANS || chan >= _GP(game).numGameChannels)
		quitprintf("!AudioClip.PlayOnChannel: invalid channel %d, the range is %d - %d",
			chan, NUM_SPEECH_CHANS, _GP(game).numGameChannels - 1);
	if (priority == SCR_NO_VALUE)
		priority = clip->defaultPriority;
	if (repeat == SCR_NO_VALUE)
		repeat = clip->defaultRepeat;
	return play_audio_clip_on_channel(chan, clip, priority, repeat, 0);
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

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
	API_OBJCALL_OBJ_PINT2(ScriptAudioClip, ScriptAudioChannel, _GP(ccDynamicAudio), AudioClip_Play);
}

// ScriptAudioChannel* | ScriptAudioClip *clip, int position, int priority, int repeat
RuntimeScriptValue Sc_AudioClip_PlayFrom(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ_PINT3(ScriptAudioClip, ScriptAudioChannel, _GP(ccDynamicAudio), AudioClip_PlayFrom);
}

// ScriptAudioChannel* | ScriptAudioClip *clip, int priority, int repeat
RuntimeScriptValue Sc_AudioClip_PlayQueued(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ_PINT2(ScriptAudioClip, ScriptAudioChannel, _GP(ccDynamicAudio), AudioClip_PlayQueued);
}

RuntimeScriptValue Sc_AudioClip_PlayOnChannel(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ_PINT3(ScriptAudioClip, ScriptAudioChannel, _GP(ccDynamicAudio), AudioClip_PlayOnChannel);
}

void RegisterAudioClipAPI() {
	ccAddExternalObjectFunction("AudioClip::Play^2", Sc_AudioClip_Play);
	ccAddExternalObjectFunction("AudioClip::PlayFrom^3", Sc_AudioClip_PlayFrom);
	ccAddExternalObjectFunction("AudioClip::PlayQueued^2", Sc_AudioClip_PlayQueued);
	ccAddExternalObjectFunction("AudioClip::PlayOnChannel^3", Sc_AudioClip_PlayOnChannel);
	ccAddExternalObjectFunction("AudioClip::Stop^0", Sc_AudioClip_Stop);
	ccAddExternalObjectFunction("AudioClip::get_ID", Sc_AudioClip_GetID);
	ccAddExternalObjectFunction("AudioClip::get_FileType", Sc_AudioClip_GetFileType);
	ccAddExternalObjectFunction("AudioClip::get_IsAvailable", Sc_AudioClip_GetIsAvailable);
	ccAddExternalObjectFunction("AudioClip::get_Type", Sc_AudioClip_GetType);
}

} // namespace AGS3
