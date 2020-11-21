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
#include "media/audio/clip_mystaticogg.h"
#include "media/audio/audiointernaldefs.h"
#include "media/audio/soundcache.h"
#include "util/mutex_lock.h"

#include "platform/base/agsplatformdriver.h"

extern "C" {
    extern int alogg_is_end_of_oggstream(ALOGG_OGGSTREAM *ogg);
    extern int alogg_is_end_of_ogg(ALOGG_OGG *ogg);
    extern int alogg_get_ogg_freq(ALOGG_OGG *ogg);
    extern int alogg_get_ogg_stereo(ALOGG_OGG *ogg);
}

extern int use_extra_sound_offset;  // defined in ac.cpp

void MYSTATICOGG::poll()
{
    if (state_ != SoundClipPlaying) { return; }

    if (alogg_poll_ogg(tune) == ALOGG_POLL_PLAYJUSTFINISHED) {
        if (!repeat)
        {
            state_ = SoundClipStopped;
        }
    }
    else get_pos();  // call this to keep the last_but_one stuff up to date
}

void MYSTATICOGG::adjust_stream()
{
    if (!is_playing()) { return; }
    alogg_adjust_ogg(tune, get_final_volume(), panning, speed, repeat);
}

void MYSTATICOGG::adjust_volume()
{
    adjust_stream();
}

void MYSTATICOGG::set_volume(int newvol)
{
    vol = newvol;
    adjust_stream();
}

void MYSTATICOGG::set_speed(int new_speed)
{
    speed = new_speed;
    adjust_stream();
}

void MYSTATICOGG::destroy()
{
    if (tune) {
        alogg_stop_ogg(tune);
        alogg_destroy_ogg(tune);
    }
    tune = nullptr;

    if (mp3buffer) {
        sound_cache_free(mp3buffer, false);
    }
    mp3buffer = nullptr;

    state_ = SoundClipStopped;
}

void MYSTATICOGG::seek(int pos)
{
    if (!is_playing()) { return; }

    // we stop and restart it because otherwise the buffer finishes
    // playing first and the seek isn't quite accurate
    alogg_stop_ogg(tune);
    state_ = SoundClipInitial;
    play_from(pos);
}

int MYSTATICOGG::get_pos()
{
    if (!is_playing()) { return -1; }
    return get_pos_ms();
}

int MYSTATICOGG::get_pos_ms()
{
    if (!is_playing()) { return -1; }

    // Unfortunately the alogg_get_pos_msecs function
    // returns the ms offset that was last decoded, so it's always
    // ahead of the actual playback. Therefore we have this
    // hideous hack below to sort it out.
    if (!alogg_is_playing_ogg(tune))
        return 0;

    AUDIOSTREAM *str = alogg_get_audiostream_ogg(tune);
    long offs = (voice_get_position(str->voice) * 1000) / str->samp->freq;

    if (last_ms_offs != alogg_get_pos_msecs_ogg(tune)) {
        last_but_one_but_one = last_but_one;
        last_but_one = last_ms_offs;
        last_ms_offs = alogg_get_pos_msecs_ogg(tune);
    }

    // just about to switch buffers
    if (offs < 0)
        return last_but_one;

    int end_of_stream = alogg_is_end_of_ogg(tune);

    if ((str->active == 1) && (last_but_one_but_one > 0) && (str->locked == nullptr)) {
        switch (end_of_stream) {
case 0:
case 2:
    offs -= (last_but_one - last_but_one_but_one);
    break;
case 1:
    offs -= (last_but_one - last_but_one_but_one);
    break;
        }
    }

    if (end_of_stream == 1) {
        return offs + last_but_one + extraOffset;
    }

    return offs + last_but_one_but_one + extraOffset;
}

int MYSTATICOGG::get_length_ms()
{
    if (tune == nullptr) { return -1; }
    return alogg_get_length_msecs_ogg(tune);
}

int MYSTATICOGG::get_voice()
{
    if (!is_playing()) { return -1; }
    AUDIOSTREAM *ast = alogg_get_audiostream_ogg(tune);
    if (ast)
        return ast->voice;
    return -1;
}

int MYSTATICOGG::get_sound_type() {
    return MUS_OGG;
}

int MYSTATICOGG::play_from(int position)
{
    if (tune == nullptr) { return 0; }

    if (use_extra_sound_offset) 
        extraOffset = ((16384 / (alogg_get_wave_is_stereo_ogg(tune) ? 2 : 1)) * 1000) / alogg_get_wave_freq_ogg(tune);
    else
        extraOffset = 0;

    if (alogg_play_ex_ogg(tune, 16384, vol, panning, 1000, repeat) != ALOGG_OK) {
        return 0;
    }

    last_ms_offs = position;
    last_but_one = position;
    last_but_one_but_one = position;

    if (position > 0)
        alogg_seek_abs_msecs_ogg(tune, position);

    state_ = SoundClipPlaying;

    if (!psp_audio_multithreaded)
      poll();

    return 1;
}

int MYSTATICOGG::play() {
    return play_from(0);
}

MYSTATICOGG::MYSTATICOGG() : SOUNDCLIP() {
    tune = nullptr;
    mp3buffer = nullptr;
    mp3buffersize = 0;
    extraOffset = 0;
    last_but_one = 0;
    last_ms_offs = 0;
    last_but_one_but_one = 0;
}
