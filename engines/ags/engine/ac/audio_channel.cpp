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

#include "ags/engine/ac/audio_channel.h"
#include "ags/engine/ac/game_state.h"
#include "ags/engine/ac/global_audio.h"
#include "ags/engine/ac/dynobj/cc_audio_clip.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/game/room_struct.h"
#include "ags/engine/script/runtime_script_value.h"
#include "ags/engine/media/audio/audio_system.h"
#include "ags/shared/ac/game_setup_struct.h"
#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"
#include "ags/globals.h"

namespace AGS3 {

using namespace AGS::Shared;

int AudioChannel_GetID(ScriptAudioChannel *channel) {
	return channel->id;
}

int AudioChannel_GetIsPlaying(ScriptAudioChannel *channel) {
	if (_GP(play).fast_forward) {
		return 0;
	}

	return AudioChans::ChannelIsPlaying(channel->id) ? 1 : 0;
}

bool AudioChannel_GetIsPaused(ScriptAudioChannel *channel) {
	auto *ch = AudioChans::GetChannelIfPlaying(channel->id);

	if (ch) return ch->is_paused();
	return false;
}

int AudioChannel_GetPanning(ScriptAudioChannel *channel) {
	auto *ch = AudioChans::GetChannelIfPlaying(channel->id);

	if (ch) {
		return ch->get_panning();
	}
	return 0;
}

void AudioChannel_SetPanning(ScriptAudioChannel *channel, int newPanning) {
	if ((newPanning < -100) || (newPanning > 100))
		quitprintf("!AudioChannel.Panning: panning value must be between -100 and 100 (passed=%d)", newPanning);

	auto *ch = AudioChans::GetChannelIfPlaying(channel->id);

	if (ch) {
		ch->set_panning(newPanning);
	}
}

ScriptAudioClip *AudioChannel_GetPlayingClip(ScriptAudioChannel *channel) {
	auto *ch = AudioChans::GetChannelIfPlaying(channel->id);

	if (ch && ch->_sourceClipID >= 0) {
		return &_GP(game).audioClips[ch->_sourceClipID];
	}
	return nullptr;
}

int AudioChannel_GetPosition(ScriptAudioChannel *channel) {
	auto *ch = AudioChans::GetChannelIfPlaying(channel->id);

	if (ch) {
		if (_GP(play).fast_forward)
			return 999999999;

		return ch->get_pos();
	}
	return 0;
}

int AudioChannel_GetPositionMs(ScriptAudioChannel *channel) {
	auto *ch = AudioChans::GetChannelIfPlaying(channel->id);

	if (ch) {
		if (_GP(play).fast_forward)
			return 999999999;

		return ch->get_pos_ms();
	}
	return 0;
}

int AudioChannel_GetLengthMs(ScriptAudioChannel *channel) {
	auto *ch = AudioChans::GetChannelIfPlaying(channel->id);

	if (ch) {
		return ch->get_length_ms();
	}
	return 0;
}

int AudioChannel_GetVolume(ScriptAudioChannel *channel) {
	auto *ch = AudioChans::GetChannelIfPlaying(channel->id);

	if (ch) {
		return ch->get_volume100();
	}
	return 0;
}

int AudioChannel_SetVolume(ScriptAudioChannel *channel, int newVolume) {
	if ((newVolume < 0) || (newVolume > 100))
		quitprintf("!AudioChannel.Volume: new value out of range (supplied: %d, range: 0..100)", newVolume);

	auto *ch = AudioChans::GetChannelIfPlaying(channel->id);

	if (ch) {
		ch->set_volume100(newVolume);
	}
	return 0;
}

int AudioChannel_GetSpeed(ScriptAudioChannel *channel) {
	auto *ch = AudioChans::GetChannelIfPlaying(channel->id);

	if (ch) {
		return ch->get_speed();
	}
	return 0;
}

void AudioChannel_SetSpeed(ScriptAudioChannel *channel, int new_speed) {
	auto *ch = AudioChans::GetChannelIfPlaying(channel->id);

	if (ch) {
		ch->set_speed(new_speed);
	}
}

void AudioChannel_Stop(ScriptAudioChannel *channel) {
	if (channel->id == SCHAN_SPEECH && _GP(play).IsNonBlockingVoiceSpeech())
		stop_voice_nonblocking();
	else
		stop_or_fade_out_channel(channel->id, -1, nullptr);
}

void AudioChannel_Pause(ScriptAudioChannel *channel) {
	auto *ch = AudioChans::GetChannelIfPlaying(channel->id);
	if (ch) ch->pause();
}

void AudioChannel_Resume(ScriptAudioChannel *channel) {
	auto *ch = AudioChans::GetChannelIfPlaying(channel->id);
	if (ch) ch->resume();
}

void AudioChannel_Seek(ScriptAudioChannel *channel, int newPosition) {
	if (newPosition < 0)
		quitprintf("!AudioChannel.Seek: invalid seek position %d", newPosition);

	auto *ch = AudioChans::GetChannelIfPlaying(channel->id);
	if (ch)
		ch->seek(newPosition);
}

void AudioChannel_SeekMs(ScriptAudioChannel *channel, int newPosition) {
	if (newPosition < 0)
		quitprintf("!AudioChannel.SeekMs: invalid seek position %d", newPosition);

	auto* ch = AudioChans::GetChannelIfPlaying(channel->id);
	if (ch)
		ch->seek_ms(newPosition);
}

void AudioChannel_SetRoomLocation(ScriptAudioChannel *channel, int xPos, int yPos) {
	auto *ch = AudioChans::GetChannelIfPlaying(channel->id);

	if (ch) {
		int maxDist = ((xPos > _GP(thisroom).Width / 2) ? xPos : (_GP(thisroom).Width - xPos)) - AMBIENCE_FULL_DIST;
		ch->_xSource = (xPos > 0) ? xPos : -1;
		ch->_ySource = yPos;
		ch->_maximumPossibleDistanceAway = maxDist;
		if (xPos > 0) {
			update_directional_sound_vol();
		} else {
			ch->apply_directional_modifier(0);
		}
	}
}

//=============================================================================
//
// Script API Functions
//
//=============================================================================

// int | ScriptAudioChannel *channel
RuntimeScriptValue Sc_AudioChannel_GetID(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptAudioChannel, AudioChannel_GetID);
}

