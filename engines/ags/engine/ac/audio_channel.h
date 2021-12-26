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

#ifndef AGS_ENGINE_AC_AUDIO_CHANNEL_H
#define AGS_ENGINE_AC_AUDIO_CHANNEL_H

#include "ags/shared/ac/dynobj/script_audio_clip.h"
#include "ags/engine/ac/dynobj/script_audio_channel.h"

namespace AGS3 {

int     AudioChannel_GetID(ScriptAudioChannel *channel);
int     AudioChannel_GetIsPlaying(ScriptAudioChannel *channel);
int     AudioChannel_GetPanning(ScriptAudioChannel *channel);
void    AudioChannel_SetPanning(ScriptAudioChannel *channel, int newPanning);
ScriptAudioClip *AudioChannel_GetPlayingClip(ScriptAudioChannel *channel);
int     AudioChannel_GetPosition(ScriptAudioChannel *channel);
int     AudioChannel_GetPositionMs(ScriptAudioChannel *channel);
int     AudioChannel_GetLengthMs(ScriptAudioChannel *channel);
int     AudioChannel_GetVolume(ScriptAudioChannel *channel);
int     AudioChannel_SetVolume(ScriptAudioChannel *channel, int newVolume);
void    AudioChannel_Stop(ScriptAudioChannel *channel);
void    AudioChannel_Seek(ScriptAudioChannel *channel, int newPosition);
void    AudioChannel_SetRoomLocation(ScriptAudioChannel *channel, int xPos, int yPos);

} // namespace AGS3

#endif
