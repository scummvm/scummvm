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

#include "ags/plugins/core/audio_clip.h"
#include "ags/engine/ac/audio_clip.h"

namespace AGS3 {
namespace Plugins {
namespace Core {

void AudioClip::AGS_EngineStartup(IAGSEngine *engine) {
	ScriptContainer::AGS_EngineStartup(engine);

	SCRIPT_METHOD(AudioClip::Play^2, AudioClip::Play);
	SCRIPT_METHOD(AudioClip::PlayFrom^3, AudioClip::PlayFrom);
	SCRIPT_METHOD(AudioClip::PlayQueued^2, AudioClip::PlayQueued);
	SCRIPT_METHOD(AudioClip::Stop^0, AudioClip::Stop);
	SCRIPT_METHOD(AudioClip::get_FileType, AudioClip::GetFileType);
	SCRIPT_METHOD(AudioClip::get_IsAvailable, AudioClip::GetIsAvailable);
	SCRIPT_METHOD(AudioClip::get_Type, AudioClip::GetType);
}

void AudioClip::Play(ScriptMethodParams &params) {
	PARAMS3(ScriptAudioClip *, clip, int, priority, int, repeat);
	params._result = AGS3::AudioClip_Play(clip, priority, repeat);
}

void AudioClip::PlayFrom(ScriptMethodParams &params) {
	PARAMS4(ScriptAudioClip *, clip, int, position, int, priority, int, repeat);
	params._result = AGS3::AudioClip_PlayFrom(clip, position, priority, repeat);
}

void AudioClip::PlayQueued(ScriptMethodParams &params) {
	PARAMS3(ScriptAudioClip *, clip, int, priority, int, repeat);
	params._result = AGS3::AudioClip_PlayQueued(clip, priority, repeat);
}

void AudioClip::Stop(ScriptMethodParams &params) {
	PARAMS1(ScriptAudioClip *, clip);
	AGS3::AudioClip_Stop(clip);
}

void AudioClip::GetFileType(ScriptMethodParams &params) {
	PARAMS1(ScriptAudioClip *, clip);
	params._result = AGS3::AudioClip_GetFileType(clip);
}

void AudioClip::GetIsAvailable(ScriptMethodParams &params) {
	PARAMS1(ScriptAudioClip *, clip);
	params._result = AGS3::AudioClip_GetIsAvailable(clip);
}

void AudioClip::GetType(ScriptMethodParams &params) {
	PARAMS1(ScriptAudioClip *, clip);
	params._result = AGS3::AudioClip_GetType(clip);
}

} // namespace Core
} // namespace Plugins
} // namespace AGS3
