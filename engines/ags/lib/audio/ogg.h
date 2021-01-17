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

#ifndef AGS_LIB_AUDIO_OGG_H
#define AGS_LIB_AUDIO_OGG_H

#include "ags/lib/audio/sound.h"
#include "audio/mixer.h"

namespace AGS3 {
#ifdef DEPRECATED
#define ALOGG_OK 0

#define ALOGG_PLAY_BUFFERTOOSMALL    -1

#define ALOGG_POLL_PLAYJUSTFINISHED  1
#define ALOGG_POLL_NOTPLAYING        -1
#define ALOGG_POLL_FRAMECORRUPT      -2
#define ALOGG_POLL_BUFFERUNDERRUN    -3
#define ALOGG_POLL_INTERNALERROR     -4

typedef AUDIOSTREAM ALOGG_OGGSTREAM;
struct ALOGG_OGG {
	ALOGG_OGGSTREAM *_stream;
	Audio::SoundHandle _handle;
};

extern ALOGG_OGG *alogg_create_ogg_from_buffer(void *data, int data_len);
extern void alogg_destroy_ogg(ALOGG_OGG *ogg);
extern int alogg_play_ogg(ALOGG_OGG *ogg, int buffer_len, int vol, int pan);
extern int alogg_play_ex_ogg(ALOGG_OGG *ogg, int buffer_len, int vol, int pan, int speed, int loop);
extern void alogg_stop_ogg(ALOGG_OGG *ogg);
extern int alogg_poll_ogg(ALOGG_OGG *ogg);
extern bool alogg_is_playing_ogg(ALOGG_OGG *ogg);
extern void alogg_adjust_ogg(ALOGG_OGG *ogg, int volume, int panning, int speed, bool repeat);
extern AUDIOSTREAM *alogg_get_audiostream_ogg(ALOGG_OGG *ogg);
extern int alogg_get_pos_msecs_ogg(ALOGG_OGG *ogg);
extern bool alogg_get_wave_is_stereo_ogg(ALOGG_OGG *ogg);
extern int alogg_get_length_msecs_ogg(ALOGG_OGG *ogg);
extern void alogg_seek_abs_msecs_ogg(ALOGG_OGG *ogg, int msecs);
extern int alogg_get_wave_freq_ogg(ALOGG_OGG *ogg);
extern int alogg_is_end_of_ogg(ALOGG_OGG *ogg);

ALOGG_OGGSTREAM *alogg_create_oggstream(void *first_data_buffer, int data_buffer_len, int last_block);
extern int alogg_play_oggstream(ALOGG_OGGSTREAM *ogg, int buffer_len, int vol, int pan);
extern void alogg_stop_oggstream(ALOGG_OGGSTREAM *ogg);
extern void alogg_destroy_oggstream(ALOGG_OGGSTREAM *ogg);
extern int alogg_poll_oggstream(ALOGG_OGGSTREAM *ogg);
extern void alogg_adjust_oggstream(ALOGG_OGGSTREAM *ogg, int volume, int panning, int speed);
extern bool alogg_is_playing_oggstream(ALOGG_OGGSTREAM *ogg);
extern int alogg_is_end_of_oggstream(ALOGG_OGGSTREAM *ogg);
extern AUDIOSTREAM *alogg_get_audiostream_oggstream(ALOGG_OGGSTREAM *ogg);
extern int alogg_get_pos_msecs_oggstream(ALOGG_OGGSTREAM *ogg);
#endif
} // namespace AGS3

#endif
