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

#include "util/wgt2allg.h"
#include "media/audio/audiodefines.h"
#include "media/audio/clip_mywave.h"
#include "media/audio/audiointernaldefs.h"
#include "media/audio/soundcache.h"
#include "util/mutex_lock.h"

#include "platform/base/agsplatformdriver.h"

void MYWAVE::poll()
{
    if (state_ != SoundClipPlaying) { return; }

    if (voice_get_position(voice) < 0)
    {
        state_ = SoundClipStopped;
    }
}

void MYWAVE::adjust_volume()
{
    if (!is_playing()) { return; }
    if (voice < 0) { return; }
    voice_set_volume(voice, get_final_volume());
}

void MYWAVE::set_volume(int newvol)
{
    vol = newvol;
    adjust_volume();
}

void MYWAVE::destroy()
{
    // Stop sound and decrease reference count.
    if (wave) {
        stop_sample(wave);
        sound_cache_free((char*)wave, true);
    }
    wave = nullptr; 

    state_ = SoundClipStopped;
}

void MYWAVE::seek(int pos)
{
    if (!is_playing()) { return; }
    voice_set_position(voice, pos);
}

int MYWAVE::get_pos()
{
    if (!is_playing()) { return -1; }
    return voice_get_position(voice);
}

int MYWAVE::get_pos_ms()
{
    // convert the offset in samples into the offset in ms
    //return ((1000000 / voice_get_frequency(voice)) * voice_get_position(voice)) / 1000;

    if (voice_get_frequency(voice) < 100)
        return 0;
    // (number of samples / (samples per second / 100)) * 10 = ms
    return (voice_get_position(voice) / (voice_get_frequency(voice) / 100)) * 10;
}

int MYWAVE::get_length_ms()
{
    if (wave == nullptr) { return -1; }
    if (wave->freq < 100)
        return 0;
    return (wave->len / (wave->freq / 100)) * 10;
}

int MYWAVE::get_voice()
{
    if (!is_playing()) { return -1; }
    return voice;
}

int MYWAVE::get_sound_type() {
    return MUS_WAVE;
}

int MYWAVE::play() {
    if (wave == nullptr) { return 0; }

    voice = play_sample(wave, vol, panning, 1000, repeat);
    if (voice < 0) {
        return 0;
    }

    state_ = SoundClipPlaying;

    return 1;
}

MYWAVE::MYWAVE() : SOUNDCLIP() {
    wave = nullptr;
    voice = -1;
}
