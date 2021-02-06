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

#include "ags/engine/ac/audiochannel.h"
#include "ags/engine/ac/gamestate.h"
#include "ags/engine/ac/global_audio.h"
#include "ags/engine/ac/dynobj/cc_audioclip.h"
#include "ags/engine/debugging/debug_log.h"
#include "ags/shared/game/roomstruct.h"
#include "ags/engine/script/runtimescriptvalue.h"
#include "ags/engine/media/audio/audio_system.h"

#include "ags/shared/debugging/out.h"
#include "ags/engine/script/script_api.h"
#include "ags/engine/script/script_runtime.h"

namespace AGS3 {

using namespace AGS::Shared;

extern GameState play;
extern RoomStruct thisroom;
extern CCAudioClip ccDynamicAudioClip;

int AudioChannel_GetID(ScriptAudioChannel *channel) {
	return channel->id;
}

int AudioChannel_GetIsPlaying(ScriptAudioChannel *channel) {
	if (play.fast_forward) {
		return 0;
	}

	return channel_is_playing(channel->id) ? 1 : 0;
}

int AudioChannel_GetPanning(ScriptAudioChannel *channel) {
	AudioChannelsLock lock;
	auto *ch = lock.GetChannelIfPlaying(channel->id);

	if (ch) {
		return ch->_panningAsPercentage;
	}
	return 0;
}

void AudioChannel_SetPanning(ScriptAudioChannel *channel, int newPanning) {
	if ((newPanning < -100) || (newPanning > 100))
		quitprintf("!AudioChannel.Panning: panning value must be between -100 and 100 (passed=%d)", newPanning);

	AudioChannelsLock lock;
	auto *ch = lock.GetChannelIfPlaying(channel->id);

	if (ch) {
		ch->set_panning(((newPanning + 100) * 255) / 200);
		ch->_panningAsPercentage = newPanning;
	}
}

ScriptAudioClip *AudioChannel_GetPlayingClip(ScriptAudioChannel *channel) {
	AudioChannelsLock lock;
	auto *ch = lock.GetChannelIfPlaying(channel->id);

	if (ch) {
		return (ScriptAudioClip *)ch->_sourceClip;
	}
	return nullptr;
}

int AudioChannel_GetPosition(ScriptAudioChannel *channel) {
	AudioChannelsLock lock;
	auto *ch = lock.GetChannelIfPlaying(channel->id);

	if (ch) {
		if (play.fast_forward)
			return 999999999;

		return ch->get_pos();
	}
	return 0;
}

int AudioChannel_GetPositionMs(ScriptAudioChannel *channel) {
	AudioChannelsLock lock;
	auto *ch = lock.GetChannelIfPlaying(channel->id);

	if (ch) {
		if (play.fast_forward)
			return 999999999;

		return ch->get_pos_ms();
	}
	return 0;
}

int AudioChannel_GetLengthMs(ScriptAudioChannel *channel) {
	AudioChannelsLock lock;
	auto *ch = lock.GetChannelIfPlaying(channel->id);

	if (ch) {
		return ch->get_length_ms();
	}
	return 0;
}

int AudioChannel_GetVolume(ScriptAudioChannel *channel) {
	AudioChannelsLock lock;
	auto *ch = lock.GetChannelIfPlaying(channel->id);

	if (ch) {
		return ch->get_volume();
	}
	return 0;
}

int AudioChannel_SetVolume(ScriptAudioChannel *channel, int newVolume) {
	if ((newVolume < 0) || (newVolume > 100))
		quitprintf("!AudioChannel.Volume: new value out of range (supplied: %d, range: 0..100)", newVolume);

	AudioChannelsLock lock;
	auto *ch = lock.GetChannelIfPlaying(channel->id);

	if (ch) {
		ch->set_volume_percent(newVolume);
	}
	return 0;
}

int AudioChannel_GetSpeed(ScriptAudioChannel *channel) {
	AudioChannelsLock lock;
	auto *ch = lock.GetChannelIfPlaying(channel->id);

	if (ch) {
		return ch->get_speed();
	}
	return 0;
}

void AudioChannel_SetSpeed(ScriptAudioChannel *channel, int new_speed) {
	AudioChannelsLock lock;
	auto *ch = lock.GetChannelIfPlaying(channel->id);

	if (ch) {
		ch->set_speed(new_speed);
	}
}

void AudioChannel_Stop(ScriptAudioChannel *channel) {
	if (channel->id == SCHAN_SPEECH && play.IsNonBlockingVoiceSpeech())
		stop_voice_nonblocking();
	else
		stop_or_fade_out_channel(channel->id, -1, nullptr);
}

void AudioChannel_Seek(ScriptAudioChannel *channel, int newPosition) {
	if (newPosition < 0)
		quitprintf("!AudioChannel.Seek: invalid seek position %d", newPosition);

	AudioChannelsLock lock;
	auto *ch = lock.GetChannelIfPlaying(channel->id);

	if (ch) {
		ch->seek(newPosition);
	}
}

void AudioChannel_SetRoomLocation(ScriptAudioChannel *channel, int xPos, int yPos) {
	AudioChannelsLock lock;
	auto *ch = lock.GetChannelIfPlaying(channel->id);

	if (ch) {
		int maxDist = ((xPos > thisroom.Width / 2) ? xPos : (thisroom.Width - xPos)) - AMBIENCE_FULL_DIST;
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
	API_OBJCALL_OBJ(ScriptAudioChannel, ScriptAudioClip, ccDynamicAudioClip, AudioChannel_GetPlayingClip);
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

void RegisterAudioChannelAPI() {
	ccAddExternalObjectFunction("AudioChannel::Seek^1", Sc_AudioChannel_Seek);
	ccAddExternalObjectFunction("AudioChannel::SetRoomLocation^2", Sc_AudioChannel_SetRoomLocation);
	ccAddExternalObjectFunction("AudioChannel::Stop^0", Sc_AudioChannel_Stop);
	ccAddExternalObjectFunction("AudioChannel::get_ID", Sc_AudioChannel_GetID);
	ccAddExternalObjectFunction("AudioChannel::get_IsPlaying", Sc_AudioChannel_GetIsPlaying);
	ccAddExternalObjectFunction("AudioChannel::get_LengthMs", Sc_AudioChannel_GetLengthMs);
	ccAddExternalObjectFunction("AudioChannel::get_Panning", Sc_AudioChannel_GetPanning);
	ccAddExternalObjectFunction("AudioChannel::set_Panning", Sc_AudioChannel_SetPanning);
	ccAddExternalObjectFunction("AudioChannel::get_PlayingClip", Sc_AudioChannel_GetPlayingClip);
	ccAddExternalObjectFunction("AudioChannel::get_Position", Sc_AudioChannel_GetPosition);
	ccAddExternalObjectFunction("AudioChannel::get_PositionMs", Sc_AudioChannel_GetPositionMs);
	ccAddExternalObjectFunction("AudioChannel::get_Volume", Sc_AudioChannel_GetVolume);
	ccAddExternalObjectFunction("AudioChannel::set_Volume", Sc_AudioChannel_SetVolume);
	ccAddExternalObjectFunction("AudioChannel::get_Speed", Sc_AudioChannel_GetSpeed);
	ccAddExternalObjectFunction("AudioChannel::set_Speed", Sc_AudioChannel_SetSpeed);
	// For compatibility with  Ahmet Kamil's (aka Gord10) custom engine
	ccAddExternalObjectFunction("AudioChannel::SetSpeed^1", Sc_AudioChannel_SetSpeed);

	/* ----------------------- Registering unsafe exports for plugins -----------------------*/

	ccAddExternalFunctionForPlugin("AudioChannel::Seek^1", (void *)AudioChannel_Seek);
	ccAddExternalFunctionForPlugin("AudioChannel::SetRoomLocation^2", (void *)AudioChannel_SetRoomLocation);
	ccAddExternalFunctionForPlugin("AudioChannel::Stop^0", (void *)AudioChannel_Stop);
	ccAddExternalFunctionForPlugin("AudioChannel::get_ID", (void *)AudioChannel_GetID);
	ccAddExternalFunctionForPlugin("AudioChannel::get_IsPlaying", (void *)AudioChannel_GetIsPlaying);
	ccAddExternalFunctionForPlugin("AudioChannel::get_LengthMs", (void *)AudioChannel_GetLengthMs);
	ccAddExternalFunctionForPlugin("AudioChannel::get_Panning", (void *)AudioChannel_GetPanning);
	ccAddExternalFunctionForPlugin("AudioChannel::set_Panning", (void *)AudioChannel_SetPanning);
	ccAddExternalFunctionForPlugin("AudioChannel::get_PlayingClip", (void *)AudioChannel_GetPlayingClip);
	ccAddExternalFunctionForPlugin("AudioChannel::get_Position", (void *)AudioChannel_GetPosition);
	ccAddExternalFunctionForPlugin("AudioChannel::get_PositionMs", (void *)AudioChannel_GetPositionMs);
	ccAddExternalFunctionForPlugin("AudioChannel::get_Volume", (void *)AudioChannel_GetVolume);
	ccAddExternalFunctionForPlugin("AudioChannel::set_Volume", (void *)AudioChannel_SetVolume);
}

} // namespace AGS3
