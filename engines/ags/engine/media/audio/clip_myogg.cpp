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

#include "media/audio/audiodefines.h"
#include "media/audio/clip_myogg.h"
#include "media/audio/audiointernaldefs.h"
#include "ac/common.h"               // quit()
#include "ac/asset_helper.h"
#include "util/mutex_lock.h"

#include "platform/base/agsplatformdriver.h"

extern "C" {
    extern int alogg_is_end_of_oggstream(ALOGG_OGGSTREAM *ogg);
    extern int alogg_is_end_of_ogg(ALOGG_OGG *ogg);
    extern int alogg_get_ogg_freq(ALOGG_OGG *ogg);
    extern int alogg_get_ogg_stereo(ALOGG_OGG *ogg);
}

void MYOGG::poll()
{
    if (state_ != SoundClipPlaying) { return; }

    AGS_PACKFILE_OBJ* obj = (AGS_PACKFILE_OBJ*)in->userdata;
    if (obj->remains > 0)
    {
        // update the buffer
        char *tempbuf = (char *)alogg_get_oggstream_buffer(stream);
        if (tempbuf != nullptr)
        {
            int free_val = -1;
            if (chunksize >= obj->remains)
            {
                chunksize = obj->remains;
                free_val = chunksize;
            }
            pack_fread(tempbuf, chunksize, in);
            alogg_free_oggstream_buffer(stream, free_val);
        }
    }

    int ret = alogg_poll_oggstream(stream);
    if (ret == ALOGG_OK || ret == ALOGG_POLL_BUFFERUNDERRUN)
        get_pos_ms();  // call this to keep the last_but_one stuff up to date
    else {
        // finished playing or error
        state_ = SoundClipStopped;
    }
}

void MYOGG::adjust_stream()
{
    if (!is_playing()) { return; }
    alogg_adjust_oggstream(stream, get_final_volume(), panning, speed);
}

void MYOGG::adjust_volume()
{
    adjust_stream();
}

void MYOGG::set_volume(int newvol)
{
    // boost MP3 volume
    newvol += 20;
    if (newvol > 255)
        newvol = 255;
    vol = newvol;
    adjust_stream();
}

void MYOGG::set_speed(int new_speed)
{
    speed = new_speed;
    adjust_stream();
}

void MYOGG::destroy()
{
    if (stream) {
        alogg_stop_oggstream(stream);
        alogg_destroy_oggstream(stream);
    }
    stream = nullptr;

    if (buffer)
        free(buffer);
    buffer = nullptr;

    pack_fclose(in);

    state_ = SoundClipStopped;
}

void MYOGG::seek(int pos)
{
    if (!is_playing()) { return; }
    quit("Attempted to seek an oggstream; operation not permitted");
}

int MYOGG::get_pos()
{
    return 0;
}

int MYOGG::get_pos_ms()
{
    // Unfortunately the alogg_get_pos_msecs_oggstream function
    // returns the ms offset that was last decoded, so it's always
    // ahead of the actual playback. Therefore we have this
    // hideous hack below to sort it out.
    if ((!is_playing()) || (!alogg_is_playing_oggstream(stream)))
        return 0;

    AUDIOSTREAM *str = alogg_get_audiostream_oggstream(stream);
    long offs = (voice_get_position(str->voice) * 1000) / str->samp->freq;

    if (last_ms_offs != alogg_get_pos_msecs_oggstream(stream)) {
        last_but_one_but_one = last_but_one;
        last_but_one = last_ms_offs;
        last_ms_offs = alogg_get_pos_msecs_oggstream(stream);
    }

    // just about to switch buffers
    if (offs < 0)
        return last_but_one;

    int end_of_stream = alogg_is_end_of_oggstream(stream);

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

        return offs + last_but_one;
    }

    return offs + last_but_one_but_one;
}

int MYOGG::get_length_ms()
{  // streamed OGG is variable bitrate so we don't know
    return 0;
}

int MYOGG::get_voice()
{
    if (!is_playing()) { return -1; }

    AUDIOSTREAM *ast = alogg_get_audiostream_oggstream(stream);
    if (ast)
        return ast->voice;
    return -1;
}

int MYOGG::get_sound_type() {
    return MUS_OGG;
}

int MYOGG::play() {
    if (in == nullptr) { return 0; }
    
    if (alogg_play_oggstream(stream, MP3CHUNKSIZE, (vol > 230) ? vol : vol + 20, panning) != ALOGG_OK) {
        return 0;
    }

    state_ = SoundClipPlaying;

    if (!psp_audio_multithreaded)
      poll();

    return 1;
}

MYOGG::MYOGG() : SOUNDCLIP() {
    stream = nullptr;
    in = nullptr;
    buffer = nullptr;
    chunksize = 0;
    last_but_one_but_one = 0;
    last_but_one = 0;
    last_ms_offs = 0;
}
