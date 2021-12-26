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

#include "ags/plugins/core/audio_channel.h"
#include "ags/engine/ac/audio_channel.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void AudioChannel::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(AudioChannel::Seek^1, AudioChannel::Seek);
	SCRIPT_METHOD(AudioChannel::SetRoomLocation^2, AudioChannel::SetRoomLocation);
	SCRIPT_METHOD(AudioChannel::Stop^0, AudioChannel::Stop);
	SCRIPT_METHOD(AudioChannel::get_ID, AudioChannel::GetID);
	SCRIPT_METHOD(AudioChannel::get_IsPlaying, AudioChannel::GetIsPlaying);
	SCRIPT_METHOD(AudioChannel::get_LengthMs, AudioChannel::GetLengthMs);
	SCRIPT_METHOD(AudioChannel::get_Panning, AudioChannel::GetPanning);
	SCRIPT_METHOD(AudioChannel::set_Panning, AudioChannel::SetPanning);
	SCRIPT_METHOD(AudioChannel::get_PlayingClip, AudioChannel::GetPlayingClip);
	SCRIPT_METHOD(AudioChannel::get_Position, AudioChannel::GetPosition);
	SCRIPT_METHOD(AudioChannel::get_PositionMs, AudioChannel::GetPositionMs);
	SCRIPT_METHOD(AudioChannel::get_Volume, AudioChannel::GetVolume);
	SCRIPT_METHOD(AudioChannel::set_Volume, AudioChannel::SetVolume);
}

void AudioChannel::Seek(ScriptMethodParams &params) {
	PARAMS2(ScriptAudioChannel *, channel, int, newPosition);
	AGS3::AudioChannel_Seek(channel, newPosition);
}

void AudioChannel::SetRoomLocation(ScriptMethodParams &params) {
	PARAMS3(ScriptAudioChannel *, channel, int, xPos, int, yPos);
	AGS3::AudioChannel_SetRoomLocation(channel, xPos, yPos);
}

void AudioChannel::Stop(ScriptMethodParams &params) {
	PARAMS1(ScriptAudioChannel *, channel);
	AGS3::AudioChannel_Stop(channel);
}

void AudioChannel::GetID(ScriptMethodParams &params) {
	PARAMS1(ScriptAudioChannel *, channel);
	params._result = AGS3::AudioChannel_GetID(channel);
}

void AudioChannel::GetIsPlaying(ScriptMethodParams &params) {
	PARAMS1(ScriptAudioChannel *, channel);
	params._result = AGS3::AudioChannel_GetIsPlaying(channel);
}

void AudioChannel::GetLengthMs(ScriptMethodParams &params) {
	PARAMS1(ScriptAudioChannel *, channel);
	params._result = AGS3::AudioChannel_GetLengthMs(channel);
}

void AudioChannel::GetPanning(ScriptMethodParams &params) {
	PARAMS1(ScriptAudioChannel *, channel);
	params._result = AGS3::AudioChannel_GetPanning(channel);
}

void AudioChannel::SetPanning(ScriptMethodParams &params) {
	PARAMS2(ScriptAudioChannel *, channel, int, newPanning);
	AGS3::AudioChannel_SetPanning(channel, newPanning);
}

void AudioChannel::GetPlayingClip(ScriptMethodParams &params) {
	PARAMS1(ScriptAudioChannel *, channel);
	params._result = AGS3::AudioChannel_GetPlayingClip(channel);
}

void AudioChannel::GetPosition(ScriptMethodParams &params) {
	PARAMS1(ScriptAudioChannel *, channel);
	params._result = AGS3::AudioChannel_GetPosition(channel);
}

void AudioChannel::GetPositionMs(ScriptMethodParams &params) {
	PARAMS1(ScriptAudioChannel *, channel);
	params._result = AGS3::AudioChannel_GetPositionMs(channel);
}

void AudioChannel::GetVolume(ScriptMethodParams &params) {
	PARAMS1(ScriptAudioChannel *, channel);
	params._result = AGS3::AudioChannel_GetVolume(channel);
}

void AudioChannel::SetVolume(ScriptMethodParams &params) {
	PARAMS2(ScriptAudioChannel *, channel, int, newVolume);
	params._result = AGS3::AudioChannel_SetVolume(channel, newVolume);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
