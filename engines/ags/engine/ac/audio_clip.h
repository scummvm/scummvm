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

#ifndef AGS_ENGINE_AC_AUDIO_CLIP_H
#define AGS_ENGINE_AC_AUDIO_CLIP_H

#include "ags/shared/ac/dynobj/script_audio_clip.h"
#include "ags/engine/ac/dynobj/script_audio_channel.h"

namespace AGS3 {

int     AudioClip_GetFileType(ScriptAudioClip *clip);
int     AudioClip_GetType(ScriptAudioClip *clip);
int     AudioClip_GetIsAvailable(ScriptAudioClip *clip);
void    AudioClip_Stop(ScriptAudioClip *clip);
ScriptAudioChannel *AudioClip_Play(ScriptAudioClip *clip, int priority, int repeat);
ScriptAudioChannel *AudioClip_PlayFrom(ScriptAudioClip *clip, int position, int priority, int repeat);
ScriptAudioChannel *AudioClip_PlayQueued(ScriptAudioClip *clip, int priority, int repeat);
ScriptAudioChannel *AudioClip_PlayOnChannel(ScriptAudioClip *clip, int chan, int priority, int repeat);

} // namespace AGS3

#endif
