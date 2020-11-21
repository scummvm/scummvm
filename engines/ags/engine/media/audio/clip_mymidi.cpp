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

#include "media/audio/audiodefines.h"
#include "util/wgt2allg.h"
#include "media/audio/clip_mymidi.h"
#include "media/audio/audiointernaldefs.h"

void MYMIDI::poll()
{
    if (state_ != SoundClipPlaying) { return; }

    if (midi_pos < 0)
        state_ = SoundClipStopped;
}

void MYMIDI::adjust_volume()
{
    if (!is_playing()) { return; }
    ::set_volume(-1, get_final_volume());
}

void MYMIDI::set_volume(int newvol)
{
    vol = newvol;
    adjust_volume();
}

void MYMIDI::destroy()
{
    stop_midi();
    
    if (tune) {
        destroy_midi(tune);
    }
    tune = nullptr;

    state_ = SoundClipStopped;
}

void MYMIDI::seek(int pos)
{
    if (!is_playing()) { return; }
    midi_seek(pos);
}

int MYMIDI::get_pos()
{
    if (!is_playing()) { return -1; }
    return midi_pos;
}

int MYMIDI::get_pos_ms()
{
    return 0;                   // we don't know ms with midi
}

int MYMIDI::get_length_ms()
{
    return lengthInSeconds * 1000;
}

int MYMIDI::get_voice()
{
    // voice is N/A for midi
    return -1;
}

void MYMIDI::pause() {
    if (state_ != SoundClipPlaying) { return; }
    midi_pause();
    state_ = SoundClipPaused;
}

void MYMIDI::resume() {
    if (state_ != SoundClipPaused) { return; }
    midi_resume();
    state_ = SoundClipPlaying;
}

int MYMIDI::get_sound_type() {
    return MUS_MIDI;
}

int MYMIDI::play() {
    if (tune == nullptr) { return 0; }

    lengthInSeconds = get_midi_length(tune);
    if (::play_midi(tune, repeat)) {
        return 0;
    }

    state_ = SoundClipPlaying;
    return 1;
}

MYMIDI::MYMIDI() : SOUNDCLIP() {
    tune = nullptr;
    lengthInSeconds = 0;
}
