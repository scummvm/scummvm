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

#include "ags/lib/audio/digi.h"
#include "ags/shared/util/wgt2allg.h"
#include "ags/engine/media/audio/audiodefines.h"
#include "ags/engine/media/audio/clip_mywave.h"
#include "ags/engine/media/audio/audiointernaldefs.h"
#include "ags/engine/media/audio/soundcache.h"
#include "ags/engine/util/mutex_lock.h"

#include "ags/engine/platform/base/agsplatformdriver.h"

namespace AGS3 {
#ifdef  DEPRECATED

void MYWAVE::poll() {
	if (state_ != SoundClipPlaying) {
		return;
	}

	if (voice_get_position(voice) < 0) {
		state_ = SoundClipStopped;
	}
}

void MYWAVE::adjust_volume() {
	if (!is_playing()) {
		return;
	}
	if (voice < 0) {
		return;
	}
	voice_set_volume(voice, get_final_volume());
}

void MYWAVE::set_volume(int newvol) {
	vol = newvol;
	adjust_volume();
}

void MYWAVE::destroy() {
	// Stop sound and decrease reference count.
	if (wave) {
		stop_sample(wave);
		sound_cache_free((char *)wave, true);
	}
	wave = nullptr;

	state_ = SoundClipStopped;
}

void MYWAVE::seek(int pos) {
	if (!is_playing()) {
		return;
	}
	voice_set_position(voice, pos);
}

int MYWAVE::get_pos() {
	if (!is_playing()) {
		return -1;
	}
	return voice_get_position(voice);
}

int MYWAVE::get_pos_ms() {
	// convert the offset in samples into the offset in ms
	//return ((1000000 / voice_get_frequency(voice)) * voice_get_position(voice)) / 1000;

	if (voice_get_frequency(voice) < 100)
		return 0;
	// (number of samples / (samples per second / 100)) * 10 = ms
	return (voice_get_position(voice) / (voice_get_frequency(voice) / 100)) * 10;
}

int MYWAVE::get_length_ms() {
	if (wave == nullptr) {
		return -1;
	}
	if (wave->freq < 100)
		return 0;
	return (wave->len / (wave->freq / 100)) * 10;
}

int MYWAVE::get_voice() {
	if (!is_playing()) {
		return -1;
	}
	return voice;
}

int MYWAVE::get_sound_type() {
	return MUS_WAVE;
}

int MYWAVE::play() {
	if (wave == nullptr) {
		return 0;
	}

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
#endif
} // namespace AGS3
