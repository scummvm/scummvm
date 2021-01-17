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

#include "ags/lib/audio/ogg.h"
#include "common/textconsole.h"
#include "ags/ags.h"

namespace AGS3 {
#ifdef DEPRECATED
ALOGG_OGG *alogg_create_ogg_from_buffer(void *data, int data_len) {
	warning("TODO: alogg_create_ogg_from_buffer");
	return nullptr;
}

void alogg_destroy_ogg(ALOGG_OGG *ogg) {
	delete ogg;
}

int alogg_play_ogg(ALOGG_OGG *ogg, int buffer_len, int vol, int pan) {
	warning("TODO: alogg_play_ogg");
	return 0;
}

int alogg_play_ex_ogg(ALOGG_OGG *ogg, int buffer_len, int vol, int pan, int speed, int loop) {
	warning("TODO: alogg_play_ex_ogg");
	return 0;
}

void alogg_stop_ogg(ALOGG_OGG *ogg) {
	warning("TODO: alogg_stop_ogg");
}

int alogg_poll_ogg(ALOGG_OGG *ogg) {
	return ::AGS::g_vm->_mixer->isSoundHandleActive(ogg->_handle) ? 0 : ALOGG_POLL_PLAYJUSTFINISHED;
}

bool alogg_is_playing_ogg(ALOGG_OGG *ogg) {
	warning("TODO: alogg_is_playing_ogg");
	return false;
}

void alogg_adjust_ogg(ALOGG_OGG *ogg, int volume, int panning, int speed, bool repeat) {
	warning("TODO: alogg_adjust_oggstream");
}

AUDIOSTREAM *alogg_get_audiostream_ogg(ALOGG_OGG *ogg) {
	return ogg->_stream;
}

int alogg_get_pos_msecs_ogg(ALOGG_OGG *ogg) {
	warning("TODO: alogg_get_pos_msecs_ogg");
	return 0;
}

bool alogg_get_wave_is_stereo_ogg(ALOGG_OGG *ogg) {
	warning("TODO: alogg_get_wave_is_stereo_ogg");
	return true;
}

int alogg_get_length_msecs_ogg(ALOGG_OGG *ogg) {
	warning("TODO: alogg_get_length_msecs_ogg");
	return 0;
}

void alogg_seek_abs_msecs_ogg(ALOGG_OGG *ogg, int msecs) {
	warning("TODO: alogg_seek_abs_msecs_ogg");
}

int alogg_get_wave_freq_ogg(ALOGG_OGG *ogg) {
	warning("TODO: alogg_get_wave_freq_ogg");
	return 0;
}

int alogg_is_end_of_ogg(ALOGG_OGG *ogg) {
	warning("TODO: alogg_is_end_of_oggstream");
	return 0;
}


ALOGG_OGGSTREAM *alogg_create_oggstream(void *first_data_buffer, int data_buffer_len, int last_block) {
	warning("TODO: alogg_create_oggstream");
	return nullptr;
}

void alogg_destroy_oggstream(ALOGG_OGGSTREAM *ogg) {
	warning("TODO: alogg_destroy_oggstream");
}

int alogg_play_oggstream(ALOGG_OGGSTREAM *ogg, int buffer_len, int vol, int pan) {
	warning("TODO: alogg_play_oggstream");
	return 0;
}

void alogg_stop_oggstream(ALOGG_OGGSTREAM *ogg) {
	warning("TODO: alogg_stop_oggstream");
}

int alogg_poll_oggstream(ALOGG_OGGSTREAM *ogg) {
	warning("TODO: alogg_poll_oggstream");
	return 0;
}

void alogg_adjust_oggstream(ALOGG_OGGSTREAM *ogg, int volume, int panning, int speed) {
	warning("TODO: alogg_adjust_oggstream");
}

bool alogg_is_playing_oggstream(ALOGG_OGGSTREAM *ogg) {
	warning("TODO: alogg_is_playing_oggstream");
	return false;
}

int alogg_is_end_of_oggstream(ALOGG_OGGSTREAM *ogg) {
	warning("TODO: alogg_is_end_of_oggstream");
	return 0;
}

AUDIOSTREAM *alogg_get_audiostream_oggstream(ALOGG_OGGSTREAM *ogg) {
	warning("TODO: alogg_get_audiostream_oggstream");
	return nullptr;
}

int alogg_get_pos_msecs_oggstream(ALOGG_OGGSTREAM *ogg) {
	warning("TODO: alogg_get_pos_msecs_oggstream");
	return 0;
}
#endif
} // namespace AGS3
