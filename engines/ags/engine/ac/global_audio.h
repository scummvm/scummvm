//=============================================================================
//
// Adventure Game Studio (AGS)
//
// Copyright (C) 1999-2011 Chris Jones and 2011-20xx others
// The full list of copyright holders can be found in the Copyright.txt
// file, which is part of this source code distribution.
//
// The AGS source code is provided under the Artistic License 2.0.
// A copy of this license can be found in the file License.txt and at
// http://www.opensource.org/licenses/artistic-license-2.0.php
//
//=============================================================================
//
//
//
//=============================================================================
#ifndef __AGS_EE_AC__GLOBALAUDIO_H
#define __AGS_EE_AC__GLOBALAUDIO_H

void    StopAmbientSound (int channel);
void    PlayAmbientSound (int channel, int sndnum, int vol, int x, int y);
int     IsChannelPlaying(int chan);
int     IsSoundPlaying();
// returns -1 on failure, channel number on success
int     PlaySoundEx(int val1, int channel);
void    StopAllSounds(int evenAmbient);

void    PlayMusicResetQueue(int newmus);
void    SeekMIDIPosition (int position);
int     GetMIDIPosition ();
int     IsMusicPlaying();
int     PlayMusicQueued(int musnum);
void    scr_StopMusic();
void    SeekMODPattern(int patnum);
void    SeekMP3PosMillis (int posn);
int     GetMP3PosMillis ();
void    SetMusicVolume(int newvol);
void    SetMusicMasterVolume(int newvol);
void    SetSoundVolume(int newvol);
void    SetChannelVolume(int chan, int newvol);
void    SetDigitalMasterVolume (int newvol);
int     GetCurrentMusic();
void    SetMusicRepeat(int loopflag);
void    PlayMP3File (const char *filename);
void    PlaySilentMIDI (int mnum);

void    SetSpeechVolume(int newvol);
void    SetVoiceMode (int newmod);
int     GetVoiceMode ();
int     IsVoxAvailable();
int     IsMusicVoxAvailable ();

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

#endif // __AGS_EE_AC__GLOBALAUDIO_H
