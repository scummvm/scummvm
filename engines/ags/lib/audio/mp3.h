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

#ifndef AGS_LIB_AUDIO_MP3_H
#define AGS_LIB_AUDIO_MP3_H

#include "ags/lib/audio/sound.h"
#include "audio/mixer.h"

namespace AGS3 {

#define ALMP3_OK 0
#define ALMP3_POLL_PLAYJUSTFINISHED 1

typedef Audio::AudioStream ALMP3_MP3STREAM;
struct ALMP3_MP3 {
	ALMP3_MP3STREAM *_stream;
	Audio::SoundHandle _handle;
};

extern ALMP3_MP3 *almp3_create_mp3(void *data, int data_len);
extern void almp3_destroy_mp3(ALMP3_MP3 *mp3);
extern int almp3_play_mp3(ALMP3_MP3 *mp3, int buffer_len, int vol, int pan);
extern int almp3_play_ex_mp3(ALMP3_MP3 *mp3, int buffer_len, int vol, int pan, int speed, int loop);
extern void almp3_stop_mp3(ALMP3_MP3 *mp3);
extern int almp3_poll_mp3(ALMP3_MP3 *mp3);
extern void almp3_adjust_mp3(ALMP3_MP3 *mp3, int volume, int panning, int speed, bool repeat);
extern void almp3_seek_abs_msecs_mp3(ALMP3_MP3 *mp3, int pos);
extern int almp3_get_pos_msecs_mp3(ALMP3_MP3 *mp3);
extern AUDIOSTREAM *almp3_get_audiostream_mp3(ALMP3_MP3 *mp3);
extern int almp3_get_length_msecs_mp3(ALMP3_MP3 *mp3);

extern ALMP3_MP3STREAM *almp3_create_mp3stream(void *first_data_buffer, int data_buffer_len, int last_block);
extern int almp3_play_mp3stream(ALMP3_MP3STREAM *mp3, int buffer_len, int vol, int pan);
extern int almp3_poll_mp3stream(ALMP3_MP3STREAM *mp3);
extern void almp3_adjust_mp3stream(ALMP3_MP3STREAM *mp3, int volume, int panning, int speed);
extern void almp3_stop_mp3stream(ALMP3_MP3STREAM *mp3);
extern void almp3_destroy_mp3stream(ALMP3_MP3STREAM *mp3);
extern int almp3_get_pos_msecs_mp3stream(ALMP3_MP3STREAM *mp3);
extern AUDIOSTREAM *almp3_get_audiostream_mp3stream(ALMP3_MP3STREAM *mp3);
extern int almp3_get_length_msecs_mp3stream(ALMP3_MP3STREAM *mp3, int total_size);

} // namespace AGS3

#endif
