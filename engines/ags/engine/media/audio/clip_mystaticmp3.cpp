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

#include "ags/engine/media/audio/clip_mystaticmp3.h"
#include "ags/engine/media/audio/audiointernaldefs.h"
#include "ags/engine/media/audio/soundcache.h"
#include "ags/engine/util/mutex_lock.h"

#include "ags/engine/platform/base/agsplatformdriver.h"

namespace AGS3 {

extern int our_eip;

// ALMP3 functions are not reentrant! This mutex should be locked before calling any
// of the mp3 functions and unlocked afterwards.
AGS::Engine::Mutex _mp3_mutex;

void MYSTATICMP3::poll() {
	if (state_ != SoundClipPlaying) {
		return;
	}

	int oldeip = our_eip;
	our_eip = 5997;

	AGS::Engine::MutexLock _lockMp3(_mp3_mutex);
	int result = almp3_poll_mp3(tune);

	if (result == ALMP3_POLL_PLAYJUSTFINISHED) {
		if (!repeat) {
			state_ = SoundClipStopped;
		}
	}
	our_eip = oldeip;
}

void MYSTATICMP3::adjust_stream() {
	if (!is_playing()) {
		return;
	}
	AGS::Engine::MutexLock _lockMp3(_mp3_mutex);
	almp3_adjust_mp3(tune, get_final_volume(), panning, speed, repeat);
}

void MYSTATICMP3::adjust_volume() {
	adjust_stream();
}

void MYSTATICMP3::set_volume(int newvol) {
	vol = newvol;
	adjust_stream();
}

void MYSTATICMP3::set_speed(int new_speed) {
	speed = new_speed;
	adjust_stream();
}

void MYSTATICMP3::destroy() {
	if (tune) {
		AGS::Engine::MutexLock _lockMp3(_mp3_mutex);
		almp3_stop_mp3(tune);
		almp3_destroy_mp3(tune);
	}
	tune = nullptr;

	if (mp3buffer) {
		sound_cache_free(mp3buffer, false);
	}
	mp3buffer = nullptr;

	state_ = SoundClipStopped;
}

void MYSTATICMP3::seek(int pos) {
	if (!is_playing()) {
		return;
	}
	AGS::Engine::MutexLock _lockMp3(_mp3_mutex);
	almp3_seek_abs_msecs_mp3(tune, pos);
}

int MYSTATICMP3::get_pos() {
	if (!is_playing()) {
		return -1;
	}
	AGS::Engine::MutexLock _lockMp3(_mp3_mutex);
	return almp3_get_pos_msecs_mp3(tune);
}

int MYSTATICMP3::get_pos_ms() {
	int result = get_pos();
	return result;
}

int MYSTATICMP3::get_length_ms() {
	if (tune == nullptr) {
		return -1;
	}
	AGS::Engine::MutexLock _lockMp3(_mp3_mutex);
	return almp3_get_length_msecs_mp3(tune);
}

int MYSTATICMP3::get_voice() {
	if (!is_playing()) {
		return -1;
	}
	AGS::Engine::MutexLock _lockMp3(_mp3_mutex);
	AUDIOSTREAM *ast = almp3_get_audiostream_mp3(tune);
	return (ast != nullptr ? ast->voice : -1);
}

int MYSTATICMP3::get_sound_type() {
	return MUS_MP3;
}

int MYSTATICMP3::play() {
	if (tune == nullptr) {
		return 0;
	}

	{
		AGS::Engine::MutexLock _lockMp3(_mp3_mutex);
		int result = almp3_play_ex_mp3(tune, 16384, vol, panning, 1000, repeat);
		if (result != ALMP3_OK) {
			return 0;
		}
	}

	state_ = SoundClipPlaying;

	if (!psp_audio_multithreaded)
		poll();

	return 1;
}

MYSTATICMP3::MYSTATICMP3() : SOUNDCLIP() {
	tune = nullptr;
	mp3buffer = nullptr;
}

} // namespace AGS3

#endif
