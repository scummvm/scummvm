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

#include "ags/lib/audio/mp3.h"
#include "common/textconsole.h"
#include "ags/ags.h"

namespace AGS3 {

ALMP3_MP3 *almp3_create_mp3(void *data, int data_len) {
	warning("TODO: almp3_create_mp3");
	return nullptr;
}

int almp3_play_mp3(ALMP3_MP3 *mp3, int buffer_len, int vol, int pan) {
	warning("TODO: almp3_play_mp3");
	return 0;
}

int almp3_play_ex_mp3(ALMP3_MP3 *mp3, int buffer_len, int vol, int pan, int speed, int loop) {
	warning("TODO: almp3_play_ex_mp3");
	return 0;
}

void almp3_stop_mp3(ALMP3_MP3 *mp3) {
	::AGS::g_vm->_mixer->stopHandle(mp3->_handle);
}

void almp3_destroy_mp3(ALMP3_MP3 *mp3) {
	delete mp3;
}

int almp3_poll_mp3(ALMP3_MP3 *mp3) {
	return ::AGS::g_vm->_mixer->isSoundHandleActive(mp3->_handle) ? 0 : ALMP3_POLL_PLAYJUSTFINISHED;
}

void almp3_adjust_mp3(ALMP3_MP3 *mp3, int volume, int panning, int speed, bool repeat) {
	warning("TODO: almp3_adjust_mp3");
}

void almp3_seek_abs_msecs_mp3(ALMP3_MP3 *mp3, int pos) {
	warning("TODO: almp3_seek_abs_msecs_mp3");
}

int almp3_get_pos_msecs_mp3(ALMP3_MP3 *mp3) {
	warning("TODO: almp3_get_pos_msecs_mp3");
	return 0;
}

AUDIOSTREAM *almp3_get_audiostream_mp3(ALMP3_MP3 *mp3) {
	warning("TODO: almp3_get_audiostream_mp3");
	return nullptr;
}

int almp3_get_length_msecs_mp3(ALMP3_MP3 *mp3) {
	warning("TODO: almp3_get_length_msecs_mp3");
	return 0;
}


ALMP3_MP3STREAM *almp3_create_mp3stream(void *first_data_buffer, int data_buffer_len, int last_block) {
	warning("TODO: almp3_create_mp3stream");
	return nullptr;
}

int almp3_play_mp3stream(ALMP3_MP3STREAM *mp3, int buffer_len, int vol, int pan) {
	warning("TODO: almp3_play_mp3stream");
	return -1;
}

int almp3_poll_mp3stream(ALMP3_MP3STREAM *mp3) {
	warning("TODO: almp3_poll_mp3stream");
	return 0;
}

void almp3_adjust_mp3stream(ALMP3_MP3STREAM *mp3, int volume, int panning, int speed) {
	warning("TODO: almp3_adjust_mp3stream");
}

void almp3_stop_mp3stream(ALMP3_MP3STREAM *mp3) {
	warning("TODO: almp3_stop_mp3stream");
}

void almp3_destroy_mp3stream(ALMP3_MP3STREAM *mp3) {
	warning("TODO: almp3_destroy_mp3stream");
}

int almp3_get_pos_msecs_mp3stream(ALMP3_MP3STREAM *mp3) {
	warning("TODO: almp3_get_pos_msecs_mp3stream");
	return 0;
}

AUDIOSTREAM *almp3_get_audiostream_mp3stream(ALMP3_MP3STREAM *mp3) {
	warning("TODO: almp3_get_audiostream_mp3stream");
	return nullptr;
}

} // namespace AGS3