// int | ScriptAudioChannel *channel
RuntimeScriptValue Sc_AudioChannel_GetIsPlaying(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptAudioChannel, AudioChannel_GetIsPlaying);
}

// int | ScriptAudioChannel *channel
RuntimeScriptValue Sc_AudioChannel_GetPanning(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptAudioChannel, AudioChannel_GetPanning);
}

// void | ScriptAudioChannel *channel, int newPanning
RuntimeScriptValue Sc_AudioChannel_SetPanning(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptAudioChannel, AudioChannel_SetPanning);
}

// ScriptAudioClip* | ScriptAudioChannel *channel
RuntimeScriptValue Sc_AudioChannel_GetPlayingClip(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_OBJ(ScriptAudioChannel, ScriptAudioClip, _GP(ccDynamicAudioClip), AudioChannel_GetPlayingClip);
}

// int | ScriptAudioChannel *channel
RuntimeScriptValue Sc_AudioChannel_GetPosition(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptAudioChannel, AudioChannel_GetPosition);
}

// int | ScriptAudioChannel *channel
RuntimeScriptValue Sc_AudioChannel_GetPositionMs(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptAudioChannel, AudioChannel_GetPositionMs);
}

// int | ScriptAudioChannel *channel
RuntimeScriptValue Sc_AudioChannel_GetLengthMs(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptAudioChannel, AudioChannel_GetLengthMs);
}

// int | ScriptAudioChannel *channel
RuntimeScriptValue Sc_AudioChannel_GetVolume(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptAudioChannel, AudioChannel_GetVolume);
}

// int | ScriptAudioChannel *channel, int newVolume
RuntimeScriptValue Sc_AudioChannel_SetVolume(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT_PINT(ScriptAudioChannel, AudioChannel_SetVolume);
}

