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

#include "ags/engine/media/audio/audiodefines.h"

#ifndef NO_MP3_PLAYER

#include "ags/engine/media/audio/clip_mymp3.h"
#include "ags/engine/media/audio/audiointernaldefs.h"
#include "ags/shared/ac/common.h"               // quit()
#include "ags/engine/ac/asset_helper.h"
#include "ags/engine/util/mutex_lock.h"
#include "ags/engine/platform/base/agsplatformdriver.h"
#include "ags/ags.h"

namespace AGS3 {

void MYMP3::poll() {
	if (state_ != SoundClipPlaying) {
		return;
	}
#if !AGS_PLATFORM_SCUMMVM
	// update the buffer
	char *tempbuf = nullptr;
	{
		AGS::Engine::MutexLock _lockMp3(::AGS::g_vm->_mp3Mutex);
		tempbuf = (char *)almp3_get_mp3stream_buffer(stream);
	}

	if (tempbuf != nullptr) {
		AGS_PACKFILE_OBJ *obj = (AGS_PACKFILE_OBJ *)in->userdata;
		int free_val = -1;
		if (chunksize >= obj->remains) {
			chunksize = obj->remains;
			free_val = chunksize;
		}
		pack_fread(tempbuf, chunksize, in);

		{
			AGS::Engine::MutexLock _lockMp3(::AGS::g_vm->_mp3Mutex);
			almp3_free_mp3stream_buffer(stream, free_val);
		}
	}
#endif
	{
		AGS::Engine::MutexLock _lockMp3(::AGS::g_vm->_mp3Mutex);
		if (almp3_poll_mp3stream(stream) == ALMP3_POLL_PLAYJUSTFINISHED) {
			state_ = SoundClipStopped;
		}
	}
}

void MYMP3::adjust_stream() {
	if (!is_playing()) {
		return;
	}
	AGS::Engine::MutexLock _lockMp3(::AGS::g_vm->_mp3Mutex);
	almp3_adjust_mp3stream(stream, get_final_volume(), panning, speed);
}

void MYMP3::adjust_volume() {
	adjust_stream();
}

void MYMP3::set_volume(int newvol) {
	// boost MP3 volume
	newvol += 20;
	if (newvol > 255)
		newvol = 255;

	vol = newvol;
	adjust_stream();
}

void MYMP3::set_speed(int new_speed) {
	speed = new_speed;
	adjust_stream();
}

void MYMP3::destroy() {
	if (stream) {
		AGS::Engine::MutexLock _lockMp3(::AGS::g_vm->_mp3Mutex);
		almp3_stop_mp3stream(stream);
		almp3_destroy_mp3stream(stream);
	}
	stream = nullptr;

	if (buffer)
		free(buffer);
	buffer = nullptr;

	pack_fclose(in);

	state_ = SoundClipStopped;
}

void MYMP3::seek(int pos) {
	if (!is_playing()) {
		return;
	}
	quit("Tried to seek an mp3stream");
}

int MYMP3::get_pos() {
	return 0; // Return 0 to signify that Seek is not supported
	// return almp3_get_pos_msecs_mp3stream (stream);
}

int MYMP3::get_pos_ms() {
	if (!is_playing()) {
		return -1;
	}
	AGS::Engine::MutexLock _lockMp3(::AGS::g_vm->_mp3Mutex);
	return almp3_get_pos_msecs_mp3stream(stream);
}

int MYMP3::get_length_ms() {
	if (!is_playing()) {
		return -1;
	}
	AGS::Engine::MutexLock _lockMp3(::AGS::g_vm->_mp3Mutex);
	return almp3_get_length_msecs_mp3stream(stream, filesize);
}

int MYMP3::get_voice() {
	if (!is_playing()) {
		return -1;
	}
	AGS::Engine::MutexLock _lockMp3(::AGS::g_vm->_mp3Mutex);
	AUDIOSTREAM *ast = almp3_get_audiostream_mp3stream(stream);
	return (ast != nullptr ? ast->voice : -1);
}

int MYMP3::get_sound_type() {
	return MUS_MP3;
}

int MYMP3::play() {
	if (in == nullptr) {
		return 0;
	}

	{
		AGS::Engine::MutexLock _lockMp3(::AGS::g_vm->_mp3Mutex);
		if (almp3_play_mp3stream(stream, chunksize, (vol > 230) ? vol : vol + 20, panning) != ALMP3_OK) {
			return 0;
		}
	}

	state_ = SoundClipPlaying;

	if (!psp_audio_multithreaded)
		poll();

	return 1;
}

MYMP3::MYMP3() : SOUNDCLIP() {
	stream = nullptr;
	in = nullptr;
	filesize = 0;
	buffer = nullptr;
	chunksize = 0;
}

} // namespace AGS3

#endif
