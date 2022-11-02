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

#ifndef AGS_ENGINE_AC_GLOBAL_AUDIO_H
#define AGS_ENGINE_AC_GLOBAL_AUDIO_H

#include "ags/engine/ac/speech.h"

namespace AGS3 {

void    StopAmbientSound(int channel);
void    PlayAmbientSound(int channel, int sndnum, int vol, int x, int y);
int     IsChannelPlaying(int chan);
int     IsSoundPlaying();
// returns -1 on failure, channel number on success
int     PlaySoundEx(int val1, int channel);
void    StopAllSounds(int evenAmbient);

void    PlayMusicResetQueue(int newmus);
void    SeekMIDIPosition(int position);
int     GetMIDIPosition();
int     IsMusicPlaying();
int     PlayMusicQueued(int musnum);
void    scr_StopMusic();
void    SeekMODPattern(int patnum);
void    SeekMP3PosMillis(int posn);
int     GetMP3PosMillis();
void    SetMusicVolume(int newvol);
void    SetMusicMasterVolume(int newvol);
void    SetSoundVolume(int newvol);
void    SetChannelVolume(int chan, int newvol);
void    SetDigitalMasterVolume(int newvol);
int     GetCurrentMusic();
void    SetMusicRepeat(int loopflag);
void    PlayMP3File(const char *filename);
void    PlaySilentMIDI(int mnum);

void    SetSpeechVolume(int newvol);
void    SetVoiceMode(int newmod);
int     GetVoiceMode();
int     IsVoxAvailable();
int     IsMusicVoxAvailable();

struct CharacterInfo;
struct ScriptAudioChannel;
// Starts voice-over playback and returns audio channel it is played on;
// as_speech flag determines whether engine should apply speech-related logic
// as well, such as temporary volume reduction.
ScriptAudioChannel *PlayVoiceClip(CharacterInfo *ch, int sndid, bool as_speech);

//=============================================================================
// Play voice-over for the active blocking speech and initialize relevant data
bool    play_voice_speech(int charid, int sndid);
// Play voice-over clip in non-blocking manner
bool    play_voice_nonblocking(int charid, int sndid, bool as_speech);
// Stop voice-over for the active speech and reset relevant data
void    stop_voice_speech();
// Stop non-blocking voice-over and revert audio volumes if necessary
void    stop_voice_nonblocking();

} // namespace AGS3

#endif