// void | ScriptAudioChannel *channel
RuntimeScriptValue Sc_AudioChannel_Stop(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(ScriptAudioChannel, AudioChannel_Stop);
}

// void | ScriptAudioChannel *channel, int newPosition
RuntimeScriptValue Sc_AudioChannel_Seek(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptAudioChannel, AudioChannel_Seek);
}

RuntimeScriptValue Sc_AudioChannel_SeekMs(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptAudioChannel, AudioChannel_SeekMs);
}

// void | ScriptAudioChannel *channel, int xPos, int yPos
RuntimeScriptValue Sc_AudioChannel_SetRoomLocation(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT2(ScriptAudioChannel, AudioChannel_SetRoomLocation);
}

RuntimeScriptValue Sc_AudioChannel_GetSpeed(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_INT(ScriptAudioChannel, AudioChannel_GetSpeed);
}

RuntimeScriptValue Sc_AudioChannel_SetSpeed(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID_PINT(ScriptAudioChannel, AudioChannel_SetSpeed);
}

RuntimeScriptValue Sc_AudioChannel_Pause(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(ScriptAudioChannel, AudioChannel_Pause);
}

RuntimeScriptValue Sc_AudioChannel_Resume(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_VOID(ScriptAudioChannel, AudioChannel_Resume);
}

RuntimeScriptValue Sc_AudioChannel_GetIsPaused(void *self, const RuntimeScriptValue *params, int32_t param_count) {
	API_OBJCALL_BOOL(ScriptAudioChannel, AudioChannel_GetIsPaused);
}

void RegisterAudioChannelAPI() {
	ScFnRegister audiochan_api[] = {
		{"AudioChannel::Pause^0", API_FN_PAIR(AudioChannel_Pause)},
		{"AudioChannel::Resume^0", API_FN_PAIR(AudioChannel_Resume)},
		{"AudioChannel::Seek^1", API_FN_PAIR(AudioChannel_Seek)},
		{"AudioChannel::SeekMs^1", API_FN_PAIR(AudioChannel_SeekMs)},
		{"AudioChannel::SetRoomLocation^2", API_FN_PAIR(AudioChannel_SetRoomLocation)},
		{"AudioChannel::Stop^0", API_FN_PAIR(AudioChannel_Stop)},
		{"AudioChannel::get_ID", API_FN_PAIR(AudioChannel_GetID)},
		{"AudioChannel::get_IsPaused", API_FN_PAIR(AudioChannel_GetIsPaused)},
		{"AudioChannel::get_IsPlaying", API_FN_PAIR(AudioChannel_GetIsPlaying)},
		{"AudioChannel::get_LengthMs", API_FN_PAIR(AudioChannel_GetLengthMs)},
		{"AudioChannel::get_Panning", API_FN_PAIR(AudioChannel_GetPanning)},
		{"AudioChannel::set_Panning", API_FN_PAIR(AudioChannel_SetPanning)},
		{"AudioChannel::get_PlayingClip", API_FN_PAIR(AudioChannel_GetPlayingClip)},
		{"AudioChannel::get_Position", API_FN_PAIR(AudioChannel_GetPosition)},
		{"AudioChannel::get_PositionMs", API_FN_PAIR(AudioChannel_GetPositionMs)},
		{"AudioChannel::get_Volume", API_FN_PAIR(AudioChannel_GetVolume)},
		{"AudioChannel::set_Volume", API_FN_PAIR(AudioChannel_SetVolume)},
		{"AudioChannel::get_Speed", API_FN_PAIR(AudioChannel_GetSpeed)},
		{"AudioChannel::set_Speed", API_FN_PAIR(AudioChannel_SetSpeed)},
		// For compatibility with  Ahmet Kamil's (aka Gord10) custom engine
		{"AudioChannel::SetSpeed^1", API_FN_PAIR(AudioChannel_SetSpeed)},
	};

	ccAddExternalFunctions361(audiochan_api);
}

} // namespace AGS3
